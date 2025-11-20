#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <libxml/xmlreader.h>
#include <unistd.h>
#include <glob.h>
#include <sys/stat.h>

#define APP_NAME "Anāsrava"
#define VERSION "1.0"

// 历史记录类型枚举
typedef enum {
    RECENTLY_USED = 0,
    BASH_HISTORY,
    ZSH_HISTORY,
    POWERSHELL_HISTORY,
    FIREFOX_HISTORY,
    CHROME_HISTORY,
    OTHER_HISTORY
} HistoryType;

// 全局变量
GtkWidget *main_window;
GtkWidget *history_list;
GtkWidget *content_view;
GtkWidget *status_label;
GList *current_entries = NULL;
GtkWidget *type_combo;

// 历史记录条目结构
typedef struct {
    gchar *id;
    gchar *title;
    gchar *url;
    gchar *timestamp;
    gchar *description;
    gchar *applications;  // 存储应用程序信息
    HistoryType type;
} HistoryEntry;

// 文件路径
const gchar *history_files[] = {
    "~/.local/share/recently-used.xbel",
    "~/.bash_history",
    "~/.zsh_history",
    "~/.local/share/powershell/PSReadLine/ConsoleHost_history.txt",
    "~/.mozilla/firefox/*/places.sqlite",
    "~/.config/google-chrome/Default/History",
    "~/.config/chromium/Default/History",
    "~/.config/microsoft-edge/Default/History"
};

// 函数声明
gchar* expand_path(const gchar *path);
gboolean file_exists(const gchar *path);
gchar* find_file_by_pattern(const gchar *pattern);
gint64 get_file_size(const gchar *path);
void update_status(const gchar *message);
GList* load_recently_used();
GList* load_shell_history(const gchar *path, HistoryType type);
GList* load_powershell_history();
GList* load_browser_history(HistoryType type);
void load_history(GtkWidget *widget, gpointer user_data);
void on_selection_changed(GtkTreeSelection *selection, gpointer user_data);
void delete_selected(GtkWidget *widget, gpointer user_data);
void clear_all_history(GtkWidget *widget, gpointer user_data);
GtkWidget* create_main_window();

// 展开路径中的波浪号
gchar* expand_path(const gchar *path) {
    if (path[0] == '~') {
        const gchar *home = g_get_home_dir();
        return g_build_filename(home, path + 1, NULL);
    }
    return g_strdup(path);
}

// 检查文件是否存在
gboolean file_exists(const gchar *path) {
    gchar *expanded_path = expand_path(path);
    gboolean exists = g_file_test(expanded_path, G_FILE_TEST_EXISTS);
    g_free(expanded_path);
    return exists;
}

// 使用glob模式查找文件
gchar* find_file_by_pattern(const gchar *pattern) {
    gchar *expanded_pattern = expand_path(pattern);
    glob_t glob_result;
    gchar *found_path = NULL;
    
    if (glob(expanded_pattern, 0, NULL, &glob_result) == 0) {
        if (glob_result.gl_pathc > 0) {
            found_path = g_strdup(glob_result.gl_pathv[0]);
        }
    }
    
    globfree(&glob_result);
    g_free(expanded_pattern);
    return found_path;
}

// 获取文件大小
gint64 get_file_size(const gchar *path) {
    gchar *expanded_path = expand_path(path);
    struct stat st;
    gint64 size = -1;
    
    if (stat(expanded_path, &st) == 0) {
        size = st.st_size;
    }
    
    g_free(expanded_path);
    return size;
}

// 更新状态标签
void update_status(const gchar *message) {
    if (status_label) {
        gtk_label_set_text(GTK_LABEL(status_label), message);
        // 强制刷新GUI
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
    }
}

// 从文件路径提取文件名
gchar* get_filename_from_path(const gchar *path) {
    const gchar *filename = strrchr(path, '/');
    if (filename) {
        return g_strdup(filename + 1);
    }
    return g_strdup(path);
}

// 加载最近使用文件的内容
GList* load_recently_used() {
    GList *entries = NULL;
    gchar *path = expand_path(history_files[RECENTLY_USED]);
    
    if (!file_exists(path)) {
        g_free(path);
        update_status("recently-used.xbel not found");
        return NULL;
    }
    
    gint64 file_size = get_file_size(path);
    if (file_size == 0) {
        g_free(path);
        update_status("recently-used.xbel is empty");
        return NULL;
    }
    
    xmlTextReaderPtr reader = xmlReaderForFile(path, NULL, 0);
    if (reader == NULL) {
        g_free(path);
        update_status("Failed to parse recently-used.xbel");
        return NULL;
    }
    
    int ret;
    gchar *current_href = NULL;
    gchar *current_modified = NULL;
    gchar *current_added = NULL;
    GString *current_applications = NULL;
    int entry_count = 0;
    int depth = 0;
    gboolean in_bookmark = FALSE;
    gboolean in_applications = FALSE;
    
    while ((ret = xmlTextReaderRead(reader)) == 1 && entry_count < 1000) {
        const xmlChar *name = xmlTextReaderConstName(reader);
        const xmlChar *local_name = xmlTextReaderConstLocalName(reader);
        int node_type = xmlTextReaderNodeType(reader);
        int current_depth = xmlTextReaderDepth(reader);
        
        if (node_type == XML_READER_TYPE_ELEMENT) {
            if (xmlStrEqual(local_name, (const xmlChar*)"bookmark")) {
                in_bookmark = TRUE;
                depth = current_depth;
                
                // 获取bookmark属性
                xmlChar *href = xmlTextReaderGetAttribute(reader, (const xmlChar*)"href");
                xmlChar *modified = xmlTextReaderGetAttribute(reader, (const xmlChar*)"modified");
                xmlChar *added = xmlTextReaderGetAttribute(reader, (const xmlChar*)"added");
                
                if (href) {
                    current_href = g_strdup((const gchar*)href);
                    current_modified = modified ? g_strdup((const gchar*)modified) : NULL;
                    current_added = added ? g_strdup((const gchar*)added) : NULL;
                    current_applications = g_string_new("");
                }
                
                if (href) xmlFree(href);
                if (modified) xmlFree(modified);
                if (added) xmlFree(added);
            }
            else if (in_bookmark && xmlStrEqual(local_name, (const xmlChar*)"applications")) {
                in_applications = TRUE;
            }
            else if (in_applications && xmlStrEqual(local_name, (const xmlChar*)"application")) {
                // 获取应用程序名称
                xmlChar *app_name = xmlTextReaderGetAttribute(reader, (const xmlChar*)"name");
                if (app_name) {
                    if (current_applications->len > 0) {
                        g_string_append(current_applications, ", ");
                    }
                    g_string_append(current_applications, (const gchar*)app_name);
                    xmlFree(app_name);
                }
            }
        }
        else if (node_type == XML_READER_TYPE_END_ELEMENT) {
            if (in_bookmark && current_depth == depth && xmlStrEqual(local_name, (const xmlChar*)"bookmark")) {
                // 结束当前bookmark，创建条目
                if (current_href) {
                    HistoryEntry *entry = g_new0(HistoryEntry, 1);
                    entry->url = g_strdup(current_href);
                    
                    // 从URL中提取文件名作为标题
                    gchar *filename = get_filename_from_path((const gchar*)current_href);
                    // URL解码文件名（处理中文等）
                    gchar *decoded_filename = g_uri_unescape_string(filename, NULL);
                    entry->title = decoded_filename ? decoded_filename : g_strdup(filename);
                    g_free(filename);
                    
                    entry->timestamp = current_modified ? g_strdup(current_modified) : 
                                      (current_added ? g_strdup(current_added) : NULL);
                    entry->type = RECENTLY_USED;
                    
                    // 设置应用程序信息
                    if (current_applications && current_applications->len > 0) {
                        entry->applications = g_string_free(current_applications, FALSE);
                        current_applications = NULL;
                    } else {
                        entry->applications = g_strdup("Unknown application");
                    }
                    
                    // 构建描述信息
                    GString *desc = g_string_new("");
                    g_string_append_printf(desc, "File: %s\n", entry->title);
                    g_string_append_printf(desc, "URL: %s\n", entry->url);
                    if (entry->timestamp) {
                        g_string_append_printf(desc, "Modified: %s\n", entry->timestamp);
                    }
                    if (entry->applications) {
                        g_string_append_printf(desc, "Applications: %s", entry->applications);
                    }
                    
                    entry->description = g_string_free(desc, FALSE);
                    
                    entries = g_list_append(entries, entry);
                    entry_count++;
                    
                    // 重置当前状态
                    g_free(current_href);
                    g_free(current_modified);
                    g_free(current_added);
                    current_href = NULL;
                    current_modified = NULL;
                    current_added = NULL;
                    in_bookmark = FALSE;
                }
            }
            else if (in_applications && xmlStrEqual(local_name, (const xmlChar*)"applications")) {
                in_applications = FALSE;
            }
        }
    }
    
    // 清理可能剩余的资源
    if (current_applications) {
        g_string_free(current_applications, TRUE);
    }
    g_free(current_href);
    g_free(current_modified);
    g_free(current_added);
    
    xmlFreeTextReader(reader);
    g_free(path);
    
    if (ret != 0 && ret != 1) {
        update_status("Error reading recently-used.xbel");
    }
    
    return entries;
}

// 加载shell历史记录（Bash/Zsh）
GList* load_shell_history(const gchar *path, HistoryType type) {
    GList *entries = NULL;
    gchar *expanded_path = expand_path(path);
    
    if (!file_exists(expanded_path)) {
        g_free(expanded_path);
        update_status("Shell history file not found");
        return NULL;
    }
    
    gint64 file_size = get_file_size(expanded_path);
    if (file_size == 0) {
        g_free(expanded_path);
        update_status("Shell history file is empty");
        return NULL;
    }
    
    GError *error = NULL;
    gchar *content;
    gsize length;
    
    if (g_file_get_contents(expanded_path, &content, &length, &error)) {
        gchar **lines = g_strsplit(content, "\n", -1);
        int line_count = 0;
        
        for (int i = 0; lines[i] != NULL && lines[i][0] != '\0' && line_count < 500; i++) {
            // 跳过空行和注释
            if (g_strcmp0(lines[i], "") == 0 || lines[i][0] == '#') {
                continue;
            }
            
            HistoryEntry *entry = g_new0(HistoryEntry, 1);
            entry->description = g_strdup(lines[i]);
            entry->title = g_strdup_printf("Command %d", line_count + 1);
            entry->type = type;
            entries = g_list_append(entries, entry);
            line_count++;
        }
        
        g_strfreev(lines);
        g_free(content);
        
        if (line_count == 0) {
            update_status("No valid commands found in shell history");
        }
    } else {
        update_status("Failed to read shell history file");
        if (error) {
            g_error_free(error);
        }
    }
    
    g_free(expanded_path);
    return entries;
}

// 加载PowerShell历史记录
GList* load_powershell_history() {
    GList *entries = NULL;
    
    // 尝试多个可能的PowerShell历史记录位置
    const gchar *possible_paths[] = {
        "~/.local/share/powershell/PSReadLine/ConsoleHost_history.txt",
        "~/.config/powershell/PSReadLine/ConsoleHost_history.txt",
        NULL
    };
    
    gchar *actual_path = NULL;
    for (int i = 0; possible_paths[i] != NULL; i++) {
        gchar *path = expand_path(possible_paths[i]);
        if (file_exists(path)) {
            actual_path = path;
            break;
        }
        g_free(path);
    }
    
    if (!actual_path) {
        update_status("PowerShell history file not found");
        return NULL;
    }
    
    GError *error = NULL;
    gchar *content;
    gsize length;
    
    if (g_file_get_contents(actual_path, &content, &length, &error)) {
        gchar **lines = g_strsplit(content, "\n", -1);
        int line_count = 0;
        
        for (int i = 0; lines[i] != NULL && lines[i][0] != '\0' && line_count < 500; i++) {
            if (g_strcmp0(lines[i], "") == 0) {
                continue;
            }
            
            HistoryEntry *entry = g_new0(HistoryEntry, 1);
            entry->description = g_strdup(lines[i]);
            entry->title = g_strdup_printf("PowerShell Command %d", line_count + 1);
            entry->type = POWERSHELL_HISTORY;
            entries = g_list_append(entries, entry);
            line_count++;
        }
        
        g_strfreev(lines);
        g_free(content);
        
        if (line_count == 0) {
            update_status("PowerShell history is empty");
        }
    } else {
        update_status("Failed to read PowerShell history");
        if (error) {
            g_error_free(error);
        }
    }
    
    g_free(actual_path);
    return entries;
}

// SQLite回调函数
static int sqlite_callback(void *data, int argc, char **argv, char **col_names) {
    GList **entries = (GList**)data;
    HistoryEntry *entry = g_new0(HistoryEntry, 1);
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(col_names[i], "url") == 0 && argv[i]) {
            entry->url = g_strdup(argv[i]);
        } else if (strcmp(col_names[i], "title") == 0 && argv[i]) {
            entry->title = g_strdup(argv[i]);
        } else if ((strcmp(col_names[i], "last_visit_time") == 0 || 
                   strcmp(col_names[i], "visit_time") == 0) && argv[i]) {
            // 转换Chrome/Firefox时间戳为可读格式
            gint64 timestamp = g_ascii_strtoll(argv[i], NULL, 10);
            if (timestamp > 10000000000000000LL) {
                // Chrome时间戳（微秒 since 1601）
                timestamp = timestamp / 1000000 - 11644473600LL;
            } else if (timestamp > 1000000000000LL) {
                // Firefox时间戳（微秒）
                timestamp = timestamp / 1000000;
            }
            
            GDateTime *dt = g_date_time_new_from_unix_utc(timestamp);
            if (dt) {
                gchar *time_str = g_date_time_format(dt, "%Y-%m-%d %H:%M:%S");
                entry->timestamp = time_str;
                g_date_time_unref(dt);
            }
        }
    }
    
    if (entry->title || entry->url) {
        if (!entry->title) entry->title = g_strdup("Untitled");
        if (!entry->url) entry->url = g_strdup("Unknown URL");
        
        if (entry->timestamp) {
            entry->description = g_strdup_printf("%s\n%s\nVisited: %s", 
                                               entry->title, entry->url, entry->timestamp);
        } else {
            entry->description = g_strdup_printf("%s\n%s", entry->title, entry->url);
        }
        
        *entries = g_list_append(*entries, entry);
    } else {
        g_free(entry);
    }
    
    return 0;
}

// 加载浏览器历史记录
GList* load_browser_history(HistoryType type) {
    GList *entries = NULL;
    gchar *db_path = NULL;
    const gchar *query = NULL;
    
    // 根据浏览器类型设置路径和查询
    switch (type) {
        case FIREFOX_HISTORY:
            db_path = find_file_by_pattern(history_files[FIREFOX_HISTORY]);
            query = "SELECT url, title, last_visit_time FROM moz_places WHERE url NOT LIKE 'place:%' ORDER BY last_visit_time DESC LIMIT 500";
            break;
        case CHROME_HISTORY:
            // 尝试多个Chrome系浏览器路径
            const gchar *chrome_paths[] = {
                "~/.config/google-chrome/Default/History",
                "~/.config/chromium/Default/History", 
                "~/.config/microsoft-edge/Default/History",
                NULL
            };
            
            for (int i = 0; chrome_paths[i] != NULL; i++) {
                gchar *path = expand_path(chrome_paths[i]);
                if (file_exists(path)) {
                    db_path = path;
                    break;
                }
                g_free(path);
            }
            
            query = "SELECT url, title, last_visit_time FROM urls ORDER BY last_visit_time DESC LIMIT 500";
            break;
        default:
            return NULL;
    }
    
    if (!db_path) {
        update_status("Browser history database not found");
        return NULL;
    }
    
    sqlite3 *db;
    int rc = sqlite3_open(db_path, &db);
    
    if (rc != SQLITE_OK) {
        update_status("Failed to open browser history database");
        g_free(db_path);
        return NULL;
    }
    
    // 执行查询
    char *err_msg = NULL;
    rc = sqlite3_exec(db, query, sqlite_callback, &entries, &err_msg);
    
    if (rc != SQLITE_OK) {
        update_status("Failed to query browser history");
        if (err_msg) {
            sqlite3_free(err_msg);
        }
    }
    
    sqlite3_close(db);
    g_free(db_path);
    
    // 设置条目类型
    GList *iter = entries;
    while (iter) {
        HistoryEntry *entry = (HistoryEntry*)iter->data;
        entry->type = type;
        iter = g_list_next(iter);
    }
    
    return entries;
}

// 加载历史记录
void load_history(GtkWidget *widget, gpointer user_data) {
    // 获取当前选择的类型
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo));
    HistoryType type = (HistoryType)active;
    
    // 清空当前条目
    if (current_entries) {
        GList *iter = current_entries;
        while (iter) {
            HistoryEntry *entry = (HistoryEntry*)iter->data;
            g_free(entry->title);
            g_free(entry->url);
            g_free(entry->timestamp);
            g_free(entry->description);
            g_free(entry->applications);
            iter = g_list_next(iter);
        }
        g_list_free(current_entries);
        current_entries = NULL;
    }
    
    // 清空列表
    gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(history_list))));
    
    // 清空内容视图
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(content_view)), "", -1);
    
    update_status("Loading history...");
    
    switch (type) {
        case RECENTLY_USED:
            current_entries = load_recently_used();
            break;
        case BASH_HISTORY:
            current_entries = load_shell_history(history_files[BASH_HISTORY], BASH_HISTORY);
            break;
        case ZSH_HISTORY:
            current_entries = load_shell_history(history_files[ZSH_HISTORY], ZSH_HISTORY);
            break;
        case POWERSHELL_HISTORY:
            current_entries = load_powershell_history();
            break;
        case FIREFOX_HISTORY:
            current_entries = load_browser_history(FIREFOX_HISTORY);
            break;
        case CHROME_HISTORY:
            current_entries = load_browser_history(CHROME_HISTORY);
            break;
        default:
            update_status("History type not implemented yet");
            return;
    }
    
    // 填充列表
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(history_list)));
    GList *iter = current_entries;
    int count = 0;
    
    while (iter) {
        HistoryEntry *entry = (HistoryEntry*)iter->data;
        GtkTreeIter tree_iter;
        
        gtk_list_store_append(store, &tree_iter);
        gtk_list_store_set(store, &tree_iter,
                          0, entry->title ? entry->title : "No Title",
                          1, entry->description ? entry->description : "No Description",
                          -1);
        
        iter = g_list_next(iter);
        count++;
    }
    
    if (count > 0) {
        update_status(g_strdup_printf("Loaded %d entries", count));
    } else {
        update_status("No entries found");
    }
}

// 列表选择变化回调
void on_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *description;
        gtk_tree_model_get(model, &iter, 1, &description, -1);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(content_view)), 
                                description ? description : "No details available", -1);
        g_free(description);
    }
}

// 删除选中的条目
void delete_selected(GtkWidget *widget, gpointer user_data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(history_list));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                                  GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_QUESTION,
                                                  GTK_BUTTONS_YES_NO,
                                                  "Are you sure you want to delete the selected entry?");
        
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (result == GTK_RESPONSE_YES) {
            // 获取选中的行号
            GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
            gint *indices = gtk_tree_path_get_indices(path);
            gint row = indices[0];
            gtk_tree_path_free(path);
            
            // 从列表中删除
            gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
            
            // 从内存中删除（这里简化处理，实际应该根据类型删除源文件中的条目）
            update_status("Delete function - Entry removed from view (source file not modified)");
        }
    } else {
        update_status("Please select an entry to delete");
    }
}

// 清除所有历史记录
void clear_all_history(GtkWidget *widget, gpointer user_data) {
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo));
    HistoryType type = (HistoryType)active;
    
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_WARNING,
                                              GTK_BUTTONS_YES_NO,
                                              "Are you sure you want to clear all history? This action cannot be undone.");
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (result == GTK_RESPONSE_YES) {
        gboolean success = FALSE;
        
        switch (type) {
            case RECENTLY_USED:
                {
                    gchar *path = expand_path(history_files[RECENTLY_USED]);
                    if (file_exists(path)) {
                        if (remove(path) == 0) {
                            success = TRUE;
                            update_status("Recently used files history cleared successfully");
                        } else {
                            update_status("Failed to clear recently used files history");
                        }
                    } else {
                        update_status("Recently used files history not found");
                    }
                    g_free(path);
                }
                break;
            case BASH_HISTORY:
            case ZSH_HISTORY:
                {
                    gchar *path = expand_path(history_files[type]);
                    if (file_exists(path)) {
                        if (remove(path) == 0) {
                            success = TRUE;
                        } else {
                            update_status("Failed to clear shell history");
                        }
                    } else {
                        update_status("Shell history file not found");
                    }
                    g_free(path);
                }
                break;
            case POWERSHELL_HISTORY:
                {
                    GList *powershell_entries = load_powershell_history();
                    if (powershell_entries) {
                        gchar *path = NULL;
                        const gchar *possible_paths[] = {
                            "~/.local/share/powershell/PSReadLine/ConsoleHost_history.txt",
                            "~/.config/powershell/PSReadLine/ConsoleHost_history.txt",
                            NULL
                        };
                        
                        for (int i = 0; possible_paths[i] != NULL; i++) {
                            gchar *test_path = expand_path(possible_paths[i]);
                            if (file_exists(test_path)) {
                                path = test_path;
                                break;
                            }
                            g_free(test_path);
                        }
                        
                        if (path && remove(path) == 0) {
                            success = TRUE;
                        } else {
                            update_status("Failed to clear PowerShell history");
                        }
                        g_free(path);
                        
                        // 清理内存中的条目
                        GList *iter = powershell_entries;
                        while (iter) {
                            HistoryEntry *entry = (HistoryEntry*)iter->data;
                            g_free(entry->title);
                            g_free(entry->description);
                            iter = g_list_next(iter);
                        }
                        g_list_free(powershell_entries);
                    } else {
                        update_status("PowerShell history not found");
                    }
                }
                break;
            default:
                update_status("Clear function not implemented for this history type");
                return;
        }
        
        if (success) {
            // 重新加载空列表
            load_history(NULL, NULL);
        }
    }
}

// 创建主界面
GtkWidget* create_main_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME " - History Cleaner");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // 历史类型选择
    type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Recently Used Files");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Bash History");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Zsh History");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "PowerShell History");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Firefox History");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Chrome/Edge History");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Other History");
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combo), 0);
    
    g_signal_connect(type_combo, "changed", G_CALLBACK(load_history), NULL);
    
    // 按钮栏
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *load_btn = gtk_button_new_with_label("Load");
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete Selected");
    GtkWidget *clear_btn = gtk_button_new_with_label("Clear All");
    
    g_signal_connect(load_btn, "clicked", G_CALLBACK(load_history), NULL);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(delete_selected), NULL);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(clear_all_history), NULL);
    
    gtk_box_pack_start(GTK_BOX(button_box), load_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), delete_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), clear_btn, FALSE, FALSE, 0);
    
    // 主内容区域
    GtkWidget *hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    
    // 历史记录列表
    GtkWidget *scrolled_list = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_list),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    history_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column1 = gtk_tree_view_column_new_with_attributes("Title", renderer, "text", 0, NULL);
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", 1, NULL);
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(history_list), column1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(history_list), column2);
    
    // 设置列宽
    gtk_tree_view_column_set_min_width(column1, 150);
    gtk_tree_view_column_set_min_width(column2, 400);
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(history_list));
    g_signal_connect(selection, "changed", G_CALLBACK(on_selection_changed), NULL);
    
    gtk_container_add(GTK_CONTAINER(scrolled_list), history_list);
    
    // 内容查看区域
    GtkWidget *scrolled_content = gtk_scrolled_window_new(NULL, NULL);
    content_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(content_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(content_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled_content), content_view);
    
    gtk_paned_add1(GTK_PANED(hpaned), scrolled_list);
    gtk_paned_add2(GTK_PANED(hpaned), scrolled_content);
    gtk_paned_set_position(GTK_PANED(hpaned), 400);
    
    // 状态栏
    status_label = gtk_label_new("Welcome to Anāsrava - No Leakage History Cleaner");
    
    // 组装界面
    gtk_box_pack_start(GTK_BOX(vbox), type_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);
    
    return window;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    main_window = create_main_window();
    
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_show_all(main_window);
    
    // 初始加载最近使用文件
    load_history(NULL, NULL);
    
    gtk_main();
    
    return 0;
}

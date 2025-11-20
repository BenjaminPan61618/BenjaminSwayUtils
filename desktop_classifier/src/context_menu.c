#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <gio/gdesktopappinfo.h>
#include "context_menu.h"

// 前向声明
static void on_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data);

// 创建上下文菜单
GtkWidget* create_category_context_menu(const gchar *category_name) {
    GtkWidget *menu;
    GtkWidget *menu_item;
    GtkWidget *separator;
    
    menu = gtk_menu_new();
    
    // 添加"打开"选项
    menu_item = gtk_menu_item_new_with_label("打开");
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_menu_item_activate), g_strdup_printf("%s_open", category_name));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    
    // 添加"属性"选项
    menu_item = gtk_menu_item_new_with_label("属性");
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_menu_item_activate), g_strdup_printf("%s_properties", category_name));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    
    // 添加分隔符
    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    
    // 添加"刷新"选项
    menu_item = gtk_menu_item_new_with_label("刷新");
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_menu_item_activate), g_strdup_printf("%s_refresh", category_name));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    
    // 添加"排序方式"子菜单
    GtkWidget *sort_menu_item = gtk_menu_item_new_with_label("排序方式");
    GtkWidget *sort_menu = gtk_menu_new();
    
    GtkWidget *sort_name_item = gtk_menu_item_new_with_label("按名称");
    g_signal_connect(sort_name_item, "activate", G_CALLBACK(on_menu_item_activate), g_strdup_printf("%s_sort_name", category_name));
    gtk_menu_shell_append(GTK_MENU_SHELL(sort_menu), sort_name_item);
    
    GtkWidget *sort_date_item = gtk_menu_item_new_with_label("按日期");
    g_signal_connect(sort_date_item, "activate", G_CALLBACK(on_menu_item_activate), g_strdup_printf("%s_sort_date", category_name));
    gtk_menu_shell_append(GTK_MENU_SHELL(sort_menu), sort_date_item);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(sort_menu_item), sort_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sort_menu_item);
    
    // 添加分隔符
    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    
    // 添加"关闭"选项
    menu_item = gtk_menu_item_new_with_label("关闭");
    g_signal_connect(menu_item, "activate", G_CALLBACK(on_menu_item_activate), g_strdup_printf("%s_close", category_name));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    
    gtk_widget_show_all(menu);
    return menu;
}

// 菜单项激活回调函数
static void on_menu_item_activate(GtkMenuItem *menuitem, gpointer user_data) {
    gchar *action = (gchar*)user_data;
    g_print("执行操作: %s\n", action);
    
    // 根据不同的操作执行相应逻辑
    if (g_str_has_suffix(action, "_open")) {
        g_print("打开分类: %s\n", action);
    } else if (g_str_has_suffix(action, "_properties")) {
        g_print("显示分类属性: %s\n", action);
    } else if (g_str_has_suffix(action, "_refresh")) {
        g_print("刷新分类内容: %s\n", action);
    } else if (g_str_has_suffix(action, "_sort_name")) {
        g_print("按名称排序: %s\n", action);
    } else if (g_str_has_suffix(action, "_sort_date")) {
        g_print("按日期排序: %s\n", action);
    } else if (g_str_has_suffix(action, "_close")) {
        g_print("关闭分类: %s\n", action);
    }
    
    g_free(action);
}

// 显示上下文菜单
void show_category_context_menu(GtkWidget *button, const gchar *category_name) {
    GtkWidget *menu = create_category_context_menu(category_name);
    
    // 在按钮位置显示菜单
    gtk_menu_popup_at_widget(GTK_MENU(menu), 
                            button,
                            GDK_GRAVITY_SOUTH_WEST,
                            GDK_GRAVITY_NORTH_WEST,
                            NULL);
}

// 创建桌面右键菜单
GtkWidget* create_desktop_context_menu(gpointer user_data) {
    GtkWidget *menu = gtk_menu_new();
    
    // 刷新
    GtkWidget *refresh_item = gtk_menu_item_new_with_label("刷新");
    g_signal_connect(refresh_item, "activate", G_CALLBACK(on_refresh), NULL);
    
    // 排序方式子菜单
    GtkWidget *sort_item = gtk_menu_item_new_with_label("排序方式");
    GtkWidget *sort_menu = gtk_menu_new();
    GtkWidget *sort_name = gtk_menu_item_new_with_label("按名称");
    GtkWidget *sort_size = gtk_menu_item_new_with_label("按大小");
    GtkWidget *sort_date = gtk_menu_item_new_with_label("按日期");
    
    // 查看选项子菜单
    GtkWidget *view_item = gtk_menu_item_new_with_label("查看");
    GtkWidget *view_menu = gtk_menu_new();
    GtkWidget *view_large = gtk_menu_item_new_with_label("大图标");
    GtkWidget *view_small = gtk_menu_item_new_with_label("小图标");
    GtkWidget *toggle_hidden = gtk_check_menu_item_new_with_label("显示隐藏文件");
    GtkWidget *toggle_extensions = gtk_check_menu_item_new_with_label("显示扩展名");
    
    // 粘贴（如果剪贴板有内容）
    GtkWidget *paste_item = gtk_menu_item_new_with_label("粘贴");
    g_signal_connect(paste_item, "activate", G_CALLBACK(on_paste), NULL);
    
    // 新建
    GtkWidget *new_item = gtk_menu_item_new_with_label("新建");
    GtkWidget *new_menu = gtk_menu_new();
    GtkWidget *new_folder = gtk_menu_item_new_with_label("文件夹");
    GtkWidget *new_file = gtk_menu_item_new_with_label("文本文档");
    
    // 使用文件管理器打开
    GtkWidget *open_desktop_item = gtk_menu_item_new_with_label("在文件管理器中打开桌面");
    g_signal_connect(open_desktop_item, "activate", G_CALLBACK(on_open_desktop_in_file_manager), NULL);
    // 退出（作为没有托盘图标时的兜底）
    GtkWidget *exit_item = gtk_menu_item_new_with_label("退出");
    g_signal_connect(exit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    
    // 组装菜单
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), refresh_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), sort_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), view_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), paste_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), new_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_desktop_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), exit_item);
    
    // 设置子菜单
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(sort_item), sort_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(sort_menu), sort_name);
    gtk_menu_shell_append(GTK_MENU_SHELL(sort_menu), sort_size);
    gtk_menu_shell_append(GTK_MENU_SHELL(sort_menu), sort_date);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_item), view_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_large);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_small);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), toggle_hidden);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), toggle_extensions);
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_item), new_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(new_menu), new_folder);
    gtk_menu_shell_append(GTK_MENU_SHELL(new_menu), new_file);
    
    return menu;
}

// 文件右键菜单
GtkWidget* create_file_context_menu(DesktopFile *file) {
    GtkWidget *menu = gtk_menu_new();
    
    // 打开/打开方式
    GtkWidget *open_item = gtk_menu_item_new_with_label("打开");
    GtkWidget *open_with_item = gtk_menu_item_new_with_label("打开方式");
    GtkWidget *open_with_menu = gtk_menu_new();
    GtkWidget *open_in_folder_item = gtk_menu_item_new_with_label("在文件夹中显示");
    
    // 符号链接特殊功能
    if (file->is_symlink) {
        GtkWidget *open_target_item = gtk_menu_item_new_with_label("打开指向的路径");
        g_signal_connect(open_target_item, "activate", G_CALLBACK(on_open_symlink_target), file);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_target_item);
    }
    
    // 文件操作
    GtkWidget *rename_item = gtk_menu_item_new_with_label("重命名");
    GtkWidget *delete_item = gtk_menu_item_new_with_label("删除");
    GtkWidget *copy_item = gtk_menu_item_new_with_label("复制");
    GtkWidget *cut_item = gtk_menu_item_new_with_label("剪切");
    
    // 属性
    GtkWidget *properties_item = gtk_menu_item_new_with_label("属性");
    
    // 组装菜单
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_with_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_in_folder_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), rename_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delete_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), copy_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), cut_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), properties_item);
    
    // 连接信号
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open_file), file);
    g_signal_connect(open_in_folder_item, "activate", G_CALLBACK(on_open_containing_folder), file);
    g_signal_connect(rename_item, "activate", G_CALLBACK(on_rename), file);
    g_signal_connect(delete_item, "activate", G_CALLBACK(on_delete), file);
    g_signal_connect(copy_item, "activate", G_CALLBACK(on_copy), file);
    g_signal_connect(cut_item, "activate", G_CALLBACK(on_cut), file);
    g_signal_connect(properties_item, "activate", G_CALLBACK(on_properties), file);
    
    // Open With submenu entries from available apps for the mime type
    const gchar *content_type = NULL;
    GFile *gfile = g_file_new_for_path(file->filepath);
    GFileInfo *info = g_file_query_info(gfile, "standard::content-type", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (info) {
        content_type = g_file_info_get_content_type(info);
    }
    if (content_type) {
        GList *apps = g_app_info_get_all_for_type(content_type);
        for (GList *a = apps; a; a = a->next) {
            GAppInfo *app = G_APP_INFO(a->data);
            const gchar *name = g_app_info_get_display_name(app);
            GtkWidget *mi = gtk_menu_item_new_with_label(name ? name : "应用程序");
            g_object_set_data_full(G_OBJECT(mi), "app-info", g_object_ref(app), g_object_unref);
            g_signal_connect(mi, "activate", G_CALLBACK(on_open_with), file);
            gtk_menu_shell_append(GTK_MENU_SHELL(open_with_menu), mi);
        }
        g_list_free_full(apps, g_object_unref);
    }
    if (info) g_object_unref(info);
    g_object_unref(gfile);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(open_with_item), open_with_menu);
    return menu;
}

// Simple internal clipboard for copy/cut between menus
static gchar *clipboard_path = NULL;
static gboolean clipboard_is_cut = FALSE;

// Declared in main.c
extern void update_file_classification();

static const gchar* get_desktop_dir() {
    const gchar *path = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    return path ? path : g_get_home_dir();
}

void on_refresh(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    update_file_classification();
}

void on_paste(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    if (!clipboard_path) return;
    const gchar *desktop_dir = get_desktop_dir();
    gchar *basename = g_path_get_basename(clipboard_path);
    gchar *dest_path = g_build_filename(desktop_dir, basename, NULL);
    GFile *src = g_file_new_for_path(clipboard_path);
    GFile *dst = g_file_new_for_path(dest_path);

    GError *error = NULL;
    if (clipboard_is_cut) {
        if (!g_file_move(src, dst, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
            if (error) { g_warning("Move failed: %s", error->message); g_error_free(error); }
        }
    } else {
        if (!g_file_copy(src, dst, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error)) {
            if (error) { g_warning("Copy failed: %s", error->message); g_error_free(error); }
        }
    }
    g_object_unref(src);
    g_object_unref(dst);
    g_free(basename);
    g_free(dest_path);

    if (clipboard_is_cut) {
        g_clear_pointer(&clipboard_path, g_free);
        clipboard_is_cut = FALSE;
    }
    update_file_classification();
}

void on_open_desktop_in_file_manager(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    const gchar *desktop_dir = get_desktop_dir();
    gchar *uri = g_filename_to_uri(desktop_dir, NULL, NULL);
    GError *error = NULL;
    gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
    if (error) { g_warning("Open desktop failed: %s", error->message); g_error_free(error); }
    g_free(uri);
}

void on_open_symlink_target(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->target_path) return;
    gchar *uri = g_filename_to_uri(file->target_path, NULL, NULL);
    GError *error = NULL;
    gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
    if (error) { g_warning("Open target failed: %s", error->message); g_error_free(error); }
    g_free(uri);
}

void on_open_file(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->filepath) return;
    if (g_str_has_suffix(file->filename, ".desktop")) {
        GDesktopAppInfo *app = g_desktop_app_info_new_from_filename(file->filepath);
        if (app) {
            GError *error = NULL;
            if (!g_app_info_launch(G_APP_INFO(app), NULL, NULL, &error)) {
                if (error) { g_warning("Launch desktop file failed: %s", error->message); g_error_free(error); }
            }
            g_object_unref(app);
            return;
        }
    }
    gchar *uri = g_filename_to_uri(file->filepath, NULL, NULL);
    GError *error = NULL;
    gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
    if (error) { g_warning("Open file failed: %s", error->message); g_error_free(error); }
    g_free(uri);
}
static gchar* resolve_folder_for_item(DesktopFile *file) {
    if (!file || !file->filepath) return NULL;
    // 1) Symlink: open the directory of its target
    if (file->is_symlink && file->target_path && *file->target_path) {
        if (g_file_test(file->target_path, G_FILE_TEST_IS_DIR)) {
            return g_strdup(file->target_path);
        }
        return g_path_get_dirname(file->target_path);
    }
    // 2) .desktop with Path field
    if (g_str_has_suffix(file->filename, ".desktop")) {
        GDesktopAppInfo *app = g_desktop_app_info_new_from_filename(file->filepath);
        if (app) {
            const gchar *desktop_path = g_desktop_app_info_get_filename(app);
            if (desktop_path) {
                GKeyFile *kf = g_key_file_new();
                GError *err = NULL;
                if (g_key_file_load_from_file(kf, desktop_path, G_KEY_FILE_NONE, &err)) {
                    gchar *workdir = g_key_file_get_string(kf, "Desktop Entry", "Path", NULL);
                    if (workdir && *workdir) {
                        // If relative, make it relative to the .desktop file directory
                        if (!g_path_is_absolute(workdir)) {
                            gchar *base = g_path_get_dirname(desktop_path);
                            gchar *abs = g_build_filename(base, workdir, NULL);
                            g_free(base);
                            g_free(workdir);
                            workdir = abs;
                        }
                        g_key_file_unref(kf);
                        g_object_unref(app);
                        return workdir; // directory the app declares
                    }
                }
                if (err) g_error_free(err);
                g_key_file_unref(kf);
            }
            g_object_unref(app);
        }
        // Fallback: open the directory containing the .desktop file itself
        return g_path_get_dirname(file->filepath);
    }
    // 3) Regular file: open its directory
    return g_path_get_dirname(file->filepath);
}

void on_open_containing_folder(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    gchar *dir = resolve_folder_for_item(file);
    if (!dir) return;
    gchar *uri = g_filename_to_uri(dir, NULL, NULL);
    GError *error = NULL;
    gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
    if (error) { g_warning("Open containing folder failed: %s", error->message); g_error_free(error); }
    g_free(uri);
    g_free(dir);
}

static void show_message(GtkWindow *parent, GtkMessageType type, const gchar *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    gchar *msg = g_strdup_vprintf(fmt, ap);
    va_end(ap);
    GtkWidget *dlg = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    g_free(msg);
}

void on_rename(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->filepath) return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("重命名", NULL, GTK_DIALOG_MODAL,
                                                    "取消", GTK_RESPONSE_CANCEL,
                                                    "确定", GTK_RESPONSE_OK, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), file->filename ? file->filename : "");
    gtk_box_pack_start(GTK_BOX(content), entry, TRUE, TRUE, 8);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *new_name = gtk_entry_get_text(GTK_ENTRY(entry));
        if (!new_name || !*new_name) {
            show_message(NULL, GTK_MESSAGE_WARNING, "文件名不能为空");
            gtk_widget_destroy(dialog);
            return;
        }
        if (g_strrstr(new_name, "/")) {
            show_message(NULL, GTK_MESSAGE_WARNING, "文件名不能包含斜杠");
            gtk_widget_destroy(dialog);
            return;
        }

        gchar *dir = g_path_get_dirname(file->filepath);
        gchar *dest_path = g_build_filename(dir, new_name, NULL);
        if (g_file_test(dest_path, G_FILE_TEST_EXISTS)) {
            show_message(NULL, GTK_MESSAGE_WARNING, "目标已存在: %s", dest_path);
            g_free(dir); g_free(dest_path);
            gtk_widget_destroy(dialog);
            return;
        }

        GFile *src = g_file_new_for_path(file->filepath);
        GFile *dst = g_file_new_for_path(dest_path);
        GError *error = NULL;
        if (!g_file_move(src, dst, G_FILE_COPY_NONE, NULL, NULL, NULL, &error)) {
            if (error) { show_message(NULL, GTK_MESSAGE_ERROR, "重命名失败: %s", error->message); g_error_free(error); }
        } else {
            update_file_classification();
        }
        g_object_unref(src);
        g_object_unref(dst);
        g_free(dir); g_free(dest_path);
    }
    gtk_widget_destroy(dialog);
}

void on_delete(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->filepath) return;
    GFile *gfile = g_file_new_for_path(file->filepath);
    GError *error = NULL;
    if (!g_file_trash(gfile, NULL, &error)) {
        if (error) { g_warning("Trash failed: %s", error->message); g_error_free(error); }
    } else {
        update_file_classification();
    }
    g_object_unref(gfile);
}

void on_copy(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->filepath) return;
    g_free(clipboard_path);
    clipboard_path = g_strdup(file->filepath);
    clipboard_is_cut = FALSE;
}

void on_cut(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->filepath) return;
    g_free(clipboard_path);
    clipboard_path = g_strdup(file->filepath);
    clipboard_is_cut = TRUE;
}

void on_properties(GtkMenuItem *item, gpointer data) {
    (void)item;
    DesktopFile *file = (DesktopFile*)data;
    if (!file) return;
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "名称: %s\n路径: %s\n分类: %d\n隐藏: %s\n符号链接: %s",
        file->filename ? file->filename : "",
        file->filepath ? file->filepath : "",
        file->category,
        file->is_hidden ? "是" : "否",
        file->is_symlink ? "是" : "否");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void on_open_with(GtkMenuItem *item, gpointer data) {
    DesktopFile *file = (DesktopFile*)data;
    if (!file || !file->filepath) return;
    GAppInfo *app = G_APP_INFO(g_object_get_data(G_OBJECT(item), "app-info"));
    if (!app) return;
    GError *error = NULL;
    GFile *gfile = g_file_new_for_path(file->filepath);
    GList *files = NULL; files = g_list_prepend(files, gfile);
    if (!g_app_info_launch(app, files, NULL, &error)) {
        if (error) { g_warning("Open with failed: %s", error->message); g_error_free(error); }
    }
    g_list_free(files);
    g_object_unref(gfile);
}

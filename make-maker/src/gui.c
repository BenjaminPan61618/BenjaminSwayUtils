#include "gui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glib.h>
#include <glib/gstdio.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR "\\"
#else
#include <unistd.h>
#define PATH_SEPARATOR "/"
#endif

// 常见的库到包的映射（保持不变）
typedef struct {
    const char *library;
    const char *ubuntu_pkg;
    const char *fedora_pkg;
    const char *arch_pkg;
    const char *macos_brew;
} LibPackageMap;

static LibPackageMap package_map[] = {
    {"gtk/gtk.h", "libgtk-3-dev", "gtk3-devel", "gtk3", "gtk+3"},
    {"glib.h", "libglib2.0-dev", "glib2-devel", "glib2", "glib"},
    {"gdk/gdk.h", "libgtk-3-dev", "gtk3-devel", "gtk3", "gtk+3"},
    {"pango/pango.h", "libpango1.0-dev", "pango-devel", "pango", "pango"},
    {"cairo.h", "libcairo2-dev", "cairo-devel", "cairo", "cairo"},
    {"SDL.h", "libsdl2-dev", "SDL2-devel", "sdl2", "sdl2"},
    {"SDL2/SDL.h", "libsdl2-dev", "SDL2-devel", "sdl2", "sdl2"},
    {"openssl/ssl.h", "libssl-dev", "openssl-devel", "openssl", "openssl"},
    {"zlib.h", "zlib1g-dev", "zlib-devel", "zlib", "zlib"},
    {"jpeglib.h", "libjpeg-dev", "libjpeg-turbo-devel", "libjpeg-turbo", "jpeg"},
    {"png.h", "libpng-dev", "libpng-devel", "libpng", "libpng"},
    {"mysql.h", "libmysqlclient-dev", "mysql-devel", "mysql", "mysql"},
    {"pq-fe.h", "libpq-dev", "postgresql-devel", "postgresql-libs", "postgresql"},
    {"sqlite3.h", "libsqlite3-dev", "sqlite-devel", "sqlite", "sqlite"},
    {"curl/curl.h", "libcurl4-openssl-dev", "libcurl-devel", "curl", "curl"},
    {"pthread.h", "", "", "", ""},
    {"stdio.h", "", "", "", ""},
    {NULL, NULL, NULL, NULL, NULL}
};

void update_status(AppWidgets *widgets, const char *message) {
    gtk_label_set_text(GTK_LABEL(widgets->status_label), message);
}

void append_to_deps_view(AppWidgets *widgets, const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(widgets->deps_buffer, &end);
    gtk_text_buffer_insert(widgets->deps_buffer, &end, text, -1);
}

char* detect_platform() {
#ifdef _WIN32
    return "Windows";
#elif __APPLE__
    return "macOS";
#elif __linux__
    FILE *fp = fopen("/etc/os-release", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "ID=")) {
                fclose(fp);
                if (strstr(line, "ubuntu")) return "Ubuntu";
                if (strstr(line, "debian")) return "Debian";
                if (strstr(line, "fedora")) return "Fedora";
                if (strstr(line, "centos")) return "CentOS";
                if (strstr(line, "arch")) return "Arch";
                return "Linux";
            }
        }
        fclose(fp);
    }
    return "Linux";
#else
    return "Unknown";
#endif
}

char* detect_system_packages(const char *library) {
    for (int i = 0; package_map[i].library != NULL; i++) {
        if (strstr(library, package_map[i].library)) {
            const char *platform = detect_platform();
            
            if (strcmp(platform, "Ubuntu") == 0 || strcmp(platform, "Debian") == 0) {
                return g_strdup(package_map[i].ubuntu_pkg);
            } else if (strcmp(platform, "Fedora") == 0 || strcmp(platform, "CentOS") == 0) {
                return g_strdup(package_map[i].fedora_pkg);
            } else if (strcmp(platform, "Arch") == 0) {
                return g_strdup(package_map[i].arch_pkg);
            } else if (strcmp(platform, "macOS") == 0) {
                return g_strdup(package_map[i].macos_brew);
            }
        }
    }
    return NULL;
}

gboolean file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

gboolean is_source_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return FALSE;
    return (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || 
            strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0);
}

gboolean is_header_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return FALSE;
    return (strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0 || 
            strcmp(ext, ".hh") == 0 || strcmp(ext, ".hxx") == 0);
}

gboolean analyze_source_dependencies(const char *filename, GString *deps) {
    if (!file_exists(filename)) {
        return FALSE;
    }
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        return FALSE;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char *include_pos = strstr(line, "#include");
        if (include_pos) {
            char *start = strchr(include_pos, '"');
            if (!start) start = strchr(include_pos, '<');
            if (start) {
                char *end = strchr(start + 1, start[0] == '"' ? '"' : '>');
                if (end) {
                    char header[256];
                    int len = end - start - 1;
                    if (len > 0 && len < sizeof(header) - 1) {
                        strncpy(header, start + 1, len);
                        header[len] = '\0';
                        
                        if (start[0] == '<') {
                            char *pkg = detect_system_packages(header);
                            if (pkg && strlen(pkg) > 0) {
                                g_string_append_printf(deps, "头文件: %s -> 需要安装: %s\n", header, pkg);
                                g_free(pkg);
                            } else {
                                g_string_append_printf(deps, "头文件: %s (系统库)\n", header);
                            }
                        } else {
                            g_string_append_printf(deps, "头文件: %s (本地文件)\n", header);
                        }
                    }
                }
            }
        }
    }
    
    fclose(file);
    return TRUE;
}

void scan_directory_recursive(const char *base_path, const char *path, GtkListStore *store, int depth) {
    char full_path[1024];
    if (strcmp(path, "") == 0) {
        snprintf(full_path, sizeof(full_path), "%s", base_path);
    } else {
        snprintf(full_path, sizeof(full_path), "%s%s%s", base_path, PATH_SEPARATOR, path);
    }
    
    DIR *dir = opendir(full_path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        char entry_path[1024];
        if (strcmp(path, "") == 0) {
            snprintf(entry_path, sizeof(entry_path), "%s%s%s", base_path, PATH_SEPARATOR, entry->d_name);
        } else {
            snprintf(entry_path, sizeof(entry_path), "%s%s%s%s%s", base_path, PATH_SEPARATOR, path, PATH_SEPARATOR, entry->d_name);
        }
        
        struct stat st;
        if (stat(entry_path, &st) == 0) {
            char display_path[1024];
            if (strcmp(path, "") == 0) {
                snprintf(display_path, sizeof(display_path), "%s", entry->d_name);
            } else {
                snprintf(display_path, sizeof(display_path), "%s%s%s", path, PATH_SEPARATOR, entry->d_name);
            }
            
            // 添加到树视图
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            
            const char *icon = "folder";
            if (S_ISREG(st.st_mode)) {
                if (is_source_file(entry->d_name)) {
                    icon = "source";
                } else if (is_header_file(entry->d_name)) {
                    icon = "header";
                } else if (strcmp(entry->d_name, "Makefile") == 0 || strcmp(entry->d_name, "makefile") == 0) {
                    icon = "makefile";
                } else {
                    icon = "file";
                }
            }
            
            gtk_list_store_set(store, &iter,
                              0, display_path,
                              1, icon,
                              -1);
            
            if (S_ISDIR(st.st_mode) && depth < 5) { // 限制递归深度
                char new_path[1024];
                if (strcmp(path, "") == 0) {
                    snprintf(new_path, sizeof(new_path), "%s", entry->d_name);
                } else {
                    snprintf(new_path, sizeof(new_path), "%s%s%s", path, PATH_SEPARATOR, entry->d_name);
                }
                scan_directory_recursive(base_path, new_path, store, depth + 1);
            }
        }
    }
    
    closedir(dir);
}

void scan_project_directory(AppWidgets *widgets, const char *path) {
    if (!path) return;
    
    // 清空现有项目文件
    free_project_files(widgets);
    
    // 更新项目路径
    if (widgets->project_path) {
        g_free(widgets->project_path);
    }
    widgets->project_path = g_strdup(path);
    
    // 清空树视图
    gtk_list_store_clear(widgets->project_store);
    
    // 扫描目录
    scan_directory_recursive(path, "", widgets->project_store, 0);
    
    // 扫描源文件
    GQueue *source_queue = g_queue_new();
    
    DIR *dir = opendir(path);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (is_source_file(entry->d_name)) {
                char *full_path = g_build_filename(path, entry->d_name, NULL);
                g_queue_push_tail(source_queue, full_path);
            }
        }
        closedir(dir);
    }
    
    // 更新源文件列表
    widgets->source_count = g_queue_get_length(source_queue);
    if (widgets->source_count > 0) {
        widgets->source_files = g_malloc(sizeof(char*) * (widgets->source_count + 1));
        int i = 0;
        while (!g_queue_is_empty(source_queue)) {
            widgets->source_files[i++] = g_queue_pop_head(source_queue);
        }
        widgets->source_files[i] = NULL;
    }
    
    g_queue_free(source_queue);
    
    update_source_files_list(widgets);
}

void update_source_files_list(AppWidgets *widgets) {
    GString *files_list = g_string_new("");
    
    for (int i = 0; i < widgets->source_count; i++) {
        if (i > 0) g_string_append(files_list, " ");
        // 只显示文件名，不显示完整路径
        const char *filename = strrchr(widgets->source_files[i], PATH_SEPARATOR[0]);
        if (filename) {
            g_string_append(files_list, filename + 1);
        } else {
            g_string_append(files_list, widgets->source_files[i]);
        }
    }
    
    gtk_entry_set_text(GTK_ENTRY(widgets->src_files_entry), files_list->str);
    g_string_free(files_list, TRUE);
}

void free_project_files(AppWidgets *widgets) {
    if (widgets->source_files) {
        for (int i = 0; i < widgets->source_count; i++) {
            g_free(widgets->source_files[i]);
        }
        g_free(widgets->source_files);
        widgets->source_files = NULL;
    }
    widgets->source_count = 0;
}

void on_browse_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    GtkWidget *dialog = gtk_file_chooser_dialog_new("选择项目目录",
                                                   GTK_WINDOW(widgets->window),
                                                   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                   "_取消", GTK_RESPONSE_CANCEL,
                                                   "_选择", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) {
            scan_project_directory(widgets, filename);
            update_status(widgets, "项目目录已加载");
            g_free(filename);
        }
    }
    
    gtk_widget_destroy(dialog);
}

void on_project_selection_changed(GtkTreeSelection *selection, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *filename;
        gtk_tree_model_get(model, &iter, 0, &filename, -1);
        
        if (filename && widgets->project_path) {
            char *full_path = g_build_filename(widgets->project_path, filename, NULL);
            
            // 如果是源文件，可以在这里进行一些操作
            // 比如显示文件内容或详细信息
            
            g_free(full_path);
        }
        
        g_free(filename);
    }
}

void detect_platform_dependencies(AppWidgets *widgets) {
    const char *platform = detect_platform();
    GString *platform_info = g_string_new("");
    
    g_string_append_printf(platform_info, "检测到系统平台: %s\n", platform);
    g_string_append(platform_info, "平台特定依赖:\n");
    
    if (strcmp(platform, "Ubuntu") == 0 || strcmp(platform, "Debian") == 0) {
        g_string_append(platform_info, "包管理器: apt\n");
        g_string_append(platform_info, "安装命令示例: sudo apt install <package>\n");
    } else if (strcmp(platform, "Fedora") == 0 || strcmp(platform, "CentOS") == 0) {
        g_string_append(platform_info, "包管理器: dnf/yum\n");
        g_string_append(platform_info, "安装命令示例: sudo dnf install <package>\n");
    } else if (strcmp(platform, "Arch") == 0) {
        g_string_append(platform_info, "包管理器: pacman\n");
        g_string_append(platform_info, "安装命令示例: sudo pacman -S <package>\n");
    } else if (strcmp(platform, "macOS") == 0) {
        g_string_append(platform_info, "包管理器: brew\n");
        g_string_append(platform_info, "安装命令示例: brew install <package>\n");
    } else if (strcmp(platform, "Windows") == 0) {
        g_string_append(platform_info, "包管理器: vcpkg 或 MSYS2 pacman\n");
        g_string_append(platform_info, "安装命令示例: vcpkg install <package>\n");
    }
    
    append_to_deps_view(widgets, platform_info->str);
    g_string_free(platform_info, TRUE);
}

void on_analyze_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    gtk_text_buffer_set_text(widgets->deps_buffer, "", -1);
    update_status(widgets, "正在分析依赖...");
    
    detect_platform_dependencies(widgets);
    append_to_deps_view(widgets, "\n=== 源文件依赖分析 ===\n");
    
    if (widgets->source_count == 0) {
        append_to_deps_view(widgets, "错误: 没有源文件可分析\n");
        update_status(widgets, "分析完成 - 无源文件");
        return;
    }
    
    int analyzed_files = 0;
    for (int i = 0; i < widgets->source_count; i++) {
        GString *deps = g_string_new("");
        const char *short_name = strrchr(widgets->source_files[i], PATH_SEPARATOR[0]);
        g_string_append_printf(deps, "\n分析文件: %s\n", short_name ? short_name + 1 : widgets->source_files[i]);
        
        if (analyze_source_dependencies(widgets->source_files[i], deps)) {
            append_to_deps_view(widgets, deps->str);
            analyzed_files++;
        } else {
            g_string_append_printf(deps, "错误: 无法分析文件\n");
            append_to_deps_view(widgets, deps->str);
        }
        
        g_string_free(deps, TRUE);
    }
    
    if (analyzed_files > 0) {
        append_to_deps_view(widgets, "\n=== 依赖分析完成 ===\n");
        update_status(widgets, "依赖分析完成");
    } else {
        update_status(widgets, "分析完成 - 无有效文件");
    }
}

void on_save_deps_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(widgets->deps_buffer, &start, &end);
    gchar *deps_text = gtk_text_buffer_get_text(widgets->deps_buffer, &start, &end, FALSE);
    
    if (strlen(deps_text) == 0) {
        update_status(widgets, "错误: 没有依赖数据可保存");
        g_free(deps_text);
        return;
    }
    
    FILE *file = fopen("dependencies.txt", "w");
    if (!file) {
        update_status(widgets, "错误: 无法创建依赖文件");
        g_free(deps_text);
        return;
    }
    
    fprintf(file, "# 项目依赖文件\n");
    fprintf(file, "# 生成时间: %s", g_date_time_format(g_date_time_new_now_local(), "%Y-%m-%d %H:%M:%S"));
    fprintf(file, "# 系统平台: %s\n\n", detect_platform());
    fprintf(file, "%s", deps_text);
    
    fclose(file);
    g_free(deps_text);
    
    update_status(widgets, "依赖文件已保存为 dependencies.txt");
}

void generate_makefile(AppWidgets *widgets) {
    const gchar *project_name = gtk_entry_get_text(GTK_ENTRY(widgets->project_name_entry));
    const gchar *compiler = gtk_entry_get_text(GTK_ENTRY(widgets->compiler_entry));
    const gchar *cflags = gtk_entry_get_text(GTK_ENTRY(widgets->cflags_entry));
    const gchar *src_files = gtk_entry_get_text(GTK_ENTRY(widgets->src_files_entry));
    const gchar *output_name = gtk_entry_get_text(GTK_ENTRY(widgets->output_name_entry));
    const gchar *include_dirs = gtk_entry_get_text(GTK_ENTRY(widgets->include_dirs_entry));
    const gchar *libs = gtk_entry_get_text(GTK_ENTRY(widgets->libs_entry));
    
    if (strlen(compiler) == 0) compiler = "gcc";
    if (strlen(cflags) == 0) cflags = "-Wall -g";
    if (strlen(output_name) == 0) output_name = "program";
    
    FILE *file = fopen("Makefile", "w");
    if (!file) {
        update_status(widgets, "错误: 无法创建 Makefile");
        return;
    }
    
    fprintf(file, "# Generated by Makefile Generator\n");
    fprintf(file, "# Platform: %s\n\n", detect_platform());
    
    fprintf(file, "CC = %s\n", compiler);
    fprintf(file, "CFLAGS = %s\n", cflags);
    fprintf(file, "TARGET = %s\n", output_name);
    fprintf(file, "SRC = %s\n", src_files);
    
    if (strlen(include_dirs) > 0) {
        fprintf(file, "INCLUDES = %s\n", include_dirs);
    }
    
    if (strlen(libs) > 0) {
        fprintf(file, "LIBS = %s\n", libs);
    }
    
    fprintf(file, "\nOBJ = $(SRC:.c=.o)\n\n");
    
    fprintf(file, "all: $(TARGET)\n\n");
    
    fprintf(file, "$(TARGET): $(OBJ)\n");
    if (strlen(include_dirs) > 0 || strlen(libs) > 0) {
        fprintf(file, "\t$(CC) $(CFLAGS)");
        if (strlen(include_dirs) > 0) fprintf(file, " $(INCLUDES)");
        if (strlen(libs) > 0) fprintf(file, " $(LIBS)");
        fprintf(file, " -o $(TARGET) $(OBJ)\n\n");
    } else {
        fprintf(file, "\t$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)\n\n");
    }
    
    fprintf(file, "%.o: %.c\n");
    if (strlen(include_dirs) > 0) {
        fprintf(file, "\t$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@\n\n");
    } else {
        fprintf(file, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");
    }
    
    fprintf(file, "clean:\n");
    fprintf(file, "\trm -f $(OBJ) $(TARGET)\n\n");
    
    fprintf(file, "install: $(TARGET)\n");
    fprintf(file, "\tcp $(TARGET) /usr/local/bin/\n\n");
    
    fprintf(file, "deps:\n");
    fprintf(file, "\t@echo \"检查系统依赖...\"\n");
    fprintf(file, "\t# 请根据 dependencies.txt 安装所需依赖\n\n");
    
    fprintf(file, ".PHONY: all clean install deps\n");
    
    fclose(file);
    
    update_status(widgets, "Makefile 生成成功!");
}

void on_generate_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;
    generate_makefile(widgets);
}

// 创建带图标的单元格渲染器
GtkCellRenderer* create_icon_cell_renderer() {
    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
    return renderer;
}

void setup_ui(AppWidgets *widgets) {
    memset(widgets, 0, sizeof(AppWidgets));
    
    widgets->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "智能 Makefile 生成器 - 带项目浏览器");
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 800, 700);
    gtk_container_set_border_width(GTK_CONTAINER(widgets->window), 10);
    
    // 创建主容器
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(widgets->window), main_vbox);
    
    // 创建水平分割面板
    GtkWidget *hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), hpaned, TRUE, TRUE, 0);
    
    // 左侧：项目浏览器和设置
    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_paned_add1(GTK_PANED(hpaned), left_vbox);
    
    // 项目浏览器框架
    GtkWidget *browser_frame = gtk_frame_new("项目浏览器");
    gtk_box_pack_start(GTK_BOX(left_vbox), browser_frame, TRUE, TRUE, 0);
    
    GtkWidget *browser_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(browser_vbox), 10);
    gtk_container_add(GTK_CONTAINER(browser_frame), browser_vbox);
    
    // 浏览按钮
    GtkWidget *browse_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(browser_vbox), browse_hbox, FALSE, FALSE, 0);
    
    widgets->browse_btn = gtk_button_new_with_label("浏览项目目录");
    gtk_box_pack_start(GTK_BOX(browse_hbox), widgets->browse_btn, TRUE, TRUE, 0);
    
    // 项目树视图
    GtkWidget *scrolled_tree = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_tree),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_tree), 200);
    gtk_box_pack_start(GTK_BOX(browser_vbox), scrolled_tree, TRUE, TRUE, 0);
    
    // 创建项目列表存储
    widgets->project_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    
    widgets->project_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(widgets->project_store));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widgets->project_treeview), TRUE);
    
    // 创建列
    GtkTreeViewColumn *column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, "项目文件");
    
    // 图标渲染器
    GtkCellRenderer *icon_renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, icon_renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, icon_renderer, "icon-name", 1, NULL);
    
    // 文本渲染器
    GtkCellRenderer *text_renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, text_renderer, TRUE);
    gtk_tree_view_column_set_attributes(column, text_renderer, "text", 0, NULL);
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(widgets->project_treeview), column);
    
    gtk_container_add(GTK_CONTAINER(scrolled_tree), widgets->project_treeview);
    
    // 项目设置框架
    GtkWidget *settings_frame = gtk_frame_new("项目设置");
    gtk_box_pack_start(GTK_BOX(left_vbox), settings_frame, FALSE, FALSE, 0);
    
    GtkWidget *settings_grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(settings_grid), 10);
    gtk_container_add(GTK_CONTAINER(settings_frame), settings_grid);
    gtk_grid_set_row_spacing(GTK_GRID(settings_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(settings_grid), 5);
    
    int row = 0;
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("项目名称:"), 0, row, 1, 1);
    widgets->project_name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->project_name_entry, 1, row, 1, 1);
    row++;
    
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("编译器:"), 0, row, 1, 1);
    widgets->compiler_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->compiler_entry), "gcc");
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->compiler_entry, 1, row, 1, 1);
    row++;
    
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("编译选项:"), 0, row, 1, 1);
    widgets->cflags_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->cflags_entry), "-Wall -g");
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->cflags_entry, 1, row, 1, 1);
    row++;
    
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("源文件:"), 0, row, 1, 1);
    widgets->src_files_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->src_files_entry), "main.c utils.c");
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->src_files_entry, 1, row, 1, 1);
    row++;
    
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("输出文件名:"), 0, row, 1, 1);
    widgets->output_name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->output_name_entry), "program");
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->output_name_entry, 1, row, 1, 1);
    row++;
    
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("包含目录:"), 0, row, 1, 1);
    widgets->include_dirs_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->include_dirs_entry), "-I./include -I/usr/local/include");
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->include_dirs_entry, 1, row, 1, 1);
    row++;
    
    gtk_grid_attach(GTK_GRID(settings_grid), gtk_label_new("链接库:"), 0, row, 1, 1);
    widgets->libs_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->libs_entry), "-lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0");
    gtk_grid_attach(GTK_GRID(settings_grid), widgets->libs_entry, 1, row, 1, 1);
    
    // 右侧：依赖分析
    GtkWidget *right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_paned_add2(GTK_PANED(hpaned), right_vbox);
    
    GtkWidget *deps_frame = gtk_frame_new("依赖分析");
    gtk_box_pack_start(GTK_BOX(right_vbox), deps_frame, TRUE, TRUE, 0);
    
    GtkWidget *deps_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(deps_vbox), 10);
    gtk_container_add(GTK_CONTAINER(deps_frame), deps_vbox);
    
    // 依赖分析按钮
    GtkWidget *deps_buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(deps_vbox), deps_buttons_box, FALSE, FALSE, 0);
    
    widgets->analyze_btn = gtk_button_new_with_label("分析依赖");
    gtk_box_pack_start(GTK_BOX(deps_buttons_box), widgets->analyze_btn, TRUE, TRUE, 0);
    
    widgets->save_deps_btn = gtk_button_new_with_label("保存依赖文件");
    gtk_box_pack_start(GTK_BOX(deps_buttons_box), widgets->save_deps_btn, TRUE, TRUE, 0);
    
    // 依赖显示区域
    GtkWidget *scrolled_text = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_text),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(deps_vbox), scrolled_text, TRUE, TRUE, 0);
    
    widgets->deps_textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets->deps_textview), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(widgets->deps_textview), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled_text), widgets->deps_textview);
    
    widgets->deps_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->deps_textview));
    
    // 操作按钮和状态
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), action_box, FALSE, FALSE, 0);
    
    widgets->generate_btn = gtk_button_new_with_label("生成 Makefile");
    gtk_box_pack_start(GTK_BOX(action_box), widgets->generate_btn, TRUE, TRUE, 0);
    
    widgets->status_label = gtk_label_new("准备就绪 - 请选择项目目录");
    gtk_box_pack_start(GTK_BOX(main_vbox), widgets->status_label, FALSE, FALSE, 0);
    
    // 设置面板初始位置
    gtk_paned_set_position(GTK_PANED(hpaned), 400);
    
    // 连接信号
    g_signal_connect(widgets->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(widgets->generate_btn, "clicked", G_CALLBACK(on_generate_clicked), widgets);
    g_signal_connect(widgets->analyze_btn, "clicked", G_CALLBACK(on_analyze_clicked), widgets);
    g_signal_connect(widgets->save_deps_btn, "clicked", G_CALLBACK(on_save_deps_clicked), widgets);
    g_signal_connect(widgets->browse_btn, "clicked", G_CALLBACK(on_browse_clicked), widgets);
    
    // 项目选择变化信号
    widgets->project_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets->project_treeview));
    g_signal_connect(widgets->project_selection, "changed", 
                    G_CALLBACK(on_project_selection_changed), widgets);
}
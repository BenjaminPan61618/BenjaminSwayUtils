#include <gtk/gtk.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <json-glib/json-glib.h>

// 全局变量
GtkWidget *restart_btn;
gint timer_id;

// 布局数据结构
typedef struct {
    GtkWidget *left_list;
    GtkWidget *center_list; 
    GtkWidget *right_list;
} LayoutDisplay;

// 查找Waybar进程的PID
pid_t find_waybar_pid() {
    FILE *cmd = popen("pgrep -x waybar", "r");
    if (cmd) {
        int pid;
        if (fscanf(cmd, "%d", &pid) == 1) {
            pclose(cmd);
            return (pid_t)pid;
        }
        pclose(cmd);
    }
    return -1;
}

// 检查Waybar是否正在运行
bool is_waybar_running() {
    return find_waybar_pid() != -1;
}

// 启动Waybar
void start_waybar() {
    if (system("waybar &") == 0) {
        g_print("Waybar started successfully.\n");
    } else {
        g_print("Failed to start Waybar.\n");
    }
}

// 重启Waybar
void restart_waybar_process() {
    pid_t waybar_pid = find_waybar_pid();
    if (waybar_pid != -1) {
        kill(waybar_pid, SIGTERM);
        sleep(1);
    }
    start_waybar();
}

// 智能启动/重启函数
void smart_restart_waybar(GtkButton *button, gpointer user_data) {
    if (is_waybar_running()) {
        g_print("Waybar is running, restarting...\n");
        restart_waybar_process();
    } else {
        g_print("Waybar is not running, starting...\n");
        start_waybar();
    }
}
// 更新重启按钮的标签和状态
void update_restart_button_state() {
    if (restart_btn && GTK_IS_BUTTON(restart_btn)) {
        if (is_waybar_running()) {
            gtk_button_set_label(GTK_BUTTON(restart_btn), "重启 Waybar");
        } else {
            gtk_button_set_label(GTK_BUTTON(restart_btn), "启动 Waybar");
        }
    }
}
// 停止Waybar
void stop_waybar(GtkButton *button, gpointer user_data) {
    pid_t waybar_pid = find_waybar_pid();
    if (waybar_pid != -1) {
        kill(waybar_pid, SIGTERM);
        g_print("Waybar stopped.\n");
        update_restart_button_state();
    } else {
        g_print("Waybar is not running.\n");
    }
}



// 打开 nwg-look 程序
void open_nwg_look(GtkButton *button, gpointer user_data) {
    g_print("Launching nwg-look...\n");
    int result = system("nwg-look &");
    
    if (result == 0) {
        g_print("nwg-look launched successfully.\n");
    } else {
        g_print("Failed to launch nwg-look. Please make sure it is installed.\n");
    }
}

// 定时器回调函数，定期检查Waybar状态
gboolean check_waybar_status(gpointer user_data) {
    update_restart_button_state();
    return G_SOURCE_CONTINUE;
}

// 清空列表内容
void clear_list(GtkWidget *list) {
    GList *children, *iter;
    
    children = gtk_container_get_children(GTK_CONTAINER(list));
    for(iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

// 向列表中添加模块项
void add_module_to_list(GtkWidget *list, const gchar *module_name) {
    GtkWidget *label = gtk_label_new(module_name);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0); // 左对齐
    gtk_widget_set_margin_start(label, 5);
    gtk_widget_set_margin_end(label, 5);
    gtk_widget_set_margin_top(label, 2);
    gtk_widget_set_margin_bottom(label, 2);
    
    GtkWidget *row = gtk_list_box_row_new();
    gtk_container_add(GTK_CONTAINER(row), label);
    gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
}

// 解析JSON配置并更新布局显示
void update_layout_display(LayoutDisplay *display) {
    const gchar *config_path = "~/.config/waybar/config.jsonc";
    gchar *expanded_path = g_strdup(config_path);
    
    // 展开 ~ 到用户主目录
    if (expanded_path[0] == '~') {
        gchar *home = g_get_home_dir();
        gchar *new_path = g_build_filename(home, expanded_path + 1, NULL);
        g_free(expanded_path);
        expanded_path = new_path;
    }
    
    g_print("Loading config from: %s\n", expanded_path);
    
    // 读取和解析JSON文件
    GError *error = NULL;
    JsonParser *parser = json_parser_new();
    
    if (json_parser_load_from_file(parser, expanded_path, &error)) {
        JsonNode *root = json_parser_get_root(parser);
        JsonReader *reader = json_reader_new(root);
        
        // 清空现有列表
        clear_list(display->left_list);
        clear_list(display->center_list);
        clear_list(display->right_list);
        
        // 读取左侧模块
        if (json_reader_read_member(reader, "modules-left")) {
            if (json_reader_is_array(reader)) {
                gint count = json_reader_count_elements(reader);
                for (gint i = 0; i < count; i++) {
                    json_reader_read_element(reader, i);
                    const gchar *module = json_reader_get_string_value(reader);
                    if (module) {
                        add_module_to_list(display->left_list, module);
                    }
                    json_reader_end_element(reader);
                }
            }
            json_reader_end_member(reader);
        }
        
        // 读取中间模块
        if (json_reader_read_member(reader, "modules-center")) {
            if (json_reader_is_array(reader)) {
                gint count = json_reader_count_elements(reader);
                for (gint i = 0; i < count; i++) {
                    json_reader_read_element(reader, i);
                    const gchar *module = json_reader_get_string_value(reader);
                    if (module) {
                        add_module_to_list(display->center_list, module);
                    }
                    json_reader_end_element(reader);
                }
            }
            json_reader_end_member(reader);
        }
        
        // 读取右侧模块
        if (json_reader_read_member(reader, "modules-right")) {
            if (json_reader_is_array(reader)) {
                gint count = json_reader_count_elements(reader);
                for (gint i = 0; i < count; i++) {
                    json_reader_read_element(reader, i);
                    const gchar *module = json_reader_get_string_value(reader);
                    if (module) {
                        add_module_to_list(display->right_list, module);
                    }
                    json_reader_end_element(reader);
                }
            }
            json_reader_end_member(reader);
        }
        
        g_object_unref(reader);
        g_print("Layout configuration loaded successfully.\n");
    } else {
        g_print("Error loading config file: %s\n", error->message);
        g_error_free(error);
        
        // 显示错误信息
        GtkWidget *error_label = gtk_label_new("无法加载配置文件，请检查路径和格式");
        gtk_container_add(GTK_CONTAINER(display->left_list), error_label);
    }
    
    g_object_unref(parser);
    g_free(expanded_path);
    
    // 刷新显示
    gtk_widget_show_all(display->left_list);
    gtk_widget_show_all(display->center_list);
    gtk_widget_show_all(display->right_list);
}

// 刷新布局按钮回调
void refresh_layout(GtkButton *button, gpointer user_data) {
    LayoutDisplay *display = (LayoutDisplay *)user_data;
    g_print("Refreshing layout display...\n");
    update_layout_display(display);
}

// 创建布局显示标签页
GtkWidget* create_layout_tab(LayoutDisplay *display) {
    // 主容器
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // 刷新按钮
    GtkWidget *refresh_btn = gtk_button_new_with_label("刷新布局显示");
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(refresh_layout), display);
    gtk_box_pack_start(GTK_BOX(vbox), refresh_btn, FALSE, FALSE, 0);
    
    // 创建三栏水平布局
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    
    // 左侧栏
    GtkWidget *left_frame = gtk_frame_new("左侧模块 (modules-left)");
    gtk_widget_set_size_request(left_frame, 200, 300);
    gtk_box_pack_start(GTK_BOX(hbox), left_frame, TRUE, TRUE, 0);
    
    GtkWidget *left_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(left_scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(left_frame), left_scrolled);
    
    display->left_list = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(left_scrolled), display->left_list);
    
    // 中间栏
    GtkWidget *center_frame = gtk_frame_new("中间模块 (modules-center)");
    gtk_widget_set_size_request(center_frame, 200, 300);
    gtk_box_pack_start(GTK_BOX(hbox), center_frame, TRUE, TRUE, 0);
    
    GtkWidget *center_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(center_scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(center_frame), center_scrolled);
    
    display->center_list = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(center_scrolled), display->center_list);
    
    // 右侧栏
    GtkWidget *right_frame = gtk_frame_new("右侧模块 (modules-right)");
    gtk_widget_set_size_request(right_frame, 200, 300);
    gtk_box_pack_start(GTK_BOX(hbox), right_frame, TRUE, TRUE, 0);
    
    GtkWidget *right_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(right_scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(right_frame), right_scrolled);
    
    display->right_list = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(right_scrolled), display->right_list);
    
    return vbox;
}

// 创建控制标签页（原有的功能）
GtkWidget* create_control_tab() {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    
    // 状态指示标签
    GtkWidget *status_label = gtk_label_new("Waybar 状态: 检测中...");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);
    
    // 创建智能重启按钮
    restart_btn = gtk_button_new_with_label("检测中...");
    g_signal_connect(restart_btn, "clicked", G_CALLBACK(smart_restart_waybar), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), restart_btn, FALSE, FALSE, 0);
    
    // 创建停止按钮
    GtkWidget *stop_btn = gtk_button_new_with_label("停止 Waybar");
    g_signal_connect(stop_btn, "clicked", G_CALLBACK(stop_waybar), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), stop_btn, FALSE, FALSE, 0);
    
    // 创建主题设置按钮
    GtkWidget *gtk_settings_btn = gtk_button_new_with_label("打开 GTK 设置 (nwg-look)");
    g_signal_connect(gtk_settings_btn, "clicked", G_CALLBACK(open_nwg_look), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_settings_btn, FALSE, FALSE, 0);
    
    return vbox;
}

// 窗口关闭时的清理函数
void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    if (timer_id > 0) {
        g_source_remove(timer_id);
    }
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // 初始化布局显示结构
    LayoutDisplay display;
    
    // 创建主窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Waybar 管理器");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    // 创建笔记本（Tab容器）
    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);
    
    // 创建控制标签页
    GtkWidget *control_tab = create_control_tab();
    GtkWidget *control_label = gtk_label_new("控制");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), control_tab, control_label);
    
    // 创建布局标签页
    GtkWidget *layout_tab = create_layout_tab(&display);
    GtkWidget *layout_label = gtk_label_new("布局");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), layout_tab, layout_label);
    
    // 初始更新按钮状态
    update_restart_button_state();
    
    // 加载布局配置
    update_layout_display(&display);
    
    // 创建状态更新定时器（每3秒检查一次）
    timer_id = g_timeout_add_seconds(3, check_waybar_status, NULL);
    
    // 显示所有组件
    gtk_widget_show_all(window);
    
    // 主循环
    gtk_main();
    
    return 0;
}

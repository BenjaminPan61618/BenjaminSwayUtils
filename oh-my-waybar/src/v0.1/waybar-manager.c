#include <gtk/gtk.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

// 全局变量
GtkWidget *restart_btn;
gint timer_id;

// 查找Waybar进程的PID（改进版本）
pid_t find_waybar_pid() {
    FILE *cmd = popen("pgrep -x waybar", "r");  // -x 精确匹配进程名
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
        sleep(1); // 等待进程完全结束
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
void update_restart_button_state() {
    if (is_waybar_running()) {
        gtk_button_set_label(GTK_BUTTON(restart_btn), "重启 Waybar");
        // 可以在这里添加视觉提示，比如改变按钮颜色
        gtk_widget_set_sensitive(restart_btn, TRUE);
    } else {
        gtk_button_set_label(GTK_BUTTON(restart_btn), "启动 Waybar");
        gtk_widget_set_sensitive(restart_btn, TRUE);
    }
}
// 停止Waybar
void stop_waybar(GtkButton *button, gpointer user_data) {
    pid_t waybar_pid = find_waybar_pid();
    if (waybar_pid != -1) {
        kill(waybar_pid, SIGTERM);
        g_print("Waybar stopped.\n");
        
        // 更新按钮状态
        update_restart_button_state();
    } else {
        g_print("Waybar is not running.\n");
    }
}

// 更新重启按钮的标签和状态


// 定时器回调函数，定期检查Waybar状态
gboolean check_waybar_status(gpointer user_data) {
    update_restart_button_state();
    return G_SOURCE_CONTINUE; // 返回TRUE保持定时器运行
}

void open_nwg_look(GtkButton *button, gpointer user_data) {
    g_print("Launching nwg-look...\n");
    // 使用 system 命令启动 nwg-look
    // '&' 是为了让 nwg-look 在后台运行，不阻塞我们的管理器
    int result = system("nwg-look");
    
    if (result == 0) {
        g_print("nwg-look launched successfully.\n");
    } else {
        g_print("Failed to launch nwg-look. Please make sure it is installed.\n");
    }
}

// 窗口关闭时的清理函数
void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    // 移除定时器
    if (timer_id > 0) {
        g_source_remove(timer_id);
    }
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // 创建主窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Waybar 控制面板");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // 创建垂直布局容器
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 创建状态指示标签
    GtkWidget *status_label = gtk_label_new("Waybar 控制面板");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 0);

    // 创建智能重启按钮
    restart_btn = gtk_button_new_with_label("检测中...");
    g_signal_connect(restart_btn, "clicked", G_CALLBACK(smart_restart_waybar), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), restart_btn, FALSE, FALSE, 0);

    // 创建停止按钮
    GtkWidget *stop_btn = gtk_button_new_with_label("停止 Waybar");
    g_signal_connect(stop_btn, "clicked", G_CALLBACK(stop_waybar), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), stop_btn, FALSE, FALSE, 0);
    GtkWidget *gtk_settings_btn = gtk_button_new_with_label("打开 GTK 设置");
    g_signal_connect(gtk_settings_btn, "clicked", G_CALLBACK(open_nwg_look), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_settings_btn, FALSE, FALSE, 0);
 

    // 初始更新按钮状态
    update_restart_button_state();

    // 创建状态更新定时器（每3秒检查一次）
    timer_id = g_timeout_add_seconds(3, check_waybar_status, NULL);

    // 显示所有组件
    gtk_widget_show_all(window);

    // 主循环
    gtk_main();

    return 0;
}

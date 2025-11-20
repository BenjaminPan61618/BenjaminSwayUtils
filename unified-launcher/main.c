#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#define NUM_APPS 6

typedef struct {
    const char *name;
    const char *description;
    const char *icon;
    const char *exec_path;
    const char *url;
} AppInfo;

AppInfo apps[NUM_APPS] = {
    {"Anasrava", "History Manager", "utilities-terminal", "anasrava-bin/anasrava", "anasrava-bin/src"},
    {"Brightness Ctrl", "Brightness Control", "display-brightness-symbolic", "bright-control/brightness-control", "bright-control/src"},
    {"Desktop Organzier", "Desktop Icons Organzier", "system-file-manager", "desktop_classifier/desktop-organizer", "desktop_classifier/src"},
    {"Make Maker", "Makefile Generator", "text-editor", "make-maker/makefile-generator", "make-maker/src"},
    {"Waybar Manager", "Waybar Control Panel", "preferences-desktop", "oh-my-waybar/waybar-manager", "oh-my-waybar/src"},
    {"Disk Space Monitor", "Disk Usage Guard", "utilities-terminal", "disk-space-monitor.sh", "."}
};

// 检查自启动状态
static gboolean check_autostart_status(const char *app_name) {
    char autostart_path[256];
    const char *config_dir = g_get_user_config_dir();
    snprintf(autostart_path, sizeof(autostart_path), "%s/autostart/%s.desktop", config_dir, app_name);
    
    return g_file_test(autostart_path, G_FILE_TEST_EXISTS);
}

// 创建desktop文件内容
static char* generate_desktop_content(const AppInfo *app) {
    static char content[1024];
    snprintf(content, sizeof(content),
             "[Desktop Entry]\n"
             "Type=Application\n"
             "Name=%s\n"
             "Comment=%s\n"
             "Exec=%s/%s\n"
             "Icon=%s\n"
             "Terminal=false\n"
             "Categories=Utility;\n",
             app->name, app->description, g_get_current_dir(), app->exec_path, app->icon);
    return content;
}

// 设置自启动状态
static void set_autostart_status(const AppInfo *app, gboolean enable) {
    char autostart_path[256];
    const char *config_dir = g_get_user_config_dir();
    snprintf(autostart_path, sizeof(autostart_path), "%s/autostart", config_dir);
    
    // 确保autostart目录存在
    g_mkdir_with_parents(autostart_path, 0755);
    
    // 构造desktop文件路径
    char desktop_path[256];
    snprintf(desktop_path, sizeof(desktop_path), "%s/%s.desktop", autostart_path, app->name);
    
    if (enable) {
        // 创建desktop文件
        FILE *file = fopen(desktop_path, "w");
        if (file) {
            fputs(generate_desktop_content(app), file);
            fclose(file);
        }
    } else {
        // 删除desktop文件
        remove(desktop_path);
    }
}

// 启动应用程序
static void launch_app(GtkWidget *widget, gpointer data) {
    AppInfo *app = (AppInfo *)data;
    char command[256];
    snprintf(command, sizeof(command), "./%s &", app->exec_path);
    system(command);
}

// 打开项目地址
static void open_url(GtkWidget *widget, gpointer data) {
    AppInfo *app = (AppInfo *)data;
    char command[256];
    snprintf(command, sizeof(command), "xdg-open %s &", app->url);
    system(command);
}

// 切换自启动状态
static void toggle_autostart(GtkWidget *widget, gpointer data) {
    AppInfo *app = (AppInfo *)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    
    set_autostart_status(app, active);
    
    if (active) {
        g_print("启用自启动: %s\n", app->name);
    } else {
        g_print("禁用自启动: %s\n", app->name);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *header_bar;
    GtkWidget *scroll_window;
    
    // 创建主窗口
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Unified Launcher");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    // 创建标题栏
    header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Unified Launcher");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Benjamin Sway Utils Manager");
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    
    // 创建滚动窗口
    scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), scroll_window);
    
    // 创建网格布局
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_window), grid);
    
    // 为每个应用创建界面元素
    for (int i = 0; i < NUM_APPS; i++) {
        GtkWidget *app_frame;
        GtkWidget *app_grid;
        GtkWidget *icon;
        GtkWidget *name_label;
        GtkWidget *desc_label;
        GtkWidget *launch_button;
        GtkWidget *url_button;
        GtkWidget *autostart_check;
        
        // 创建应用框架
        app_frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(app_frame), GTK_SHADOW_ETCHED_IN);
        
        // 创建应用网格
        app_grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(app_grid), 5);
        gtk_grid_set_column_spacing(GTK_GRID(app_grid), 5);
        gtk_container_set_border_width(GTK_CONTAINER(app_grid), 10);
        gtk_container_add(GTK_CONTAINER(app_frame), app_grid);
        
        // 应用图标
        icon = gtk_image_new_from_icon_name(apps[i].icon, GTK_ICON_SIZE_DIALOG);
        gtk_grid_attach(GTK_GRID(app_grid), icon, 0, 0, 1, 2);
        
        // 应用名称
        name_label = gtk_label_new(apps[i].name);
        gtk_widget_set_halign(name_label, GTK_ALIGN_START);
        gtk_widget_override_font(name_label, pango_font_description_from_string("Sans Bold 12"));
        gtk_grid_attach(GTK_GRID(app_grid), name_label, 1, 0, 2, 1);
        
        // 应用描述
        desc_label = gtk_label_new(apps[i].description);
        gtk_widget_set_halign(desc_label, GTK_ALIGN_START);
        gtk_label_set_line_wrap(GTK_LABEL(desc_label), TRUE);
        gtk_grid_attach(GTK_GRID(app_grid), desc_label, 1, 1, 2, 1);
        
        // 启动按钮
        launch_button = gtk_button_new_with_label("启动");
        g_signal_connect(launch_button, "clicked", G_CALLBACK(launch_app), &apps[i]);
        gtk_grid_attach(GTK_GRID(app_grid), launch_button, 1, 2, 1, 1);
        
        // URL按钮
        url_button = gtk_button_new_with_label("项目地址");
        g_signal_connect(url_button, "clicked", G_CALLBACK(open_url), &apps[i]);
        gtk_grid_attach(GTK_GRID(app_grid), url_button, 2, 2, 1, 1);
        
        // 自启动复选框
        autostart_check = gtk_check_button_new_with_label("自启动");
        // 检查当前自启动状态并设置初始值
        gboolean is_autostart_enabled = check_autostart_status(apps[i].name);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autostart_check), is_autostart_enabled);
        g_signal_connect(autostart_check, "toggled", G_CALLBACK(toggle_autostart), &apps[i]);
        gtk_grid_attach(GTK_GRID(app_grid), autostart_check, 3, 0, 1, 1);
        
        // 将应用框架添加到主网格
        int row = i / 2;
        int col = i % 2;
        gtk_grid_attach(GTK_GRID(grid), app_frame, col, row, 1, 1);
    }
    
    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    
    app = gtk_application_new("org.benjamin.unifiedlauncher", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
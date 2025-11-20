#include <gtk/gtk.h>
#include "window_manager.h"
#include "file_classifier.h"
#include "context_menu.h"
#include "tray_icon.h"
#include "desktop_monitor.h"
#include "settings.h"
#include "custom_categories.h"
#include "ui_components.h"

// 函数声明（保持你原有的函数）
void update_file_classification();
void init_gui();

void update_file_classification() {
    g_print("更新文件分类...\n");
    // 调用新的文件分类函数
    GList *files = scan_desktop_files();
    update_category_windows_from_list(files);
}

void init_gui() {
    // 使用新的窗口创建函数
    create_category_windows();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // 加载配置
    load_settings();
    load_custom_categories();
    
    // 初始化系统托盘
    init_tray_icon();
    
    // 启动桌面监控
    start_desktop_monitoring();
    
    // 初始化GUI
    init_gui();
    
    // 更新文件分类
    update_file_classification();
    
    gtk_main();
    return 0;
}

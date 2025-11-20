#include <gtk/gtk.h>
#include "gui.h"

int main(int argc, char *argv[]) {
    AppWidgets widgets;
    
    gtk_init(&argc, &argv);
    
    // 初始化界面
    setup_ui(&widgets);
    
    // 显示所有控件
    gtk_widget_show_all(widgets.window);
    
    // 启动主循环
    gtk_main();
    
    return 0;
}
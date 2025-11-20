#include <glib.h>
#include <gio/gio.h>
#include "desktop_monitor.h"

static GFileMonitor *desktop_monitor = NULL;

void on_desktop_changed(GFileMonitor *monitor, GFile *file, GFile *other_file,
                       GFileMonitorEvent event_type, gpointer user_data) {
    
    switch (event_type) {
        case G_FILE_MONITOR_EVENT_CREATED:
            g_print("文件创建: %s\n", g_file_get_basename(file));
            break;
        case G_FILE_MONITOR_EVENT_DELETED:
            g_print("文件删除: %s\n", g_file_get_basename(file));
            break;
        case G_FILE_MONITOR_EVENT_CHANGED:
            g_print("文件修改: %s\n", g_file_get_basename(file));
            break;
        default:
            break;
    }
    
    // 触发界面更新
    update_desktop_display();
}

void start_desktop_monitoring() {
    const gchar *desktop_path = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    GFile *desktop_dir = g_file_new_for_path(desktop_path);
    
    if (desktop_dir) {
        desktop_monitor = g_file_monitor_directory(desktop_dir, G_FILE_MONITOR_NONE, 
                                                 NULL, NULL);
        if (desktop_monitor) {
            g_signal_connect(desktop_monitor, "changed", 
                           G_CALLBACK(on_desktop_changed), NULL);
        }
        g_object_unref(desktop_dir);
    }
}

void update_desktop_display() {
    // 重新扫描桌面并更新所有分类窗口
    g_print("更新桌面显示...\n");
    // 这里会调用文件分类和窗口更新函数
}

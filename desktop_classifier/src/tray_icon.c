#include <gtk/gtk.h>
#include <gio/gio.h>
#include <libayatana-appindicator/app-indicator.h>
#include <stdio.h>
#include "tray_icon.h"
#include "window_manager.h"
#include "settings.h"

static GtkStatusIcon *tray_icon = NULL; // fallback
static AppIndicator *indicator = NULL;

static gchar* get_autostart_desktop_path() {
    const gchar *cfg = g_get_user_config_dir();
    return g_build_filename(cfg, "autostart", "desktop-organizer.desktop", NULL);
}

static gboolean is_autostart_enabled() {
    gchar *path = get_autostart_desktop_path();
    gboolean exists = g_file_test(path, G_FILE_TEST_EXISTS);
    g_free(path);
    return exists;
}

static void set_autostart_enabled(gboolean enabled) {
    gchar *path = get_autostart_desktop_path();
    if (enabled) {
        gchar *dir = g_path_get_dirname(path);
        g_mkdir_with_parents(dir, 0700);
        g_free(dir);
        const gchar *content =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Desktop Organizer\n"
            "Exec=desktop-organizer\n"
            "X-GNOME-Autostart-enabled=true\n";
        GError *err = NULL;
        if (!g_file_set_contents(path, content, -1, &err)) {
            if (err) { g_warning("Failed to write autostart file: %s", err->message); g_error_free(err); }
        }
    } else {
        remove(path);
    }
    g_free(path);
}

// 托盘图标点击事件
void on_tray_icon_click(GtkStatusIcon *icon, gpointer data) {
    g_print("托盘图标被点击\n");
}

// 托盘菜单
// 更新 create_tray_menu 函数，添加更多选项
GtkWidget* create_tray_menu() {
    GtkWidget *menu = gtk_menu_new();
    
    GtkWidget *show_item = gtk_menu_item_new_with_label("显示/隐藏窗口");
    GtkWidget *refresh_item = gtk_menu_item_new_with_label("刷新分类");
    GtkWidget *open_folder_item = gtk_menu_item_new_with_label("打开主目录");
    GtkWidget *settings_item = gtk_menu_item_new_with_label("设置");
    GtkWidget *autostart_item = gtk_check_menu_item_new_with_label("开机自启动");
    GtkWidget *separator1 = gtk_separator_menu_item_new();
    GtkWidget *about_item = gtk_menu_item_new_with_label("关于");
    GtkWidget *separator2 = gtk_separator_menu_item_new();
    GtkWidget *quit_item = gtk_menu_item_new_with_label("退出");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), show_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), refresh_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_folder_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), settings_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), autostart_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), about_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator2);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);
    
    g_signal_connect(show_item, "activate", G_CALLBACK(on_tray_show_hide), NULL);
    g_signal_connect(refresh_item, "activate", G_CALLBACK(on_tray_refresh), NULL);
    g_signal_connect(open_folder_item, "activate", G_CALLBACK(on_tray_open_folder), NULL);
    g_signal_connect(settings_item, "activate", G_CALLBACK(on_tray_settings), NULL);
    g_signal_connect(autostart_item, "activate", G_CALLBACK(on_tray_autostart), NULL);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_tray_about), NULL);
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_tray_quit), NULL);

    // 反映当前自动启动状态
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(autostart_item), is_autostart_enabled());
    
    gtk_widget_show_all(menu);
    return menu;
}

// 初始化系统托盘
void init_tray_icon() {
    // Try AppIndicator first
    GtkWidget *menu = create_tray_menu();
    indicator = app_indicator_new("desktop-organizer", "folder", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    if (indicator) {
        app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_menu(indicator, GTK_MENU(menu));
        app_indicator_set_title(indicator, "桌面整理工具");
        return;
    }

    // Fallback to GtkStatusIcon
    tray_icon = gtk_status_icon_new_from_icon_name("folder");
    gtk_status_icon_set_tooltip_text(tray_icon, "桌面整理工具");
    gtk_status_icon_set_visible(tray_icon, TRUE);

    g_signal_connect(tray_icon, "activate", G_CALLBACK(on_tray_icon_click), NULL);
    g_signal_connect(tray_icon, "popup-menu", G_CALLBACK(on_tray_menu_popup), NULL);
}

void on_tray_menu_popup(GtkStatusIcon *icon, guint button, guint activate_time, gpointer data) {
    GtkWidget *menu = create_tray_menu();
    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

void on_tray_show_hide(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    if (are_any_category_windows_visible()) {
        hide_all_category_windows();
    } else {
        show_all_category_windows();
    }
}

void on_tray_settings(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("设置", NULL, GTK_DIALOG_MODAL,
                                                    "关闭", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *show_hidden = gtk_check_button_new_with_label("显示隐藏文件");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_hidden), get_show_hidden_files());
    gtk_box_pack_start(GTK_BOX(box), show_hidden, FALSE, FALSE, 4);
    gtk_container_add(GTK_CONTAINER(area), box);
    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_CLOSE) {
        set_show_hidden_files(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(show_hidden)));
    }
    gtk_widget_destroy(dialog);
}

void on_tray_autostart(GtkMenuItem *item, gpointer data) {
    (void)data;
    gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));
    set_autostart_enabled(active);
}

void on_tray_quit(GtkMenuItem *item, gpointer data) {
    gtk_main_quit();
}
// 添加这些函数到 tray_icon.c 文件中合适的位置

void on_tray_refresh(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    // 刷新分类逻辑
    g_print("刷新桌面文件分类\n");
    // 这里应该调用重新扫描和分类的函数
}

void on_tray_about(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_INFO,
                                              GTK_BUTTONS_OK,
                                              "桌面整理工具\n版本 1.0\n一个简单的桌面文件分类工具");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void on_tray_open_folder(GtkMenuItem *item, gpointer data) {
    (void)item; (void)data;
    // 打开主目录或指定分类目录
    const gchar *home_dir = g_get_home_dir();
    gchar *cmd = g_strdup_printf("xdg-open '%s'", home_dir);
    system(cmd);
    g_free(cmd);
}
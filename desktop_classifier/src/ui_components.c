#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include "ui_components.h"
#include "file_classifier.h"
#include "context_menu.h"

// 创建文件图标
GtkWidget* create_file_icon(DesktopFile *file) {
    GtkWidget *event_box = gtk_event_box_new();
    gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK);
    gtk_widget_set_hexpand(event_box, FALSE);
    gtk_widget_set_halign(event_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(event_box, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(event_box, TRUE);
    gtk_widget_set_size_request(event_box, 72, -1);
    gtk_widget_set_margin_start(event_box, 2);
    gtk_widget_set_margin_end(event_box, 2);
    gtk_widget_set_margin_top(event_box, 2);
    gtk_widget_set_margin_bottom(event_box, 0);
    gtk_widget_set_name(event_box, "icon-cell");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    GtkWidget *image = gtk_image_new();
    GtkWidget *label = gtk_label_new(file->filename);
    gtk_widget_set_halign(image, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(image, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_label_set_xalign(GTK_LABEL(label), 0.5f);
    
    // 设置图标与名称：优先处理 .desktop 应用程序
    if (g_str_has_suffix(file->filename, ".desktop")) {
        GDesktopAppInfo *app = g_desktop_app_info_new_from_filename(file->filepath);
        if (app) {
            const gchar *app_name = g_app_info_get_display_name(G_APP_INFO(app));
            if (app_name && *app_name) {
                gtk_label_set_text(GTK_LABEL(label), app_name);
            }
            GIcon *app_icon = g_app_info_get_icon(G_APP_INFO(app));
            if (G_IS_ICON(app_icon)) {
                gtk_image_set_from_gicon(GTK_IMAGE(image), app_icon, GTK_ICON_SIZE_DIALOG);
            } else {
                // Fallback icon for apps without icon
                gtk_image_set_from_icon_name(GTK_IMAGE(image), "application-x-executable", GTK_ICON_SIZE_DIALOG);
            }
            g_object_unref(app);
        } else {
            // Fallback if .desktop could not be parsed
            gtk_image_set_from_icon_name(GTK_IMAGE(image), "application-x-executable", GTK_ICON_SIZE_DIALOG);
        }
    } else {
        GIcon *gicon = NULL;
        GFile *gfile = g_file_new_for_path(file->filepath);
        GFileInfo *info = g_file_query_info(gfile, "standard::icon", 
                                           G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info) {
            gicon = g_file_info_get_icon(info);
            if (G_IS_ICON(gicon)) {
                gtk_image_set_from_gicon(GTK_IMAGE(image), gicon, GTK_ICON_SIZE_DIALOG);
            } else {
                // Generic fallback icon for unknown files
                gtk_image_set_from_icon_name(GTK_IMAGE(image), "text-x-generic", GTK_ICON_SIZE_DIALOG);
            }
            g_object_unref(info);
        } else {
            gtk_image_set_from_icon_name(GTK_IMAGE(image), "text-x-generic", GTK_ICON_SIZE_DIALOG);
        }
        g_object_unref(gfile);
    }
    
    // 设置标签
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 15);
    gtk_label_set_single_line_mode(GTK_LABEL(label), TRUE);
    
    // 组装
    gtk_container_add(GTK_CONTAINER(event_box), box);
    gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    
    // 设置右键菜单
    g_signal_connect(event_box, "button-press-event", 
                    G_CALLBACK(on_file_icon_button_press), file);
    
    return event_box;
}

// 文件图标点击事件
gboolean on_file_icon_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    DesktopFile *file = (DesktopFile *)data;
    
    if (event->button == 3) { // 右键
        GtkWidget *menu = create_file_context_menu(file);
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
        return TRUE;
    } else if (event->button == 1 && event->type == GDK_2BUTTON_PRESS) { // 双击左键
        // 打开文件
        GError *error = NULL;
        gchar *uri = g_filename_to_uri(file->filepath, NULL, NULL);
        gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
        g_free(uri);
        
        if (error) {
            g_print("打开文件错误: %s\n", error->message);
            g_error_free(error);
        }
        return TRUE;
    }
    
    return FALSE;
}

// 创建分类窗口的标题栏
GtkWidget* create_category_header(const gchar *category_name, gint file_count) {
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label = gtk_label_new(category_name);
    GtkWidget *count_label = gtk_label_new(g_strdup_printf("(%d)", file_count));
    GtkWidget *menu_button = gtk_menu_button_new();
    gtk_widget_set_name(label, "category-title");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_widget_set_margin_start(header_box, 8);
    gtk_widget_set_margin_end(header_box, 8);
    gtk_widget_set_margin_top(header_box, 4);
    gtk_widget_set_margin_bottom(header_box, 4);
    
    // 设置样式
    gtk_widget_set_name(header_box, "category-header");
    
    // 分类菜单
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *hide_item = gtk_menu_item_new_with_label("隐藏此分类");
    GtkWidget *settings_item = gtk_menu_item_new_with_label("分类设置");
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), hide_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), settings_item);
    
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(menu_button), menu);
    gtk_menu_button_set_direction(GTK_MENU_BUTTON(menu_button), GTK_ARROW_DOWN);
    
    // 组装标题栏
    gtk_box_pack_start(GTK_BOX(header_box), label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(header_box), count_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header_box), menu_button, FALSE, FALSE, 0);
    
    return header_box;
}

#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <gtk/gtk.h>

void init_tray_icon();
void on_tray_show_hide(GtkMenuItem *item, gpointer data);
void on_tray_settings(GtkMenuItem *item, gpointer data);
void on_tray_autostart(GtkMenuItem *item, gpointer data);
void on_tray_quit(GtkMenuItem *item, gpointer data);
void on_tray_menu_popup(GtkStatusIcon *icon, guint button, guint activate_time, gpointer data);
// 确保在 tray_icon.h 中有以下声明
void on_tray_refresh(GtkMenuItem *item, gpointer data);
void on_tray_about(GtkMenuItem *item, gpointer data);
void on_tray_open_folder(GtkMenuItem *item, gpointer data);
#endif

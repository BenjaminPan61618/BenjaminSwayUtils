#ifndef CONTEXT_MENU_H
#define CONTEXT_MENU_H

#include <gtk/gtk.h>
#include "file_classifier.h"

// 右键菜单函数
GtkWidget* create_desktop_context_menu(gpointer user_data);
GtkWidget* create_file_context_menu(DesktopFile *file);
GtkWidget* create_context_menu(GtkWidget* parent, const gchar* filename);

// 菜单回调函数
void on_refresh(GtkMenuItem *item, gpointer data);
void on_rename(GtkMenuItem *item, gpointer data);
void on_delete(GtkMenuItem *item, gpointer data);
void on_copy(GtkMenuItem *item, gpointer data);
void on_cut(GtkMenuItem *item, gpointer data);
void on_paste(GtkMenuItem *item, gpointer data);
void on_properties(GtkMenuItem *item, gpointer data);
void on_open_symlink_target(GtkMenuItem *item, gpointer data);
void on_open_file(GtkMenuItem *item, gpointer data);
void on_open_desktop_in_file_manager(GtkMenuItem *item, gpointer data);
void on_open_containing_folder(GtkMenuItem *item, gpointer data);
void on_open_with(GtkMenuItem *item, gpointer data);

#endif

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *flowbox;
    GtkWidget *header_label;
    gint x, y;
    gint width, height;
    gchar *category; // internal grouping key
    gchar *display_name; // user-visible name, editable
    gboolean visible;
} CategoryWindow;

// 窗口管理函数
GtkWidget* create_category_window(const gchar *category, gint x, gint y);
void check_window_snap(CategoryWindow *win);
void save_window_positions();
void create_category_windows();
void toggle_category_visibility(const gchar *category, gboolean visible);
void update_category_windows_from_list(GList *files);
gboolean are_any_category_windows_visible();
void show_all_category_windows();
void hide_all_category_windows();

#endif

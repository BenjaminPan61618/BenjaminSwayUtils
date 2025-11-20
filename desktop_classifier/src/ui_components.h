#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <gtk/gtk.h>
#include "file_classifier.h"

GtkWidget* create_file_icon(DesktopFile *file);
GtkWidget* create_category_header(const gchar *category_name, gint file_count);
gboolean on_file_icon_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);

#endif

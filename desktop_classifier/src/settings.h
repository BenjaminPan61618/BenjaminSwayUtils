#ifndef SETTINGS_H
#define SETTINGS_H

#include <glib.h>

void load_settings();
void save_settings();
gboolean get_show_hidden_files();
void set_show_hidden_files(gboolean value);
gboolean is_file_excluded(const gchar *filename);
void add_excluded_pattern(const gchar *pattern);

#endif

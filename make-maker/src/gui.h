#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *project_name_entry;
    GtkWidget *compiler_entry;
    GtkWidget *cflags_entry;
    GtkWidget *src_files_entry;
    GtkWidget *output_name_entry;
    GtkWidget *include_dirs_entry;
    GtkWidget *libs_entry;
    GtkWidget *generate_btn;
    GtkWidget *analyze_btn;
    GtkWidget *save_deps_btn;
    GtkWidget *browse_btn;
    GtkWidget *status_label;
    GtkWidget *deps_textview;
    GtkWidget *project_treeview;
    GtkTextBuffer *deps_buffer;
    GtkListStore *project_store;
    GtkTreeSelection *project_selection;
    char *project_path;
    char **source_files;
    int source_count;
} AppWidgets;

void on_generate_clicked(GtkButton *button, gpointer user_data);
void on_analyze_clicked(GtkButton *button, gpointer user_data);
void on_save_deps_clicked(GtkButton *button, gpointer user_data);
void on_browse_clicked(GtkButton *button, gpointer user_data);
void on_project_selection_changed(GtkTreeSelection *selection, gpointer user_data);
void setup_ui(AppWidgets *widgets);
void detect_platform_dependencies(AppWidgets *widgets);
gboolean analyze_source_dependencies(const char *filename, GString *deps);
char* detect_system_packages(const char *library);
void scan_project_directory(AppWidgets *widgets, const char *path);
void update_source_files_list(AppWidgets *widgets);
gboolean is_source_file(const char *filename);
gboolean is_header_file(const char *filename);
void free_project_files(AppWidgets *widgets);

#endif
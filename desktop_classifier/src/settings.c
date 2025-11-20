#include <glib.h>
#include <gio/gio.h>
#include "settings.h"

static GSettings *settings = NULL;

typedef struct {
    gboolean show_hidden_files;
    gboolean show_file_extensions;
    gboolean window_snap;
    gboolean auto_start;
    gboolean remember_positions;
    GList *excluded_patterns;
} AppSettings;

static AppSettings app_settings = {
    .show_hidden_files = FALSE,
    .show_file_extensions = TRUE,
    .window_snap = TRUE,
    .auto_start = FALSE,
    .remember_positions = TRUE,
    .excluded_patterns = NULL
};

void load_settings() {
    // Try to find the schema; if not installed, fall back to defaults
    GSettingsSchemaSource *default_source = g_settings_schema_source_get_default();
    GSettingsSchema *schema = NULL;
    if (default_source) {
        schema = g_settings_schema_source_lookup(default_source, "com.example.desktop-organizer", TRUE);
    }

    if (schema) {
        settings = g_settings_new_full(schema, NULL, NULL);
    } else {
        g_warning("GSettings schema 'com.example.desktop-organizer' not found. Using default settings only.");
        settings = NULL;
    }

    if (settings) {
        app_settings.show_hidden_files = g_settings_get_boolean(settings, "show-hidden-files");
        app_settings.show_file_extensions = g_settings_get_boolean(settings, "show-file-extensions");
        app_settings.window_snap = g_settings_get_boolean(settings, "window-snap");
        app_settings.auto_start = g_settings_get_boolean(settings, "auto-start");
        app_settings.remember_positions = g_settings_get_boolean(settings, "remember-positions");
        
        // 加载排除模式
        gchar **patterns = g_settings_get_strv(settings, "excluded-patterns");
        if (patterns) {
            for (gchar **pattern = patterns; *pattern; pattern++) {
                app_settings.excluded_patterns = g_list_append(app_settings.excluded_patterns, g_strdup(*pattern));
            }
            g_strfreev(patterns);
        }
    }
}

void save_settings() {
    if (settings) {
        g_settings_set_boolean(settings, "show-hidden-files", app_settings.show_hidden_files);
        g_settings_set_boolean(settings, "show-file-extensions", app_settings.show_file_extensions);
        g_settings_set_boolean(settings, "window-snap", app_settings.window_snap);
        g_settings_set_boolean(settings, "auto-start", app_settings.auto_start);
        g_settings_set_boolean(settings, "remember-positions", app_settings.remember_positions);
        
        // 保存排除模式
        GPtrArray *patterns = g_ptr_array_new();
        GList *iter = app_settings.excluded_patterns;
        while (iter) {
            g_ptr_array_add(patterns, iter->data);
            iter = iter->next;
        }
        g_ptr_array_add(patterns, NULL);
        g_settings_set_strv(settings, "excluded-patterns", (const gchar **)patterns->pdata);
        g_ptr_array_free(patterns, TRUE);
    }
}

gboolean get_show_hidden_files() {
    return app_settings.show_hidden_files;
}

void set_show_hidden_files(gboolean value) {
    app_settings.show_hidden_files = value;
    save_settings();
}

gboolean is_file_excluded(const gchar *filename) {
    GList *iter = app_settings.excluded_patterns;
    while (iter) {
        const gchar *pattern = (const gchar *)iter->data;
        if (g_pattern_match_simple(pattern, filename)) {
            return TRUE;
        }
        iter = iter->next;
    }
    return FALSE;
}

void add_excluded_pattern(const gchar *pattern) {
    app_settings.excluded_patterns = g_list_append(app_settings.excluded_patterns, g_strdup(pattern));
    save_settings();
}

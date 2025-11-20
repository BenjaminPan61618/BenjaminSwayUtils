#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include "file_classifier.h"
#include "custom_categories.h"

GList *custom_categories = NULL;

void load_custom_categories() {
    gchar *config_path = g_build_filename(g_get_user_config_dir(), 
                                         "desktop-organizer", "categories.json", NULL);
    
    if (!g_file_test(config_path, G_FILE_TEST_EXISTS)) {
        g_free(config_path);
        return;
    }
    
    JsonParser *parser = json_parser_new();
    if (json_parser_load_from_file(parser, config_path, NULL)) {
        JsonNode *root = json_parser_get_root(parser);
        JsonArray *array = json_node_get_array(root);
        
        guint length = json_array_get_length(array);
        for (guint i = 0; i < length; i++) {
            JsonObject *obj = json_array_get_object_element(array, i);
            
            CustomCategory *cat = g_new0(CustomCategory, 1);
            cat->name = g_strdup(json_object_get_string_member(obj, "name"));
            cat->display_name = g_strdup(json_object_get_string_member(obj, "display_name"));
            cat->pattern = g_regex_new(json_object_get_string_member(obj, "pattern"), 
                                     0, 0, NULL);
            cat->enabled = json_object_get_boolean_member(obj, "enabled");
            cat->position = json_object_get_int_member(obj, "position");
            
            custom_categories = g_list_append(custom_categories, cat);
        }
    }
    
    g_object_unref(parser);
    g_free(config_path);
}

void add_custom_category(const gchar *name, const gchar *pattern, const gchar *display_name) {
    CustomCategory *cat = g_new0(CustomCategory, 1);
    cat->name = g_strdup(name);
    cat->display_name = g_strdup(display_name ? display_name : name);
    cat->pattern = g_regex_new(pattern, 0, 0, NULL);
    cat->enabled = TRUE;
    cat->position = g_list_length(custom_categories);
    
    custom_categories = g_list_append(custom_categories, cat);
    save_custom_categories();
}

FileCategory classify_with_custom_categories(const gchar *filename) {
    GList *iter = custom_categories;
    while (iter) {
        CustomCategory *cat = (CustomCategory*)iter->data;
        if (cat->enabled && g_regex_match(cat->pattern, filename, 0, NULL)) {
            return CATEGORY_CUSTOM; // 或者返回特定的自定义分类ID
        }
        iter = iter->next;
    }
    return CATEGORY_OTHER;
}

// --- Stubs to satisfy header declarations ---
void save_custom_categories() {}
void remove_custom_category(const gchar *name) { (void)name; }
GList* get_custom_categories() { return custom_categories; }

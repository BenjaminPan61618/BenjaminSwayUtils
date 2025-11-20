#ifndef CUSTOM_CATEGORIES_H
#define CUSTOM_CATEGORIES_H

#include <glib.h>

typedef struct {
    gchar *name;
    gchar *display_name;
    GRegex *pattern;
    gboolean enabled;
    gint position;
} CustomCategory;

// 自定义分类管理函数
void load_custom_categories();
void save_custom_categories();
void add_custom_category(const gchar *name, const gchar *pattern, const gchar *display_name);
void remove_custom_category(const gchar *name);
GList* get_custom_categories();

#endif

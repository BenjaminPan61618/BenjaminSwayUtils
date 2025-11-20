#ifndef FILE_CLASSIFIER_H
#define FILE_CLASSIFIER_H

#include <glib.h>

typedef enum {
    CATEGORY_FOLDER,
    CATEGORY_APPLICATION,
    CATEGORY_MUSIC,
    CATEGORY_VIDEO,
    CATEGORY_DOCUMENT,
    CATEGORY_IMAGE,
    CATEGORY_ARCHIVE,
    CATEGORY_CUSTOM,
    CATEGORY_OTHER
} FileCategory;

typedef struct {
    gchar *filename;
    gchar *filepath;
    FileCategory category;
    gboolean is_hidden;
    gboolean is_symlink;
    gchar *target_path;
} DesktopFile;

// 文件分类函数
FileCategory classify_file(const gchar *filename, const gchar *filepath);
GList* scan_desktop_files();
gboolean is_file_excluded(const gchar *filename);
FileCategory classify_with_custom_categories(const gchar *filename);

#endif

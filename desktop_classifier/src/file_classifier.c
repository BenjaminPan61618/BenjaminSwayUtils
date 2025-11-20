#include <glib.h>
#include <gio/gio.h>
#include "settings.h"
#include "file_classifier.h"


FileCategory classify_file(const gchar *filename, const gchar *filepath) {
    GFile *file = g_file_new_for_path(filepath);
    GFileInfo *info = g_file_query_info(file, "standard::*", 
                                       G_FILE_QUERY_INFO_NONE, NULL, NULL);
    
    if (g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
        g_object_unref(info);
        g_object_unref(file);
        return CATEGORY_FOLDER;
    }
    
    // 检查文件扩展名和MIME类型
    const gchar *content_type = g_file_info_get_content_type(info);
    gchar *basename = g_path_get_basename(filename);
    
    // 应用程序（.desktop文件）
    if (g_str_has_suffix(basename, ".desktop")) {
        g_free(basename);
        g_object_unref(info);
        g_object_unref(file);
        return CATEGORY_APPLICATION;
    }
    
    // 根据MIME类型分类
    if (g_str_has_prefix(content_type, "audio/")) {
        return CATEGORY_MUSIC;
    } else if (g_str_has_prefix(content_type, "video/")) {
        return CATEGORY_VIDEO;
    } else if (g_str_has_prefix(content_type, "image/")) {
        return CATEGORY_IMAGE;
    } else if (g_str_has_prefix(content_type, "text/") || 
               g_str_has_suffix(basename, ".pdf") ||
               g_str_has_suffix(basename, ".doc")) {
        return CATEGORY_DOCUMENT;
    } else if (g_str_has_prefix(content_type, "application/x-") ||
               g_str_has_suffix(basename, ".zip") ||
               g_str_has_suffix(basename, ".tar")) {
        return CATEGORY_ARCHIVE;
    }
    
    g_free(basename);
    g_object_unref(info);
    g_object_unref(file);
    return CATEGORY_OTHER;
}

// 扫描桌面文件夹
GList* scan_desktop_files() {
    GList *files = NULL;
    const gchar *desktop_path = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP);
    GDir *dir = g_dir_open(desktop_path, 0, NULL);
    
    if (!dir) return NULL;
    
    const gchar *filename;
    while ((filename = g_dir_read_name(dir))) {
        // 跳过隐藏文件（根据设置）
        if (filename[0] == '.' && !get_show_hidden_files()) {
            continue;
        }
        
        // 跳过排除的文件
        if (is_file_excluded(filename)) {
            continue;
        }
        
        gchar *full_path = g_build_filename(desktop_path, filename, NULL);
        DesktopFile *dfile = g_new0(DesktopFile, 1);
        dfile->filename = g_strdup(filename);
        dfile->filepath = full_path;
        dfile->category = classify_file(filename, full_path);
        dfile->is_hidden = (filename[0] == '.');
        
        // 检查符号链接
        GFile *file = g_file_new_for_path(full_path);
        GFileInfo *info = g_file_query_info(file, "standard::is-symlink", 
                                           G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (info && g_file_info_get_is_symlink(info)) {
            dfile->is_symlink = TRUE;
            dfile->target_path = g_file_read_link(full_path, NULL);
        }
        
        files = g_list_append(files, dfile);
        g_object_unref(info);
        g_object_unref(file);
    }
    
    g_dir_close(dir);
    return files;
}

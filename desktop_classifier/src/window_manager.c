#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>
#include <cairo.h>
#include "window_manager.h"
#include "ui_components.h"

static GHashTable *windows = NULL;
static GSettings *settings = NULL;

// 点击空白处时取消选中
static gboolean on_flowbox_button(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GtkFlowBox *flow = GTK_FLOW_BOX(user_data);
    if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
        GtkFlowBoxChild *child = gtk_flow_box_get_child_at_pos(flow, (gint)event->x, (gint)event->y);
        if (child == NULL) {
            gtk_flow_box_unselect_all(flow);
        }
    }
    return FALSE;
}

// 窗口聚焦/失焦样式切换
static gboolean on_window_focus_in(GtkWidget *w, GdkEvent *e, gpointer data) {
    (void)e; (void)data;
    GtkStyleContext *ctx = gtk_widget_get_style_context(w);
    gtk_style_context_remove_class(ctx, "unfocused");
    return FALSE;
}

static gboolean on_window_focus_out(GtkWidget *w, GdkEvent *e, gpointer data) {
    (void)e; (void)data;
    GtkStyleContext *ctx = gtk_widget_get_style_context(w);
    gtk_style_context_add_class(ctx, "unfocused");
    return FALSE;
}

// 简单的盒式模糊实现，用于窗口背景截图
static void box_blur_rgba(guchar *data, int width, int height, int stride, int radius) {
    if (radius <= 0) return;
    const int channels = 4;
    const int kernel = radius * 2 + 1;
    guchar *tmp = g_malloc((gsize)height * (gsize)stride);

    // 水平
    for (int y = 0; y < height; y++) {
        int sum[4] = {0,0,0,0};
        guchar *row = data + y * stride;
        for (int k = -radius; k <= radius; k++) {
            int xi = k < 0 ? 0 : (k >= width ? width - 1 : k);
            guchar *p = row + xi * channels;
            sum[0]+=p[0]; sum[1]+=p[1]; sum[2]+=p[2]; sum[3]+=p[3];
        }
        for (int x = 0; x < width; x++) {
            guchar *dst = tmp + y * stride + x * channels;
            dst[0] = (guchar)(sum[0] / kernel);
            dst[1] = (guchar)(sum[1] / kernel);
            dst[2] = (guchar)(sum[2] / kernel);
            dst[3] = (guchar)(sum[3] / kernel);

            int xout = x - radius; if (xout < 0) xout = 0;
            int xin  = x + radius + 1; if (xin >= width) xin = width - 1;
            guchar *pout = row + xout * channels;
            guchar *pin  = row + xin  * channels;
            sum[0] += pin[0] - pout[0];
            sum[1] += pin[1] - pout[1];
            sum[2] += pin[2] - pout[2];
            sum[3] += pin[3] - pout[3];
        }
    }

    // 垂直
    for (int x = 0; x < width; x++) {
        int sum[4] = {0,0,0,0};
        for (int k = -radius; k <= radius; k++) {
            int yi = k < 0 ? 0 : (k >= height ? height - 1 : k);
            guchar *p = tmp + yi * stride + x * channels;
            sum[0]+=p[0]; sum[1]+=p[1]; sum[2]+=p[2]; sum[3]+=p[3];
        }
        for (int y = 0; y < height; y++) {
            guchar *dst = data + y * stride + x * channels;
            dst[0] = (guchar)(sum[0] / kernel);
            dst[1] = (guchar)(sum[1] / kernel);
            dst[2] = (guchar)(sum[2] / kernel);
            dst[3] = (guchar)(sum[3] / kernel);

            int yout = y - radius; if (yout < 0) yout = 0;
            int yin  = y + radius + 1; if (yin >= height) yin = height - 1;
            guchar *pout = tmp + yout * stride + x * channels;
            guchar *pin  = tmp + yin  * stride + x * channels;
            sum[0] += pin[0] - pout[0];
            sum[1] += pin[1] - pout[1];
            sum[2] += pin[2] - pout[2];
            sum[3] += pin[3] - pout[3];
        }
    }
    g_free(tmp);
}

static gboolean on_blur_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    (void)user_data;
    // 仅在 X11 下可用，Wayland 下返回 FALSE 以避免崩溃
    if (!gdk_display_get_default()) return FALSE;
    GdkWindow *root = gdk_get_default_root_window();
    if (!root) return FALSE;

    // 截取窗口所在区域的背景
    gint wx = 0, wy = 0;
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
    if (GTK_IS_WINDOW(toplevel)) {
        gtk_window_get_position(GTK_WINDOW(toplevel), &wx, &wy);
    }
    gint w = gtk_widget_get_allocated_width(widget);
    gint h = gtk_widget_get_allocated_height(widget);
    if (w <= 0 || h <= 0) return FALSE;

    GdkPixbuf *shot = gdk_pixbuf_get_from_window(root, wx, wy, w, h);
    if (!shot) return FALSE;

    // 确保有 alpha 通道
    if (!gdk_pixbuf_get_has_alpha(shot)) {
        GdkPixbuf *tmp = gdk_pixbuf_add_alpha(shot, FALSE, 0, 0, 0);
        g_object_unref(shot);
        shot = tmp;
    }

    guchar *pixels = gdk_pixbuf_get_pixels(shot);
    int stride = gdk_pixbuf_get_rowstride(shot);
    int width = gdk_pixbuf_get_width(shot);
    int height = gdk_pixbuf_get_height(shot);
    box_blur_rgba(pixels, width, height, stride, 8);

    gdk_cairo_set_source_pixbuf(cr, shot, 0, 0);
    cairo_paint_with_alpha(cr, 0.85);
    g_object_unref(shot);
    return TRUE;
}

// 创建无标题栏分类窗口
GtkWidget* create_category_window(const gchar *category, gint x, gint y) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // 移除标题栏和边框
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    
    // 设置窗口属性
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_below(GTK_WINDOW(window), TRUE);

    // 使用 Overlay 在底层绘制模糊背景
    GtkWidget *overlay = gtk_overlay_new();
    GtkWidget *blur_bg = gtk_drawing_area_new();
    g_signal_connect(blur_bg, "draw", G_CALLBACK(on_blur_draw), NULL);
    gtk_container_add(GTK_CONTAINER(window), overlay);
    gtk_container_add(GTK_CONTAINER(overlay), blur_bg);

    // 布局：标题 + 滚动 + FlowBox
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *header = create_category_header(category, 0);
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *flowbox = gtk_flow_box_new();
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flowbox), 8);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flowbox), 8);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flowbox), GTK_SELECTION_SINGLE);
    gtk_widget_add_events(flowbox, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(flowbox, "button-press-event", G_CALLBACK(on_flowbox_button), flowbox);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), vbox);
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(scrolled), flowbox);

    // 记住 flowbox 以便后续更新
    g_object_set_data(G_OBJECT(window), "flowbox", flowbox);
    
    // 样式
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, 
        ".category-window { background-color: rgba(40, 40, 40, 0.6); border-radius: 10px; }"
        ".category-window.unfocused { background-color: rgba(40, 40, 40, 0.4); }"
        ".category-header { background-color: rgba(0, 0, 0, 0.5); color: white; padding: 5px; }"
        "#icon-cell { padding: 4px; border-radius: 6px; }"
        "#icon-cell:hover { background-color: rgba(255,255,255,0.12); }"
        "#icon-cell:selected { background-color: rgba(66,133,244,0.35); }",
        -1, NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), 
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_class(context, "category-window");
    g_signal_connect(window, "focus-in-event", G_CALLBACK(on_window_focus_in), NULL);
    g_signal_connect(window, "focus-out-event", G_CALLBACK(on_window_focus_out), NULL);
    
    // 位置和大小
    gtk_window_move(GTK_WINDOW(window), x, y);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    
    return window;
}

// 窗口吸附功能
void check_window_snap(CategoryWindow *win) {
    // 实现窗口吸附逻辑，避免堆叠
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, windows);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CategoryWindow *other = (CategoryWindow*)value;
        if (other != win) {
            // 检查窗口间距，实现吸附
            gint snap_distance = 20;
            if (abs(win->x - other->x) < snap_distance) {
                win->x = other->x;
            }
            if (abs(win->y - other->y) < snap_distance) {
                win->y = other->y;
            }
        }
    }
}

// 保存窗口位置
void save_window_positions() {
    GHashTableIter iter;
    gpointer key, value;
    
    g_hash_table_iter_init(&iter, windows);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CategoryWindow *win = (CategoryWindow*)value;
        gchar *key_name = g_strdup_printf("window-position-%s", win->category);
        g_settings_set_value(settings, key_name, 
                           g_variant_new("(iiii)", win->x, win->y, win->width, win->height));
        g_free(key_name);
    }
}

// Create all category windows (stub implementation)
void create_category_windows() {
    if (!windows) {
        windows = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }
    // Example: create a default window; real categories can be added later
    GtkWidget *win = create_category_window("Default", 50, 50);
    gtk_widget_show_all(win);

    CategoryWindow *cw = g_new0(CategoryWindow, 1);
    cw->window = win;
    cw->flowbox = g_object_get_data(G_OBJECT(win), "flowbox");
    cw->x = 50; cw->y = 50; cw->width = 300; cw->height = 400;
    cw->category = g_strdup("Default");
    cw->visible = TRUE;
    g_hash_table_insert(windows, g_strdup(cw->category), cw);
}

// Toggle visibility for a category window (stub implementation)
void toggle_category_visibility(const gchar *category, gboolean visible) {
    if (!windows || !category) return;
    CategoryWindow *cw = g_hash_table_lookup(windows, category);
    if (!cw) return;
    cw->visible = visible;
    if (visible) {
        gtk_widget_show_all(cw->window);
    } else {
        gtk_widget_hide(cw->window);
    }
}

gboolean are_any_category_windows_visible() {
    if (!windows) return FALSE;
    GHashTableIter iter; gpointer key, value;
    g_hash_table_iter_init(&iter, windows);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CategoryWindow *cw = (CategoryWindow*)value;
        if (cw->visible) return TRUE;
    }
    return FALSE;
}

void show_all_category_windows() {
    if (!windows) return;
    GHashTableIter iter; gpointer key, value;
    g_hash_table_iter_init(&iter, windows);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CategoryWindow *cw = (CategoryWindow*)value;
        cw->visible = TRUE;
        gtk_widget_show_all(cw->window);
    }
}

void hide_all_category_windows() {
    if (!windows) return;
    GHashTableIter iter; gpointer key, value;
    g_hash_table_iter_init(&iter, windows);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        CategoryWindow *cw = (CategoryWindow*)value;
        cw->visible = FALSE;
        gtk_widget_hide(cw->window);
    }
}

static const gchar* category_to_name(FileCategory c) {
    switch (c) {
        case CATEGORY_FOLDER: return "Folders";
        case CATEGORY_APPLICATION: return "Applications";
        case CATEGORY_MUSIC: return "Music";
        case CATEGORY_VIDEO: return "Video";
        case CATEGORY_DOCUMENT: return "Documents";
        case CATEGORY_IMAGE: return "Images";
        case CATEGORY_ARCHIVE: return "Archives";
        case CATEGORY_CUSTOM: return "Custom";
        default: return "Other";
    }
}

static CategoryWindow* ensure_window_for_category(const gchar *name) {
    if (!windows) {
        windows = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    }
    CategoryWindow *cw = g_hash_table_lookup(windows, name);
    if (cw) return cw;

    GtkWidget *win = create_category_window(name, 50, 50);
    GtkWidget *flowbox = g_object_get_data(G_OBJECT(win), "flowbox");

    cw = g_new0(CategoryWindow, 1);
    cw->window = win;
    cw->flowbox = flowbox;
    cw->header_label = NULL;
    cw->x = 50; cw->y = 50; cw->width = 300; cw->height = 400;
    cw->category = g_strdup(name);
    cw->display_name = g_strdup(name);
    cw->visible = TRUE;
    g_hash_table_insert(windows, g_strdup(cw->category), cw);
    gtk_widget_show_all(win);
    return cw;
}

static void clear_flowbox_children(GtkWidget *flowbox) {
    if (!flowbox) return;
    GList *children = gtk_container_get_children(GTK_CONTAINER(flowbox));
    for (GList *c = children; c; c = c->next) {
        gtk_widget_destroy(GTK_WIDGET(c->data));
    }
    g_list_free(children);
}

void update_category_windows_from_list(GList *files) {
    // Clear previous icons to avoid stale/duplicate items
    if (windows) {
        GHashTableIter iter; gpointer key, value;
        g_hash_table_iter_init(&iter, windows);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            CategoryWindow *cw_any = (CategoryWindow*)value;
            clear_flowbox_children(cw_any->flowbox);
        }
    }

    // Track which categories have items; show only those
    GHashTable *has_items = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    for (GList *it = files; it; it = it->next) {
        DesktopFile *df = (DesktopFile*)it->data;
        const gchar *name = category_to_name(df->category);
        g_hash_table_insert(has_items, g_strdup(name), GINT_TO_POINTER(1));
        CategoryWindow *cw = ensure_window_for_category(name);
        if (cw && cw->flowbox) {
            GtkWidget *icon = create_file_icon(df);
            gtk_flow_box_insert(GTK_FLOW_BOX(cw->flowbox), icon, -1);
        }
    }

    // Toggle visibility based on presence
    if (windows) {
        GHashTableIter iter; gpointer key, value;
        g_hash_table_iter_init(&iter, windows);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            const gchar *name = (const gchar*)key;
            gboolean present = g_hash_table_contains(has_items, name);
            toggle_category_visibility(name, present);
        }
    }
    g_hash_table_destroy(has_items);
}

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <json-glib/json-glib.h>

// 结构体定义
typedef struct {
    int battery_brightness;
    int ac_brightness;
    int performance_brightness;
    int power_save_brightness;
    gboolean auto_adjust;
    gboolean use_sensor;
    gboolean time_based_adjust;
    int update_interval;
} AppConfig;

typedef struct {
    gboolean enabled;
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;
    int brightness;
    gboolean recurring;
} BrightnessSchedule;

// 全局变量定义
AppIndicator *indicator;
GtkWidget *brightness_window;
GtkWidget *settings_window;
GtkAdjustment *brightness_adj;
AppConfig app_config;
GArray *schedules;
gboolean brightness_window_visible = FALSE;

// 函数声明
void create_tray_icon(void);
GtkMenu* create_tray_menu(void);
void on_menu_quit_activated(GtkMenuItem *item, gpointer user_data);
void on_menu_brightness_activated(GtkMenuItem *item, gpointer user_data);
void on_menu_auto_brightness_activated(GtkMenuItem *item, gpointer user_data);
void on_menu_settings_activated(GtkMenuItem *item, gpointer user_data);
void create_brightness_window(void);
void position_brightness_window(void);
void on_brightness_changed(GtkAdjustment *adj, gpointer data);
void create_settings_window(void);
void save_config(void);
void load_config(void);
int get_current_brightness(void);
gboolean update_brightness_display(gpointer data);
gboolean is_ac_connected(void);
char* get_power_profile(void);
void apply_auto_brightness(void);
void check_scheduled_brightness(void);

// 配置文件路径
const gchar* get_config_path(void) {
    static gchar config_path[256];
    const gchar *home_dir = g_get_home_dir();
    snprintf(config_path, sizeof(config_path), "%s/.config/brightness-control/config.json", home_dir);
    return config_path;
}

// 确保配置文件目录存在
void ensure_config_dir(void) {
    const gchar *home_dir = g_get_home_dir();
    gchar config_dir[256];
    snprintf(config_dir, sizeof(config_dir), "%s/.config/brightness-control", home_dir);
    
    if (g_mkdir_with_parents(config_dir, 0755) == -1) {
        g_warning("无法创建配置目录: %s", config_dir);
    }
}

// 加载配置
void load_config(void) {
    // 默认配置
    app_config.battery_brightness = 30;
    app_config.ac_brightness = 70;
    app_config.performance_brightness = 80;
    app_config.power_save_brightness = 40;
    app_config.auto_adjust = TRUE;
    app_config.use_sensor = FALSE;
    app_config.time_based_adjust = TRUE;
    app_config.update_interval = 5;
    
    const gchar *config_file = get_config_path();
    if (!g_file_test(config_file, G_FILE_TEST_EXISTS)) {
        return;
    }
    
    GError *error = NULL;
    JsonParser *parser = json_parser_new();
    
    if (json_parser_load_from_file(parser, config_file, &error)) {
        JsonNode *root = json_parser_get_root(parser);
        JsonObject *root_obj = json_node_get_object(root);
        
        if (json_object_has_member(root_obj, "presets")) {
            JsonObject *presets = json_object_get_object_member(root_obj, "presets");
            app_config.battery_brightness = json_object_get_int_member(presets, "battery_brightness");
            app_config.ac_brightness = json_object_get_int_member(presets, "ac_brightness");
            app_config.performance_brightness = json_object_get_int_member(presets, "performance_brightness");
            app_config.power_save_brightness = json_object_get_int_member(presets, "power_save_brightness");
        }
        
        if (json_object_has_member(root_obj, "auto_adjust")) {
            app_config.auto_adjust = json_object_get_boolean_member(root_obj, "auto_adjust");
        }
        
        if (json_object_has_member(root_obj, "use_sensor")) {
            app_config.use_sensor = json_object_get_boolean_member(root_obj, "use_sensor");
        }
        
        if (json_object_has_member(root_obj, "time_based_adjust")) {
            app_config.time_based_adjust = json_object_get_boolean_member(root_obj, "time_based_adjust");
        }
        
        if (json_object_has_member(root_obj, "update_interval")) {
            app_config.update_interval = json_object_get_int_member(root_obj, "update_interval");
        }
    } else {
        g_warning("加载配置文件失败: %s", error->message);
        g_error_free(error);
    }
    
    g_object_unref(parser);
}

// 保存配置
void save_config(void) {
    ensure_config_dir();
    
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    
    // 保存预设值
    json_builder_set_member_name(builder, "presets");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "battery_brightness");
    json_builder_add_int_value(builder, app_config.battery_brightness);
    json_builder_set_member_name(builder, "ac_brightness");
    json_builder_add_int_value(builder, app_config.ac_brightness);
    json_builder_set_member_name(builder, "performance_brightness");
    json_builder_add_int_value(builder, app_config.performance_brightness);
    json_builder_set_member_name(builder, "power_save_brightness");
    json_builder_add_int_value(builder, app_config.power_save_brightness);
    json_builder_end_object(builder);
    
    // 保存其他设置
    json_builder_set_member_name(builder, "auto_adjust");
    json_builder_add_boolean_value(builder, app_config.auto_adjust);
    json_builder_set_member_name(builder, "use_sensor");
    json_builder_add_boolean_value(builder, app_config.use_sensor);
    json_builder_set_member_name(builder, "time_based_adjust");
    json_builder_add_boolean_value(builder, app_config.time_based_adjust);
    json_builder_set_member_name(builder, "update_interval");
    json_builder_add_int_value(builder, app_config.update_interval);
    
    json_builder_end_object(builder);
    
    JsonNode *root = json_builder_get_root(builder);
    JsonGenerator *generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);
    
    gchar *data = json_generator_to_data(generator, NULL);
    const gchar *config_file = get_config_path();
    
    GError *error = NULL;
    if (!g_file_set_contents(config_file, data, -1, &error)) {
        g_warning("保存配置文件失败: %s", error->message);
        g_error_free(error);
    }
    
    g_free(data);
    g_object_unref(generator);
    g_object_unref(builder);
    json_node_free(root);
}

// 菜单项回调函数
void on_menu_quit_activated(GtkMenuItem *item, gpointer user_data) {
    save_config();
    gtk_main_quit();
}

void on_menu_brightness_activated(GtkMenuItem *item, gpointer user_data) {
    if (brightness_window_visible) {
        gtk_widget_hide(brightness_window);
        brightness_window_visible = FALSE;
    } else {
        if (!brightness_window) {
            create_brightness_window();
        }
        
        position_brightness_window();
        gtk_widget_show_all(brightness_window);
        brightness_window_visible = TRUE;
        
        // 更新当前亮度值
        int current_brightness = get_current_brightness();
        g_signal_handlers_block_by_func(brightness_adj, on_brightness_changed, NULL);
        gtk_adjustment_set_value(brightness_adj, current_brightness);
        g_signal_handlers_unblock_by_func(brightness_adj, on_brightness_changed, NULL);
    }
}

void on_menu_auto_brightness_activated(GtkMenuItem *item, gpointer user_data) {
    apply_auto_brightness();
}

void on_menu_settings_activated(GtkMenuItem *item, gpointer user_data) {
    create_settings_window();
}

// 创建托盘菜单
GtkMenu* create_tray_menu(void) {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *brightness_item = gtk_menu_item_new_with_label("调整亮度");
    GtkWidget *auto_item = gtk_menu_item_new_with_label("自动亮度");
    GtkWidget *settings_item = gtk_menu_item_new_with_label("设置");
    GtkWidget *separator = gtk_separator_menu_item_new();
    GtkWidget *quit_item = gtk_menu_item_new_with_label("退出");

    // 连接菜单项的信号
    g_signal_connect(brightness_item, "activate", G_CALLBACK(on_menu_brightness_activated), NULL);
    g_signal_connect(auto_item, "activate", G_CALLBACK(on_menu_auto_brightness_activated), NULL);
    g_signal_connect(settings_item, "activate", G_CALLBACK(on_menu_settings_activated), NULL);
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_menu_quit_activated), NULL);

    // 将菜单项添加到菜单中
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), brightness_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), auto_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), settings_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

    // 显示所有菜单项
    gtk_widget_show_all(menu);

    return GTK_MENU(menu);
}

// 创建托盘图标
void create_tray_icon(void) {
    indicator = app_indicator_new(
        "brightness-control-app",
        "display-brightness-symbolic",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS
    );
    
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_title(indicator, "亮度控制");
    
    // 创建并设置菜单
    GtkMenu *menu = create_tray_menu();
    app_indicator_set_menu(indicator, menu);
}

// 定位亮度窗口（根据托盘位置）
void position_brightness_window(void) {
    if (!brightness_window) return;
    
    // 获取屏幕尺寸
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (!monitor) {
        // 如果没有主显示器，使用第一个显示器
        monitor = gdk_display_get_monitor(display, 0);
    }
    
    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);
    
    // 获取鼠标位置作为托盘图标的近似位置
    GdkSeat *seat = gdk_display_get_default_seat(display);
    GdkDevice *device = gdk_seat_get_pointer(seat);
    gint x, y;
    gdk_device_get_position(device, NULL, &x, &y);
    
    // 计算窗口尺寸
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(brightness_window), &width, &height);
    
    // 定位窗口（避免遮挡视觉中心）
    gint pos_x, pos_y;
    
    // 如果鼠标在屏幕上半部分，窗口显示在下方；否则显示在上方
    if (y < geometry.height / 2) {
        pos_y = y + 20; // 在鼠标下方
    } else {
        pos_y = y - height - 20; // 在鼠标上方
    }
    
    // 水平居中或靠近鼠标位置
    pos_x = x - width / 2;
    
    // 确保窗口在屏幕内
    if (pos_x < geometry.x) pos_x = geometry.x;
    if (pos_x + width > geometry.x + geometry.width) pos_x = geometry.x + geometry.width - width;
    if (pos_y < geometry.y) pos_y = geometry.y;
    if (pos_y + height > geometry.y + geometry.height) pos_y = geometry.y + geometry.height - height;
    
    gtk_window_move(GTK_WINDOW(brightness_window), pos_x, pos_y);
}

// 创建亮度调节窗口
void create_brightness_window(void) {
    brightness_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(brightness_window), "亮度控制");
    gtk_window_set_default_size(GTK_WINDOW(brightness_window), 320, 120);
    gtk_window_set_resizable(GTK_WINDOW(brightness_window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(brightness_window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(brightness_window), TRUE);
    gtk_window_set_deletable(GTK_WINDOW(brightness_window), FALSE);
    
    // 设置窗口类型为工具窗口
    gtk_window_set_type_hint(GTK_WINDOW(brightness_window), GDK_WINDOW_TYPE_HINT_DIALOG);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(brightness_window), box);
    
    // 创建滑动条
    brightness_adj = gtk_adjustment_new(50, 0, 100, 1, 10, 0);
    GtkWidget *scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, brightness_adj);
    gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
    gtk_scale_set_has_origin(GTK_SCALE(scale), TRUE);
    
    // 连接值改变信号
    g_signal_connect(G_OBJECT(brightness_adj), "value-changed", 
                     G_CALLBACK(on_brightness_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(box), scale, TRUE, TRUE, 10);
    
    // 添加按钮行
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(button_box), 5);
    
    // 设置按钮
    GtkWidget *settings_button = gtk_button_new_from_icon_name("preferences-system-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(settings_button, "打开设置");
    g_signal_connect(settings_button, "clicked", G_CALLBACK(on_menu_settings_activated), NULL);
    
    // 关闭按钮
    GtkWidget *close_button = gtk_button_new_with_label("关闭");
    g_signal_connect_swapped(close_button, "clicked", 
                           G_CALLBACK(gtk_widget_hide), brightness_window);
    
    gtk_container_add(GTK_CONTAINER(button_box), settings_button);
    gtk_container_add(GTK_CONTAINER(button_box), close_button);
    gtk_box_pack_start(GTK_BOX(box), button_box, FALSE, FALSE, 5);
    
    // 连接窗口隐藏事件
    g_signal_connect(brightness_window, "hide", 
                     G_CALLBACK(gtk_widget_hide), brightness_window);
}

// 设置窗口的回调函数
void on_auto_adjust_toggled(GtkToggleButton *button, gpointer user_data) {
    app_config.auto_adjust = gtk_toggle_button_get_active(button);
}

void on_use_sensor_toggled(GtkToggleButton *button, gpointer user_data) {
    app_config.use_sensor = gtk_toggle_button_get_active(button);
}

void on_time_based_toggled(GtkToggleButton *button, gpointer user_data) {
    app_config.time_based_adjust = gtk_toggle_button_get_active(button);
}

void on_battery_brightness_changed(GtkRange *range, gpointer user_data) {
    app_config.battery_brightness = (int)gtk_range_get_value(range);
}

void on_ac_brightness_changed(GtkRange *range, gpointer user_data) {
    app_config.ac_brightness = (int)gtk_range_get_value(range);
}

void on_performance_brightness_changed(GtkRange *range, gpointer user_data) {
    app_config.performance_brightness = (int)gtk_range_get_value(range);
}

void on_power_save_brightness_changed(GtkRange *range, gpointer user_data) {
    app_config.power_save_brightness = (int)gtk_range_get_value(range);
}

void on_save_settings_clicked(GtkButton *button, gpointer user_data) {
    save_config();
    if (settings_window) {
        gtk_widget_hide(settings_window);
    }
}

// 创建设置窗口
void create_settings_window(void) {
    if (settings_window) {
        gtk_window_present(GTK_WINDOW(settings_window));
        return;
    }
    
    settings_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(settings_window), "亮度控制设置");
    gtk_window_set_default_size(GTK_WINDOW(settings_window), 400, 500);
    gtk_window_set_resizable(GTK_WINDOW(settings_window), FALSE);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(settings_window), box);
    
    // 创建笔记本（选项卡）
    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 10);
    
    // 基本设置选项卡
    GtkWidget *basic_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(basic_page), 10);
    
    // 自动调整设置
    GtkWidget *auto_adjust_check = gtk_check_button_new_with_label("启用自动亮度调整");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_adjust_check), app_config.auto_adjust);
    g_signal_connect(auto_adjust_check, "toggled", G_CALLBACK(on_auto_adjust_toggled), NULL);
    gtk_box_pack_start(GTK_BOX(basic_page), auto_adjust_check, FALSE, FALSE, 5);
    
    GtkWidget *sensor_check = gtk_check_button_new_with_label("使用亮度传感器（如果可用）");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sensor_check), app_config.use_sensor);
    g_signal_connect(sensor_check, "toggled", G_CALLBACK(on_use_sensor_toggled), NULL);
    gtk_box_pack_start(GTK_BOX(basic_page), sensor_check, FALSE, FALSE, 5);
    
    GtkWidget *time_based_check = gtk_check_button_new_with_label("启用基于时间的亮度调整");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(time_based_check), app_config.time_based_adjust);
    g_signal_connect(time_based_check, "toggled", G_CALLBACK(on_time_based_toggled), NULL);
    gtk_box_pack_start(GTK_BOX(basic_page), time_based_check, FALSE, FALSE, 5);
    
    // 添加选项卡
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), basic_page, gtk_label_new("基本设置"));
    
    // 亮度预设选项卡
    GtkWidget *presets_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(presets_page), 10);
    
    // 电池模式亮度
    GtkWidget *battery_frame = gtk_frame_new("电池模式亮度");
    GtkWidget *battery_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(battery_frame), battery_box);
    
    GtkWidget *battery_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
    gtk_range_set_value(GTK_RANGE(battery_scale), app_config.battery_brightness);
    g_signal_connect(battery_scale, "value-changed", G_CALLBACK(on_battery_brightness_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(battery_box), battery_scale, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(presets_page), battery_frame, FALSE, FALSE, 5);
    
    // 电源模式亮度
    GtkWidget *ac_frame = gtk_frame_new("电源模式亮度");
    GtkWidget *ac_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(ac_frame), ac_box);
    
    GtkWidget *ac_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
    gtk_range_set_value(GTK_RANGE(ac_scale), app_config.ac_brightness);
    g_signal_connect(ac_scale, "value-changed", G_CALLBACK(on_ac_brightness_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(ac_box), ac_scale, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(presets_page), ac_frame, FALSE, FALSE, 5);
    
    // 性能模式亮度
    GtkWidget *performance_frame = gtk_frame_new("性能模式亮度");
    GtkWidget *performance_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(performance_frame), performance_box);
    
    GtkWidget *performance_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
    gtk_range_set_value(GTK_RANGE(performance_scale), app_config.performance_brightness);
    g_signal_connect(performance_scale, "value-changed", G_CALLBACK(on_performance_brightness_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(performance_box), performance_scale, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(presets_page), performance_frame, FALSE, FALSE, 5);
    
    // 节能模式亮度
    GtkWidget *power_save_frame = gtk_frame_new("节能模式亮度");
    GtkWidget *power_save_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(power_save_frame), power_save_box);
    
    GtkWidget *power_save_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
    gtk_range_set_value(GTK_RANGE(power_save_scale), app_config.power_save_brightness);
    g_signal_connect(power_save_scale, "value-changed", G_CALLBACK(on_power_save_brightness_changed), NULL);
    
    gtk_box_pack_start(GTK_BOX(power_save_box), power_save_scale, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(presets_page), power_save_frame, FALSE, FALSE, 5);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), presets_page, gtk_label_new("亮度预设"));
    
    // 保存按钮
    GtkWidget *save_button = gtk_button_new_with_label("保存设置");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_settings_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), save_button, FALSE, FALSE, 5);
    
    gtk_widget_show_all(settings_window);
}

// 亮度值改变回调
void on_brightness_changed(GtkAdjustment *adj, gpointer data) {
    int brightness_value = (int)gtk_adjustment_get_value(adj);
    char command[256];
    snprintf(command, sizeof(command), "brightnessctl set %d%%", brightness_value);
    int result = system(command);
    if (result != 0) {
        g_warning("亮度设置命令执行失败: %s", command);
    }
}

// 从brightnessctl获取当前亮度值
int get_current_brightness(void) {
    FILE *fp;
    char path[1035];
    int brightness = 50; // 默认值
    
    // 执行brightnessctl get命令
    fp = popen("brightnessctl get", "r");
    if (fp == NULL) {
        g_warning("无法执行brightnessctl命令");
        return brightness;
    }
    
    if (fgets(path, sizeof(path), fp) != NULL) {
        brightness = atoi(path);
        // 转换为百分比（假设brightnessctl返回的是绝对值）
        if (brightness > 100) {
            // 假设最大值是255或类似，转换为百分比
            brightness = (brightness * 100) / 255;
        }
    }
    
    pclose(fp);
    return brightness;
}

// 定期更新滑动条值
gboolean update_brightness_display(gpointer data) {
    if (!brightness_window || !brightness_window_visible) {
        return TRUE; // 窗口未显示时跳过更新
    }
    
    int current_brightness = get_current_brightness();
    // 避免递归触发value-changed信号
    g_signal_handlers_block_by_func(brightness_adj, on_brightness_changed, NULL);
    gtk_adjustment_set_value(brightness_adj, current_brightness);
    g_signal_handlers_unblock_by_func(brightness_adj, on_brightness_changed, NULL);
    return TRUE; // 保持定时器运行
}

// 检测电源连接状态
gboolean is_ac_connected(void) {
    // 检查AC电源状态（通过sysfs）
    FILE *fp = fopen("/sys/class/power_supply/AC/online", "r");
    if (fp) {
        int ac_status;
        fscanf(fp, "%d", &ac_status);
        fclose(fp);
        return ac_status > 0;
    }
    
    // 备用检查方法
    fp = fopen("/sys/class/power_supply/ACAD/online", "r");
    if (fp) {
        int ac_status;
        fscanf(fp, "%d", &ac_status);
        fclose(fp);
        return ac_status > 0;
    }
    
    return TRUE; // 默认假设已连接
}

// 检测当前性能模式
char* get_power_profile(void) {
    // 适用于Sway环境的电源模式检测 
    FILE *fp = popen("swaymsg -t get_outputs 2>/dev/null | grep -o '\"power_profile\":\"[^\"]*\"' | cut -d'\"' -f4", "r");
    if (fp) {
        static char profile[32];
        if (fgets(profile, sizeof(profile), fp) != NULL) {
            // 移除换行符
            profile[strcspn(profile, "\n")] = 0;
            pclose(fp);
            return profile;
        }
        pclose(fp);
    }
    return "balanced"; // 默认平衡模式
}

// 根据电源状态自动调整亮度
void apply_auto_brightness(void) {
    if (!app_config.auto_adjust) {
        return;
    }
    
    gboolean ac_connected = is_ac_connected();
    char *power_profile = get_power_profile();
    
    int target_brightness;
    
    if (strcmp(power_profile, "performance") == 0) {
        target_brightness = app_config.performance_brightness;
    } else if (strcmp(power_profile, "power-save") == 0) {
        target_brightness = app_config.power_save_brightness;
    } else {
        target_brightness = ac_connected ? app_config.ac_brightness : app_config.battery_brightness;
    }
    
    char command[256];
    snprintf(command, sizeof(command), "brightnessctl set %d%%", target_brightness);
    int result = system(command);
    if (result != 0) {
        g_warning("自动亮度设置失败");
    }
}

// 检查并应用定时亮度设置
void check_scheduled_brightness(void) {
    if (!app_config.time_based_adjust || schedules->len == 0) {
        return;
    }
    
    GDateTime *now = g_date_time_new_now_local();
    int current_hour = g_date_time_get_hour(now);
    int current_minute = g_date_time_get_minute(now);
    
    guint i;
    for (i = 0; i < schedules->len; i++) {
        BrightnessSchedule *schedule = &g_array_index(schedules, BrightnessSchedule, i);
        if (schedule->enabled) {
            int current_total_minutes = current_hour * 60 + current_minute;
            int start_total_minutes = schedule->start_hour * 60 + schedule->start_minute;
            int end_total_minutes = schedule->end_hour * 60 + schedule->end_minute;
            
            if ((start_total_minutes <= current_total_minutes && current_total_minutes <= end_total_minutes) ||
                (start_total_minutes > end_total_minutes && 
                 (current_total_minutes >= start_total_minutes || current_total_minutes <= end_total_minutes))) {
                // 在定时范围内，应用预设亮度
                char command[256];
                snprintf(command, sizeof(command), "brightnessctl set %d%%", schedule->brightness);
                system(command);
                break;
            }
        }
    }
    
    g_date_time_unref(now);
}

// 主函数
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // 初始化配置
    load_config();
    
    // 初始化定时计划数组
    schedules = g_array_new(FALSE, FALSE, sizeof(BrightnessSchedule));
    
    create_tray_icon();
    
    // 初始化亮度值
    int current_brightness = get_current_brightness();
    printf("亮度控制程序已启动，当前亮度: %d%%\n", current_brightness);
    
    // 设置定时器，定期更新亮度显示
    g_timeout_add_seconds(app_config.update_interval, update_brightness_display, NULL);
    
    // 设置定时器，定期检查电源状态和定时计划
    g_timeout_add_seconds(30, (GSourceFunc)apply_auto_brightness, NULL);
    g_timeout_add_seconds(60, (GSourceFunc)check_scheduled_brightness, NULL);
    
    gtk_main();
    
    // 清理资源
    g_array_free(schedules, TRUE);
    
    return 0;
}

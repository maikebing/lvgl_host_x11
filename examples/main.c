#include "lvgl.h"
#include "lvgl_host_x11.h"

#if defined(LV_VERSION_MAJOR) && (LV_VERSION_MAJOR < 9)
#define lv_button_create(parent) lv_btn_create(parent)
#define lv_screen_active() lv_scr_act()
#endif

int main(void)
{
    lvgl_host_x11_t host;

    if (lvgl_host_x11_init(&host, 800, 480, "LVGL X11 Host Example") != 0) {
        return 1;
    }

    lv_obj_t *btn = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn, 220, 80);
    lv_obj_center(btn);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Hello LVGL + X11");
    lv_obj_center(label);

    while (lvgl_host_x11_is_running(&host)) {
        lvgl_host_x11_poll(&host);
        lvgl_host_x11_present(&host);
    }

    lvgl_host_x11_shutdown(&host);
    return 0;
}

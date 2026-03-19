#ifndef LVGL_HOST_X11_H
#define LVGL_HOST_X11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdint.h>

#if defined(LV_VERSION_MAJOR) && (LV_VERSION_MAJOR < 9)
typedef lv_disp_t lv_display_t;
#endif

typedef struct lvgl_host_x11 {
    int width;
    int height;
    int running;
    void *user_data;
} lvgl_host_x11_t;

int lvgl_host_x11_init(lvgl_host_x11_t *host, int width, int height, const char *title);
void lvgl_host_x11_poll(lvgl_host_x11_t *host);
void lvgl_host_x11_present(lvgl_host_x11_t *host);
void lvgl_host_x11_shutdown(lvgl_host_x11_t *host);
int lvgl_host_x11_is_running(lvgl_host_x11_t *host);

#ifdef __cplusplus
}
#endif

#endif

#include "lvgl_host_x11.h"
#include "lvgl.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#if !defined(LV_COLOR_DEPTH)
#error "lvgl_host_x11 requires LV_COLOR_DEPTH to be defined"
#endif

#if (LV_COLOR_DEPTH != 16) && (LV_COLOR_DEPTH != 32)
#error "lvgl_host_x11 supports LV_COLOR_DEPTH 16 or 32"
#endif

#if defined(LV_VERSION_MAJOR) && (LV_VERSION_MAJOR == 8)
#define LVGL_HOST_X11_LVGL_V8 1
#else
#define LVGL_HOST_X11_LVGL_V8 0
#endif

#if LVGL_HOST_X11_LVGL_V8
typedef lv_disp_t lv_display_t;
#endif

typedef struct host_internal {
    Display *display;
    int screen;
    Window window;
    GC gc;
    Atom wm_delete_window;
    XImage *ximage;
    uint32_t *framebuffer;
    int width;
    int height;
    int mouse_x;
    int mouse_y;
    int mouse_pressed;
    uint32_t last_key;
    int key_pressed;
    int16_t wheel_diff;
    lv_display_t *lv_display;
    lv_indev_t *mouse_indev;
    lv_indev_t *keyboard_indev;
    lv_indev_t *wheel_indev;
#if LVGL_HOST_X11_LVGL_V8
    lv_disp_draw_buf_t draw_buf;
    lv_disp_drv_t disp_drv;
    lv_indev_drv_t mouse_drv;
    lv_indev_drv_t keyboard_drv;
    lv_indev_drv_t wheel_drv;
#endif
    uint32_t *draw_buf_1;
    size_t draw_buf_pixels;
} host_internal_t;

static host_internal_t *g_host = NULL;

static uint64_t tick_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
}

static inline uint32_t lv_color16_to_x11(uint16_t pixel)
{
    uint32_t r = (uint32_t)((pixel >> 11) & 0x1F);
    uint32_t g = (uint32_t)((pixel >> 5) & 0x3F);
    uint32_t b = (uint32_t)(pixel & 0x1F);

    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return r | (g << 8) | (b << 16);
}

#if LVGL_HOST_X11_LVGL_V8
static void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *px_map)
#else
static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
#endif
{
    (void)px_map;

#if LVGL_HOST_X11_LVGL_V8
    (void)disp_drv;
#else
    (void)disp;
#endif

    if (!g_host || !g_host->framebuffer) {
#if LVGL_HOST_X11_LVGL_V8
        lv_disp_flush_ready(disp_drv);
#else
        lv_display_flush_ready(disp);
#endif
        return;
    }

    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    for (int32_t y = 0; y < h; y++) {
        uint32_t *dst = g_host->framebuffer + (area->y1 + y) * g_host->width + area->x1;
#if LV_COLOR_DEPTH == 32
        const uint32_t *src = ((const uint32_t *)px_map) + y * w;
        memcpy(dst, src, (size_t)w * sizeof(uint32_t));
#else
        const uint16_t *src = ((const uint16_t *)px_map) + y * w;
        for (int32_t x = 0; x < w; x++) {
            dst[x] = lv_color16_to_x11(src[x]);
        }
#endif
    }

#if LVGL_HOST_X11_LVGL_V8
    lv_disp_flush_ready(disp_drv);
#else
    lv_display_flush_ready(disp);
#endif
}

#if LVGL_HOST_X11_LVGL_V8
static bool mouse_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
#else
static void mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
#endif
{
#if LVGL_HOST_X11_LVGL_V8
    (void)indev_drv;
#else
    (void)indev;
#endif

    if (!g_host) {
        data->point.x = 0;
        data->point.y = 0;
        data->state = LV_INDEV_STATE_RELEASED;
#if LVGL_HOST_X11_LVGL_V8
        return false;
#else
        return;
#endif
    }

    data->point.x = g_host->mouse_x;
    data->point.y = g_host->mouse_y;
    data->state = g_host->mouse_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

#if LVGL_HOST_X11_LVGL_V8
    return false;
#endif
}

#if LVGL_HOST_X11_LVGL_V8
static bool keyboard_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
#else
static void keyboard_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
#endif
{
#if LVGL_HOST_X11_LVGL_V8
    (void)indev_drv;
#else
    (void)indev;
#endif

    data->key = 0;
    data->state = LV_INDEV_STATE_RELEASED;

    if (!g_host) {
#if LVGL_HOST_X11_LVGL_V8
        return false;
#else
        return;
#endif
    }

    data->key = g_host->last_key;
    data->state = g_host->key_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

#if LVGL_HOST_X11_LVGL_V8
    return false;
#endif
}

#if LVGL_HOST_X11_LVGL_V8
static bool wheel_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
#else
static void wheel_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
#endif
{
#if LVGL_HOST_X11_LVGL_V8
    (void)indev_drv;
#else
    (void)indev;
#endif

    data->enc_diff = 0;
    data->state = LV_INDEV_STATE_RELEASED;

    if (!g_host) {
#if LVGL_HOST_X11_LVGL_V8
        return false;
#else
        return;
#endif
    }

    data->enc_diff = g_host->wheel_diff;
    g_host->wheel_diff = 0;

#if LVGL_HOST_X11_LVGL_V8
    return false;
#endif
}

static uint32_t keysym_to_lv_key(KeySym keysym)
{
    switch (keysym) {
    case XK_Return:
    case XK_KP_Enter:
        return LV_KEY_ENTER;
    case XK_Escape:
        return LV_KEY_ESC;
    case XK_BackSpace:
        return LV_KEY_BACKSPACE;
    case XK_Tab:
        return LV_KEY_NEXT;
    case XK_ISO_Left_Tab:
        return LV_KEY_PREV;
    case XK_Home:
        return LV_KEY_HOME;
    case XK_End:
        return LV_KEY_END;
    case XK_Delete:
        return LV_KEY_DEL;
    case XK_Left:
        return LV_KEY_LEFT;
    case XK_Right:
        return LV_KEY_RIGHT;
    case XK_Up:
        return LV_KEY_UP;
    case XK_Down:
        return LV_KEY_DOWN;
    default:
        if (keysym >= XK_space && keysym <= XK_asciitilde) {
            return (uint32_t)keysym;
        }
        return 0;
    }
}

int lvgl_host_x11_init(lvgl_host_x11_t *host, int width, int height, const char *title)
{
    if (!host || width <= 0 || height <= 0) {
        return -1;
    }

    memset(host, 0, sizeof(*host));

    host_internal_t *internal = (host_internal_t *)calloc(1, sizeof(host_internal_t));
    if (!internal) {
        return -1;
    }

    internal->width = width;
    internal->height = height;

    internal->display = XOpenDisplay(NULL);
    if (!internal->display) {
        free(internal);
        return -1;
    }

    internal->screen = DefaultScreen(internal->display);
    internal->window = XCreateSimpleWindow(
        internal->display,
        RootWindow(internal->display, internal->screen),
        10, 10,
        (unsigned int)width,
        (unsigned int)height,
        1,
        BlackPixel(internal->display, internal->screen),
        WhitePixel(internal->display, internal->screen));

    internal->wm_delete_window = XInternAtom(internal->display, "WM_DELETE_WINDOW", False);
    XStoreName(internal->display, internal->window, title ? title : "LVGL X11 Host");
    XSetWMProtocols(internal->display, internal->window, &internal->wm_delete_window, 1);
    XSelectInput(internal->display, internal->window,
                 ExposureMask |
                 ButtonPressMask |
                 ButtonReleaseMask |
                 PointerMotionMask |
                 KeyPressMask |
                 KeyReleaseMask |
                 StructureNotifyMask);
    XMapWindow(internal->display, internal->window);

    internal->gc = DefaultGC(internal->display, internal->screen);
    internal->framebuffer = (uint32_t *)calloc((size_t)width * (size_t)height, sizeof(uint32_t));
    if (!internal->framebuffer) {
        XDestroyWindow(internal->display, internal->window);
        XCloseDisplay(internal->display);
        free(internal);
        return -1;
    }

    Visual *visual = DefaultVisual(internal->display, internal->screen);
    int depth = DefaultDepth(internal->display, internal->screen);

    internal->ximage = XCreateImage(
        internal->display,
        visual,
        (unsigned int)depth,
        ZPixmap,
        0,
        (char *)internal->framebuffer,
        (unsigned int)width,
        (unsigned int)height,
        32,
        width * (int)sizeof(uint32_t));

    if (!internal->ximage) {
        free(internal->framebuffer);
        XDestroyWindow(internal->display, internal->window);
        XCloseDisplay(internal->display);
        free(internal);
        return -1;
    }

    internal->draw_buf_pixels = (size_t)width * 80U;
    internal->draw_buf_1 = (uint32_t *)malloc(internal->draw_buf_pixels * sizeof(uint32_t));
    if (!internal->draw_buf_1) {
        internal->ximage->data = NULL;
        XDestroyImage(internal->ximage);
        free(internal->framebuffer);
        XDestroyWindow(internal->display, internal->window);
        XCloseDisplay(internal->display);
        free(internal);
        return -1;
    }

    if (!lv_is_initialized()) {
        lv_init();
    }

#if LVGL_HOST_X11_LVGL_V8
    lv_disp_draw_buf_init(&internal->draw_buf, internal->draw_buf_1, NULL, (uint32_t)internal->draw_buf_pixels);

    lv_disp_drv_init(&internal->disp_drv);
    internal->disp_drv.hor_res = (lv_coord_t)width;
    internal->disp_drv.ver_res = (lv_coord_t)height;
    internal->disp_drv.draw_buf = &internal->draw_buf;
    internal->disp_drv.flush_cb = flush_cb;
    internal->lv_display = lv_disp_drv_register(&internal->disp_drv);
#else
    internal->lv_display = lv_display_create(width, height);
    lv_display_set_buffers(
        internal->lv_display,
        internal->draw_buf_1,
        NULL,
        internal->draw_buf_pixels * sizeof(uint32_t),
        LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(internal->lv_display, flush_cb);
#endif

#if LVGL_HOST_X11_LVGL_V8
    lv_indev_drv_init(&internal->mouse_drv);
    internal->mouse_drv.type = LV_INDEV_TYPE_POINTER;
    internal->mouse_drv.read_cb = mouse_read_cb;
    internal->mouse_indev = lv_indev_drv_register(&internal->mouse_drv);

    lv_indev_drv_init(&internal->keyboard_drv);
    internal->keyboard_drv.type = LV_INDEV_TYPE_KEYPAD;
    internal->keyboard_drv.read_cb = keyboard_read_cb;
    internal->keyboard_indev = lv_indev_drv_register(&internal->keyboard_drv);

    lv_indev_drv_init(&internal->wheel_drv);
    internal->wheel_drv.type = LV_INDEV_TYPE_ENCODER;
    internal->wheel_drv.read_cb = wheel_read_cb;
    internal->wheel_indev = lv_indev_drv_register(&internal->wheel_drv);
#else
    internal->mouse_indev = lv_indev_create();
    lv_indev_set_type(internal->mouse_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(internal->mouse_indev, mouse_read_cb);
    lv_indev_set_display(internal->mouse_indev, internal->lv_display);

    internal->keyboard_indev = lv_indev_create();
    lv_indev_set_type(internal->keyboard_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(internal->keyboard_indev, keyboard_read_cb);
    lv_indev_set_display(internal->keyboard_indev, internal->lv_display);

    internal->wheel_indev = lv_indev_create();
    lv_indev_set_type(internal->wheel_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(internal->wheel_indev, wheel_read_cb);
    lv_indev_set_display(internal->wheel_indev, internal->lv_display);
#endif

    host->width = width;
    host->height = height;
    host->running = 1;
    host->user_data = internal;

    g_host = internal;
    return 0;
}

void lvgl_host_x11_poll(lvgl_host_x11_t *host)
{
    if (!host || !host->user_data) {
        return;
    }

    host_internal_t *internal = (host_internal_t *)host->user_data;

    while (XPending(internal->display)) {
        XEvent ev;
        XNextEvent(internal->display, &ev);

        switch (ev.type) {
        case MotionNotify:
            internal->mouse_x = ev.xmotion.x;
            internal->mouse_y = ev.xmotion.y;
            break;
        case ButtonPress:
            if (ev.xbutton.button == Button1) {
                internal->mouse_pressed = 1;
                internal->mouse_x = ev.xbutton.x;
                internal->mouse_y = ev.xbutton.y;
            } else if (ev.xbutton.button == Button4) {
                internal->wheel_diff++;
            } else if (ev.xbutton.button == Button5) {
                internal->wheel_diff--;
            }
            break;
        case ButtonRelease:
            if (ev.xbutton.button == Button1) {
                internal->mouse_pressed = 0;
                internal->mouse_x = ev.xbutton.x;
                internal->mouse_y = ev.xbutton.y;
            }
            break;
        case KeyPress: {
            uint32_t key = keysym_to_lv_key(XLookupKeysym(&ev.xkey, 0));
            if (key != 0) {
                internal->last_key = key;
                internal->key_pressed = 1;
            }
            break;
        }
        case KeyRelease:
            internal->key_pressed = 0;
            break;
        case ClientMessage:
            if (ev.xclient.data.l[0] == (long)internal->wm_delete_window) {
                host->running = 0;
            }
            break;
        case DestroyNotify:
            host->running = 0;
            break;
        default:
            break;
        }
    }
}

void lvgl_host_x11_present(lvgl_host_x11_t *host)
{
    static uint64_t last_tick = 0;

    if (!host || !host->user_data) {
        return;
    }

    host_internal_t *internal = (host_internal_t *)host->user_data;

    uint64_t now = tick_ms();
    if (last_tick == 0) {
        last_tick = now;
    }

    uint32_t diff = (uint32_t)(now - last_tick);
    last_tick = now;

    lv_tick_inc(diff);
    lv_timer_handler();

    XPutImage(internal->display, internal->window, internal->gc, internal->ximage,
              0, 0, 0, 0, (unsigned int)host->width, (unsigned int)host->height);
    XFlush(internal->display);

    usleep(5000);
}

void lvgl_host_x11_shutdown(lvgl_host_x11_t *host)
{
    if (!host || !host->user_data) {
        return;
    }

    host_internal_t *internal = (host_internal_t *)host->user_data;

#if !LVGL_HOST_X11_LVGL_V8
    if (internal->mouse_indev) {
        lv_indev_delete(internal->mouse_indev);
    }
    if (internal->keyboard_indev) {
        lv_indev_delete(internal->keyboard_indev);
    }
    if (internal->wheel_indev) {
        lv_indev_delete(internal->wheel_indev);
    }
    if (internal->lv_display) {
        lv_display_delete(internal->lv_display);
    }
#endif

    if (internal->ximage) {
        internal->ximage->data = NULL;
        XDestroyImage(internal->ximage);
    }
    if (internal->framebuffer) {
        free(internal->framebuffer);
    }
    if (internal->draw_buf_1) {
        free(internal->draw_buf_1);
    }
    if (internal->window) {
        XDestroyWindow(internal->display, internal->window);
    }
    if (internal->display) {
        XCloseDisplay(internal->display);
    }

    if (g_host == internal) {
        g_host = NULL;
    }

    free(internal);
    host->user_data = NULL;
    host->running = 0;
}

int lvgl_host_x11_is_running(lvgl_host_x11_t *host)
{
    if (!host) {
        return 0;
    }
    return host->running;
}

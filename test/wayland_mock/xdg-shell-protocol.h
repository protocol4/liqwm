// Structural mock of what `wayland-scanner server-header` actually
// generates from xdg-shell.xml. Real field order matches the protocol's
// stable request list, unchanged across versions 1-7 for the requests
// used here.
#pragma once
#include "wayland-server-core.h"

extern "C" {

extern const struct wl_interface xdg_wm_base_interface;
extern const struct wl_interface xdg_positioner_interface;
extern const struct wl_interface xdg_surface_interface;
extern const struct wl_interface xdg_toplevel_interface;
extern const struct wl_interface xdg_popup_interface;

struct xdg_wm_base_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*create_positioner)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*get_xdg_surface)(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*);
    void (*pong)(struct wl_client*, struct wl_resource*, uint32_t);
};

struct xdg_surface_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*get_toplevel)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*get_popup)(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*, struct wl_resource*);
    void (*set_window_geometry)(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t);
    void (*ack_configure)(struct wl_client*, struct wl_resource*, uint32_t);
};

struct xdg_toplevel_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*set_parent)(struct wl_client*, struct wl_resource*, struct wl_resource*);
    void (*set_title)(struct wl_client*, struct wl_resource*, const char*);
    void (*set_app_id)(struct wl_client*, struct wl_resource*, const char*);
    void (*show_window_menu)(struct wl_client*, struct wl_resource*, struct wl_resource*, uint32_t, int32_t, int32_t);
    void (*move)(struct wl_client*, struct wl_resource*, struct wl_resource*, uint32_t);
    void (*resize)(struct wl_client*, struct wl_resource*, struct wl_resource*, uint32_t, uint32_t);
    void (*set_max_size)(struct wl_client*, struct wl_resource*, int32_t, int32_t);
    void (*set_min_size)(struct wl_client*, struct wl_resource*, int32_t, int32_t);
    void (*set_maximized)(struct wl_client*, struct wl_resource*);
    void (*unset_maximized)(struct wl_client*, struct wl_resource*);
    void (*set_fullscreen)(struct wl_client*, struct wl_resource*, struct wl_resource*);
    void (*unset_fullscreen)(struct wl_client*, struct wl_resource*);
    void (*set_minimized)(struct wl_client*, struct wl_resource*);
};

void xdg_surface_send_configure(struct wl_resource*, uint32_t);
void xdg_toplevel_send_configure(struct wl_resource*, int32_t, int32_t, struct wl_array*);

} // extern "C"

// Structural mock of the real <wayland-server-protocol.h> (the CORE
// protocol, generated from libwayland's bundled wayland.xml). The whole
// point of this file existing is to test the claim made in Surface.cpp:
// that a struct TAG (the dispatch table type) and an `extern const`
// VARIABLE of the same name can coexist, because C++ keeps a hidden
// class name reachable via `struct Name` even when an ordinary
// declaration of the same name is in scope. If this file's shape is
// wrong, Surface.cpp won't compile against it -- that's the test.
#pragma once
#include "wayland-server-core.h"

extern "C" {

extern const struct wl_interface wl_compositor_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_region_interface;
extern const struct wl_interface wl_callback_interface;

struct wl_compositor_interface {
    void (*create_surface)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*create_region)(struct wl_client*, struct wl_resource*, uint32_t);
};

struct wl_surface_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*attach)(struct wl_client*, struct wl_resource*, struct wl_resource*, int32_t, int32_t);
    void (*damage)(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t);
    void (*frame)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*set_opaque_region)(struct wl_client*, struct wl_resource*, struct wl_resource*);
    void (*set_input_region)(struct wl_client*, struct wl_resource*, struct wl_resource*);
    void (*commit)(struct wl_client*, struct wl_resource*);
    void (*set_buffer_transform)(struct wl_client*, struct wl_resource*, int32_t);
    void (*set_buffer_scale)(struct wl_client*, struct wl_resource*, int32_t);
    void (*damage_buffer)(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t);
    void (*offset)(struct wl_client*, struct wl_resource*, int32_t, int32_t);
};

struct wl_region_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*add)(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t);
    void (*subtract)(struct wl_client*, struct wl_resource*, int32_t, int32_t, int32_t, int32_t);
};

extern const struct wl_interface wl_subcompositor_interface;
extern const struct wl_interface wl_subsurface_interface;

struct wl_subcompositor_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*get_subsurface)(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*, struct wl_resource*);
};

struct wl_subsurface_interface {
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*set_position)(struct wl_client*, struct wl_resource*, int32_t, int32_t);
    void (*place_above)(struct wl_client*, struct wl_resource*, struct wl_resource*);
    void (*place_below)(struct wl_client*, struct wl_resource*, struct wl_resource*);
    void (*set_sync)(struct wl_client*, struct wl_resource*);
    void (*set_desync)(struct wl_client*, struct wl_resource*);
};

extern const struct wl_interface wl_data_device_manager_interface;
extern const struct wl_interface wl_data_device_interface;
extern const struct wl_interface wl_data_source_interface;

struct wl_data_device_manager_interface {
    void (*create_data_source)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*get_data_device)(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*);
};

struct wl_data_device_interface {
    void (*start_drag)(struct wl_client*, struct wl_resource*, struct wl_resource*, struct wl_resource*, struct wl_resource*, uint32_t);
    void (*set_selection)(struct wl_client*, struct wl_resource*, struct wl_resource*, uint32_t);
    void (*release)(struct wl_client*, struct wl_resource*);
};

struct wl_data_source_interface {
    void (*offer)(struct wl_client*, struct wl_resource*, const char*);
    void (*destroy)(struct wl_client*, struct wl_resource*);
    void (*set_actions)(struct wl_client*, struct wl_resource*, uint32_t);
};

extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_pointer_interface;
extern const struct wl_interface wl_keyboard_interface;
extern const struct wl_interface wl_touch_interface;

struct wl_seat_interface {
    void (*get_pointer)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*get_keyboard)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*get_touch)(struct wl_client*, struct wl_resource*, uint32_t);
    void (*release)(struct wl_client*, struct wl_resource*);
};

struct wl_pointer_interface {
    void (*set_cursor)(struct wl_client*, struct wl_resource*, uint32_t, struct wl_resource*, int32_t, int32_t);
    void (*release)(struct wl_client*, struct wl_resource*);
};

struct wl_keyboard_interface {
    void (*release)(struct wl_client*, struct wl_resource*);
};

void wl_seat_send_capabilities(struct wl_resource*, uint32_t);
void wl_seat_send_name(struct wl_resource*, const char*);

extern const struct wl_interface wl_output_interface;

struct wl_output_interface {
    void (*release)(struct wl_client*, struct wl_resource*);
};

void wl_output_send_geometry(struct wl_resource*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t);
void wl_output_send_mode(struct wl_resource*, uint32_t, int32_t, int32_t, int32_t);
void wl_output_send_done(struct wl_resource*);
void wl_output_send_scale(struct wl_resource*, int32_t);
void wl_output_send_name(struct wl_resource*, const char*);
void wl_output_send_description(struct wl_resource*, const char*);

} // extern "C"

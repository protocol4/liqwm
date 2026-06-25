// Trivial stub bodies for every function declared in the wayland_mock
// headers, plus dummy metadata objects for the `extern const struct
// wl_interface ...` declarations. Exists purely so the real protocol
// glue (Surface.cpp, XdgShell.cpp) can be linked into a throwaway test
// binary -- see link_test_main.cpp.
#include "wayland-server.h"
#include "xdg-shell-protocol.h"
#include <cstdint>

extern "C" {

void wl_array_init(struct wl_array* arr) { arr->size = 0; arr->alloc = 0; arr->data = nullptr; }
void wl_array_release(struct wl_array*) {}

struct wl_display* wl_display_create(void) { return reinterpret_cast<struct wl_display*>(0x1); }
void wl_display_destroy(struct wl_display*) {}
struct wl_event_loop* wl_display_get_event_loop(struct wl_display*) { return reinterpret_cast<struct wl_event_loop*>(0x2); }
const char* wl_display_add_socket_auto(struct wl_display*) { return "wayland-mock-0"; }
void* wl_display_init_shm(struct wl_display*) { return reinterpret_cast<void*>(0x3); }
void wl_display_flush_clients(struct wl_display*) {}

int wl_event_loop_dispatch(struct wl_event_loop*, int) { return 0; }
void* wl_event_loop_add_fd(struct wl_event_loop*, int, uint32_t, wl_event_loop_fd_func_t, void*) { return nullptr; }

struct wl_global* wl_global_create(struct wl_display*, const struct wl_interface*, int, void*, wl_global_bind_func_t) {
    return reinterpret_cast<struct wl_global*>(0x4);
}
void wl_global_destroy(struct wl_global*) {}

// No real wl_client ever binds in this test, so wl_resource_create is
// never actually exercised on the request-handling path -- it's enough
// for this to link and return null without being called for real.
struct wl_resource* wl_resource_create(struct wl_client*, const struct wl_interface*, int, uint32_t) { return nullptr; }
void wl_resource_set_implementation(struct wl_resource*, const void*, void*, wl_resource_destroy_func_t) {}
void* wl_resource_get_user_data(struct wl_resource*) { return nullptr; }
void wl_resource_destroy(struct wl_resource*) {}
int wl_resource_get_version(struct wl_resource*) { return 1; }
void wl_client_post_no_memory(struct wl_client*) {}

void xdg_surface_send_configure(struct wl_resource*, uint32_t) {}
void xdg_toplevel_send_configure(struct wl_resource*, int32_t, int32_t, struct wl_array*) {}

void wl_seat_send_capabilities(struct wl_resource*, uint32_t) {}
void wl_seat_send_name(struct wl_resource*, const char*) {}

void wl_output_send_geometry(struct wl_resource*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t) {}
void wl_output_send_mode(struct wl_resource*, uint32_t, int32_t, int32_t, int32_t) {}
void wl_output_send_done(struct wl_resource*) {}
void wl_output_send_scale(struct wl_resource*, int32_t) {}
void wl_output_send_name(struct wl_resource*, const char*) {}
void wl_output_send_description(struct wl_resource*, const char*) {}

const struct wl_interface wl_compositor_interface{};
const struct wl_interface wl_surface_interface{};
const struct wl_interface wl_region_interface{};
const struct wl_interface wl_callback_interface{};
const struct wl_interface wl_subcompositor_interface{};
const struct wl_interface wl_subsurface_interface{};
const struct wl_interface wl_data_device_manager_interface{};
const struct wl_interface wl_data_device_interface{};
const struct wl_interface wl_data_source_interface{};
const struct wl_interface wl_seat_interface{};
const struct wl_interface wl_pointer_interface{};
const struct wl_interface wl_keyboard_interface{};
const struct wl_interface wl_touch_interface{};
const struct wl_interface wl_output_interface{};
const struct wl_interface xdg_wm_base_interface{};
const struct wl_interface xdg_positioner_interface{};
const struct wl_interface xdg_surface_interface{};
const struct wl_interface xdg_toplevel_interface{};
const struct wl_interface xdg_popup_interface{};

} // extern "C"

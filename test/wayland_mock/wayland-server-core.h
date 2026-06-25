// Structural mock of the real <wayland-server-core.h>, used ONLY to
// syntax/type-check src/wayland/*.cpp without needing libwayland-server-dev
// installed. Function bodies are irrelevant -- this never links into the
// real binary. See test/wayland_mock/README.md.
#pragma once
#include <cstdint>
#include <cstddef>

extern "C" {

struct wl_display;
struct wl_event_loop;
struct wl_client;
struct wl_resource;
struct wl_global;
struct wl_interface {}; // mock only: real one has real fields, opacity doesn't matter here

typedef void (*wl_global_bind_func_t)(struct wl_client*, void*, uint32_t, uint32_t);
typedef void (*wl_resource_destroy_func_t)(struct wl_resource*);
typedef int (*wl_event_loop_fd_func_t)(int, uint32_t, void*);

struct wl_array {
    size_t size;
    size_t alloc;
    void* data;
};

void wl_array_init(struct wl_array*);
void wl_array_release(struct wl_array*);

struct wl_display* wl_display_create(void);
void wl_display_destroy(struct wl_display*);
struct wl_event_loop* wl_display_get_event_loop(struct wl_display*);
const char* wl_display_add_socket_auto(struct wl_display*);
void* wl_display_init_shm(struct wl_display*);
void wl_display_flush_clients(struct wl_display*);

int wl_event_loop_dispatch(struct wl_event_loop*, int);
void* wl_event_loop_add_fd(struct wl_event_loop*, int, uint32_t, wl_event_loop_fd_func_t, void*);

#define WL_EVENT_READABLE 1

struct wl_global* wl_global_create(struct wl_display*, const struct wl_interface*, int, void*, wl_global_bind_func_t);
void wl_global_destroy(struct wl_global*);

struct wl_resource* wl_resource_create(struct wl_client*, const struct wl_interface*, int, uint32_t);
void wl_resource_set_implementation(struct wl_resource*, const void*, void*, wl_resource_destroy_func_t);
void* wl_resource_get_user_data(struct wl_resource*);
void wl_resource_destroy(struct wl_resource*);
int wl_resource_get_version(struct wl_resource*);
void wl_client_post_no_memory(struct wl_client*);

} // extern "C"

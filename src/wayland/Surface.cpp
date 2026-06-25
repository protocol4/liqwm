// wl_compositor_interface / wl_surface_interface / wl_region_interface /
// wl_callback_interface come from <wayland-server-protocol.h>, generated
// from the CORE wayland.xml that ships with libwayland itself (the
// `libwayland-server-dev` package on Debian/Ubuntu) -- you do NOT need
// wayland-protocols for these, only for xdg-shell (see XdgShell.cpp).
#include "Surface.hpp"
#include <wayland-server.h>

namespace {

// --- wl_surface ---

void surface_destroy(wl_client*, wl_resource* resource) {
    wl_resource_destroy(resource);
}

void surface_attach(wl_client*, wl_resource* resource, wl_resource* buffer, int32_t, int32_t) {
    auto* state = static_cast<WlSurfaceState*>(wl_resource_get_user_data(resource));
    state->pendingBuffer = buffer;
}

void surface_damage(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t) {}

void surface_frame(wl_client* client, wl_resource* resource, uint32_t callbackId) {
    wl_resource* cb = wl_resource_create(client, &wl_callback_interface, 1, callbackId);
    // Real build: stash `cb` on the WlSurfaceState and fire
    // wl_callback_send_done(cb, frameTimeMs) from renderFrame() right
    // after THIS surface's content actually hits the screen, then
    // wl_resource_destroy(cb). Doing nothing with it (current state)
    // means well-behaved clients that throttle their rendering to frame
    // callbacks will stall after their first frame -- fine for now since
    // there's no renderer yet either, but it's the next thing this file
    // needs once renderFrame() is real.
    (void)resource;
    (void)cb;
}

void surface_set_opaque_region(wl_client*, wl_resource*, wl_resource*) {}
void surface_set_input_region(wl_client*, wl_resource*, wl_resource*) {}

void surface_commit(wl_client*, wl_resource* resource) {
    auto* state = static_cast<WlSurfaceState*>(wl_resource_get_user_data(resource));
    state->currentBuffer = state->pendingBuffer;
    state->everCommitted = true;
    if (state->onCommit) state->onCommit();
}

void surface_set_buffer_transform(wl_client*, wl_resource*, int32_t) {}
void surface_set_buffer_scale(wl_client*, wl_resource*, int32_t) {}
void surface_damage_buffer(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t) {}

void surface_offset(wl_client*, wl_resource*, int32_t, int32_t) {}
// ^ `offset` is a relatively recent addition (wl_surface version 5,
// libwayland >= 1.21). If your installed wayland-server is older than
// that, your wl_surface_interface struct won't have this field -- delete
// this function and its line in kSurfaceImpl below.

// Note the explicit `struct` here: wayland-scanner-generated headers
// define BOTH a `struct wl_surface_interface { ...function pointers... }`
// (the dispatch table type, used below) AND an
// `extern const struct wl_interface wl_surface_interface` (protocol
// metadata, used above in surface_frame/compositor_create_surface). In C
// these live in separate tag/ordinary namespaces and never collide; in
// C++ they're both visible, so you must say `struct wl_surface_interface`
// when you mean the TYPE, or the compiler resolves the bare name to the
// variable instead and you get a "expected type-specifier" error. Same
// reasoning applies to every *_interface dispatch table below and in
// XdgShell.cpp.
constexpr struct wl_surface_interface kSurfaceImpl = {
    surface_destroy,
    surface_attach,
    surface_damage,
    surface_frame,
    surface_set_opaque_region,
    surface_set_input_region,
    surface_commit,
    surface_set_buffer_transform,
    surface_set_buffer_scale,
    surface_damage_buffer,
    surface_offset,
};

void surface_resource_destroy(wl_resource* resource) {
    delete static_cast<WlSurfaceState*>(wl_resource_get_user_data(resource));
}

// --- wl_region (stub: full-surface hit testing only, no precise shapes) ---

void region_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }
void region_add(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t) {}
void region_subtract(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t) {}

constexpr struct wl_region_interface kRegionImpl = {
    region_destroy,
    region_add,
    region_subtract,
};

// --- wl_compositor ---

void compositor_create_surface(wl_client* client, wl_resource* resource, uint32_t id) {
    wl_resource* surfaceResource =
        wl_resource_create(client, &wl_surface_interface, wl_resource_get_version(resource), id);
    if (!surfaceResource) {
        wl_client_post_no_memory(client);
        return;
    }

    auto* state = new WlSurfaceState();
    state->resource = surfaceResource;
    wl_resource_set_implementation(surfaceResource, &kSurfaceImpl, state, surface_resource_destroy);
}

void compositor_create_region(wl_client* client, wl_resource*, uint32_t id) {
    wl_resource* regionResource = wl_resource_create(client, &wl_region_interface, 1, id);
    if (!regionResource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(regionResource, &kRegionImpl, nullptr, nullptr);
}

constexpr struct wl_compositor_interface kCompositorImpl = {
    compositor_create_surface,
    compositor_create_region,
};

void compositor_bind(wl_client* client, void*, uint32_t version, uint32_t id) {
    wl_resource* resource = wl_resource_create(client, &wl_compositor_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kCompositorImpl, nullptr, nullptr);
}

} // namespace

CompositorGlobal::CompositorGlobal(wl_display* display) {
    global_ = wl_global_create(display, &wl_compositor_interface, 6, nullptr, compositor_bind);
}

CompositorGlobal::~CompositorGlobal() {
    if (global_) wl_global_destroy(global_);
}

WlSurfaceState* CompositorGlobal::stateOf(wl_resource* surfaceResource) {
    return static_cast<WlSurfaceState*>(wl_resource_get_user_data(surfaceResource));
}

// --- wl_subcompositor / wl_subsurface ---
// Confirmed required by a real client (foot refused to start without
// this global existing at all). See Surface.hpp for scope/intent.

namespace {

struct SubsurfaceState {
    wl_resource* surfaceResource = nullptr;
    wl_resource* parentResource = nullptr;
    int32_t x = 0;
    int32_t y = 0;
    bool sync = true; // wl_subsurface defaults to synchronized per spec
};

void subsurface_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

void subsurface_set_position(wl_client*, wl_resource* resource, int32_t x, int32_t y) {
    auto* st = static_cast<SubsurfaceState*>(wl_resource_get_user_data(resource));
    st->x = x;
    st->y = y;
    // Real build: this is double-buffered state (applies on the PARENT's
    // next commit, per spec) -- not yet acted on by anything since
    // there's no renderer to composite child-into-parent yet.
}

void subsurface_place_above(wl_client*, wl_resource*, wl_resource*) {}
void subsurface_place_below(wl_client*, wl_resource*, wl_resource*) {}

void subsurface_set_sync(wl_client*, wl_resource* resource) {
    static_cast<SubsurfaceState*>(wl_resource_get_user_data(resource))->sync = true;
}
void subsurface_set_desync(wl_client*, wl_resource* resource) {
    static_cast<SubsurfaceState*>(wl_resource_get_user_data(resource))->sync = false;
}

constexpr struct wl_subsurface_interface kSubsurfaceImpl = {
    subsurface_destroy,
    subsurface_set_position,
    subsurface_place_above,
    subsurface_place_below,
    subsurface_set_sync,
    subsurface_set_desync,
};

void subsurface_resource_destroy(wl_resource* resource) {
    delete static_cast<SubsurfaceState*>(wl_resource_get_user_data(resource));
}

void subcompositor_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

void subcompositor_get_subsurface(wl_client* client, wl_resource* resource, uint32_t id,
                                   wl_resource* surface, wl_resource* parent) {
    wl_resource* subsurfaceResource =
        wl_resource_create(client, &wl_subsurface_interface, wl_resource_get_version(resource), id);
    if (!subsurfaceResource) {
        wl_client_post_no_memory(client);
        return;
    }

    auto* st = new SubsurfaceState();
    st->surfaceResource = surface;
    st->parentResource = parent;
    wl_resource_set_implementation(subsurfaceResource, &kSubsurfaceImpl, st, subsurface_resource_destroy);
}

constexpr struct wl_subcompositor_interface kSubcompositorImpl = {
    subcompositor_destroy,
    subcompositor_get_subsurface,
};

void subcompositor_bind(wl_client* client, void*, uint32_t version, uint32_t id) {
    wl_resource* resource = wl_resource_create(client, &wl_subcompositor_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kSubcompositorImpl, nullptr, nullptr);
}

} // namespace

SubcompositorGlobal::SubcompositorGlobal(wl_display* display) {
    global_ = wl_global_create(display, &wl_subcompositor_interface, 1, nullptr, subcompositor_bind);
}

SubcompositorGlobal::~SubcompositorGlobal() {
    if (global_) wl_global_destroy(global_);
}

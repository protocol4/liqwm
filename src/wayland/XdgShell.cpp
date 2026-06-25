// xdg_wm_base_interface / xdg_surface_interface / xdg_toplevel_interface
// (and xdg_popup_interface / xdg_positioner_interface, stubbed) come from
// xdg-shell-protocol.h, which CMake generates at build time from
// wayland-protocols' stable/xdg-shell/xdg-shell.xml via wayland-scanner.
// See CMakeLists.txt -- there's no hand-maintained header for this.
#include "XdgShell.hpp"
#include "Surface.hpp"
#include <wayland-server.h>
#include <xdg-shell-protocol.h>
#include <string>

namespace {

uint32_t nextSerial() {
    static uint32_t serial = 0;
    return ++serial;
}

// One of these per xdg_surface. Also pointed to by the xdg_toplevel
// resource created from it (xdg_toplevel has no independent state of its
// own in this MVP -- popups aren't implemented, so "the surface's role
// object" and "the toplevel" are the same lifetime).
struct XdgSurfaceState {
    XdgShellGlobal* owner = nullptr;
    wl_resource* toplevelResource = nullptr;
    WlSurfaceState* surface = nullptr;

    uint32_t configureSerial = 0;
    bool ackedFirstConfigure = false;
    bool mapped = false;
    uint64_t windowId = 0;

    std::string pendingTitle;
    std::string pendingAppId;
};

// --- xdg_toplevel ---

void toplevel_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }
void toplevel_set_parent(wl_client*, wl_resource*, wl_resource*) {}

void toplevel_set_title(wl_client*, wl_resource* resource, const char* title) {
    auto* st = static_cast<XdgSurfaceState*>(wl_resource_get_user_data(resource));
    st->pendingTitle = title ? title : "";
}

void toplevel_set_app_id(wl_client*, wl_resource* resource, const char* appId) {
    auto* st = static_cast<XdgSurfaceState*>(wl_resource_get_user_data(resource));
    st->pendingAppId = appId ? appId : "";
}

void toplevel_show_window_menu(wl_client*, wl_resource*, wl_resource*, uint32_t, int32_t, int32_t) {}

void toplevel_move(wl_client*, wl_resource*, wl_resource*, uint32_t) {
    // Real build: interactive-move request. Only meaningful for
    // free-floating windows (islandId == 0) -- forward into
    // InputManager's drag state. A tiled island window requesting this
    // should be ignored, same as Hyprland ignores `move` on tiled windows.
}
void toplevel_resize(wl_client*, wl_resource*, wl_resource*, uint32_t, uint32_t) {}
void toplevel_set_max_size(wl_client*, wl_resource*, int32_t, int32_t) {}
void toplevel_set_min_size(wl_client*, wl_resource*, int32_t, int32_t) {}
void toplevel_set_maximized(wl_client*, wl_resource*) {}
void toplevel_unset_maximized(wl_client*, wl_resource*) {}
void toplevel_set_fullscreen(wl_client*, wl_resource*, wl_resource*) {}
void toplevel_unset_fullscreen(wl_client*, wl_resource*) {}
void toplevel_set_minimized(wl_client*, wl_resource*) {}

constexpr struct xdg_toplevel_interface kToplevelImpl = {
    toplevel_destroy,
    toplevel_set_parent,
    toplevel_set_title,
    toplevel_set_app_id,
    toplevel_show_window_menu,
    toplevel_move,
    toplevel_resize,
    toplevel_set_max_size,
    toplevel_set_min_size,
    toplevel_set_maximized,
    toplevel_unset_maximized,
    toplevel_set_fullscreen,
    toplevel_unset_fullscreen,
    toplevel_set_minimized,
};

void toplevel_resource_destroy(wl_resource* resource) {
    auto* st = static_cast<XdgSurfaceState*>(wl_resource_get_user_data(resource));
    if (st->mapped && st->owner) st->owner->notifyUnmapped(st->windowId);
    st->toplevelResource = nullptr;
    st->mapped = false;
    // st itself belongs to the xdg_surface resource (see
    // xdgsurface_resource_destroy) -- the toplevel resource is a SECOND
    // wl_resource pointing at the same XdgSurfaceState, so it must not be
    // deleted here too.
}

// --- xdg_surface ---

void xdgsurface_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

void xdgsurface_get_toplevel(wl_client* client, wl_resource* resource, uint32_t id) {
    auto* st = static_cast<XdgSurfaceState*>(wl_resource_get_user_data(resource));

    wl_resource* toplevel =
        wl_resource_create(client, &xdg_toplevel_interface, wl_resource_get_version(resource), id);
    if (!toplevel) {
        wl_client_post_no_memory(client);
        return;
    }

    st->toplevelResource = toplevel;
    wl_resource_set_implementation(toplevel, &kToplevelImpl, st, toplevel_resource_destroy);

    // xdg-shell requires at least one configure round-trip before the
    // client's first commit is meaningful. width=0,height=0 in the
    // toplevel configure means "client, you pick your own initial size" --
    // we accept whatever it chooses (see onToplevelMapped in Compositor.cpp
    // for the fallback if it picks 0x0 right back).
    wl_array states;
    wl_array_init(&states);
    xdg_toplevel_send_configure(toplevel, 0, 0, &states);
    wl_array_release(&states);

    st->configureSerial = nextSerial();
    xdg_surface_send_configure(resource, st->configureSerial);
}

void xdgsurface_get_popup(wl_client* client, wl_resource*, uint32_t id, wl_resource*, wl_resource*) {
    // Popups (menus/tooltips/combo boxes) are out of scope for this MVP.
    // Create a resource so clients that probe for popup support don't
    // get a protocol error, but implement none of its behavior yet --
    // this is a real, known gap, not silently swallowed.
    wl_resource* popup = wl_resource_create(client, &xdg_popup_interface, 1, id);
    if (popup) wl_resource_set_implementation(popup, nullptr, nullptr, nullptr);
}

void xdgsurface_set_window_geometry(wl_client*, wl_resource*, int32_t, int32_t, int32_t, int32_t) {}

void xdgsurface_ack_configure(wl_client*, wl_resource* resource, uint32_t serial) {
    auto* st = static_cast<XdgSurfaceState*>(wl_resource_get_user_data(resource));
    if (serial == st->configureSerial) st->ackedFirstConfigure = true;
}

constexpr struct xdg_surface_interface kXdgSurfaceImpl = {
    xdgsurface_destroy,
    xdgsurface_get_toplevel,
    xdgsurface_get_popup,
    xdgsurface_set_window_geometry,
    xdgsurface_ack_configure,
};

void xdgsurface_resource_destroy(wl_resource* resource) {
    auto* st = static_cast<XdgSurfaceState*>(wl_resource_get_user_data(resource));
    if (st->mapped && st->owner) st->owner->notifyUnmapped(st->windowId);
    delete st;
}

// --- xdg_wm_base ---

void wmbase_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

void wmbase_create_positioner(wl_client* client, wl_resource*, uint32_t id) {
    // Positioners only matter for popups, which aren't implemented yet.
    wl_resource* positioner = wl_resource_create(client, &xdg_positioner_interface, 1, id);
    if (positioner) wl_resource_set_implementation(positioner, nullptr, nullptr, nullptr);
}

void wmbase_get_xdg_surface(wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surfaceResource) {
    auto* owner = static_cast<XdgShellGlobal*>(wl_resource_get_user_data(resource));

    wl_resource* xdgSurface =
        wl_resource_create(client, &xdg_surface_interface, wl_resource_get_version(resource), id);
    if (!xdgSurface) {
        wl_client_post_no_memory(client);
        return;
    }

    auto* st = new XdgSurfaceState();
    st->owner = owner;
    st->surface = CompositorGlobal::stateOf(surfaceResource);

    wl_resource_set_implementation(xdgSurface, &kXdgSurfaceImpl, st, xdgsurface_resource_destroy);

    // The heart of the map lifecycle: on every commit of the underlying
    // wl_surface, check whether this is the FIRST commit after we've
    // been acked AND a buffer has actually landed. That exact moment is
    // what xdg-shell defines as "mapped."
    st->surface->onCommit = [st]() {
        if (st->mapped || !st->ackedFirstConfigure || !st->surface->currentBuffer) return;
        if (!st->toplevelResource) return; // only toplevels map this way; popups unimplemented

        st->mapped = true;
        st->windowId = st->owner->notifyMapped(st->pendingTitle, st->pendingAppId, 0, 0);
        // NOTE: real size should come from the buffer's actual pixel
        // dimensions (wl_shm_buffer_get_width/height, or the dmabuf
        // equivalent) once buffer inspection exists -- passing 0,0 here
        // just means "Compositor, pick a sane default" for now. See
        // Compositor::onToplevelMapped's fallback.
    };
}

void wmbase_pong(wl_client*, wl_resource*, uint32_t) {}

constexpr struct xdg_wm_base_interface kWmBaseImpl = {
    wmbase_destroy,
    wmbase_create_positioner,
    wmbase_get_xdg_surface,
    wmbase_pong,
};

void wmbase_bind(wl_client* client, void* data, uint32_t version, uint32_t id) {
    wl_resource* resource = wl_resource_create(client, &xdg_wm_base_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kWmBaseImpl, data, nullptr);
}

} // namespace

XdgShellGlobal::XdgShellGlobal(wl_display* display, ToplevelMappedCallback onMapped, ToplevelUnmappedCallback onUnmapped)
    : onMapped_(std::move(onMapped)), onUnmapped_(std::move(onUnmapped)) {
    global_ = wl_global_create(display, &xdg_wm_base_interface, 6, this, wmbase_bind);
}

XdgShellGlobal::~XdgShellGlobal() {
    if (global_) wl_global_destroy(global_);
}

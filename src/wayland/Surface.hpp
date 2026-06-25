#pragma once
#include <wayland-server-core.h>
#include <functional>

// Minimal wl_compositor + wl_surface implementation. This is CORE
// Wayland protocol (wl_compositor/wl_surface come from wayland.xml,
// bundled with libwayland itself) -- NOT part of wayland-protocols or
// xdg-shell. Every xdg_surface wraps one of these, so xdg-shell can't
// exist without this file existing first.
//
// Deliberately minimal: enough to know "this surface has a buffer
// attached and was committed" so XdgShell can drive its configure/ack/
// map lifecycle, and enough to eventually hand a buffer to the renderer.
// No subsurfaces, no buffer transform/scale handling, no per-rect damage
// tracking yet -- all real gaps, listed in the README.

struct WlSurfaceState {
    wl_resource* resource = nullptr;
    wl_resource* pendingBuffer = nullptr;
    wl_resource* currentBuffer = nullptr;
    bool everCommitted = false;

    // Fired after every wl_surface.commit. XdgShell hooks this to know
    // exactly when a buffer has landed on an acked surface -- the precise
    // moment xdg-shell defines a toplevel as "mapped."
    std::function<void()> onCommit;
};

class CompositorGlobal {
public:
    explicit CompositorGlobal(wl_display* display);
    ~CompositorGlobal();

    // XdgShell calls this when a client calls xdg_wm_base.get_xdg_surface,
    // to find the WlSurfaceState backing the wl_surface resource passed in.
    static WlSurfaceState* stateOf(wl_resource* surfaceResource);

private:
    wl_global* global_ = nullptr;
};

// wl_subcompositor: a SEPARATE core global from wl_compositor, used to
// create parent/child wl_subsurface relationships (decorations, cursor
// compositing, video overlays -- many toolkits use these even for
// otherwise-simple windows). CONFIRMED necessary, not optional polish:
// foot refused to even start with "no sub compositor" until this existed.
// Positioning/sync state is tracked but not yet acted on by anything
// (there's no renderer yet to composite child surfaces into parents) --
// the immediate goal is just "the global exists, requests don't crash."
class SubcompositorGlobal {
public:
    explicit SubcompositorGlobal(wl_display* display);
    ~SubcompositorGlobal();

private:
    wl_global* global_ = nullptr;
};

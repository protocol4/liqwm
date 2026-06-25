#pragma once
#include <wayland-server-core.h>
#include <functional>
#include <string>
#include <cstdint>

// xdg-shell is the piece Aquamarine deliberately doesn't provide: it's
// how a bare wl_surface becomes an actual application window with a
// title, app id, and a configure/ack/map lifecycle. The dispatch-table
// glue lives in XdgShell.cpp, generated against xdg-shell-protocol.h
// (built from wayland-protocols' xdg-shell.xml by CMake, see
// CMakeLists.txt) -- this header only exposes what Compositor needs.

// Fired the moment a toplevel is considered "mapped" per the xdg-shell
// spec: its xdg_surface.configure has been acked AND its wl_surface has
// a buffer attached and committed. Returns the new Window's id.
using ToplevelMappedCallback =
    std::function<uint64_t(const std::string& title, const std::string& appId, int width, int height)>;
using ToplevelUnmappedCallback = std::function<void(uint64_t windowId)>;

class XdgShellGlobal {
public:
    XdgShellGlobal(wl_display* display, ToplevelMappedCallback onMapped, ToplevelUnmappedCallback onUnmapped);
    ~XdgShellGlobal();

    // Called by the free-function protocol glue in XdgShell.cpp. Public
    // only because wayland-server's C callback style has no `this` to
    // call a private member through -- not meant to be called from
    // outside this pair of files.
    uint64_t notifyMapped(const std::string& title, const std::string& appId, int width, int height) {
        return onMapped_ ? onMapped_(title, appId, width, height) : 0;
    }
    void notifyUnmapped(uint64_t windowId) {
        if (onUnmapped_) onUnmapped_(windowId);
    }

private:
    wl_global* global_ = nullptr;
    ToplevelMappedCallback onMapped_;
    ToplevelUnmappedCallback onUnmapped_;
};

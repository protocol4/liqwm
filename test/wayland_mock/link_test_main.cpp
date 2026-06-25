// Proves the REAL src/wayland/Surface.cpp (CompositorGlobal AND
// SubcompositorGlobal) + XdgShell.cpp link and run against a (mocked)
// wl_display without crashing -- construction, destruction, and the
// callback wiring all actually execute, just against stub libwayland
// symbols instead of the real library.
#include "wayland/Surface.hpp"
#include "wayland/XdgShell.hpp"
#include "wayland/DataDevice.hpp"
#include "wayland/Seat.hpp"
#include "wayland/Output.hpp"
#include <wayland-server-core.h>
#include <cstdio>

int main() {
    wl_display* display = wl_display_create();
    printf("wl_display_create -> %p\n", static_cast<void*>(display));

    {
        CompositorGlobal compositorGlobal(display);
        printf("CompositorGlobal constructed\n");

        SubcompositorGlobal subcompositorGlobal(display);
        printf("SubcompositorGlobal constructed\n");

        DataDeviceManagerGlobal dataDeviceManager(display);
        printf("DataDeviceManagerGlobal constructed\n");

        SeatGlobal seatGlobal(display);
        printf("SeatGlobal constructed\n");

        OutputGlobal outputGlobal(display, "Virtual-1", 1920, 1080);
        printf("OutputGlobal constructed\n");

        XdgShellGlobal xdgShell(
            display,
            [](const std::string& title, const std::string& appId, int w, int h) -> uint64_t {
                printf("onMapped callback fired: title=\"%s\" appId=\"%s\" %dx%d\n",
                       title.c_str(), appId.c_str(), w, h);
                return 42;
            },
            [](uint64_t id) {
                printf("onUnmapped callback fired: id=%llu\n", static_cast<unsigned long long>(id));
            });
        printf("XdgShellGlobal constructed\n");
        // Both destruct here at end of scope -- proves ~CompositorGlobal
        // and ~XdgShellGlobal (which call wl_global_destroy) don't crash.
    }

    wl_display_destroy(display);
    printf("link test OK -- construction/destruction of both globals "
           "completed without crashing\n");
    return 0;
}

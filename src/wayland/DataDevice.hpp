#pragma once
#include <wayland-server-core.h>

// Minimal wl_data_device_manager (clipboard/drag-and-drop protocol).
// CONFIRMED required by a real client: foot refused to start with
// "wl_data_device_manager not implemented by server" until this exists.
//
// Scope, deliberately: the global exists, create_data_source/
// get_data_device/etc. return live resources so nothing crashes, but
// nothing is actually wired between clients yet -- set_selection from
// one client doesn't get offered to another. Real cross-client clipboard
// is still a known gap (see README's "what's missing for a non-technical
// user" discussion) -- this only stops clients from refusing to start
// over its mere absence.
class DataDeviceManagerGlobal {
public:
    explicit DataDeviceManagerGlobal(wl_display* display);
    ~DataDeviceManagerGlobal();

private:
    wl_global* global_ = nullptr;
};

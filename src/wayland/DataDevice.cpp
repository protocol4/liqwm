// wl_data_device_manager_interface / wl_data_device_interface /
// wl_data_source_interface come from <wayland-server-protocol.h>, same
// core wayland.xml as wl_compositor/wl_surface -- no codegen needed.
#include "DataDevice.hpp"
#include <wayland-server.h>
#include <vector>
#include <string>

namespace {

// --- wl_data_source ---
// Real clipboard offering (telling another client "I have text/plain")
// isn't wired up yet -- offer() just records mime types for a future
// implementation to read, it doesn't broadcast anything.

struct DataSourceState {
    std::vector<std::string> mimeTypes;
};

void data_source_offer(wl_client*, wl_resource* resource, const char* mimeType) {
    auto* st = static_cast<DataSourceState*>(wl_resource_get_user_data(resource));
    if (mimeType) st->mimeTypes.emplace_back(mimeType);
}

void data_source_destroy(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }
void data_source_set_actions(wl_client*, wl_resource*, uint32_t) {}

constexpr struct wl_data_source_interface kDataSourceImpl = {
    data_source_offer,
    data_source_destroy,
    data_source_set_actions,
};

void data_source_resource_destroy(wl_resource* resource) {
    delete static_cast<DataSourceState*>(wl_resource_get_user_data(resource));
}

// --- wl_data_device ---
// set_selection currently only remembers the source on this resource --
// it does NOT yet offer it to other clients' data devices on focus
// change, which is the part that makes clipboard actually work
// cross-client. That's real future work, not silently skipped: see the
// header comment and the README.

struct DataDeviceState {
    wl_resource* currentSelection = nullptr;
};

void data_device_start_drag(wl_client*, wl_resource*, wl_resource*, wl_resource*, wl_resource*, uint32_t) {
    // Drag-and-drop: not implemented yet, same scope note as clipboard.
}

void data_device_set_selection(wl_client*, wl_resource* resource, wl_resource* source, uint32_t) {
    auto* st = static_cast<DataDeviceState*>(wl_resource_get_user_data(resource));
    st->currentSelection = source;
}

void data_device_release(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

constexpr struct wl_data_device_interface kDataDeviceImpl = {
    data_device_start_drag,
    data_device_set_selection,
    data_device_release,
};

void data_device_resource_destroy(wl_resource* resource) {
    delete static_cast<DataDeviceState*>(wl_resource_get_user_data(resource));
}

// --- wl_data_device_manager ---

void data_device_manager_create_data_source(wl_client* client, wl_resource* resource, uint32_t id) {
    wl_resource* sourceResource =
        wl_resource_create(client, &wl_data_source_interface, wl_resource_get_version(resource), id);
    if (!sourceResource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto* st = new DataSourceState();
    wl_resource_set_implementation(sourceResource, &kDataSourceImpl, st, data_source_resource_destroy);
}

void data_device_manager_get_data_device(wl_client* client, wl_resource* resource, uint32_t id, wl_resource*) {
    wl_resource* deviceResource =
        wl_resource_create(client, &wl_data_device_interface, wl_resource_get_version(resource), id);
    if (!deviceResource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto* st = new DataDeviceState();
    wl_resource_set_implementation(deviceResource, &kDataDeviceImpl, st, data_device_resource_destroy);
}

constexpr struct wl_data_device_manager_interface kDataDeviceManagerImpl = {
    data_device_manager_create_data_source,
    data_device_manager_get_data_device,
};

void data_device_manager_bind(wl_client* client, void*, uint32_t version, uint32_t id) {
    wl_resource* resource = wl_resource_create(client, &wl_data_device_manager_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kDataDeviceManagerImpl, nullptr, nullptr);
}

} // namespace

DataDeviceManagerGlobal::DataDeviceManagerGlobal(wl_display* display) {
    global_ = wl_global_create(display, &wl_data_device_manager_interface, 3, nullptr, data_device_manager_bind);
}

DataDeviceManagerGlobal::~DataDeviceManagerGlobal() {
    if (global_) wl_global_destroy(global_);
}

// wl_seat_interface / wl_pointer_interface / wl_keyboard_interface come
// from <wayland-server-protocol.h>, same core wayland.xml as
// wl_compositor/wl_surface -- no codegen needed.
#include "Seat.hpp"
#include <wayland-server.h>

namespace {

constexpr uint32_t kSeatCapabilityPointer = 1;  // WL_SEAT_CAPABILITY_POINTER
constexpr uint32_t kSeatCapabilityKeyboard = 2; // WL_SEAT_CAPABILITY_KEYBOARD

// --- wl_pointer ---

void pointer_set_cursor(wl_client*, wl_resource*, uint32_t, wl_resource*, int32_t, int32_t) {
    // Real build: track the cursor surface so renderFrame() can draw it.
    // No-op for now -- there's no renderer to show a cursor in yet either.
}
void pointer_release(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

constexpr struct wl_pointer_interface kPointerImpl = {
    pointer_set_cursor,
    pointer_release,
};

// --- wl_keyboard ---

void keyboard_release(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

constexpr struct wl_keyboard_interface kKeyboardImpl = {
    keyboard_release,
};

// --- wl_seat ---

void seat_get_pointer(wl_client* client, wl_resource* resource, uint32_t id) {
    wl_resource* pointerResource =
        wl_resource_create(client, &wl_pointer_interface, wl_resource_get_version(resource), id);
    if (!pointerResource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(pointerResource, &kPointerImpl, nullptr, nullptr);
}

void seat_get_keyboard(wl_client* client, wl_resource* resource, uint32_t id) {
    wl_resource* keyboardResource =
        wl_resource_create(client, &wl_keyboard_interface, wl_resource_get_version(resource), id);
    if (!keyboardResource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(keyboardResource, &kKeyboardImpl, nullptr, nullptr);
    // NOTE: no wl_keyboard.keymap event sent here. Real clients generally
    // expect one right after creation. Deferred until we actually have a
    // keymap to send (needs libxkbcommon -- see README's input section).
}

void seat_get_touch(wl_client* client, wl_resource*, uint32_t id) {
    // We never advertise the TOUCH capability, so well-behaved clients
    // shouldn't call this -- stub purely so a client that calls it anyway
    // doesn't get a protocol error.
    wl_resource* touchResource = wl_resource_create(client, &wl_touch_interface, 1, id);
    if (touchResource) wl_resource_set_implementation(touchResource, nullptr, nullptr, nullptr);
}

void seat_release(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

constexpr struct wl_seat_interface kSeatImpl = {
    seat_get_pointer,
    seat_get_keyboard,
    seat_get_touch,
    seat_release,
};

void seat_bind(wl_client* client, void*, uint32_t version, uint32_t id) {
    wl_resource* resource = wl_resource_create(client, &wl_seat_interface, version, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &kSeatImpl, nullptr, nullptr);

    wl_seat_send_capabilities(resource, kSeatCapabilityPointer | kSeatCapabilityKeyboard);
    if (version >= 2) {
        wl_seat_send_name(resource, "seat0");
    }
}

} // namespace

SeatGlobal::SeatGlobal(wl_display* display) {
    global_ = wl_global_create(display, &wl_seat_interface, 7, nullptr, seat_bind);
}

SeatGlobal::~SeatGlobal() {
    if (global_) wl_global_destroy(global_);
}

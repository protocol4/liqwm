// wl_output_interface comes from <wayland-server-protocol.h>, same core
// wayland.xml as wl_compositor/wl_surface -- no codegen needed.
#include "Output.hpp"
#include <wayland-server.h>

namespace {

constexpr int32_t kSubpixelUnknown = 0;   // WL_OUTPUT_SUBPIXEL_UNKNOWN
constexpr int32_t kTransformNormal = 0;   // WL_OUTPUT_TRANSFORM_NORMAL
constexpr uint32_t kModeCurrent = 0x1;    // WL_OUTPUT_MODE_CURRENT
constexpr uint32_t kModePreferred = 0x2;  // WL_OUTPUT_MODE_PREFERRED

void output_release(wl_client*, wl_resource* resource) { wl_resource_destroy(resource); }

constexpr struct wl_output_interface kOutputImpl = {
    output_release,
};

} // namespace

OutputGlobal::OutputGlobal(wl_display* display, const std::string& name, int widthPx, int heightPx, int refreshMilliHz)
    : name_(name), widthPx_(widthPx), heightPx_(heightPx), refreshMilliHz_(refreshMilliHz) {
    auto bind = [](wl_client* client, void* data, uint32_t version, uint32_t id) {
        auto* self = static_cast<OutputGlobal*>(data);

        wl_resource* resource = wl_resource_create(client, &wl_output_interface, version, id);
        if (!resource) {
            wl_client_post_no_memory(client);
            return;
        }
        wl_resource_set_implementation(resource, &kOutputImpl, nullptr, nullptr);

        wl_output_send_geometry(resource, 0, 0, 0, 0, kSubpixelUnknown,
                                 "liqwm", "virtual", kTransformNormal);
        wl_output_send_mode(resource, kModeCurrent | kModePreferred, self->widthPx_, self->heightPx_, self->refreshMilliHz_);

        if (version >= 2) {
            wl_output_send_scale(resource, 1);
        }
        if (version >= 4) {
            wl_output_send_name(resource, self->name_.c_str());
            wl_output_send_description(resource, "Synthetic output (no real backend output yet)");
        }

        wl_output_send_done(resource);
    };

    global_ = wl_global_create(display, &wl_output_interface, 4, this, bind);
}

OutputGlobal::~OutputGlobal() {
    if (global_) wl_global_destroy(global_);
}

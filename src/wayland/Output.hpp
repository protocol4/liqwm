#pragma once
#include <wayland-server-core.h>
#include <string>

// Minimal wl_output. CONFIRMED required by a real client: foot refused
// to start with "no monitors available" until at least one exists.
//
// Deliberately decoupled from Aquamarine's real IOutput/onNewOutput for
// now: this advertises a fixed, made-up mode regardless of whether the
// backend ever successfully starts. That's a real limitation (it won't
// reflect your actual display, can't change resolution, etc.) but it
// means protocol-chain testing isn't blocked on Aquamarine finding a
// working GPU allocator -- those are genuinely separate problems. Wiring
// this to real per-monitor info from onNewOutput is the natural next
// step once a backend actually starts successfully.
class OutputGlobal {
public:
    OutputGlobal(wl_display* display, const std::string& name, int widthPx, int heightPx, int refreshMilliHz = 60000);
    ~OutputGlobal();

private:
    wl_global* global_ = nullptr;
    std::string name_;
    int widthPx_;
    int heightPx_;
    int refreshMilliHz_;
};

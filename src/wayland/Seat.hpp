#pragma once
#include <wayland-server-core.h>

// Minimal wl_seat (+ wl_pointer/wl_keyboard objects it hands out).
// CONFIRMED required by a real client: foot refused to start with
// "no seats available" until this exists.
//
// Scope, deliberately: advertises pointer+keyboard capabilities (we DO
// have real IPointer/IKeyboard devices from Aquamarine, even if nothing
// forwards their events to these objects yet) and hands out live
// wl_pointer/wl_keyboard resources on request. What's NOT done yet:
// no keymap is sent on wl_keyboard (real clients typically expect one
// immediately -- if the next error is keymap-related, that's the next
// thing to implement, not a surprise), and no enter/motion/button/key
// events are ever sent (that's the Aquamarine-IPointer-to-wl_pointer
// relay -- real future work, listed in the README).
class SeatGlobal {
public:
    explicit SeatGlobal(wl_display* display);
    ~SeatGlobal();

private:
    wl_global* global_ = nullptr;
};

// CONFIRMED against the user's real /usr/include/aquamarine/input/Input.hpp
// (0.12.1) -- this is no longer a placeholder for IPointer/IKeyboard.
// Trimmed to just the events Compositor.cpp actually touches; the real
// header also has ITouch/ISwitch/ITablet/ITabletTool/ITabletPad, omitted
// here since nothing in this project uses them yet.
#pragma once
#include "../backend/Backend.hpp"
#include <hyprutils/signal/Signal.hpp>
#include <hyprutils/math/Vector2D.hpp>
#include <string>
#include <cstdint>

namespace Aquamarine {

class IKeyboard {
public:
    struct SKeyEvent {
        uint32_t timeMs = 0;
        uint32_t key = 0;
        bool pressed = false;
    };
    struct SModifiersEvent {
        uint32_t depressed = 0, latched = 0, locked = 0, group = 0;
    };

    struct {
        Hyprutils::Signal::CSignalT<> destroy;
        Hyprutils::Signal::CSignalT<SKeyEvent> key;
        Hyprutils::Signal::CSignalT<SModifiersEvent> modifiers;
    } events;
};

class IPointer {
public:
    struct SMoveEvent {
        uint32_t timeMs = 0;
        Hyprutils::Math::Vector2D delta, unaccel;
    };
    struct SWarpEvent {
        uint32_t timeMs = 0;
        Hyprutils::Math::Vector2D absolute;
    };
    struct SButtonEvent {
        uint32_t timeMs = 0;
        uint32_t button = 0;
        bool pressed = false;
    };
    struct SAxisEvent {
        uint32_t timeMs = 0;
        double delta = 0.0, discrete = 0.0;
    };

    struct {
        Hyprutils::Signal::CSignalT<> destroy;
        Hyprutils::Signal::CSignalT<SMoveEvent> move;
        Hyprutils::Signal::CSignalT<SWarpEvent> warp;
        Hyprutils::Signal::CSignalT<SButtonEvent> button;
        Hyprutils::Signal::CSignalT<SAxisEvent> axis;
        Hyprutils::Signal::CSignalT<> frame;
    } events;
};

} // namespace Aquamarine

#pragma once
#include "../Window.hpp"
#include "../Camera.hpp"
#include <vector>

// Every layout mode does exactly one job: given the current list of
// windows + monitors, decide each Window's worldPosition/size. Nothing
// else in the compositor needs to know which mode is active — input
// handling and rendering only ever read worldPosition off the Window.
class ILayoutMode {
public:
    virtual ~ILayoutMode() = default;

    virtual LayoutModeKind kind() const = 0;

    // Called whenever the window set changes (open/close/focus) or a
    // tiling-relevant event happens. CanvasMode mostly no-ops this since
    // positions are user-driven, not computed.
    virtual void arrange(std::vector<Window>& windows, const std::vector<Monitor>& monitors) = 0;

    // Optional hook: called every input event so a mode can react
    // (e.g. CanvasMode handling pan/zoom gestures directly).
    virtual void onActivate(Camera& camera, std::vector<Window>& windows, const std::vector<Monitor>& monitors) {}
};

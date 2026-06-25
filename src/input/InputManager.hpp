#pragma once
#include "../Camera.hpp"
#include "../Window.hpp"
#include "../layout/ILayoutMode.hpp"
#include <vector>
#include <functional>

enum class WMMode { Hypr, Canvas };

// Bridges raw input events (from Aquamarine's libinput wrapper, in the
// real build) into mode-aware actions. Hypr-mode keybinds are mostly
// "switch workspace / move focus"; Canvas-mode gestures are mostly
// "drag to pan / scroll to zoom / click-to-fly in bird's-eye".
class InputManager {
public:
    InputManager(Camera& camera, WMMode& mode) : camera_(camera), mode_(mode) {}

    // --- Canvas-mode gesture handlers ---
    void onPointerDrag(const Vec2& screenDelta);
    void onScroll(double deltaY, const Vec2& screenPos, const Vec2& viewportSize);

    // SUPER+SPACE: zoom out to fit every window, click anywhere to fly there.
    void enterBirdsEye(std::vector<Window>& windows, const Vec2& viewportSize);
    // Called on click while bird's-eye is active.
    void birdsEyeClick(const Vec2& screenPos, const Vec2& viewportSize);

    // SUPER+TAB (or whatever bind): flips the active paradigm without
    // touching a single Window's worldPosition/size.
    void toggleMode();

    bool birdsEyeActive() const { return birdsEyeActive_; }

private:
    Camera& camera_;
    WMMode& mode_;
    bool birdsEyeActive_ = false;
};

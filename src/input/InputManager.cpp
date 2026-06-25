#include "InputManager.hpp"

void InputManager::onPointerDrag(const Vec2& screenDelta) {
    if (mode_ != WMMode::Canvas) return; // Hypr mode ignores free dragging entirely
    camera_.pan(screenDelta);
}

void InputManager::onScroll(double deltaY, const Vec2& screenPos, const Vec2& viewportSize) {
    if (mode_ != WMMode::Canvas) return;
    // deltaY > 0 -> scroll down -> zoom out. Tune the exponent to taste.
    double factor = (deltaY > 0) ? 0.9 : (1.0 / 0.9);
    camera_.zoomAt(factor, screenPos, viewportSize);
}

void InputManager::enterBirdsEye(std::vector<Window>& windows, const Vec2& viewportSize) {
    if (mode_ != WMMode::Canvas) return; // bird's-eye is a Canvas-mode concept
    birdsEyeActive_ = true;
    camera_.flyToFit(windows, viewportSize);
}

void InputManager::birdsEyeClick(const Vec2& screenPos, const Vec2& viewportSize) {
    if (!birdsEyeActive_) return;
    Vec2 worldTarget = camera_.screenToWorld(screenPos, viewportSize);
    camera_.animateTo(worldTarget, 1.0); // fly back to zoom=1 centered on click
    birdsEyeActive_ = false;
}

void InputManager::toggleMode() {
    mode_ = (mode_ == WMMode::Hypr) ? WMMode::Canvas : WMMode::Hypr;
    birdsEyeActive_ = false;
}

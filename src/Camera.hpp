#pragma once
#include "Window.hpp"
#include <vector>

// Camera is the entire trick that makes two totally different window-
// management paradigms share one window model. Hypr mode keeps the
// camera pinned at zoom=1, origin wherever the active workspace's
// world-space "slot" is. Canvas mode lets the user actually drive this
// thing around.
class Camera {
public:
    Vec2 position{0, 0};   // world-space point currently at the center of the screen
    double zoom = 1.0;     // 1.0 = 1 world px : 1 screen px

    static constexpr double kMinZoom = 0.05;   // bird's-eye floor
    static constexpr double kMaxZoom = 4.0;
    static constexpr double kZoomSmoothing = 12.0; // higher = snappier

    Vec2 worldToScreen(const Vec2& world, const Vec2& viewportSize) const;
    Vec2 screenToWorld(const Vec2& screen, const Vec2& viewportSize) const;

    // Multiplicative zoom around a screen-space pivot (e.g. cursor position),
    // so zooming feels anchored under the mouse instead of recentring.
    void zoomAt(double factor, const Vec2& screenPivot, const Vec2& viewportSize);

    void pan(const Vec2& screenDelta);

    // Smoothly animates position+zoom toward a target over time. Call once
    // per frame with dt in seconds; returns true while still animating.
    bool tick(double dtSeconds);

    // Computes the camera state needed to fit every window in `wins` on
    // screen at once, with margin. This is what SUPER+SPACE (bird's-eye)
    // requests, and it's also reusable for "zoom to fit cluster".
    void flyToFit(const std::vector<Window>& wins, const Vec2& viewportSize, double marginPx = 80.0);

    // Begins a smooth animated transition rather than an instant jump.
    void animateTo(const Vec2& targetPos, double targetZoom);

private:
    bool animating_ = false;
    Vec2 targetPosition_{0, 0};
    double targetZoom_ = 1.0;
};

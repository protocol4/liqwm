#include "Camera.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

Vec2 Camera::worldToScreen(const Vec2& world, const Vec2& viewportSize) const {
    Vec2 centered = (world - position) * zoom;
    return centered + viewportSize * 0.5;
}

Vec2 Camera::screenToWorld(const Vec2& screen, const Vec2& viewportSize) const {
    Vec2 centered = screen - viewportSize * 0.5;
    return Vec2{centered.x / zoom, centered.y / zoom} + position;
}

void Camera::zoomAt(double factor, const Vec2& screenPivot, const Vec2& viewportSize) {
    // World point currently under the cursor, before the zoom changes.
    Vec2 worldUnderPivot = screenToWorld(screenPivot, viewportSize);

    zoom = std::clamp(zoom * factor, kMinZoom, kMaxZoom);

    // After changing zoom, re-derive position so that the SAME world point
    // is still under the cursor. This is the "anchor under mouse" feel.
    Vec2 newScreenOfPoint = worldToScreen(worldUnderPivot, viewportSize);
    Vec2 screenError = screenPivot - newScreenOfPoint;
    position = position - Vec2{screenError.x / zoom, screenError.y / zoom};
}

void Camera::pan(const Vec2& screenDelta) {
    position = position - Vec2{screenDelta.x / zoom, screenDelta.y / zoom};
}

void Camera::animateTo(const Vec2& targetPos, double targetZoom) {
    targetPosition_ = targetPos;
    targetZoom_ = std::clamp(targetZoom, kMinZoom, kMaxZoom);
    animating_ = true;
}

bool Camera::tick(double dtSeconds) {
    if (!animating_) return false;

    double t = std::clamp(dtSeconds * kZoomSmoothing, 0.0, 1.0);
    position = position + (targetPosition_ - position) * t;
    zoom += (targetZoom_ - zoom) * t;

    bool closeEnough =
        std::abs(position.x - targetPosition_.x) < 0.5 &&
        std::abs(position.y - targetPosition_.y) < 0.5 &&
        std::abs(zoom - targetZoom_) < 0.001;

    if (closeEnough) {
        position = targetPosition_;
        zoom = targetZoom_;
        animating_ = false;
    }
    return animating_;
}

void Camera::flyToFit(const std::vector<Window>& wins, const Vec2& viewportSize, double marginPx) {
    if (wins.empty()) {
        animateTo({0, 0}, 1.0);
        return;
    }

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& w : wins) {
        if (!w.mapped) continue;
        minX = std::min(minX, w.worldPosition.x);
        minY = std::min(minY, w.worldPosition.y);
        maxX = std::max(maxX, w.worldPosition.x + w.size.x);
        maxY = std::max(maxY, w.worldPosition.y + w.size.y);
    }

    Vec2 center{(minX + maxX) * 0.5, (minY + maxY) * 0.5};
    double spanX = std::max(1.0, maxX - minX) + marginPx * 2.0;
    double spanY = std::max(1.0, maxY - minY) + marginPx * 2.0;

    double fitZoom = std::min(viewportSize.x / spanX, viewportSize.y / spanY);
    animateTo(center, fitZoom);
}

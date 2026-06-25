#include "CanvasMode.hpp"
#include "../SpatialUtil.hpp"
#include <cmath>
#include <algorithm>

void CanvasMode::registerClusterRule(const std::string& appId, const std::string& clusterName) {
    clusterRules_[appId] = clusterName;
}

void CanvasMode::arrange(std::vector<Window>& windows, const std::vector<Monitor>& monitors) {
    // Intentionally near-empty: positions in Canvas mode are authoritative
    // state, not derived. We only auto-tag clusters here so the UI (e.g.
    // a neighborhood label drawn under a group of windows) has something
    // to render; we never move an existing, already-placed window.
    for (auto& w : windows) {
        if (w.cluster.empty()) {
            auto it = clusterRules_.find(w.appId);
            if (it != clusterRules_.end())
                w.cluster = it->second;
        }
    }
}

Vec2 CanvasMode::findFreeSpotNear(const Vec2& anchor, const Vec2& size, const std::vector<Window>& existing) {
    std::vector<SpatialUtil::Rect> occupied;
    occupied.reserve(existing.size());
    for (const auto& w : existing) {
        if (w.mapped) occupied.push_back({w.worldPosition, w.size});
    }
    return SpatialUtil::findFreeSpotNear(anchor, size, occupied);
}

void CanvasMode::placeNewWindow(Window& win, std::vector<Window>& existing, const Vec2& cameraFocus) {
    auto it = clusterRules_.find(win.appId);
    std::string targetCluster = (it != clusterRules_.end()) ? it->second : "";
    win.cluster = targetCluster;

    Vec2 anchor = cameraFocus;

    if (!targetCluster.empty()) {
        // Spawn near the centroid of existing members of the same cluster.
        Vec2 sum{0, 0};
        int count = 0;
        for (const auto& w : existing) {
            if (w.cluster == targetCluster) {
                sum = sum + w.worldPosition;
                ++count;
            }
        }
        if (count > 0)
            anchor = Vec2{sum.x / count, sum.y / count};
    }

    win.worldPosition = findFreeSpotNear(anchor, win.size, existing);
}

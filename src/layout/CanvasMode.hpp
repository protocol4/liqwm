#pragma once
#include "ILayoutMode.hpp"
#include <unordered_map>
#include <string>

// CanvasMode deliberately does almost nothing in arrange() — the whole
// point is that the user (or app placement heuristics on first map)
// controls worldPosition directly. What it DOES own:
//   1. Placing newly-mapped windows somewhere sane (near their cluster,
//      or near the camera if no cluster).
//   2. Optional auto-clustering: grouping windows by appId into named
//      neighborhoods so "Programming Zone" / "Gaming Zone" emerge
//      automatically instead of needing manual layout.
class CanvasMode : public ILayoutMode {
public:
    LayoutModeKind kind() const override { return LayoutModeKind::Canvas; }

    void arrange(std::vector<Window>& windows, const std::vector<Monitor>& monitors) override;

    // Maps an appId (e.g. "code", "kitty", "firefox") to a cluster name.
    // In a real build this would be loaded from config, same spirit as
    // hyprland.conf windowrule entries.
    void registerClusterRule(const std::string& appId, const std::string& clusterName);

    // Call when a new window is mapped in Canvas mode. Picks a spawn
    // point near its cluster's existing members (or near the camera's
    // current focus point if it's a brand new cluster), then nudges to
    // avoid exact overlap.
    void placeNewWindow(Window& win, std::vector<Window>& existing, const Vec2& cameraFocus);

private:
    std::unordered_map<std::string, std::string> clusterRules_;

    Vec2 findFreeSpotNear(const Vec2& anchor, const Vec2& size, const std::vector<Window>& existing);
};

#pragma once
#include "ILayoutMode.hpp"

enum class TilingAlgorithm { Master, Dwindle };

// Reproduces the two layouts Hyprland is known for. Each workspace is
// mapped onto its own dedicated rectangle of WORLD space (see the
// `worldSlotFor` helper in the .cpp) so that, when the user flips into
// Canvas mode, every tiled workspace simply becomes a neighborhood they
// can pan over to — nothing is destroyed or recreated.
class HyprMode : public ILayoutMode {
public:
    explicit HyprMode(TilingAlgorithm algo = TilingAlgorithm::Dwindle) : algo_(algo) {}

    LayoutModeKind kind() const override { return LayoutModeKind::Hypr; }

    void arrange(std::vector<Window>& windows, const std::vector<Monitor>& monitors) override;

    // Tiles `wins` into the given rect directly, with no notion of
    // workspace numbering or monitor geometry. This is what IslandManager
    // calls for each tiling island: arrange() handles "one global
    // Hyprland-style desktop"; arrangeRect() handles "tile whatever
    // windows live in this one neighborhood of the canvas."
    void arrangeRect(std::vector<Window*>& wins, const Vec2& origin, const Vec2& size);

    // Tunables, mirroring hyprland.conf-style knobs.
    double gapsIn = 6.0;
    double gapsOut = 12.0;
    double masterRatio = 0.55; // fraction of width given to the master window

private:
    TilingAlgorithm algo_;

    void arrangeMaster(std::vector<Window*>& wsWindows, const Vec2& origin, const Vec2& size);
    void arrangeDwindle(std::vector<Window*>& wsWindows, const Vec2& origin, const Vec2& size);
};

#pragma once
#include "Window.hpp"
#include "layout/HyprMode.hpp"
#include <vector>
#include <string>
#include <cstdint>

// An Island is a named region of the infinite canvas that runs its own
// internal Hyprland-style tiling. This is the concrete form of "tiling
// islands": instead of a single global Hypr-mode-vs-Canvas-mode switch,
// the world can contain any number of these neighborhoods side by side
// with free-floating windows, all under one camera. Zoom out far enough
// in Canvas mode and an island just looks like a tiled cluster of tiny
// rectangles sitting at its spot in the world — because that's literally
// what it is, not a separate rendering path.
struct Island {
    uint64_t id = 0;
    std::string name;
    Vec2 worldOrigin;      // top-left corner in world space
    Vec2 size{1920, 1080}; // logical area its internal tiling lays windows out into
};

class IslandManager {
public:
    // Places a new island near `nearWorldPoint`, spiral-searching outward
    // (via SpatialUtil) until it finds a spot that doesn't overlap any
    // existing island. Margin between islands is intentionally generous
    // so they read as distinct neighborhoods even before zooming in.
    // Returns the new island's id (NOT a reference) -- islands_ is a
    // vector, and a reference returned here would dangle the moment the
    // next createIsland() call causes a reallocation. Use findIsland(id)
    // if you need a live pointer afterward.
    uint64_t createIsland(const std::string& name, const Vec2& size, const Vec2& nearWorldPoint);

    Island* findIsland(uint64_t id);
    const std::vector<Island>& islands() const { return islands_; }

    // Groups `windows` by islandId and runs `tiling`.arrangeRect() on each
    // island's slice, scoped to that island's own worldOrigin/size.
    // Windows with islandId == 0 are left completely alone — those are
    // CanvasMode's free-floating windows, arranged separately.
    void arrangeAll(std::vector<Window>& windows, HyprMode& tiling);

private:
    std::vector<Island> islands_;
    uint64_t nextId_ = 1;
};

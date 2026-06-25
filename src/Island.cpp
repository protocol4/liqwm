#include "Island.hpp"
#include "SpatialUtil.hpp"
#include <map>

uint64_t IslandManager::createIsland(const std::string& name, const Vec2& size, const Vec2& nearWorldPoint) {
    std::vector<SpatialUtil::Rect> occupied;
    occupied.reserve(islands_.size());
    for (const auto& isl : islands_) occupied.push_back({isl.worldOrigin, isl.size});

    // Larger step than CanvasMode's per-window search: islands are big,
    // and we want clear daylight between neighborhoods, not a tight pack.
    Vec2 origin = SpatialUtil::findFreeSpotNear(nearWorldPoint, size, occupied, /*step=*/400.0);

    Island isl;
    isl.id = nextId_++;
    isl.name = name;
    isl.worldOrigin = origin;
    isl.size = size;
    islands_.push_back(isl);
    return isl.id;
}

Island* IslandManager::findIsland(uint64_t id) {
    for (auto& isl : islands_) {
        if (isl.id == id) return &isl;
    }
    return nullptr;
}

void IslandManager::arrangeAll(std::vector<Window>& windows, HyprMode& tiling) {
    std::map<uint64_t, std::vector<Window*>> byIsland;
    for (auto& w : windows) {
        if (!w.mapped || w.islandId == 0) continue;
        byIsland[w.islandId].push_back(&w);
    }

    for (auto& [id, wins] : byIsland) {
        Island* isl = findIsland(id);
        if (!isl) continue; // window references an island that no longer exists
        tiling.arrangeRect(wins, isl->worldOrigin, isl->size);
    }
}

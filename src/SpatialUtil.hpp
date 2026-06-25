#pragma once
#include "Window.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

// Shared spiral-search placement helper. Originally lived only inside
// CanvasMode (for placing a single new window near a cluster centroid);
// pulled out so IslandManager can use the exact same logic for placing
// whole islands near a requested spawn point without overlapping other
// islands. Same algorithm, different scale of rectangle.
namespace SpatialUtil {

struct Rect {
    Vec2 pos;
    Vec2 size;
};

inline bool overlaps(const Vec2& posA, const Vec2& sizeA, const Vec2& posB, const Vec2& sizeB) {
    return posA.x < posB.x + sizeB.x && posA.x + sizeA.x > posB.x &&
           posA.y < posB.y + sizeB.y && posA.y + sizeA.y > posB.y;
}

// Spirals outward from `anchor` looking for a spot where placing a rect
// of `size` wouldn't overlap anything in `occupied`. Defaults suit
// window-sized objects; pass a larger `step` for island-sized ones.
inline Vec2 findFreeSpotNear(const Vec2& anchor, const Vec2& size,
                              const std::vector<Rect>& occupied,
                              double step = 60.0, int maxRings = 40) {
    for (int ring = 0; ring <= maxRings; ++ring) {
        double radius = ring * step;
        int samples = std::max(1, ring * 8);

        for (int s = 0; s < samples; ++s) {
            double angle = (2.0 * M_PI * s) / samples;
            Vec2 candidate{
                anchor.x + std::cos(angle) * radius,
                anchor.y + std::sin(angle) * radius
            };

            bool collides = false;
            for (const auto& r : occupied) {
                if (overlaps(candidate, size, r.pos, r.size)) {
                    collides = true;
                    break;
                }
            }
            if (!collides) return candidate;
        }
    }
    return anchor; // give up gracefully, stack on top rather than crash
}

} // namespace SpatialUtil

#include "HyprMode.hpp"
#include <map>
#include <algorithm>

namespace {
// Each workspace number gets its own fixed-size rectangle laid out on an
// infinite world grid, far enough apart that they never visually overlap
// even at min zoom. This is the bridge between "workspace N" (an index)
// and "a place in the infinite canvas" (a world rect) — the same idea
// the README's neighborhood/cluster concept uses for Canvas mode.
constexpr double kWorkspaceCellW = 4000.0;
constexpr double kWorkspaceCellH = 3000.0;
constexpr int kWorkspacesPerRow = 8;

Vec2 worldSlotFor(int workspace, const Vec2& monitorRes) {
    int idx = workspace - 1;
    int col = idx % kWorkspacesPerRow;
    int row = idx / kWorkspacesPerRow;
    return Vec2{col * kWorkspaceCellW, row * kWorkspaceCellH};
}
}

void HyprMode::arrange(std::vector<Window>& windows, const std::vector<Monitor>& monitors) {
    if (monitors.empty()) return;
    const Monitor& mon = monitors.front(); // single-monitor focus for this skeleton

    std::map<int, std::vector<Window*>> byWorkspace;
    for (auto& w : windows) {
        if (!w.mapped || w.floating) continue;
        byWorkspace[w.workspace].push_back(&w);
    }

    for (auto& [ws, wins] : byWorkspace) {
        Vec2 origin = worldSlotFor(ws, mon.resolution);
        Vec2 usable{
            mon.resolution.x - gapsOut * 2.0,
            mon.resolution.y - gapsOut * 2.0
        };
        origin = origin + Vec2{gapsOut, gapsOut};

        arrangeRect(wins, origin, usable);
    }
}

void HyprMode::arrangeRect(std::vector<Window*>& wins, const Vec2& origin, const Vec2& size) {
    if (algo_ == TilingAlgorithm::Master)
        arrangeMaster(wins, origin, size);
    else
        arrangeDwindle(wins, origin, size);
}

void HyprMode::arrangeMaster(std::vector<Window*>& wins, const Vec2& origin, const Vec2& size) {
    if (wins.empty()) return;

    if (wins.size() == 1) {
        wins[0]->worldPosition = origin;
        wins[0]->size = size;
        return;
    }

    double masterW = size.x * masterRatio - gapsIn * 0.5;
    double stackW = size.x - masterW - gapsIn;

    wins[0]->worldPosition = origin;
    wins[0]->size = {masterW, size.y};

    size_t stackCount = wins.size() - 1;
    double stackH = (size.y - gapsIn * (stackCount - 1)) / static_cast<double>(stackCount);

    double cursorY = origin.y;
    for (size_t i = 1; i < wins.size(); ++i) {
        wins[i]->worldPosition = {origin.x + masterW + gapsIn, cursorY};
        wins[i]->size = {stackW, stackH};
        cursorY += stackH + gapsIn;
    }
}

void HyprMode::arrangeDwindle(std::vector<Window*>& wins, const Vec2& origin, const Vec2& size) {
    if (wins.empty()) return;

    // Classic binary-split dwindle: each new window halves whatever space
    // the previous window was occupying, alternating split axis.
    Vec2 curOrigin = origin;
    Vec2 curSize = size;

    for (size_t i = 0; i < wins.size(); ++i) {
        bool isLast = (i == wins.size() - 1);

        if (isLast) {
            wins[i]->worldPosition = curOrigin;
            wins[i]->size = curSize;
            break;
        }

        bool splitVertical = (i % 2 == 0); // alternate split axis per depth
        if (splitVertical) {
            double half = (curSize.x - gapsIn) * 0.5;
            wins[i]->worldPosition = curOrigin;
            wins[i]->size = {half, curSize.y};

            curOrigin = {curOrigin.x + half + gapsIn, curOrigin.y};
            curSize = {half, curSize.y};
        } else {
            double half = (curSize.y - gapsIn) * 0.5;
            wins[i]->worldPosition = curOrigin;
            wins[i]->size = {curSize.x, half};

            curOrigin = {curOrigin.x, curOrigin.y + half + gapsIn};
            curSize = {curSize.x, half};
        }
    }
}

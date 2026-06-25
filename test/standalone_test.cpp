// Standalone sanity test for the parts of this project that have ZERO
// Aquamarine/Wayland dependency: Camera transforms, HyprMode tiling math,
// CanvasMode placement/clustering. Build & run with:
//
//   g++ -std=c++23 -I src test/standalone_test.cpp \
//       src/Camera.cpp src/Island.cpp \
//       src/layout/HyprMode.cpp src/layout/CanvasMode.cpp \
//       -o standalone_test && ./standalone_test
//
// This is exactly the command used in the project README to verify the
// logic compiles/runs before you go anywhere near a real Wayland session.

#include "../src/Camera.hpp"
#include "../src/layout/HyprMode.hpp"
#include "../src/layout/CanvasMode.hpp"
#include "../src/Island.hpp"
#include <cassert>
#include <cstdio>
#include <cmath>

static bool approxEq(double a, double b, double eps = 0.001) {
    return std::fabs(a - b) < eps;
}

void test_camera_round_trip() {
    Camera cam;
    cam.position = {100, 200};
    cam.zoom = 2.0;

    Vec2 viewport{1920, 1080};
    Vec2 world{500, 300};

    Vec2 screen = cam.worldToScreen(world, viewport);
    Vec2 backToWorld = cam.screenToWorld(screen, viewport);

    assert(approxEq(backToWorld.x, world.x));
    assert(approxEq(backToWorld.y, world.y));
    printf("[PASS] camera world<->screen round trip\n");
}

void test_zoom_anchors_under_cursor() {
    Camera cam;
    cam.position = {0, 0};
    cam.zoom = 1.0;

    Vec2 viewport{1000, 1000};
    Vec2 cursor{700, 300};

    Vec2 worldUnderCursorBefore = cam.screenToWorld(cursor, viewport);
    cam.zoomAt(1.5, cursor, viewport);
    Vec2 worldUnderCursorAfter = cam.screenToWorld(cursor, viewport);

    assert(approxEq(worldUnderCursorBefore.x, worldUnderCursorAfter.x));
    assert(approxEq(worldUnderCursorBefore.y, worldUnderCursorAfter.y));
    printf("[PASS] zoom keeps the point under the cursor fixed\n");
}

void test_flytofit_contains_all_windows() {
    Camera cam;
    std::vector<Window> wins;

    Window a; a.worldPosition = {0, 0}; a.size = {800, 600}; a.mapped = true;
    Window b; b.worldPosition = {5000, 2000}; b.size = {400, 400}; b.mapped = true;
    wins.push_back(a);
    wins.push_back(b);

    Vec2 viewport{1920, 1080};
    cam.flyToFit(wins, viewport);
    // flyToFit only sets a target; drive the animation to completion.
    for (int i = 0; i < 200; ++i) cam.tick(0.1);

    for (auto& w : wins) {
        Vec2 tl = cam.worldToScreen(w.worldPosition, viewport);
        Vec2 br = cam.worldToScreen(w.worldPosition + w.size, viewport);
        assert(tl.x >= -1.0 && tl.y >= -1.0);
        assert(br.x <= viewport.x + 1.0 && br.y <= viewport.y + 1.0);
    }
    printf("[PASS] bird's-eye fit keeps every window on screen\n");
}

void test_hypr_dwindle_no_overlap() {
    HyprMode layout(TilingAlgorithm::Dwindle);
    std::vector<Monitor> monitors{ Monitor{"DP-1", {0,0}, {1920, 1080}} };

    std::vector<Window> wins;
    for (int i = 0; i < 4; ++i) {
        Window w;
        w.id = i;
        w.workspace = 1;
        wins.push_back(w);
    }

    layout.arrange(wins, monitors);

    // Pairwise overlap check.
    for (size_t i = 0; i < wins.size(); ++i) {
        for (size_t j = i + 1; j < wins.size(); ++j) {
            bool overlapX = wins[i].worldPosition.x < wins[j].worldPosition.x + wins[j].size.x &&
                             wins[i].worldPosition.x + wins[i].size.x > wins[j].worldPosition.x;
            bool overlapY = wins[i].worldPosition.y < wins[j].worldPosition.y + wins[j].size.y &&
                             wins[i].worldPosition.y + wins[i].size.y > wins[j].worldPosition.y;
            assert(!(overlapX && overlapY));
        }
    }
    printf("[PASS] dwindle tiling produces non-overlapping windows\n");
}

void test_hypr_master_ratio() {
    HyprMode layout(TilingAlgorithm::Master);
    layout.masterRatio = 0.6;
    std::vector<Monitor> monitors{ Monitor{"DP-1", {0,0}, {2000, 1000}} };

    std::vector<Window> wins;
    for (int i = 0; i < 3; ++i) {
        Window w; w.id = i; w.workspace = 1;
        wins.push_back(w);
    }
    layout.arrange(wins, monitors);

    // Master window should be roughly 60% of usable width.
    double usableW = 2000 - layout.gapsOut * 2.0;
    assert(wins[0].size.x > usableW * 0.55 && wins[0].size.x < usableW * 0.65);
    printf("[PASS] master layout respects masterRatio\n");
}

void test_canvas_clustering_and_placement() {
    CanvasMode canvas;
    canvas.registerClusterRule("code", "Programming Zone");
    canvas.registerClusterRule("steam", "Gaming Zone");

    std::vector<Window> existing;

    Window ide; ide.appId = "code"; ide.size = {800, 600};
    canvas.placeNewWindow(ide, existing, {0, 0});
    existing.push_back(ide);

    Window term; term.appId = "code"; term.size = {600, 400}; // pretend terminal shares the rule for this test
    canvas.placeNewWindow(term, existing, {0, 0});

    assert(term.cluster == "Programming Zone");

    // Should not overlap the IDE window it spawned near.
    bool overlapX = term.worldPosition.x < ide.worldPosition.x + ide.size.x &&
                     term.worldPosition.x + term.size.x > ide.worldPosition.x;
    bool overlapY = term.worldPosition.y < ide.worldPosition.y + ide.size.y &&
                     term.worldPosition.y + term.size.y > ide.worldPosition.y;
    assert(!(overlapX && overlapY));

    printf("[PASS] canvas clustering tags + collision-free placement\n");
}

void test_islands_dont_overlap_each_other() {
    IslandManager mgr;
    uint64_t idA = mgr.createIsland("Programming Island", {2400, 1400}, {0, 0});
    // Deliberately request the SAME spawn point -- this is the case that
    // matters: two islands both wanting to be "near the origin" must not
    // be allowed to overlap.
    uint64_t idB = mgr.createIsland("Gaming Island", {1800, 1200}, {0, 0});

    Island* a = mgr.findIsland(idA);
    Island* b = mgr.findIsland(idB);
    assert(a && b);

    bool overlapX = a->worldOrigin.x < b->worldOrigin.x + b->size.x &&
                    a->worldOrigin.x + a->size.x > b->worldOrigin.x;
    bool overlapY = a->worldOrigin.y < b->worldOrigin.y + b->size.y &&
                    a->worldOrigin.y + a->size.y > b->worldOrigin.y;
    assert(!(overlapX && overlapY));
    printf("[PASS] islands requested at the same point still don't overlap\n");
}

void test_island_tiling_is_scoped_and_isolated() {
    IslandManager mgr;
    HyprMode tiling(TilingAlgorithm::Dwindle);

    uint64_t idA = mgr.createIsland("Programming Island", {2400, 1400}, {0, 0});
    uint64_t idB = mgr.createIsland("Gaming Island", {1800, 1200}, {6000, 0});

    // Look up fresh pointers AFTER both islands exist -- never hold an
    // Island& across a createIsland() call, it can reallocate the vector.
    Island* a = mgr.findIsland(idA);
    Island* b = mgr.findIsland(idB);
    assert(a && b);

    std::vector<Window> windows;
    for (int i = 0; i < 3; ++i) {
        Window w; w.id = i; w.islandId = idA;
        windows.push_back(w);
    }
    for (int i = 0; i < 2; ++i) {
        Window w; w.id = 100 + i; w.islandId = idB;
        windows.push_back(w);
    }
    // One free-floating window, not part of any island -- must be left
    // completely untouched by arrangeAll.
    Window freeWin; freeWin.id = 999; freeWin.islandId = 0;
    freeWin.worldPosition = {12345, 6789};
    windows.push_back(freeWin);

    mgr.arrangeAll(windows, tiling);

    // Re-fetch after arrangeAll too, for the same reason.
    a = mgr.findIsland(idA);
    b = mgr.findIsland(idB);

    // Every window in island A must land within island A's rect, and
    // every window in island B within island B's rect -- proving the two
    // islands' tiling passes didn't bleed into each other.
    for (const auto& w : windows) {
        if (w.islandId == idA) {
            assert(w.worldPosition.x >= a->worldOrigin.x - 0.01);
            assert(w.worldPosition.y >= a->worldOrigin.y - 0.01);
            assert(w.worldPosition.x + w.size.x <= a->worldOrigin.x + a->size.x + 0.01);
        } else if (w.islandId == idB) {
            assert(w.worldPosition.x >= b->worldOrigin.x - 0.01);
            assert(w.worldPosition.y >= b->worldOrigin.y - 0.01);
            assert(w.worldPosition.x + w.size.x <= b->worldOrigin.x + b->size.x + 0.01);
        } else {
            // The free window: untouched.
            assert(approxEq(w.worldPosition.x, 12345));
            assert(approxEq(w.worldPosition.y, 6789));
        }
    }
    printf("[PASS] each island tiles only its own windows; free windows untouched\n");
}

int main() {
    test_camera_round_trip();
    test_zoom_anchors_under_cursor();
    test_flytofit_contains_all_windows();
    test_hypr_dwindle_no_overlap();
    test_hypr_master_ratio();
    test_canvas_clustering_and_placement();
    test_islands_dont_overlap_each_other();
    test_island_tiling_is_scoped_and_isolated();
    printf("\nAll standalone tests passed.\n");
    return 0;
}

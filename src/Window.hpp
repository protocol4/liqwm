#pragma once
#include <string>
#include <vector>
#include <cstdint>

// Forward decl: in the real integration this wraps Aquamarine/Wayland surface
// objects. Kept opaque here so layout code never needs to know about
// rendering internals.
struct SurfaceHandle;

// A 2D vector. Used both for world-space and screen-space coordinates,
// distinguished only by which Camera transform was applied.
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(double s) const { return {x * s, y * s}; }
};

enum class LayoutModeKind {
    Hypr,   // dynamic tiling, Hyprland-style
    Canvas  // infinite pan/zoom canvas
};

// The single most important design decision in this whole project:
// a Window's position is ALWAYS expressed in world space (pixels on an
// infinite plane), never in screen space. Tiling mode is just a layout
// engine that *writes* worldPosition/size every time it re-arranges;
// canvas mode lets the user write it directly via drag/resize. The
// renderer is the only thing that ever converts world -> screen, via
// Camera. This is what lets us hot-swap layout modes without recreating
// or reparenting any window.
struct Window {
    uint64_t id = 0;
    std::string title;
    std::string appId;          // wayland app_id / wm class, used for clustering

    Vec2 worldPosition;
    Vec2 size{800, 600};

    // Tiling-mode bookkeeping. Ignored entirely in Canvas mode.
    int workspace = 1;
    bool floating = false;

    // Tiling-ISLANDS bookkeeping (see Island.hpp). 0 means "free-floating
    // in the open canvas, managed by CanvasMode"; any other value is the
    // id of an Island this window is tiled inside of. Independent of
    // `workspace` above, which is only used by HyprMode's plain global
    // arrange() (single-desktop mode, no canvas at all).
    uint64_t islandId = 0;

    // Canvas-mode bookkeeping. Ignored entirely in Hypr mode.
    std::string cluster;        // e.g. "Programming Zone", "Gaming Zone"
    bool pinned = false;        // stays put even if auto-clustering runs

    SurfaceHandle* surface = nullptr;

    bool mapped = true;
};

struct Monitor {
    std::string name;
    Vec2 position;   // position in the *output layout*, not world space
    Vec2 resolution{1920, 1080};
    double refreshHz = 60.0;
};

#pragma once
#include "Window.hpp"
#include "Camera.hpp"
#include "Island.hpp"
#include "layout/ILayoutMode.hpp"
#include "layout/HyprMode.hpp"
#include "layout/CanvasMode.hpp"
#include "input/InputManager.hpp"
#include "wayland/Surface.hpp"
#include "wayland/XdgShell.hpp"
#include "wayland/DataDevice.hpp"
#include "wayland/Seat.hpp"
#include "wayland/Output.hpp"

#include <aquamarine/backend/Backend.hpp>
#include <aquamarine/output/Output.hpp>
#include <aquamarine/input/Input.hpp>
#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/signal/Signal.hpp>
#include <wayland-server-core.h>

#include <vector>
#include <memory>
#include <string>
#include <cstdint>

// NOTE: Backend.hpp only forward-declares IOutput/IPointer/IKeyboard
// (`class IOutput;` etc) -- it deliberately doesn't pull in their full
// definitions, since not every Aquamarine consumer needs them. We do,
// so we include the headers that actually define them. If your installed
// version uses different filenames/paths for these, that's the next
// thing the compiler will tell us, in an unambiguous "No such file" way.

// CONFIRMED (this is the part I previously flagged as unverified, and
// got wrong on the first guess -- see history): Aquamarine's ref-counted
// handles are plain Hyprutils::Memory::CSharedPointer<T>, NOT something
// living in the Aquamarine namespace. `SP` is a convenience alias
// HYPRLAND ITSELF defines in its own codebase (its
// src/helpers/memory/Memory.hpp), not something Aquamarine or hyprutils
// hand you -- so we define the same convenience here, the same way.
template <typename T>
using SP = Hyprutils::Memory::CSharedPointer<T>;

class Compositor {
public:
    Compositor();
    ~Compositor();

    // Initializes Wayland + Aquamarine, registers all listeners, and runs
    // the combined event loop until told to stop. Blocking call.
    int run();

private:
    // --- Wayland protocol wiring (this is "real" -- see src/wayland/) ---
    wl_display* display_ = nullptr;
    wl_event_loop* loop_ = nullptr;
    std::unique_ptr<CompositorGlobal> wlCompositor_;
    std::unique_ptr<SubcompositorGlobal> wlSubcompositor_;
    std::unique_ptr<DataDeviceManagerGlobal> wlDataDeviceManager_;
    std::unique_ptr<SeatGlobal> wlSeat_;
    std::unique_ptr<OutputGlobal> wlOutput_;
    std::unique_ptr<XdgShellGlobal> xdgShell_;

    void initWayland();
    uint64_t onToplevelMapped(const std::string& title, const std::string& appId, int width, int height);
    void onToplevelUnmapped(uint64_t windowId);
    Window* findWindow(uint64_t id);
    uint64_t nextWindowId_ = 1;

    // --- Aquamarine wiring ---
    SP<Aquamarine::CBackend> backend_;

    // CONFIRMED by the real compiler error: registerListener() returns a
    // Hyprutils::Signal::CHyprSignalListener, which is itself a
    // Hyprutils::Memory::CSharedPointer<CSignalListener> -- ref-counted.
    // Discard it (as earlier versions of this file did) and the listener
    // is very likely disconnected the instant the temporary is destroyed.
    // Every registerListener() call now stores its handle here instead.
    // Not cleaned up per-device on unplug yet (no output/pointer removal
    // handling exists at all currently) -- this just stops the "silently
    // never fires" bug, it doesn't yet free handles for removed devices.
    std::vector<Hyprutils::Signal::CHyprSignalListener> signalListeners_;

    void initBackend();
    void onNewOutput(SP<Aquamarine::IOutput> output);
    void onNewPointer(SP<Aquamarine::IPointer> pointer);
    void onNewKeyboard(SP<Aquamarine::IKeyboard> keyboard);
    void renderFrame(Aquamarine::IOutput& output);

    // --- WM state, independent of any backend ---
    std::vector<Window> windows_;
    std::vector<Monitor> monitors_;
    Camera camera_;

    // mode_ now only affects INPUT semantics (does a drag pan the camera
    // or move a window? does SUPER+SPACE matter right now?) -- it no
    // longer gates which arrange logic runs. Arrangement is per-region:
    // every tiling Island always tiles itself, and every free window is
    // always managed by CanvasMode, simultaneously, all the time.
    WMMode mode_ = WMMode::Canvas;
    HyprMode hyprLayout_;       // shared tiling engine, reused by every Island
    CanvasMode canvasLayout_;   // owns free-floating windows + cluster tagging
    IslandManager islandManager_;

    InputManager input_{camera_, mode_};

    void rearrange();
    void registerKeybinds();

    bool running_ = true;
};

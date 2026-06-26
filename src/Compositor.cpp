// NOTE ON AQUAMARINE API CALLS IN THIS FILE
// ------------------------------------------
// CONFIRMED against real-world Aquamarine/Hyprland source and crash
// reports: ref-counted handles are Hyprutils::Memory::CSharedPointer<T>,
// aliased here as `SP<T>` (see Compositor.hpp) -- NOT `Aquamarine::SP<T>`,
// which was my first guess and doesn't exist. That's now fixed throughout
// this file.
//
// STILL UNVERIFIED, likely the next thing to break: CBackend::create()'s
// exact parameter types, SBackendImplementationOptions' fields, the
// IOutputState::state()/mode/pixelSize access path, and whether
// events.newOutput etc. really take a bare std::any (vs. a more specific
// payload type) in your installed 0.12.1. Fix these as the compiler
// reports them -- that's a faster feedback loop than guessing further.
// Everything OUTSIDE this file (Camera, HyprMode, CanvasMode,
// InputManager, Surface, XdgShell) has zero Aquamarine dependency and
// compiles/runs/tests standalone already.

#include "Compositor.hpp"
#include <iostream>
#include <chrono>
#include <any>
#include <algorithm>

Compositor::Compositor() {
    hyprLayout_.gapsIn = 6.0;
    hyprLayout_.gapsOut = 12.0;

    canvasLayout_.registerClusterRule("code", "Programming Zone");
    canvasLayout_.registerClusterRule("kitty", "Programming Zone");
    canvasLayout_.registerClusterRule("firefox", "Programming Zone");
    canvasLayout_.registerClusterRule("steam", "Gaming Zone");
    canvasLayout_.registerClusterRule("discord", "Gaming Zone");

    // Demo tiling islands: named neighborhoods that tile internally.
    // In the real build, you'd create these on demand (e.g. a keybind
    // "turn this free-floating window group into an island") rather than
    // hardcoding two at startup -- this just proves the mechanism end to
    // end. Any Window with islandId set to one of these ids gets tiled
    // inside it automatically by rearrange() below.
    islandManager_.createIsland("Programming Island", {2400, 1400}, {0, 0});
    islandManager_.createIsland("Gaming Island", {1800, 1200}, {4000, 0});
}

Compositor::~Compositor() = default;

void Compositor::rearrange() {
    // Always-on, every pass, regardless of mode_:
    //   - free windows (islandId == 0) get cluster-tagged by CanvasMode
    //   - every tiling Island re-tiles its own windows independently
    // This is the actual "tiling islands" mechanism: there's no global
    // switch deciding how the whole desktop is laid out, only a per-
    // window/per-region decision that was made once, at placement time.
    canvasLayout_.arrange(windows_, monitors_);
    islandManager_.arrangeAll(windows_, hyprLayout_);
}

void Compositor::initWayland() {
    display_ = wl_display_create();
    if (!display_) {
        std::cerr << "Failed to create wl_display\n";
        return;
    }

    if (!wl_display_init_shm(display_)) {
        std::cerr << "Failed to init wl_shm (software-buffer support) -- "
                      "clients that only do CPU rendering won't work\n";
    }

    const char* socketName = wl_display_add_socket_auto(display_);
    if (!socketName) {
        std::cerr << "Failed to open a Wayland socket\n";
        return;
    }
    std::cout << "Listening on WAYLAND_DISPLAY=" << socketName << "\n";

    loop_ = wl_display_get_event_loop(display_);

    wlCompositor_ = std::make_unique<CompositorGlobal>(display_);
    wlSubcompositor_ = std::make_unique<SubcompositorGlobal>(display_);
    wlDataDeviceManager_ = std::make_unique<DataDeviceManagerGlobal>(display_);
    wlSeat_ = std::make_unique<SeatGlobal>(display_);

    // Synthetic output, deliberately NOT gated on Aquamarine ever
    // succeeding -- see Output.hpp for why. Also populated into
    // monitors_ so HyprMode's plain arrange() path (unused by Islands,
    // but still real code) has something to work with even with no real
    // backend output.
    wlOutput_ = std::make_unique<OutputGlobal>(display_, "Virtual-1", 1920, 1080);
    Monitor virtualMon;
    virtualMon.name = "Virtual-1";
    virtualMon.resolution = {1920, 1080};
    monitors_.push_back(virtualMon);

    xdgShell_ = std::make_unique<XdgShellGlobal>(
        display_,
        [this](const std::string& title, const std::string& appId, int w, int h) {
            return onToplevelMapped(title, appId, w, h);
        },
        [this](uint64_t id) { onToplevelUnmapped(id); });
}

uint64_t Compositor::onToplevelMapped(const std::string& title, const std::string& appId, int width, int height) {
    Window win;
    win.id = nextWindowId_++;
    win.title = title;
    win.appId = appId;
    // width/height of 0 means the client deferred to us; xdg-shell clients
    // that DO report a real size on first commit will get it respected
    // once renderFrame() actually inspects the attached buffer's pixel
    // size (see the NOTE in XdgShell.cpp's onCommit handler) -- for now
    // every new toplevel gets the same sane default.
    win.size = {
        width > 0 ? static_cast<double>(width) : 800.0,
        height > 0 ? static_cast<double>(height) : 600.0
    };
    win.islandId = 0; // new toplevels start free-floating; promoting one
                       // into a tiling Island is a deliberate user action
                       // (SUPER+I), not a guess made at map time.

    canvasLayout_.placeNewWindow(win, windows_, camera_.position);

    windows_.push_back(win);
    rearrange();

    std::cout << "Mapped \"" << title << "\" (" << appId << ") as window #" << win.id
              << " at (" << win.worldPosition.x << ", " << win.worldPosition.y << ")\n";
    return win.id;
}

void Compositor::onToplevelUnmapped(uint64_t windowId) {
    windows_.erase(
        std::remove_if(windows_.begin(), windows_.end(),
                        [windowId](const Window& w) { return w.id == windowId; }),
        windows_.end());
    rearrange();
}

Window* Compositor::findWindow(uint64_t id) {
    for (auto& w : windows_) {
        if (w.id == id) return &w;
    }
    return nullptr;
}

void Compositor::initBackend() {
    // Requesting headless first, with NULL as an explicit fallback if it
    // fails -- using eBackendRequestMode exactly as its own doc comments
    // describe ("If any IF_AVAILABLE backend fails, use this one"). This
    // matters concretely in constrained VMs: headless can still fail with
    // "no allocator available" if the VM's virtual GPU can't do real
    // buffer allocation, and NULL -- a backend that does nothing at all
    // -- can't hit that failure mode since it never tries to allocate
    // anything. Switch backendType to Aquamarine::AQ_BACKEND_WAYLAND
    // (nested, mandatory) once you want to see an actual window on a real
    // display instead.
    Aquamarine::SBackendImplementationOptions implOptions{};
    implOptions.backendType = Aquamarine::AQ_BACKEND_HEADLESS;
    implOptions.backendRequestMode = Aquamarine::AQ_BACKEND_REQUEST_IF_AVAILABLE;

    Aquamarine::SBackendImplementationOptions fallbackOptions{};
    fallbackOptions.backendType = Aquamarine::AQ_BACKEND_NULL;
    fallbackOptions.backendRequestMode = Aquamarine::AQ_BACKEND_REQUEST_FALLBACK;

    Aquamarine::SBackendOptions options{};
    // CONFIRMED real field, never wired up until now: SBackendOptions has
    // a logFunction. Without this, Aquamarine's own internal diagnostics
    // for exactly this kind of failure (why did start() return false?)
    // were being silently dropped -- we were guessing in the dark when
    // the library was almost certainly already telling us the real
    // reason internally.
    options.logFunction = [](Aquamarine::eBackendLogLevel level, std::string msg) {
        std::cerr << "[Aquamarine] " << msg << "\n";
    };

    backend_ = Aquamarine::CBackend::create({implOptions, fallbackOptions}, options);
    if (!backend_) {
        std::cerr << "Failed to create Aquamarine backend\n";
        return;
    }

    signalListeners_.push_back(backend_->events.newOutput.registerListener([this](std::any data) {
        auto output = std::any_cast<SP<Aquamarine::IOutput>>(data);
        onNewOutput(output);
    }));

    signalListeners_.push_back(backend_->events.newPointer.registerListener([this](std::any data) {
        auto pointer = std::any_cast<SP<Aquamarine::IPointer>>(data);
        onNewPointer(pointer);
    }));

    signalListeners_.push_back(backend_->events.newKeyboard.registerListener([this](std::any data) {
        auto kb = std::any_cast<SP<Aquamarine::IKeyboard>>(data);
        onNewKeyboard(kb);
    }));

    if (!backend_->start()) {
        std::cerr << "Aquamarine backend failed to start\n";
        return;
    }

    // Fold Aquamarine's own pollable fds into loop_ (set up in
    // initWayland(), called before this) so run()'s single
    // wl_event_loop_dispatch() call services both Wayland clients and the
    // backend, instead of needing two separate poll loops merged by hand.
    // for (auto& pollFD : backend_->getPollFDs()) {   // verify accessor name against your header
    //     wl_event_loop_add_fd(loop_, pollFD.fd, WL_EVENT_READABLE,
    //         [](int, uint32_t, void* data) -> int {
    //             static_cast<Aquamarine::CBackend*>(data)->dispatchEvents(); // verify method name too
    //             return 0;
    //         }, backend_.get());
    // }
}

void Compositor::onNewOutput(SP<Aquamarine::IOutput> output) {
    // CONFIRMED against the real Output.hpp: an output does nothing until
    // you enable it and give it a mode via setters on output->state, then
    // commit(). Doing this BEFORE reading state().mode for the Monitor's
    // resolution (unlike the old code, which read it first and always
    // hit the 1920x1080 fallback since nothing had set a mode yet).
    output->state->setEnabled(true);
    if (auto mode = output->preferredMode()) {
        output->state->setMode(mode);
    }
    if (!output->commit()) {
        std::cerr << "onNewOutput: initial commit (enable+mode) failed for \"" << output->name << "\"\n";
    }

    Monitor mon;
    mon.name = output->name;
    mon.resolution = {
        output->state->state().mode ? output->state->state().mode->pixelSize.x : 1920,
        output->state->state().mode ? output->state->state().mode->pixelSize.y : 1080
    };
    monitors_.push_back(mon);

    // Set up somewhere to actually render into. CONFIRMED shape:
    // CSwapchain::create(allocator, backendImpl), reconfigure() with
    // scanout=true (this is what actually reaches the screen, not an
    // offscreen buffer). `swapchain` is a public field directly on
    // IOutput, no separate storage needed on our side.
    auto allocator = output->getBackend()->preferredAllocator();
    output->swapchain = Aquamarine::CSwapchain::create(allocator, output->getBackend());

    Aquamarine::SSwapchainOptions swapchainOpts;
    swapchainOpts.size = {mon.resolution.x, mon.resolution.y};
    swapchainOpts.scanout = true;
    output->swapchain->reconfigure(swapchainOpts);

    signalListeners_.push_back(output->events.frame.registerListener([this, output](std::any) {
        renderFrame(*output);
    }));

    rearrange();
}

void Compositor::onNewPointer(SP<Aquamarine::IPointer> pointer) {
    // CONFIRMED, with a correction from my last attempt: the event really
    // is named `move` (not `motion`), BUT registerListener's parameter is
    // ALWAYS std::function<void(std::any)>, regardless of the signal's
    // template payload type -- the real compiler error spelled this out
    // exactly: `registerListener(std::function<void(std::any d)> handler)`
    // on CSignalT<Args...>, with Args not appearing in that signature at
    // all. My previous "typed lambda" fix was an overcorrection; this is
    // back to std::any_cast, same shape as the backend-level listeners
    // above, just with the right member name and payload type now.
    //
    // ALSO CONFIRMED: registerListener returns
    // Hyprutils::Signal::CHyprSignalListener, a
    // Hyprutils::Memory::CSharedPointer<CSignalListener> -- ref-counted,
    // so the handle must be kept alive or the listener almost certainly
    // disconnects immediately. Stored in signalListeners_ now.
    signalListeners_.push_back(pointer->events.move.registerListener([this](std::any data) {
        auto ev = std::any_cast<Aquamarine::IPointer::SMoveEvent>(data);
        input_.onPointerDrag({ev.delta.x, ev.delta.y});
    }));
}

void Compositor::onNewKeyboard(SP<Aquamarine::IKeyboard> keyboard) {
    // Real build: hook keyboard->events.key, decode via libxkbcommon,
    // and call registerKeybinds()'s dispatch table (SUPER+TAB, SUPER+SPACE,
    // SUPER+1..9 for workspaces, etc).
}

void Compositor::registerKeybinds() {
    // Placeholder dispatch table. In the real build this maps
    // (modmask, keysym) -> std::function<void()> and gets consulted from
    // the keyboard key-event listener above.
    //
    //   SUPER + TAB    -> input_.toggleMode() (drag-to-pan vs drag-to-move-window)
    //   SUPER + SPACE  -> input_.enterBirdsEye(windows_, viewportSize)
    //   SUPER + I      -> wrap the focused window's island-mates into a
    //                     new/existing Island (turn a free cluster tiled)
    //   SUPER + Q      -> close focused window
    //
    // Note islands tile themselves regardless of mode_ now -- mode_ only
    // changes what dragging/scrolling DOES (pan camera vs move a window),
    // not whether any given window is tiled.
}

void Compositor::renderFrame(Aquamarine::IOutput& output) {
    if (!output.swapchain) return; // not set up yet (shouldn't happen -- onNewOutput always creates one)

    int age = 0;
    auto buffer = output.swapchain->next(&age);
    if (!buffer) {
        std::cerr << "renderFrame: swapchain has no buffer available\n";
        return;
    }

    // CONFIRMED: not every buffer is CPU-writable (GPU-tiled/compressed
    // ones aren't). This is exactly the GBM-vs-DRM_DUMB allocator split
    // from Allocator.hpp -- a DRM_DUMB-backed buffer should have this
    // capability; a GBM one might not, depending on modifiers. If this
    // branch is hit constantly, that's the real next thing to solve (a
    // GL/EGL render path), not something to paper over here.
    if (!(buffer->caps() & Aquamarine::BUFFER_CAPABILITY_DATAPTR)) {
        std::cerr << "renderFrame: buffer has no CPU data pointer (GPU-only render path not implemented yet)\n";
        return;
    }

    auto [data, format, byteSize] = buffer->beginDataPtr(0);
    if (!data) {
        std::cerr << "renderFrame: beginDataPtr returned null\n";
        return;
    }

    const int width = static_cast<int>(buffer->size.x);
    const int height = static_cast<int>(buffer->size.y);

    // bpp/byte-order: CONFIRMED correct for XRGB8888/ARGB8888 specifically
    // (drm_fourcc.h's own comment: "[31:0] x:R:G:B 8:8:8:8 little endian"
    // -> bytes in increasing address order are B,G,R,X). Any other format
    // falls back to this same assumption -- UNVERIFIED for anything else,
    // and the first thing to check if colors come out wrong.
    const int bpp = 4;
    const bool knownFormat = (format == DRM_FORMAT_XRGB8888 || format == DRM_FORMAT_ARGB8888);
    if (!knownFormat) {
        std::cerr << "renderFrame: unexpected buffer format 0x" << std::hex << format << std::dec
                  << ", assuming XRGB8888 byte order anyway -- colors may be wrong\n";
    }

    // Stride: prefer whatever the buffer itself reports (padding/alignment
    // is common and width*bpp isn't always right) -- only fall back to
    // width*bpp if neither dmabuf() nor shm() report one. That fallback
    // is a real, unverified guess; if the image comes out sheared
    // diagonally, this is the line to check first.
    int stride = width * bpp;
    if (auto dmaAttrs = buffer->dmabuf(); dmaAttrs.success) {
        stride = dmaAttrs.strides[0];
    } else if (auto shmAttrs = buffer->shm(); shmAttrs.success) {
        stride = shmAttrs.stride;
    }

    auto putPixel = [&](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        uint8_t* px = data + y * stride + x * bpp;
        px[0] = b;
        px[1] = g;
        px[2] = r;
        px[3] = 255;
    };

    // Clear to a dark background.
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            putPixel(x, y, 30, 30, 30);
        }
    }

    // Draw each mapped window as a flat-colored rect at its camera-
    // projected screen position. This is NOT the window's actual client
    // content yet -- that needs reading the client's own surface buffer
    // (WlSurfaceState::currentBuffer in src/wayland/Surface.hpp), which
    // has its own separate format/stride/scaling concerns. This step only
    // proves the output commit pipeline itself actually reaches the
    // screen, which is the real point of doing this incrementally.
    constexpr uint8_t kPalette[][3] = {
        {70, 130, 180}, {180, 90, 70}, {90, 180, 90}, {170, 170, 60}, {150, 90, 180},
    };
    Vec2 viewport{static_cast<double>(width), static_cast<double>(height)};
    int colorIndex = 0;

    for (const auto& win : windows_) {
        if (!win.mapped) continue;

        Vec2 topLeft = camera_.worldToScreen(win.worldPosition, viewport);
        Vec2 bottomRight = camera_.worldToScreen(win.worldPosition + win.size, viewport);

        int x0 = std::clamp(static_cast<int>(topLeft.x), 0, width);
        int y0 = std::clamp(static_cast<int>(topLeft.y), 0, height);
        int x1 = std::clamp(static_cast<int>(bottomRight.x), 0, width);
        int y1 = std::clamp(static_cast<int>(bottomRight.y), 0, height);

        const auto& color = kPalette[colorIndex % (sizeof(kPalette) / sizeof(kPalette[0]))];
        ++colorIndex;

        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                putPixel(x, y, color[0], color[1], color[2]);
            }
        }
    }

    buffer->endDataPtr();

    output.state->setBuffer(buffer);
    output.state->setFormat(format);
    if (!output.commit()) {
        std::cerr << "renderFrame: commit failed, rolling back swapchain\n";
        output.swapchain->rollback();
    }
}

int Compositor::run() {
    initWayland();
    if (!display_) return 1;

    initBackend();
    registerKeybinds();

    auto last = std::chrono::steady_clock::now();
    while (running_) {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - last).count();
        last = now;

        camera_.tick(dt);

        // Services Wayland client requests (xdg-shell etc.) every pass,
        // and -- once initBackend()'s commented-out block above is wired
        // up for your Aquamarine version -- the backend's own fds too,
        // since they share this same loop_. The 16ms timeout caps us near
        // 60Hz; replace with real frame-driven scheduling (wait on each
        // IOutput's `frame` event) once renderFrame() actually draws
        // something, rather than polling on a fixed timer.
        wl_event_loop_dispatch(loop_, 16);
        wl_display_flush_clients(display_);
    }

    wl_display_destroy(display_);
    return 0;
}

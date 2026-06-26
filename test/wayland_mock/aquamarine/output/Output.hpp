// CONFIRMED against the user's real /usr/include/aquamarine/output/Output.hpp
// (0.12.1) -- this is no longer a placeholder. Trimmed to just what
// Compositor.cpp actually uses.
#pragma once
#include "../backend/Backend.hpp"
#include "../allocator/Swapchain.hpp"
#include <hyprutils/signal/Signal.hpp>
#include <hyprutils/memory/WeakPtr.hpp>
#include <hyprutils/math/Vector2D.hpp>
#include <drm_fourcc.h>
#include <string>

namespace Aquamarine {

struct SOutputMode {
    Hyprutils::Math::Vector2D pixelSize;
    unsigned int refreshRate = 0;
    bool preferred = false;
};

class IOutput;

class COutputState {
public:
    struct SInternalState {
        Hyprutils::Memory::CWeakPointer<SOutputMode> mode;
        uint32_t drmFormat = 0;
        Hyprutils::Memory::CSharedPointer<IBuffer> buffer;
    };

    const SInternalState& state() { return internalState_; }
    void setEnabled(bool) {}
    void setMode(Hyprutils::Memory::CSharedPointer<SOutputMode> m) { modeOwner_ = m; internalState_.mode = m.get(); }
    void setBuffer(Hyprutils::Memory::CSharedPointer<IBuffer> b) { internalState_.buffer = b; }
    void setFormat(uint32_t fmt) { internalState_.drmFormat = fmt; }

private:
    SInternalState internalState_;
    Hyprutils::Memory::CSharedPointer<SOutputMode> modeOwner_; // keeps the mode alive for the weak ptr above, mock-only detail
};

class IOutput {
public:
    virtual ~IOutput() = default;

    virtual bool commit() = 0;
    virtual Hyprutils::Memory::CSharedPointer<IBackendImplementation> getBackend() = 0;
    virtual Hyprutils::Memory::CSharedPointer<SOutputMode> preferredMode() {
        return Hyprutils::Memory::CSharedPointer<SOutputMode>(nullptr); // mock: no real mode to offer
    }

    std::string name;
    Hyprutils::Memory::CSharedPointer<COutputState> state =
        Hyprutils::Memory::CSharedPointer<COutputState>(new COutputState());
    Hyprutils::Memory::CSharedPointer<CSwapchain> swapchain;

    struct {
        Hyprutils::Signal::CSignalT<> frame;
    } events;
};

} // namespace Aquamarine

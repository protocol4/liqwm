// CONFIRMED against the user's real /usr/include/aquamarine/allocator/Swapchain.hpp.
#pragma once
#include "Allocator.hpp"
#include <hyprutils/memory/WeakPtr.hpp>

namespace Aquamarine {

class IBackendImplementation;
class IOutput;

struct SSwapchainOptions {
    size_t length = 0;
    Hyprutils::Math::Vector2D size;
    uint32_t format = 0;
    bool scanout = false, cursor = false, multigpu = false;
    Hyprutils::Memory::CWeakPointer<IOutput> scanoutOutput;
};

class CSwapchain {
public:
    static Hyprutils::Memory::CSharedPointer<CSwapchain> create(Hyprutils::Memory::CSharedPointer<IAllocator>,
                                                                  Hyprutils::Memory::CSharedPointer<IBackendImplementation>) {
        return Hyprutils::Memory::CSharedPointer<CSwapchain>(new CSwapchain());
    }

    bool reconfigure(const SSwapchainOptions& opts) { options_ = opts; return true; }
    Hyprutils::Memory::CSharedPointer<IBuffer> next(int* age) {
        if (age) *age = 0;
        return Hyprutils::Memory::CSharedPointer<IBuffer>(nullptr); // mock: no real buffer to hand back
    }
    void rollback() {}
    const SSwapchainOptions& currentOptions() { return options_; }

private:
    SSwapchainOptions options_;
};

} // namespace Aquamarine

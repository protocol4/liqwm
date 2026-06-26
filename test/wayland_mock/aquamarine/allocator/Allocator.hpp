// CONFIRMED against the user's real /usr/include/aquamarine/allocator/Allocator.hpp.
#pragma once
#include <hyprutils/memory/SharedPtr.hpp>
#include "../buffer/Buffer.hpp"

namespace Aquamarine {

class CBackend;
class CSwapchain;

struct SAllocatorBufferParams {
    Hyprutils::Math::Vector2D size;
    uint32_t format = 0;
    bool scanout = false, cursor = false, multigpu = false;
};

enum eAllocatorType {
    AQ_ALLOCATOR_TYPE_GBM = 0,
    AQ_ALLOCATOR_TYPE_DRM_DUMB,
};

class IAllocator {
public:
    virtual ~IAllocator() = default;
    virtual Hyprutils::Memory::CSharedPointer<IBuffer> acquire(const SAllocatorBufferParams&, Hyprutils::Memory::CSharedPointer<CSwapchain>) = 0;
    virtual int drmFD() = 0;
    virtual eAllocatorType type() = 0;
    virtual void destroyBuffers() {}
};

} // namespace Aquamarine

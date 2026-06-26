// CONFIRMED against the user's real /usr/include/aquamarine/buffer/Buffer.hpp.
#pragma once
#include <hyprutils/math/Vector2D.hpp>
#include <array>
#include <tuple>
#include <cstdint>

namespace Aquamarine {

enum eBufferCapability : uint32_t {
    BUFFER_CAPABILITY_NONE = 0,
    BUFFER_CAPABILITY_DATAPTR = (1 << 0),
};

enum eBufferType : uint32_t {
    BUFFER_TYPE_DMABUF = 0,
    BUFFER_TYPE_SHM,
    BUFFER_TYPE_MISC,
};

struct SDMABUFAttrs {
    bool success = false;
    Hyprutils::Math::Vector2D size;
    uint32_t format = 0;
    uint64_t modifier = 0;
    int planes = 1;
    std::array<uint32_t, 4> offsets = {0};
    std::array<uint32_t, 4> strides = {0};
    std::array<int, 4> fds = {-1, -1, -1, -1};
};

struct SSHMAttrs {
    bool success = false;
    int fd = 0;
    uint32_t format = 0;
    Hyprutils::Math::Vector2D size;
    int stride = 0;
    int64_t offset = 0;
};

// Mock returns a fixed "no data" answer for our syntax-check purposes --
// real behavior (does it actually expose a CPU pointer?) isn't something
// this kind of mock can meaningfully fake.
class IBuffer {
public:
    virtual ~IBuffer() = default;
    virtual eBufferCapability caps() = 0;
    virtual eBufferType type() = 0;
    virtual bool good() = 0;
    virtual SDMABUFAttrs dmabuf() { return {}; }
    virtual SSHMAttrs shm() { return {}; }
    virtual std::tuple<uint8_t*, uint32_t, size_t> beginDataPtr(uint32_t) { return {nullptr, 0, 0}; }
    virtual void endDataPtr() {}

    Hyprutils::Math::Vector2D size;
    bool opaque = false;
};

} // namespace Aquamarine

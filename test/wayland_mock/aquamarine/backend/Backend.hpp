// CONFIRMED against the user's real /usr/include/aquamarine/backend/Backend.hpp
// (0.12.1) -- enums, SBackendImplementationOptions/SBackendOptions fields,
// and CBackend::create()'s signature are no longer guesses.
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <any>
#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/signal/Signal.hpp>

namespace Aquamarine {

// Forward-declared only, NOT defined here -- confirmed by the real
// compiler error ("forward declaration of 'class Aquamarine::IOutput'"
// pointing at this exact file/line). Full definitions are in
// aquamarine/output/Output.hpp and aquamarine/input/Input.hpp instead.
class IOutput;
class IPointer;
class IKeyboard;

enum eBackendType : uint32_t {
    AQ_BACKEND_WAYLAND = 0,
    AQ_BACKEND_DRM,
    AQ_BACKEND_HEADLESS,
    AQ_BACKEND_NULL,
};

enum eBackendRequestMode : uint32_t {
    AQ_BACKEND_REQUEST_MANDATORY = 0,
    AQ_BACKEND_REQUEST_IF_AVAILABLE,
    AQ_BACKEND_REQUEST_FALLBACK,
};

enum eBackendLogLevel : uint32_t {
    AQ_LOG_TRACE = 0,
    AQ_LOG_DEBUG,
    AQ_LOG_WARNING,
    AQ_LOG_ERROR,
    AQ_LOG_CRITICAL,
};

struct SBackendImplementationOptions {
    eBackendType backendType{};
    eBackendRequestMode backendRequestMode{};
};

struct SBackendOptions {
    std::function<void(eBackendLogLevel, std::string)> logFunction;
};

class CBackend {
public:
    struct {
        Hyprutils::Signal::CSignalT<Hyprutils::Memory::CSharedPointer<IOutput>> newOutput;
        Hyprutils::Signal::CSignalT<Hyprutils::Memory::CSharedPointer<IPointer>> newPointer;
        Hyprutils::Signal::CSignalT<Hyprutils::Memory::CSharedPointer<IKeyboard>> newKeyboard;
    } events;

    static Hyprutils::Memory::CSharedPointer<CBackend> create(const std::vector<SBackendImplementationOptions>& implementations, const SBackendOptions& options) {
        (void)implementations;
        (void)options;
        return Hyprutils::Memory::CSharedPointer<CBackend>(new CBackend());
    }

    bool start() { return true; }
};

} // namespace Aquamarine

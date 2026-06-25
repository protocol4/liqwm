// Structural mock of Hyprutils::Signal::CSignalT<Args...>, updated to
// match the EXACT signature the real compiler error revealed:
// `CHyprSignalListener registerListener(std::function<void(std::any d)>)`
// -- note Args does NOT appear in registerListener's own signature at
// all, even though the class is templated on it. CHyprSignalListener is
// confirmed to be Hyprutils::Memory::CSharedPointer<CSignalListener>:
// ref-counted, meaning the real API expects callers to keep the handle
// alive (Compositor now does, via signalListeners_).
#pragma once
#include <functional>
#include <vector>
#include <any>
#include <hyprutils/memory/SharedPtr.hpp>

namespace Hyprutils::Signal {

class CSignalListener {};
using CHyprSignalListener = Hyprutils::Memory::CSharedPointer<CSignalListener>;

template <typename... Args>
class CSignalT {
public:
    CHyprSignalListener registerListener(std::function<void(std::any)> handler) {
        listeners_.push_back(std::move(handler));
        return CHyprSignalListener(new CSignalListener());
    }
    void emit(Args... args) {
        (void)sizeof...(args); // real impl packs into std::any; not needed for this syntax check
    }
private:
    std::vector<std::function<void(std::any)>> listeners_;
};

} // namespace Hyprutils::Signal

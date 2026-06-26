// Structural mock of the real <hyprutils/memory/WeakPtr.hpp>. Confirmed
// to exist (real Backend.hpp/Output.hpp both #include it) and that
// SOutputMode is held via CWeakPointer, not CSharedPointer, in
// SInternalState::mode.
#pragma once

namespace Hyprutils::Memory {

template <typename T>
class CWeakPointer {
public:
    CWeakPointer() = default;
    CWeakPointer(T* ptr) : ptr_(ptr) {}
    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }
    T* get() const { return ptr_; }
private:
    T* ptr_ = nullptr;
};

} // namespace Hyprutils::Memory

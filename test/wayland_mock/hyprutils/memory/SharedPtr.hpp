// Structural mock of the real <hyprutils/memory/SharedPtr.hpp>, just
// enough to syntax-check Compositor.hpp/.cpp's own `SP<T>` alias and
// usage. The real CSharedPointer is reference-counted; this mock is a
// bare pointer wrapper -- fine for a syntax check, not a semantics check.
#pragma once

namespace Hyprutils::Memory {

template <typename T>
class CSharedPointer {
public:
    CSharedPointer() = default;
    CSharedPointer(T* ptr) : ptr_(ptr) {}
    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }
    T* get() const { return ptr_; }
private:
    T* ptr_ = nullptr;
};

} // namespace Hyprutils::Memory

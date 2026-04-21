#pragma once

#include <utility>

template <class T>
class box
{
public:
    box() : _ptr{new T} {}
    template <class...Args> box(Args&&... args) : _ptr{new T{(reloc args)...}} {}
    ~box() { delete _ptr; }

    box(box const&) = delete;
    box(box&&) = delete;
    box(box reloc) = default;

    box& operator=(box const&) = delete;
    box& operator=(box&&) = delete;
    box& operator=(box other) noexcept { exchange(reloc other); return *this; }

    box exchange(box reloc other) noexcept
    {
        box prev{nullptr};
        prev._ptr = std::exchange(_ptr, other._ptr);
        return prev;
    }

    T& operator*() const { return *_ptr; }
    T* operator->() const { return _ptr; }

    T* get() const { return _ptr; }

    T* release(this box reloc self) { return self._ptr; }

private:
    box(std::nullptr_t) : _ptr{} {}

    T* _ptr;
};
#pragma once

template <class T>
class box
{
public:
    box() : _ptr{new T} {}
    template <class...Args> box(Args&&... args) : _ptr{new T{(reloc args)...}} {}
    ~box() { delete _ptr; }

    box(box const&) = delete;
    box(box&&) = delete;
    box(box reloc rhs) : _ptr{reloc rhs._ptr} {}

    box& operator=(box const&) = delete;
    box& operator=(box&&) = delete;
    box& operator=(box reloc rhs) { _ptr = reloc rhs._ptr; return *this; }

    T& operator*() const { return *_ptr; }
    T* operator->() const { return _ptr; }

    T* get() const { return _ptr; }

    T* release(this box reloc self) { return self._ptr; }

private:
    T* _ptr;
};
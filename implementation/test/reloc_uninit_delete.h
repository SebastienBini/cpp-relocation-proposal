// reloc_uninit_delete.h — memory-safe wrapper around std::reloc_and_uninitialize.
//
// std::reloc_and_uninitialize leaves the source memory unfreed (by design).
// This wrapper frees it afterward, including on the exception path, so that
// tests using reloc_and_uninitialize are valgrind-clean.
//
// For polymorphic types with virtual bases, the base pointer may differ from
// the allocation pointer. We use dynamic_cast<void*> before relocation to
// recover the most-derived (= allocation) pointer.
#ifndef RELOC_UNINIT_DELETE_H
#define RELOC_UNINIT_DELETE_H

#include <memory>
#include <new>
#include <type_traits>

template <class T>
std::remove_cv_t<T> reloc_uninit_and_delete(T* src) {
    // For polymorphic types, dynamic_cast<void*> returns the most-derived
    // object address, which is the pointer that new returned.
    // For non-polymorphic types, src is always the allocation pointer.
    void* alloc_ptr;
    if constexpr (std::is_polymorphic_v<T>)
        alloc_ptr = dynamic_cast<void*>(src);
    else
        alloc_ptr = static_cast<void*>(src);

    struct Guard {
        void* p;
        ~Guard() { ::operator delete(p); }
    } guard{alloc_ptr};
    return std::reloc_and_uninitialize(src);
}

#endif

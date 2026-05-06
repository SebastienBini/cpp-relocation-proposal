// virtual-slicing-022: Verify destruction order in V-is-virtual shortcut
// when non-virtual bases exist both before and after the virtual base.
//
// Hierarchy: T : PreV (non-virtual), virtual V, PostV (non-virtual)
// All have snooping fields. Slicing T* -> V* should destroy in this order:
//   1. T's fields (reverse decl order)
//   2. All non-virtual bases (reverse base-specifier order: PostV, PreV)
//   3. Move-construct V into dest, destroy moved-from V
// This matches the destructor's subobject destruction order.

#include <iostream>
#include <memory>
#include "snoop.h"

struct V {
    snoop v1{"v1"};
    V() { std::cout << "V() " << this << std::endl; }
    V(V&& src) : v1(std::move(src.v1)) {
        std::cout << "V(move) " << this << " <- " << &src << std::endl;
    }
    V(V reloc src) : v1(reloc src.v1) {
        std::cout << "V(reloc) " << this << " <- " << src.this << std::endl;
    }
    virtual ~V() = default;
};

struct PreV {
    snoop pre{"pre"};
    PreV() { std::cout << "PreV() " << this << std::endl; }
    ~PreV() { std::cout << "~PreV() " << this << std::endl; }
};

struct PostV {
    snoop post{"post"};
    PostV() { std::cout << "PostV() " << this << std::endl; }
    ~PostV() { std::cout << "~PostV() " << this << std::endl; }
};

struct T : PreV, virtual V, PostV {
    snoop t1{"t1"};
    snoop t2{"t2"};
    T() { std::cout << "T() " << this << std::endl; }
    T(T reloc src)
        : V(std::move(src.base<V>))
        , PreV(reloc src.base<PreV>)
        , PostV(reloc src.base<PostV>)
        , t1(reloc src.t1)
        , t2(reloc src.t2) {
        std::cout << "T(reloc) " << this << " <- " << src.this << std::endl;
    }
    ~T() override = default;
};

int main() {
    std::cout << "---construct---" << std::endl;
    T* obj = new T();
    std::cout << "--- constructed ---" << std::endl;

    V* vp = obj;
    std::cout << "---slice T* to V* (V-is-virtual shortcut)---" << std::endl;
    V result = std::reloc_and_uninitialize(vp);
    std::cout << "result.v1=" << result.v1.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// v1() 0x1
// V() 0x2
// pre() 0x3
// PreV() 0x3
// post() 0x4
// PostV() 0x4
// t1() 0x5
// t2() 0x6
// T() 0x7
// --- constructed ---
// ---slice T* to V* (V-is-virtual shortcut)---
// ~t2() 0x6
// ~t1() 0x5
// ~PostV() 0x4
// ~post() 0x4
// ~PreV() 0x3
// ~pre() 0x3
// v1(v1&&) 0x8 <- 0x1
// V(move) 0x9 <- 0x2
// ~v1() 0x1
// result.v1=v1
// ---end---
// ~v1() 0x8

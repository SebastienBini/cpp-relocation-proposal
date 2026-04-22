#include "snoop.h"

#include <utility>

struct VBase
{
    VBase() : vb_m{"vb_m"} { std::cout << "VBase() " << this << std::endl; }
    VBase(VBase const& rhs) : vb_m(rhs.vb_m) { std::cout << "VBase(VBase const&) " << this << " <- " << &rhs << std::endl; }
    VBase(VBase&& rhs) : vb_m(std::move(rhs.vb_m)) { std::cout << "VBase(VBase&&) " << this << " <- " << &rhs << std::endl; }
    VBase(VBase reloc rhs) : vb_m(reloc rhs.vb_m) { std::cout << "VBase(VBase reloc) " << this << " <- " << rhs.this << std::endl; }
    VBase& operator=(VBase const& rhs) { vb_m = static_cast<snoop const&>(rhs.vb_m); std::cout << "VBase::operator=(VBase const&) " << this << " = " << &rhs << std::endl; return *this; }
    VBase& operator=(VBase&& rhs) { vb_m = static_cast<snoop&&>(rhs.vb_m); std::cout << "VBase::operator=(VBase&&) " << this << " = " << &rhs << std::endl; return *this; }
    VBase& operator=(VBase reloc rhs) { vb_m = reloc rhs.vb_m; std::cout << "VBase::operator=(VBase reloc) " << this << " = " << rhs.this << std::endl; return *this; }
    ~VBase() { std::cout << "~VBase() " << this << std::endl; }
    snoop vb_m;
    friend void decomp_vb(VBase reloc);
};

struct B1 : public virtual VBase
{
    B1() : VBase(), b1_m{"b1_m"} { std::cout << "B1() " << this << std::endl; }
    B1(B1 const& rhs) : VBase(rhs), b1_m(rhs.b1_m) { std::cout << "B1(B1 const&) " << this << " <- " << &rhs << std::endl; }
    B1(B1&& rhs) : VBase(std::move(rhs)), b1_m(std::move(rhs.b1_m)) { std::cout << "B1(B1&&) " << this << " <- " << &rhs << std::endl; }
    B1(B1 reloc rhs) : VBase(std::move(rhs.base<VBase>)), b1_m(reloc rhs.b1_m) { std::cout << "B1(B1 reloc) " << this << " <- " << rhs.this << std::endl; }
    B1& operator=(B1 const& rhs) { VBase::operator=(static_cast<VBase const&>(rhs)); b1_m = static_cast<snoop const&>(rhs.b1_m); std::cout << "B1::operator=(B1 const&) " << this << " = " << &rhs << std::endl; return *this; }
    B1& operator=(B1&& rhs) { VBase::operator=(static_cast<VBase&&>(rhs)); b1_m = static_cast<snoop&&>(rhs.b1_m); std::cout << "B1::operator=(B1&&) " << this << " = " << &rhs << std::endl; return *this; }
    B1& operator=(B1 reloc rhs) { VBase::operator=(std::move(rhs.base<VBase>)); b1_m = reloc rhs.b1_m; std::cout << "B1::operator=(B1 reloc) " << this << " = " << rhs.this << std::endl; return *this; }
    ~B1() { std::cout << "~B1() " << this << std::endl; }
    snoop b1_m;
    friend void decomp_b1(B1 reloc);
};
struct B2 : public virtual VBase
{
    B2() : VBase(), b2_m{"b2_m"} { std::cout << "B2() " << this << std::endl; }
    B2(B2 const& rhs) : VBase(rhs), b2_m(rhs.b2_m) { std::cout << "B2(B2 const&) " << this << " <- " << &rhs << std::endl; }
    B2(B2&& rhs) : VBase(std::move(rhs)), b2_m(std::move(rhs.b2_m)) { std::cout << "B2(B2&&) " << this << " <- " << &rhs << std::endl; }
    B2(B2 reloc rhs) : VBase(std::move(rhs.base<VBase>)), b2_m(reloc rhs.b2_m) { std::cout << "B2(B2 reloc) " << this << " <- " << rhs.this << std::endl; }
    B2& operator=(B2 const& rhs) { VBase::operator=(static_cast<VBase const&>(rhs)); b2_m = static_cast<snoop const&>(rhs.b2_m); std::cout << "B2::operator=(B2 const&) " << this << " = " << &rhs << std::endl; return *this; }
    B2& operator=(B2&& rhs) { VBase::operator=(static_cast<VBase&&>(rhs)); b2_m = static_cast<snoop&&>(rhs.b2_m); std::cout << "B2::operator=(B2&&) " << this << " = " << &rhs << std::endl; return *this; }
    B2& operator=(B2 reloc rhs) { VBase::operator=(std::move(rhs.base<VBase>)); b2_m = reloc rhs.b2_m; std::cout << "B2::operator=(B2 reloc) " << this << " = " << rhs.this << std::endl; return *this; }
    ~B2() { std::cout << "~B2() " << this << std::endl; }
    snoop b2_m;
    friend void decomp_b2(B2 reloc);
};
struct C 
{
    C() : c_m{"c_m"} { std::cout << "C() " << this << std::endl; }
    C(C const& rhs) : c_m(rhs.c_m) { std::cout << "C(C const&) " << this << " <- " << &rhs << std::endl; }
    C(C&& rhs) : c_m(std::move(rhs.c_m)) { std::cout << "C(C&&) " << this << " <- " << &rhs << std::endl; }
    C(C reloc rhs) : c_m(reloc rhs.c_m) { std::cout << "C(C reloc) " << this << " <- " << rhs.this << std::endl; }
    C& operator=(C const& rhs) { c_m = static_cast<snoop const&>(rhs.c_m); std::cout << "C::operator=(C const&) " << this << " = " << &rhs << std::endl; return *this; }
    C& operator=(C&& rhs) { c_m = static_cast<snoop&&>(rhs.c_m); std::cout << "C::operator=(C&&) " << this << " = " << &rhs << std::endl; return *this; }
    C& operator=(C reloc rhs) { c_m = reloc rhs.c_m; std::cout << "C::operator=(C reloc) " << this << " = " << rhs.this << std::endl; return *this; }
    ~C() { std::cout << "~C() " << this << std::endl; }
    snoop c_m;
    friend void decomp_c(C reloc);
};

struct D : public B1, public B2, public C 
{
    D() : VBase(), B1(), B2(), C(), d_m{"d_m"} { std::cout << "D() " << this << std::endl; }
    D(D const& rhs) : VBase(static_cast<VBase const&>(rhs)), B1(static_cast<B1 const&>(rhs)), B2(static_cast<B2 const&>(rhs)), C(static_cast<C const&>(rhs)), d_m(rhs.d_m) { std::cout << "D(D const&) " << this << " <- " << &rhs << std::endl; }
    D(D&& rhs) : VBase(static_cast<VBase&&>(rhs)), B1(static_cast<B1&&>(rhs)), B2(static_cast<B2&&>(rhs)), C(static_cast<C&&>(rhs)), d_m(std::move(rhs.d_m)) { std::cout << "D(D&&) " << this << " <- " << &rhs << std::endl; }
    D(D reloc rhs) : VBase(std::move(rhs.base<VBase>)), B1(reloc rhs.base<B1>), B2(reloc rhs.base<B2>), C(reloc rhs.base<C>), d_m(reloc rhs.d_m) { std::cout << "D(D reloc) " << this << " <- " << rhs.this << std::endl; }
    D& operator=(D const& rhs) { B1::operator=(static_cast<B1 const&>(rhs)); B2::operator=(static_cast<B2 const&>(rhs)); C::operator=(static_cast<C const&>(rhs)); d_m = static_cast<snoop const&>(rhs.d_m); std::cout << "D::operator=(D const&) " << this << " = " << &rhs << std::endl; return *this; }
    D& operator=(D&& rhs) { B1::operator=(static_cast<B1&&>(rhs)); B2::operator=(static_cast<B2&&>(rhs)); C::operator=(static_cast<C&&>(rhs)); d_m = static_cast<snoop&&>(rhs.d_m); std::cout << "D::operator=(D&&) " << this << " = " << &rhs << std::endl; return *this; }
    D& operator=(D reloc rhs) { B1::operator=(reloc rhs.base<B1>); B2::operator=(reloc rhs.base<B2>); C::operator=(reloc rhs.base<C>); d_m = reloc rhs.d_m; std::cout << "D::operator=(D reloc) " << this << " = " << rhs.this << std::endl; return *this; }
    ~D() { std::cout << "~D() " << this << std::endl; }
    snoop d_m;
    friend void decomp_d(D reloc);
    friend B1 getB1(D reloc d);
    template <class base> friend base getBase(D reloc d);
};

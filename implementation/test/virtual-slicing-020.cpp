// virtual-slicing-020: Compile-time trait checks — deleted reloc ctor,
// various positive/negative cases.

struct A {
    A();
    A(A reloc);
    virtual ~A() = default;
};
static_assert(__has_virtual_slicing_function(A), "");

struct B {
    B();
    B(B reloc);
    ~B() = default; // no virtual dtor
};
static_assert(!__has_virtual_slicing_function(B), "no virtual dtor");

struct C {
    C();
    virtual ~C() = default; // no explicit reloc ctor, no base with slicing fn
};
static_assert(!__has_virtual_slicing_function(C), "no reloc ctor");

struct D : A {
    D();
    D(D reloc);
};
static_assert(__has_virtual_slicing_function(D), "inherits from A");

struct E : A {
    E(); // no explicit reloc ctor — Phase 12e gives implicit one
};
static_assert(__has_virtual_slicing_function(E), "Phase 12e");

struct F {
    F();
    F(F reloc) = delete;
    virtual ~F() = default;
};
static_assert(!__has_virtual_slicing_function(F), "deleted reloc ctor");

struct G : D {
    G();
    G(G reloc);
};
static_assert(__has_virtual_slicing_function(G), "multi-level");

template<typename T>
struct Poly {
    T val;
    Poly() : val{} {}
    Poly(Poly reloc) : val{} {}
    virtual ~Poly() = default;
};
static_assert(__has_virtual_slicing_function(Poly<int>), "");
static_assert(__has_virtual_slicing_function(Poly<double>), "");

int main() { return 0; }

////// BUILD SUCCESS

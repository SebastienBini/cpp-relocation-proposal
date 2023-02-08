Following on from structured relocation and destructuring member functions, a possible deficiency arises in
accessing subobjects of prvalues of tuple-like types. Supposing we have a `std::pair` where a member (say `first`) is of
a relocate-only type, `std::get<0>` would be able to relocate from that data member, so accessing it by name should
similarly relocate that member while (pseudo)-destroying the other member:

```c++
std::pair<gsl::not_null<std::unique_ptr<int>>, bool> p1 = f(), p2 = f();
if (p1.second and p2.second) {
    auto q1 = std::get<0>(reloc p1); // OK
    auto q2 = (reloc p2).first; // also OK
}
```

Naturally, this should hold for all aggregate class types, not only those which participate in the tuple-like protocol;
however, it should be restricted to ensure that destructors (of the complete-object class type) are not bypassed and
that any invariants holding between the data member relocated from and other data members are not broken. This can be
accomplished by requiring that every class type whose subobject is relocated from has no user-declared special member
function.

Next, there is the question of multi-level access (`(reloc a).x.y[1]`); the rules for lifetime extension by reference
binding form an adequate parallel.

Finally, as well as relocating data member and array element subobjects, the facility should also work for base class
subobjects.

Considering all this, we adjust lvalue-to-rvalue conversion thus: when lvalue-to-rvalue conversion is performed on an
xvalue, if that xvalue is a subobject of a materialized temporary object, and if reference binding to that xvalue would
extend the lifetime of that temporary object, and if every containing object of that xvalue has no user-declared
destructor, copy/move/relocation constructor, or copy/move/relocation assignment operator, then said xvalue subobject is
instead considered a prvalue, with initialization of the destination rvalue performed in sequence with destruction of
other subobjects, in the usual order. (Note that a call to the relocation constructor of the class type of the xvalue
subobject may be elided; nevertheless, the other subobjects are still destroyed at that time.) Example:

```c++
struct A {
    int i;
    ~A() { std::print("~A({}) ", i); }
    A(A) { std::print("A(A:{}) ", i); }
};
int main() {
    A as[3] = {A{0}, A{1}, A{2}};
    auto a1 = (reloc as)[1];
    // Expected output: ~A(0) A(A:1) ~A(2) ~A(1)
    // or: ~A(0) ~A(2) ~A(1) (if relocation construction of `a1` is elided)
}
```

In this example, `&a1 + 1` is a past-the-end pointer of the implicit singular array of which every object not explicitly
an array element is a member. It is *not* a pointer to the destroyed `as[2]`, since `a1` is not a subobject of the
(destroyed) array `as`, even though it may (if elision occurred) have the same address as the destroyed `as[1]`.

There is an open question of what to do if some other subobject of the immediate containing object of the xvalue
subobject is inaccessible (or has a destructor inaccessible) from the context performing the lvalue-to-rvalue
conversion. We see three options:

1. Make the code ill-formed;
2. Do not treat the xvalue as a prvalue, instead performing construction of the destination rvalue from the xvalue;
3. Proceed anyway.

The first option could break existing code, so should be avoided. The second option seems fragile and unpredictable, so
we favor the third option. The defensible aspect is that the containing objects by implication all have trivial
destructors, with the full-object having an accessible trivial destructor, so the sequence of destruction is
effectively performed by the context anyway. Example:

```c++
struct A { private: A(); ~A(); friend struct X; };
struct X {
    A a = {};
    int i = 1;
};
int main() {
    X x;
    int i = (reloc x).i;  // OK; calls x.a.~A().
}
```

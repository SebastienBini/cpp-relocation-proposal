struct B { int b; };
struct D : B { int d; };

void foo1(D reloc d)
{
    reloc d.base<B>;
    (void)d.b;
}

void foo2(D reloc d)
{
    constexpr auto pb = &D::b;
    (void)(d.*pb);
    reloc d.base<B>;
    (void)(d.*pb);
}

void foo3(D reloc d)
{
    constexpr auto pb = &D::b;
    reloc d.*pb;
}

void foo4(D reloc d)
{
    constexpr auto pd = &D::d;
    reloc d.*pd;
    (void)d.d;
}

void foo5(D reloc d)
{
    constexpr auto pd = &D::d;
    reloc d.d;
    (void)(d.*pd);
}

void foo6(D reloc d)
{
    constexpr auto pd = &D::d;
    reloc d.*pd;
    (void)(d.*pd);
}

template <class base, auto mptr>
void foo7helper(D reloc d)
{
    reloc d.base<base>;
    (void)(d.*mptr);
}

void foo7()
{
    foo7helper<B, &D::d>(D{}); // should be okay
    foo7helper<B, &D::b>(D{}); // ill-formed
}

////// BUILD FAILURE
// decomposition-010.cpp:7:13: error: use of 'd.b' after base 'B' has been relocated
//     7 |     (void)d.b;
//       |             ^
// decomposition-010.cpp:6:5: note: base subobject relocated here
//     6 |     reloc d.base<B>;
//       |     ^
// decomposition-010.cpp:15:13: error: use of 'd.b' after base 'B' has been relocated
//    15 |     (void)(d.*pb);
//       |             ^
// decomposition-010.cpp:14:5: note: base subobject relocated here
//    14 |     reloc d.base<B>;
//       |     ^
// decomposition-010.cpp:21:11: error: cannot relocate 'b': it is declared in base class 'B'; use 'reloc obj.base<B>' to relocate the base subobject instead
//    21 |     reloc d.*pb;
//       |           ^
// decomposition-010.cpp:28:13: error: use of 'd.d' after the member has been relocated
//    28 |     (void)d.d;
//       |             ^
// decomposition-010.cpp:27:5: note: member relocated here
//    27 |     reloc d.*pd;
//       |     ^
// decomposition-010.cpp:35:13: error: use of 'd.d' after the member has been relocated
//    35 |     (void)(d.*pd);
//       |             ^
// decomposition-010.cpp:34:5: note: member relocated here
//    34 |     reloc d.d;
//       |     ^
// decomposition-010.cpp:42:13: error: use of 'd.d' after the member has been relocated
//    42 |     (void)(d.*pd);
//       |             ^
// decomposition-010.cpp:41:5: note: member relocated here
//    41 |     reloc d.*pd;
//       |     ^
// decomposition-010.cpp:49:13: error: use of 'd.b' after base 'B' has been relocated
//    49 |     (void)(d.*mptr);
//       |             ^
// decomposition-010.cpp:55:5: note: in instantiation of function template specialization 'foo7helper<B, &B::b>' requested here
//    55 |     foo7helper<B, &D::b>(D{}); // ill-formed
//       |     ^
// decomposition-010.cpp:48:5: note: base subobject relocated here
//    48 |     reloc d.base<base>;
//       |     ^
// 7 errors generated.

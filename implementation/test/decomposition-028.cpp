void testWellformed01()
{
    struct base
    {
    private:
        int b{10};
    };
    struct D : base
    {
        int d{20};
    };

    D reloc obj;
}

void testWellformed02()
{
    struct base
    {
    protected:
        int b{10};
    };
    struct D : base
    {
        int d{20};
    };

    D reloc obj;
}

void testWellformed03()
{
    struct base
    {
        ~base();
        int b{10};
    };
    struct D : base
    {
        int d{20};
    };

    D reloc obj;
}

void testIllformed01()
{
    struct base
    {
        int b{10};
    };
    struct D : base
    {
    private:
        int d{20};
    };

    D reloc obj;
}

void testIllformed02()
{
    struct base
    {
        int b{10};
    };
    struct D : base
    {
    protected:
        int d{20};
    };

    D reloc obj;
}

void testIllformed03()
{
    struct base
    {
        int b{10};
    };
    struct D : base
    {
        ~D();
        int d{20};
    };

    D reloc obj;
}

void testIllformed04()
{
    struct base
    {
        int b{10};
    };
    struct D : private base
    {
        int d{20};
    };

    D reloc obj;
}

void testIllformed05()
{
    struct base
    {
        int b{10};
    };
    struct D : protected base
    {
        int d{20};
    };

    D reloc obj;
}

////// BUILD FAILURE
// decomposition-028.cpp:58:7: error: cannot decompose type 'D': private member 'd' is not accessible
//    58 |     D reloc obj;
//       |       ^
// decomposition-028.cpp:55:13: note: declared private here
//    55 |         int d{20};
//       |             ^
// decomposition-028.cpp:73:7: error: cannot decompose type 'D': protected member 'd' is not accessible
//    73 |     D reloc obj;
//       |       ^
// decomposition-028.cpp:70:13: note: declared protected here
//    70 |         int d{20};
//       |             ^
// decomposition-028.cpp:88:7: error: cannot decompose an object of type 'D' with a user-provided destructor from outside the class or its friends; move individual members instead
//    88 |     D reloc obj;
//       |       ^
// decomposition-028.cpp:102:7: error: cannot decompose type 'D': base class 'base' is not accessible
//   102 |     D reloc obj;
//       |       ^
// decomposition-028.cpp:97:16: note: declared private here
//    97 |     struct D : private base
//       |                ^~~~~~~~~~~~
// decomposition-028.cpp:116:7: error: cannot decompose type 'D': base class 'base' is not accessible
//   116 |     D reloc obj;
//       |       ^
// decomposition-028.cpp:111:16: note: declared protected here
//   111 |     struct D : protected base
//       |                ^~~~~~~~~~~~~~
// 5 errors generated.

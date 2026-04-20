#include "snoop.h"

struct base
{
    virtual ~base() = default;
    virtual void print() { std::cout << "base print" << std::endl; }
    int b;
};
struct D : base
{
    void print() override { std::cout << "D print" << std::endl; }
    int d;
};

int main(int, char**)
{
    D obj;
    std::cout << "obj " << &obj << std::endl;
    std::cout << "base " << &static_cast<base&>(obj) << std::endl;
    std::cout << "obj.b " << &obj.b << std::endl;
    std::cout << "obj.d " << &obj.d << std::endl;
    std::cout << typeid(static_cast<base&>(obj)).name() << std::endl;
    D reloc d_obj = reloc obj; // relocation elision
    std::cout << "d_obj.this " << d_obj.this << std::endl;
    std::cout << "d_obj.base " << &d_obj.base<base> << std::endl;
    std::cout << typeid(d_obj.base<base>).name() << std::endl;
    std::cout << "d_obj.b " << &d_obj.b << std::endl;
    std::cout << "d_obj.d " << &d_obj.d << std::endl;
    d_obj.base<base>.print();

    return 0;
}

////// BUILD SUCCESS
// obj 0x1
// base 0x1
// obj.b 0x2
// obj.d 0x3
// 1D
// d_obj.this 0x1
// d_obj.base 0x1
// 4base
// d_obj.b 0x2
// d_obj.d 0x3
// base print

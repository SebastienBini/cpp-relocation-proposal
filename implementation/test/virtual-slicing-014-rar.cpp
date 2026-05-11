// virtual-slicing-014: Polymorphic container - relocating objects of different
// dynamic types through a common base pointer using std::reloc_and_reclaim.

#include <iostream>
#include <memory>
#include "snoop.h"

struct Animal {
    snoop a1{"a1"};
    snoop a2{"a2"};
    Animal() { std::cout << "Animal()" << std::endl; }
    Animal(Animal reloc src) : a1(reloc src.a1), a2(reloc src.a2) {
        std::cout << "Animal(reloc)" << std::endl;
    }
    virtual ~Animal() = default;
    virtual const char* speak() const { return "..."; }
};

struct Dog : Animal {
    snoop dog1{"dog1"};
    snoop dog2{"dog2"};
    Dog() { std::cout << "Dog()" << std::endl; }
    Dog(Dog reloc src) : Animal(reloc src.base<Animal>), dog1(reloc src.dog1), dog2(reloc src.dog2) {
        std::cout << "Dog(reloc)" << std::endl;
    }
    ~Dog() override = default;
    const char* speak() const override { return "Woof"; }
};

struct Cat : Animal {
    snoop cat1{"cat1"};
    snoop cat2{"cat2"};
    Cat() { std::cout << "Cat()" << std::endl; }
    Cat(Cat reloc src) : Animal(reloc src.base<Animal>), cat1(reloc src.cat1), cat2(reloc src.cat2) {
        std::cout << "Cat(reloc)" << std::endl;
    }
    ~Cat() override = default;
    const char* speak() const override { return "Meow"; }
};

int main() {
    std::cout << "---construct---" << std::endl;
    Dog* dog = new Dog();
    std::cout << "dog constructed" << std::endl;
    Cat* cat = new Cat();
    std::cout << "cat constructed" << std::endl;

    std::cout << "---relocate exact types---" << std::endl;
    Dog dog2 = std::reloc_and_reclaim(dog);
    std::cout << "dog relocated" << std::endl;
    Cat cat2 = std::reloc_and_reclaim(cat);
    std::cout << "cat relocated" << std::endl;

    std::cout << "---verify---" << std::endl;
    std::cout << dog2.speak() << " dog1=" << dog2.dog1.name << std::endl;
    std::cout << cat2.speak() << " cat1=" << cat2.cat1.name << std::endl;
    std::cout << "---end---" << std::endl;
    return 0;
}

////// BUILD SUCCESS
// ---construct---
// a1() 0x1
// a2() 0x2
// Animal()
// dog1() 0x3
// dog2() 0x4
// Dog()
// dog constructed
// a1() 0x5
// a2() 0x6
// Animal()
// cat1() 0x7
// cat2() 0x8
// Cat()
// cat constructed
// ---relocate exact types---
// a1(a1 reloc) 0x9 <- 0x1
// a2(a2 reloc) 0x10 <- 0x2
// Animal(reloc)
// dog1(dog1 reloc) 0x11 <- 0x3
// dog2(dog2 reloc) 0x12 <- 0x4
// Dog(reloc)
// dog relocated
// a1(a1 reloc) 0x13 <- 0x5
// a2(a2 reloc) 0x14 <- 0x6
// Animal(reloc)
// cat1(cat1 reloc) 0x15 <- 0x7
// cat2(cat2 reloc) 0x16 <- 0x8
// Cat(reloc)
// cat relocated
// ---verify---
// Woof dog1=dog1
// Meow cat1=cat1
// ---end---
// ~cat2() 0x16
// ~cat1() 0x15
// ~a2() 0x14
// ~a1() 0x13
// ~dog2() 0x12
// ~dog1() 0x11
// ~a2() 0x10
// ~a1() 0x9

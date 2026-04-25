#pragma once

#include <string_view>
#include <iostream>

struct snoop
{
    snoop() : snoop{"snoop"} {}
    snoop(std::string_view name) : name{name} { std::cout << name << "()" << " " << this << std::endl; }
    snoop(snoop const& rhs) : name(rhs.name) { std::cout << name << "(" << name << " const&)" << "; " << this << " <- " << &rhs << std::endl; }
    snoop(snoop&& rhs) : name(rhs.name) { std::cout << name << "(" << name << "&&)" << " " << this << " <- " << &rhs << std::endl; }
    snoop(snoop reloc rhs) : name(reloc rhs.name) { std::cout << name << "(" << name << " reloc)" << " " << this << " <- " << rhs.this << std::endl; }
    snoop& operator=(snoop const& rhs) { name = rhs.name; std::cout << name << "& " << name << "::operator=(" << name << " const&)" << " " << this << " <- " << &rhs << std::endl; return *this; }
    snoop& operator=(snoop&& rhs) { name = rhs.name;  std::cout << name << "& " << name << "::operator=(" << name << "&&)" << " " << this << " <- " << &rhs << std::endl; return *this; }
    snoop& operator=(snoop reloc rhs) { name = reloc rhs.name;  std::cout << name << "& " << name << "::operator=(" << name << " reloc)" << " " << this << " <- " << rhs.this << std::endl; return *this; }
    ~snoop() { std::cout << '~' << name << "() " << this << std::endl; }

    std::string_view name;
};
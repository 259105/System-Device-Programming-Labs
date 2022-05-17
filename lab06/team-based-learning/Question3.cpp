#include <iostream>
#include <list>

class X {

public: //the five operations are overloaded
    //constructors
    X()  { std::cout << "dc " << std::endl; } //default constructor dc
    X(const X &) {  std::cout << "cc" << std::endl; } //copy constructor cc
    X(X &&) noexcept { std::cout << "mc" << std::endl; }; //move constructor mc
    //assignments
    X &operator=(const X &) { std::cout << "ca" << std::endl; return *this;} //copy assignment ca
    X &operator=(X &&) {std::cout << "ma" << std::endl; return *this;} //move assignment ma
    //destructor
    ~X() { std::cout << "d" << std::endl; } //destructor d
};

void f1(X x) { }

void f2(X &x) { }


int main() {
    std::cout << "\n----- X x1;\n";
    X x1;

    std::cout << "\n----- f1(x1);\n";
    f1(x1);

    std::cout << "\n----- f2(x1);\n";
    f2(x1);

    std::cout << "\n----- X *x2 = new X;\n";
    X *x2 = new X;

    std::cout << "\n----- X *x3; x3 = (std::move(x2));\n";
    X x3;
    x3 = (std::move(x1));

    std::cout << "\n----- END PROGRAM\n";
    return 0;
}



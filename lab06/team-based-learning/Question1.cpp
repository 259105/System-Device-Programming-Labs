#include <iostream>
#include <list>

class Y {

public: //the five operations are overloaded
    //constructors
    Y()  { std::cout << "dc " << std::endl; } //default constructor dc
    Y(const Y &) {  std::cout << "cc" << std::endl; } //copy constructor cc
    Y(Y &&) noexcept { std::cout << "mc" << std::endl; }; //move constructor mc
    //assignments
    Y &operator=(const Y &) { std::cout << "ca" << std::endl; return *this;} //copy assignment ca
    Y &operator=(Y &&) {std::cout << "ma" << std::endl; return *this;} //move assignment ma
    //destructor
    ~Y() { std::cout << "d" << std::endl; } //destructor d
};

void f1(Y y) { }

void f2(Y &y) { }


int main() {
    std::cout << "\n----- Y y1 ; \n";
    Y y1;

    std::cout << "\n-----  Y y2=y1; \n";
    Y y2=y1;

    std::cout << "\n----- f1(y1);\n";
    f1(y1);

    std::cout << "\n-----  f1(std::move(y1));\n";
    f1(std::move(y1));

    std::cout << "\n----- END PROGRAM\n";
    return 0;
}



#include <iostream>

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

Y f2(Y &y) { Y ay; return ay; }

void f3(Y y1, Y &y2){ }

int main() {
    std::cout << "\n----- Y y1, y2, y3;  \n";
    Y y1, y2, y3;

    std::cout << "\n-----  y1=y2; \n";
    y1=y2;

    std::cout << "\n----- f3(y1, y3); \n";
    f3(y1, y3);

    std::cout << "\n-----  Y y4 = f2(y1); \n";
    Y y4 = f2(y1);

    std::cout << "\n----- END PROGRAM\n";
    return 0;
}



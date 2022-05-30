#include "StackClass.h"
#include <iosfwd>
#include <iostream>
#include <exception>
#include <vector>
using namespace std;

template<typename T>
StackClass<T>::StackClass():
    _size(0), _dataContainer(nullptr) {
    cout << "Constructor with parameters" << endl;
}

template<typename T>
void StackClass<T>::push(const T &val) {
    T *oldContainer = this->_dataContainer;
    T *newContainer = new T[this->_size+1];

    int i;
    for(i=0;i<this->_size;i++)
        newContainer[i] = oldContainer[i];
    newContainer[i] = val;

    this->_dataContainer = newContainer;
    this->_size++;
}



template<typename T>
T StackClass<T>::pop() {
    if(this->empty())
        throw StackEmptyException();

    T val = _dataContainer[_size-1];
    T *oldContainer = _dataContainer;
    _size--;
    T *newContainer = new T[_size];

    for(int i=0;i<_size;i++)
        newContainer[i] = oldContainer[i];

    _dataContainer = newContainer;

    return val;
}

template<typename T>
bool StackClass<T>::empty() const {
    return _size==0;
}

template<typename T>
T* StackClass<T>::getDataContainer() {
    return _dataContainer;
}

template<typename T>
std::vector<T> StackClass<T>::getStackAsVector() {
    std::vector<T> dataVector;

    for(int i=0; i<_size ; i++)
        dataVector.push_back(_dataContainer[i]);

    return dataVector;
}

template<typename T>
int StackClass<T>::getSize() const {
    return _size;
}

template<typename T>
StackClass<T>::~StackClass() {
    // Destructor
    cout << "destructor" << endl;
    delete[] this->_dataContainer;
    this->_dataContainer = nullptr;
    this->_size = 0;
}

//copy constructor
template<typename T>
StackClass<T>::StackClass(const StackClass<T> &copyStack):
    _size(copyStack._size), _dataContainer(new T[copyStack._size]) {

    // std::memcpy(this->_dataContainer, copyStack._dataContainer, _size * sizeof(T));
    
    for(int i=0; i<this->_size; i++)
        this->_dataContainer[i] = copyStack._dataContainer[i];
    
    cout << "copy constructor" << endl;
}

//copy operator
template<typename T>
StackClass<T>& StackClass<T>::operator=(const StackClass<T> &copyStack) {

    cout << "copy assignment" << endl;
    if(this == &copyStack)
        return *this;

    delete[] this->_dataContainer;
    this->_size = copyStack._size;
    this->_dataContainer = new T[this->_size];

    // std::memcpy(this->_dataContainer, copyStack._dataContainer, _size * sizeof(T));

    for(int i=0; i<this->_size; i++)
        this->_dataContainer[i] = copyStack._dataContainer[i];
    
    return *this;
}

//move constructor
template<typename T>
StackClass<T>::StackClass(StackClass<T> &&other) noexcept:
    _size(other._size), _dataContainer(other._dataContainer){

    this->_size = 0;
    this->_dataContainer = nullptr;
    cout << "move contructor" << endl;
}

//move operator
template<typename T>
StackClass<T>& StackClass<T>::operator=(StackClass<T> &&other) noexcept {
    cout << "move assignment" << endl;
    if(this == &other)
        return *this;

    delete[] this->_dataContainer;
    this->_size = other._size;
    this->_dataContainer = other._dataContainer;
    other._size = 0;
    other._dataContainer = nullptr;

    return *this;
}

template<typename T>
void StackClass<T>::reverse() {
    for(int i=0; i<_size/2 ; i++){
        // swap with temp file
        T tmp = std::move(_dataContainer[i]);
        _dataContainer[i] = std::move(_dataContainer[_size-i-1]);
        _dataContainer[_size-i-1] = std::move(tmp);
    }
}

template<typename T>
StackClass<T> StackClass<T>::operator+(const StackClass<T> &toAdd) {
    StackClass<T> output;
    output._size = _size + toAdd._size;
    output._dataContainer = new T[output._size];

    for(int i=0; i<output._size; i++)
        if(i<_size) output._dataContainer[i] = _dataContainer[i];
        else output._dataContainer[i] = toAdd._dataContainer[i-_size];

    return output; // move assignment + destructor
}

template<typename T>
std::ostream& operator<<(std::ostream &os, const StackClass<T> &stack) {
    for(int i=0; i < stack._size; i++)
        os << stack._dataContainer[i] << std::endl;
    return os;
}

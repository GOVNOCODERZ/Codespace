#include "Array.h"
#include <iostream>
#include <vector>
using namespace std;

void Array::input() {
    for (size_t i = 0; i < size; ++i) {
        cout << "Insert element #" << i + 1 << ": ";
        cin >> data[i];
    }
}

void Array::output() const {
    cout << "[";
    for (size_t i = 0; i < size; ++i) {
        cout << data[i] << (i < size - 1 ? ", " : "");
    }
    cout << "]" << endl;
}

Array& Array::operator=(const Array& other) {
    if (this != &other) {
        delete[] data;
        size = other.size;
        data = other.data;
    }
    return *this;
}

Array& Array::operator-=(const Array& other) {
    int lesserMassSize = size;
    if(size > other.size){
        lesserMassSize = other.size;
    }
    for (size_t i = 0; i < lesserMassSize; ++i) {
        data[i] -= other.data[i];
    }
    return *this;
}

istream& operator>>(istream& is, Array& arr) {
    for (size_t i = 0; i < arr.size; ++i) {
        is >> arr.data[i];
    }
    return is;
}

ostream& operator<<(ostream& os, const Array& arr) {
    os << "[";
    for (size_t i = 0; i < arr.size; ++i) {
        os << arr.data[i] << (i < arr.size - 1 ? ", " : "");
    }
    os << "]";
    return os;
}
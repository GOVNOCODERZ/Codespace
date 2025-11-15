#include "Array.h"
#include <iostream>
#include <vector>
using namespace std;

Array::Array(): 
    size(0),
    data(nullptr){}

Array::Array(size_t sz): 
    size(sz) {
        if (size == 0) {
            data = nullptr;
        } 
        else {
            data = new int[size];
            for (size_t i = 0; i < size; ++i) {data[i] = 0;}
        }
    }

Array::Array(const Array& other) {
    copyFrom(other);
}

Array::~Array() {
    delete[] data;
}

void Array::input() {
    cout << "Inserting data in massive (size of " << size << "):" << endl;
    for (size_t i = 0; i < size; ++i) {
        cout << "Insert element #" << i + 1 << ": ";
        cin >> data[i];
    }
}

void Array::output() const {
    cout << "Outputing data from massive (size of " << size << "):";
    cout << "[";
    for (size_t i = 0; i < size; ++i) {
        cout << data[i] << (i < size - 1 ? ", " : "");
    }
    cout << "]" << endl;
}

void Array::copyFrom(const Array& other) {
    size = other.size;
    if (size == 0) {
        data = nullptr;
    }
    else {
        data = new int[size];
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }
}

Array& Array::operator=(const Array& other) {
    if (this != &other) {
        delete[] data;
        copyFrom(other);
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
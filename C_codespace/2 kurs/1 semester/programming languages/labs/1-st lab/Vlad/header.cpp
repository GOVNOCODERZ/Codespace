#include "header.h"
#include <iostream>
#include <vector>
using namespace std;

Movie::Movie(): 
    size(0),
    data(nullptr){}

Movie::Movie(string myName, string myDirector, int myYear, string myGenre, int myrating, int myTime): 
    size(sz) {
        if (size == 0) {
            data = nullptr;
        } 
        else {
            data = new int[size];
            for (size_t i = 0; i < size; ++i) {data[i] = 0;}
        }
    }

Movie::Movie(const Movie& other) {
    copyFrom(other);
}

Movie::~Movie() {
    delete[] data;
}

void Movie::input() {
    cout << "Inserting data in massive (size of " << size << "):" << endl;
    for (size_t i = 0; i < size; ++i) {
        cout << "Insert element #" << i + 1 << ": ";
        cin >> data[i];
    }
}

void Movie::output() const {
    cout << "Outputing data from massive (size of " << size << "):";
    cout << "[";
    for (size_t i = 0; i < size; ++i) {
        cout << data[i] << (i < size - 1 ? ", " : "");
    }
    cout << "]" << endl;
}

void Movie::copyFrom(const Movie& other) {
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

istream& operator>>(istream& is, Movie& arr) {
    for (size_t i = 0; i < arr.size; ++i) {
        is >> arr.data[i];
    }
    return is;
}

ostream& operator<<(ostream& os, const Movie& arr) {
    os << "[";
    for (size_t i = 0; i < arr.size; ++i) {
        os << arr.data[i] << (i < arr.size - 1 ? ", " : "");
    }
    os << "]";
    return os;
}
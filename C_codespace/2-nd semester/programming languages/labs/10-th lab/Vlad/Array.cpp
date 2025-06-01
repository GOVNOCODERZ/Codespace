/*
СДЕЛАТЬ ТАК ЧТОБЫ ПУСТЫЕ ДЕФОЛТНЫЕ ОБЪЕКТЫ БЫЛИ ФУНКЦИОНАЛЬНЫ + ПРОТЕСТИРОВАТЬ РАБОТУ ПУСТОГО ОБЪЕКТА В РАЗНЫХ ФУНКЦИЯХ
*/



#include "Array.h"
#include <iostream>
#include <vector>

Array::Array(size_t sz) : size(sz) {
    if (size == 0) {
        data = nullptr;
    } else {
        data = new int[size];
        for (size_t i = 0; i < size; ++i) {
            data[i] = 0;
        }
    }
}

Array::Array() : size(0), data(nullptr) {}

void Array::copyFrom(const Array& other) {
    size = other.size;
    if (size == 0) {
        data = nullptr;
    } else {
        data = new int[size];
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }
}

Array::Array(const Array& other) {
    copyFrom(other);
}

Array::~Array() {
    delete[] data;
}

void Array::input() {
    if (size == 0) {
        std::cout << "Massive is empty. Operation aborted.\n";
        return;
    }
    for (size_t i = 0; i < size; ++i) {
        std::cout << "Insert element #" << i + 1 << ": ";
        std::cin >> data[i];
    }
}

void Array::output() const {
    if (size == 0) {
        std::cout << "Massive is empty. Operation aborted.\n";
        return;
    }
    std::cout << "[";
    for (size_t i = 0; i < size; ++i) {
        std::cout << data[i] << (i < size - 1 ? ", " : "");
    }
    std::cout << "]" << std::endl;
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

std::istream& operator>>(std::istream& is, Array& arr) {
    if (arr.size == 0) {
        std::cout << "Massive is empty. Operation aborted.\n";
        return is;
    }
    for (size_t i = 0; i < arr.size; ++i) {
        is >> arr.data[i];
    }
    return is;
}

std::ostream& operator<<(std::ostream& os, const Array& arr) {
    if (arr.size == 0) {
        os << "Massive is empty. Operation aborted.";
        return os;
    }
    os << "[";
    for (size_t i = 0; i < arr.size; ++i) {
        os << arr.data[i] << (i < arr.size - 1 ? ", " : "");
    }
    os << "]";
    return os;
}
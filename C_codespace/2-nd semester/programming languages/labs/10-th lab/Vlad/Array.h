#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <stdexcept>
using namespace std;

class Array {
    private:
        int* data;    
        size_t size;  

    public:
        Array(): 
            size(0),
            data(nullptr){}

        Array(size_t sz): 
            size(sz) {
                if (size == 0) {
                    data = nullptr;
                } 
                else {
                    data = new int[size];
                    for (size_t i = 0; i < size; ++i) {data[i] = 0;}
                }
            }

        Array(const Array&other):
            size(other.size), 
            data(other.data){}

        ~Array() {
            delete[] data;
        }

        void input();                          
        void output() const;                   

        Array& operator=(const Array& other);  
        Array& operator-=(const Array& other);

        friend std::istream& operator>>(std::istream& is, Array& arr);
        friend std::ostream& operator<<(std::ostream& os, const Array& arr);
};

#endif
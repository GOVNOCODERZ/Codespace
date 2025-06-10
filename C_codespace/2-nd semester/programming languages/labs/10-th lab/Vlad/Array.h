#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
#include <stdexcept>
using namespace std;

class Array {
private:
    int* data;    
    size_t size;  

    void copyFrom(const Array& other);     

public:
    Array();                               
    Array(size_t sz);                      
    Array(const Array& other);             
    ~Array();                              

    void input();                          
    void output() const;                   

    Array& operator=(const Array& other);  
    Array& operator-=(const Array& other);

    friend std::istream& operator>>(std::istream& is, Array& arr);
    friend std::ostream& operator<<(std::ostream& os, const Array& arr);
};

#endif
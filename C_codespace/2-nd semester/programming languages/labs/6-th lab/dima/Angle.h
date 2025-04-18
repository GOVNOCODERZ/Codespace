#ifndef ANGLE_H
#define ANGLE_H

#include <iostream>
#include <string>

class Angle {
private:
    std::string name;
    int degrees;
    int minutes;
    int seconds;

public:
    void input();
    void display() const;
    int toSeconds() const;
};

#endif
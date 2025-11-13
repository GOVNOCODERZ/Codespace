#ifndef ANGLE_H
#define ANGLE_H

#include <iostream>

class Angle {
private:
    int degrees;
    int minutes;
    int seconds;

    void normalize();

public:
    Angle();
    Angle(int deg, int min, int sec);
    Angle(const Angle& other);
    Angle(int totalSeconds);
    ~Angle();

    Angle& operator=(const Angle& other);
    Angle operator+(const Angle& other) const;
    bool operator>=(const Angle& other) const;

    void input();
    void display() const;
};

#endif

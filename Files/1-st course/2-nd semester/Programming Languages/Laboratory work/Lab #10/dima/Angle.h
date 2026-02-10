#ifndef ANGLE_H
#define ANGLE_H

#include <iostream>

class Angle {
private:
    int degrees;
    int minutes;
    int seconds;

    void normalize();

    static int objectCount;

public:
    Angle();
    Angle(int deg, int min, int sec);
    Angle(const Angle& other);
    Angle(int totalSeconds);
    ~Angle();

    Angle& operator=(const Angle& other);
    Angle operator+(const Angle& other) const;
    bool operator>=(const Angle& other) const;

    static int getObjectCount();

    friend std::ostream& operator<<(std::ostream& os, const Angle& angle);
    friend std::istream& operator>>(std::istream& is, Angle& angle);
};

#endif

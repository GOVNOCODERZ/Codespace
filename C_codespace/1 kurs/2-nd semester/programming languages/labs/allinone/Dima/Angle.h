#ifndef ANGLE_H
#define ANGLE_H

#include <iostream>
using namespace std;

class Angle {
private:
    int degrees;
    int minutes;
    int seconds;
    static int objectCount;

    void normalize();

public:
    Angle();
    Angle(int d, int m, int s);
    Angle(int totalSeconds);
    Angle(const Angle& other);
    ~Angle();

    int toSeconds() const;

    Angle& operator=(const Angle& other);
    Angle operator+(const Angle& other) const;
    bool operator>=(const Angle& other) const;

    friend ostream& operator<<(ostream& os, const Angle& a);
    friend istream& operator>>(istream& is, Angle& a);

    static int getObjectCount();
};

#endif

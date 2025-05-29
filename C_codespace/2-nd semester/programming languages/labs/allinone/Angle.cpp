#include "Angle.h"

int Angle::objectCount = 0;

void Angle::normalize() {
    if (seconds >= 60) {
        minutes += seconds / 60;
        seconds %= 60;
    }
    if (minutes >= 60) {
        degrees += minutes / 60;
        minutes %= 60;
    }
}

Angle::Angle(int totalSeconds) {
    degrees = totalSeconds / 3600;
    int remainder = totalSeconds % 3600;
    minutes = remainder / 60;
    seconds = remainder % 60;
    ++objectCount;
}


Angle::Angle() : degrees(0), minutes(0), seconds(0) { ++objectCount; }

Angle::Angle(int d, int m, int s) : degrees(d), minutes(m), seconds(s) {
    normalize();
    ++objectCount;
}

Angle::Angle(const Angle& other) : degrees(other.degrees), minutes(other.minutes), seconds(other.seconds) {
    ++objectCount;
}

Angle::~Angle() {
    --objectCount;
}

int Angle::toSeconds() const {
    return degrees * 3600 + minutes * 60 + seconds;
}

Angle& Angle::operator=(const Angle& other) {
    if (this != &other) {
        degrees = other.degrees;
        minutes = other.minutes;
        seconds = other.seconds;
    }
    return *this;
}

Angle Angle::operator+(const Angle& other) const {
    int totalSeconds1 = degrees * 3600 + minutes * 60 + seconds;
    int totalSeconds2 = other.degrees * 3600 + other.minutes * 60 + other.seconds;
    return Angle(totalSeconds1 + totalSeconds2);
}
bool Angle::operator>=(const Angle& other) const {
    return toSeconds() >= other.toSeconds();
}

ostream& operator<<(ostream& os, const Angle& a) {
    os << a.degrees << " " << a.minutes << " " << a.seconds << "\n";
    return os;
}

istream& operator>>(istream& is, Angle& a) {
    is >> a.degrees >> a.minutes >> a.seconds;
    a.normalize();
    return is;
}

int Angle::getObjectCount() {
    return objectCount;
}

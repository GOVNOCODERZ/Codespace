#include "Angle.h"
#include <iostream>

using namespace std;

Angle::Angle() : degrees(0), minutes(0), seconds(0) {}

Angle::Angle(int totalSeconds) {
    degrees = totalSeconds / 3600;
    minutes = (totalSeconds % 3600) / 60;
    seconds = totalSeconds % 60;
}

Angle::Angle(int deg, int min, int sec) : degrees(deg), minutes(min), seconds(sec) {
    normalize();
}

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

void Angle::display() const {
    cout << degrees << "* " << minutes << "' " << seconds << "\"";
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
    int totalSeconds1 = degrees * 3600 + minutes * 60 + seconds;
    int totalSeconds2 = other.degrees * 3600 + other.minutes * 60 + other.seconds;
    return totalSeconds1 >= totalSeconds2;
}
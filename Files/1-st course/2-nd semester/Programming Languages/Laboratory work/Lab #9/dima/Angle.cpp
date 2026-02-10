#include "Angle.h"
#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;

Angle::Angle() : degrees(0), minutes(0), seconds(0) {}

Angle::~Angle() {}

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

ostream& operator<<(ostream& os, const Angle& angle) {
    os << angle.degrees << " " << angle.minutes << " " << angle.seconds << "\"";
    return os;
}

istream& operator>>(istream& is, Angle& angle) {
    is >> angle.degrees >> angle.minutes >> angle.seconds;
    angle.normalize();
    return is;
}
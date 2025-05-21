#include "Angle.h"
#include <ctime>
#include <cstdlib>

using namespace std;

int Angle::objectCount = 0;

Angle::Angle() {
    srand(time(0));
    degrees = rand() % 360;
    minutes = rand() % 60;
    seconds = rand() % 60;
    ++objectCount;
}

Angle::Angle(int deg, int min, int sec)
    : degrees(deg), minutes(min), seconds(sec) {
    normalize();
    ++objectCount;
}

Angle::Angle(const Angle& other)
    : degrees(other.degrees), minutes(other.minutes), seconds(other.seconds) {
    ++objectCount;
}

Angle::Angle(int totalSeconds) {
    degrees = totalSeconds / 3600;
    minutes = (totalSeconds % 3600) / 60;
    seconds = totalSeconds % 60;
    normalize();
    ++objectCount;
}

Angle::~Angle() {
    cout << "Object Angle has destroyed.\n";
    --objectCount;
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
    if (degrees < 0 || minutes < 0 || seconds < 0) {
        degrees = minutes = seconds = 0;
    }
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
    Angle result(degrees + other.degrees,
                 minutes + other.minutes,
                 seconds + other.seconds);
    result.normalize();
    return result;
}

bool Angle::operator>=(const Angle& other) const {
    if (degrees > other.degrees) return true;
    if (degrees < other.degrees) return false;
    if (minutes > other.minutes) return true;
    if (minutes < other.minutes) return false;
    return seconds >= other.seconds;
}

int Angle::getObjectCount() {
    return objectCount;
}

ostream& operator<<(ostream& os, const Angle& angle) {
    os << angle.degrees << "* " << angle.minutes << "' " << angle.seconds << "\"";
    return os;
}

istream& operator>>(istream& is, Angle& angle) {
    is >> angle.degrees >> angle.minutes >> angle.seconds;
    angle.normalize();
    return is;
}

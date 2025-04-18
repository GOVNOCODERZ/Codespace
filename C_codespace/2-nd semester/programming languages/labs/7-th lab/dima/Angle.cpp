#include "Angle.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

void Angle::In() {
    cout << "\nEnter the name of the angle: ";
    cin >> name;
    cout << "Enter degrees: ";
    cin >> degrees;
    cout << "Enter minutes: ";
    cin >> minutes;
    cout << "Enter seconds: ";
    cin >> seconds;
}

void Angle::Out() const {
    cout << name << ": " << degrees << "* " << minutes << "' " << seconds << "\"" << endl;
}

int Angle::ToSeconds() const {
    return degrees * 3600 + minutes * 60 + seconds;
}
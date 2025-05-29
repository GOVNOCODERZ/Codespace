#include <iostream>
#include "Angle.h"
#include <cstdlib>
using namespace std;

int main() {
    cout << "Angle created by default (random): ";
    Angle a1;
    a1.display();

    cout << "\nAngle specified by parameters (10*, 40', 75\"): ";
    Angle a2(10, 40, 75);
    a2.display();

    cout << "\nCopy of the previous angle: ";
    Angle a3(a2);
    a3.display();

    cout << "\nAngle created by the number of seconds (3723): ";
    Angle a4(3723);
    a4.display();

    Angle a5;
    a5 = a4;
    cout << "\nAngle a5 after assignment from a4: ";
    a5.display();

    return 0;
}
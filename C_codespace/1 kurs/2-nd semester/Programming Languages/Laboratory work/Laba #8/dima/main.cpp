#include <iostream>
#include "Angle.h"

using namespace std;

int main() {
    cout << "Angle A (random): ";
    Angle A;
    A.display();

    cout << "\nAngle B (manually specified): ";
    Angle B(30, 45, 50);
    B.display();

    cout << "\nAngle C (copy of B): ";
    Angle C = B;
    C.display();

    cout << "\nAngle D (created from 3723 seconds): ";
    Angle D(3723);
    D.display();

    cout << "\nAssignment: E = D\n";
    Angle E;
    E = D;
    E.display();

    cout << "\nAddition: F = B + D\n";
    Angle F = B + D;
    F.display();

    cout << "\nComparison: B >= D ?\n";
    if (B >= D)
        cout << "B is greater than or equal to D\n";
    else
        cout << "B is less than D\n";

    return 0;
}
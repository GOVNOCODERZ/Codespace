#include "Angle.h"
#include <iostream>

using namespace std;

int main() {
    Angle A;
    Angle B("CustomAngle", 45, 30, 15);
    Angle C(A);
    Angle* D = new Angle(7295);

    cout << "Angle A: ";
    A.Out();

    cout << "Angle B: ";
    B.Out();

    cout << "Angle C: ";
    C.Out();

    cout << "Angle D: ";
    D->Out();

    cout << "\nTotal seconds for each angle:\n";
    cout << "Angle A: " << A.ToSeconds() << "\n";
    cout << "Angle B: " << B.ToSeconds() << "\n";
    cout << "Angle C: " << C.ToSeconds() << "\n";
    cout << "Angle D: " << D->ToSeconds() << "\n";

    delete D;

    return 0;
}
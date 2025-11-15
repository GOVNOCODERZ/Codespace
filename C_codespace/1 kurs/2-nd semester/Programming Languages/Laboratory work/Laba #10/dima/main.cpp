#include <iostream>
#include "Angle.h"

using namespace std;

int main() {
    cout << "Number of objects before creation: " << Angle::getObjectCount() << "\n";

    {
        Angle a1;
        cout << "Object a1 created: " << a1 << "\n";
        cout << "Number of objects: " << Angle::getObjectCount() << "\n";

        Angle a2(10, 70, 75);
        cout << "Object a2 created: " << a2 << "\n";
        cout << "Number of objects: " << Angle::getObjectCount() << "\n";

        Angle a3 = a2;
        cout << "Object a3 created (copy of a2): " << a3 << "\n";
        cout << "Number of objects: " << Angle::getObjectCount() << "\n";

        Angle a4(3723);
        cout << "Object a4 created (from seconds): " << a4 << "\n";
        cout << "Number of objects: " << Angle::getObjectCount() << "\n";
    }

    cout << "Number of objects after destruction: " << Angle::getObjectCount() << "\n";

    return 0;
}
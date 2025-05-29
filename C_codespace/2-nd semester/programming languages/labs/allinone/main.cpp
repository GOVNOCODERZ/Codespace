#include <iostream>
#include <fstream>
#include "Angle.h"
using namespace std;

int main() {
    ofstream fout("angles.txt");
    if (!fout) {
        cerr << "Failed to open file for writing.\n";
        return 1;
    }

    Angle* da1 = new Angle;
    Angle* da2 = new Angle;

    cout << "Enter angle a1 (degrees minutes seconds): ";
    cin >> *da1;
    cout << "Enter angle a2 (degrees minutes seconds): ";
    cin >> *da2;

    fout << *da1 << endl;
    fout << *da2 << endl;
    fout.close();

    
    ifstream fin("angles.txt");
    if (!fin) {
        cerr << "Failed to open file for reading.\n";
        return 1;
    }

    Angle fileA1, fileA2;
    fin >> fileA1 >> fileA2;
    fin.close();

    cout << "Angle a1 from file: " << fileA1 << endl;
    cout << "Angle a2 from file: " << fileA2 << endl;

    Angle sum = fileA1 + fileA2;
    cout << "Sum of angles: " << sum << endl;
    cout << "a1 >= a2? " << (fileA1 >= fileA2 ? "Yes" : "No") << endl;
    cout << "Current number of Angle objects: " << Angle::getObjectCount() << endl;
    
    Angle a1;
    cout << "Object a1 created: " << a1 << "\n";
    cout << "Number of objects: " << Angle::getObjectCount() << "\n";

    Angle a3 = a1;
    cout << "Object a3 created (copy of a2): " << a3 << "\n";
    cout << "Number of objects: " << Angle::getObjectCount() << "\n";

    Angle a4(3723);
    cout << "Object a4 created (from seconds): " << a4 << "\n";
    cout << "Number of objects: " << Angle::getObjectCount() << "\n";

    delete da1;
    delete da2;

    cout << "After deleting dynamic objects, object count: " << Angle::getObjectCount() << endl;

    return 0;
}

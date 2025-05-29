#include <iostream>
#include <fstream>
#include "Angle.h"

using namespace std;

int main() {
    Angle a;
    cout << "Enter the angle in the format: degrees minutes seconds: ";
    cin >> a;

    cout << "Entered angle: " << a << endl;

    ofstream fout("angle.txt");
    if (fout.is_open()) {
        fout << a;
        fout.close();
        cout << "The angle has been written to the file angle.txt\n";
    }

    Angle b;
    ifstream fin("angle.txt");
    if (fin.is_open()) {
        fin >> b;
        fin.close();
        cout << "The angle read from the file: " << b << endl;
    }

    return 0;
}
#include <iostream>
#include <fstream>
using namespace std;
#include "Age.h"    


int main() 
{
    srand(static_cast<unsigned int>(time(0)));

    cout << "Initial objects count: " << Age::getObjectsCount() << endl;


    Age A;
    cout << "After creating A: " << Age::getObjectsCount() << " active objects." << endl;

    Age B("test_name", 10, 10, 10);
    cout << "After creating B: " << Age::getObjectsCount() << " active objects." << endl;

    Age C(A);
    cout << "After creating C: " << Age::getObjectsCount() << " active objects." << endl;

    Age *D = new Age("test_name_2", 5, 5, 5);
    cout << "After creating D: " << Age::getObjectsCount() << " active objects." << endl;

    Age E(10000);
    cout << "After creating E: " << Age::getObjectsCount() << " active objects." << endl;

    delete []D;
    cout << "After deleting D: " << Age::getObjectsCount() << " active objects." << endl;

    
    cout << "\nFinal active objects: " << Age::getObjectsCount() << " (should be 0)\n";
    
/*
    person.input();
    person.output();

    int totalDays = person.calculateDays();
    cout << "Total object age in days: " << totalDays << " days" << endl;

    cout << "Initial objects:\n";
    cout << "Object A: "; A.output();
    cout << "Object B: "; B.output();
    cout << "Object C: "; C.output();
    cout << "Object D: "; D->output();
    cout << "Object E: "; E.output();

    cout << "\nOperation \"C = B\":\n";
    C = B;
    cout << "Object C: "; C.output();

    cout << "\nOperation \"B*=2\":\n";
    B *= 2;
    cout << "Object B: "; B.output();

    cout << "\nOperation \"A < B\":\n";
    switch (A < B) {
        case 1:
            cout << "A < B: TRUE";
            break;
        case 0:
            cout << "A < B: FALSE";
            break;
    }


    cout << "\n\nUsing >> and << streams:";
    Age F;
    cout << "\nCreating object \"F\"" << endl;
    cin >> F;
    cout << "\nReading object \"F\"" << endl;
    cout << F;

    ofstream outFile("age_data_in.txt");
    if (!outFile) {
        cout << "\nError opening file for writing.";
        return 1;
    }

    outFile << F;
    outFile.close();


    ifstream inFile("age_data_out.txt");
    if (!inFile) {
        cout << "\nError opening file for reading.";
        return 1;
    }

    Age G;
    inFile >> G;
    inFile.close();

    cout << "\nData from file:\n" << G;
*/

    return 0;
}
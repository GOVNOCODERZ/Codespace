#include <iostream>
#include <fstream>
using namespace std;
#include "Age.h"    


int main() 
{
    srand(static_cast<unsigned int>(time(0)));

    Age A;
    Age B("test_name", 10, 10, 10);
    Age C(A);
    Age *D = new Age("test_name_2", 5, 5, 5);
    Age E(10000);
    
/*
    person.input();
    person.output();

    int totalDays = person.calculateDays();
    cout << "Total object age in days: " << totalDays << " days" << endl;
*/
    cout << "Initial objects:\n";
    cout << "Object A: "; A.output();
    cout << "Object B: "; B.output();
    cout << "Object C: "; C.output();
/*
    cout << "Object D: "; D->output();
    cout << "Object E: "; E.output();
*/

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


    // delete []D;
    return 0;
}
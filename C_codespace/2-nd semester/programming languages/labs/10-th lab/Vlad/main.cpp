#include "Array.h"
#include <iostream>
using namespace std;

int main() {
    
        cout << "\nCreating and trying to output an empty massive:" << endl;
        Array* arr0 = new Array();
        arr0->output();

        cout << "Deleting empty massive (via destructor).\n";
        delete arr0;

        Array arr1(4);
        Array arr2(3);

    
        cout << "\nInsert 1st massive (size of 4):\n";
        arr1.input();
        cout << "Insert 2nd massive (size of 3):\n";
        arr2.input();

    
        cout << "\n1st massive before operation: ";
        arr1.output();
        cout << "2nd massive before operation: ";
        arr2.output();

    
        arr1 -= arr2;

    
        cout << "\n1st massive after operation (1st -= 2nd): ";
        arr1.output();

    
        cout << "\n3rd massive (as 1st massive copy): ";
        Array arr3(arr1);
        arr3.output();

        Array arr4(4);
        cout << "\nInsert 4nd massive (via >>):\n";
        cin >> arr4;
        cout << "\n4nd massive output (via <<) after insertion: ";
        cout << arr4;

        Array arr5(4);
        arr5 = arr4;
        cout << "\n5th massive after operation (arr5 = arr4): ";
        arr5.output();

    return 0;
}
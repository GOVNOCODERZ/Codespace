#include "header.h"
#include <iostream>
using namespace std;

int main() {
    cout << "\nCreating a default empty massive:";
    Array* arr0 = new Array();
    cout << "\nTrying to input data in the created massive:" << endl;
    arr0->input();
    cout << "\nTrying to output data from the created massive:" << endl;
    arr0->output();

    cout << "Deleting default massive (via destructor).";
    //delete arr0;

    cout << "\n======================================" << endl;

    cout << "Creating a 1st massive (size of 4)." << endl;
    Array arr1(4);
    cout << "Creating a 2nd massive (size of 3).";
    Array arr2(3);

    cout << "\n======================================" << endl;

    arr1.input();
    arr2.input();

    cout << "\n======================================" << endl;

    cout << "\n1st & 2nd massive data BEFORE operation (array1 -= array2):" << endl;
    arr1.output();
    arr2.output();

    arr1 -= arr2;

    cout << "1st & 2nd massive data AFTER operation (array1 -= array2):" << endl;
    arr1.output();
    arr2.output();

    cout << "\n======================================" << endl;

    cout << "\n3rd massive (as 1st massive copy): " << endl;
    Array arr3(arr1);
    arr3.output();

    Array arr4(4);
    cout << "\nInsert 4nd massive (via >>):" << endl;
    cin >> arr4;
    cout << "\n4nd massive output (via <<) after insertion: ";
    cout << arr4;

    cout << "\n======================================" << endl;

    Array arr5(4);
    *arr0 = arr4;
    cout << "\n5th massive after operation (arr5 = arr4): ";
    arr0->output();

return 0;
}
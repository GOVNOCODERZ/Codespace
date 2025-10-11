#include "laba.h"
#include <iostream>
using namespace std;

int main() {
    int mode;
    do {
        cout << "\n====== Laboratory Work #1 ======\n";
        cout << "Goal: Comparative analysis of sorting algorithm efficiency.\n";
        cout << "Variant 5: Selection Sort, Heap Sort.\n\n";

        cout << "============= MENU =============\n";
        cout << "1. Sort a single array\n";
        cout << "2. Statistical data collection mode\n";
        cout << "3. Close the program\n";
        cout << "Choose execution mode: ";

        cin >> mode;

    switch (mode) {
        case 1:
            runSingleSort();
            break;
        case 2:
            runStatisticalAnalysis();
            break;
        case 3:
            cout << "Exiting the program...\n";
            break;
        default:
            cout << "Invalid choice.\n";
    }
    
    } while (mode != 3);

    return 0;
};
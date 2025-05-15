#include <iostream>
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

    cout << "Object A: "; A.output();
    cout << "Object B: "; B.output();
    cout << "Object C: "; C.output();
    cout << "Object D: "; D->output();

    cout << "Object E: "; E.output();

    delete []D;
    
    return 0;
}
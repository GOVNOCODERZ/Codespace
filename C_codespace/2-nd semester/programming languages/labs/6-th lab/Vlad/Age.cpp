#include <iostream>
using namespace std;
#include "Age.h"

void Age::input() 
{
    cout << "Object data input:" << endl;

    cout << "Input object name: ";
    cin >> name;
    cout << "Input years number: ";
    cin >> years;
    cout << "Input month number: ";
    cin >> months;
    cout << "Input day number: ";
    cin >> days;
}

void Age::output() 
{
    cout << "\nObject data output:" << endl;
    cout << "Object name: \"" << name << "\"" << endl;
    cout << "Object age: " << years << " years, " << months << " months, " << days << " days" << endl;
}

int Age::calculateDays() 
{
    int totalDays = (years * 365) + (months * 30) + days;
    return totalDays;
}
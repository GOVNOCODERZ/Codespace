#include <iostream>
#include <string>
using namespace std;
#include "Age.h"

int Age::objectCount = 0;

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
    // cout << "\nObject data output:" << endl;
    cout << "\nObject name: \"" << name << "\"" << endl;
    cout << "Object age: " << years << " years, " << months << " months, " << days << " days" << endl;
}

int Age::calculateDays() 
{
    int totalDays = (years * 365) + (months * 30) + days;
    return totalDays;
}


Age& Age::operator=(const Age& other) {
    if (this != &other) { 
        name = other.name;
        years = other.years;
        months = other.months;
        days = other.days;
    }
    return *this;
}

Age& Age::operator*=(int multiplier) {
    int totalDays = calculateDays();
    totalDays *= multiplier;        
    years = totalDays / 365;
    int remainingDays = totalDays % 365;
    months = remainingDays / 30;
    days = remainingDays % 30;
    return *this;
}

bool Age::operator<(Age& other) {
    int thisTotal = calculateDays();
    int otherTotal = other.calculateDays();
    return thisTotal < otherTotal;
}


std::istream& operator>>(std::istream& is, Age& obj) {
    cout << "Enter name: ";
    is >> obj.name;
    cout << "Enter years: ";
    is >> obj.years;
    cout << "Enter months: ";
    is >> obj.months;
    cout << "Enter days: ";
    is >> obj.days;
    return is;
}

std::ostream& operator<<(std::ostream& os, const Age& obj) {
    os << "Object name: \"" << obj.name << "\""
       << "\nObject age: " << obj.years << " years, "
       << obj.months << " months, " << obj.days << " days" << endl;
    return os;
}
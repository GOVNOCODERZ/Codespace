#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <string>
#include <vector>

// Используем пространство имен std, чтобы не писать префикс std::
using namespace std;

// ОБЪЯВЛЕНИЕ КЛАССА
class Employee {
private:
    string fullName;
    string position;
    string department;
    double salary;
    int startYear;

public:
    // --- Конструкторы ---
    Employee(); // По умолчанию
    Employee(const string& fn, const string& pos, const string& dep, double sal, int year);

    // --- Геттеры ---
    string getFullName() const;
    string getDepartment() const;
    double getSalary() const;
    int getStartYear() const;

    // --- Перегрузка операторов (объявления) ---
    friend ostream& operator<<(ostream& os, const Employee& emp);
    friend istream& operator>>(istream& is, Employee& emp);
    bool operator>(const Employee& other) const;
};


// ОБЪЯВЛЕНИЯ (ПРОТОТИПЫ) ФУНКЦИЙ
void sortEmployees(vector<Employee>& employees);

void printEmployeesByDepartment(const vector<Employee>& employees, const string& department);
void printEmployeesWithSalaryAboveAverage(const vector<Employee>& employees, const string& department);
void printEmployeesWorkingLongerThan(const vector<Employee>& employees, int years);

void showMenu();

#endif // EMPLOYEE_H
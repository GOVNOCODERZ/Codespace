#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <string>
#include <fstream>

class Employee {
private:
    std::string fullName;   // ФИО
    std::string position;   // Должность
    std::string department; // Отдел
    double salary;          // Зарплата
    int startYear;          // Год приема на работу

public:
    // конструктор по умолчанию
    Employee();
    // конструктор с параметрами
    Employee(const std::string& fn, const std::string& pos, const std::string& dept, double sal, int year);
    // конструктор копирования
    Employee(const Employee& other);
    
    // геттеры и сеттеры
    std::string getFullName() const;
    std::string getPosition() const;
    std::string getDepartment() const;
    double getSalary() const;
    int getStartYear() const;
    void setSalary(double newSalary);

    // перегрузка операторов
    // операторы сравнения по зарплате для сортировки
    bool operator>(const Employee& other) const;
    bool operator<(const Employee& other) const;
    bool operator==(const Employee& other) const;

    // Методы для ввода/вывода
    void read(std::istream& is);
    void print(std::ostream& os) const;

    // дружественные функции для перегрузки операторов ввода/вывода
    friend std::istream& operator>>(std::istream& is, Employee& emp);
    friend std::ostream& operator<<(std::ostream& os, const Employee& emp);
};

// прототипы функций

// функция для чтения массива сотрудников из файла
Employee* readFromFile(const std::string& filename, int& size);

// функция для записи массива сотрудников в файл
void writeToFile(const std::string& filename, const Employee* employees, int size);

// а) вывод списка сотрудников указанного отдела
void employeesByDep(const Employee* employees, int size, const std::string& department);

// б) вывод списка сотрудников с зарплатой выше средней по отделу
void employeesSalAboveAvg(const Employee* employees, int size, const std::string& department);

// в) вывод списка сотрудников, работающих в компании больше 5 лет
void employees5years(const Employee* employees, int size, int currentYear);

// сортировка массива по убыванию методом пузырька
void sortBySalary(Employee* employees, int size);

// добавление нового сотрудника
Employee* addEmployeeToArray(Employee* employees, int& size, const Employee& newEmployee);

#endif
// Employee.h - заголовочный файл для класса Employee

// Стражи включения, чтобы избежать двойного подключения файла
#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <string>
#include <vector>

// Объявление класса Сотрудник
class Employee {

// private секция - данные скрыты от прямого доступа
private:
    std::string fullName;      // ФИО
    std::string position;      // Должность
    std::string department;    // Отдел
    double salary;             // Зарплата
    int employmentYear;        // Год приема на работу

// public секция - методы, доступные для использования в программе
public:
    // --- Конструкторы ---
    Employee(); // по умолчанию
    Employee(const std::string& fn, const std::string& pos, const std::string& dept, double sal, int year); // с параметрами
    Employee(const Employee& other); // копирования

    // --- Геттеры (методы для получения значений полей) ---
    std::string getDepartment() const;
    double getSalary() const;

    // --- Основные методы ---
    void print(std::ostream& os) const; // для вывода информации
    void read(std::istream& is);        // для интерактивного ввода
    int getWorkExperience() const;      // для расчета стажа

    // --- Перегрузка операторов ---
    bool operator<(const Employee& other) const; // для сортировки

    // --- Дружественные функции (для перегрузки операторов ввода/вывода) ---
    friend std::ostream& operator<<(std::ostream& os, const Employee& emp);
    friend std::istream& operator>>(std::istream& is, Employee& emp);

    // --- Статические методы (работают с классом, а не с объектом) ---
    static std::vector<Employee> readFromFile(const std::string& filename);
    static void writeToFile(const std::string& filename, const std::vector<Employee>& employees);

}; // конец объявления класса

#endif // EMPLOYEE_H
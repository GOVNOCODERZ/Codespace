// main.cpp - главный файл программы

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <windows.h> // Для настройки кодировки консоли (полезно для Windows)

#include "Employee.h"

// Прототипы функций
void sortEmployees(std::vector<Employee>& employees);
void findByDepartment(const std::vector<Employee>& employees);
void findAboveAverageSalary(const std::vector<Employee>& employees);
void findWithExperience(const std::vector<Employee>& employees);

int main() {
    // Эти строки настраивают консоль Windows на корректную работу с кодировкой UTF-8.
    // Для чисто английского языка они не строго обязательны, но это хорошая практика.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::vector<Employee> employees;
    std::string filename = "employees.txt";
    int choice;

    do {
        std::cout << "\n========= MENU =========\n"
                  << "1. Load from file\n"
                  << "2. Save to file\n"
                  << "3. Show all employees\n"
                  << "4. Add a new employee\n"
                  << "5. Sort by salary (descending)\n"
                  << "6. Find by department\n"
                  << "7. Find with salary > average in department\n"
                  << "8. Find with experience > 5 years\n"
                  << "0. Exit\n"
                  << "========================\n"
                  << "Your choice: ";

        std::cin >> choice;
        if (std::cin.fail()) {
            std::cout << "Error! Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1:
                employees = Employee::readFromFile(filename);
                break;
            case 2:
                Employee::writeToFile(filename, employees);
                break;
            case 3:
                if (employees.empty()) {
                    std::cout << "The list is empty.\n";
                } else {
                    for (const auto& emp : employees) {
                        std::cout << "--------------------\n";
                        emp.print(std::cout);
                    }
                }
                break;
            case 4: {
                Employee newEmp;
                newEmp.read(std::cin);
                employees.push_back(newEmp);
                std::cout << "Employee added successfully.\n";
                break;
            }
            case 5:
                if (!employees.empty()) sortEmployees(employees);
                else std::cout << "The list is empty, nothing to sort.\n";
                break;
            case 6:
                if (!employees.empty()) findByDepartment(employees);
                else std::cout << "The list is empty.\n";
                break;
            case 7:
                 if (!employees.empty()) findAboveAverageSalary(employees);
                 else std::cout << "The list is empty.\n";
                break;
            case 8:
                 if (!employees.empty()) findWithExperience(employees);
                 else std::cout << "The list is empty.\n";
                break;
            case 0:
                std::cout << "Exiting the program...\n";
                break;
            default:
                std::cout << "Invalid menu option. Please try again.\n";
                break;
        }
    } while (choice != 0);

    return 0;
}

// --- Реализация вспомогательных функций ---
void sortEmployees(std::vector<Employee>& employees) {
    std::sort(employees.begin(), employees.end());
    std::cout << "Employees sorted by salary (descending).\n";
}

void findByDepartment(const std::vector<Employee>& employees) {
    std::string dept;
    std::cout << "Enter department name: ";
    std::getline(std::cin >> std::ws, dept);

    bool found = false;
    std::cout << "\n--- Employees in department '" << dept << "' ---\n";
    for (const auto& emp : employees) {
        if (emp.getDepartment() == dept) {
            emp.print(std::cout);
            std::cout << "--------------------\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "No employees found in this department.\n";
    }
}

void findAboveAverageSalary(const std::vector<Employee>& employees) {
    std::string dept;
    std::cout << "Enter department name: ";
    std::getline(std::cin >> std::ws, dept);

    double totalSalary = 0;
    int count = 0;
    for (const auto& emp : employees) {
        if (emp.getDepartment() == dept) {
            totalSalary += emp.getSalary();
            count++;
        }
    }

    if (count == 0) {
        std::cout << "Department not found or it has no employees.\n";
        return;
    }

    double average = totalSalary / count;
    std::cout << "\n--- Employees with salary > average (" << std::fixed << average << ") in department '" << dept << "' ---\n";
    bool found = false;
    for (const auto& emp : employees) {
        if (emp.getDepartment() == dept && emp.getSalary() > average) {
            emp.print(std::cout);
            std::cout << "--------------------\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "No such employees found.\n";
    }
}

void findWithExperience(const std::vector<Employee>& employees) {
    const int years = 5;
    bool found = false;
    std::cout << "\n--- Employees with more than " << years << " years of experience ---\n";
    for (const auto& emp : employees) {
        if (emp.getWorkExperience() > years) {
            emp.print(std::cout);
            std::cout << "Experience: " << emp.getWorkExperience() << " years\n";
            std::cout << "--------------------\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "No employees with such experience found.\n";
    }
}
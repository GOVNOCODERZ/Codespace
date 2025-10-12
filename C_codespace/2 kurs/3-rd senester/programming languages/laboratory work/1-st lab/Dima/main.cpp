#include <iostream>
#include <string>
#include <limits> // для std::numeric_limits
#include "employee.h"

void menu() {
    std::cout << "\n===== MENU =====\n";
    std::cout << "1. Load data from file\n";
    std::cout << "2. Display data on screen\n";
    std::cout << "3. Write data to file\n";
    std::cout << "4. Show employees by department\n";
    std::cout << "5. Show employees with salary above department average\n";
    std::cout << "6. Show employees working more than 5 years\n";
    std::cout << "7. Sort employees by salary (descending)\n";
    std::cout << "8. Add new employee manually\n";
    std::cout << "0. Exit\n";
    std::cout << "Enter your choice: ";
}

int main() {
    Employee* employees = nullptr; // указатель на динамический массив
    int size = 0; // размер массива
    int choice;

    const std::string inputFile = "output.txt";
    const std::string outputFile = "output.txt";
    const int currentYear = 2025;

    do {
        menu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            choice = -1; // присваиваем неверное значение, чтобы цикл продолжился
        }

        switch (choice) {
            case 1: {
                if (employees != nullptr) {
                    delete[] employees; // освобождаем старую память перед загрузкой новых данных
                }
                employees = readFromFile(inputFile, size);
                if (employees != nullptr) {
                    std::cout << size << " records loaded successfully.\n";
                }
                break;
            }
            case 2: {
                if (size > 0) {
                    std::cout << "\n--- List of All Employees ---\n";
                    for (int i = 0; i < size; ++i) {
                        std::cout << "--- Record " << i + 1 << " ---\n";
                        std::cout << employees[i] << std::endl;
                    }
                } else {
                    std::cout << "No data to display. Please load from file first.\n";
                }
                break;
            }
            case 3: {
                if (size > 0) {
                    writeToFile(outputFile, employees, size);
                } else {
                    std::cout << "No data to write. Please load from file first.\n";
                }
                break;
            }
            case 4: {
                if (size > 0) {
                    std::string dept;
                    std::cout << "Enter department name: ";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // очистить буфер перед getline
                    std::getline(std::cin, dept);
                    employeesByDep(employees, size, dept);
                } else {
                    std::cout << "No data available. Please load from file first.\n";
                }
                break;
            }
            case 5: {
                if (size > 0) {
                    std::string dept;
                    std::cout << "Enter department name: ";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::getline(std::cin, dept);
                    employeesSalAboveAvg(employees, size, dept);
                } else {
                    std::cout << "No data available. Please load from file first.\n";
                }
                break;
            }
            case 6: {
                if (size > 0) {
                    employees5years(employees, size, currentYear);
                } else {
                    std::cout << "No data available. Please load from file first.\n";
                }
                break;
            }
            case 7: {
                if (size > 0) {
                    sortBySalary(employees, size);
                    std::cout << "Data sorted. Use option 2 to see the result.\n";
                } else {
                    std::cout << "No data to sort. Please load from file first.\n";
                }
                break;
            }
            case 8: {
                std::cout << "\n--- Adding a New Employee ---\n";
                Employee newEmployee;
                std::cin >> newEmployee;
                employees = addEmployeeToArray(employees, size, newEmployee);
                std::cout << "Employee added successfully! Current number of employees: " << size << std::endl;
                break;
            }
            case 0:
                std::cout << "Exiting program.\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }

    } while (choice != 0);

    // освобождение памяти перед выходом
    if (employees != nullptr) {
        delete[] employees;
    }

    return 0;
}
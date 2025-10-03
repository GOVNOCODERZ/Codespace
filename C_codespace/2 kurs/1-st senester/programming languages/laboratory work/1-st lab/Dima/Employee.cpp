#include "employee.h"
#include <iostream>
#include <string>
#include <vector>

// конструктор по умолчанию
Employee::Employee() : salary(0.0), startYear(0) {}

// конструктор с параметрами
Employee::Employee(const std::string& fn, const std::string& pos, const std::string& dept, double sal, int year)
    : fullName(fn), position(pos), department(dept), salary(sal), startYear(year) {}

// конструктор копирования
Employee::Employee(const Employee& other)
    : fullName(other.fullName), position(other.position), department(other.department),
      salary(other.salary), startYear(other.startYear) {}

// геттеры и сеттеры
std::string Employee::getFullName() const { return fullName; }
std::string Employee::getPosition() const { return position; }
std::string Employee::getDepartment() const { return department; }
double Employee::getSalary() const { return salary; }
int Employee::getStartYear() const { return startYear; }
void Employee::setSalary(double newSalary) {
    if (newSalary > 0) {
        salary = newSalary;
    }
}

// перегруженные операторы
// оператор > для сортировки по убыванию
bool Employee::operator>(const Employee& other) const { return this->salary > other.salary; }
// оператор <
bool Employee::operator<(const Employee& other) const { return this->salary < other.salary; }
// оператор ==
bool Employee::operator==(const Employee& other) const { return this->salary == other.salary; }

// перегрузка оператора >> для ввода из потока
std::istream& operator>>(std::istream& is, Employee& emp) {
    std::getline(is >> std::ws, emp.fullName);
    std::getline(is >> std::ws, emp.position);
    std::getline(is >> std::ws, emp.department);
    is >> emp.salary >> emp.startYear;
    return is;
}

// перегрузка оператора << для вывода в поток
std::ostream& operator<<(std::ostream& os, const Employee& emp) {
    os << "Full Name: " << emp.fullName << std::endl;
    os << "Position: " << emp.position << std::endl;
    os << "Department: " << emp.department << std::endl;
    os << "Salary: " << emp.salary << std::endl;
    os << "Start Year: " << emp.startYear << std::endl;
    return os;
}

Employee* readFromFile(const std::string& filename, int& size) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        size = 0;
        return nullptr;
    }
    file >> size;
    if (size <= 0) {
        size = 0;
        return nullptr;
    }
    // динамически выделяем память под массив
    Employee* employees = new Employee[size];
    for (int i = 0; i < size; ++i) {
        file >> employees[i]; // используем перегруженный оператор >>
    }
    file.close();
    return employees;
}

void writeToFile(const std::string& filename, const Employee* employees, int size) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to file " << filename << std::endl;
        return;
    }
    file << size << std::endl; // записываем количество объектов
    for (int i = 0; i < size; ++i) {
        file << employees[i].getFullName() << std::endl;
        file << employees[i].getPosition() << std::endl;
        file << employees[i].getDepartment() << std::endl;
        file << employees[i].getSalary() << std::endl;
        file << employees[i].getStartYear() << std::endl;
    }
    file.close();
    std::cout << "Data successfully written to " << filename << std::endl;
}


// выборка
void employeesByDep(const Employee* employees, int size, const std::string& department) {
    std::cout << "\n--- Employees in department '" << department << "' ---\n";
    bool found = false;
    for (int i = 0; i < size; ++i) {
        if (employees[i].getDepartment() == department) {
            std::cout << employees[i] << std::endl;
            found = true;
        }
    }
    if (!found) {
        std::cout << "No employees found in this department.\n";
    }
}

void employeesSalAboveAvg(const Employee* employees, int size, const std::string& department) {
    double totalSalary = 0;
    int count = 0;
    for (int i = 0; i < size; ++i) {
        if (employees[i].getDepartment() == department) {
            totalSalary += employees[i].getSalary();
            count++;
        }
    }
    if (count == 0) {
        std::cout << "No employees found in department '" << department << "' to calculate average salary.\n";
        return;
    }
    double averageSalary = totalSalary / count;
    std::cout << "\n--- Employees in '" << department << "' with salary above average (" << averageSalary << ") ---\n";
    bool found = false;
    for (int i = 0; i < size; ++i) {
        if (employees[i].getDepartment() == department && employees[i].getSalary() > averageSalary) {
            std::cout << employees[i] << std::endl;
            found = true;
        }
    }
    if (!found) {
        std::cout << "No employees found with salary above average.\n";
    }
}

void employees5years(const Employee* employees, int size, int currentYear) {
    std::cout << "\n--- Employees working more than 5 years (as of " << currentYear << ") ---\n";
    bool found = false;
    for (int i = 0; i < size; ++i) {
        if ((currentYear - employees[i].getStartYear()) > 5) {
            std::cout << employees[i] << std::endl;
            found = true;
        }
    }
    if (!found) {
        std::cout << "No employees found with more than 5 years of experience.\n";
    }
}

void sortBySalary(Employee* employees, int size) {
    for (int i = 0; i < size - 1; ++i) {
        for (int j = 0; j < size - i - 1; ++j) {
            // используем перегруженный оператор < для сортировки по убыванию
            if (employees[j] < employees[j + 1]) {
                Employee temp = employees[j];
                employees[j] = employees[j + 1];
                employees[j + 1] = temp;
            }
        }
    }
    std::cout << "\nArray has been sorted by salary in descending order.\n";
}

Employee* addEmployeeToArray(Employee* employees, int& size, const Employee& newEmployee) {
    // 1. создаем новый массив размером на 1 больше
    Employee* newArray = new Employee[size + 1];
    // 2. копируем все элементы из старого массива в новый
    for (int i = 0; i < size; ++i) {
        newArray[i] = employees[i];
    }
    // 3. добавляем новый элемент в конец нового массива
    newArray[size] = newEmployee;
    // 4. увеличиваем счетчик размера
    size++;
    // 5. удаляем старый массив, чтобы освободить память (если он не был пустым)
    if (employees != nullptr) {
        delete[] employees;
    }
    // 6. возвращаем указатель на новый, расширенный массив
    return newArray;
}
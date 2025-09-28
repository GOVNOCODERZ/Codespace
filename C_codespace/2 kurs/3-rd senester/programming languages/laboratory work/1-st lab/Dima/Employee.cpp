// Employee.cpp - файл реализации методов класса Employee

#include "Employee.h"
#include <fstream>
#include <limits>
#include <iomanip> // для std::fixed

// Указываем текущий год для расчета стажа
const int CURRENT_YEAR = 2025;

// --- Реализация конструкторов ---
Employee::Employee() {
    fullName = "";
    position = "";
    department = "";
    salary = 0.0;
    employmentYear = 0;
}

Employee::Employee(const std::string& fn, const std::string& pos, const std::string& dept, double sal, int year)
    : fullName(fn), position(pos), department(dept), salary(sal), employmentYear(year) {}

Employee::Employee(const Employee& other)
    : fullName(other.fullName), position(other.position), department(other.department),
      salary(other.salary), employmentYear(other.employmentYear) {}

// --- Реализация геттеров ---
std::string Employee::getDepartment() const { return department; }
double Employee::getSalary() const { return salary; }
int Employee::getWorkExperience() const { return CURRENT_YEAR - employmentYear; }


// --- Реализация основных методов ---
void Employee::print(std::ostream& os) const {
    os << "Full Name: " << fullName << "\n";
    os << "Position: " << position << "\n";
    os << "Department: " << department << "\n";
    // Используем std::fixed для вывода полного числа
    os << "Salary: " << std::fixed << salary << "\n";
    os << "Employment Year: " << employmentYear << "\n";
}

void Employee::read(std::istream& is) {
    std::cout << "Enter Full Name: ";
    std::getline(is >> std::ws, fullName);
    std::cout << "Enter Position: ";
    std::getline(is, position);
    std::cout << "Enter Department: ";
    std::getline(is, department);
    std::cout << "Enter Salary: ";
    is >> salary;
    while (is.fail() || salary < 0) {
        std::cout << "Error! Please enter a positive number: ";
        is.clear();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        is >> salary;
    }
    std::cout << "Enter Employment Year: ";
    is >> employmentYear;
    while (is.fail() || employmentYear < 1900 || employmentYear > CURRENT_YEAR) {
        std::cout << "Error! Please enter a valid year: ";
        is.clear();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        is >> employmentYear;
    }
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// --- Реализация перегрузки операторов ---
bool Employee::operator<(const Employee& other) const {
    // Сортировка по убыванию: "меньше" тот, у кого зарплата "больше"
    return this->salary > other.salary;
}

std::ostream& operator<<(std::ostream& os, const Employee& emp) {
    os << emp.fullName << "\n"
       << emp.position << "\n"
       << emp.department << "\n"
       << std::fixed << emp.salary << "\n"
       << emp.employmentYear;
    return os;
}

std::istream& operator>>(std::istream& is, Employee& emp) {
    std::getline(is >> std::ws, emp.fullName);
    std::getline(is, emp.position);
    std::getline(is, emp.department);
    is >> emp.salary;
    is >> emp.employmentYear;
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return is;
}

// --- Реализация статических методов ---
std::vector<Employee> Employee::readFromFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Employee> employees;

    if (!file.is_open()) {
        std::cerr << "Error! Failed to open file for reading: " << filename << std::endl;
        return employees;
    }
    
    // Проверяем и пропускаем метку BOM, если она есть
    char bom[3] = {0};
    file.read(bom, 3);
    if (bom[0] != (char)0xEF || bom[1] != (char)0xBB || bom[2] != (char)0xBF) {
        file.seekg(0); // если это не BOM, возвращаемся в начало файла
    }

    int count;
    file >> count;
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < count; ++i) {
        Employee temp;
        file >> temp;
        if(file.fail()) {
            std::cerr << "Error reading data from file." << std::endl;
            employees.clear();
            break;
        }
        employees.push_back(temp);
    }

    file.close();
    std::cout << "Data loaded successfully. Records loaded: " << employees.size() << std::endl;
    return employees;
}

void Employee::writeToFile(const std::string& filename, const std::vector<Employee>& employees) {
    // Открываем файл в бинарном режиме для корректной записи BOM
    std::ofstream file(filename, std::ios::binary); 

    if (!file.is_open()) {
        std::cerr << "Error! Failed to open file for writing: " << filename << std::endl;
        return;
    }

    // Записываем метку BOM для UTF-8 (три байта: 0xEF, 0xBB, 0xBF)
    // Это нужно для правильного отображения не-английских символов в программах вроде Блокнота
    file << (char)0xEF << (char)0xBB << (char)0xBF;

    file << employees.size() << "\n";

    for (const auto& emp : employees) {
        file << emp << "\n";
    }

    file.close();
    std::cout << "Data saved successfully to file '" << filename << "'." << std::endl;
}
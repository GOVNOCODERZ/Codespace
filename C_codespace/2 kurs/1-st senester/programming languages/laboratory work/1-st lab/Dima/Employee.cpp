#include "Employee.h" // Подключаем наш заголовочный файл

#include <algorithm> // для sort
#include <fstream>   // для ifstream, ofstream
#include <ctime>     // для time, localtime

// --- РЕАЛИЗАЦИЯ МЕТОДОВ КЛАССА Employee ---

Employee::Employee() : salary(0.0), startYear(0) {}

Employee::Employee(const string& fn, const string& pos, const string& dep, double sal, int year)
    : fullName(fn), position(pos), department(dep), salary(sal), startYear(year) {}

string Employee::getFullName() const { return fullName; }
string Employee::getDepartment() const { return department; }
double Employee::getSalary() const { return salary; }
int Employee::getStartYear() const { return startYear; }

ostream& operator<<(ostream& os, const Employee& emp) {
    os << "ФИО: " << emp.fullName << "\n"
       << "Должность: " << emp.position << "\n"
       << "Отдел: " << emp.department << "\n"
       << "Зарплата: " << emp.salary << "\n"
       << "Год приема: " << emp.startYear << "\n";
    return os;
}

istream& operator>>(istream& is, Employee& emp) {
    is >> ws;
    getline(is, emp.fullName);
    getline(is, emp.position);
    getline(is, emp.department);
    is >> emp.salary >> emp.startYear;
    return is;
}

bool Employee::operator>(const Employee& other) const {
    return this->salary > other.salary;
}


// --- РЕАЛИЗАЦИЯ ФУНКЦИЙ ---

void sortEmployees(vector<Employee>& employees) {
    sort(employees.begin(), employees.end(), greater<Employee>());
}

void printEmployeesByDepartment(const vector<Employee>& employees, const string& department) {
    cout << "\n--- Сотрудники отдела '" << department << "' ---\n";
    bool found = false;
    for (const auto& emp : employees) {
        if (emp.getDepartment() == department) {
            cout << emp << "--------------------\n";
            found = true;
        }
    }
    if (!found) {
        cout << "Сотрудники в данном отделе не найдены.\n";
    }
}

void printEmployeesWithSalaryAboveAverage(const vector<Employee>& employees, const string& department) {
    double totalSalary = 0.0;
    int count = 0;
    for (const auto& emp : employees) {
        if (emp.getDepartment() == department) {
            totalSalary += emp.getSalary();
            count++;
        }
    }
    if (count == 0) {
        cout << "В отделе '" << department << "' нет сотрудников.\n";
        return;
    }
    double averageSalary = totalSalary / count;
    cout << "\n--- Средняя зарплата по отделу '" << department << "': " << averageSalary << " ---\n";
    cout << "--- Сотрудники с зарплатой выше средней ---\n";
    bool found = false;
    for (const auto& emp : employees) {
        if (emp.getDepartment() == department && emp.getSalary() > averageSalary) {
            cout << emp << "--------------------\n";
            found = true;
        }
    }
    if (!found) {
        cout << "Таких сотрудников не найдено.\n";
    }
}

void printEmployeesWorkingLongerThan(const vector<Employee>& employees, int years) {
    time_t t = time(0);
    tm* now = localtime(&t);
    int currentYear = now->tm_year + 1900;
    cout << "\n--- Сотрудники, работающие более " << years << " лет ---\n";
    bool found = false;
    for (const auto& emp : employees) {
        if ((currentYear - emp.getStartYear()) > years) {
            cout << emp << "--------------------\n";
            found = true;
        }
    }
    if (!found) {
        cout << "Таких сотрудников не найдено.\n";
    }
}

void showMenu() {
    cout << "\n========= МЕНЮ =========\n"
         << "1. Загрузить данные из файла\n"
         << "2. Вывести всех сотрудников на экран\n"
         << "3. Записать данные в файл\n"
         << "4. Показать сотрудников указанного отдела\n"
         << "5. Показать сотрудников с ЗП выше средней по отделу\n"
         << "6. Показать сотрудников, работающих больше 5 лет\n"
         << "7. Сортировать по зарплате (по убыванию)\n"
         << "8. Добавить нового сотрудника\n"
         << "0. Выход\n"
         << "======================\n"
         << "Выберите опцию: ";
}
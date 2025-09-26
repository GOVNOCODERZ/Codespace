#include "Employee.h" // Подключаем наш заголовочный файл, чтобы использовать класс и функции
#include <limits>   // для numeric_limits
#include <fstream>  // для ifstream, ofstream

// --- ГЛАВНАЯ ФУНКЦИЯ ---
int main() {
    setlocale(LC_ALL, "Russian");

    vector<Employee> employees;
    int choice;

    do {
        showMenu();
        cin >> choice;
        // Очистка буфера ввода, чтобы можно было использовать getline после cin
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

        switch (choice) {
            case 1: {
                cout << "Введите имя файла для загрузки: ";
                string filename;
                getline(cin, filename);
                ifstream inFile(filename);
                if (!inFile) {
                    cerr << "Ошибка: не удалось открыть файл " << filename << endl;
                    break;
                }
                employees.clear();
                Employee temp;
                while (inFile >> temp) {
                    employees.push_back(temp);
                }
                cout << "Данные успешно загружены. Загружено " << employees.size() << " записей.\n";
                break;
            }
            case 2: {
                 if (employees.empty()) {
                    cout << "Список сотрудников пуст.\n";
                } else {
                    cout << "\n--- Список всех сотрудников ---\n";
                    for(const auto& emp : employees) {
                        cout << emp << "--------------------\n";
                    }
                }
                break;
            }
            case 3: {
                 if (employees.empty()) {
                    cout << "Нет данных для записи.\n";
                    break;
                }
                cout << "Введите имя файла для сохранения: ";
                string filename;
                getline(cin, filename);
                ofstream outFile(filename);
                 if (!outFile) {
                    cerr << "Ошибка: не удалось создать файл " << filename << endl;
                    break;
                }
                for(size_t i = 0; i < employees.size(); ++i) {
                     outFile << employees[i].getFullName() << "\n"
                            << "Developer\n" // Заглушка, т.к. поле position не имеет сеттера
                            << employees[i].getDepartment() << "\n"
                            << employees[i].getSalary() << "\n"
                            << employees[i].getStartYear()
                            << (i == employees.size() - 1 ? "" : "\n");
                }
                cout << "Данные успешно сохранены в " << filename << endl;
                break;
            }
            case 4: {
                cout << "Введите название отдела: ";
                string dep;
                getline(cin, dep);
                printEmployeesByDepartment(employees, dep);
                break;
            }
            case 5: {
                cout << "Введите название отдела: ";
                string dep;
                getline(cin, dep);
                printEmployeesWithSalaryAboveAverage(employees, dep);
                break;
            }
            case 6: {
                printEmployeesWorkingLongerThan(employees, 5);
                break;
            }
            case 7: {
                sortEmployees(employees);
                cout << "Сотрудники отсортированы по убыванию зарплаты.\n";
                break;
            }
            case 8: {
                 string fn, pos, dep;
                 double sal;
                 int year;
                 cout << "Введите ФИО: ";
                 getline(cin, fn);
                 cout << "Введите должность: ";
                 getline(cin, pos);
                 cout << "Введите отдел: ";
                 getline(cin, dep);
                 cout << "Введите зарплату: ";
                 cin >> sal;
                 cout << "Введите год приема на работу: ";
                 cin >> year;
                 employees.emplace_back(fn, pos, dep, sal, year);
                 cout << "Новый сотрудник добавлен.\n";
                 break;
            }
            case 0: {
                cout << "Выход из программы.\n";
                break;
            }
            default: {
                cout << "Неверная опция, попробуйте снова.\n";
                break;
            }
        }
    } while (choice != 0);

    return 0;

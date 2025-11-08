#include <iostream>
#include <string>
using namespace std;

struct Student {
    string fullName;
    string groupNumber;
    string addressType;
};

void inputStudent(Student& student) {
    cout << "Введите фамилию, имя и отчество студента: ";
    getline(cin >> ws, student.fullName);
    cout << "Введите номер группы (например, БИ13-01): ";
    getline(cin, student.groupNumber);
    cout << "Укажите место жительства (Parents/Dorms/Rent apparts): ";
    getline(cin, student.addressType);
}

void outputStudent(const Student& student) {
    cout << "ФИО: " << student.fullName << endl;
    cout << "Номер группы: " << student.groupNumber << endl;
    cout << "Место жительства: " << student.addressType << endl;
}

void fillMassive(Student* students, int size) {
    for (int i = 0; i < size; ++i) {
        cout << "\nВвод данных для студента №" << i + 1 << ":" << endl;
        inputStudent(students[i]);
    }
}

void printMassive(const Student* students, int size) {
    for (int i = 0; i < size; ++i) {
        cout << "\nДанные студента №" << i + 1 << ":" << endl;
        outputStudent(students[i]);
    }
}

bool filterStudents(const Student* students, int size, const string& group, const string& addressType) {
    bool found = false;

    for (int i = 0; i < size; ++i) {
        if (students[i].groupNumber == group && students[i].addressType == addressType) {
            outputStudent(students[i]);
            found = true;
        }
    }
    
    return found;
}

int main() {
    int studentNum = 0;
    Student* students = nullptr;

    while (true) {
        cout << "\nМеню:\n"
             << "1. Ввод данных студентов\n"
             << "2. Вывод данных студентов\n"
             << "3. Поиск по фильтру\n"
             << "4. Выход из программы\n"
             << "Выберите действие: ";
        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Введите количество студентов: ";
                cin >> studentNum;
                students = new Student[studentNum];
                fillMassive(students, studentNum);
                break;

            case 2:
                if (students == nullptr)
                {
                    cout << "\nСначала необходимо ввести данные студентов!" << endl;
                } 
                else 
                {
                    printMassive(students, studentNum);
                }
                break;

            case 3:
                if (students == nullptr) 
                {
                    cout << "\nСначала необходимо ввести данные студентов!" << endl;
                } 
                else 
                {
                    string targetGroup, targetAddressType;
                    cout << "\nВведите номер группы для выборки: ";
                    getline(cin >> ws, targetGroup);
                    cout << "Укажите место жительства для выборки (например, Dorms): ";
                    getline(cin, targetAddressType);

                    cout << "\nРезультаты выборки:" << endl;
                    if (!filterStudents(students, studentNum, targetGroup, targetAddressType)) {
                        cout << "Студенты, удовлетворяющие условиям, не найдены." << endl;
                    }
                }
                break;

            case 4:
                if (students != nullptr) 
                {
                    delete[] students;
                }
                cout << "\nПрограмма завершена." << endl;
                return 0;

            default:
                cout << "\nКоманда не распознана." << endl;
        }
    }

    return 0;
}
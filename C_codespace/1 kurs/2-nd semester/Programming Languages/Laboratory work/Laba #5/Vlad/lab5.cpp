#include <iostream>
#include <string>
using namespace std;

struct Student {
    string fullName;
    string groupNumber;
    string addressType;
};

struct Node {
    Student student;
    Node* next;
    Node* prev;
};

Node* first(void);
Node* add(Node* pend);
Node* insertBefore(Node* pbegin, const string& key, Student newStudent);
void printForward(Node* pbegin, int size);
void printBackward(Node* pend, int size);

int main(void) {
    Node *pbegin, *pend;

    int studentNum;
    cout << "\nВведите количество студентов: ";
    cin >> studentNum;

    pend = pbegin = first();

    for(int i = 0; i < studentNum - 1; i++) {
        pend = add(pend);
    }

    cout << "\nСписок сверху вниз (от начала к концу):\n";
    printForward(pbegin, studentNum);

    cout << "\nСписок снизу вверх (от конца к началу):\n";
    printBackward(pend, studentNum);

    Student newStudent;
    cout << "\nВведите данные для нового студента:\n";
    cout << "Введите фамилию, имя и отчество студента: ";
    getline(cin >> ws, newStudent.fullName);
    cout << "Введите номер группы (например, БИ13-01): ";
    getline(cin, newStudent.groupNumber);
    cout << "Укажите место жительства (Parents/Dorms/Rent apparts): ";
    getline(cin, newStudent.addressType);

    string key;
    cout << "\nВведите ключ - ФИО студента для вставки нового студента (кроме 1 элемента): ";
    getline(cin >> ws, key);

    pbegin = insertBefore(pbegin, key, newStudent);
    studentNum++;

    cout << "\nОбновленный список сверху вниз (после вставки):\n";
    printForward(pbegin, studentNum);

    cout << "\nОбновлённый список снизу вверх (после вставки):\n";
    printBackward(pend, studentNum);

    return 0;
}

Node* first(void) {
    Node* pv = new Node;
    cout << "Введите фамилию, имя и отчество студента: ";
    getline(cin >> ws, pv->student.fullName);
    cout << "Введите номер группы (например, БИ13-01): ";
    getline(cin, pv->student.groupNumber);
    cout << "Укажите место жительства (Parents/Dorms/Rent apparts): ";
    getline(cin, pv->student.addressType);
    pv->next = nullptr;
    pv->prev = nullptr;
    return pv;
}

Node* add(Node* pend) {
    Node* pv = new Node;
    cout << "Введите фамилию, имя и отчество студента: ";
    getline(cin >> ws, pv->student.fullName);
    cout << "Введите номер группы (например, БИ13-01): ";
    getline(cin, pv->student.groupNumber);
    cout << "Укажите место жительства (Parents/Dorms/Rent apparts): ";
    getline(cin, pv->student.addressType);
    pv->next = nullptr;
    pv->prev = pend;
    pend->next = pv;
    return pv;
}

Node* insertBefore(Node* pbegin, const string& key, Student newStudent) {

    if (pbegin == nullptr) {
        cout << "Список пустой. Невозможно выполнить вставку!" << endl;
        return nullptr;
    }

    Node* current = pbegin;
    while (current != nullptr && current->student.fullName != key) {
        current = current->next;
    }

    if (current == nullptr) {
        cout << "Элемент с ключом \"" << key << "\" не найден." << endl;
        return nullptr;
    }

    Node* newNode = new Node;
    newNode->student = newStudent;
    newNode->next = current;
    newNode->prev = current->prev;

    if (current->prev != nullptr) {
        current->prev->next = newNode;
    } else {
        pbegin = newNode;
    }
    current->prev = newNode;

    return pbegin;
}

void printForward(Node* pbegin, int size) {
    Node* pv = pbegin;
    int count = 1;
    while (pv) {
        cout << "Данные студента №" << count << ": ";
        cout << "ФИО студента: " << pv->student.fullName << "; ";
        cout << "Группа: " << pv->student.groupNumber << "; ";
        cout << "Место проживания: " << pv->student.addressType << "; ";
        cout << endl;
        pv = pv->next;
        count++;
    }
}

void printBackward(Node* pend, int size) {
    Node* pv = pend;
    int count = size;
    while (pv) {
        cout << "Данные студента №" << count << ": ";
        cout << "ФИО студента: " << pv->student.fullName << "; ";
        cout << "Группа: " << pv->student.groupNumber << "; ";
        cout << "Место проживания: " << pv->student.addressType << "; ";
        cout << endl;
        pv = pv->prev;
        count--;
    }
}
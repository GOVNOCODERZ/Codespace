#include <iostream>
#include <cctype>  // isalpha — проверка, буква ли символ
#include <string>
using namespace std;

// Узел списка
struct Node {
    char data;       // символ
    Node* prev;      // указатель на предыдущий узел
    Node* next;      // указатель на следующий узел
    Node(char value) : data(value), prev(nullptr), next(nullptr) {}
};

// Класс двусвязного списка
class List {
private:
    Node* head;      // начало списка
    Node* tail;      // конец списка
    int size;        // количество элементов

public:
    // Конструктор по умолчанию
    List() : head(nullptr), tail(nullptr), size(0) {}

    // Конструктор копирования
    List(const List& other) : head(nullptr), tail(nullptr), size(0) {
        Node* temp = other.head;
        while (temp) {
            push_back(temp->data);
            temp = temp->next;
        }
    }

    // Деструктор
    ~List() { clear(); }

    // Очистка списка
    void clear() { while (head) pop_front(); }

    void push_back(char value) {
        Node* node = new Node(value);
        if (!head) head = tail = node;
        else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
        size++;
    }

    void push_front(char value) {
        Node* node = new Node(value);
        if (!head) head = tail = node;
        else {
            node->next = head;
            head->prev = node;
            head = node;
        }
        size++;
    }

    void insert(int index, char value) {
        if (index <= 0) { push_front(value); return; }
        if (index >= size) { push_back(value); return; }

        Node* temp = head;
        for (int i = 0; i < index - 1; i++)
            temp = temp->next;

        Node* node = new Node(value);
        node->next = temp->next;
        node->prev = temp;
        temp->next->prev = node;
        temp->next = node;
        size++;
    }

    void pop_front() {
        if (!head) return;
        Node* temp = head;
        head = head->next;
        if (head) head->prev = nullptr;
        else tail = nullptr;
        delete temp;
        size--;
    }

    void pop_back() {
        if (!tail) return;
        Node* temp = tail;
        tail = tail->prev;
        if (tail) tail->next = nullptr;
        else head = nullptr;
        delete temp;
        size--;
    }

    void erase(int index) {
        if (index < 0 || index >= size) return;
        if (index == 0) { pop_front(); return; }
        if (index == size - 1) { pop_back(); return; }

        Node* temp = head;
        for (int i = 0; i < index; i++)
            temp = temp->next;

        temp->prev->next = temp->next;
        temp->next->prev = temp->prev;
        delete temp;
        size--;
    }

    int find(char value) {
        Node* temp = head;
        int index = 0;
        while (temp) {
            if (temp->data == value) return index;
            temp = temp->next;
            index++;
        }
        return -1;
    }

    void print() {
        Node* temp = head;
        cout << "\nНовый список: ";
        while (temp) {
            cout << temp->data;
            temp = temp->next;
        }
        cout << endl;
    }

    int getSize() { return size; } // получить размер

    void removeFirstLetters() {
        if (!head) return;
        Node* current = head;
        bool inWord = false;
        int index = 0;

        while (current) {
            if (isalpha(current->data) && !inWord) { // начало слова
                Node* nextNode = current->next;
                erase(index);                        // удалить первую букву
                current = nextNode;
                inWord = true;
                index--;
            }
            else if (!isalpha(current->data)) {      // разделитель слов
                inWord = false;
                current = current->next;
            } else current = current->next;
            index++;
        }
    }

    // Построить список из строки
    void fromString(const string& text) {
        clear();
        for (char c : text)
            push_back(c);
    }
};

int main() {
    List list;
    int choice, index;
    char value;
    string text;

    while (true) {
        cout << "\n МЕНЮ \n";
        cout << "1. Ввести текст и построить список\n";
        cout << "2. Добавить символ в начало\n";
        cout << "3. Добавить символ в конец\n";
        cout << "4. Удалить символ по индексу\n";
        cout << "5. Найти символ\n";
        cout << "6. Удалить первые буквы всех слов (через копию)\n";
        cout << "7. Вывести список\n";
        cout << "0. Выход\n";
        cout << "Выбор: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            cout << "Введите строку текста: ";
            getline(cin, text);
            list.fromString(text);
            cout << "Список построен.\n";
            list.print();
            break;
        case 2:
            cout << "Введите символ: ";
            cin >> value;
            list.push_front(value);
            list.print();
            break;
        case 3:
            cout << "Введите символ: ";
            cin >> value;
            list.push_back(value);
            list.print();
            break;
        case 4:
            cout << "Введите индекс: ";
            cin >> index;
            list.erase(index);
            list.print();
            break;
        case 5:
            cout << "Введите символ: ";
            cin >> value;
            cout << "Индекс: " << list.find(value) << endl;
            break;
        case 6: {
            List copy = list;
            copy.removeFirstLetters();
            cout << "Исходный список: ";
            list.print();
            cout << "После удаления первых букв (копия): ";
            copy.print();
            break;
        }
        case 7:
            list.print();
            break;
        case 0:
            return 0;
        default:
            cout << "Неверный выбор!\n";
        }
    }
}

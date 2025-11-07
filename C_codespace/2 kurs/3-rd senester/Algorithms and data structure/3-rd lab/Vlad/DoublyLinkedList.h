#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <iostream>
#include <initializer_list> // Для конструктора с параметрами

// Узел двусвязного списка
template <typename T>
struct Node {
    T data;           // Данные, хранящиеся в узле
    Node* prev;       // Указатель на предыдущий узел
    Node* next;       // Указатель на следующий узел

    // Конструктор для инициализации узла с данными
    Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
};

// Шаблон класса для Двусвязного Списка
template <typename T>
class DoublyLinkedList {
private:
    Node<T>* head;    // Указатель на первый узел (голову) списка
    Node<T>* tail;    // Указатель на последний узел (хвост) списка
    int size;         // Количество узлов в списке

public:
    // 1. Конструктор по умолчанию: инициализирует пустой список
    DoublyLinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // 2. Конструктор с параметрами (std::initializer_list): создает список с заданными элементами
    DoublyLinkedList(std::initializer_list<T> init_list) : head(nullptr), tail(nullptr), size(0) {
        for (const auto& item : init_list) {
            InsertEnd(item);
        }
    }

    // 3. Конструктор копирования: создает новый список как копию другого списка
    DoublyLinkedList(const DoublyLinkedList& other) : head(nullptr), tail(nullptr), size(0) {
        Node<T>* current = other.head;
        while (current) {
            InsertEnd(current->data);
            current = current->next;
        }
    }

    // Оператор присваивания (не строго требуемый, но полезный)
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) { // Проверка на самоприсваивание
            Clear(); // Очистить текущий список
            Node<T>* current = other.head;
            while (current) {
                InsertEnd(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Деструктор: очищает все узлы в списке
    ~DoublyLinkedList() {
        Clear();
    }

    // Метод для добавления элемента в начало списка
    void InsertBegin(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (!head) { // Если список пуст
            head = tail = newNode;
        } else { // Список не пуст
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    // Метод для добавления элемента в конец списка
    void InsertEnd(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (!tail) { // Если список пуст
            head = tail = newNode;
        } else { // Список не пуст
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    // Метод для добавления элемента в конкретную позицию (индекс) в списке
    void InsertAt(int pos, const T& value) {
        if (pos < 0 || pos > size) {
            std::cout << "Invalid position for insertion." << std::endl;
            return;
        }
        if (pos == 0) {
            InsertBegin(value);
            return;
        }
        if (pos == size) {
            InsertEnd(value);
            return;
        }

        Node<T>* newNode = new Node<T>(value);
        Node<T>* current = head;
        for (int i = 0; i < pos; ++i) {
            current = current->next;
        }

        // Настроить указатели для вставки нового узла
        newNode->next = current;
        newNode->prev = current->prev;
        current->prev->next = newNode;
        current->prev = newNode;
        size++;
    }

    // Метод для удаления элемента из начала списка
    void DeleteBegin() {
        if (!head) { // Если список пуст
            std::cout << "List is empty, cannot delete." << std::endl;
            return;
        }
        Node<T>* oldHead = head;
        if (head == tail) { // Только один элемент в списке
            head = tail = nullptr;
        } else { // Больше одного элемента
            head = head->next;
            head->prev = nullptr;
        }
        delete oldHead; // Освободить память
        size--;
    }

    // Метод для удаления элемента из конца списка
    void DeleteEnd() {
        if (!tail) { // Если список пуст
            std::cout << "List is empty, cannot delete." << std::endl;
            return;
        }
        Node<T>* oldTail = tail;
        if (head == tail) { // Только один элемент в списке
            head = tail = nullptr;
        } else { // Больше одного элемента
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete oldTail; // Освободить память
        size--;
    }

    // Метод для удаления элемента в конкретной позиции (индексе) в списке
    void DeleteAt(int pos) {
        if (pos < 0 || pos >= size) {
            std::cout << "Invalid position for deletion." << std::endl;
            return;
        }
        if (pos == 0) {
            DeleteBegin();
            return;
        }
        if (pos == size - 1) {
            DeleteEnd();
            return;
        }

        Node<T>* current = head;
        for (int i = 0; i < pos; ++i) {
            current = current->next;
        }

        // Настроить указатели для удаления текущего узла
        current->prev->next = current->next;
        current->next->prev = current->prev;
        delete current; // Освободить память
        size--;
    }

    // Метод для поиска значения в списке и возврата его индекса
    int SearchByValue(const T& value) const {
        Node<T>* current = head;
        int index = 0;
        while (current) {
            if (current->data == value) {
                return index; // Вернуть индекс, если найдено
            }
            current = current->next;
            index++;
        }
        return -1; // Вернуть -1 если не найдено
    }

    // Метод для поиска элемента по индексу и возврата его значения
    T SearchByIndex(int pos) const {
        if (pos < 0 || pos >= size) {
            std::cout << "Invalid position for search." << std::endl;
            return T{}; // Вернуть значение по умолчанию для типа T
        }
        Node<T>* current = head;
        for (int i = 0; i < pos; ++i) {
            current = current->next;
        }
        return current->data;
    }

    // Метод для вывода всех элементов списка в консоль
    void Display() const {
        if (!head) { // Если список пуст
            std::cout << "List is empty." << std::endl;
            return;
        }
        Node<T>* current = head;
        while (current) {
            std::cout << current->data << " ";
            current = current->next;
        }
        std::cout << std::endl;
    }

    // Метод для удаления всех элементов из списка
    void Clear() {
        while (head) {
            Node<T>* temp = head;
            head = head->next;
            delete temp; // Освободить память для каждого узла
        }
        tail = nullptr; // Сбросить указатели на голову и хвост
        size = 0;       // Сбросить размер
    }

    // Метод для получения количества элементов в списке
    int Size() const {
        return size;
    }

    // Метод для Лабораторной Работы 3, Вариант 5: Удалить все узлы, содержащие максимальное значение
    void RemoveMaxElements() {
        if (!head) return; // Если список пуст, ничего не делать

        // Шаг 1: Найти максимальное значение в списке
        T maxValue = head->data;
        Node<T>* current = head;
        while (current) {
            if (current->data > maxValue) {
                maxValue = current->data;
            }
            current = current->next;
        }

        // Шаг 2: Удалить все узлы, которые имеют максимальное значение
        current = head;
        while (current) {
            if (current->data == maxValue) {
                Node<T>* nodeToDelete = current;
                current = current->next; // Переместить указатель current перед удалением

                if (nodeToDelete->prev) {
                    nodeToDelete->prev->next = nodeToDelete->next;
                } else {
                    head = nodeToDelete->next; // Обновить голову, если удаляется первый узел
                }

                if (nodeToDelete->next) {
                    nodeToDelete->next->prev = nodeToDelete->prev;
                } else {
                    tail = nodeToDelete->prev; // Обновить хвост, если удаляется последний узел
                }

                delete nodeToDelete; // Освободить память
                size--;
            } else {
                current = current->next; // Перейти к следующему узлу только если мы не удаляли
            }
        }
    }
};

#endif // DOUBLYLINKEDLIST_H
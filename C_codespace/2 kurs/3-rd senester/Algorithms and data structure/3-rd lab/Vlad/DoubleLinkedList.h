#ifndef DOUBLELINKEDLIST_H
#define DOUBLELINKEDLIST_H

#include <iostream>

// Узел двусвязного списка
template <typename T>
struct Node {
    T data;
    Node* prev;
    Node* next;

    Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
};

// Шаблон класса Двусвязный список
template <typename T>
class DoubleLinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int size;

public:
    // Конструктор по умолчанию
    DoubleLinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // Деструктор
    ~DoubleLinkedList() {
        Clear();
    }

    // Конструктор копирования
    DoubleLinkedList(const DoubleLinkedList& other) : head(nullptr), tail(nullptr), size(0) {
        Node<T>* current = other.head;
        while (current) {
            InsertEnd(current->data);
            current = current->next;
        }
    }

    // Оператор присваивания (не строго требуемый, но полезный)
    DoubleLinkedList& operator=(const DoubleLinkedList& other) {
        if (this != &other) {
            Clear();
            Node<T>* current = other.head;
            while (current) {
                InsertEnd(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Методы
    void InsertBegin(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (!head) {
            head = tail = newNode;
        } else {
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    void InsertEnd(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (!tail) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    void InsertAt(int pos, const T& value) {
        if (pos < 0 || pos > size) {
            std::cout << "Неверная позиция для вставки." << std::endl;
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

        newNode->next = current;
        newNode->prev = current->prev;
        current->prev->next = newNode;
        current->prev = newNode;
        size++;
    }

    void DeleteBegin() {
        if (!head) {
            std::cout << "Список пуст, невозможно удалить." << std::endl;
            return;
        }
        Node<T>* oldHead = head;
        if (head == tail) { // Один элемент
            head = tail = nullptr;
        } else {
            head = head->next;
            head->prev = nullptr;
        }
        delete oldHead;
        size--;
    }

    void DeleteEnd() {
        if (!tail) {
            std::cout << "Список пуст, невозможно удалить." << std::endl;
            return;
        }
        Node<T>* oldTail = tail;
        if (head == tail) { // Один элемент
            head = tail = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete oldTail;
        size--;
    }

    void DeleteAt(int pos) {
        if (pos < 0 || pos >= size) {
            std::cout << "Неверная позиция для удаления." << std::endl;
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

        current->prev->next = current->next;
        current->next->prev = current->prev;
        delete current;
        size--;
    }

    int SearchByValue(const T& value) const {
        Node<T>* current = head;
        int index = 0;
        while (current) {
            if (current->data == value) {
                return index;
            }
            current = current->next;
            index++;
        }
        return -1; // Не найдено
    }

    T SearchByIndex(int pos) const {
        if (pos < 0 || pos >= size) {
            std::cout << "Неверная позиция для поиска." << std::endl;
            return T{}; // Возвращаем значение по умолчанию
        }
        Node<T>* current = head;
        for (int i = 0; i < pos; ++i) {
            current = current->next;
        }
        return current->data;
    }

    void Display() const {
        if (!head) {
            std::cout << "Список пуст." << std::endl;
            return;
        }
        Node<T>* current = head;
        while (current) {
            std::cout << current->data << " ";
            current = current->next;
        }
        std::cout << std::endl;
    }

    void Clear() {
        while (head) {
            Node<T>* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        size = 0;
    }

    int Size() const {
        return size;
    }

    // Метод для варианта 5: удаление максимальных элементов
    void RemoveMaxElements() {
        if (!head) return; // Список пуст

        // 1. Найдём максимальное значение
        T maxValue = head->data;
        Node<T>* current = head;
        while (current) {
            if (current->data > maxValue) {
                maxValue = current->data;
            }
            current = current->next;
        }

        // 2. Удалим все узлы с максимальным значением
        current = head;
        while (current) {
            if (current->data == maxValue) {
                Node<T>* nodeToDelete = current;
                current = current->next; // Передвигаем указатель перед удалением

                if (nodeToDelete->prev) {
                    nodeToDelete->prev->next = nodeToDelete->next;
                } else {
                    head = nodeToDelete->next; // Удаляем голову
                }

                if (nodeToDelete->next) {
                    nodeToDelete->next->prev = nodeToDelete->prev;
                } else {
                    tail = nodeToDelete->prev; // Удаляем хвост
                }

                delete nodeToDelete;
                size--;
            } else {
                current = current->next;
            }
        }
    }
};

#endif // DOUBLELINKEDLIST_H
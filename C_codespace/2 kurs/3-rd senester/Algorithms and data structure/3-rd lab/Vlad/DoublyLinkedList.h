#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <iostream>
#include <initializer_list> // Для конструктора с параметрами
using namespace std;

// Стуктура — узел двусвязного списка
template <typename T>
struct Node {
    T data; // Данные, хранящиеся в узле
    Node* prev; // Указатель на предыдущий узел
    Node* next; // Указатель на следующий узел

    // Конструктор для инициализации узла с данными
    Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
};

// Шаблон класса для двусвязного списка
template <typename T>
class DoublyLinkedList {
private:
    Node<T>* head; // Указатель на первый узел (голову) списка
    Node<T>* tail; // Указатель на последний узел (хвост) списка
    int size; // Количество узлов в списке

public:
    // Конструктор по умолчанию — пустой список
    DoublyLinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // Конструктор с параметрами — список с заданными элементами
    DoublyLinkedList(initializer_list<T> init_list) : head(nullptr), tail(nullptr), size(0) {
        for (const auto& item : init_list) {
            InsertEnd(item);
        }
    }

    // Конструктор копирования: создает новый список как копию другого списка
    DoublyLinkedList(const DoublyLinkedList& other) : head(nullptr), tail(nullptr), size(0) {
        Node<T>* current = other.head;
        while (current) { // Поэлементно вносим узлы входящего списка в данные создаваемого
            InsertEnd(current->data);
            current = current->next;
        }
    }

    // Деструктор — удаление всех узлов списка
    ~DoublyLinkedList() {
        Clear();
    }

    /// @brief Добавление элемента в начало списка
    /// @param value значение добавлямого узла
    void InsertBegin(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (!head) { // Нет начального узла => список пуст
            head = tail = newNode;
        } else { // Список не пуст
            newNode->next = head;
            head->prev = newNode;
            head = newNode;
        }
        size++;
    }

    /// @brief Добавление элемента в конец списка
    /// @param value значение добавлямого узла
    void InsertEnd(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (!tail) { // Нет конечного узла => список пуст
            head = tail = newNode;
        } else { // Список не пуст
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    /// @brief Добавление элемента в конкретную позицию
    /// @param pos индекс позиции
    /// @param value значение добавлямого узла
    void InsertAt(int pos, const T& value) {
        if (pos < 0 || pos > size) { // Нужный индекс не найден
            cout << "ERROR: Invalid position for insertion." << endl;
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

        Node<T>* newNode = new Node<T>(value); // Создаём добавляемый узел
        Node<T>* current = head; // Узел списка, который станет следующим для добавляемого
        for (int i = 0; i < pos; ++i) { // Перемещаем указатель до нужного индекса
            current = current->next;
        }

        // Обновляем ссылки соседних узлов ("вставляем" нужный узел "между ними")
        newNode->next = current;
        newNode->prev = current->prev;
        current->prev->next = newNode;
        current->prev = newNode;
        size++;
    }

    /// @brief Удаление начального узла
    void DeleteBegin() {
        if (!head) { // Нет начального узла => список пуст
            cout << "ERROR: List is empty, cannot delete." << endl;
            return;
        }
        Node<T>* oldHead = head;
        if (head == tail) { // Только один элемент в списке
            head = tail = nullptr;
        } else { // Больше одного элемента
            head = head->next;
            head->prev = nullptr;
        }
        delete oldHead; // Удаляем узел из памяти
        size--;
    }

    /// @brief Удаление конечного узла
    void DeleteEnd() {
        if (!tail) { // Нет конечного узла => список пуст
            cout << "ERROR: List is empty, cannot delete." << endl;
            return;
        }
        Node<T>* oldTail = tail;
        if (head == tail) { // Только один элемент в списке
            head = tail = nullptr;
        } else { // Больше одного элемента
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete oldTail; // Удаляем узел из памяти
        size--;
    }

    // Метод для удаления элемента в конкретной позиции (индексе) в списке

    /// @brief Удаление узла с конкретным индексом
    /// @param pos индекс узла
    void DeleteAt(int pos) {
        if (pos < 0 || pos >= size) {
            cout << "ERROR: Invalid position for deletion." << endl;
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

        // Обновляем ссылки для соседних узлов
        current->prev->next = current->next;
        current->next->prev = current->prev;
        delete current; // Удаляем узел из памяти
        size--;
    }
    /// @brief Поиск узла по его значению
    /// @param value искомое значение
    /// @return индекс узла
    int SearchByValue(const T& value) const {
        Node<T>* current = head;
        int index = 0;
        while (current) { // Проходимся по списку, сравниваем значения с искомым
            if (current->data == value) {
                return index; // Узел найден
            }
            current = current->next;
            index++;
        }
        return -1; // Узел не найден
    }

    /// @brief Поиск узла по его индексу
    /// @param value искомый индекс
    /// @return значение узла
    T SearchByIndex(int pos) const {
        Node<T>* current = head;
        for (int i = 0; i < pos; ++i) { // Перемещаем указатель на искомый узел
            current = current->next;
        }
        return current->data; // Узел найден
    }

    // Метод для вывода всех элементов списка в консоль

    /// @brief Вывод всех узлов списка
    void Display() const {
        if (!head) { // Нет начального узла => список пуст
            cout << "ERROR: List is empty." << endl;
            return;
        }
        Node<T>* current = head;
        while (current) { // Поочерёдно выводим каждый элемент
            cout << current->data << " ";
            current = current->next;
        }
        cout << endl;
    }
    /// @brief Удаление всех элементов списка
    void Clear() {
        while (head) { // Проходимся по списку, удаляем каждый узел из памяти
            Node<T>* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr; // Сбрасываем указатель на конец
        size = 0; // Сбрасываем значение размера списка
    }


    /// @brief Геттер для количества элементов списка
    /// @return размер списка
    int GetSize() const {
        return size;
    }

    // Метод для Лабораторной Работы 3, Вариант 5: Удалить все узлы, содержащие максимальное значение

    /// @brief Удаление всех узлов, имеющих максимальное значение в списке
    void RemoveMaxElements() {
        if (!head)
            return; // Нет начального узла => список пуст

        // Поиск максимального значения
        T maxValue = head->data;
        Node<T>* current = head;
        while (current) { // Проходимся по каждому узлу, сравниваем значения
            if (current->data > maxValue) {
                maxValue = current->data;
            }
            current = current->next;
        }

        // Удаляем все узлы с максимальным значением
        current = head;
        while (current) { // Проходимся по всему списку, ищем нужные узлы
            if (current->data == maxValue) { // В случае совпадения обновим ссылки соседних узлов
                Node<T>* nodeToDelete = current;
                current = current->next;

                if (nodeToDelete->prev) {
                    nodeToDelete->prev->next = nodeToDelete->next;
                } else {
                    head = nodeToDelete->next; // Обновляем ссылку на начало, если удаляем первый узел
                }

                if (nodeToDelete->next) {
                    nodeToDelete->next->prev = nodeToDelete->prev;
                } else {
                    tail = nodeToDelete->prev; // Обновляем ссылку на конец, если удаляем последний узел
                }

                delete nodeToDelete; // Удаляем узел из памяти
                size--;
            } else {
                current = current->next; // Совпадения нет, начинаем цикл снова
            }
        }
    }
};

#endif
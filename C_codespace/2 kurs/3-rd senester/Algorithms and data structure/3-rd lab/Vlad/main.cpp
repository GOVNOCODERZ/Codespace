#include "DoublyLinkedList.h"
#include <iostream>
#include <limits>
using namespace std;

int main() {
    DoublyLinkedList<int> list; // Основной используемый список — Конструктор по умолчанию
    DoublyLinkedList<int> listInit = {1, 2, 3, 4, 5}; // Список для демонстрации Конструктора с параметрами
    DoublyLinkedList<int> listToCopy; // Список для демонстрации Конструктора копирования

    int choice, value, pos; // Переменные для выбора опции меню, значения и индекса узла пользователем

    while (true) {
        cout << "\n===== Laboratory Work #3 =====\n\n";

        cout << "============= MENU =============" << endl;
        cout << "1. Enter sequence manually (0 to stop)" << endl;
        cout << "2. Display current list" << endl;
        cout << "3. Display list — Default constructor" << endl;
        cout << "4. Display copied list — Copy constructor" << endl;
        cout << "5. Add element to the beginning" << endl;
        cout << "6. Add element to the end" << endl;
        cout << "7. Add element at index" << endl;
        cout << "8. Delete element from the beginning" << endl;
        cout << "9. Delete element from the end" << endl;
        cout << "10. Delete element at index" << endl;
        cout << "11. Search element by value" << endl;
        cout << "12. Search element by index" << endl;
        cout << "13. Remove maximum elements" << endl;
        cout << "0. Exit" << endl;
        cout << "Choose an action: ";
        cin >> choice;

        switch (choice) {
            case 1: // Задаём список вручную
                list.Clear(); // Очищаем список перед новым вводом
                cout << "Enter integers (enter 0 to stop): ";
                while (true) {
                    cin >> value;
                    if (value == 0) { // Введён "0" как отдельный элемент — ввод прекращается
                        break;
                    }
                    if (cin.fail()) { // Ошибка: введено не число
                        cin.clear(); // Очищаем флаг ошибки
                        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Пропускаем неверный ввод
                        cout << "ERROR: Invalid input. Please enter an integer or 0 to stop." << endl;
                        continue; // Продолжить цикл ввода
                    }
                    list.InsertEnd(value); // Добавляем полученное значение в конец списка
                }
                cout << "List after input: ";
                list.Display();
                break;
            case 2: // Вывод текущих значений списка
                cout << "Current list: ";
                list.Display();
                break;
            case 3: // Вывод списка, созданного с помощью конструктора по умолчанию
                cout << "List created with Default constructor: ";
                listInit.Display();
                break;
            case 4: // Копирование списка из конструктора по умолчанию и вывод списка
                listToCopy = DoublyLinkedList<int>(listInit);
                cout << "Copy of Default constructor list (Copy constructor): " << endl;
                listToCopy.Display();
                break;
            case 5: // Добавление элемента в начало
                cout << "Enter value: ";
                cin >> value;
                list.InsertBegin(value);
                cout << "Element added to the beginning." << endl;
                break;
            case 6: // Добавление элемента в конец
                cout << "Enter value: ";
                cin >> value;
                list.InsertEnd(value);
                cout << "Element added to the end." << endl;
                break;
            case 7: // Добавление элемента по конкретному индексу
                cout << "Enter index: ";
                cin >> pos;
                cout << "Enter value: ";
                cin >> value;
                list.InsertAt(pos, value);
                cout << "Element added at index \"" << pos << "\"." << endl;
                break;
            case 8: // Удаление начального элемента
                list.DeleteBegin();
                cout << "Element deleted from the beginning." << endl;
                break;
            case 9: // Удаление конечного элемента
                list.DeleteEnd();
                cout << "Element deleted from the end." << endl;
                break;
            case 10: // Удаление элемента по конкретному индексу
                cout << "Enter index: ";
                cin >> pos;
                list.DeleteAt(pos);
                cout << "Element deleted at index \"" << pos << "\"." << endl;
                break;
            case 11: // Поиск элемента по значению
                cout << "Enter value: ";
                cin >> value;
                pos = list.SearchByValue(value);
                if (pos != -1) { // -1 => элемент не был найден
                    cout << "Element \"" << value << "\" found at index \"" << pos << "\"." << endl;
                } else {
                    cout << "Element \"" << value << "\" not found." << endl;
                }
                break;
            case 12: // Поиск элемента по индексу
                cout << "Enter index: ";
                cin >> pos;
                if (pos >= 0 && pos < list.GetSize()) {
                    value = list.SearchByIndex(pos);
                    cout << "Element at index \"" << pos << "\" is \"" << value << "\"." << endl;
                } else {
                    cout << "ERROR: Invalid index." << endl;
                }
                break;
            case 13: // Удаление элементов с максимальным значением (5 вариант)
                cout << "List before removing maximums: ";
                list.Display();
                list.RemoveMaxElements();
                cout << "List after removing maximums: ";
                list.Display();
                break;
            case 0: // Выход из программы
                cout << "Exiting the program..." << endl;
                return 0;
            default: // Некорректный выбор опции меню
                cout << "ERROR: Invalid choice." << endl;
        }
    }

    return 0;
}
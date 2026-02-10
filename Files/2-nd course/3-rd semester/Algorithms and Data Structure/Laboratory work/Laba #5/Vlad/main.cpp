#include "HashTable.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    HashTable<string> ht(37);

    int choice;
    string buffer;

    while (true) {
        cout << "\nLABORATORY WORK #5 (Variant 5)\n";
        cout << "Method: Division, Collision Resolution: Open Addressing (Quadratic Probing)\n";
        cout << "1. Load from file\n";
        cout << "2. Save to file\n";
        cout << "3. Search element\n";
        cout << "4. Add element\n";
        cout << "5. Remove element\n";
        cout << "6. Show table\n";
        cout << "7. General Statistics (for report)\n";
        cout << "0. Exit\n";

        cout << "Your choice: ";

        if (!(cin >> choice)) { // Защита от ввода букв вместо цифр
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        
        switch (choice) {
        case 1: // Загрузка из файла
            cout << "Enter filename: ";
            cin >> buffer;
            ht.loadFromFile(buffer);
            break;
        case 2: // Выгрузка в файл
            cout << "Enter filename to save: ";
            cin >> buffer;
            ht.saveToFile(buffer);
            break;
        case 3: // Поиск элемента
            cout << "Enter word to search: ";
            cin >> buffer;
            ht.search(buffer);
            break;
        case 4: // Добавление элемиента
            cout << "Enter word to add: ";
            cin >> buffer;
            if (ht.insert(buffer))
                cout << "Successfully added.\n";
            else
                cout << "Not added (duplicate or table full).\n";
            break;
        case 5: // Удаление элемента
            cout << "Enter word to remove: ";
            cin >> buffer;
            if (ht.remove(buffer))
                cout << "Successfully removed.\n";
            else
                cout << "Element not found.\n";
            break;
        case 6: // Вывод таблицы
            ht.printTable();
            break;
        case 7: // Статистическая сводка
            ht.printAverageStats();
            break;
        case 0: // Выход
            cout << "Exit.\n";
            return 0;
        default:
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}
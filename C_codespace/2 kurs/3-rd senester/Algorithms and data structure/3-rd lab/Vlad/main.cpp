#include "DoublyLinkedList.h" // Подключить заголовочный файл для Двусвязного Списка
#include <iostream>
#include <limits> // Для std::numeric_limits

int main() {
    DoublyLinkedList<int> list; // Создать экземпляр двусвязного списка (конструктор по умолчанию)
    DoublyLinkedList<int> listInit = {1, 2, 3, 4, 5}; // Создать список с начальными значениями (конструктор с параметрами)
    DoublyLinkedList<int> listCopy; // Создать пустой список для копирования

    int choice, value, pos;     // Переменные для ввода пользователя: выбор меню, значение, позиция

    // Основной цикл программы с меню
    while (true) {
        std::cout << "\n--- Menu ---" << std::endl;
        std::cout << "1. Enter sequence manually (0 to stop)" << std::endl; // Только этот пункт ввода
        std::cout << "2. Display default constructed list" << std::endl;
        std::cout << "3. Display list created with initializer list" << std::endl;
        std::cout << "4. Copy list created with initializer list" << std::endl;
        std::cout << "5. Display copied list" << std::endl;
        std::cout << "6. Display list" << std::endl;
        std::cout << "7. Add element to the beginning" << std::endl;
        std::cout << "8. Add element to the end" << std::endl;
        std::cout << "9. Add element at index" << std::endl;
        std::cout << "10. Delete element from the beginning" << std::endl;
        std::cout << "11. Delete element from the end" << std::endl;
        std::cout << "12. Delete element at index" << std::endl;
        std::cout << "13. Search element by value" << std::endl;
        std::cout << "14. Search element by index" << std::endl;
        std::cout << "15. Variant 5: Remove maximum elements" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Choose an action: ";
        std::cin >> choice;

        switch (choice) {
            case 1: // Ввести последовательность вручную
                list.Clear(); // Очистить текущий список перед новым вводом
                std::cout << "Enter integers (enter 0 to stop): ";
                // Бесконечный цикл для ввода чисел
                while (true) {
                    std::cin >> value;
                    // Проверить на '0' для остановки ввода
                    if (value == 0) {
                        // Не добавляем '0' в список, просто выходим из цикла ввода
                        break;
                    }
                    // Если ввод не число, обработать ошибку
                    if (std::cin.fail()) {
                        std::cin.clear(); // Очистить флаг ошибки
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Пропустить неверный ввод
                        std::cout << "Invalid input. Please enter an integer or 0 to stop." << std::endl;
                        continue; // Продолжить цикл ввода
                    }
                    list.InsertEnd(value); // Добавить значение в конец списка
                }
                std::cout << "List after input: ";
                list.Display(); // Показать вновь созданный список
                break;
            case 2: // Вывести список, созданный конструктором по умолчанию
                std::cout << "Default constructed list: ";
                list.Display();
                break;
            case 3: // Вывести список, созданный с помощью initializer_list
                std::cout << "List created with initializer list: ";
                listInit.Display();
                break;
            case 4: // Скопировать один список в другой (используя конструктор копирования)
                listCopy = DoublyLinkedList<int>(listInit); // Создаём временный объект с помощью конструктора копирования и присваиваем
                // Или можно просто: listCopy = listInit; // Используя оператор присваивания, который также вызывает копирование
                std::cout << "List copied from initializer list list." << std::endl;
                break;
            case 5: // Вывести скопированный список
                std::cout << "Copied list: ";
                listCopy.Display();
                break;
            case 6: // Вывести основной список
                std::cout << "Main list: ";
                list.Display();
                break;
            case 7: // Добавить элемент в начало
                std::cout << "Enter value: ";
                std::cin >> value;
                list.InsertBegin(value);
                std::cout << "Element added to the beginning." << std::endl;
                break;
            case 8: // Добавить элемент в конец
                std::cout << "Enter value: ";
                std::cin >> value;
                list.InsertEnd(value);
                std::cout << "Element added to the end." << std::endl;
                break;
            case 9: // Добавить элемент по конкретному индексу
                std::cout << "Enter index: ";
                std::cin >> pos;
                std::cout << "Enter value: ";
                std::cin >> value;
                list.InsertAt(pos, value);
                std::cout << "Element added at index " << pos << "." << std::endl;
                break;
            case 10: // Удалить элемент из начала
                list.DeleteBegin(); // Вызвать метод в классе списка
                std::cout << "Element deleted from the beginning." << std::endl;
                break;
            case 11: // Удалить элемент из конца
                list.DeleteEnd(); // Вызвать метод в классе списка
                std::cout << "Element deleted from the end." << std::endl;
                break;
            case 12: // Удалить элемент по конкретному индексу
                std::cout << "Enter index: ";
                std::cin >> pos;
                list.DeleteAt(pos); // Вызвать метод в классе списка
                std::cout << "Element deleted at index " << pos << "." << std::endl;
                break;
            case 13: // Найти элемент по значению
                std::cout << "Enter value: ";
                std::cin >> value;
                pos = list.SearchByValue(value); // Вызвать метод в классе списка
                if (pos != -1) {
                    std::cout << "Element " << value << " found at position " << pos << "." << std::endl;
                } else {
                    std::cout << "Element " << value << " not found." << std::endl;
                }
                break;
            case 14: // Найти элемент по индексу
                std::cout << "Enter index: ";
                std::cin >> pos;
                if (pos >= 0 && pos < list.Size()) { // Проверить допустимость индекса
                    value = list.SearchByIndex(pos); // Вызвать метод в классе списка
                    std::cout << "Element at index " << pos << " is " << value << "." << std::endl;
                } else {
                    std::cout << "Invalid index." << std::endl;
                }
                break;
            case 15: // Выполнить задачу по Варианту 5
                std::cout << "List before removing maximums: ";
                list.Display(); // Вывести список до изменений
                list.RemoveMaxElements(); // Вызвать специфичный метод для варианта 5
                std::cout << "List after removing maximums: ";
                list.Display(); // Вывести список после изменений
                break;
            case 0: // Выйти из программы
                std::cout << "Exiting program." << std::endl;
                return 0;
            default: // Обработать некорректный выбор в меню
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }

    return 0; // Эта строка теоретически недостижима из-за цикла while(true)
}
#include "DoubleLinkedList.h" // Подключаем заголовочный файл
#include <iostream>
#include <limits> // Для std::numeric_limits

int main() {
    setlocale(LC_ALL, "Russian");
    DoublyLinkedList<int> list;
    int choice, value, pos;
    int inputChoice;

    while (true) {
        std::cout << "\n--- Меню ---" << std::endl;
        std::cout << "1. Ввести последовательность вручную" << std::endl;
        std::cout << "2. Загрузить последовательность из потока (введите числа до EOF - Ctrl+Z или Ctrl+D)" << std::endl;
        std::cout << "3. Вывести список" << std::endl;
        std::cout << "4. Добавить элемент в начало" << std::endl;
        std::cout << "5. Добавить элемент в конец" << std::endl;
        std::cout << "6. Добавить элемент по индексу" << std::endl;
        std::cout << "7. Удалить элемент из начала" << std::endl;
        std::cout << "8. Удалить элемент из конца" << std::endl;
        std::cout << "9. Удалить элемент по индексу" << std::endl;
        std::cout << "10. Найти элемент по значению" << std::endl;
        std::cout << "11. Найти элемент по индексу" << std::endl;
        std::cout << "12. Вариант 5: Удалить максимальные элементы" << std::endl;
        std::cout << "0. Выход" << std::endl;
        std::cout << "Выберите действие: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
            case 2:
                list.Clear(); // Очищаем текущий список перед новым вводом
                std::cout << "Введите целые числа (введите 0 для завершения ввода вручную или EOF для потока): ";
                if (choice == 1) {
                    while (true) {
                        std::cin >> value;
                        if (std::cin.fail() || value == 0) { // Проверка на EOF или 0 для ручного ввода
                             if (value == 0) std::cin.clear(); // Сброс флага ошибки, если ввели 0
                             break;
                        }
                        list.InsertEnd(value);
                    }
                } else { // choice == 2
                    while (std::cin >> value) {
                        list.InsertEnd(value);
                    }
                    std::cin.clear(); // Сброс флага ошибки после EOF
                    // Очищаем буфер ввода после EOF
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                std::cout << "Список после ввода: ";
                list.Display();
                break;
            case 3:
                std::cout << "Список: ";
                list.Display();
                break;
            case 4:
                std::cout << "Введите значение: ";
                std::cin >> value;
                list.InsertBegin(value);
                std::cout << "Элемент добавлен." << std::endl;
                break;
            case 5:
                std::cout << "Введите значение: ";
                std::cin >> value;
                list.InsertEnd(value);
                std::cout << "Элемент добавлен." << std::endl;
                break;
            case 6:
                std::cout << "Введите индекс: ";
                std::cin >> pos;
                std::cout << "Введите значение: ";
                std::cin >> value;
                list.InsertAt(pos, value);
                std::cout << "Элемент добавлен." << std::endl;
                break;
            case 7:
                list.DeleteBegin();
                std::cout << "Элемент удален из начала." << std::endl;
                break;
            case 8:
                list.DeleteEnd();
                std::cout << "Элемент удален из конца." << std::endl;
                break;
            case 9:
                std::cout << "Введите индекс: ";
                std::cin >> pos;
                list.DeleteAt(pos);
                std::cout << "Элемент удален." << std::endl;
                break;
            case 10:
                std::cout << "Введите значение: ";
                std::cin >> value;
                pos = list.SearchByValue(value);
                if (pos != -1) {
                    std::cout << "Элемент " << value << " найден на позиции " << pos << "." << std::endl;
                } else {
                    std::cout << "Элемент " << value << " не найден." << std::endl;
                }
                break;
            case 11:
                std::cout << "Введите индекс: ";
                std::cin >> pos;
                if (pos >= 0 && pos < list.Size()) {
                    value = list.SearchByIndex(pos);
                    std::cout << "Элемент на позиции " << pos << " равен " << value << "." << std::endl;
                } else {
                    std::cout << "Неверный индекс." << std::endl;
                }
                break;
            case 12:
                std::cout << "Список до удаления максимальных: ";
                list.Display();
                list.RemoveMaxElements();
                std::cout << "Список после удаления максимальных: ";
                list.Display();
                break;
            case 0:
                std::cout << "Выход из программы." << std::endl;
                return 0;
            default:
                std::cout << "Неверный выбор. Попробуйте снова." << std::endl;
        }
    }

    return 0;
}
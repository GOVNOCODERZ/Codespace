#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

#include "TFigureArray.h"
#include "Square.h"
#include "Circle.h"

// Вспомогательная функция для очистки потока ввода
void clear_cin() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Шаблонная функция для демонстрации работы с любым типом фигур
template <typename T>
void demonstrate(const std::string& type_name) {
    TFigureArray<T> figures;
    std::string filename;

    while (true) {
        std::cout << "\n--- Меню для массива " << type_name << " ---\n";
        std::cout << "1. Прочитать массив из файла\n";
        std::cout << "2. Вывести массив в консоль\n";
        std::cout << "3. Отсортировать массив по площади (сортировка Шелла)\n";
        std::cout << "4. Найти фигуры с площадью больше средней\n";
        std::cout << "5. Изменить элемент через консоль\n";
        std::cout << "6. Установить случайное значение для элемента\n";
        std::cout << "7. Сгенерировать новый случайный массив\n";
        std::cout << "8. Демонстрация конструктора копирования и оператора присваивания\n";
        std::cout << "9. Записать массив в файл\n";
        std::cout << "0. Вернуться в главное меню\n";
        std::cout << "Введите ваш выбор: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            clear_cin();
            continue;
        }
        clear_cin();

        if (choice == 0) {
            break;
        }

        try {
            switch (choice) {
                case 1: {
                    std::cout << "Введите имя входного файла: ";
                    std::cin >> filename;
                    std::ifstream infile(filename);
                    if (!infile) {
                        std::cerr << "Ошибка: Не удалось открыть файл " << filename << std::endl;
                        break;
                    }
                    infile >> figures;
                    std::cout << "Массив загружен из файла " << filename << std::endl;
                    break;
                }
                case 2: {
                    std::cout << figures;
                    break;
                }
                case 3: {
                    figures.ShellSortByArea();
                    std::cout << "Массив отсортирован.\n" << figures;
                    break;
                }
                case 4: {
                    std::vector<T> result = figures.FindFiguresWithAreaGreaterThanAverage();
                    std::cout << "Фигуры с площадью больше средней:\n";
                    if (result.empty()) {
                        std::cout << "Ничего не найдено.\n";
                    } else {
                        for (const auto& fig : result) {
                            std::cout << "- " << fig << "\n";
                        }
                    }
                    break;
                }
                case 5:
                case 6: {
                    if (figures.GetSize() == 0) {
                        std::cout << "Массив пуст.\n";
                        break;
                    }
                    int index;
                    std::cout << "Введите индекс для изменения (0-" << figures.GetSize() - 1 << "): ";
                    std::cin >> index;
                     if (std::cin.fail() || index < 0 || index >= figures.GetSize()) {
                        std::cout << "Неверный индекс.\n";
                        std::cin.clear();
                        clear_cin();
                        break;
                    }
                    if (choice == 5) figures.ModifyElementFromConsole(index);
                    else figures.SetRandomValue(index);
                    std::cout << "Элемент с индексом " << index << " изменен.\n" << figures;
                    break;
                }
                case 7: {
                    int count;
                    std::cout << "Введите количество случайных элементов для генерации: ";
                    std::cin >> count;
                    figures.GenerateRandomValues(count);
                    std::cout << "Новый случайный массив сгенерирован.\n" << figures;
                    break;
                }
                case 8: {
                    std::cout << "--- Демонстрация конструктора копирования ---\n";
                    TFigureArray<T> figures_copy = figures;
                    std::cout << "Исходный массив:\n" << figures;
                    std::cout << "Скопированный массив:\n" << figures_copy;

                    std::cout << "\n--- Демонстрация оператора присваивания ---\n";
                    TFigureArray<T> figures_assign;
                    figures_assign = figures;
                    std::cout << "Присвоенный массив:\n" << figures_assign;
                    break;
                }
                 case 9: {
                    std::cout << "Введите имя выходного файла: ";
                    std::cin >> filename;
                    std::ofstream outfile(filename);
                    if (!outfile) {
                        std::cerr << "Ошибка: Не удалось открыть файл " << filename << " для записи." << std::endl;
                        break;
                    }
                    // Записываем сначала размер, потом сами элементы
                    outfile << figures.GetSize() << std::endl;
                    for(int i = 0; i < figures.GetSize(); ++i) {
                        outfile << figures[i] << std::endl; // Предполагается, что оператор << для T не добавляет \n
                    }
                    std::cout << "Массив успешно записан в файл " << filename << std::endl;
                    break;
                }
                default:
                    std::cout << "Неверный выбор. Пожалуйста, попробуйте снова.\n";
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Произошла ошибка: " << e.what() << '\n';
        }
    }
}

int main() {
    while (true) {
        std::cout << "\n====== Главное меню ======\n";
        std::cout << "Выберите тип фигуры для работы:\n";
        std::cout << "1. Квадраты\n";
        std::cout << "2. Круги\n";
        std::cout << "0. Выход\n";
        std::cout << "Введите ваш выбор: ";
        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "Некорректный ввод. Пожалуйста, введите число.\n";
            std::cin.clear();
            clear_cin();
            continue;
        }
        clear_cin();

        switch (choice) {
            case 1:
                demonstrate<Square>("Квадрат");
                break;
            case 2:
                demonstrate<Circle>("Круг");
                break;
            case 0:
                std::cout << "Завершение программы.\n";
                return 0;
            default:
                std::cout << "Неверный выбор. Пожалуйста, попробуйте снова.\n";
                break;
        }
    }
    return 0;
}
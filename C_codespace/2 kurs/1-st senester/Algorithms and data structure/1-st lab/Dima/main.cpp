#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include "Array.h"

// Прототипы функций для режимов
void singleSortMode();
void statisticsMode();

int main() {

    int choice;
    do {
        std::cout << "\nМеню программы:\n";
        std::cout << "1. Сортировка одного массива\n";
        std::cout << "2. Режим накопления статистики\n";
        std::cout << "3. Выход\n";
        std::cout << "Ваш выбор: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                singleSortMode();
                break;
            case 2:
                statisticsMode();
                break;
            case 3:
                std::cout << "Завершение работы программы.\n";
                break;
            default:
                std::cout << "Некорректный ввод. Пожалуйста, выберите пункт меню.\n";
        }
    } while (choice != 3);

    return 0;
}

void singleSortMode() {
    int size;
    std::cout << "\n--- Режим 1: Сортировка одного массива ---\n";
    std::cout << "Введите размер массива: ";
    std::cin >> size;

    int min_val, max_val;
    std::cout << "Введите диапазон значений (min max): ";
    std::cin >> min_val >> max_val;

    int method;
    std::cout << "Выберите метод сортировки (1 - Шелла, 2 - Пирамидальная): ";
    std::cin >> method;
    
    Array<int> arr(size);
    arr.fillRandom(min_val, max_val);
    
    Array<int> original_arr = arr;

    std::cout << "\nИсходный массив:\n";
    original_arr.display();
    
    SortStatistics stats;
    std::string methodName;
    
    if (method == 1) {
        methodName = "Сортировка Шелла";
        std::cout << "\nСортировка методом Шелла...\n";
        stats = arr.shellSort();
    } else if (method == 2) {
        methodName = "Пирамидальная сортировка";
        std::cout << "\nПирамидальная сортировка...\n";
        stats = arr.heapSort();
    } else {
        std::cout << "Неверный метод сортировки.\n";
        return;
    }
    
    std::cout << "\nСортировка завершена.\n";
    std::cout << "Отсортированный массив:\n";
    arr.display();
    
    std::cout << "\n--- Статистика ---\n";
    std::cout << "Метод: " << methodName << "\n";
    std::cout << "Сравнений: " << stats.comparisons << "\n";
    std::cout << "Перестановок: " << stats.swaps << "\n";

    std::string filename;
    std::cout << "\nВведите имя файла для сохранения результатов (например, result.txt): ";
    std::cin >> filename;
    
    original_arr.writeToFile(filename, arr);
}

void statisticsMode() {
    int startSize, endSize, step;
    std::cout << "\n--- Режим 2: Сбор статистики ---\n";
    std::cout << "Введите начальный размер массива: ";
    std::cin >> startSize;
    std::cout << "Введите конечный размер массива: ";
    std::cin >> endSize;
    std::cout << "Введите шаг изменения размера: ";
    std::cin >> step;

    int arrayType;
    std::cout << "Выберите способ формирования массива:\n";
    std::cout << "1. Случайные значения\n";
    std::cout << "2. Упорядоченная последовательность\n";
    std::cout << "3. Обратно упорядоченная последовательность\n";
    std::cout << "Ваш выбор: ";
    std::cin >> arrayType;
    
    int method;
    std::cout << "Выберите метод сортировки (1 - Шелла, 2 - Пирамидальная): ";
    std::cin >> method;
    
    std::string filename;
    std::cout << "\nВведите имя файла для сохранения статистики (например, stats.csv): ";
    std::cin >> filename;
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return;
    }
    
    file << "ArraySize,SortTime(ms)\n";
    std::cout << "\nИдет сбор статистики...\n";
    
    for (int currentSize = startSize; currentSize <= endSize; currentSize += step) {
        Array<int> arr(currentSize);
        
        switch (arrayType) {
            case 1: arr.fillRandom(0, currentSize * 10); break;
            case 2: arr.fillSorted(); break;
            case 3: arr.fillReverseSorted(); break;
            default: arr.fillRandom(0, currentSize * 10); break;
        }

        auto startTime = std::chrono::high_resolution_clock::now();
        
        if (method == 1) {
            arr.shellSort();
        } else {
            arr.heapSort();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = endTime - startTime;
        
        file << currentSize << "," << std::fixed << std::setprecision(4) << duration.count() << "\n";
        std::cout << "Размер: " << currentSize << ", Время: " << std::fixed << std::setprecision(4) << duration.count() << " мс\n";
    }
    
    file.close();
    std::cout << "\nСтатистика успешно сохранена в файл " << filename << std::endl;
}
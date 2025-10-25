#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <clocale> // Для setlocale
#include <iomanip> // Для форматирования вывода (std::fixed, std::setprecision)
#include "direct_merge_sort.h"
#include "balanced_multiway_merge_sort.h"

// Перечисление для выбора способа формирования данных
enum DataGenerationMode {
    RANDOM,
    SORTED,
    REVERSE_SORTED
};

// Функция для создания файла с данными
void generateDataFile(const std::string& filename, int size, int min_val, int max_val, DataGenerationMode mode) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Не удалось создать файл для записи данных." << std::endl;
        return;
    }

    std::vector<int> data(size);
    if (mode == RANDOM) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(min_val, max_val);
        for (int i = 0; i < size; ++i) {
            data[i] = distrib(gen);
        }
    } else {
        for (int i = 0; i < size; ++i) {
            data[i] = i + min_val;
        }
        if (mode == REVERSE_SORTED) {
            std::reverse(data.begin(), data.end());
        }
    }

    for (int i = 0; i < size; ++i) {
        file << data[i] << (i == size - 1 ? "" : " ");
    }
    file.close();
}

// Режим одиночной сортировки
void singleSortMode() {
    int size, min_val, max_val, method;
    std::string filename = "data.txt";

    std::cout << "Введите размер числовой последовательности: ";
    std::cin >> size;
    std::cout << "Введите минимальное значение элемента: ";
    std::cin >> min_val;
    std::cout << "Введите максимальное значение элемента: ";
    std::cin >> max_val;

    generateDataFile(filename, size, min_val, max_val, RANDOM);
    std::cout << "Файл '" << filename << "' со случайными данными сгенерирован." << std::endl;

    std::cout << "\nВыберите метод внешней сортировки:\n";
    std::cout << "1. Прямое слияние\n";
    std::cout << "2. Сбалансированное многопутевое слияние\n";
    std::cout << "Ваш выбор: ";
    std::cin >> method;

    SortStats stats;
    if (method == 1) {
        stats = directMergeSort(filename);
    } else if (method == 2) {
        int m;
        std::cout << "Введите количество путей (m) для слияния (например, 3): ";
        std::cin >> m;
        stats = balancedMultiwayMergeSort(filename, m);
    } else {
        std::cout << "Неверный выбор." << std::endl;
        return;
    }

    std::cout << "\nРезультаты сортировки:\n";
    std::cout << "Время выполнения: " << stats.time_ms << " мс\n";
    std::cout << "Количество сравнений: " << stats.comparisons << "\n";
    std::cout << "Количество перестановок (записей): " << stats.swaps << "\n";
    std::cout << "Отсортированные данные находятся в файле '" << filename << "'." << std::endl;
}

// Режим накопления статистики
void statisticsMode() {
    int data_mode_choice, sort_method_choice;
    int start_size, end_size, step;
    std::string output_filename;

    std::cout << "Выберите способ формирования элементов последовательности:\n";
    std::cout << "1. Случайные значения\n";
    std::cout << "2. Упорядоченная последовательность\n";
    std::cout << "3. Значения в обратном порядке\n";
    std::cout << "Ваш выбор: ";
    std::cin >> data_mode_choice;

    DataGenerationMode data_mode;
    switch (data_mode_choice) {
        case 1: data_mode = RANDOM; break;
        case 2: data_mode = SORTED; break;
        case 3: data_mode = REVERSE_SORTED; break;
        default: std::cout << "Неверный выбор.\n"; return;
    }

    std::cout << "Задайте диапазон изменения размера последовательности:\n";
    std::cout << "Начальный размер: ";
    std::cin >> start_size;
    std::cout << "Конечный размер: ";
    std::cin >> end_size;
    std::cout << "Шаг изменения: ";
    std::cin >> step;

    std::cout << "\nВыберите метод внешней сортировки:\n";
    std::cout << "1. Прямое слияние\n";
    std::cout << "2. Сбалансированное многопутевое слияние\n";
    std::cout << "Ваш выбор: ";
    std::cin >> sort_method_choice;
    
    int m = 0;
    if(sort_method_choice == 2) {
        std::cout << "Введите количество путей (m) для слияния (например, 3): ";
        std::cin >> m;
    }

    std::cout << "Введите имя файла для сохранения результатов (например, stats.csv): ";
    std::cin >> output_filename;

    std::ofstream stats_file(output_filename);
    if (!stats_file) {
        std::cerr << "Не удалось создать файл для статистики." << std::endl;
        return;
    }

    stats_file << "Размер Время_(мс)\n";
    stats_file << std::fixed << std::setprecision(0);
    std::cout << "\nНачинается сбор статистики...\n";

    for (int size = start_size; size <= end_size; size += step) {
        std::cout << "Сортировка для размера " << size << "..." << std::endl;
        std::string temp_filename = "temp_data.txt";
        generateDataFile(temp_filename, size, 0, size * 10, data_mode);

        SortStats stats;
        if (sort_method_choice == 1) {
            stats = directMergeSort(temp_filename);
        } else {
            stats = balancedMultiwayMergeSort(temp_filename, m);
        }

        stats_file << size << " " << stats.time_ms << "\n";
    }

    stats_file.close();
    std::cout << "\nСбор статистики завершен. Результаты сохранены в файле '" << output_filename << "'." << std::endl;
}

int main() {
    int choice;
    do {
        std::cout << "\n--- Меню ---\n";
        std::cout << "1. Сортировка файла данных (случайные значения)\n";
        std::cout << "2. Режим накопления статистических данных\n";
        std::cout << "0. Выход\n";
        std::cout << "Ваш выбор: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                singleSortMode();
                break;
            case 2:
                statisticsMode();
                break;
            case 0:
                std::cout << "Выход из программы." << std::endl;
                break;
            default:
                std::cout << "Неверный выбор. Пожалуйста, попробуйте снова." << std::endl;
        }
    } while (choice != 0);

    return 0;
}
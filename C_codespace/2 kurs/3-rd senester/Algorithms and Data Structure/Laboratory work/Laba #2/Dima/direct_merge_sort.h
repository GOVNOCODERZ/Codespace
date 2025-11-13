#ifndef DIRECT_MERGE_SORT_H
#define DIRECT_MERGE_SORT_H

#include <string>

// Структура для хранения статистики сортировки
struct SortStats {
    long long comparisons = 0; // Количество сравнений
    long long swaps = 0;       // Количество "перестановок" (записей в файлы)
    double time_ms = 0.0;      // Время выполнения в миллисекундах
};

// Функция для выполнения прямой сортировки слиянием
SortStats directMergeSort(const std::string& filename);

#endif
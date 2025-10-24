#ifndef BALANCED_MULTIWAY_MERGE_SORT_H
#define BALANCED_MULTIWAY_MERGE_SORT_H

#include <string>
#include "direct_merge_sort.h" // Для использования структуры SortStats

// Функция для сбалансированной многопутевой сортировки слиянием
SortStats balancedMultiwayMergeSort(const std::string& filename, int num_ways);

#endif // BALANCED_MULTIWAY_MERGE_SORT_H
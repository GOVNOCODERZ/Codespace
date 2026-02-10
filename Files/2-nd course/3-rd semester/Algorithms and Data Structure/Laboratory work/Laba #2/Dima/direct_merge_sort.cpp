#include "direct_merge_sort.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdio> // Для remove

// Функция для прямой сортировки слиянием
SortStats directMergeSort(const std::string& filename) {
    auto start_time = std::chrono::high_resolution_clock::now();
    SortStats stats;

    std::ifstream initial_file_counter(filename);
    if (!initial_file_counter) {
        std::cerr << "Не удалось открыть исходный файл." << std::endl;
        return stats;
    }

    // Подсчет количества элементов
    int n = 0;
    int temp;
    while (initial_file_counter >> temp) {
        n++;
    }
    initial_file_counter.close();

    if (n <= 1) {
        // Если элементов 0 или 1, то файл уже отсортирован
        auto end_time = std::chrono::high_resolution_clock::now();
        stats.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        return stats;
    }

    std::string fileA = filename;
    std::string fileB = "temp_b.txt";
    std::string fileC = "temp_c.txt";


    for (int run_size = 1; run_size < n; run_size *= 2) {
        // Шаг 1: Распределение из A в B и C
        std::ifstream source(fileA);
        std::ofstream dest_b(fileB);
        std::ofstream dest_c(fileC);
        
        int current_val;
        int count = 0;
        while (source >> current_val) {
            if ((count / run_size) % 2 == 0) {
                dest_b << current_val << " ";
            } else {
                dest_c << current_val << " ";
            }
            stats.swaps++; // Каждая запись считается "перестановкой"
            count++;
        }
        source.close();
        dest_b.close();
        dest_c.close();

        // Шаг 2: Слияние из B и C в A
        std::ofstream dest_a(fileA);
        std::ifstream source_b(fileB);
        std::ifstream source_c(fileC);

        int val_b, val_c;
        bool has_b = static_cast<bool>(source_b >> val_b);
        bool has_c = static_cast<bool>(source_c >> val_c);
        
        while(has_b || has_c) {
            int b_count = 0;
            int c_count = 0;
            
            // Сливаем одну серию
            while(b_count < run_size && c_count < run_size && has_b && has_c) {
                stats.comparisons++;
                if (val_b <= val_c) {
                    dest_a << val_b << " ";
                    stats.swaps++;
                    b_count++;
                    has_b = static_cast<bool>(source_b >> val_b);
                } else {
                    dest_a << val_c << " ";
                    stats.swaps++;
                    c_count++;
                    has_c = static_cast<bool>(source_c >> val_c);
                }
            }

            // Дописываем остатки серии
            while(b_count < run_size && has_b) {
                dest_a << val_b << " ";
                stats.swaps++;
                b_count++;
                has_b = static_cast<bool>(source_b >> val_b);
            }
            while(c_count < run_size && has_c) {
                dest_a << val_c << " ";
                stats.swaps++;
                c_count++;
                has_c = static_cast<bool>(source_c >> val_c);
            }
        }
        
        dest_a.close();
        source_b.close();
        source_c.close();
    }

    remove(fileB.c_str());
    remove(fileC.c_str());

    auto end_time = std::chrono::high_resolution_clock::now();
    stats.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return stats;
}
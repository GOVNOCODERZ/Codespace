#ifndef DIRECT_MERGE_SORT_H
#define DIRECT_MERGE_SORT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cstdio>
using namespace std;

// Структура для хранения статистики сортировки
struct SortStats {
    long long comparisons = 0; // Количество сравнений
    long long swaps = 0; // Количество перестановок
    double time_ms = 0.0; // Время выполнения в миллисекундах
};

/// @brief Функция прямой сортировки слиянием
/// @param filename имя исходного файла с данными
/// @return Структура со статистикой сортировки
SortStats directMergeSort(const string& filename) {
    // Засекаем время начала сортировки
    auto start_time = chrono::high_resolution_clock::now();
    SortStats stats;

    // Открываем поток для чтения из файла
    ifstream initial_file_counter(filename);
    if (!initial_file_counter) {
        cerr << "ERROR: Failed to open the source file." << endl;
        return stats;
    }

    // Подсчитываем общее количество элементов в файле

    /// Счётчик элементов в файле
    int n = 0;
    int temp;
    while (initial_file_counter >> temp) {
        n++;
    }
    initial_file_counter.close();

    // Если элементов 1 или 0, сортировка не требуется
    if (n <= 1) {
        auto end_time = chrono::high_resolution_clock::now();
        stats.time_ms = chrono::duration<double, milli>(end_time - start_time).count();
        return stats;
    }

    // Используем три файла для сортировки
    string fileA = filename; // Основной файл с данными
    string fileB = "temp_b.txt"; // Временный файл для разделения и слияния
    string fileC = "temp_c.txt"; // Временный файл для разделения и слияния

    // Цикл сортировки — увеличиваем размер серий в 2 раза на каждой итерации
    for (int run_size = 1; run_size < n; run_size *= 2) {
        ifstream source(fileA); // Поток для чтения из основного файла
        ofstream dest_b(fileB); // Поток для записи во временный файл
        ofstream dest_c(fileC); // Поток для записи во временный файл
        
        // Разделяем элементы из основного файла и поочерёдно записываем в два временных файла
        int current_val; // Переменная для записи/чтения значений
        int count = 0; // Счётчик текущей серии цикла
        while (source >> current_val) {
            // Определяем файл для записи по номеру текущей серии
            if ((count / run_size) % 2 == 0) {
                dest_b << current_val << " "; 
            } else {
                dest_c << current_val << " ";
            }
            stats.swaps++; // Увеличиваем счетчик перестановок
            count++;
        }
        source.close();
        dest_b.close();
        dest_c.close();

        // Теперь сливаем отсортированные серии из файлов B и C обратно в файл A
        ofstream dest_a(fileA); // Поток для записи в основной файл
        ifstream source_b(fileB); // Поток для чтения из временного файла
        ifstream source_c(fileC); // Поток для чтения из временного файла

        // Читаем первые элементы из каждого временного файла
        int val_b, val_c;
        bool has_b = static_cast<bool>(source_b >> val_b); // Есть ли элемент в файле B?
        bool has_c = static_cast<bool>(source_c >> val_c); // Есть ли элемент в файле C?
        
        // Цикл слияния серий
        while(has_b || has_c) {
            int b_count = 0; // Счётчик элементов временного файла в текущей серии
            int c_count = 0; // Счётчик элементов временного файла в текущей серии
            
            // Сливаем элементы из текущих серий, пока не закончится одна из них
            // или пока не будет обработано run_size элементов из каждой серии
            while(b_count < run_size && c_count < run_size && has_b && has_c) {
                stats.comparisons++; // Увеличиваем счетчик сравнений
                
                // Сравниваем текущие элементы и записываем меньший
                if (val_b <= val_c) {
                    dest_a << val_b << " ";
                    stats.swaps++; // Увеличиваем счетчик перестановок
                    b_count++;

                    has_b = static_cast<bool>(source_b >> val_b);
                } else {
                    dest_a << val_c << " ";
                    stats.swaps++; // Увеличиваем счетчик перестановок
                    c_count++;

                    has_c = static_cast<bool>(source_c >> val_c);
                }
            }

            // Дописываем оставшиеся элементы из серии B (если серия C закончилась)
            while(b_count < run_size && has_b) {
                dest_a << val_b << " ";
                stats.swaps++; // Увеличиваем счетчик перестановок
                b_count++;
                has_b = static_cast<bool>(source_b >> val_b);
            }
            // Дописываем оставшиеся элементы из серии C (если серия B закончилась)
            while(c_count < run_size && has_c) {
                dest_a << val_c << " ";
                stats.swaps++; // Увеличиваем счетчик перестановок
                c_count++;
                has_c = static_cast<bool>(source_c >> val_c);
            }
        }
        
        // Закрываем все файлы
        dest_a.close();
        source_b.close();
        source_c.close();
    }

    // Удаляем все временные файлы
    remove(fileB.c_str());
    remove(fileC.c_str());

    // Засекаем время окончания и вычисляем общее время сортировки
    auto end_time = chrono::high_resolution_clock::now(); // Время окончания сортировки
    stats.time_ms = chrono::duration<double, milli>(end_time - start_time).count();

    return stats;
}

#endif
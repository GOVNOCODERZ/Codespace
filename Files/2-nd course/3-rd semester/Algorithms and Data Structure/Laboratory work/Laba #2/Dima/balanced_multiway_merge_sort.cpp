#include "balanced_multiway_merge_sort.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdio> // Для remove
#include <queue>      // Для priority_queue
#include <functional> // для std::greater

// Основная функция для сбалансированной многопутевой сортировки
SortStats balancedMultiwayMergeSort(const std::string& filename, int m) {
    if (m < 2) {
        std::cerr << "Количество путей (m) должно быть 2 или больше." << std::endl;
        return {};
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    SortStats stats;

    std::ifstream initial_file_counter(filename);
    if (!initial_file_counter) {
        std::cerr << "Не удалось открыть исходный файл." << std::endl;
        return stats;
    }

    int n = 0;
    int temp;
    while (initial_file_counter >> temp) {
        n++;
    }
    initial_file_counter.close();

    if (n <= 1) {
        auto end_time = std::chrono::high_resolution_clock::now();
        stats.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        return stats;
    }

    // Вспомогательные файлы для двух "лент"
    std::vector<std::string> files1, files2;
    for (int i = 0; i < m; ++i) {
        files1.push_back("temp_1_" + std::to_string(i) + ".txt");
        files2.push_back("temp_2_" + std::to_string(i) + ".txt");
    }

    // --- Шаг 1: Начальное распределение ---
    // Распределяем исходный файл на m файлов (серии длиной 1)
    {
        std::ifstream source(filename);
        std::vector<std::ofstream> dests;
        for (int i = 0; i < m; ++i) {
            dests.emplace_back(files1[i]);
        }
        int file_index = 0;
        while (source >> temp) {
            dests[file_index] << temp << " ";
            stats.swaps++;
            file_index = (file_index + 1) % m;
        }
    }

    long long run_size = 1;
    std::vector<std::string>* from_files = &files1;
    std::vector<std::string>* to_files = &files2;

    // --- Основной цикл слияния ---
    while (run_size < n) {
        std::vector<std::ifstream> sources;
        for (int i = 0; i < m; ++i) {
            sources.emplace_back((*from_files)[i]);
        }

        std::vector<std::ofstream> dests;
        for (int i = 0; i < m; ++i) {
            dests.emplace_back((*to_files)[i]);
        }

        int dest_index = 0;
        while (true) {
            // Используем приоритетную очередь для эффективного нахождения минимума
            // Храним пару {значение, индекс_файла}
            std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;
            
            // Счётчики, сколько элементов мы взяли из каждого файла для ТЕКУЩЕЙ серии
            std::vector<int> current_run_counts(m, 0);

            // "Заполняем" очередь первыми элементами каждой будущей серии
            for (int i = 0; i < m; ++i) {
                if (sources[i] >> temp) {
                    pq.push({temp, i});
                }
            }

            // Если очередь пуста, значит все исходные файлы прочитаны
            if (pq.empty()) {
                break;
            }

            // Сливаем одну m-путевую серию
            while (!pq.empty()) {
                std::pair<int, int> min_pair = pq.top();
                pq.pop();

                int value = min_pair.first;
                int file_idx = min_pair.second;

                dests[dest_index] << value << " ";
                stats.swaps++;

                current_run_counts[file_idx]++;

                // Если еще не исчерпали текущую серию из этого файла,
                // читаем следующий элемент и добавляем в очередь
                if (current_run_counts[file_idx] < run_size) {
                    if (sources[file_idx] >> temp) {
                        stats.comparisons++; // Сравнение произойдет при вставке в очередь
                        pq.push({temp, file_idx});
                    }
                }
            }
            dest_index = (dest_index + 1) % m;
        }
        
        run_size *= m;
        std::swap(from_files, to_files);
    }

    // Копируем результат из первого непустого файла в исходный
    {
        std::ofstream final_dest(filename);
        std::ifstream final_source((*from_files)[0]);
        while (final_source >> temp) {
            final_dest << temp << " ";
        }
    }

    // Удаляем все временные файлы
    for (int i = 0; i < m; ++i) {
        remove(files1[i].c_str());
        remove(files2[i].c_str());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    stats.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return stats;
}
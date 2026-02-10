#ifndef BALANCED_MULTIWAY_MERGE_SORT_H
#define BALANCED_MULTIWAY_MERGE_SORT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <queue>
#include <functional>
#include "direct_merge_sort.h"
using namespace std;

/// @brief Функция сбалансированной многопутевой сортировки слиянием
/// @param filename имя исходного файла с данными
/// @param m количество путей (временных файлов) для сортировки
/// @return Структура со статистикой сортировки
SortStats balancedMultiwayMergeSort(const string& filename, int m) {
    // Проверка корректности параметра m (количество путей)
    if (m < 2) {
        cerr << "ERROR: Number of ways must be >= 2." << endl;
        return {};
    }

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

    /// Массивы названий временных файлов, используются поочерёдно для чтения и записи
    vector<string> files1, files2;
    for (int i = 0; i < m; ++i) {
        files1.push_back("temp_1_" + to_string(i) + ".txt");
        files2.push_back("temp_2_" + to_string(i) + ".txt");
    }

    // Создаём начальные серии и распределяем исходные данные по m файлам поочерёдно
    
    {
        ifstream source(filename);
        vector<ofstream> dests;
        for (int i = 0; i < m; ++i) {
            dests.emplace_back(files1[i]);
        }
        int file_index = 0;
        while (source >> temp) {
            dests[file_index] << temp << " ";
            stats.swaps++; // Увеличиваем счетчик перестановок
            file_index = (file_index + 1) % m; // Переход к следующему файлу
        }
    }

    // Инициализируем размер серий и указатели на используемые файлы
    long long run_size = 1; // Размер текущих серий
    vector<string>* from_files = &files1; // Файлы, из которых читаем
    vector<string>* to_files = &files2; // Файлы, в которые записываем

    // Основной цикл сортировки - продолжаем, пока размер серий меньше общего количества элементов
    while (run_size < n) {
        vector<ifstream> sources; // Массив потоков файлов для чтения
        for (int i = 0; i < m; ++i) {
            sources.emplace_back((*from_files)[i]);
        }

        vector<ofstream> dests; // Массив потоков файлов для записи
        for (int i = 0; i < m; ++i) {
            dests.emplace_back((*to_files)[i]);
        }

        // Массив для отслеживания количества элементов, уже прочитанных из каждого исходного файла в этой фазе
        vector<long long> elements_read_from_source(m, 0);

        int dest_index = 0; // Индекс файла назначения для записи новой серии

        // Цикл объединения серий
        while (true) {
            // Очередь с приоритетом для нахождения минимального элемента среди всех файлов
            // Хранит пары {значение, индекс_файла_источника}
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

            // Флаг, чтобы определить, есть ли ещё элементы для обработки
            bool elements_remaining = false;

            // Инициализируем очередь первыми элементами из каждой серии (по `run_size` элементов из каждого файла)
            for (int i = 0; i < m; ++i) {
                // Проверяем, не исчерпаны ли элементы из этого файла в этой фазе
                // и можно ли начать новую серию длиной run_size
                if (elements_read_from_source[i] < n && elements_read_from_source[i] % run_size == 0) {
                    // Пытаемся прочитать первый элемент новой серии из файла i
                    if (sources[i] >> temp) {
                        elements_remaining = true; // Найден элемент, продолжаем
                        elements_read_from_source[i]++;
                        pq.push({temp, i}); // Пары {значение, индекс файла}
                    }
                }
            }

            // Если очередь пуста, все элементы обработаны в этой фазе
            if (!elements_remaining) {
                break;
            }

            // Извлекаем элементы из очереди в отсортированном порядке для одной новой серии
            int elements_in_current_output_run = 0;
            const int expected_elements_in_run = m * run_size; // Ожидаемое количество элементов в одной новой серии

            while (!pq.empty() && elements_in_current_output_run < expected_elements_in_run) {
                pair<int, int> min_pair = pq.top();
                pq.pop();

                int value = min_pair.first; // Значение минимального элемента
                int file_idx = min_pair.second; // Индекс файла, из которого взят элемент

                // Записываем минимальный элемент в текущий файл назначения
                dests[dest_index] << value << " ";
                stats.swaps++; // Увеличиваем счетчик перестановок (или записей?)
                elements_in_current_output_run++;

                // Проверяем, не исчерпаны ли элементы из источника file_idx для текущей серии
                // (т.е. не прочитано ли уже `run_size` элементов из этой серии)
                long long series_start_index = (elements_read_from_source[file_idx] / run_size) * run_size;
                long long elements_read_in_current_series = elements_read_from_source[file_idx] - series_start_index;

                if (elements_read_in_current_series < run_size) {
                    if (sources[file_idx] >> temp) {
                        stats.comparisons++;
                        elements_read_from_source[file_idx]++;
                        pq.push({temp, file_idx});
                    }
                }
            }

            // Переходим к следующему файлу назначения для равномерного распределения новой серии
            dest_index = (dest_index + 1) % m;
        }

        // Увеличиваем размер серий в m раз
        run_size *= m;
        // Меняем местами файлы чтения и записи для следующей итерации
        swap(from_files, to_files);
    }

    // Копируем отсортированные данные обратно в исходный файл
    // Результат находится в первом файле последнего набора
    {
        ofstream final_dest(filename);
        ifstream final_source((*from_files)[0]);
        while (final_source >> temp) {
            final_dest << temp << " ";
        }
    }

    // Удаляем все временные файлы
    for (int i = 0; i < m; ++i) {
        remove(files1[i].c_str());
        remove(files2[i].c_str());
    }

    // Засекаем время окончания и вычисляем общее время сортировки
    auto end_time = chrono::high_resolution_clock::now(); // Время окончания сортировки
    stats.time_ms = chrono::duration<double, milli>(end_time - start_time).count();

    return stats;
}

#endif
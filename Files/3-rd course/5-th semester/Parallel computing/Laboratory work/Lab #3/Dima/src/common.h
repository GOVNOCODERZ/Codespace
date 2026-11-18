// common.h
// Вспомогательные функции: генерация данных, доверительный интервал для
// фильтрации выбросов при замерах времени, простые утилиты вывода.
//
// Лабораторная работа №3 "Параллельные алгоритмы сортировки в OpenMP..."
#pragma once

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>

// ---------------------------------------------------------------------
// Генерация данных. Значение зависит от индекса элемента, чтобы массив
// был детерминированным и воспроизводимым между запусками/реализациями.
// ---------------------------------------------------------------------

inline void FillArray(int* array, size_t N) {
    // Знакопеременная "пилообразная" последовательность с достаточным
    // разбросом значений, чтобы не получить почти отсортированный массив.
    for (size_t i = 0; i < N; i++) {
        array[i] = static_cast<int>((1103515245u * (unsigned)(i + 12345) + 12345u) % 1000003u) - 500000;
    }
}

inline void FillArray(double* array, size_t N) {
    for (size_t i = 0; i < N; i++) {
        array[i] = std::sin(static_cast<double>(i)) * 1000.0 + std::cos(static_cast<double>(i) / 2.0) * 137.0;
    }
}

// ---------------------------------------------------------------------
// Доверительный интервал для фильтрации выбросов (Лекция "4 Доверительный
// интервал"). Два варианта: по среднему и по медиане (предпочтительный).
// ---------------------------------------------------------------------

inline double AvgTrustedIntervalAVG(double* times, int cnt) {
    double sum = 0.0;
    for (int i = 0; i < cnt; i++) sum += times[i];
    double avg = sum / cnt;

    double sqsum = 0.0;
    for (int i = 0; i < cnt; i++) sqsum += (times[i] - avg) * (times[i] - avg);
    double sd = std::sqrt(sqsum / (cnt - 1));

    double newSum = 0.0;
    int newCnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (times[i] >= avg - sd && times[i] <= avg + sd) {
            newSum += times[i];
            newCnt++;
        }
    }
    if (newCnt == 0) return avg; // защита от вырожденного случая
    return newSum / newCnt;
}

inline double AvgTrustedIntervalMed(double* times, int cnt) {
    // Среднее до сортировки (используется только для расчёта SD).
    double sum = 0.0;
    for (int i = 0; i < cnt; i++) sum += times[i];
    double avg = sum / cnt;

    std::vector<double> sorted(times, times + cnt);
    std::sort(sorted.begin(), sorted.end()); // любой алгоритм сортировки
    double med = sorted[cnt / 2];

    double sqsum = 0.0;
    for (int i = 0; i < cnt; i++) sqsum += (times[i] - avg) * (times[i] - avg);
    double sd = std::sqrt(sqsum / (cnt - 1));

    double newSum = 0.0;
    int newCnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (times[i] >= med - sd && times[i] <= med + sd) {
            newSum += times[i];
            newCnt++;
        }
    }
    if (newCnt == 0) return med;
    return newSum / newCnt;
}

// ---------------------------------------------------------------------
// Проверка корректности: сравнение с эталоном (последовательным
// результатом). Для double используется допуск.
// ---------------------------------------------------------------------

inline bool ArraysEqual(const int* a, const int* b, size_t N) {
    for (size_t i = 0; i < N; i++) if (a[i] != b[i]) return false;
    return true;
}

inline bool ArraysEqual(const double* a, const double* b, size_t N, double eps = 1e-9) {
    for (size_t i = 0; i < N; i++) if (std::fabs(a[i] - b[i]) > eps) return false;
    return true;
}

inline bool IsSorted(const int* a, size_t N) {
    for (size_t i = 1; i < N; i++) if (a[i - 1] > a[i]) return false;
    return true;
}
inline bool IsSorted(const double* a, size_t N) {
    for (size_t i = 1; i < N; i++) if (a[i - 1] > a[i]) return false;
    return true;
}

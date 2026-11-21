// common.h
// Вспомогательные функции: генерация матриц, доверительный интервал для
// фильтрации выбросов (как в лабораторной работе 3), проверка корректности.
#pragma once

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>

// ---------------------------------------------------------------------
// Генерация квадратной матрицы вещественных чисел размера N x N.
// Значение зависит от индексов (i,j), чтобы матрица была детерминирована
// и воспроизводима между запусками/реализациями. Два разных набора
// (mas1/mas2) получаются через параметр seedShift.
// ---------------------------------------------------------------------
inline void FillMatrix(double* mat, size_t N, int seedShift) {
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            double v = std::sin((double)(i + seedShift) * 0.01) * 50.0
                     + std::cos((double)(j + seedShift) * 0.013) * 50.0
                     + (double)((i * 7 + j * 13 + seedShift) % 97);
            mat[i * N + j] = v;
        }
    }
}

// ---------------------------------------------------------------------
// Доверительный интервал (идентично лабораторной работе 3).
// ---------------------------------------------------------------------
inline double AvgTrustedIntervalMed(double* times, int cnt) {
    double sum = 0.0;
    for (int i = 0; i < cnt; i++) sum += times[i];
    double avg = sum / cnt;

    std::vector<double> sorted(times, times + cnt);
    std::sort(sorted.begin(), sorted.end());
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
// Проверка корректности: сравнение с допуском (вещественные числа).
// ---------------------------------------------------------------------
inline bool ArraysEqual(const double* a, const double* b, size_t N, double eps = 1e-6) {
    for (size_t i = 0; i < N; i++) {
        if (std::fabs(a[i] - b[i]) > eps) return false;
    }
    return true;
}

inline bool ScalarsEqual(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) <= eps;
}

// matrix_ops_omp.h
// Задание 4.1. Операции над квадратными матрицами: последовательные и
// OpenMP-реализации (parallel for, sections, reduction).
#pragma once

#include <omp.h>
#include <cstddef>
#include <algorithm>
#include <limits>

// =======================================================================
// Сложение / поэлементное умножение — последовательные версии.
// =======================================================================
inline double MatAddSeq(const double* a, const double* b, double* c, size_t N) {
    double t0 = omp_get_wtime();
    size_t total = N * N;
    for (size_t k = 0; k < total; k++) c[k] = a[k] + b[k];
    return omp_get_wtime() - t0;
}

inline double MatMulSeq(const double* a, const double* b, double* c, size_t N) {
    double t0 = omp_get_wtime();
    size_t total = N * N;
    for (size_t k = 0; k < total; k++) c[k] = a[k] * b[k];
    return omp_get_wtime() - t0;
}

// =======================================================================
// OpenMP FOR (parallel for, collapse(2)) — независимые итерации по (i,j).
// =======================================================================
inline double MatAddOmpFor(const double* a, const double* b, double* c, size_t N, int nth) {
    omp_set_num_threads(nth);
    long long n = (long long)N;
    double t0 = omp_get_wtime();
    #pragma omp parallel for collapse(2) default(none) shared(a, b, c, n)
    for (long long i = 0; i < n; i++) {
        for (long long j = 0; j < n; j++) {
            c[i * n + j] = a[i * n + j] + b[i * n + j];
        }
    }
    return omp_get_wtime() - t0;
}

inline double MatMulOmpFor(const double* a, const double* b, double* c, size_t N, int nth) {
    omp_set_num_threads(nth);
    long long n = (long long)N;
    double t0 = omp_get_wtime();
    #pragma omp parallel for collapse(2) default(none) shared(a, b, c, n)
    for (long long i = 0; i < n; i++) {
        for (long long j = 0; j < n; j++) {
            c[i * n + j] = a[i * n + j] * b[i * n + j];
        }
    }
    return omp_get_wtime() - t0;
}

// =======================================================================
// OpenMP SECTIONS — задание требует для "нескольких разных задач (2-4 шт.)"
// именно #pragma omp sections. Чтобы варьировать число потоков (2,3,4),
// матрица делится на до 4 блоков строк; при nth < 4 "лишние" секции
// получают пустой диапазон строк и мгновенно завершаются.
// =======================================================================
namespace detail {
inline void AddRowsRange(const double* a, const double* b, double* c, long long n,
                          long long rowFrom, long long rowTo) {
    for (long long i = rowFrom; i < rowTo; i++)
        for (long long j = 0; j < n; j++)
            c[i * n + j] = a[i * n + j] + b[i * n + j];
}
inline void MulRowsRange(const double* a, const double* b, double* c, long long n,
                          long long rowFrom, long long rowTo) {
    for (long long i = rowFrom; i < rowTo; i++)
        for (long long j = 0; j < n; j++)
            c[i * n + j] = a[i * n + j] * b[i * n + j];
}
// Вычисляет границы до 4 блоков строк (блоки за пределами nth — пустые).
inline void ComputeRowChunks(long long n, int nth, long long bounds[5]) {
    bounds[0] = 0;
    for (int k = 0; k < 4; k++) {
        if (k < nth) {
            long long chunk = n / nth + ((k < (n % nth)) ? 1 : 0);
            bounds[k + 1] = bounds[k] + chunk;
        } else {
            bounds[k + 1] = bounds[k]; // пустой блок
        }
    }
}
} // namespace detail

inline double MatAddOmpSections(const double* a, const double* b, double* c, size_t N, int nth) {
    long long n = (long long)N;
    long long bounds[5];
    detail::ComputeRowChunks(n, nth, bounds);

    double t0 = omp_get_wtime();
    #pragma omp parallel sections num_threads(nth) default(none) shared(a, b, c, n, bounds)
    {
        #pragma omp section
        { detail::AddRowsRange(a, b, c, n, bounds[0], bounds[1]); }
        #pragma omp section
        { detail::AddRowsRange(a, b, c, n, bounds[1], bounds[2]); }
        #pragma omp section
        { detail::AddRowsRange(a, b, c, n, bounds[2], bounds[3]); }
        #pragma omp section
        { detail::AddRowsRange(a, b, c, n, bounds[3], bounds[4]); }
    }
    return omp_get_wtime() - t0;
}

inline double MatMulOmpSections(const double* a, const double* b, double* c, size_t N, int nth) {
    long long n = (long long)N;
    long long bounds[5];
    detail::ComputeRowChunks(n, nth, bounds);

    double t0 = omp_get_wtime();
    #pragma omp parallel sections num_threads(nth) default(none) shared(a, b, c, n, bounds)
    {
        #pragma omp section
        { detail::MulRowsRange(a, b, c, n, bounds[0], bounds[1]); }
        #pragma omp section
        { detail::MulRowsRange(a, b, c, n, bounds[1], bounds[2]); }
        #pragma omp section
        { detail::MulRowsRange(a, b, c, n, bounds[2], bounds[3]); }
        #pragma omp section
        { detail::MulRowsRange(a, b, c, n, bounds[3], bounds[4]); }
    }
    return omp_get_wtime() - t0;
}

// =======================================================================
// Сумма всех элементов (mas1 + mas2) — последовательно и через
// reduction(+:sum) (единственный требуемый подход для OpenMP по заданию).
// =======================================================================
inline double SumElSeq(const double* a, const double* b, size_t N, double& outSum) {
    double t0 = omp_get_wtime();
    size_t total = N * N;
    double sum = 0.0;
    for (size_t k = 0; k < total; k++) sum += a[k] + b[k];
    outSum = sum;
    return omp_get_wtime() - t0;
}

inline double SumElOmpReduction(const double* a, const double* b, size_t N, int nth, double& outSum) {
    omp_set_num_threads(nth);
    long long total = (long long)(N * N);
    double sum = 0.0;
    double t0 = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum) default(none) shared(a, b, total)
    for (long long k = 0; k < total; k++) sum += a[k] + b[k];
    outSum = sum;
    return omp_get_wtime() - t0;
}

// =======================================================================
// Val_range = max(mas1,mas2) - min(mas1,mas2) — последовательно и через
// reduction(max:) reduction(min:) (OpenMP 4.5+).
// =======================================================================
inline double ValRangeSeq(const double* a, const double* b, size_t N, double& outRange) {
    double t0 = omp_get_wtime();
    size_t total = N * N;
    double mx = -std::numeric_limits<double>::max();
    double mn = std::numeric_limits<double>::max();
    for (size_t k = 0; k < total; k++) {
        mx = std::max({ mx, a[k], b[k] });
        mn = std::min({ mn, a[k], b[k] });
    }
    outRange = mx - mn;
    return omp_get_wtime() - t0;
}

inline double ValRangeOmpReduction(const double* a, const double* b, size_t N, int nth, double& outRange) {
    omp_set_num_threads(nth);
    long long total = (long long)(N * N);
    double mx = -std::numeric_limits<double>::max();
    double mn = std::numeric_limits<double>::max();
    double t0 = omp_get_wtime();
    #pragma omp parallel for reduction(max:mx) reduction(min:mn) default(none) shared(a, b, total)
    for (long long k = 0; k < total; k++) {
        if (a[k] > mx) mx = a[k];
        if (b[k] > mx) mx = b[k];
        if (a[k] < mn) mn = a[k];
        if (b[k] < mn) mn = b[k];
    }
    outRange = mx - mn;
    return omp_get_wtime() - t0;
}

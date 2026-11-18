// sorts.h
// Реализации алгоритмов сортировки (последовательные и параллельные
// варианты на OpenMP) для Задания 3.1.
#pragma once

#include <omp.h>
#include <cstddef>
#include <algorithm>
#include <utility>
#include <chrono>
// =======================================================================
// 1. Сортировка пузырьком (классический алгоритм). Только последовательная
//    реализация — так требует задание.
// =======================================================================




inline double GetTime() {
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

template <typename T>
double BubbleClassicSeq(T* array, size_t N) {
    double t0 = GetTime();
    for (size_t i = 0; i < N - 1; i++) {
        for (size_t j = 0; j < N - 1 - i; j++) {
            if (array[j] > array[j + 1]) {
                std::swap(array[j], array[j + 1]);
            }
        }
    }
    return GetTime() - t0;
}

// =======================================================================
// 2. Сортировка пузырьком — алгоритм чет-нечетной перестановки
//    (odd-even transposition sort).
// =======================================================================

// Последовательная реализация (вариант с ранним выходом, аналог
// предоставленного примера oddEvenSortingV3).
template <typename T>
double OddEvenSeq(T* array, size_t N) {
    double t0 = GetTime();
    bool swapped;
    do {
        swapped = false;
        for (size_t i = 0; i + 1 < N; i += 2) {
            if (array[i] > array[i + 1]) { std::swap(array[i], array[i + 1]); swapped = true; }
        }
        for (size_t i = 1; i + 1 < N; i += 2) {
            if (array[i] > array[i + 1]) { std::swap(array[i], array[i + 1]); swapped = true; }
        }
    } while (swapped);
    return GetTime() - t0;
}

// Параллельная реализация: каждая из двух фаз (чётная/нечётная) состоит
// из полностью независимых сравнений-обменов -> распараллеливаем по
// индексам внутри каждой фазы (#pragma omp parallel for).
// Флаг "было ли изменение" собирается через reduction(||:...), чтобы
// избежать гонки данных (см. чек-лист типичных ошибок, п. OpenMP).
template <typename T>
double OddEvenParallel(T* array, size_t N, int nthreads) {
    omp_set_num_threads(nthreads);
    double t0 = GetTime();
    bool anySwap;
    do {
        anySwap = false;
        long long limit0 = (long long)N - 1;
        #pragma omp parallel for reduction(||:anySwap) default(none) shared(array, limit0)
        for (long long i = 0; i < limit0; i += 2) {
            if (array[i] > array[i + 1]) { std::swap(array[i], array[i + 1]); anySwap = true; }
        }
        long long limit1 = (long long)N - 1;
        #pragma omp parallel for reduction(||:anySwap) default(none) shared(array, limit1)
        for (long long i = 1; i < limit1; i += 2) {
            if (array[i] > array[i + 1]) { std::swap(array[i], array[i + 1]); anySwap = true; }
        }
    } while (anySwap);
    return GetTime() - t0;
}

// =======================================================================
// 3. Сортировка Шелла.
// =======================================================================

template <typename T>
double ShellSeq(T* array, size_t N) {
    double t0 = GetTime();
    for (size_t gap = N / 2; gap > 0; gap /= 2) {
        for (size_t i = gap; i < N; i++) {
            T temp = array[i];
            size_t j = i;
            while (j >= gap && array[j - gap] > temp) {
                array[j] = array[j - gap];
                j -= gap;
            }
            array[j] = temp;
        }
    }
    return GetTime() - t0;
}

// Параллельная реализация: для фиксированного gap массив распадается на
// `gap` независимых прореженных подпоследовательностей (с шагом gap) —
// их можно сортировать вставками одновременно в разных потоках.
// Между проходами с разным gap стоит неявный барьер в конце `parallel for`,
// что обязательно, т.к. следующий gap зависит от результата предыдущего.
template <typename T>
double ShellParallel(T* array, size_t N, int nthreads) {
    omp_set_num_threads(nthreads);
    double t0 = GetTime();
    for (size_t gap = N / 2; gap > 0; gap /= 2) {
        long long g = (long long)gap;
        #pragma omp parallel for default(none) shared(array, N, g)
        for (long long start = 0; start < g; start++) {
            for (long long i = start + g; i < (long long)N; i += g) {
                T temp = array[i];
                long long j = i;
                while (j >= g && array[j - g] > temp) {
                    array[j] = array[j - g];
                    j -= g;
                }
                array[j] = temp;
            }
        }
    }
    return GetTime() - t0;
}

// =======================================================================
// 4. Быстрая сортировка (qsort).
// =======================================================================

template <typename T>
static long long PartitionLomuto(T* array, long long lo, long long hi) {
    // Выбор опорного элемента — средний (снижает деградацию на
    // упорядоченных/симметричных данных по сравнению с крайним элементом).
    long long mid = lo + (hi - lo) / 2;
    std::swap(array[mid], array[hi]);
    T pivot = array[hi];
    long long i = lo - 1;
    for (long long j = lo; j < hi; j++) {
        if (array[j] <= pivot) {
            i++;
            std::swap(array[i], array[j]);
        }
    }
    std::swap(array[i + 1], array[hi]);
    return i + 1;
}

template <typename T>
static void QuickSortSeqImpl(T* array, long long lo, long long hi) {
    while (lo < hi) {
        long long p = PartitionLomuto(array, lo, hi);
        // Хвостовая рекурсия по большей части заменена циклом (экономия стека).
        if (p - lo < hi - p) {
            QuickSortSeqImpl(array, lo, p - 1);
            lo = p + 1;
        } else {
            QuickSortSeqImpl(array, p + 1, hi);
            hi = p - 1;
        }
    }
}

template <typename T>
double QuickSortSeq(T* array, size_t N) {
    double t0 = GetTime();
    if (N > 1) QuickSortSeqImpl(array, 0LL, (long long)N - 1);
    return GetTime() - t0;
}

// Параллельная реализация на задачах OpenMP (#pragma omp task).
// При малых подзадачах (< THRESHOLD) переключаемся на последовательную
// версию — иначе накладные расходы на создание задач превысят выгоду.
template <typename T>
static void QuickSortTask(T* array, long long lo, long long hi, long long threshold) {
    if (lo >= hi) return;
    if (hi - lo < threshold) {
        QuickSortSeqImpl(array, lo, hi);
        return;
    }
    long long p = PartitionLomuto(array, lo, hi);
    #pragma omp task default(none) firstprivate(array, lo, p, threshold)
    QuickSortTask(array, lo, p - 1, threshold);
    #pragma omp task default(none) firstprivate(array, p, hi, threshold)
    QuickSortTask(array, p + 1, hi, threshold);
    #pragma omp taskwait
}

template <typename T>
double QuickSortParallel(T* array, size_t N, int nthreads) {
    omp_set_num_threads(nthreads);
    long long threshold = std::max<long long>(1000, (long long)N / (nthreads * 8 + 1));
    double t0 = GetTime();
    if (N > 1) {
        #pragma omp parallel default(none) shared(array, N, threshold)
        {
            #pragma omp single
            QuickSortTask(array, 0LL, (long long)N - 1, threshold);
        }
    }
    return GetTime() - t0;
}

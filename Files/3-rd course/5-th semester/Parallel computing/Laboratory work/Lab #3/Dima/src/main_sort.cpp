// main_sort.cpp
// Задание 3.1. Консольное приложение, реализующее сортировку одномерного
// массива разными алгоритмами (последовательная и параллельная реализация
// на OpenMP): пузырёк классический (только посл.), чет-нечетная
// перестановка, Шелла, быстрая сортировка (qsort).
//
// Эксперимент проводится на трёх наборах данных (20000-50000 элементов),
// для типов int и double, не менее 30 запусков на реализацию
// (>=20 для параллельных, >=5 для последовательных — по заданию).
// Результат: сводная таблица (аналог табл. 3.1) + CSV-файл с данными.
//
// Сборка: g++ -O2 -fopenmp -std=c++17 main_sort.cpp -o sort_app
#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "common.h"
#include "sorts.h"

// Количество запусков по заданию
static const int RUNS_PARALLEL = 30; // >= 20 требуемых по заданию
static const int RUNS_SEQ      = 5;  // >= 5 требуемых по заданию

// Наборы данных (3 набора по заданию, диапазон 20000-50000)
static const size_t SIZES[3] = { 20000, 35000, 50000 };

// Число потоков для параллельных реализаций
static const int THREADS[3] = { 2, 3, 4 };

struct ResultRow {
    std::string func;     // название функции сортировки
    std::string threadsS; // "-" для посл., иначе число потоков
    double time[3];       // время на каждый из 3 наборов
    double sp[3];         // ускорение
    double ep[3];         // эффективность
};

template <typename T>
static void RunOne(const char* name, double(*sortFn)(T*, size_t),
                    T* base, size_t N, double* times, int runs,
                    T* refSorted, bool* okOut) {
    for (int r = 0; r < runs; r++) {
        std::vector<T> arr(base, base + N);
        times[r] = sortFn(arr.data(), N);
        if (r == runs - 1) {
            *okOut = IsSorted(arr.data(), N) && ArraysEqual(arr.data(), refSorted, N);
        }
    }
}

template <typename T>
static void RunOnePar(const char* name, double(*sortFn)(T*, size_t, int),
                       T* base, size_t N, int nth, double* times, int runs,
                       T* refSorted, bool* okOut) {
    for (int r = 0; r < runs; r++) {
        std::vector<T> arr(base, base + N);
        times[r] = sortFn(arr.data(), N, nth);
        if (r == runs - 1) {
            *okOut = IsSorted(arr.data(), N) && ArraysEqual(arr.data(), refSorted, N);
        }
    }
}

template <typename T>
static void RunDataType(const char* typeName, std::ofstream& csv) {
    std::cout << "\n=============== Тип данных: " << typeName << " ===============\n";

    std::vector<ResultRow> rows;
    // индекс по функциям: classic, oddeven_seq, oddeven_p2, oddeven_p3, oddeven_p4,
    // shell_seq, shell_p2.. , quick_seq, quick_p2..
    const char* funcNames[] = {
        "Классический пузырек", "Чет-нечетной перестановки (посл.)",
        "Чет-нечет перестановки (парал.)", "Чет-нечет перестановки (парал.)", "Чет-нечет перестановки (парал.)",
        "Шелла (посл.)",
        "Шелла (парал.)", "Шелла (парал.)", "Шелла (парал.)",
        "Быстрой сортировки (посл.)",
        "Быстрой сортировки (парал.)", "Быстрой сортировки (парал.)", "Быстрой сортировки (парал.)"
    };
    const char* threadsLbl[] = { "-", "-", "2", "3", "4", "-", "2", "3", "4", "-", "2", "3", "4" };
    const int NFUNC = 13;
    std::vector<std::vector<double>> allTimes(NFUNC); // [func][sizeIdx] -> avg time
    std::vector<std::vector<double>> allSp(NFUNC);
    std::vector<std::vector<double>> allEp(NFUNC);
    for (int f = 0; f < NFUNC; f++) { allTimes[f].resize(3); allSp[f].resize(3); allEp[f].resize(3); }

    for (int sIdx = 0; sIdx < 3; sIdx++) {
        size_t N = SIZES[sIdx];
        std::cout << "\n--- Набор данных: N=" << N << " ---\n";

        std::vector<T> base(N);
        FillArray(base.data(), N);

        // Эталон для проверки корректности — std::sort на копии
        std::vector<T> ref(base);
        std::sort(ref.begin(), ref.end());

        double T1 = 0.0; // время последовательной (эталонной для Sp/Ep) сортировки
        bool ok;

        // 1. Классический пузырёк (посл.)
        {
            std::vector<double> times(RUNS_SEQ);
            RunOne<T>("classic", BubbleClassicSeq<T>, base.data(), N, times.data(), RUNS_SEQ, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
            allTimes[0][sIdx] = avg; allSp[0][sIdx] = 1.0; allEp[0][sIdx] = 1.0;
            std::cout << std::fixed << std::setprecision(6)
                      << "Классический пузырек (посл.): t=" << avg << " c, корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }

        // 2. Чет-нечетная (посл.) — служит базой T1 для Sp/Ep чет-нечетной
        double T1_oe;
        {
            std::vector<double> times(RUNS_SEQ);
            RunOne<T>("oe_seq", OddEvenSeq<T>, base.data(), N, times.data(), RUNS_SEQ, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
            T1_oe = avg;
            allTimes[1][sIdx] = avg; allSp[1][sIdx] = 1.0; allEp[1][sIdx] = 1.0;
            std::cout << "Чет-нечетной перестановки (посл.): t=" << avg << " c, корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }
        // 2п. Чет-нечетная (парал.) threads=2,3,4
        for (int th = 0; th < 3; th++) {
            int nth = THREADS[th];
            std::vector<double> times(RUNS_PARALLEL);
            RunOnePar<T>("oe_par", OddEvenParallel<T>, base.data(), N, nth, times.data(), RUNS_PARALLEL, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_PARALLEL);
            int idx = 2 + th;
            allTimes[idx][sIdx] = avg;
            allSp[idx][sIdx] = T1_oe / avg;
            allEp[idx][sIdx] = allSp[idx][sIdx] / nth;
            std::cout << "Чет-нечет перестановки (парал., p=" << nth << "): t=" << avg
                      << " c, Sp=" << allSp[idx][sIdx] << ", Ep=" << allEp[idx][sIdx]
                      << ", корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }

        // 3. Шелла (посл.)
        double T1_shell;
        {
            std::vector<double> times(RUNS_SEQ);
            RunOne<T>("shell_seq", ShellSeq<T>, base.data(), N, times.data(), RUNS_SEQ, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
            T1_shell = avg;
            allTimes[5][sIdx] = avg; allSp[5][sIdx] = 1.0; allEp[5][sIdx] = 1.0;
            std::cout << "Шелла (посл.): t=" << avg << " c, корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }
        for (int th = 0; th < 3; th++) {
            int nth = THREADS[th];
            std::vector<double> times(RUNS_PARALLEL);
            RunOnePar<T>("shell_par", ShellParallel<T>, base.data(), N, nth, times.data(), RUNS_PARALLEL, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_PARALLEL);
            int idx = 6 + th;
            allTimes[idx][sIdx] = avg;
            allSp[idx][sIdx] = T1_shell / avg;
            allEp[idx][sIdx] = allSp[idx][sIdx] / nth;
            std::cout << "Шелла (парал., p=" << nth << "): t=" << avg
                      << " c, Sp=" << allSp[idx][sIdx] << ", Ep=" << allEp[idx][sIdx]
                      << ", корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }

        // 4. Быстрая сортировка (посл.)
        double T1_quick;
        {
            std::vector<double> times(RUNS_SEQ);
            RunOne<T>("quick_seq", QuickSortSeq<T>, base.data(), N, times.data(), RUNS_SEQ, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
            T1_quick = avg;
            allTimes[9][sIdx] = avg; allSp[9][sIdx] = 1.0; allEp[9][sIdx] = 1.0;
            std::cout << "Быстрой сортировки (посл.): t=" << avg << " c, корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }
        for (int th = 0; th < 3; th++) {
            int nth = THREADS[th];
            std::vector<double> times(RUNS_PARALLEL);
            RunOnePar<T>("quick_par", QuickSortParallel<T>, base.data(), N, nth, times.data(), RUNS_PARALLEL, ref.data(), &ok);
            double avg = AvgTrustedIntervalMed(times.data(), RUNS_PARALLEL);
            int idx = 10 + th;
            allTimes[idx][sIdx] = avg;
            allSp[idx][sIdx] = T1_quick / avg;
            allEp[idx][sIdx] = allSp[idx][sIdx] / nth;
            std::cout << "Быстрой сортировки (парал., p=" << nth << "): t=" << avg
                      << " c, Sp=" << allSp[idx][sIdx] << ", Ep=" << allEp[idx][sIdx]
                      << ", корректность=" << (ok ? "OK" : "FAIL") << "\n";
        }
    }

    // CSV
    for (int f = 0; f < NFUNC; f++) {
        for (int s = 0; s < 3; s++) {
            csv << typeName << ";" << funcNames[f] << ";" << threadsLbl[f] << ";" << SIZES[s] << ";"
                << allTimes[f][s] << ";" << allSp[f][s] << ";" << allEp[f][s] << "\n";
        }
    }
}

int main() {
    std::cout << "Задание 3.1. Параллельные алгоритмы сортировки (OpenMP)\n";
    std::cout << "omp_get_num_procs() = " << omp_get_num_procs() << "\n";
    std::cout << "omp_get_max_threads() = " << omp_get_max_threads() << "\n";
    std::cout << "Наборы данных N: 20000, 35000, 50000\n";
    std::cout << "Запусков: посл.=" << RUNS_SEQ << ", парал.=" << RUNS_PARALLEL << "\n";

    std::ofstream csv("../data/sort_results.csv");
    csv << "type;function;threads;N;time_s;Sp;Ep\n";

    RunDataType<int>("int", csv);
    RunDataType<double>("double", csv);

    csv.close();
    std::cout << "\nГотово. Результаты сохранены в data/sort_results.csv\n";
    return 0;
}

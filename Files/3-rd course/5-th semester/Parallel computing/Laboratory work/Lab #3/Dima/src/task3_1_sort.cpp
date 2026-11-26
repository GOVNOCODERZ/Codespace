#include "common_stats.h"

#include <Windows.h>
#include <omp.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// Подсчёт видимой ширины UTF-8 строки для ровного форматирования консольных таблиц
static size_t utf8Len(const std::string& s) {
    size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0u) != 0x80u) {
            count++;
        }
    }
    return count;
}

static std::string padRightU8(const std::string& s, size_t width) {
    size_t len = utf8Len(s);
    return (len < width) ? (s + std::string(width - len, ' ')) : s;
}

static std::string padLeftU8(const std::string& s, size_t width) {
    size_t len = utf8Len(s);
    return (len < width) ? (std::string(width - len, ' ') + s) : s;
}

// ============================================================================
// Детерминированная функция генерации исходного массива в зависимости от индекса
// ============================================================================
template <typename T>
T generateElement(int index);

template <>
inline int generateElement<int>(int index) {
    // Комбинация гармонических функций и модульной арифметики обеспечивает
    // равномерный разброс без упорядоченности и зависит строго от индекса
    double trig = 10000.0 * std::sin(0.37 * index + 0.5) + 5000.0 * std::cos(1.13 * index);
    int modPart = ((index * 7919 + 104729) % 10000) - 5000;
    return static_cast<int>(trig) + modPart;
}

template <>
inline double generateElement<double>(int index) {
    double trig = 1000.0 * std::sin(0.37 * index + 0.5) + 500.0 * std::cos(1.13 * index);
    double modPart = static_cast<double>((index * 7919 + 104729) % 10000) * 0.01;
    return trig + modPart;
}

template <typename T>
std::vector<T> generateDataset(int n) {
    std::vector<T> data(static_cast<size_t>(n));
    for (int i = 0; i < n; i++) {
        data[static_cast<size_t>(i)] = generateElement<T>(i);
    }
    return data;
}

template <typename T>
bool verifySorted(const T* arr, int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// 1. Классический алгоритм сортировки пузырьком (только последовательный)
// ============================================================================
template <typename T>
void bubbleSortClassicSeq(T* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

// ============================================================================
// 2. Сортировка пузырьком: алгоритм Чет-нечетной перестановки
// ============================================================================
template <typename T>
void oddEvenSortSeq(T* arr, int n) {
    for (int phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) {
            for (int i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    std::swap(arr[i - 1], arr[i]);
                }
            }
        } else {
            for (int i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    std::swap(arr[i], arr[i + 1]);
                }
            }
        }
    }
}

// Вспомогательная операция Compare-Split (слияние-разделение двух смежных отсортированных блоков)
template <typename T>
void compareSplitBlocks(T* arr, T* scratch, int leftStart, int leftLen, int rightLen) {
    const T* leftPtr = arr + leftStart;
    const T* rightPtr = arr + leftStart + leftLen;
    T* outPtr = scratch + leftStart;

    int i = 0;
    int j = 0;
    int k = 0;
    while (i < leftLen && j < rightLen) {
        if (leftPtr[i] <= rightPtr[j]) {
            outPtr[k++] = leftPtr[i++];
        } else {
            outPtr[k++] = rightPtr[j++];
        }
    }
    while (i < leftLen) {
        outPtr[k++] = leftPtr[i++];
    }
    while (j < rightLen) {
        outPtr[k++] = rightPtr[j++];
    }
    std::memcpy(arr + leftStart, outPtr, static_cast<size_t>(leftLen + rightLen) * sizeof(T));
}

// Параллельная блочная чёт-нечётная перестановка (модель Гергеля с OpenMP)
template <typename T>
void oddEvenSortPar(T* arr, int n, int numThreads, T* scratch) {
    int p = std::min(numThreads, n);
    if (p <= 1) {
        oddEvenSortSeq(arr, n);
        return;
    }

    // Разбиение массива на p смежных блоков с разницей длин не более 1 элемента
    std::vector<int> starts(static_cast<size_t>(p) + 1, 0);
    int baseLen = n / p;
    int rem = n % p;
    for (int b = 0; b < p; b++) {
        starts[static_cast<size_t>(b) + 1] = starts[static_cast<size_t>(b)] + baseLen + (b < rem ? 1 : 0);
    }

    // Единая параллельная область исключает повторные накладные расходы fork-join
    #pragma omp parallel num_threads(p) default(none) shared(arr, scratch, starts, p)
    {
        int tid = omp_get_thread_num();
        int myStart = starts[static_cast<size_t>(tid)];
        int myLen = starts[static_cast<size_t>(tid) + 1] - myStart;

        // Шаг 1: локальная сортировка собственного блока методом чёт-нечётной перестановки
        oddEvenSortSeq(arr + myStart, myLen);

        #pragma omp barrier

        // Шаг 2: p фаз чёт-нечётного парного слияния-разделения (Compare-Split) смежных блоков
        for (int phase = 0; phase < p; phase++) {
            if (phase % 2 == 0) {
                if (tid % 2 == 0 && tid + 1 < p) {
                    int rightLen = starts[static_cast<size_t>(tid) + 2] - starts[static_cast<size_t>(tid) + 1];
                    compareSplitBlocks(arr, scratch, myStart, myLen, rightLen);
                }
            } else {
                if (tid % 2 == 1 && tid + 1 < p) {
                    int rightLen = starts[static_cast<size_t>(tid) + 2] - starts[static_cast<size_t>(tid) + 1];
                    compareSplitBlocks(arr, scratch, myStart, myLen, rightLen);
                }
            }
            #pragma omp barrier
        }
    }
}

// ============================================================================
// 3. Сортировка Шелла (последовательность шагов Кнута: h = 3*h + 1)
// ============================================================================
template <typename T>
void shellSortSeq(T* arr, int n) {
    int h = 1;
    while (h < n / 3) {
        h = 3 * h + 1;
    }
    for (; h > 0; h /= 3) {
        for (int start = 0; start < h; start++) {
            for (int i = start + h; i < n; i += h) {
                T key = arr[i];
                int j = i;
                while (j >= h && arr[j - h] > key) {
                    arr[j] = arr[j - h];
                    j -= h;
                }
                arr[j] = key;
            }
        }
    }
}

template <typename T>
void shellSortPar(T* arr, int n, int numThreads) {
    int hStart = 1;
    while (hStart < n / 3) {
        hStart = 3 * hStart + 1;
    }

    // На каждом шаге h независимые h-подмассивы сортируются параллельно
    #pragma omp parallel num_threads(numThreads) default(none) shared(arr, n, hStart)
    {
        for (int h = hStart; h > 0; h /= 3) {
            #pragma omp for schedule(static)
            for (int start = 0; start < h; start++) {
                for (int i = start + h; i < n; i += h) {
                    T key = arr[i];
                    int j = i;
                    while (j >= h && arr[j - h] > key) {
                        arr[j] = arr[j - h];
                        j -= h;
                    }
                    arr[j] = key;
                }
            }
        }
    }
}

// ============================================================================
// 4. Быстрая сортировка (qsort: разбиение Хоара + OpenMP tasks)
// ============================================================================
template <typename T>
void quickSortSeqImpl(T* arr, int left, int right) {
    while (left < right) {
        int i = left;
        int j = right;
        T pivot = arr[left + ((right - left) >> 1)];
        while (i <= j) {
            while (arr[i] < pivot) i++;
            while (arr[j] > pivot) j--;
            if (i <= j) {
                std::swap(arr[i], arr[j]);
                i++;
                j--;
            }
        }
        // Устранение хвостовой рекурсии гарантирует глубину стека O(log n)
        if (j - left < right - i) {
            if (left < j) {
                quickSortSeqImpl(arr, left, j);
            }
            left = i;
        } else {
            if (i < right) {
                quickSortSeqImpl(arr, i, right);
            }
            right = j;
        }
    }
}

template <typename T>
void quickSortSeq(T* arr, int n) {
    if (n > 1) {
        quickSortSeqImpl(arr, 0, n - 1);
    }
}

template <typename T>
void quickSortParImpl(T* arr, int left, int right, int cutoff) {
    if (right - left < cutoff) {
        quickSortSeqImpl(arr, left, right);
        return;
    }
    int i = left;
    int j = right;
    T pivot = arr[left + ((right - left) >> 1)];
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            std::swap(arr[i], arr[j]);
            i++;
            j--;
        }
    }

    // Левая ветвь выделяется в независимую задачу OpenMP, правая выполняется текущим потоком
    if (left < j) {
        #pragma omp task default(none) shared(arr) firstprivate(left, j, cutoff)
        {
            quickSortParImpl(arr, left, j, cutoff);
        }
    }
    if (i < right) {
        quickSortParImpl(arr, i, right, cutoff);
    }
    #pragma omp taskwait
}

template <typename T>
void quickSortPar(T* arr, int n, int numThreads) {
    if (n <= 1) {
        return;
    }
    int cutoff = std::max(512, n / (numThreads * 8));
    #pragma omp parallel num_threads(numThreads) default(none) shared(arr, n, cutoff)
    {
        #pragma omp single nowait
        {
            quickSortParImpl(arr, 0, n - 1, cutoff);
        }
    }
}

// ============================================================================
// Проведение серии замеров через omp_get_wtime() с пакетной компенсацией тика
// ============================================================================
struct BenchmarkRow {
    std::string dtype;
    int n = 0;
    std::string algo;
    int threads = 1;
    int runs = 0;
    int batchSize = 1;
    TrustedStats stats;
    double speedup = 1.0;
    double efficiency = 1.0;
    double costMs = 0.0;
    std::vector<double> rawTimesMs;
};

template <typename T, typename SortFunc>
BenchmarkRow runSeries(const std::string& dtype,
                       int n,
                       const std::string& algo,
                       int threads,
                       int runs,
                       int batchSize,
                       const std::vector<T>& baseData,
                       SortFunc sortFunc) {
    // Предварительное выделение рабочих копий исходного массива вне замера времени
    std::vector<std::vector<T>> copies(static_cast<size_t>(batchSize), std::vector<T>(static_cast<size_t>(n)));

    // Прогрев кэша и пула потоков OpenMP (не включается в статистику)
    std::memcpy(copies[0].data(), baseData.data(), static_cast<size_t>(n) * sizeof(T));
    sortFunc(copies[0].data(), n);
    if (!verifySorted(copies[0].data(), n)) {
        throw std::runtime_error("Ошибка верификации сортировки: " + algo + " (" + dtype + ", n=" + std::to_string(n) + ")");
    }

    double* timesMs = new double[runs];
    std::vector<double> rawCopy(static_cast<size_t>(runs));

    for (int r = 0; r < runs; r++) {
        // Копирование исходных данных в рабочий массив выполняется ДО начала замера
        for (int b = 0; b < batchSize; b++) {
            std::memcpy(copies[static_cast<size_t>(b)].data(), baseData.data(), static_cast<size_t>(n) * sizeof(T));
        }

        double tStart = omp_get_wtime();
        for (int b = 0; b < batchSize; b++) {
            sortFunc(copies[static_cast<size_t>(b)].data(), n);
        }
        double tEnd = omp_get_wtime();

        double elapsedMs = ((tEnd - tStart) * 1000.0) / static_cast<double>(batchSize);
        timesMs[r] = elapsedMs;
        rawCopy[static_cast<size_t>(r)] = elapsedMs;
    }

    // Проверка корректности последнего прогона
    if (!verifySorted(copies[0].data(), n)) {
        delete[] timesMs;
        throw std::runtime_error("Массив не отсортирован после серии замеров: " + algo);
    }

    BenchmarkRow row;
    row.dtype = dtype;
    row.n = n;
    row.algo = algo;
    row.threads = threads;
    row.runs = runs;
    row.batchSize = batchSize;
    row.stats = computeTrustedStats(timesMs, runs);
    row.rawTimesMs = std::move(rawCopy);
    delete[] timesMs;
    return row;
}

template <typename T>
void runAllForType(const std::string& dtypeName,
                   const std::vector<int>& sizes,
                   const std::vector<int>& parThreadsQuad,
                   const std::vector<int>& parThreadsFast,
                   int runsBubble,
                   int runsOddEven,
                   int runsFast,
                   std::vector<BenchmarkRow>& allRows) {
    std::cout << "\n=========================================================================================\n";
    std::cout << "  Тип данных: " << dtypeName << "\n";
    std::cout << "=========================================================================================\n";

    for (int n : sizes) {
        std::vector<T> baseData = generateDataset<T>(n);
        std::vector<T> scratch(static_cast<size_t>(n));

        std::cout << "\n--- Набор данных: n = " << n << " элементов ---\n";
        std::cout << padRightU8("Алгоритм", 24)
                  << padLeftU8("Пот.", 6)
                  << padLeftU8("Зам.", 7)
                  << padLeftU8("Исх.ср.(мс)", 13)
                  << padLeftU8("Отбр.", 8)
                  << padLeftU8("Итог(мс)", 13)
                  << padLeftU8("Sp(n)", 9)
                  << padLeftU8("Ep(n)", 9)
                  << padLeftU8("Cp(мс)", 13)
                  << "\n";
        std::cout << std::string(102, '-') << "\n";

        auto printAndStore = [&](BenchmarkRow& row, double t1Ms) {
            row.speedup = (row.stats.trustedAvgMed > 0.0) ? (t1Ms / row.stats.trustedAvgMed) : 1.0;
            row.efficiency = row.speedup / static_cast<double>(row.threads);
            row.costMs = static_cast<double>(row.threads) * row.stats.trustedAvgMed;

            std::cout << padRightU8(row.algo, 24)
                      << std::right << std::fixed
                      << std::setw(6) << row.threads
                      << std::setw(7) << row.runs
                      << std::setw(13) << std::setprecision(4) << row.stats.rawAvg
                      << std::setw(8) << row.stats.discardedRuns
                      << std::setw(13) << std::setprecision(4) << row.stats.trustedAvgMed
                      << std::setw(9) << std::setprecision(4) << row.speedup
                      << std::setw(9) << std::setprecision(4) << row.efficiency
                      << std::setw(13) << std::setprecision(4) << row.costMs
                      << "\n" << std::flush;
            allRows.push_back(row);
        };

        // 1. Классический пузырёк (только последовательный)
        {
            BenchmarkRow r = runSeries<T>(dtypeName, n, "Bubble_Classic_Seq", 1, runsBubble, 1, baseData,
                [](T* a, int len) { bubbleSortClassicSeq(a, len); });
            printAndStore(r, r.stats.trustedAvgMed);
        }

        // 2. Чет-нечетная перестановка (последовательный и параллельный)
        double t1OddEven = 0.0;
        {
            BenchmarkRow r = runSeries<T>(dtypeName, n, "OddEven_Seq", 1, runsOddEven, 1, baseData,
                [](T* a, int len) { oddEvenSortSeq(a, len); });
            t1OddEven = r.stats.trustedAvgMed;
            printAndStore(r, t1OddEven);
        }
        for (int p : parThreadsQuad) {
            // При больших p блочная сортировка выполняется быстро, используем batch для точности omp_get_wtime()
            int batch = (p >= 6 && n <= 20000) ? 10 : ((p >= 4 && n <= 20000) ? 4 : ((p >= 8) ? 5 : 1));
            BenchmarkRow r = runSeries<T>(dtypeName, n, "OddEven_Par", p, runsOddEven, batch, baseData,
                [p, &scratch](T* a, int len) { oddEvenSortPar(a, len, p, scratch.data()); });
            printAndStore(r, t1OddEven);
        }

        // 3. Сортировка Шелла (последовательный и параллельный)
        double t1Shell = 0.0;
        {
            int batch = (n <= 20000) ? 30 : 20;
            BenchmarkRow r = runSeries<T>(dtypeName, n, "Shell_Seq", 1, runsFast, batch, baseData,
                [](T* a, int len) { shellSortSeq(a, len); });
            t1Shell = r.stats.trustedAvgMed;
            printAndStore(r, t1Shell);
        }
        for (int p : parThreadsFast) {
            int batch = (n <= 20000) ? 40 : 25;
            BenchmarkRow r = runSeries<T>(dtypeName, n, "Shell_Par", p, runsFast, batch, baseData,
                [p](T* a, int len) { shellSortPar(a, len, p); });
            printAndStore(r, t1Shell);
        }

        // 4. Быстрая сортировка qsort (последовательный и параллельный)
        double t1Qsort = 0.0;
        {
            int batch = (n <= 20000) ? 40 : 25;
            BenchmarkRow r = runSeries<T>(dtypeName, n, "QuickSort_Seq", 1, runsFast, batch, baseData,
                [](T* a, int len) { quickSortSeq(a, len); });
            t1Qsort = r.stats.trustedAvgMed;
            printAndStore(r, t1Qsort);
        }
        for (int p : parThreadsFast) {
            int batch = (n <= 20000) ? 50 : 30;
            BenchmarkRow r = runSeries<T>(dtypeName, n, "QuickSort_Par", p, runsFast, batch, baseData,
                [p](T* a, int len) { quickSortPar(a, len, p); });
            printAndStore(r, t1Qsort);
        }
    }
}

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    bool demoMode = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--demo") {
            demoMode = true;
        }
    }

    std::cout << "=========================================================================================\n";
    std::cout << "  Лабораторная работа №3 — Задание 3.1: Параллельные алгоритмы сортировки (OpenMP)\n";
    std::cout << "=========================================================================================\n";
    std::cout << "Стандарт OpenMP:      " << _OPENMP << "\n";
    std::cout << "Доступно потоков CPU: " << omp_get_max_threads() << "\n";
    std::cout << "Точность таймера:     omp_get_wtick() = " << std::fixed << std::setprecision(6)
              << (omp_get_wtick() * 1000.0) << " мс\n";
    std::cout << "Метод фильтрации:     AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n";

    std::vector<int> sizes = demoMode ? std::vector<int>{ 20000 } : std::vector<int>{ 20000, 35000, 50000 };
    std::vector<int> parThreadsQuad = demoMode ? std::vector<int>{ 2, 3, 4 } : std::vector<int>{ 2, 3, 4, 6, 8, 12, 16 };
    std::vector<int> parThreadsFast = demoMode ? std::vector<int>{ 2, 3, 4 } : std::vector<int>{ 2, 3, 4, 6, 8, 12, 16, 32 };

    int runsBubble = demoMode ? 5 : 30;
    int runsOddEven = demoMode ? 5 : 35;
    int runsFast = demoMode ? 30 : 110;

    std::vector<BenchmarkRow> allRows;
    runAllForType<int>("int", sizes, parThreadsQuad, parThreadsFast, runsBubble, runsOddEven, runsFast, allRows);
    if (!demoMode) {
        runAllForType<double>("double", sizes, parThreadsQuad, parThreadsFast, runsBubble, runsOddEven, runsFast, allRows);
    }

    if (!demoMode) {
        // Сохранение сводной таблицы результатов в CSV
        std::ofstream summaryOut("results/task3_1_summary.csv");
        summaryOut << "dtype,n,algo,threads,runs,batch_size,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,trusted_avg_mean_ms,speedup,efficiency,cost_ms\n";
        summaryOut << std::fixed << std::setprecision(6);
        for (const auto& r : allRows) {
            summaryOut << r.dtype << "," << r.n << "," << r.algo << "," << r.threads << ","
                       << r.runs << "," << r.batchSize << ","
                       << r.stats.rawAvg << "," << r.stats.median << "," << r.stats.sd << ","
                       << r.stats.keptRuns << "," << r.stats.discardedRuns << ","
                       << r.stats.trustedAvgMed << "," << r.stats.trustedAvgMean << ","
                       << r.speedup << "," << r.efficiency << "," << r.costMs << "\n";
        }
        summaryOut.close();

        // Сохранение сырых замеров для построения графиков доверительного интервала
        std::ofstream rawOut("results/task3_1_raw_samples.csv");
        rawOut << "dtype,n,algo,threads,run_idx,time_ms,median_ms,sd_ms,in_interval\n";
        rawOut << std::fixed << std::setprecision(6);
        for (const auto& r : allRows) {
            bool saveRaw = (r.dtype == "int" && r.n == 50000 &&
                            ((r.algo == "Shell_Seq" && r.threads == 1) ||
                             (r.algo == "Shell_Par" && r.threads == 4) ||
                             (r.algo == "QuickSort_Seq" && r.threads == 1) ||
                             (r.algo == "QuickSort_Par" && r.threads == 4) ||
                             (r.algo == "OddEven_Seq" && r.threads == 1)));
            if (!saveRaw) continue;
            for (size_t i = 0; i < r.rawTimesMs.size(); i++) {
                double val = r.rawTimesMs[i];
                int inInt = (val >= r.stats.median - r.stats.sd && val <= r.stats.median + r.stats.sd) ? 1 : 0;
                rawOut << r.dtype << "," << r.n << "," << r.algo << "," << r.threads << ","
                       << (i + 1) << "," << val << "," << r.stats.median << "," << r.stats.sd << "," << inInt << "\n";
            }
        }
        rawOut.close();
        std::cout << "\nРезультаты сохранены в results/task3_1_summary.csv и results/task3_1_raw_samples.csv\n";
    }

    return 0;
}

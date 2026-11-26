#include "common_stats.h"

#include <Windows.h>
#include <omp.h>
#include <oneapi/tbb.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Подсчёт видимой ширины UTF-8 строки для выравнивания консольных таблиц
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
// Выделение и освобождение квадратной матрицы double[n][n] единым блоком
// ============================================================================
static double** allocMatrix(int n) {
    double** m = new double*[n];
    m[0] = new double[static_cast<size_t>(n) * static_cast<size_t>(n)];
    for (int i = 1; i < n; i++) {
        m[i] = &m[0][static_cast<size_t>(i) * static_cast<size_t>(n)];
    }
    return m;
}

static void freeMatrix(double**& m) {
    if (m != nullptr) {
        delete[] m[0];
        delete[] m;
        m = nullptr;
    }
}

// Детерминированное заполнение исходных матриц mas1 и mas2 вещественными значениями
static void fillMatrices(double** mas1, double** mas2, int n) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            mas1[i][j] = 100.0 * std::sin(0.001 * i + 0.002 * j) + static_cast<double>((i * 31 + j * 17) % 100) * 0.01;
            mas2[i][j] = 50.0 * std::cos(0.002 * i - 0.001 * j) + static_cast<double>((i * 13 + j * 29) % 100) * 0.01 + 1.5;
        }
    }
}

// ============================================================================
// 1. Сложение матриц: mas3[i][j] = mas1[i][j] + mas2[i][j]
// ============================================================================
static void addSeq(double* const* m1, double* const* m2, double** m3, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m3[i][j] = m1[i][j] + m2[i][j];
        }
    }
}

static void addOmpFor(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    #pragma omp parallel for num_threads(threads) default(none) shared(m1, m2, m3, n) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m3[i][j] = m1[i][j] + m2[i][j];
        }
    }
}

static void addOmpSections(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    auto workRange = [&](int rBegin, int rEnd) {
        for (int i = rBegin; i < rEnd; i++) {
            for (int j = 0; j < n; j++) {
                m3[i][j] = m1[i][j] + m2[i][j];
            }
        }
    };
    if (threads == 2) {
        int s1 = n / 2;
        #pragma omp parallel sections num_threads(2) default(none) shared(workRange, s1, n)
        {
            #pragma omp section
            { workRange(0, s1); }
            #pragma omp section
            { workRange(s1, n); }
        }
    } else if (threads == 3) {
        int s1 = n / 3;
        int s2 = (2 * n) / 3;
        #pragma omp parallel sections num_threads(3) default(none) shared(workRange, s1, s2, n)
        {
            #pragma omp section
            { workRange(0, s1); }
            #pragma omp section
            { workRange(s1, s2); }
            #pragma omp section
            { workRange(s2, n); }
        }
    } else {
        int s1 = n / 4;
        int s2 = n / 2;
        int s3 = (3 * n) / 4;
        #pragma omp parallel sections num_threads(4) default(none) shared(workRange, s1, s2, s3, n)
        {
            #pragma omp section
            { workRange(0, s1); }
            #pragma omp section
            { workRange(s1, s2); }
            #pragma omp section
            { workRange(s2, s3); }
            #pragma omp section
            { workRange(s3, n); }
        }
    }
}

// oneTBB Вариант 1: task_group
static void addTbbTaskGroup(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::task_group tg;
        for (int t = 0; t < threads; t++) {
            int rBegin = (t * n) / threads;
            int rEnd = ((t + 1) * n) / threads;
            tg.run([=]() {
                for (int i = rBegin; i < rEnd; i++) {
                    for (int j = 0; j < n; j++) {
                        m3[i][j] = m1[i][j] + m2[i][j];
                    }
                }
            });
        }
        tg.wait();
    });
}

// oneTBB Вариант 2: parallel_for на основе лямбда-выражения
static void addTbbParForLambda(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, n),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int i = r.begin(); i < r.end(); i++) {
                    for (int j = 0; j < n; j++) {
                        m3[i][j] = m1[i][j] + m2[i][j];
                    }
                }
            });
    });
}

// oneTBB Вариант 3: parallel_for на основе класса
class MatrixAddBody {
    double* const* m1_;
    double* const* m2_;
    double** m3_;
    int n_;
public:
    MatrixAddBody(double* const* m1, double* const* m2, double** m3, int n)
        : m1_(m1), m2_(m2), m3_(m3), n_(n) {}

    void operator()(const oneapi::tbb::blocked_range<int>& r) const {
        for (int i = r.begin(); i < r.end(); i++) {
            for (int j = 0; j < n_; j++) {
                m3_[i][j] = m1_[i][j] + m2_[i][j];
            }
        }
    }
};

static void addTbbParForClass(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, n), MatrixAddBody(m1, m2, m3, n));
    });
}

// ============================================================================
// 2. Поэлементное перемножение матриц: mas3[i][j] = mas1[i][j] * mas2[i][j]
// ============================================================================
static void mulSeq(double* const* m1, double* const* m2, double** m3, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m3[i][j] = m1[i][j] * m2[i][j];
        }
    }
}

static void mulOmpFor(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    #pragma omp parallel for num_threads(threads) default(none) shared(m1, m2, m3, n) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            m3[i][j] = m1[i][j] * m2[i][j];
        }
    }
}

static void mulOmpSections(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    auto workRange = [&](int rBegin, int rEnd) {
        for (int i = rBegin; i < rEnd; i++) {
            for (int j = 0; j < n; j++) {
                m3[i][j] = m1[i][j] * m2[i][j];
            }
        }
    };
    if (threads == 2) {
        int s1 = n / 2;
        #pragma omp parallel sections num_threads(2) default(none) shared(workRange, s1, n)
        {
            #pragma omp section
            { workRange(0, s1); }
            #pragma omp section
            { workRange(s1, n); }
        }
    } else if (threads == 3) {
        int s1 = n / 3;
        int s2 = (2 * n) / 3;
        #pragma omp parallel sections num_threads(3) default(none) shared(workRange, s1, s2, n)
        {
            #pragma omp section
            { workRange(0, s1); }
            #pragma omp section
            { workRange(s1, s2); }
            #pragma omp section
            { workRange(s2, n); }
        }
    } else {
        int s1 = n / 4;
        int s2 = n / 2;
        int s3 = (3 * n) / 4;
        #pragma omp parallel sections num_threads(4) default(none) shared(workRange, s1, s2, s3, n)
        {
            #pragma omp section
            { workRange(0, s1); }
            #pragma omp section
            { workRange(s1, s2); }
            #pragma omp section
            { workRange(s2, s3); }
            #pragma omp section
            { workRange(s3, n); }
        }
    }
}

static void mulTbbTaskGroup(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::task_group tg;
        for (int t = 0; t < threads; t++) {
            int rBegin = (t * n) / threads;
            int rEnd = ((t + 1) * n) / threads;
            tg.run([=]() {
                for (int i = rBegin; i < rEnd; i++) {
                    for (int j = 0; j < n; j++) {
                        m3[i][j] = m1[i][j] * m2[i][j];
                    }
                }
            });
        }
        tg.wait();
    });
}

static void mulTbbParForLambda(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, n),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int i = r.begin(); i < r.end(); i++) {
                    for (int j = 0; j < n; j++) {
                        m3[i][j] = m1[i][j] * m2[i][j];
                    }
                }
            });
    });
}

class MatrixMulBody {
    double* const* m1_;
    double* const* m2_;
    double** m3_;
    int n_;
public:
    MatrixMulBody(double* const* m1, double* const* m2, double** m3, int n)
        : m1_(m1), m2_(m2), m3_(m3), n_(n) {}

    void operator()(const oneapi::tbb::blocked_range<int>& r) const {
        for (int i = r.begin(); i < r.end(); i++) {
            for (int j = 0; j < n_; j++) {
                m3_[i][j] = m1_[i][j] * m2_[i][j];
            }
        }
    }
};

static void mulTbbParForClass(double* const* m1, double* const* m2, double** m3, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, n), MatrixMulBody(m1, m2, m3, n));
    });
}

// ============================================================================
// 3. Редукция суммы всех элементов двух матриц: sumEl = sum(m1[i][j] + m2[i][j])
//    Подходы oneTBB: Вариант 1 (класс) и Вариант 2 (лямбда-выражение)
// ============================================================================
static double sumSeq(double* const* m1, double* const* m2, int n) {
    double total = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            total += m1[i][j] + m2[i][j];
        }
    }
    return total;
}

static double sumOmpForRed(double* const* m1, double* const* m2, int n, int threads) {
    double total = 0.0;
    #pragma omp parallel for num_threads(threads) default(none) shared(m1, m2, n) reduction(+:total) schedule(static)
    for (int i = 0; i < n; i++) {
        double rowSum = 0.0;
        for (int j = 0; j < n; j++) {
            rowSum += m1[i][j] + m2[i][j];
        }
        total += rowSum;
    }
    return total;
}

// oneTBB редукция (Вариант 1 — на основе класса)
class SumMatrixReducer {
    double* const* m1_;
    double* const* m2_;
    int n_;
public:
    double sum;

    SumMatrixReducer(double* const* m1, double* const* m2, int n)
        : m1_(m1), m2_(m2), n_(n), sum(0.0) {}

    SumMatrixReducer(const SumMatrixReducer& other, oneapi::tbb::split)
        : m1_(other.m1_), m2_(other.m2_), n_(other.n_), sum(0.0) {}

    void operator()(const oneapi::tbb::blocked_range<int>& r) {
        double local = sum;
        for (int i = r.begin(); i < r.end(); i++) {
            for (int j = 0; j < n_; j++) {
                local += m1_[i][j] + m2_[i][j];
            }
        }
        sum = local;
    }

    void join(const SumMatrixReducer& rhs) {
        sum += rhs.sum;
    }
};

static double sumTbbRedClass(double* const* m1, double* const* m2, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    SumMatrixReducer reducer(m1, m2, n);
    arena.execute([&]() {
        oneapi::tbb::parallel_reduce(oneapi::tbb::blocked_range<int>(0, n), reducer);
    });
    return reducer.sum;
}

// oneTBB редукция (Вариант 2 — на основе лямбда-выражения)
static double sumTbbRedLambda(double* const* m1, double* const* m2, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    double result = 0.0;
    arena.execute([&]() {
        result = oneapi::tbb::parallel_reduce(
            oneapi::tbb::blocked_range<int>(0, n),
            0.0,
            [&](const oneapi::tbb::blocked_range<int>& r, double init) -> double {
                for (int i = r.begin(); i < r.end(); i++) {
                    for (int j = 0; j < n; j++) {
                        init += m1[i][j] + m2[i][j];
                    }
                }
                return init;
            },
            [](double a, double b) -> double { return a + b; }
        );
    });
    return result;
}

// ============================================================================
// 4. Редукция разницы максимума и минимума: Val_range = maxEl - minEl
//    Подходы oneTBB: Вариант 1 (класс) и Вариант 3 (структурные объекты)
// ============================================================================
static double rangeSeq(double* const* m1, double* const* m2, int n) {
    double minEl = std::numeric_limits<double>::max();
    double maxEl = std::numeric_limits<double>::lowest();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double v1 = m1[i][j];
            double v2 = m2[i][j];
            double lo = (v1 < v2) ? v1 : v2;
            double hi = (v1 > v2) ? v1 : v2;
            if (lo < minEl) minEl = lo;
            if (hi > maxEl) maxEl = hi;
        }
    }
    return maxEl - minEl;
}

static double rangeOmpForRed(double* const* m1, double* const* m2, int n, int threads) {
    double minEl = std::numeric_limits<double>::max();
    double maxEl = std::numeric_limits<double>::lowest();
    #pragma omp parallel for num_threads(threads) default(none) shared(m1, m2, n) reduction(min:minEl) reduction(max:maxEl) schedule(static)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double v1 = m1[i][j];
            double v2 = m2[i][j];
            double lo = (v1 < v2) ? v1 : v2;
            double hi = (v1 > v2) ? v1 : v2;
            if (lo < minEl) minEl = lo;
            if (hi > maxEl) maxEl = hi;
        }
    }
    return maxEl - minEl;
}

// oneTBB редукция (Вариант 1 — на основе класса)
class MinMaxMatrixReducer {
    double* const* m1_;
    double* const* m2_;
    int n_;
public:
    double minEl;
    double maxEl;

    MinMaxMatrixReducer(double* const* m1, double* const* m2, int n)
        : m1_(m1), m2_(m2), n_(n),
          minEl(std::numeric_limits<double>::max()),
          maxEl(std::numeric_limits<double>::lowest()) {}

    MinMaxMatrixReducer(const MinMaxMatrixReducer& other, oneapi::tbb::split)
        : m1_(other.m1_), m2_(other.m2_), n_(other.n_),
          minEl(std::numeric_limits<double>::max()),
          maxEl(std::numeric_limits<double>::lowest()) {}

    void operator()(const oneapi::tbb::blocked_range<int>& r) {
        double loAcc = minEl;
        double hiAcc = maxEl;
        for (int i = r.begin(); i < r.end(); i++) {
            for (int j = 0; j < n_; j++) {
                double v1 = m1_[i][j];
                double v2 = m2_[i][j];
                double lo = (v1 < v2) ? v1 : v2;
                double hi = (v1 > v2) ? v1 : v2;
                if (lo < loAcc) loAcc = lo;
                if (hi > hiAcc) hiAcc = hi;
            }
        }
        minEl = loAcc;
        maxEl = hiAcc;
    }

    void join(const MinMaxMatrixReducer& rhs) {
        if (rhs.minEl < minEl) minEl = rhs.minEl;
        if (rhs.maxEl > maxEl) maxEl = rhs.maxEl;
    }
};

static double rangeTbbRedClass(double* const* m1, double* const* m2, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    MinMaxMatrixReducer reducer(m1, m2, n);
    arena.execute([&]() {
        oneapi::tbb::parallel_reduce(oneapi::tbb::blocked_range<int>(0, n), reducer);
    });
    return reducer.maxEl - reducer.minEl;
}

// oneTBB редукция (Вариант 3 — на основе структурных объектов: состояние + функтор + объединитель)
struct MinMaxPair {
    double minVal;
    double maxVal;
};

struct MinMaxBodyStruct {
    double* const* m1;
    double* const* m2;
    int n;

    MinMaxPair operator()(const oneapi::tbb::blocked_range<int>& r, MinMaxPair init) const {
        for (int i = r.begin(); i < r.end(); i++) {
            for (int j = 0; j < n; j++) {
                double v1 = m1[i][j];
                double v2 = m2[i][j];
                double lo = (v1 < v2) ? v1 : v2;
                double hi = (v1 > v2) ? v1 : v2;
                if (lo < init.minVal) init.minVal = lo;
                if (hi > init.maxVal) init.maxVal = hi;
            }
        }
        return init;
    }
};

struct MinMaxJoinStruct {
    MinMaxPair operator()(const MinMaxPair& a, const MinMaxPair& b) const {
        return { (a.minVal < b.minVal) ? a.minVal : b.minVal,
                 (a.maxVal > b.maxVal) ? a.maxVal : b.maxVal };
    }
};

static double rangeTbbRedStruct(double* const* m1, double* const* m2, int n, int threads) {
    oneapi::tbb::task_arena arena(threads);
    MinMaxPair res{ std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() };
    MinMaxBodyStruct body{ m1, m2, n };
    MinMaxJoinStruct joiner{};
    arena.execute([&]() {
        res = oneapi::tbb::parallel_reduce(
            oneapi::tbb::blocked_range<int>(0, n),
            res,
            body,
            joiner
        );
    });
    return res.maxVal - res.minVal;
}

// ============================================================================
// Инфраструктура замеров времени
// ============================================================================
struct BenchRow {
    int dsId = 0;
    int size = 0;
    std::string opGroup;
    std::string implName;
    int threads = 1;
    int runs = 0;
    TrustedStats stats;
    double speedup = 1.0;
    double efficiency = 1.0;
    double costMs = 0.0;
    std::vector<double> rawTimesMs;
};

template <typename Fn>
static BenchRow measure(int dsId, int size, const std::string& opGroup, const std::string& implName,
                        int threads, int runs, Fn&& fn) {
    // Прогрев кэша и потоков вне замера
    fn();

    double* times = new double[runs];
    std::vector<double> raw(static_cast<size_t>(runs));
    for (int r = 0; r < runs; r++) {
        double t0 = omp_get_wtime();
        fn();
        double t1 = omp_get_wtime();
        double ms = (t1 - t0) * 1000.0;
        times[r] = ms;
        raw[static_cast<size_t>(r)] = ms;
    }

    BenchRow row;
    row.dsId = dsId;
    row.size = size;
    row.opGroup = opGroup;
    row.implName = implName;
    row.threads = threads;
    row.runs = runs;
    row.stats = computeTrustedStats(times, runs);
    row.rawTimesMs = std::move(raw);
    delete[] times;
    return row;
}

static bool verifyMatrixApprox(double* const* a, double* const* b, int n) {
    for (int i = 0; i < n; i += std::max(1, n / 200)) {
        for (int j = 0; j < n; j += std::max(1, n / 200)) {
            if (std::abs(a[i][j] - b[i][j]) > 1e-9) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "====================================================================================================\n";
    std::cout << "  Лабораторная работа №4 — Задание 4.1: Обработка матриц (OpenMP и Intel oneTBB)\n";
    std::cout << "====================================================================================================\n";
    std::cout << "Стандарт OpenMP:         " << _OPENMP << "\n";
    std::cout << "Intel oneTBB потоков:    " << oneapi::tbb::info::default_concurrency() << "\n";
    std::cout << "Точность таймера:        omp_get_wtick() = " << std::fixed << std::setprecision(6)
              << (omp_get_wtick() * 1000.0) << " мс\n";
    std::cout << "Фильтрация выбросов:     AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n";
    std::cout << "Число запусков на серию: 100\n";

    const std::vector<int> sizes = { 3000, 5500, 8000 };
    const std::vector<int> parThreads = { 2, 3, 4 };
    const int RUNS = 100;

    std::vector<BenchRow> allRows;

    for (size_t di = 0; di < sizes.size(); di++) {
        int dsId = static_cast<int>(di) + 1;
        int n = sizes[di];
        double** mas1 = allocMatrix(n);
        double** mas2 = allocMatrix(n);
        double** mas3 = allocMatrix(n);
        double** masRef = allocMatrix(n);
        fillMatrices(mas1, mas2, n);

        std::cout << "\n--- НД " << dsId << ": матрицы double " << n << "x" << n
                  << " (" << (static_cast<long long>(n) * n) << " эл.) ---\n";
        std::cout << padRightU8("Функция (реализация)", 34)
                  << padLeftU8("Пот.", 5)
                  << padLeftU8("Зам.", 6)
                  << padLeftU8("Исх.ср.(мс)", 13)
                  << padLeftU8("Отбр.", 7)
                  << padLeftU8("Итог(мс)", 12)
                  << padLeftU8("Sp(n)", 8)
                  << padLeftU8("Ep(n)", 8)
                  << padLeftU8("Cp(мс)", 12)
                  << "\n";
        std::cout << std::string(105, '-') << "\n";

        auto emit = [&](BenchRow row, double t1Ms) {
            row.speedup = (row.stats.trustedAvgMed > 0.0) ? (t1Ms / row.stats.trustedAvgMed) : 1.0;
            row.efficiency = row.speedup / static_cast<double>(row.threads);
            row.costMs = static_cast<double>(row.threads) * row.stats.trustedAvgMed;
            std::cout << padRightU8(row.implName, 34)
                      << std::right << std::fixed
                      << std::setw(5) << row.threads
                      << std::setw(6) << row.runs
                      << std::setw(13) << std::setprecision(4) << row.stats.rawAvg
                      << std::setw(7) << row.stats.discardedRuns
                      << std::setw(12) << std::setprecision(4) << row.stats.trustedAvgMed
                      << std::setw(8) << std::setprecision(4) << row.speedup
                      << std::setw(8) << std::setprecision(4) << row.efficiency
                      << std::setw(12) << std::setprecision(4) << row.costMs
                      << "\n" << std::flush;
            allRows.push_back(std::move(row));
        };

        // --- 1. Сложение матриц ---
        double t1Add = 0.0;
        {
            BenchRow r = measure(dsId, n, "Add", "Add_Seq", 1, RUNS, [&]() { addSeq(mas1, mas2, masRef, n); });
            t1Add = r.stats.trustedAvgMed;
            emit(std::move(r), t1Add);
        }
        auto runAddPar = [&](const std::string& name, auto fn) {
            for (int p : parThreads) {
                BenchRow r = measure(dsId, n, "Add", name, p, RUNS, [&]() { fn(mas1, mas2, mas3, n, p); });
                if (!verifyMatrixApprox(mas3, masRef, n)) throw std::runtime_error("Verify failed: " + name);
                emit(std::move(r), t1Add);
            }
        };
        runAddPar("Add_OMP_For", addOmpFor);
        runAddPar("Add_OMP_Sections", addOmpSections);
        runAddPar("Add_TBB_TaskGroup", addTbbTaskGroup);
        runAddPar("Add_TBB_ParFor_Lambda", addTbbParForLambda);
        runAddPar("Add_TBB_ParFor_Class", addTbbParForClass);

        // --- 2. Поэлементное перемножение матриц ---
        double t1Mul = 0.0;
        {
            BenchRow r = measure(dsId, n, "Mul", "Mul_Seq", 1, RUNS, [&]() { mulSeq(mas1, mas2, masRef, n); });
            t1Mul = r.stats.trustedAvgMed;
            emit(std::move(r), t1Mul);
        }
        auto runMulPar = [&](const std::string& name, auto fn) {
            for (int p : parThreads) {
                BenchRow r = measure(dsId, n, "Mul", name, p, RUNS, [&]() { fn(mas1, mas2, mas3, n, p); });
                if (!verifyMatrixApprox(mas3, masRef, n)) throw std::runtime_error("Verify failed: " + name);
                emit(std::move(r), t1Mul);
            }
        };
        runMulPar("Mul_OMP_For", mulOmpFor);
        runMulPar("Mul_OMP_Sections", mulOmpSections);
        runMulPar("Mul_TBB_TaskGroup", mulTbbTaskGroup);
        runMulPar("Mul_TBB_ParFor_Lambda", mulTbbParForLambda);
        runMulPar("Mul_TBB_ParFor_Class", mulTbbParForClass);

        // --- 3. Сумма всех элементов двух матриц ---
        double refSum = 0.0;
        double sink = 0.0;
        double t1Sum = 0.0;
        {
            BenchRow r = measure(dsId, n, "Sum", "Sum_Seq", 1, RUNS, [&]() { refSum = sumSeq(mas1, mas2, n); });
            t1Sum = r.stats.trustedAvgMed;
            emit(std::move(r), t1Sum);
        }
        auto runSumPar = [&](const std::string& name, auto fn) {
            for (int p : parThreads) {
                BenchRow r = measure(dsId, n, "Sum", name, p, RUNS, [&]() { sink = fn(mas1, mas2, n, p); });
                if (std::abs(sink - refSum) / (std::abs(refSum) + 1.0) > 1e-8) {
                    throw std::runtime_error("Sum verify failed: " + name);
                }
                emit(std::move(r), t1Sum);
            }
        };
        runSumPar("Sum_OMP_For_Red", sumOmpForRed);
        runSumPar("Sum_TBB_Red_Class", sumTbbRedClass);
        runSumPar("Sum_TBB_Red_Lambda", sumTbbRedLambda);

        // --- 4. Разница максимума и минимума (Val_range = maxEl - minEl) ---
        double refRange = 0.0;
        double t1Range = 0.0;
        {
            BenchRow r = measure(dsId, n, "ValRange", "Range_Seq", 1, RUNS, [&]() { refRange = rangeSeq(mas1, mas2, n); });
            t1Range = r.stats.trustedAvgMed;
            emit(std::move(r), t1Range);
        }
        auto runRangePar = [&](const std::string& name, auto fn) {
            for (int p : parThreads) {
                BenchRow r = measure(dsId, n, "ValRange", name, p, RUNS, [&]() { sink = fn(mas1, mas2, n, p); });
                if (std::abs(sink - refRange) > 1e-9) {
                    throw std::runtime_error("Range verify failed: " + name);
                }
                emit(std::move(r), t1Range);
            }
        };
        runRangePar("Range_OMP_For_Red", rangeOmpForRed);
        runRangePar("Range_TBB_Red_Class", rangeTbbRedClass);
        runRangePar("Range_TBB_Red_Struct", rangeTbbRedStruct);

        std::cout << "Контрольные значения НД " << dsId << ": sumEl = " << std::setprecision(4) << refSum
                  << ", Val_range = " << refRange << "\n" << std::flush;

        freeMatrix(mas1);
        freeMatrix(mas2);
        freeMatrix(mas3);
        freeMatrix(masRef);
    }

    // Сохранение сводной таблицы результатов в CSV
    std::ofstream sumOut("Lab4/results/task4_1_summary.csv");
    sumOut << "ds_id,size,op_group,impl_name,threads,runs,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,trusted_avg_mean_ms,speedup,efficiency,cost_ms\n";
    sumOut << std::fixed << std::setprecision(6);
    for (const auto& r : allRows) {
        sumOut << r.dsId << "," << r.size << "," << r.opGroup << "," << r.implName << ","
               << r.threads << "," << r.runs << ","
               << r.stats.rawAvg << "," << r.stats.median << "," << r.stats.sd << ","
               << r.stats.keptRuns << "," << r.stats.discardedRuns << ","
               << r.stats.trustedAvgMed << "," << r.stats.trustedAvgMean << ","
               << r.speedup << "," << r.efficiency << "," << r.costMs << "\n";
    }
    sumOut.close();

    // Сохранение сырых замеров для графика доверительного интервала (НД 3: 8000x8000, p=4)
    std::ofstream rawOut("Lab4/results/task4_1_raw_samples.csv");
    rawOut << "ds_id,size,impl_name,threads,run_idx,time_ms,median_ms,sd_ms,in_interval\n";
    rawOut << std::fixed << std::setprecision(6);
    for (const auto& r : allRows) {
        if (r.dsId == 3 && r.threads == 4 && (r.implName == "Add_TBB_ParFor_Lambda" || r.implName == "Range_TBB_Red_Struct")) {
            for (size_t i = 0; i < r.rawTimesMs.size(); i++) {
                double v = r.rawTimesMs[i];
                int inI = (v >= r.stats.median - r.stats.sd && v <= r.stats.median + r.stats.sd) ? 1 : 0;
                rawOut << r.dsId << "," << r.size << "," << r.implName << "," << r.threads << ","
                       << (i + 1) << "," << v << "," << r.stats.median << "," << r.stats.sd << "," << inI << "\n";
            }
        }
    }
    rawOut.close();

    std::cout << "\nРезультаты задания 4.1 сохранены в Lab4/results/task4_1_summary.csv\n";
    return 0;
}

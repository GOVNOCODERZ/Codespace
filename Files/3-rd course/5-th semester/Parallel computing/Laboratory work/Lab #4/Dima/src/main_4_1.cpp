// main_4_1.cpp
// Задание 4.1. Операции над квадратными матрицами (сложение, поэлементное
// умножение, сумма всех элементов, Val_range) — последовательные версии,
// OpenMP (FOR / Sections / reduction) и Intel oneTBB (task_group,
// parallel_for лямбда, parallel_for класс, parallel_reduce x3 подхода).
//
// Эксперимент: 3 набора данных (матрицы NxN, N от 3000 до 8000),
// не менее 100 запусков на каждую реализацию (по заданию).
//
// Сборка (после инициализации окружения oneAPI в терминале):
//   icx /Qiopenmp /EHsc /std:c++17 main_4_1.cpp /Fe:matrix_app.exe ^
//       /I"%TBBROOT%\include" /link "%TBBROOT%\lib\intel64\tbb12.lib"
#include <omp.h>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "common.h"
#include <tbb/info.h>
#include "matrix_ops_omp.h"
#include "matrix_ops_tbb.h"

static const int RUNS = 100;          // >= 100 по заданию
static const size_t SIZES[3] = { 3000, 5500, 8000 };
static const int THREADS[3] = { 2, 3, 4 };

// Типы операций "add"/"mul": замеряем через указатель на функцию с сигнатурой
// double(const double*, const double*, double*, size_t[, int])
template <typename Fn>
static double MeasureAvg(Fn&& fn, int runs) {
    std::vector<double> times(runs);
    for (int r = 0; r < runs; r++) times[r] = fn();
    return AvgTrustedIntervalMed(times.data(), runs);
}

static void RunAddMul(const char* opName,
                       double(*seqFn)(const double*, const double*, double*, size_t),
                       double(*ompForFn)(const double*, const double*, double*, size_t, int),
                       double(*ompSecFn)(const double*, const double*, double*, size_t, int),
                       double(*tbbTgFn)(const double*, const double*, double*, size_t, int),
                       double(*tbbLamFn)(const double*, const double*, double*, size_t, int),
                       double(*tbbClsFn)(const double*, const double*, double*, size_t, int),
                       const double* a, const double* b, double* c, double* ref,
                       size_t N, std::ofstream& csv) {
    std::cout << "\n--- " << opName << ", N=" << N << " ---\n";

    double T1 = MeasureAvg([&]() { return seqFn(a, b, c, N); }, RUNS);
    bool ok = ArraysEqual(c, ref, N * N);
    std::cout << opName << " (посл.): t=" << std::fixed << std::setprecision(6) << T1
               << " c, корректность=" << (ok ? "OK" : "FAIL") << "\n";
    csv << opName << ";" << N << ";посл.;-;" << T1 << ";1.0\n";

    struct Variant { const char* tag; double(*fn)(const double*, const double*, double*, size_t, int); };
    Variant variants[] = {
        { "OpenMP_FOR", ompForFn },
        { "OpenMP_Sections", ompSecFn },
        { "oneTBB_task_group", tbbTgFn },
        { "oneTBB_parfor_lambda", tbbLamFn },
        { "oneTBB_parfor_class", tbbClsFn },
    };

    for (auto& v : variants) {
        for (int th : THREADS) {
            double t = MeasureAvg([&]() { return v.fn(a, b, c, N, th); }, RUNS);
            bool okp = ArraysEqual(c, ref, N * N);
            double sp = T1 / t;
            std::cout << opName << " (" << v.tag << ", p=" << th << "): t=" << t
                       << " c, Sp=" << sp << ", корректность=" << (okp ? "OK" : "FAIL") << "\n";
            csv << opName << ";" << N << ";" << v.tag << ";" << th << ";" << t << ";" << sp << "\n";
        }
    }
}

static void RunSumEl(const double* a, const double* b, size_t N, std::ofstream& csv) {
    std::cout << "\n--- Сумма всех элементов, N=" << N << " ---\n";
    double refSum;
    std::vector<double> times(RUNS);
    for (int r = 0; r < RUNS; r++) { double s; times[r] = SumElSeq(a, b, N, s); refSum = s; }
    double T1 = AvgTrustedIntervalMed(times.data(), RUNS);
    std::cout << "Сумма (посл.): t=" << T1 << " c, sum=" << refSum << "\n";
    csv << "SumEl;" << N << ";посл.;-;" << T1 << ";1.0\n";

    struct V { const char* tag; double(*fn)(const double*, const double*, size_t, int, double&); };
    V variants[] = {
        { "OpenMP_reduction", SumElOmpReduction },
        { "oneTBB_V1_class", SumElTbbReductionV1 },
        { "oneTBB_V2_lambda", SumElTbbReductionV2 },
    };
    for (auto& v : variants) {
        for (int th : THREADS) {
            std::vector<double> ptimes(RUNS);
            double s = 0;
            for (int r = 0; r < RUNS; r++) ptimes[r] = v.fn(a, b, N, th, s);
            double t = AvgTrustedIntervalMed(ptimes.data(), RUNS);
            bool ok = ScalarsEqual(s, refSum, 1e-3 * std::fabs(refSum) + 1e-6);
            double sp = T1 / t;
            std::cout << "Сумма (" << v.tag << ", p=" << th << "): t=" << t << " c, Sp=" << sp
                       << ", корректность=" << (ok ? "OK" : "FAIL") << "\n";
            csv << "SumEl;" << N << ";" << v.tag << ";" << th << ";" << t << ";" << sp << "\n";
        }
    }
}

static void RunValRange(const double* a, const double* b, size_t N, std::ofstream& csv) {
    std::cout << "\n--- Val_range, N=" << N << " ---\n";
    double refRange;
    std::vector<double> times(RUNS);
    for (int r = 0; r < RUNS; r++) { double v; times[r] = ValRangeSeq(a, b, N, v); refRange = v; }
    double T1 = AvgTrustedIntervalMed(times.data(), RUNS);
    std::cout << "Val_range (посл.): t=" << T1 << " c, range=" << refRange << "\n";
    csv << "ValRange;" << N << ";посл.;-;" << T1 << ";1.0\n";

    struct V { const char* tag; double(*fn)(const double*, const double*, size_t, int, double&); };
    V variants[] = {
        { "OpenMP_reduction", ValRangeOmpReduction },
        { "oneTBB_V1_class", ValRangeTbbReductionV1 },
        { "oneTBB_V3_combinable", ValRangeTbbReductionV3 },
    };
    for (auto& v : variants) {
        for (int th : THREADS) {
            std::vector<double> ptimes(RUNS);
            double rv = 0;
            for (int r = 0; r < RUNS; r++) ptimes[r] = v.fn(a, b, N, th, rv);
            double t = AvgTrustedIntervalMed(ptimes.data(), RUNS);
            bool ok = ScalarsEqual(rv, refRange, 1e-3 * std::fabs(refRange) + 1e-6);
            double sp = T1 / t;
            std::cout << "Val_range (" << v.tag << ", p=" << th << "): t=" << t << " c, Sp=" << sp
                       << ", корректность=" << (ok ? "OK" : "FAIL") << "\n";
            csv << "ValRange;" << N << ";" << v.tag << ";" << th << ";" << t << ";" << sp << "\n";
        }
    }
}

int main() {
    std::cout << "Задание 4.1. Операции над матрицами (OpenMP + oneTBB)\n";
    std::cout << "omp_get_num_procs() = " << omp_get_num_procs() << "\n";
    std::cout << "oneTBB default_concurrency() = " << tbb::info::default_concurrency() << "\n";

    std::ofstream csv("../data/matrix_results.csv");
    csv << "function;N;variant;threads;time_s;Sp\n";

    for (size_t N : SIZES) {
        std::vector<double> a(N * N), b(N * N), c(N * N), ref(N * N);
        FillMatrix(a.data(), N, 0);
        FillMatrix(b.data(), N, 777);

        // Эталон для проверки корректности сложения/умножения
        for (size_t k = 0; k < N * N; k++) ref[k] = a[k] + b[k];
        RunAddMul("Add", MatAddSeq, MatAddOmpFor, MatAddOmpSections,
                  MatAddTbbTaskGroup, MatAddTbbParForLambda, MatAddTbbParForClass,
                  a.data(), b.data(), c.data(), ref.data(), N, csv);

        for (size_t k = 0; k < N * N; k++) ref[k] = a[k] * b[k];
        RunAddMul("Mul", MatMulSeq, MatMulOmpFor, MatMulOmpSections,
                  MatMulTbbTaskGroup, MatMulTbbParForLambda, MatMulTbbParForClass,
                  a.data(), b.data(), c.data(), ref.data(), N, csv);

        RunSumEl(a.data(), b.data(), N, csv);
        RunValRange(a.data(), b.data(), N, csv);
    }

    csv.close();
    std::cout << "\nГотово. Результаты сохранены в data/matrix_results.csv\n";
    return 0;
}

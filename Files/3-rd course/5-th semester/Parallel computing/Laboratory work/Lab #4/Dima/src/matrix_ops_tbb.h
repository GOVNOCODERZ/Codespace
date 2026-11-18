// matrix_ops_tbb.h
// Задание 4.1. Операции над квадратными матрицами — реализации на Intel
// oneTBB.
//
// Для сложения/умножения используются три варианта (по заданию):
//   1) tbb::task_group (аналог OpenMP sections — несколько фиксированных
//      задач, каждая обрабатывает свой блок строк);
//   2) tbb::parallel_for на основе лямбда-выражения;
//   3) tbb::parallel_for на основе функционального объекта (класса).
//
// Для редукционных операций используются три подхода в сумме:
//   A) класс (Body с конструктором разбиения split + join) — классический
//      parallel_reduce;
//   B) лямбда-выражение (4-аргументная форма parallel_reduce: identity,
//      тело, combine);
//   C) "структурный объект" без split/join — локальный аккумулятор на
//      поток через tbb::combinable<T>, используемый вместе с parallel_for.
// Для суммы всех элементов выбраны подходы A и B (п. "Вариант 1","Вариант 2").
// Для Val_range (иная комбинация, по заданию) выбраны подходы A и C
// ("Вариант 1" переиспользован, "Вариант 3" — новый).
//
// ВНИМАНИЕ: код написан по документированному API oneTBB (не было
// возможности скомпилировать в текущей песочнице — нет доступа к
// libtbb-dev/интернету). Собирать и проверять нужно в среде с
// установленным Intel oneAPI Base Toolkit (см. README).
#pragma once

#include <tbb/tbb.h>
#include <tbb/global_control.h>
#include <tbb/blocked_range.h>
#include <tbb/task_group.h>
#include <tbb/combinable.h>
#include <chrono>
#include <cstddef>
#include <algorithm>
#include <limits>
#include <vector>

// Удобный таймер (идентичен по смыслу omp_get_wtime, но без зависимости
// от OpenMP — высокоточные часы из <chrono>, как указано в гайде по
// лекциям для oneTBB/DPC++).
inline double NowSeconds() {
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

// =======================================================================
// Вариант 1. task_group — фиксированное число задач (блоков строк),
// число задач = числу потоков (nth), аналогично OpenMP sections.
// =======================================================================
inline double MatAddTbbTaskGroup(const double* a, const double* b, double* c, size_t N, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    long long n = (long long)N;

    double t0 = NowSeconds();
    tbb::task_group tg;
    for (int k = 0; k < nth; k++) {
        long long rowFrom = n * k / nth;
        long long rowTo = n * (k + 1) / nth;
        tg.run([=]() {
            for (long long i = rowFrom; i < rowTo; i++)
                for (long long j = 0; j < n; j++)
                    c[i * n + j] = a[i * n + j] + b[i * n + j];
        });
    }
    tg.wait();
    return NowSeconds() - t0;
}

inline double MatMulTbbTaskGroup(const double* a, const double* b, double* c, size_t N, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    long long n = (long long)N;

    double t0 = NowSeconds();
    tbb::task_group tg;
    for (int k = 0; k < nth; k++) {
        long long rowFrom = n * k / nth;
        long long rowTo = n * (k + 1) / nth;
        tg.run([=]() {
            for (long long i = rowFrom; i < rowTo; i++)
                for (long long j = 0; j < n; j++)
                    c[i * n + j] = a[i * n + j] * b[i * n + j];
        });
    }
    tg.wait();
    return NowSeconds() - t0;
}

// =======================================================================
// Вариант 2. parallel_for на основе лямбда-выражения.
// Диапазон — одномерный blocked_range по "плоскому" индексу матрицы
// (матрица хранится построчно, поэтому поэлементная операция корректна
// при любом разбиении диапазона).
// =======================================================================
inline double MatAddTbbParForLambda(const double* a, const double* b, double* c, size_t N, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;

    double t0 = NowSeconds();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, total),
        [&](const tbb::blocked_range<size_t>& r) {
            for (size_t k = r.begin(); k != r.end(); ++k) c[k] = a[k] + b[k];
        });
    return NowSeconds() - t0;
}

inline double MatMulTbbParForLambda(const double* a, const double* b, double* c, size_t N, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;

    double t0 = NowSeconds();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, total),
        [&](const tbb::blocked_range<size_t>& r) {
            for (size_t k = r.begin(); k != r.end(); ++k) c[k] = a[k] * b[k];
        });
    return NowSeconds() - t0;
}

// =======================================================================
// Вариант 3. parallel_for на основе функционального объекта (класса).
// =======================================================================
class MatAddFunctor {
    const double* a_; const double* b_; double* c_;
public:
    MatAddFunctor(const double* a, const double* b, double* c) : a_(a), b_(b), c_(c) {}
    void operator()(const tbb::blocked_range<size_t>& r) const {
        for (size_t k = r.begin(); k != r.end(); ++k) c_[k] = a_[k] + b_[k];
    }
};
class MatMulFunctor {
    const double* a_; const double* b_; double* c_;
public:
    MatMulFunctor(const double* a, const double* b, double* c) : a_(a), b_(b), c_(c) {}
    void operator()(const tbb::blocked_range<size_t>& r) const {
        for (size_t k = r.begin(); k != r.end(); ++k) c_[k] = a_[k] * b_[k];
    }
};

inline double MatAddTbbParForClass(const double* a, const double* b, double* c, size_t N, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;
    double t0 = NowSeconds();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, total), MatAddFunctor(a, b, c));
    return NowSeconds() - t0;
}

inline double MatMulTbbParForClass(const double* a, const double* b, double* c, size_t N, int nth) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;
    double t0 = NowSeconds();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, total), MatMulFunctor(a, b, c));
    return NowSeconds() - t0;
}

// =======================================================================
// Сумма всех элементов — Вариант 1 (класс, split+join) и Вариант 2
// (лямбда, 4-аргументная форма parallel_reduce).
// =======================================================================

// --- Вариант 1: класс ---
class SumBody {
public:
    const double* a_; const double* b_;
    double value;
    SumBody(const double* a, const double* b) : a_(a), b_(b), value(0.0) {}
    SumBody(SumBody& s, tbb::split) : a_(s.a_), b_(s.b_), value(0.0) {}
    void operator()(const tbb::blocked_range<size_t>& r) {
        double temp = value;
        for (size_t k = r.begin(); k != r.end(); ++k) temp += a_[k] + b_[k];
        value = temp;
    }
    void join(const SumBody& rhs) { value += rhs.value; }
};

inline double SumElTbbReductionV1(const double* a, const double* b, size_t N, int nth, double& outSum) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;
    double t0 = NowSeconds();
    SumBody body(a, b);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, total), body);
    outSum = body.value;
    return NowSeconds() - t0;
}

// --- Вариант 2: лямбда (4-аргументный parallel_reduce) ---
inline double SumElTbbReductionV2(const double* a, const double* b, size_t N, int nth, double& outSum) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;
    double t0 = NowSeconds();
    double sum = tbb::parallel_reduce(
        tbb::blocked_range<size_t>(0, total),
        0.0,
        [&](const tbb::blocked_range<size_t>& r, double running) -> double {
            for (size_t k = r.begin(); k != r.end(); ++k) running += a[k] + b[k];
            return running;
        },
        std::plus<double>()
    );
    outSum = sum;
    return NowSeconds() - t0;
}

// =======================================================================
// Val_range = max - min — Вариант 1 (класс, переиспользован, но теперь
// считает одновременно min и max) и Вариант 3 ("структурный объект":
// локальный аккумулятор на поток через tbb::combinable, без split/join).
// =======================================================================

struct MinMax {
    double mn, mx;
    MinMax() : mn(std::numeric_limits<double>::max()), mx(-std::numeric_limits<double>::max()) {}
};

// --- Вариант 1: класс (split + join), считает min и max одновременно ---
class MinMaxBody {
public:
    const double* a_; const double* b_;
    double mn, mx;
    MinMaxBody(const double* a, const double* b)
        : a_(a), b_(b), mn(std::numeric_limits<double>::max()), mx(-std::numeric_limits<double>::max()) {}
    MinMaxBody(MinMaxBody& s, tbb::split)
        : a_(s.a_), b_(s.b_), mn(std::numeric_limits<double>::max()), mx(-std::numeric_limits<double>::max()) {}
    void operator()(const tbb::blocked_range<size_t>& r) {
        double lmn = mn, lmx = mx;
        for (size_t k = r.begin(); k != r.end(); ++k) {
            if (a_[k] < lmn) lmn = a_[k];
            if (b_[k] < lmn) lmn = b_[k];
            if (a_[k] > lmx) lmx = a_[k];
            if (b_[k] > lmx) lmx = b_[k];
        }
        mn = lmn; mx = lmx;
    }
    void join(const MinMaxBody& rhs) {
        mn = std::min(mn, rhs.mn);
        mx = std::max(mx, rhs.mx);
    }
};

inline double ValRangeTbbReductionV1(const double* a, const double* b, size_t N, int nth, double& outRange) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;
    double t0 = NowSeconds();
    MinMaxBody body(a, b);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, total), body);
    outRange = body.mx - body.mn;
    return NowSeconds() - t0;
}

// --- Вариант 3: "структурный объект" — tbb::combinable<MinMax> +
// parallel_for. Каждый поток пишет в собственный локальный экземпляр
// MinMax (без конструктора разбиения/join из Варианта 1 — слияние всех
// локальных значений выполняется один раз в конце через combine()). ---
inline double ValRangeTbbReductionV3(const double* a, const double* b, size_t N, int nth, double& outRange) {
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nth);
    size_t total = N * N;

    double t0 = NowSeconds();
    tbb::combinable<MinMax> acc;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, total),
        [&](const tbb::blocked_range<size_t>& r) {
            MinMax& local = acc.local();
            for (size_t k = r.begin(); k != r.end(); ++k) {
                if (a[k] < local.mn) local.mn = a[k];
                if (b[k] < local.mn) local.mn = b[k];
                if (a[k] > local.mx) local.mx = a[k];
                if (b[k] > local.mx) local.mx = b[k];
            }
        });

    MinMax total_mm = acc.combine([](const MinMax& x, const MinMax& y) {
        MinMax r;
        r.mn = std::min(x.mn, y.mn);
        r.mx = std::max(x.mx, y.mx);
        return r;
    });
    outRange = total_mm.mx - total_mm.mn;
    return NowSeconds() - t0;
}

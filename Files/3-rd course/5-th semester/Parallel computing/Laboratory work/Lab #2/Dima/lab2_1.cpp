#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>
#include <chrono>
#include <algorithm>
#include <omp.h>
#include <windows.h>

// Доверительный интервал (Xavg ± SD)
double AvgTrustedInterval(const std::vector<double>& times) {
    int cnt = (int)times.size();
    if (cnt == 0) return 0.0;

    double avg = 0.0;
    for (int i = 0; i < cnt; i++) avg += times[i];
    avg /= cnt;

    double sd = 0.0;
    for (int i = 0; i < cnt; i++) {
        sd += (times[i] - avg) * (times[i] - avg);
    }
    sd = std::sqrt(sd / (cnt - 1.0));

    double newAvg = 0.0;
    int newCnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (times[i] >= (avg - sd) && times[i] <= (avg + sd)) {
            newAvg += times[i];
            newCnt++;
        }
    }
    return (newCnt > 0) ? (newAvg / newCnt) : avg;
}

// Вычислительные функции

// 1. Последовательная
double Pi_Serial(long long num_steps) {
    double step = 1.0 / (double)num_steps;
    double sum = 0.0;
    for (long long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    return step * sum;
}

// 2. Параллельный цикл (static, dynamic, guided) с динамическим расчетом чанка
// k (chunks_per_thread) - параметр дробления: сколько чанков приходится на 1 поток
double Pi_Parallel_For(long long num_steps, int threads, int schedule_type, int chunks_per_thread = 4) {
    double step = 1.0 / (double)num_steps;
    double sum = 0.0;

    // Динамический расчет размера блока (чанка) от размера списка и числа потоков
    long long chunk_size = std::max(1LL, num_steps / (threads * chunks_per_thread));

    if (schedule_type == 0) {
        // Для static делим массив ровно по числу потоков (или с заданным чанком)
        long long static_chunk = std::max(1LL, num_steps / threads);
        #pragma omp parallel for num_threads(threads) reduction(+:sum) schedule(static, static_chunk)
        for (long long i = 0; i < num_steps; i++) {
            double x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
    } else if (schedule_type == 1) {
        #pragma omp parallel for num_threads(threads) reduction(+:sum) schedule(dynamic, chunk_size)
        for (long long i = 0; i < num_steps; i++) {
            double x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
    } else if (schedule_type == 2) {
        #pragma omp parallel for num_threads(threads) reduction(+:sum) schedule(guided, chunk_size)
        for (long long i = 0; i < num_steps; i++) {
            double x = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
    }
    return step * sum;
}

// Частичный интеграл для секций
double CalcPartPi(long long start, long long end, double step) {
    double partial_sum = 0.0;
    for (long long i = start; i < end; i++) {
        double x = (i + 0.5) * step;
        partial_sum += 4.0 / (1.0 + x * x);
    }
    return partial_sum;
}

// 3. Секции с балансировкой
double Pi_Parallel_Sections(long long num_steps, int threads) {
    double step = 1.0 / (double)num_steps;
    long long s0 = 0;
    long long s1 = (1 * num_steps) / threads;
    long long s2 = (2 * num_steps) / threads;
    long long s3 = (3 * num_steps) / threads;
    long long s4 = num_steps;

    double p1 = 0, p2 = 0, p3 = 0, p4 = 0;

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        { p1 = CalcPartPi(s0, s1, step); }

        #pragma omp section
        { if (threads > 1) p2 = CalcPartPi(s1, s2, step); }

        #pragma omp section
        { if (threads > 2) p3 = CalcPartPi(s2, s3, step); }

        #pragma omp section
        { if (threads > 3) p4 = CalcPartPi(s3, s4, step); }
    }
    return step * (p1 + p2 + p3 + p4);
}

volatile double g_sink = 0.0;

template <typename Func>
double MeasureTime(Func f, int runs = 300) {
    std::vector<double> times(runs);
    g_sink += f();

    for (int i = 0; i < runs; i++) {
        auto t_start = std::chrono::high_resolution_clock::now();
        double res = f();
        auto t_end = std::chrono::high_resolution_clock::now();
        g_sink += res;

        std::chrono::duration<double, std::milli> ms = t_end - t_start;
        times[i] = ms.count();
    }
    return AvgTrustedInterval(times);
}

int Utf8Length(const std::string& str) {
    int len = 0;
    for (unsigned char c : str) {
        if ((c & 0xC0) != 0x80) len++;
    }
    return len;
}

void PrintRow(const std::string& name, const std::string& threads, const std::vector<double>& vals) {
    int pad = 32 - Utf8Length(name);
    if (pad < 1) pad = 1;
    std::cout << name << std::string(pad, ' ');

    std::cout << threads;
    int pad_th = 8 - (int)threads.length();
    if (pad_th < 1) pad_th = 1;
    std::cout << std::string(pad_th, ' ');

    for (double val : vals) {
        std::cout << std::setw(12) << std::fixed << std::setprecision(4) << val << "  ";
    }
    std::cout << "\n";
}

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    std::vector<long long> ND = { 200000, 500000, 1000000, 2000000 };
    int thread_counts[] = { 2, 3, 4 };
    const int RUNS = 300;
    const int CHUNKS_PER_THREAD = 4; // Коэффициент деления для dynamic и guided

    std::cout << "\nТаблица 2.1 — Время вычисления числа Pi (мс) [300 запусков]\n";
    std::cout << "Параметр дробления чанка: num_steps / (threads * " << CHUNKS_PER_THREAD << ")\n\n";

    std::cout << "Функция (реализация)            Потоки      НД 1          НД 2          НД 3          НД 4\n";
    std::cout << std::string(90, '-') << "\n";

    std::vector<double> seq_vals;
    for (auto steps : ND) {
        seq_vals.push_back(MeasureTime([&]() { return Pi_Serial(steps); }, RUNS));
    }
    PrintRow("Последовательная реализация", "1", seq_vals);
    std::cout << std::string(90, '-') << "\n";

    std::pair<std::string, int> scheds[] = {
        {"Параллельная FOR (static)", 0},
        {"Параллельная FOR (dynamic)", 1},
        {"Параллельная FOR (guided)", 2},
        {"Параллельная Section", 3}
    };

    for (const auto& sch : scheds) {
        for (int p : thread_counts) {
            std::vector<double> par_vals;
            for (auto steps : ND) {
                if (sch.second < 3) {
                    par_vals.push_back(MeasureTime([&]() { 
                        return Pi_Parallel_For(steps, p, sch.second, CHUNKS_PER_THREAD); 
                    }, RUNS));
                } else {
                    par_vals.push_back(MeasureTime([&]() { 
                        return Pi_Parallel_Sections(steps, p); 
                    }, RUNS));
                }
            }
            PrintRow(sch.first, std::to_string(p), par_vals);
        }
        std::cout << std::string(90, '-') << "\n";
    }

    return 0;
}
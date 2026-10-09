#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <omp.h>
#include <string>

// Функция расчёта среднего в доверительном интервала
// на основе среднеарифметического значения
double AvgTrustedIntervalAVG(const std::vector<double>& times)
{
    int cnt = static_cast<int>(times.size());
    if (cnt == 0) return 0.0;

    // среднеарифметическое
    double avg = 0.0;
    for (int i = 0; i < cnt; ++i)
        avg += times[i];
    avg /= cnt;

    // стандартное отклонение
    double sd = 0.0;
    for (int i = 0; i < cnt; ++i)
        sd += (times[i] - avg) * (times[i] - avg);
    sd = std::sqrt(sd / (cnt - 1));

    // отфильтрованные значения
    double newAvg = 0.0;
    int newCnt = 0;
    for (int i = 0; i < cnt; ++i)
    {
        if (times[i] >= avg - sd && times[i] <= avg + sd)
        {
            newAvg += times[i];
            ++newCnt;
        }
    }
    if (newCnt > 0)
        newAvg /= newCnt;
    else
        newAvg = avg;

    return newAvg;
}

// Последовательная реализация
double pi_sequential(long num_steps)
{
    double step = 1.0 / static_cast<double>(num_steps);
    double sum = 0.0;
    for (long i = 0; i < num_steps; ++i)
    {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    return step * sum;
}

// Parallel FOR schedule(static)
double pi_for_static(long num_steps)
{
    double step = 1.0 / static_cast<double>(num_steps);
    double sum = 0.0;

    #pragma omp parallel for schedule(static) reduction(+:sum)
    for (long i = 0; i < num_steps; ++i)
    {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    return step * sum;
}

// Parallel FOR schedule(dynamic)
double pi_for_dynamic(long num_steps)
{
    double step = 1.0 / static_cast<double>(num_steps);
    double sum = 0.0;

    #pragma omp parallel for schedule(dynamic, 1000) reduction(+:sum)
    for (long i = 0; i < num_steps; ++i)
    {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    return step * sum;
}

// Parallel FOR schedule(guided)
double pi_for_guided(long num_steps)
{
    double step = 1.0 / static_cast<double>(num_steps);
    double sum = 0.0;

    #pragma omp parallel for schedule(guided, 1000) reduction(+:sum)
    for (long i = 0; i < num_steps; ++i)
    {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    return step * sum;
}

// Parallel Sections
// Делим диапазон на 4 секции (работает при любом числе потоков)
double pi_sections(long num_steps)
{
    double step = 1.0 / static_cast<double>(num_steps);
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;

    long chunk = num_steps / 4;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (long i = 0; i < chunk; ++i)
            {
                double x = (i + 0.5) * step;
                sum1 += 4.0 / (1.0 + x * x);
            }
        }
        #pragma omp section
        {
            for (long i = chunk; i < 2 * chunk; ++i)
            {
                double x = (i + 0.5) * step;
                sum2 += 4.0 / (1.0 + x * x);
            }
        }
        #pragma omp section
        {
            for (long i = 2 * chunk; i < 3 * chunk; ++i)
            {
                double x = (i + 0.5) * step;
                sum3 += 4.0 / (1.0 + x * x);
            }
        }
        #pragma omp section
        {
            for (long i = 3 * chunk; i < num_steps; ++i)
            {
                double x = (i + 0.5) * step;
                sum4 += 4.0 / (1.0 + x * x);
            }
        }
    }
    return step * (sum1 + sum2 + sum3 + sum4);
}

// Измерение времени 
using FuncType = double(*)(long);

double measure_time(FuncType func, long num_steps, int runs = 300)
{
    std::vector<double> times(runs);

    // Прогрев
    func(num_steps / 10);

    for (int r = 0; r < runs; ++r)
    {
        double t0 = omp_get_wtime();
        volatile double pi = func(num_steps); // volatile чтобы не оптимизировали
        double t1 = omp_get_wtime();
        times[r] = (t1 - t0) * 1000.0; // в миллисекундах
        (void)pi;
    }
    return AvgTrustedIntervalAVG(times);
}

int main()
{
    // Наборы данных (num_steps)
    const long NDs[4] = {10000000L, 20000000L, 50000000L, 100000000L};
    const int threads_list[3] = {2, 3, 4};
    const int RUNS = 300;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Laboratory Work 2. Task 2.1 — Computing Pi ===\n";
    std::cout << "Confidence interval, runs: " << RUNS << "\n";
    std::cout << "Available threads (omp_get_max_threads): " << omp_get_max_threads() << "\n\n";

    // Проверка правильности
    {
        double pi_ref = pi_sequential(1000000);
        std::cout << "Check: Pi (1e6 steps) = " << pi_ref << " (expected ~3.14159)\n\n";
    }

    // Последовательная версия
    std::cout << "=== Sequential implementation ===\n";
    std::cout << "DS1\t\tDS2\t\tDS3\t\tDS4\n";
    for (int nd = 0; nd < 4; ++nd)
    {
        double t = measure_time(pi_sequential, NDs[nd], RUNS);
        std::cout << t << " ms";
        if (nd < 3) std::cout << "\t";
    }
    std::cout << "\n\n";

    // Parallel FOR static
    std::cout << "=== Parallel FOR schedule(static) ===\n";
    for (int th : threads_list)
    {
        omp_set_num_threads(th);
        std::cout << "Threads: " << th << "\n";
        std::cout << "DS1\t\tDS2\t\tDS3\t\tDS4\n";
        for (int nd = 0; nd < 4; ++nd)
        {
            double t = measure_time(pi_for_static, NDs[nd], RUNS);
            std::cout << t << " ms";
            if (nd < 3) std::cout << "\t";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // Parallel FOR dynamic
    std::cout << "=== Parallel FOR schedule(dynamic) ===\n";
    for (int th : threads_list)
    {
        omp_set_num_threads(th);
        std::cout << "Threads: " << th << "\n";
        std::cout << "DS1\t\tDS2\t\tDS3\t\tDS4\n";
        for (int nd = 0; nd < 4; ++nd)
        {
            double t = measure_time(pi_for_dynamic, NDs[nd], RUNS);
            std::cout << t << " ms";
            if (nd < 3) std::cout << "\t";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // Parallel FOR guided
    std::cout << "=== Parallel FOR schedule(guided) ===\n";
    for (int th : threads_list)
    {
        omp_set_num_threads(th);
        std::cout << "Threads: " << th << "\n";
        std::cout << "DS1\t\tDS2\t\tDS3\t\tDS4\n";
        for (int nd = 0; nd < 4; ++nd)
        {
            double t = measure_time(pi_for_guided, NDs[nd], RUNS);
            std::cout << t << " ms";
            if (nd < 3) std::cout << "\t";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // Parallel Sections
    std::cout << "=== Parallel Sections ===\n";
    for (int th : threads_list)
    {
        omp_set_num_threads(th);
        std::cout << "Threads: " << th << "\n";
        std::cout << "DS1\t\tDS2\t\tDS3\t\tDS4\n";
        for (int nd = 0; nd < 4; ++nd)
        {
            double t = measure_time(pi_sections, NDs[nd], RUNS);
            std::cout << t << " ms";
            if (nd < 3) std::cout << "\t";
        }
        std::cout << "\n";
    }

    std::cout << "\nDone. Copy the results into Table 2.1.\n";
    return 0;
}
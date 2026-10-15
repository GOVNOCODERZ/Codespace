#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <functional>
#include <cmath>
#include <omp.h>

using namespace std;

// Количество итераций
#define ITERS 300

// MARK: Доверительный интервал

/// @brief Сортировка пузырьком
void bubbleSort(double* mas, int cnt) {
    bool ex;
    do {
        ex = false;
        for (int i = 0; i < cnt - 1; i++)
            if (mas[i] > mas[i + 1]) {
                swap(mas[i], mas[i + 1]);
                ex = true;
            }
    } while (ex);
}

/// @brief Среднее время в доверительном интервале на основе медианы
/// @param times массив замеров
/// @param cnt размер массива
double AvgTrustedIntervalMED(double* times, int cnt) {
    if (cnt < 2)
        return times[0];
    bubbleSort(times, cnt);
    double avg = 0;
    for (int i = 0; i < cnt; i++)
        avg += times[i];
    avg /= cnt;

    double med = times[cnt / 2], sd = 0, res = 0;
    for (int i = 0; i < cnt; i++)
        sd += (times[i] - avg) * (times[i] - avg);
    sd = sqrt(sd / (cnt - 1.0));

    int n = 0;
    for (int i = 0; i < cnt; i++)
        if (fabs(times[i] - med) <= sd)
            { res += times[i]; n++; }
    return n ? res / n : avg;
}

/// @brief Замер времени выполнения функции
/// @param f функция для прогона
/// @param cnt количество запусков
/// @return среднее время (мс) в доверительном интервале
double MeasureFTime(function<double()> f, int cnt) {
    double* t = new double[cnt];
    for (int i = 0; i < cnt; i++) {
        t[i] = f();
        cout << "+";
    }
    cout << endl;
    double r = AvgTrustedIntervalMED(t, cnt);
    delete[] t;
    return r;
}

// MARK: Рабочие функции
// Все функции возвращают время выполнения в мс, а найденное значение Pi кладут в pi.

// Размеры порций для динамического и управляемого планировщиков
const int CHUNK_DYNAMIC = 10000;
const int CHUNK_GUIDED = 1000;

// Последовательная реализация
double Pi_posl(long num_steps, double& pi) {
    double t0 = omp_get_wtime();
    double step = 1.0 / (double)num_steps, sum = 0.0;
    for (long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    pi = step * sum;
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельная реализация: for + schedule(static) + reduction
double Pi_par_static(long num_steps, double& pi) {
    double t0 = omp_get_wtime();
    double step = 1.0 / (double)num_steps, sum = 0.0;
#pragma omp parallel for schedule(static) reduction(+:sum)
    for (long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    pi = step * sum;
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельная реализация: for + schedule(dynamic) + reduction
double Pi_par_dynamic(long num_steps, double& pi) {
    double t0 = omp_get_wtime();
    double step = 1.0 / (double)num_steps, sum = 0.0;
#pragma omp parallel for schedule(dynamic, CHUNK_DYNAMIC) reduction(+:sum)
    for (long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    pi = step * sum;
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельная реализация: for + schedule(guided) + reduction
double Pi_par_guided(long num_steps, double& pi) {
    double t0 = omp_get_wtime();
    double step = 1.0 / (double)num_steps, sum = 0.0;
#pragma omp parallel for schedule(guided, CHUNK_GUIDED) reduction(+:sum)
    for (long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    pi = step * sum;
    return (omp_get_wtime() - t0) * 1000;
}

// Частичная сумма на полуинтервале [from, to)
static double PartSum(long from, long to, double step) {
    double s = 0.0;
    for (long i = from; i < to; i++) {
        double x = (i + 0.5) * step;
        s += 4.0 / (1.0 + x * x);
    }
    return s;
}

// Параллельная реализация: sections с ручной балансировкой под p = 2..4 потока
double Pi_par_sections(long num_steps, double& pi) {
    double t0 = omp_get_wtime();
    double step = 1.0 / (double)num_steps;
    int p = omp_get_max_threads();
    long s1 = num_steps / p, s2 = 2 * num_steps / p, s3 = 3 * num_steps / p;
    double part[4] = { 0.0, 0.0, 0.0, 0.0 };
#pragma omp parallel sections
    {
#pragma omp section
        { part[0] = PartSum(0, s1, step); }
#pragma omp section
        { if (p > 1) part[1] = PartSum(s1, s2, step); }
#pragma omp section
        { if (p > 2) part[2] = PartSum(s2, s3, step); }
#pragma omp section
        { if (p > 3) part[3] = PartSum(s3, num_steps, step); }
    }
    pi = step * (part[0] + part[1] + part[2] + part[3]);
    return (omp_get_wtime() - t0) * 1000;
}

// MARK: Тестирование

/// @brief Прогон всех реализаций для одного набора данных
/// @param names названия реализаций
/// @param num_steps количество итераций (набор данных)
/// @param iters количество запусков каждой реализации
/// @param csv поток для записи результатов в CSV
void test_functions(vector<string>& names, long num_steps, int iters, ofstream& csv) {
    double pi = 0.0;

    vector<function<double()>> F = {
        [&] { return Pi_posl(num_steps, pi); },
        [&] { return Pi_par_static(num_steps, pi); },
        [&] { return Pi_par_dynamic(num_steps, pi); },
        [&] { return Pi_par_guided(num_steps, pi); },
        [&] { return Pi_par_sections(num_steps, pi); }
    };

    for (int th = 1; th <= 4; th++) {
        omp_set_num_threads(th);
        cout << "\nThreads: " << omp_get_max_threads() << endl;
        for (int f = 0; f < (int)F.size(); f++) {
            bool sequential = (f == 0);
            if ((th == 1) != sequential)
                continue; // последовательная - только при th == 1, параллельные - при th > 1

            cout << names[f] << "\t";
            double r = MeasureFTime(F[f], iters);
            cout << "  avg time: " << r << " ms, Pi = " << setprecision(10) << pi << setprecision(6) << endl;

            csv << num_steps << ',' << '"' << names[f] << '"' << ',' << th << ',' << r << '\n';
        }
    }
}

int main() {
    vector<string> names = { "Pi (seq)", "Pi (for, static)", "Pi (for, dynamic)", "Pi (for, guided)", "Pi (sections)" };

    // Наборы данных: количество итераций `num_steps`
    long long_steps[4] = { 10000000, 20000000, 30000000, 40000000 };

    ofstream csv("results_2_1.csv");
    if (!csv.is_open()) {
        cerr << "Failed to open results_2_1.csv for writing" << endl;
        return 1;
    }
    csv << "NumSteps,Function,Threads,Time_ms" << '\n';

    for (int nd = 0; nd < 4; nd++) {
        cout << "\n=== num_steps: " << long_steps[nd] << " ===" << endl;
        test_functions(names, long_steps[nd], ITERS, csv);
    }

    csv.close();
    cout << "\nResults written to results_2_1.csv" << endl;
    return 0;
}
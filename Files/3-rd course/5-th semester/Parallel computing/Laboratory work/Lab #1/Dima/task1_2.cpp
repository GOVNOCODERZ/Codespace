#include <iostream>
#include <cmath>
#include <vector>
#include <functional>
#include <algorithm>
#include <fstream>
#include <omp.h>

// ---------------- Заполнение массивов ----------------

void fill_seq(double* mas1, double* mas2, int size) {
    for (int i = 0; i < size; i++) {
        mas1[i] = sin(i - 50) + cos(i / 2.0);
        mas2[i] = pow(i, 2.0 / 3.0) * sin(i / 2.0);
    }
}

void fill_par_for(double* mas1, double* mas2, int size) {
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        mas1[i] = sin(i - 50) + cos(i / 2.0);
        mas2[i] = pow(i, 2.0 / 3.0) * sin(i / 2.0);
    }
}

// ---------------- Сложение массивов ----------------

void sum_arrays_seq(double* mas1, double* mas2, double* mas3, int size) {
    for (int i = 0; i < size; i++) mas3[i] = mas1[i] + mas2[i];
}

void sum_arrays_par_for(double* mas1, double* mas2, double* mas3, int size) {
    #pragma omp parallel for
    for (int i = 0; i < size; i++) mas3[i] = mas1[i] + mas2[i];
}

void sum_arrays_par_sections(double* mas1, double* mas2, double* mas3, int size) {
    int half = size / 2;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 0; i < half; i++) mas3[i] = mas1[i] + mas2[i];
        }
        #pragma omp section
        {
            for (int i = half; i < size; i++) mas3[i] = mas1[i] + mas2[i];
        }
    }
}

// ---------------- Подсчёт суммы элементов ----------------

double sum_el_seq(double* mas3, int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) sum += mas3[i];
    return sum;
}

double sum_el_reduction(double* mas3, int size) {
    double sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < size; i++) sum += mas3[i];
    return sum;
}

double sum_el_critical(double* mas3, int size) {
    double sum = 0;
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        #pragma omp critical
        sum += mas3[i];
    }
    return sum;
}

// volatile-приёмник результата — не даёт компилятору выбросить "бесполезные" вычисления
volatile double g_sink = 0;

// ---------------- Замер времени ----------------

// Усреднение с отбрасыванием 10% наибольших и наименьших значений (доверительный интервал)
double AvgTrustedInterval(std::vector<double>& times) {
    std::sort(times.begin(), times.end());
    int cut = (int)(times.size() * 0.1);
    double sum = 0;
    int cnt = 0;
    for (int i = cut; i < (int)times.size() - cut; i++) {
        sum += times[i];
        cnt++;
    }
    return cnt > 0 ? sum / cnt : 0;
}

// innerReps — сколько раз подряд вызвать func() внутри одного замера (нужно для
// коротких операций, время которых меньше разрешения таймера); итоговое время
// делится на innerReps, чтобы получить время одного вызова
double MeasureTime(const std::function<void()>& func, int expCnt, int innerReps = 1) {
    std::vector<double> times(expCnt);
    for (int i = 0; i < expCnt; i++) {
        double t0 = omp_get_wtime();
        for (int r = 0; r < innerReps; r++) func();
        times[i] = (omp_get_wtime() - t0) * 1000.0 / innerReps; // мс
    }
    return AvgTrustedInterval(times);
}

// ---------------- Экспериментальное исследование ----------------

const std::vector<std::string> function_names = {
    "Заполнение (посл.)", "Заполнение (парал. FOR)",
    "Сложение (посл.)", "Сложение (парал. FOR)", "Сложение (парал. Sections)",
    "Сумма эл-тов (посл.)", "Сумма эл-тов (редуктор)", "Сумма эл-тов (крит. секция)"
};

// индексы последовательных функций
bool is_seq(int f) { return f == 0 || f == 2 || f == 5; }

void run_research(int size, int iterations, std::ofstream& out) {
    double* mas1 = new double[size];
    double* mas2 = new double[size];
    double* mas3 = new double[size];
    double tsum = 0;

    for (int threads = 1; threads <= 4; threads++) {
        omp_set_num_threads(threads);
        std::cout << "\nРазмер НД: " << size << ", потоков: " << omp_get_max_threads() << std::endl;

        for (int f = 0; f < 8; f++) {
            if (threads == 1 && !is_seq(f)) continue;
            if (threads > 1 && is_seq(f)) continue;

            // для операций сложения и суммирования элементов (простые и быстрые)
            // делаем несколько вызовов подряд внутри одного замера, иначе
            // время может округлиться до 0 из-за разрешения таймера
            const int sumInnerReps = 20;
            const int elInnerReps = 20;

            double time_ms = 0;
            switch (f) {
                case 0:
                    time_ms = MeasureTime([&]() { fill_seq(mas1, mas2, size); g_sink += mas1[size - 1] + mas2[size - 1]; }, iterations);
                    break;
                case 1:
                    time_ms = MeasureTime([&]() { fill_par_for(mas1, mas2, size); g_sink += mas1[size - 1] + mas2[size - 1]; }, iterations);
                    break;
                case 2:
                    time_ms = MeasureTime([&]() { sum_arrays_seq(mas1, mas2, mas3, size); g_sink += mas3[size - 1]; }, iterations, sumInnerReps);
                    break;
                case 3:
                    time_ms = MeasureTime([&]() { sum_arrays_par_for(mas1, mas2, mas3, size); g_sink += mas3[size - 1]; }, iterations, sumInnerReps);
                    break;
                case 4:
                    time_ms = MeasureTime([&]() { sum_arrays_par_sections(mas1, mas2, mas3, size); g_sink += mas3[size - 1]; }, iterations, sumInnerReps);
                    break;
                case 5:
                    time_ms = MeasureTime([&]() { tsum = sum_el_seq(mas3, size); g_sink += tsum; }, iterations, elInnerReps);
                    break;
                case 6:
                    time_ms = MeasureTime([&]() { tsum = sum_el_reduction(mas3, size); g_sink += tsum; }, iterations, elInnerReps);
                    break;
                case 7:
                    time_ms = MeasureTime([&]() { tsum = sum_el_critical(mas3, size); g_sink += tsum; }, iterations, elInnerReps);
                    break;
            }

            std::cout << function_names[f] << " [потоков: " << threads << "]\t" << time_ms << " мс" << std::endl;
            out << size << ";" << function_names[f] << ";" << threads << ";" << time_ms << "\n";
        }
    }

    delete[] mas1;
    delete[] mas2;
    delete[] mas3;
}

int main() {
    std::cout << "Максимально доступное количество потоков: " << omp_get_max_threads() << std::endl;

    std::vector<int> data_sizes = { 100000, 170000, 240000, 310000 };
    int iterations = 300;

    std::ofstream out("results.csv");
    out << "Размер НД;Функция;Потоков;Время(мс)\n";

    for (int size : data_sizes) {
        run_research(size, iterations, out);
    }

    out.close();
    std::cout << "\nРезультаты сохранены в results.csv\n";
    // контрольный вывод накопителя — подтверждает, что все вычисления реально выполнялись
    std::cout << "Контрольная сумма (g_sink): " << g_sink << std::endl;

    return 0;
}
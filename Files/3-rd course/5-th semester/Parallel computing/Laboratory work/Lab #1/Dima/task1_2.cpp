#include <iostream>
#include <cmath>
#include <vector>
#include <functional>
#include <fstream>
#include <chrono>
#include <omp.h>
#ifdef _WIN32
#include <windows.h>
#endif

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

// Сложение с равномерной балансировкой нагрузки по секциям (см. методичку:
// "Ручная равномерная балансировка нагрузки для директивы omp sections").
// Фиксировано 4 секции, границы S_i = (i*Size)/p; реально работают только
// первые p секций - остальные проверяются условием if(p>N) и ничего не делают
void sum_arrays_par_sections(double* mas1, double* mas2, double* mas3, int size, int p) {
    int S0 = 0;
    int S1 = (1 * size) / p;
    int S2 = (2 * size) / p;
    int S3 = (3 * size) / p;
    int S4 = (4 * size) / p;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = S0; i < S1; i++) mas3[i] = mas1[i] + mas2[i];
        }
        #pragma omp section
        {
            if (p > 1) for (int i = S1; i < S2; i++) mas3[i] = mas1[i] + mas2[i];
        }
        #pragma omp section
        {
            if (p > 2) for (int i = S2; i < S3; i++) mas3[i] = mas1[i] + mas2[i];
        }
        #pragma omp section
        {
            if (p > 3) for (int i = S3; i < S4; i++) mas3[i] = mas1[i] + mas2[i];
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

// Усреднение с доверительным интервалом на основе среднеарифметического
// значения и стандартного отклонения (метод AvgTrustedIntervalAVG из
// методички "Использование доверительного интервала..."):
// 1) считаем среднее avg по всей выборке
// 2) считаем стандартное отклонение sd
// 3) оставляем только значения из диапазона [avg-sd, avg+sd]
// 4) возвращаем среднее по оставшимся значениям
double AvgTrustedInterval(std::vector<double>& times) {
    int cnt = (int)times.size();

    double avg = 0;
    for (double t : times) avg += t;
    avg /= cnt;

    double sd = 0;
    for (double t : times) sd += (t - avg) * (t - avg);
    sd /= (cnt - 1.0);
    sd = sqrt(sd);

    double newAvg = 0;
    int newCnt = 0;
    for (double t : times) {
        if (avg - sd <= t && t <= avg + sd) {
            newAvg += t;
            newCnt++;
        }
    }
    if (newCnt == 0) newCnt = 1;
    return newAvg / newCnt;
}

// innerReps — сколько раз подряд вызвать func() внутри одного замера (нужно для
// коротких операций, время которых меньше разрешения таймера); итоговое время
// делится на innerReps, чтобы получить время одного вызова.
// Используется std::chrono::high_resolution_clock вместо omp_get_wtime(),
// т.к. на некоторых сборках MinGW libgomp даёт разрешение таймера всего 1 мс
// (см. вывод omp_get_wtick() в начале main), тогда как chrono на Windows
// опирается на QueryPerformanceCounter и даёт наносекундную точность
double MeasureTime(const std::function<void()>& func, int expCnt, int innerReps = 1) {
    std::vector<double> times(expCnt);
    for (int i = 0; i < expCnt; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int r = 0; r < innerReps; r++) func();
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        times[i] = ms / innerReps;
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

    // для операций сложения и суммирования элементов (простые и быстрые)
    // делаем несколько вызовов подряд внутри одного замера, чтобы сгладить
    // статистический шум между отдельными вызовами
    const int sumInnerReps = 20;
    const int elInnerReps = 20;

    for (int threads = 1; threads <= 4; threads++) {
        omp_set_num_threads(threads);
        std::cout << "\nРазмер НД: " << size << ", потоков: " << omp_get_max_threads() << std::endl;

        for (int f = 0; f < 8; f++) {
            if (threads == 1 && !is_seq(f)) continue;
            if (threads > 1 && is_seq(f)) continue;

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
                    time_ms = MeasureTime([&]() { sum_arrays_par_sections(mas1, mas2, mas3, size, threads); g_sink += mas3[size - 1]; }, iterations, sumInnerReps);
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
#ifdef _WIN32
    // консоль Windows по умолчанию не в UTF-8 - без этого кириллица будет "кракозябрами"
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::cout << "Максимально доступное количество потоков: " << omp_get_max_threads() << std::endl;
    // разрешение таймера omp_get_wtime() на этой машине - если сравнимо
    // с измеряемым временем операции, замеры будут неточными/квантованными
    std::cout << "Разрешение таймера (omp_get_wtick): " << omp_get_wtick() * 1000.0 << " мс" << std::endl;

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
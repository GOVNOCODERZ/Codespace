#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <omp.h>

using namespace std;

// MARK: Вспомогательные функции

/// @brief Сортировка пузырьком
/// @param mas входящий массив
/// @param cnt размер массива
void bubbleSort(double* mas, int cnt) {
    bool ex;
    do {
        ex = false;
        for (int i = 0; i < cnt - 1; i++)
            if (mas[i] > mas[i + 1]) 
            {
                swap(mas[i], mas[i + 1]);
                ex = true;
            }
    }
    while (ex);
}

/// @brief Вычисление среднего времени алгоритма ч/з средний доверительный интервал
/// @param times массив замеров времени алгоритма
/// @param cnt размер массива
/// @return "очищенное" среднее время
double AvgTrustedIntervalMED(double* times, int cnt) {
    bubbleSort(times, cnt);
    double avg = 0; // среднее время по всем замерам
    for (int i = 0; i < cnt; i++)
        avg += times[i];
    avg /= cnt;

    double med = times[cnt / 2], sd = 0, res = 0;
    for (int i = 0; i < cnt; i++) // считаем стандартное отклонение от ср. значение
        sd += (times[i] - avg) * (times[i] - avg);
    sd = sqrt(sd / (cnt - 1.0));

    int n = 0; // счётчик корректных замеров
    for (int i = 0; i < cnt; i++)
        if (fabs(times[i] - med) <= sd) // фильтруем входящие в диапазон замеры
        {
            res += times[i];
            n++;
        }
    return n ? res / n : avg;
}

/// @brief Обёртка-обработчик для запуска и измерения работы функции
/// @param f переданная функция
/// @param cnt количество запусков функции
/// @return среднее время выполнения функции
double MeasureFTime(function<double()> f, int cnt) {
    double* t = new double[cnt]; // массив замеров времени
    for (int i = 0; i < cnt; i++)
    {
        t[i] = f();
        cout << "+"; // сигнализируем о прогоне функции в консоль
    }
    cout << endl;

    double r = AvgTrustedIntervalMED(t, cnt);
    delete[] t;
    return r;
}

// MARK: Рабочие функции

// Последовательное заполнение массива
double Fillarr_posl(double* a, double* b, int n) {
    double t0 = omp_get_wtime();
    for (int i = 0; i < n; i++)
    {
        a[i] = cos((i - 0.5) / 2.0);
        b[i] = sin(i * 2 / 3.0);
    }
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельное заполнение массива через `for`
double Fillarr_par_for(double* a, double* b, int n) {
    double t0 = omp_get_wtime();
#pragma omp parallel for
    for (int i = 0; i < n; i++)
    {
        a[i] = cos((i - 0.5) / 2.0);
        b[i] = sin(i * 2 / 3.0);
    }
    return (omp_get_wtime() - t0) * 1000;
}

// Последовательное сложение массивов
double SumArrays_posl(double* a, double* b, double* r, int n) {
    double t0 = omp_get_wtime();
    for (int i = 0; i < n; i++)
        r[i] = a[i] + b[i];
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельное заполнение массива через `for`
double SumArrays_par_for(double* a, double* b, double* r, int n) {
    double t0 = omp_get_wtime();
#pragma omp parallel for
    for (int i = 0; i < n; i++)
        r[i] = a[i] + b[i];
    return (omp_get_wtime() - t0) * 1000;
}

// Последовательное заполнение массива через `Sections`
double SumArrays_par_section(double* a, double* b, double* r, int n) {
    double t0 = omp_get_wtime();
    int p = omp_get_max_threads();
    int s1 = n / p, s2 = 2 * n / p, s3 = 3 * n / p;
#pragma omp parallel sections
    {
#pragma omp section
        { for (int i = 0; i < s1; i++) r[i] = a[i] + b[i]; }
#pragma omp section
        { if (p > 1) for (int i = s1; i < s2; i++) r[i] = a[i] + b[i]; }
#pragma omp section
        { if (p > 2) for (int i = s2; i < s3; i++) r[i] = a[i] + b[i]; }
#pragma omp section
        { if (p > 3) for (int i = s3; i < n; i++) r[i] = a[i] + b[i]; }
    }
    return (omp_get_wtime() - t0) * 1000;
}

// Последовательная сумма элементов массива
double ElArrSum_posl(double* a, int n, double& sum) {
    double t0 = omp_get_wtime(), s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    sum = s;
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельная сумма элементов массива через редукторы
double ElArrSum_par_reduction(double* a, int n, double& sum) {
    double t0 = omp_get_wtime(), s = 0;
#pragma omp parallel for reduction(+:s)
    for (int i = 0; i < n; i++) s += a[i];
    sum = s;
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельная сумма элементов массива через критические секции
double ElArrSum_par_critical(double* a, int n, double& sum) {
    double t0 = omp_get_wtime(), total = 0;
#pragma omp parallel
    {
        double local = 0;
#pragma omp for
        for (int i = 0; i < n; i++) local += a[i];
#pragma omp critical
        total += local;
    }
    sum = total;
    return (omp_get_wtime() - t0) * 1000;
}

// MARK: Тестирование

/// @brief Тестировщик рабочих функций
/// @param T трёхмерный массив замеров времени
/// @param names названия функций
/// @param size размер массивов для набора данных
/// @param iters кол-во повторений замеров каждой функции
void test_functions(double** T, vector<string>& names, int size, int iters) {
    double* a = new double[size], * b = new double[size], * c = new double[size]; // рабочие массивы
    double tsum = 0;

    vector<function<double()>> F = { // вектор рабочих функций
        [&] { return Fillarr_posl(a, b, size); },
        [&] { return Fillarr_par_for(a, b, size); },
        [&] { return SumArrays_posl(a, b, c, size); },
        [&] { return SumArrays_par_for(a, b, c, size); },
        [&] { return SumArrays_par_section(a, b, c, size); },
        [&] { return ElArrSum_posl(c, size, tsum); },
        [&] { return ElArrSum_par_reduction(c, size, tsum); },
        [&] { return ElArrSum_par_critical(c, size, tsum); }
    };

    for (int th = 1; th <= 4; th++) {
        omp_set_num_threads(th); // задаём глобальное число потоков
        cout << "\nThreads: " << omp_get_max_threads() << endl;
        for (int f = 0; f < (int)F.size(); f++)
        {
            bool sequential = (f == 0 || f == 2 || f == 5); // проверка на случай последовательной функции
            if ((th == 1) != sequential)
                continue; // последовательные обрабатываются только потоком th==1

            cout << names[f] << "\t";
            double r = MeasureFTime(F[f], iters);
            if (sequential)
                T[f][0] = T[f][1] = T[f][2] = r; // время выполнения посл. функций дублируется
            else
                T[f][th - 2] = r;
        }
    }
    delete[] a; delete[] b; delete[] c;
}

int main() {
    int iters = 300;
    vector<string> names = { "Fill arrays (seq)", "Fill arrays (parallel for)",
        "Sum arrays (seq)", "Sum arrays (parallel for)", "Sum arrays (sections)",
        "Sum elements (seq)", "Sum elements (reduction)", "Sum elements (critical)" };

    int ndCount = 4, fCount = 8, base = 100000, step = 70000;
    double*** T = new double** [ndCount];
    for (int nd = 0; nd < ndCount; nd++) {
        T[nd] = new double* [fCount];
        for (int f = 0; f < fCount; f++) T[nd][f] = new double[3];
    }

    for (int nd = 0; nd < ndCount; nd++) {
        int size = base + step * nd;
        cout << "\n=== Dataset size: " << size << " ===" << endl;
        test_functions(T[nd], names, size, iters);
    }

    cout << "\nResults:" << endl;
    for (int nd = 0; nd < ndCount; nd++) {
        cout << "Dataset " << (base + step * nd) << ":" << endl;
        for (int f = 0; f < fCount; f++) {
            if (f == 0 || f == 2 || f == 5) // проверка на посл. функции
                cout << "  " << names[f] << ": " << T[nd][f][0] << " ms" << endl;
            else
                for (int th = 0; th < 3; th++)
                    cout << "  " << names[f] << " [" << th + 2 << " threads]: " << T[nd][f][th] << " ms" << endl;
        }
    }
    return 0;
}
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <omp.h>

using namespace std;

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

// MARK: Создание матрицы

double** AllocMatrix(int rows, int cols) {
    double** m = new double* [rows];
    m[0] = new double[(size_t)rows * (size_t)cols];
    for (int i = 1; i < rows; i++)
        m[i] = m[0] + (size_t)i * (size_t)cols;
    return m;
}

void FreeMatrix(double** m) {
    delete[] m[0];
    delete[] m;
}

// MARK: Заполнение матриц

static double ElemA(int i, int j) { return sin(i / 50.0) + cos(j / 70.0); }
static double ElemB(int i, int j) { return cos(i / 60.0) * sin(j / 40.0 + 1.0); }
typedef double (*ElemFn)(int, int);

static void FillRows(double** m, int r0, int r1, int cols, ElemFn f) {
    for (int i = r0; i < r1; i++) {
        double* row = m[i];
        for (int j = 0; j < cols; j++)
            row[j] = f(i, j);
    }
}

// Последовательное заполнение матриц A (m x n) и B (n x k)
double Fill_posl(double** A, double** B, int m, int n, int k) {
    double t0 = omp_get_wtime();
    FillRows(A, 0, m, n, ElemA);
    FillRows(B, 0, n, k, ElemB);
    return (omp_get_wtime() - t0) * 1000;
}

// Параллельное заполнение: omp for по строкам
double Fill_par_for(double** A, double** B, int m, int n, int k) {
    double t0 = omp_get_wtime();
#pragma omp parallel
    {
#pragma omp for schedule(static) nowait
        for (int i = 0; i < m; i++) FillRows(A, i, i + 1, n, ElemA);
#pragma omp for schedule(static)
        for (int i = 0; i < n; i++) FillRows(B, i, i + 1, k, ElemB);
    }
    return (omp_get_wtime() - t0) * 1000;
}

// Размер порции строк, которую обрабатывает одна задача (task)
const int FILL_TASK_ROWS = 16;
const int MUL_TASK_ROWS = 8;

// Параллельное заполнение: omp task, каждая задача - блок строк
double Fill_par_task(double** A, double** B, int m, int n, int k) {
    double t0 = omp_get_wtime();
#pragma omp parallel
#pragma omp single
    {
        for (int r = 0; r < m; r += FILL_TASK_ROWS) {
            int r1 = min(r + FILL_TASK_ROWS, m);
#pragma omp task firstprivate(r, r1)
            FillRows(A, r, r1, n, ElemA);
        }
        for (int r = 0; r < n; r += FILL_TASK_ROWS) {
            int r1 = min(r + FILL_TASK_ROWS, n);
#pragma omp task firstprivate(r, r1)
            FillRows(B, r, r1, k, ElemB);
        }
    }
    return (omp_get_wtime() - t0) * 1000;
}

// MARK: Классическое перемножение (порядок циклов i-k-j, локальная переменная a[i][t])
// Строки результата не зависят друг от друга, поэтому распараллеливание по строкам безопасно.

/// @brief Считает строки [r0, r1) матрицы C = A (rows x n) * B (n x k); C предварительно обнуляется по строкам
static void MulRows(double** A, double** B, double** C, int r0, int r1, int n, int k) {
    for (int i = r0; i < r1; i++) {
        double* ci = C[i];
        for (int j = 0; j < k; j++) ci[j] = 0.0;
        for (int t = 0; t < n; t++) {
            double aik = A[i][t];
            const double* bt = B[t];
            for (int j = 0; j < k; j++)
                ci[j] += aik * bt[j];
        }
    }
}

double Mul_posl(double** A, double** B, double** C, int m, int n, int k) {
    double t0 = omp_get_wtime();
    MulRows(A, B, C, 0, m, n, k);
    return (omp_get_wtime() - t0) * 1000;
}

// Вариант 1: omp for по строкам результата
double Mul_par_for(double** A, double** B, double** C, int m, int n, int k) {
    double t0 = omp_get_wtime();
#pragma omp parallel for schedule(static)
    for (int i = 0; i < m; i++)
        MulRows(A, B, C, i, i + 1, n, k);
    return (omp_get_wtime() - t0) * 1000;
}

// Вариант 2: omp task, каждая задача - блок строк результата
double Mul_par_task(double** A, double** B, double** C, int m, int n, int k) {
    double t0 = omp_get_wtime();
#pragma omp parallel
#pragma omp single
    {
        for (int r = 0; r < m; r += MUL_TASK_ROWS) {
            int r1 = min(r + MUL_TASK_ROWS, m);
#pragma omp task firstprivate(r, r1)
            MulRows(A, B, C, r, r1, n, k);
        }
    }
    return (omp_get_wtime() - t0) * 1000;
}

// MARK: Алгоритм Штрассена
// Матрицы приводятся к квадрату S x S, где S подбирается так, чтобы при делении пополам
// размер оставался чётным, пока не станет <= STRASSEN_BASE. Дополнение - нулями.
// Подматрицы A11..A22 и B11..B22 - "окна" (массивы указателей на строки родителя), без копирования.

const int STRASSEN_BASE = 64;   // ниже этого размера - обычное перемножение
const int STRASSEN_PAR_DEPTH = 2; // сколько верхних уровней рекурсии порождают задачи

static double** View(double** M, int r0, int c0, int rows) {
    double** v = new double* [rows];
    for (int i = 0; i < rows; i++) v[i] = M[r0 + i] + c0;
    return v;
}

// R = X + sign * Y (квадратные n x n)
static void AddSub(double** X, double** Y, double** R, int n, int sign) {
    for (int i = 0; i < n; i++) {
        const double* x = X[i];
        const double* y = Y[i];
        double* r = R[i];
        if (sign > 0) for (int j = 0; j < n; j++) r[j] = x[j] + y[j];
        else          for (int j = 0; j < n; j++) r[j] = x[j] - y[j];
    }
}

static void StrassenRec(double** A, double** B, double** C, int n, int depth);

// Одно из семи произведений Штрассена: Mi = (X1 [+-] X2) * (Y1 [+-] Y2); если X2/Y2 == nullptr, операнд берётся как есть
static void Product(double** X1, double** X2, int sx, double** Y1, double** Y2, int sy,
                    double** Mi, int h, int depth) {
    double** TX = X1;
    double** TY = Y1;
    if (X2) { TX = AllocMatrix(h, h); AddSub(X1, X2, TX, h, sx); }
    if (Y2) { TY = AllocMatrix(h, h); AddSub(Y1, Y2, TY, h, sy); }
    StrassenRec(TX, TY, Mi, h, depth);
    if (X2) FreeMatrix(TX);
    if (Y2) FreeMatrix(TY);
}

// C = A * B для квадратных матриц размера n; depth > 0 - семь произведений выполняются задачами
static void StrassenRec(double** A, double** B, double** C, int n, int depth) {
    if (n <= STRASSEN_BASE) {
        MulRows(A, B, C, 0, n, n, n);
        return;
    }
    int h = n / 2;

    double** A11 = View(A, 0, 0, h);  double** A12 = View(A, 0, h, h);
    double** A21 = View(A, h, 0, h);  double** A22 = View(A, h, h, h);
    double** B11 = View(B, 0, 0, h);  double** B12 = View(B, 0, h, h);
    double** B21 = View(B, h, 0, h);  double** B22 = View(B, h, h, h);

    double** M1 = AllocMatrix(h, h); double** M2 = AllocMatrix(h, h);
    double** M3 = AllocMatrix(h, h); double** M4 = AllocMatrix(h, h);
    double** M5 = AllocMatrix(h, h); double** M6 = AllocMatrix(h, h);
    double** M7 = AllocMatrix(h, h);

    if (depth > 0) {
        int d = depth - 1;
#pragma omp task
        Product(A11, A22, +1, B11, B22, +1, M1, h, d);   // M1 = (A11 + A22)(B11 + B22)
#pragma omp task
        Product(A21, A22, +1, B11, nullptr, 0, M2, h, d); // M2 = (A21 + A22) B11
#pragma omp task
        Product(A11, nullptr, 0, B12, B22, -1, M3, h, d); // M3 = A11 (B12 - B22)
#pragma omp task
        Product(A22, nullptr, 0, B21, B11, -1, M4, h, d); // M4 = A22 (B21 - B11)
#pragma omp task
        Product(A11, A12, +1, B22, nullptr, 0, M5, h, d); // M5 = (A11 + A12) B22
#pragma omp task
        Product(A21, A11, -1, B11, B12, +1, M6, h, d);   // M6 = (A21 - A11)(B11 + B12)
#pragma omp task
        Product(A12, A22, -1, B21, B22, +1, M7, h, d);   // M7 = (A12 - A22)(B21 + B22)
#pragma omp taskwait
    } else {
        Product(A11, A22, +1, B11, B22, +1, M1, h, 0);
        Product(A21, A22, +1, B11, nullptr, 0, M2, h, 0);
        Product(A11, nullptr, 0, B12, B22, -1, M3, h, 0);
        Product(A22, nullptr, 0, B21, B11, -1, M4, h, 0);
        Product(A11, A12, +1, B22, nullptr, 0, M5, h, 0);
        Product(A21, A11, -1, B11, B12, +1, M6, h, 0);
        Product(A12, A22, -1, B21, B22, +1, M7, h, 0);
    }

    // Сборка результата
    for (int i = 0; i < h; i++) {
        double* c1 = C[i];
        double* c2 = C[i + h];
        for (int j = 0; j < h; j++) {
            c1[j]     = M1[i][j] + M4[i][j] - M5[i][j] + M7[i][j]; // C11
            c1[j + h] = M3[i][j] + M5[i][j];                       // C12
            c2[j]     = M2[i][j] + M4[i][j];                       // C21
            c2[j + h] = M1[i][j] - M2[i][j] + M3[i][j] + M6[i][j]; // C22
        }
    }

    FreeMatrix(M1); FreeMatrix(M2); FreeMatrix(M3); FreeMatrix(M4);
    FreeMatrix(M5); FreeMatrix(M6); FreeMatrix(M7);
    delete[] A11; delete[] A12; delete[] A21; delete[] A22;
    delete[] B11; delete[] B12; delete[] B21; delete[] B22;
}

static bool ValidSize(int size) {
    int t = size;
    while (t > STRASSEN_BASE) {
        if (t % 2 != 0) return false;
        t /= 2;
    }
    return true;
}

static int FindValidSize(int size) {
    while (!ValidSize(size)) size++;
    return size;
}

/// @brief Перемножение A (m x n) * B (n x k) методом Штрассена с дополнением до квадрата.
/// В замер входят: выделение памяти, дополнение нулями, умножение, копирование результата.
/// @param par false - последовательно; true - параллельно (omp task + omp for на подготовке данных)
double Strassen_Multiply(double** A, double** B, double** C, int m, int n, int k, bool par) {
    double t0 = omp_get_wtime();

    int S = FindValidSize(max(m, max(n, k)));
    double** TA = AllocMatrix(S, S);
    double** TB = AllocMatrix(S, S);
    double** TC = AllocMatrix(S, S);

#pragma omp parallel for if(par) schedule(static)
    for (int i = 0; i < S; i++) {
        fill(TA[i], TA[i] + S, 0.0);
        if (i < m) copy(A[i], A[i] + n, TA[i]);
        fill(TB[i], TB[i] + S, 0.0);
        if (i < n) copy(B[i], B[i] + k, TB[i]);
    }

    if (par) {
#pragma omp parallel
#pragma omp single
        StrassenRec(TA, TB, TC, S, STRASSEN_PAR_DEPTH);
    } else {
        StrassenRec(TA, TB, TC, S, 0);
    }

#pragma omp parallel for if(par) schedule(static)
    for (int i = 0; i < m; i++)
        copy(TC[i], TC[i] + k, C[i]);

    FreeMatrix(TA);
    FreeMatrix(TB);
    FreeMatrix(TC);
    return (omp_get_wtime() - t0) * 1000;
}

// MARK: Тестирование

struct DataSet { int m, n, k; }; // A: m x n, B: n x k, C: m x k

/// @brief Прогон всех реализаций для одного набора данных
void test_dataset(int index, DataSet ds, int iters, ofstream& csv) {
    int m = ds.m, n = ds.n, k = ds.k;
    string dimsA = to_string(m) + "x" + to_string(n);
    string dimsB = to_string(n) + "x" + to_string(k);
    cout << "\n=== Dataset " << index << ": A = " << dimsA << ", B = " << dimsB << " ===" << endl;

    double** A = AllocMatrix(m, n);
    double** B = AllocMatrix(n, k);
    double** C1 = AllocMatrix(m, k); // результат классических алгоритмов
    double** C2 = AllocMatrix(m, k); // результат алгоритма Штрассена

    vector<string> names = {
        "Fill matrices (seq)", "Fill matrices (omp for)", "Fill matrices (omp task)",
        "Classic multiply (seq)", "Classic multiply (omp for)", "Classic multiply (omp task)",
        "Strassen multiply (seq)", "Strassen multiply (omp task)"
    };
    vector<function<double()>> F = {
        [&] { return Fill_posl(A, B, m, n, k); },
        [&] { return Fill_par_for(A, B, m, n, k); },
        [&] { return Fill_par_task(A, B, m, n, k); },
        [&] { return Mul_posl(A, B, C1, m, n, k); },
        [&] { return Mul_par_for(A, B, C1, m, n, k); },
        [&] { return Mul_par_task(A, B, C1, m, n, k); },
        [&] { return Strassen_Multiply(A, B, C2, m, n, k, false); },
        [&] { return Strassen_Multiply(A, B, C2, m, n, k, true); }
    };

    for (int th = 1; th <= 4; th++) {
        omp_set_num_threads(th);
        cout << "\nThreads: " << omp_get_max_threads() << endl;
        for (int f = 0; f < (int)F.size(); f++) {
            bool sequential = (f == 0 || f == 3 || f == 6);
            if ((th == 1) != sequential)
                continue; // последовательные - только при th == 1, параллельные - при th > 1

            cout << names[f] << "\t" << flush;
            double r = MeasureFTime(F[f], iters);
            cout << "  avg time: " << r << " ms" << endl;

            csv << index << ',' << dimsA << ',' << dimsB << ',' << '"' << names[f] << '"'
                << ',' << th << ',' << r << '\n';
            csv.flush();
        }
    }

    // Проверка: результаты классического алгоритма и Штрассена должны совпадать
    double maxRel = 0.0;
    for (int i = 0; i < m; i++)
        for (int j = 0; j < k; j++) {
            double d = fabs(C1[i][j] - C2[i][j]) / (1.0 + fabs(C1[i][j]));
            if (d > maxRel) maxRel = d;
        }
    cout << "Validation (classic vs Strassen): max relative difference = " << maxRel << (maxRel < 1e-6 ? "  OK" : "  MISMATCH") << endl;

    FreeMatrix(A); FreeMatrix(B); FreeMatrix(C1); FreeMatrix(C2);
}

// Использование: Task2_2_Matrices [iterations] [size_divisor]
//   iterations   - число запусков каждой реализации (по умолчанию 15)
//   size_divisor - во сколько раз уменьшить размеры матриц (по умолчанию 1) - для быстрой пробы
int main(int argc, char** argv) {
    int iters = (argc > 1) ? atoi(argv[1]) : 15;
    int divisor = (argc > 2) ? atoi(argv[2]) : 1;
    if (iters < 1) iters = 1;
    if (divisor < 1) divisor = 1;

    DataSet sets[4] = {
        { 1000, 1000, 1000 },   // A = 1000x1000, B = 1000x1000
        { 1320, 1800, 1471 },   // A = 1320x1800, B = 1800x1471
        { 2420, 1971, 2500 },   // A = 2420x1971, B = 1971x2500
        { 2420, 2500, 2370 }    // A = 2420x2500, B = 2500x2370
    };

    ofstream csv("results_2_2.csv");
    if (!csv.is_open()) {
        cerr << "Failed to open results_2_2.csv for writing" << endl;
        return 1;
    }
    csv << "Dataset,A_size,B_size,Function,Threads,Time_ms" << '\n';

    cout << "Iterations per implementation: " << iters << ", size divisor: " << divisor << endl;
    for (int i = 0; i < 4; i++) {
        DataSet ds = { max(1, sets[i].m / divisor), max(1, sets[i].n / divisor), max(1, sets[i].k / divisor) };
        test_dataset(i + 1, ds, iters, csv);
    }

    csv.close();
    cout << "\nResults written to results_2_2.csv" << endl;
    return 0;
}
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <string>
#include <chrono>
#include <omp.h>
#include <windows.h>

// -----------------------------------------------------------------------------
// 1. Управление памятью: непрерывный блок (по образцу DemoMatrix2 из лекции)
// -----------------------------------------------------------------------------
double** CreateMatrix(int rows, int cols) {
    double** mat = new double*[rows];
    mat[0] = new double[rows * cols];
    for (int i = 1; i < rows; i++) {
        mat[i] = &mat[0][i * cols];
    }
    return mat;
}

void FreeMatrix(double** mat) {
    if (mat) {
        delete[] mat[0];
        delete[] mat;
    }
}

// -----------------------------------------------------------------------------
// 2. Расчет доверительного интервала (Xavg ± SD) для отсечения выбросов
// -----------------------------------------------------------------------------
double AvgTrustedInterval(const std::vector<double>& times) {
    int cnt = (int)times.size();
    if (cnt == 0) return 0.0;

    double avg = 0.0;
    for (double t : times) avg += t;
    avg /= cnt;

    double sd = 0.0;
    for (double t : times) sd += (t - avg) * (t - avg);
    sd = std::sqrt(sd / (cnt - 1.0));

    double newAvg = 0.0;
    int newCnt = 0;
    for (double t : times) {
        if (t >= (avg - sd) && t <= (avg + sd)) {
            newAvg += t;
            newCnt++;
        }
    }
    return (newCnt > 0) ? (newAvg / newCnt) : avg;
}

// Защита от удаления кода оптимизатором
volatile double g_sink = 0.0;

template <typename Func>
double RunBenchmark(Func f, int runs = 15) {
    std::vector<double> times(runs);
    // Прогрев кэша
    f();

    for (int i = 0; i < runs; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        f();
        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = t1 - t0;
        times[i] = ms.count();
    }
    return AvgTrustedInterval(times);
}

// -----------------------------------------------------------------------------
// 3. Форматированный вывод таблицы 2.2 с учетом UTF-8
// -----------------------------------------------------------------------------

// Подсчет РЕАЛЬНЫХ видимых символов на экране для UTF-8
int Utf8Length(const std::string& str) {
    int len = 0;
    for (unsigned char c : str) {
        // Пропускаем байты продолжения UTF-8 символов (10xxxxxx)
        if ((c & 0xC0) != 0x80) {
            len++;
        }
    }
    return len;
}

void PrintTableRow(const std::string& name, const std::string& threads, double time, double base_time) {
    double sp = (time > 0.0) ? (base_time / time) : 0.0;
    int p_num = std::stoi(threads);
    double ep = sp / p_num;

    // Колонка "Функция (реализация)" — строго 38 видимых символов
    int visible_len = Utf8Length(name);
    int pad_name = 38 - visible_len;
    if (pad_name < 2) pad_name = 2;
    std::cout << name << std::string(pad_name, ' ');

    // Колонка "Поток" — строго 8 символов
    std::cout << threads;
    int pad_th = 8 - (int)threads.length();
    if (pad_th < 2) pad_th = 2;
    std::cout << std::string(pad_th, ' ');

    // Вывод числовых значений
    std::cout << std::setw(14) << std::fixed << std::setprecision(2) << time
              << std::setw(14) << std::fixed << std::setprecision(2) << sp
              << std::setw(12) << std::fixed << std::setprecision(2) << ep << "\n";
}

// -----------------------------------------------------------------------------
// 4. Заполнение матриц A и B
// -----------------------------------------------------------------------------
void FillMatrix_Serial(double** A, double** B, int M, int N, int K) {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = std::sin(i) + std::cos(j / 2.3);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < K; j++)
            B[i][j] = (double)(i + 2 * j) / (double)(i + j + 1);

    g_sink += A[0][0] + B[0][0];
}

void FillMatrix_OMP_For(double** A, double** B, int M, int N, int K, int threads) {
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = std::sin(i) + std::cos(j / 2.3);

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < K; j++)
            B[i][j] = (double)(i + 2 * j) / (double)(i + j + 1);

    g_sink += A[0][0] + B[0][0];
}

void FillMatrix_OMP_Sections(double** A, double** B, int M, int N, int K, int threads) {
    int s0 = 0;
    int s1 = (1 * M) / threads;
    int s2 = (2 * M) / threads;
    int s3 = (3 * M) / threads;
    int s4 = M;

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        {
            for (int i = s0; i < s1; i++)
                for (int j = 0; j < N; j++) A[i][j] = std::sin(i) + std::cos(j / 2.3);
        }
        #pragma omp section
        {
            if (threads > 1) {
                for (int i = s1; i < s2; i++)
                    for (int j = 0; j < N; j++) A[i][j] = std::sin(i) + std::cos(j / 2.3);
            }
        }
        #pragma omp section
        {
            if (threads > 2) {
                for (int i = s2; i < s3; i++)
                    for (int j = 0; j < N; j++) A[i][j] = std::sin(i) + std::cos(j / 2.3);
            }
        }
        #pragma omp section
        {
            if (threads > 3) {
                for (int i = s3; i < s4; i++)
                    for (int j = 0; j < N; j++) A[i][j] = std::sin(i) + std::cos(j / 2.3);
            }
        }
    }

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < K; j++)
            B[i][j] = (double)(i + 2 * j) / (double)(i + j + 1);

    g_sink += A[0][0] + B[0][0];
}

// -----------------------------------------------------------------------------
// 5. Классическое перемножение матриц (Оптимальный порядок I-K-J)
// -----------------------------------------------------------------------------
void Multiply_Serial(double** A, double** B, double** C, int M, int N, int K) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) C[i][j] = 0.0;
        for (int k = 0; k < N; k++) {
            double r = A[i][k];
            for (int j = 0; j < K; j++) {
                C[i][j] += r * B[k][j];
            }
        }
    }
    g_sink += C[0][0];
}

void Multiply_Parallel_For(double** A, double** B, double** C, int M, int N, int K, int threads) {
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < K; j++) C[i][j] = 0.0;
        for (int k = 0; k < N; k++) {
            double r = A[i][k];
            #pragma omp simd
            for (int j = 0; j < K; j++) {
                C[i][j] += r * B[k][j];
            }
        }
    }
    g_sink += C[0][0];
}

void Multiply_Parallel_Sections(double** A, double** B, double** C, int M, int N, int K, int threads) {
    int s0 = 0;
    int s1 = (1 * M) / threads;
    int s2 = (2 * M) / threads;
    int s3 = (3 * M) / threads;
    int s4 = M;

    auto compute_rows = [&](int start, int end) {
        for (int i = start; i < end; i++) {
            for (int j = 0; j < K; j++) C[i][j] = 0.0;
            for (int k = 0; k < N; k++) {
                double r = A[i][k];
                for (int j = 0; j < K; j++) {
                    C[i][j] += r * B[k][j];
                }
            }
        }
    };

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        { compute_rows(s0, s1); }
        #pragma omp section
        { if (threads > 1) compute_rows(s1, s2); }
        #pragma omp section
        { if (threads > 2) compute_rows(s2, s3); }
        #pragma omp section
        { if (threads > 3) compute_rows(s3, s4); }
    }
    g_sink += C[0][0];
}

// -----------------------------------------------------------------------------
// 6. Быстрый алгоритм Штрассена
// -----------------------------------------------------------------------------
void MatAdd(double* A, double* B, double* C, int n, int stride) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i * n + j] = A[i * stride + j] + B[i * stride + j];
}

void MatSub(double* A, double* B, double* C, int n, int stride) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i * n + j] = A[i * stride + j] - B[i * stride + j];
}

void MatMulBase(double* A, double* B, double* C, int n, int stride) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) C[i * stride + j] = 0.0;
        for (int k = 0; k < n; k++) {
            double r = A[i * stride + k];
            for (int j = 0; j < n; j++) {
                C[i * stride + j] += r * B[k * stride + j];
            }
        }
    }
}

void Strassen_Recursive(double* A, double* B, double* C, int n, int stride, int threshold, bool is_parallel, int depth = 0) {
    if (n <= threshold) {
        MatMulBase(A, B, C, n, stride);
        return;
    }

    int h = n / 2;
    int h2 = h * h;
    std::vector<double> pool(7 * h2);
    double* M1 = &pool[0 * h2]; double* M2 = &pool[1 * h2];
    double* M3 = &pool[2 * h2]; double* M4 = &pool[3 * h2];
    double* M5 = &pool[4 * h2]; double* M6 = &pool[5 * h2];
    double* M7 = &pool[6 * h2];

    double* A11 = A; double* A12 = A + h;
    double* A21 = A + h * stride; double* A22 = A + h * stride + h;
    double* B11 = B; double* B12 = B + h;
    double* B21 = B + h * stride; double* B22 = B + h * stride + h;

    auto compute_M = [&](double* M, auto opA, auto opB) {
        std::vector<double> tA(h2), tB(h2);
        opA(tA.data());
        opB(tB.data());
        Strassen_Recursive(tA.data(), tB.data(), M, h, h, threshold, is_parallel, depth + 1);
    };

    #pragma omp taskgroup
    {
        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M1, [&](double* d){ MatAdd(A11, A22, d, h, stride); }, [&](double* d){ MatAdd(B11, B22, d, h, stride); });

        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M2, [&](double* d){ MatAdd(A21, A22, d, h, stride); }, [&](double* d){ for(int i=0;i<h;i++) for(int j=0;j<h;j++) d[i*h+j] = B11[i*stride+j]; });

        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M3, [&](double* d){ for(int i=0;i<h;i++) for(int j=0;j<h;j++) d[i*h+j] = A11[i*stride+j]; }, [&](double* d){ MatSub(B12, B22, d, h, stride); });

        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M4, [&](double* d){ for(int i=0;i<h;i++) for(int j=0;j<h;j++) d[i*h+j] = A22[i*stride+j]; }, [&](double* d){ MatSub(B21, B11, d, h, stride); });

        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M5, [&](double* d){ MatAdd(A11, A12, d, h, stride); }, [&](double* d){ for(int i=0;i<h;i++) for(int j=0;j<h;j++) d[i*h+j] = B22[i*stride+j]; });

        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M6, [&](double* d){ MatSub(A21, A11, d, h, stride); }, [&](double* d){ MatAdd(B11, B12, d, h, stride); });

        #pragma omp task untied if(is_parallel && depth < 2)
        compute_M(M7, [&](double* d){ MatSub(A12, A22, d, h, stride); }, [&](double* d){ MatAdd(B21, B22, d, h, stride); });
    }

    double* C11 = C; double* C12 = C + h;
    double* C21 = C + h * stride; double* C22 = C + h * stride + h;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < h; j++) {
            int idx = i * h + j;
            C11[i * stride + j] = M1[idx] + M4[idx] - M5[idx] + M7[idx];
            C12[i * stride + j] = M3[idx] + M5[idx];
            C21[i * stride + j] = M2[idx] + M4[idx];
            C22[i * stride + j] = M1[idx] + M3[idx] - M2[idx] + M6[idx];
        }
    }
}

void Strassen_Main(double** A, double** B, double** C, int M, int N, int K, bool is_parallel, int threads) {
    int maxDim = std::max({ M, N, K });
    int extSize = 1;
    while (extSize < maxDim) extSize <<= 1;

    std::vector<double> padA(extSize * extSize, 0.0);
    std::vector<double> padB(extSize * extSize, 0.0);
    std::vector<double> padC(extSize * extSize, 0.0);

    for (int i = 0; i < M; i++)
        std::copy_n(A[i], N, &padA[i * extSize]);
    for (int i = 0; i < N; i++)
        std::copy_n(B[i], K, &padB[i * extSize]);

    if (is_parallel) {
        #pragma omp parallel num_threads(threads)
        {
            #pragma omp single
            {
                Strassen_Recursive(padA.data(), padB.data(), padC.data(), extSize, extSize, 64, true);
            }
        }
    } else {
        Strassen_Recursive(padA.data(), padB.data(), padC.data(), extSize, extSize, 64, false);
    }

    for (int i = 0; i < M; i++) {
        std::copy_n(&padC[i * extSize], K, C[i]);
    }
    g_sink += C[0][0];
}

// -----------------------------------------------------------------------------
// 7. Функция запуска тестов
// -----------------------------------------------------------------------------
template <typename SeqFunc, typename ParFunc>
void BenchmarkGroup(const std::string& base_name, SeqFunc seq_fn, ParFunc par_fn, 
                    const int thread_counts[], int num_threads_cnt, int runs = 15) {
    double t_seq = RunBenchmark(seq_fn, runs);
    PrintTableRow(base_name + " (посл.)", "1", t_seq, t_seq);

    for (int i = 0; i < num_threads_cnt; ++i) {
        int p = thread_counts[i];
        double t_par = RunBenchmark([&]() { par_fn(p); }, runs);
        PrintTableRow(base_name + " (парал.)", std::to_string(p), t_par, t_seq);
    }
}

// -----------------------------------------------------------------------------
// 8. Главная функция программы
// -----------------------------------------------------------------------------
struct DataSet {
    std::string name;
    int M, N, K;
};

int main() {
    // Включение кодировки UTF-8 для корректного вывода в консоли Windows
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // 4 набора данных: 2 квадратных, 2 прямоугольных (согласно методичке)
    std::vector<DataSet> datasets = {
        { "НД1 (1000x1000, 1000x1000)", 1000, 1000, 1000 },
        { "НД2 (1320x1800, 1800x1470) [Прямоуг.]", 1320, 1800, 1470 },
        { "НД3 (1200x1200, 1200x1200)", 1200, 1200, 1200 },
        { "НД4 (1500x1200, 1200x1600) [Прямоуг.]", 1500, 1200, 1600 }
    };

    int thread_counts[] = { 2, 3, 4 };
    const int RUNS = 15;

    std::cout << "Лабораторная работа 2. Задание 2.2\n";
    std::cout << "Сводная таблица результатов исследования перемножения матриц\n";

    for (const auto& ds : datasets) {
        std::cout << "\n======================================================================================\n";
        std::cout << "  " << ds.name << "\n";
        std::cout << "======================================================================================\n";
        std::cout << "Функция (реализация)                  Поток       Время (мс)         Sp(n)       Ep(n)\n";
        std::cout << std::string(86, '-') << "\n";

        double** A = CreateMatrix(ds.M, ds.N);
        double** B = CreateMatrix(ds.N, ds.K);
        double** C = CreateMatrix(ds.M, ds.K);

        // --- 1. Заполнение матриц ---
        auto fill_seq = [&]() { FillMatrix_Serial(A, B, ds.M, ds.N, ds.K); };
        double t_fill_seq = RunBenchmark(fill_seq, RUNS);
        PrintTableRow("Заполнение матриц (посл.)", "1", t_fill_seq, t_fill_seq);

        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { FillMatrix_OMP_For(A, B, ds.M, ds.N, ds.K, p); }, RUNS);
            PrintTableRow("Заполнение (парал.) omp for", std::to_string(p), tp, t_fill_seq);
        }
        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { FillMatrix_OMP_Sections(A, B, ds.M, ds.N, ds.K, p); }, RUNS);
            PrintTableRow("Заполнение (парал.) omp sections", std::to_string(p), tp, t_fill_seq);
        }

        std::cout << std::string(86, '-') << "\n";

        // --- 2. Классическое перемножение ---
        auto mul_seq = [&]() { Multiply_Serial(A, B, C, ds.M, ds.N, ds.K); };
        double t_mul_seq = RunBenchmark(mul_seq, RUNS);
        PrintTableRow("Перемножение матриц (посл.)", "1", t_mul_seq, t_mul_seq);

        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { Multiply_Parallel_For(A, B, C, ds.M, ds.N, ds.K, p); }, RUNS);
            PrintTableRow("Перемножение Вариант 1 (omp for)", std::to_string(p), tp, t_mul_seq);
        }
        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { Multiply_Parallel_Sections(A, B, C, ds.M, ds.N, ds.K, p); }, RUNS);
            PrintTableRow("Перемножение Вариант 2 (sections)", std::to_string(p), tp, t_mul_seq);
        }

        std::cout << std::string(86, '-') << "\n";

        // --- 3. Быстрый алгоритм Штрассена ---
        // 3 прогона, так как рекурсивный алгоритм очень ресурсоемок
        auto str_seq = [&]() { Strassen_Main(A, B, C, ds.M, ds.N, ds.K, false, 1); };
        auto str_par = [&](int p) { Strassen_Main(A, B, C, ds.M, ds.N, ds.K, true, p); };
        BenchmarkGroup("Перемножение Штрассен", str_seq, str_par, thread_counts, 3, 3);

        FreeMatrix(A);
        FreeMatrix(B);
        FreeMatrix(C);
    }

    return 0;
}
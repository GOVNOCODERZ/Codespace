#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <string>
#include <chrono>
#include <omp.h>
#include <windows.h>

// 1. Динамическое управление памятью (метод DemoMatrix2)
double** CreateMatrix(int rows, int cols) {
    double** mat = new double* [rows];
    mat[0] = new double[(size_t)rows * cols];
    for (int i = 1; i < rows; i++) {
        mat[i] = &mat[0][(size_t)i * cols];
    }
    return mat;
}

void FreeMatrix(double** mat) {
    if (mat) {
        delete[] mat[0];
        delete[] mat;
    }
}

void FillMatrZero(double** matrix, int size1, int size2) {
    for (int i = 0; i < size1; i++) {
        for (int j = 0; j < size2; j++) {
            matrix[i][j] = 0.0;
        }
    }
}

// 2. Расчет доверительного интервала (Xavg ± SD)
double AvgTrustedInterval(const std::vector<double>& times) {
    int cnt = (int)times.size();
    if (cnt == 0) return 0.0;
    if (cnt <= 2) {
        double sum = 0.0;
        for (double t : times) sum += t;
        return sum / cnt;
    }

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

volatile double g_sink = 0.0;

template <typename Func>
double RunBenchmark(Func f, int runs = 15) {
    std::vector<double> times(runs);
    // Прогрев
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

// 3. Форматированный вывод таблицы 2.2
int Utf8Length(const std::string& str) {
    int len = 0;
    for (unsigned char c : str) {
        if ((c & 0xC0) != 0x80) len++;
    }
    return len;
}

void PrintTableRow(const std::string& name, const std::string& threads, double time, double base_time) {
    double sp = (time > 0.0) ? (base_time / time) : 0.0;
    int p_num = std::stoi(threads);
    double ep = sp / p_num;

    int visible_len = Utf8Length(name);
    int pad_name = 38 - visible_len;
    if (pad_name < 2) pad_name = 2;
    std::cout << name << std::string(pad_name, ' ');

    std::cout << threads;
    int pad_th = 8 - (int)threads.length();
    if (pad_th < 2) pad_th = 2;
    std::cout << std::string(pad_th, ' ');

    std::cout << std::setw(14) << std::fixed << std::setprecision(2) << time
              << std::setw(14) << std::fixed << std::setprecision(2) << sp
              << std::setw(12) << std::fixed << std::setprecision(2) << ep << "\n";
}

// 4. Заполнение матриц A и B (собственные формулы)
void FillMatrix_Serial(double** A, double** B, int sizeA, int sizeB, int sizeC) {
    for (int i = 0; i < sizeA; i++)
        for (int j = 0; j < sizeB; j++)
            A[i][j] = std::sin(i) + std::cos(j / 2.3);

    for (int i = 0; i < sizeB; i++)
        for (int j = 0; j < sizeC; j++)
            B[i][j] = (double)(i + 2 * j) / (double)(2 * i + j + 1);

    g_sink += A[0][0] + B[0][0];
}

void FillMatrix_OMP_For(double** A, double** B, int sizeA, int sizeB, int sizeC, int threads) {
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < sizeA; i++)
        for (int j = 0; j < sizeB; j++)
            A[i][j] = std::sin(i) + std::cos(j / 2.3);

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < sizeB; i++)
        for (int j = 0; j < sizeC; j++)
            B[i][j] = (double)(i + 2 * j) / (double)(2 * i + j + 1);

    g_sink += A[0][0] + B[0][0];
}

void FillMatrix_OMP_Sections(double** A, double** B, int sizeA, int sizeB, int sizeC, int threads) {
    int a0 = 0, a1 = (1 * sizeA) / threads, a2 = (2 * sizeA) / threads, a3 = (3 * sizeA) / threads, a4 = sizeA;
    int b0 = 0, b1 = (1 * sizeB) / threads, b2 = (2 * sizeB) / threads, b3 = (3 * sizeB) / threads, b4 = sizeB;

    auto fillA = [&](int start, int end) {
        for (int i = start; i < end; i++)
            for (int j = 0; j < sizeB; j++)
                A[i][j] = std::sin(i) + std::cos(j / 2.3);
    };

    auto fillB = [&](int start, int end) {
        for (int i = start; i < end; i++)
            for (int j = 0; j < sizeC; j++)
                B[i][j] = (double)(i + 2 * j) / (double)(2 * i + j + 1);
    };

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        { fillA(a0, a1); }
        #pragma omp section
        { if (threads > 1) fillA(a1, a2); }
        #pragma omp section
        { if (threads > 2) fillA(a2, a3); }
        #pragma omp section
        { if (threads > 3) fillA(a3, a4); }
    }

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        { fillB(b0, b1); }
        #pragma omp section
        { if (threads > 1) fillB(b1, b2); }
        #pragma omp section
        { if (threads > 2) fillB(b2, b3); }
        #pragma omp section
        { if (threads > 3) fillB(b3, b4); }
    }

    g_sink += A[0][0] + B[0][0];
}

// 5. Классическое перемножение матриц: Единый метод MultiplyMatr (с транспонированием)

// Последовательная
void MultiplyMatr_Serial(double** matrix1, double** matrix2, double** matrix3, int sizeA, int sizeB, int sizeC) {
    double** mtr = CreateMatrix(sizeC, sizeB);

    for (int i = 0; i < sizeB; i++)
        for (int j = 0; j < sizeC; j++)
            mtr[j][i] = matrix2[i][j];

    for (int i = 0; i < sizeA; i++) {
        for (int j = 0; j < sizeC; j++) {
            double tmp = 0.0;
            for (int k = 0; k < sizeB; k++) {
                tmp += matrix1[i][k] * mtr[j][k];
            }
            matrix3[i][j] = tmp;
        }
    }

    FreeMatrix(mtr);
    g_sink += matrix3[0][0];
}

// Параллельная Вариант 1: omp for
void MultiplyMatr_Parallel_For(double** matrix1, double** matrix2, double** matrix3, int sizeA, int sizeB, int sizeC, int threads) {
    double** mtr = CreateMatrix(sizeC, sizeB);

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < sizeB; i++)
        for (int j = 0; j < sizeC; j++)
            mtr[j][i] = matrix2[i][j];

    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < sizeA; i++) {
        for (int j = 0; j < sizeC; j++) {
            double tmp = 0.0;
            for (int k = 0; k < sizeB; k++) {
                tmp += matrix1[i][k] * mtr[j][k];
            }
            matrix3[i][j] = tmp;
        }
    }

    FreeMatrix(mtr);
    g_sink += matrix3[0][0];
}

// Параллельная: omp sections (тот же метод  с транспонированием)
void MultiplyMatr_Parallel_Sections(double** matrix1, double** matrix2, double** matrix3, int sizeA, int sizeB, int sizeC, int threads) {
    double** mtr = CreateMatrix(sizeC, sizeB);

    int t0 = 0, t1 = (1 * sizeB) / threads, t2 = (2 * sizeB) / threads, t3 = (3 * sizeB) / threads, t4 = sizeB;
    auto transpose_part = [&](int start, int end) {
        for (int i = start; i < end; i++)
            for (int j = 0; j < sizeC; j++)
                mtr[j][i] = matrix2[i][j];
    };

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        { transpose_part(t0, t1); }
        #pragma omp section
        { if (threads > 1) transpose_part(t1, t2); }
        #pragma omp section
        { if (threads > 2) transpose_part(t2, t3); }
        #pragma omp section
        { if (threads > 3) transpose_part(t3, t4); }
    }

    int s0 = 0, s1 = (1 * sizeA) / threads, s2 = (2 * sizeA) / threads, s3 = (3 * sizeA) / threads, s4 = sizeA;
    auto mul_part = [&](int start, int end) {
        for (int i = start; i < end; i++) {
            for (int j = 0; j < sizeC; j++) {
                double tmp = 0.0;
                for (int k = 0; k < sizeB; k++) {
                    tmp += matrix1[i][k] * mtr[j][k];
                }
                matrix3[i][j] = tmp;
            }
        }
    };

    #pragma omp parallel sections num_threads(threads)
    {
        #pragma omp section
        { mul_part(s0, s1); }
        #pragma omp section
        { if (threads > 1) mul_part(s1, s2); }
        #pragma omp section
        { if (threads > 2) mul_part(s2, s3); }
        #pragma omp section
        { if (threads > 3) mul_part(s3, s4); }
    }

    FreeMatrix(mtr);
    g_sink += matrix3[0][0];
}

// 6. Динамический алгоритм Штрассена
int ADD(double** MatrixA, double** MatrixB, double** MatrixResult, int MatrixSize) {
    for (int i = 0; i < MatrixSize; i++) {
        for (int j = 0; j < MatrixSize; j++) {
            MatrixResult[i][j] = MatrixA[i][j] + MatrixB[i][j];
        }
    }
    return 0;
}

int SUB(double** MatrixA, double** MatrixB, double** MatrixResult, int MatrixSize) {
    for (int i = 0; i < MatrixSize; i++) {
        for (int j = 0; j < MatrixSize; j++) {
            MatrixResult[i][j] = MatrixA[i][j] - MatrixB[i][j];
        }
    }
    return 0;
}

// Базовое перемножение MUL (метод  с транспонированием)
int MUL(double** MatrixA, double** MatrixB, double** MatrixResult, int MatrixSize) {
    double** mtr = CreateMatrix(MatrixSize, MatrixSize);

    for (int i = 0; i < MatrixSize; i++)
        for (int j = 0; j < MatrixSize; j++)
            mtr[j][i] = MatrixB[i][j];

    for (int i = 0; i < MatrixSize; i++) {
        for (int j = 0; j < MatrixSize; j++) {
            double tmp = 0.0;
            for (int k = 0; k < MatrixSize; k++) {
                tmp += MatrixA[i][k] * mtr[j][k];
            }
            MatrixResult[i][j] = tmp;
        }
    }

    FreeMatrix(mtr);
    return 0;
}

// Рекурсивная функция Strassen с динамическим выделением памяти
// и поддержкой параллельных задач (omp task)
int Strassen(double** MatrixA, double** MatrixB, double** MatrixC, int MatrixSize, int linearMultBlockSize, bool is_parallel, int depth = 0) {
    int HalfSize = MatrixSize / 2;

    if (MatrixSize <= linearMultBlockSize) {
        MUL(MatrixA, MatrixB, MatrixC, MatrixSize);
    } else {
        // Выделение динамической памяти под подматрицы
        double** A11 = CreateMatrix(HalfSize, HalfSize);
        double** A12 = CreateMatrix(HalfSize, HalfSize);
        double** A21 = CreateMatrix(HalfSize, HalfSize);
        double** A22 = CreateMatrix(HalfSize, HalfSize);

        double** B11 = CreateMatrix(HalfSize, HalfSize);
        double** B12 = CreateMatrix(HalfSize, HalfSize);
        double** B21 = CreateMatrix(HalfSize, HalfSize);
        double** B22 = CreateMatrix(HalfSize, HalfSize);

        double** C11 = CreateMatrix(HalfSize, HalfSize);
        double** C12 = CreateMatrix(HalfSize, HalfSize);
        double** C21 = CreateMatrix(HalfSize, HalfSize);
        double** C22 = CreateMatrix(HalfSize, HalfSize);

        double** M1 = CreateMatrix(HalfSize, HalfSize);
        double** M2 = CreateMatrix(HalfSize, HalfSize);
        double** M3 = CreateMatrix(HalfSize, HalfSize);
        double** M4 = CreateMatrix(HalfSize, HalfSize);
        double** M5 = CreateMatrix(HalfSize, HalfSize);
        double** M6 = CreateMatrix(HalfSize, HalfSize);
        double** M7 = CreateMatrix(HalfSize, HalfSize);

        // Разделение матрицы на четыре части
        for (int i = 0; i < HalfSize; i++) {
            for (int j = 0; j < HalfSize; j++) {
                A11[i][j] = MatrixA[i][j];
                A12[i][j] = MatrixA[i][j + HalfSize];
                A21[i][j] = MatrixA[i + HalfSize][j];
                A22[i][j] = MatrixA[i + HalfSize][j + HalfSize];

                B11[i][j] = MatrixB[i][j];
                B12[i][j] = MatrixB[i][j + HalfSize];
                B21[i][j] = MatrixB[i + HalfSize][j];
                B22[i][j] = MatrixB[i + HalfSize][j + HalfSize];
            }
        }

        if (is_parallel && depth < 2) {
            // Параллельное вычисление M1..M7 через задачи OpenMP (у каждого таска свои буферы)
            #pragma omp task shared(A11, A22, B11, B22, M1) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** AR = CreateMatrix(HalfSize, HalfSize);
                double** BR = CreateMatrix(HalfSize, HalfSize);
                ADD(A11, A22, AR, HalfSize);
                ADD(B11, B22, BR, HalfSize);
                Strassen(AR, BR, M1, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(AR); FreeMatrix(BR);
            }
            #pragma omp task shared(A21, A22, B11, M2) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** AR = CreateMatrix(HalfSize, HalfSize);
                ADD(A21, A22, AR, HalfSize);
                Strassen(AR, B11, M2, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(AR);
            }
            #pragma omp task shared(A11, B12, B22, M3) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** BR = CreateMatrix(HalfSize, HalfSize);
                SUB(B12, B22, BR, HalfSize);
                Strassen(A11, BR, M3, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(BR);
            }
            #pragma omp task shared(A22, B21, B11, M4) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** BR = CreateMatrix(HalfSize, HalfSize);
                SUB(B21, B11, BR, HalfSize);
                Strassen(A22, BR, M4, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(BR);
            }
            #pragma omp task shared(A11, A12, B22, M5) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** AR = CreateMatrix(HalfSize, HalfSize);
                ADD(A11, A12, AR, HalfSize);
                Strassen(AR, B22, M5, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(AR);
            }
            #pragma omp task shared(A21, A11, B11, B12, M6) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** AR = CreateMatrix(HalfSize, HalfSize);
                double** BR = CreateMatrix(HalfSize, HalfSize);
                SUB(A21, A11, AR, HalfSize);
                ADD(B11, B12, BR, HalfSize);
                Strassen(AR, BR, M6, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(AR); FreeMatrix(BR);
            }
            #pragma omp task shared(A12, A22, B21, B22, M7) firstprivate(HalfSize, linearMultBlockSize, depth)
            {
                double** AR = CreateMatrix(HalfSize, HalfSize);
                double** BR = CreateMatrix(HalfSize, HalfSize);
                SUB(A12, A22, AR, HalfSize);
                ADD(B21, B22, BR, HalfSize);
                Strassen(AR, BR, M7, HalfSize, linearMultBlockSize, true, depth + 1);
                FreeMatrix(AR); FreeMatrix(BR);
            }
            #pragma omp taskwait
        } else {
            // Последовательная ветка
            double** AResult = CreateMatrix(HalfSize, HalfSize);
            double** BResult = CreateMatrix(HalfSize, HalfSize);

            // P1 == M1[][]
            ADD(A11, A22, AResult, HalfSize);
            ADD(B11, B22, BResult, HalfSize);
            Strassen(AResult, BResult, M1, HalfSize, linearMultBlockSize, false, depth + 1);

            // P2 == M2[][]
            ADD(A21, A22, AResult, HalfSize);
            Strassen(AResult, B11, M2, HalfSize, linearMultBlockSize, false, depth + 1);

            // P3 == M3[][]
            SUB(B12, B22, BResult, HalfSize);
            Strassen(A11, BResult, M3, HalfSize, linearMultBlockSize, false, depth + 1);

            // P4 == M4[][]
            SUB(B21, B11, BResult, HalfSize);
            Strassen(A22, BResult, M4, HalfSize, linearMultBlockSize, false, depth + 1);

            // P5 == M5[][]
            ADD(A11, A12, AResult, HalfSize);
            Strassen(AResult, B22, M5, HalfSize, linearMultBlockSize, false, depth + 1);

            // P6 == M6[][]
            SUB(A21, A11, AResult, HalfSize);
            ADD(B11, B12, BResult, HalfSize);
            Strassen(AResult, BResult, M6, HalfSize, linearMultBlockSize, false, depth + 1);

            // P7 == M7[][]
            SUB(A12, A22, AResult, HalfSize);
            ADD(B21, B22, BResult, HalfSize);
            Strassen(AResult, BResult, M7, HalfSize, linearMultBlockSize, false, depth + 1);

            FreeMatrix(AResult);
            FreeMatrix(BResult);
        }

        double** AResult = CreateMatrix(HalfSize, HalfSize);
        double** BResult = CreateMatrix(HalfSize, HalfSize);

        // C11 = M1 + M4 - M5 + M7
        ADD(M1, M4, AResult, HalfSize);
        SUB(M7, M5, BResult, HalfSize);
        ADD(AResult, BResult, C11, HalfSize);

        // C12 = M3 + M5
        ADD(M3, M5, C12, HalfSize);

        // C21 = M2 + M4
        ADD(M2, M4, C21, HalfSize);

        // C22 = M1 + M3 - M2 + M6
        ADD(M1, M3, AResult, HalfSize);
        SUB(M6, M2, BResult, HalfSize);
        ADD(AResult, BResult, C22, HalfSize);

        // Сбор частей в единую матрицу
        for (int i = 0; i < HalfSize; i++) {
            for (int j = 0; j < HalfSize; j++) {
                MatrixC[i][j] = C11[i][j];
                MatrixC[i][j + HalfSize] = C12[i][j];
                MatrixC[i + HalfSize][j] = C21[i][j];
                MatrixC[i + HalfSize][j + HalfSize] = C22[i][j];
            }
        }

        // Очистка динамической памяти
        FreeMatrix(A11); FreeMatrix(A12); FreeMatrix(A21); FreeMatrix(A22);
        FreeMatrix(B11); FreeMatrix(B12); FreeMatrix(B21); FreeMatrix(B22);
        FreeMatrix(C11); FreeMatrix(C12); FreeMatrix(C21); FreeMatrix(C22);
        FreeMatrix(M1);  FreeMatrix(M2);  FreeMatrix(M3);  FreeMatrix(M4);
        FreeMatrix(M5);  FreeMatrix(M6);  FreeMatrix(M7);
        FreeMatrix(AResult); FreeMatrix(BResult);
    }
    return 0;
}

// Проверка и корректировка размера
int Validate_size(int size, int linearMultBlockSize) {
    int tms = size;
    while (tms > linearMultBlockSize) {
        if (tms % 2 == 0) {
            tms /= 2;
        } else {
            return 0;
        }
    }
    return 1;
}

int Find_Valid_Size(int size, int linearMultBlockSize) {
    int newsize = size;
    while (Validate_size(newsize, linearMultBlockSize) == 0) {
        newsize++;
    }
    return newsize;
}

double** ExpandMatrixToSize(double** matrix, int sizeA, int sizeB, int NewSize) {
    double** NewMatr = CreateMatrix(NewSize, NewSize);
    FillMatrZero(NewMatr, NewSize, NewSize);
    for (int i = 0; i < sizeA; i++) {
        for (int j = 0; j < sizeB; j++) {
            NewMatr[i][j] = matrix[i][j];
        }
    }
    return NewMatr;
}

// Обертка Shtrassen_Multiplication
void Shtrassen_Multiplication(double** A, double** B, double** C, int sizeA, int sizeB, int sizeC, int linearMultBlockSize, bool is_parallel, int threads) {
    int size = sizeA;

    if (sizeA == sizeB && sizeA == sizeC && Validate_size(size, linearMultBlockSize)) {
        if (is_parallel) {
            #pragma omp parallel num_threads(threads)
            {
                #pragma omp single
                {
                    Strassen(A, B, C, size, linearMultBlockSize, true, 0);
                }
            }
        } else {
            Strassen(A, B, C, size, linearMultBlockSize, false, 0);
        }
    } else {
        if (size < sizeB) size = sizeB;
        if (size < sizeC) size = sizeC;
        if (size != Find_Valid_Size(size, linearMultBlockSize)) {
            size = Find_Valid_Size(size, linearMultBlockSize);
        }

        double** TA = ExpandMatrixToSize(A, sizeA, sizeB, size);
        double** TB = ExpandMatrixToSize(B, sizeB, sizeC, size);
        double** TC = ExpandMatrixToSize(C, 0, 0, size);

        if (is_parallel) {
            #pragma omp parallel num_threads(threads)
            {
                #pragma omp single
                {
                    Strassen(TA, TB, TC, size, linearMultBlockSize, true, 0);
                }
            }
        } else {
            Strassen(TA, TB, TC, size, linearMultBlockSize, false, 0);
        }

        for (int i = 0; i < sizeA; i++) {
            for (int j = 0; j < sizeC; j++) {
                C[i][j] = TC[i][j];
            }
        }

        FreeMatrix(TA);
        FreeMatrix(TB);
        FreeMatrix(TC);
    }
    g_sink += C[0][0];
}

// 7. Запуск экспериментов
struct DataSet {
    std::string name;
    int sizeA, sizeB, sizeC;
};

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // 4 набора данных из методички
    std::vector<DataSet> datasets = {
        { "НД1 (А = 1000x1000, В = 1000x1000)", 1000, 1000, 1000 },
        { "НД2 (А = 1320x1800, В = 1800x1471)", 1320, 1800, 1471 },
        { "НД3 (А = 2420x1971, В = 1971x2500)", 2420, 1971, 2500 },
        { "НД4 (А = 2420x2500, В = 2500x2370)", 2420, 2500, 2370 }
    };

    int thread_counts[] = { 2, 3, 4 };
    const int RUNS = 15; // По методичке: не менее 15 запусков для каждой реализации
    const int STRASSEN_BLOCK = 64;

    std::cout << "Лабораторная работа 2. Задание 2.2\n";
    std::cout << "Классический метод: MultiplyMatr (с транспонированием матрицы B)\n";
    std::cout << "Быстрый метод: динамический алгоритм Штрассена (блок <= " << STRASSEN_BLOCK << ")\n";
    std::cout << "Количество запусков для каждой реализации: " << RUNS << "\n";

    for (const auto& ds : datasets) {
        int valid_strassen_size = Find_Valid_Size(std::max({ds.sizeA, ds.sizeB, ds.sizeC}), STRASSEN_BLOCK);
        std::cout << "\n======================================================================================\n";
        std::cout << "  " << ds.name << " | Размер для Штрассена: " << valid_strassen_size << "x" << valid_strassen_size << "\n";
        std::cout << "======================================================================================\n";
        std::cout << "Функция (реализация)                  Поток       Время (мс)         Sp(n)       Ep(n)\n";
        std::cout << std::string(86, '-') << "\n";

        double** A = CreateMatrix(ds.sizeA, ds.sizeB);
        double** B = CreateMatrix(ds.sizeB, ds.sizeC);
        double** C = CreateMatrix(ds.sizeA, ds.sizeC);

        // --- 1. Заполнение матриц ---
        double t_fill_seq = RunBenchmark([&]() { FillMatrix_Serial(A, B, ds.sizeA, ds.sizeB, ds.sizeC); }, RUNS);
        PrintTableRow("Заполнение матриц (посл.)", "1", t_fill_seq, t_fill_seq);

        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { FillMatrix_OMP_For(A, B, ds.sizeA, ds.sizeB, ds.sizeC, p); }, RUNS);
            PrintTableRow("Заполнение (парал.) omp for", std::to_string(p), tp, t_fill_seq);
        }
        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { FillMatrix_OMP_Sections(A, B, ds.sizeA, ds.sizeB, ds.sizeC, p); }, RUNS);
            PrintTableRow("Заполнение (парал.) omp sections", std::to_string(p), tp, t_fill_seq);
        }

        std::cout << std::string(86, '-') << "\n";

        // --- 2. Классическое перемножение матриц (единый метод  с транспонированием) ---
        double t_mul_seq = RunBenchmark([&]() { MultiplyMatr_Serial(A, B, C, ds.sizeA, ds.sizeB, ds.sizeC); }, RUNS);
        PrintTableRow("Перемножение матриц (посл.)", "1", t_mul_seq, t_mul_seq);

        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { MultiplyMatr_Parallel_For(A, B, C, ds.sizeA, ds.sizeB, ds.sizeC, p); }, RUNS);
            PrintTableRow("Перемножение Вариант 1 (omp for)", std::to_string(p), tp, t_mul_seq);
        }
        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { MultiplyMatr_Parallel_Sections(A, B, C, ds.sizeA, ds.sizeB, ds.sizeC, p); }, RUNS);
            PrintTableRow("Перемножение Вариант 2 (sections)", std::to_string(p), tp, t_mul_seq);
        }

        std::cout << std::string(86, '-') << "\n";

        // 3. Быстрый алгоритм Штрассена
        double t_str_seq = RunBenchmark([&]() { 
            Shtrassen_Multiplication(A, B, C, ds.sizeA, ds.sizeB, ds.sizeC, STRASSEN_BLOCK, false, 1); 
        }, RUNS);
        PrintTableRow("Перемножение Штрассен (посл.)", "1", t_str_seq, t_str_seq);

        for (int p : thread_counts) {
            double tp = RunBenchmark([&]() { 
                Shtrassen_Multiplication(A, B, C, ds.sizeA, ds.sizeB, ds.sizeC, STRASSEN_BLOCK, true, p); 
            }, RUNS);
            PrintTableRow("Перемножение Штрассен (парал.)", std::to_string(p), tp, t_str_seq);
        }

        FreeMatrix(A);
        FreeMatrix(B);
        FreeMatrix(C);
    }

    return 0;
}
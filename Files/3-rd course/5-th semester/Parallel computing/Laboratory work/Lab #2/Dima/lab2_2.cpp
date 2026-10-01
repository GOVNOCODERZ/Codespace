#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <omp.h>
#include <string>
#include <cstring>

// ====================== Доверительный интервал ======================
double AvgTrustedIntervalAVG(const std::vector<double>& times)
{
    int cnt = static_cast<int>(times.size());
    if (cnt <= 1) return cnt == 1 ? times[0] : 0.0;

    double avg = 0.0;
    for (double t : times) avg += t;
    avg /= cnt;

    double sd = 0.0;
    for (double t : times) sd += (t - avg) * (t - avg);
    sd = std::sqrt(sd / (cnt - 1));

    double newAvg = 0.0;
    int newCnt = 0;
    for (double t : times)
    {
        if (t >= avg - sd && t <= avg + sd)
        {
            newAvg += t;
            ++newCnt;
        }
    }
    return newCnt > 0 ? newAvg / newCnt : avg;
}

// ====================== Матрица (row-major) ======================
using Matrix = std::vector<double>;

inline double& at(Matrix& m, int rows, int cols, int i, int j)
{
    return m[static_cast<size_t>(i) * cols + j];
}
inline double at(const Matrix& m, int rows, int cols, int i, int j)
{
    return m[static_cast<size_t>(i) * cols + j];
}

// Формулы заполнения (придуманные)
void fill_A_seq(Matrix& A, int M, int N)
{
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            at(A, M, N, i, j) = 1.0 + 0.001 * (i + 1) + 0.0001 * (j + 1);
}

void fill_B_seq(Matrix& B, int N, int K)
{
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < K; ++j)
            at(B, N, K, i, j) = 2.0 - 0.0005 * (i + 1) + 0.0002 * (j + 1);
}

// Параллельное заполнение omp for
void fill_A_for(Matrix& A, int M, int N)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            at(A, M, N, i, j) = 1.0 + 0.001 * (i + 1) + 0.0001 * (j + 1);
}

void fill_B_for(Matrix& B, int N, int K)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < K; ++j)
            at(B, N, K, i, j) = 2.0 - 0.0005 * (i + 1) + 0.0002 * (j + 1);
}

// Параллельное заполнение omp sections (делим по строкам)
void fill_A_sections(Matrix& A, int M, int N)
{
    int mid = M / 2;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 0; i < mid; ++i)
                for (int j = 0; j < N; ++j)
                    at(A, M, N, i, j) = 1.0 + 0.001 * (i + 1) + 0.0001 * (j + 1);
        }
        #pragma omp section
        {
            for (int i = mid; i < M; ++i)
                for (int j = 0; j < N; ++j)
                    at(A, M, N, i, j) = 1.0 + 0.001 * (i + 1) + 0.0001 * (j + 1);
        }
    }
}

void fill_B_sections(Matrix& B, int N, int K)
{
    int mid = N / 2;
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 0; i < mid; ++i)
                for (int j = 0; j < K; ++j)
                    at(B, N, K, i, j) = 2.0 - 0.0005 * (i + 1) + 0.0002 * (j + 1);
        }
        #pragma omp section
        {
            for (int i = mid; i < N; ++i)
                for (int j = 0; j < K; ++j)
                    at(B, N, K, i, j) = 2.0 - 0.0005 * (i + 1) + 0.0002 * (j + 1);
        }
    }
}

// ====================== Классическое умножение ======================
// C = A * B, A(M×N), B(N×K), C(M×K)
void mult_classic_seq(const Matrix& A, const Matrix& B, Matrix& C, int M, int N, int K)
{
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j)
        {
            double s = 0.0;
            for (int p = 0; p < N; ++p)
                s += at(A, M, N, i, p) * at(B, N, K, p, j);
            at(C, M, K, i, j) = s;
        }
}

// Вариант 1: omp for по внешнему циклу (i)
void mult_classic_for(const Matrix& A, const Matrix& B, Matrix& C, int M, int N, int K)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j)
        {
            double s = 0.0;
            for (int p = 0; p < N; ++p)
                s += at(A, M, N, i, p) * at(B, N, K, p, j);
            at(C, M, K, i, j) = s;
        }
}

// Вариант 2: omp sections (делим строки C на 4 части)
void mult_classic_sections(const Matrix& A, const Matrix& B, Matrix& C, int M, int N, int K)
{
    int c1 = M / 4;
    int c2 = M / 2;
    int c3 = 3 * M / 4;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 0; i < c1; ++i)
                for (int j = 0; j < K; ++j)
                {
                    double s = 0.0;
                    for (int p = 0; p < N; ++p)
                        s += at(A, M, N, i, p) * at(B, N, K, p, j);
                    at(C, M, K, i, j) = s;
                }
        }
        #pragma omp section
        {
            for (int i = c1; i < c2; ++i)
                for (int j = 0; j < K; ++j)
                {
                    double s = 0.0;
                    for (int p = 0; p < N; ++p)
                        s += at(A, M, N, i, p) * at(B, N, K, p, j);
                    at(C, M, K, i, j) = s;
                }
        }
        #pragma omp section
        {
            for (int i = c2; i < c3; ++i)
                for (int j = 0; j < K; ++j)
                {
                    double s = 0.0;
                    for (int p = 0; p < N; ++p)
                        s += at(A, M, N, i, p) * at(B, N, K, p, j);
                    at(C, M, K, i, j) = s;
                }
        }
        #pragma omp section
        {
            for (int i = c3; i < M; ++i)
                for (int j = 0; j < K; ++j)
                {
                    double s = 0.0;
                    for (int p = 0; p < N; ++p)
                        s += at(A, M, N, i, p) * at(B, N, K, p, j);
                    at(C, M, K, i, j) = s;
                }
        }
    }
}

// ====================== Быстрый алгоритм: блочное (tiled) умножение ======================
const int BLOCK = 64;

void mult_blocked_seq(const Matrix& A, const Matrix& B, Matrix& C, int M, int N, int K)
{
    // Обнуляем C
    std::fill(C.begin(), C.end(), 0.0);

    for (int i0 = 0; i0 < M; i0 += BLOCK)
        for (int j0 = 0; j0 < K; j0 += BLOCK)
            for (int p0 = 0; p0 < N; p0 += BLOCK)
            {
                int i_max = std::min(i0 + BLOCK, M);
                int j_max = std::min(j0 + BLOCK, K);
                int p_max = std::min(p0 + BLOCK, N);

                for (int i = i0; i < i_max; ++i)
                    for (int j = j0; j < j_max; ++j)
                    {
                        double s = at(C, M, K, i, j);
                        for (int p = p0; p < p_max; ++p)
                            s += at(A, M, N, i, p) * at(B, N, K, p, j);
                        at(C, M, K, i, j) = s;
                    }
            }
}

void mult_blocked_for(const Matrix& A, const Matrix& B, Matrix& C, int M, int N, int K)
{
    std::fill(C.begin(), C.end(), 0.0);

    #pragma omp parallel for schedule(static) collapse(2)
    for (int i0 = 0; i0 < M; i0 += BLOCK)
        for (int j0 = 0; j0 < K; j0 += BLOCK)
            for (int p0 = 0; p0 < N; p0 += BLOCK)
            {
                int i_max = std::min(i0 + BLOCK, M);
                int j_max = std::min(j0 + BLOCK, K);
                int p_max = std::min(p0 + BLOCK, N);

                for (int i = i0; i < i_max; ++i)
                    for (int j = j0; j < j_max; ++j)
                    {
                        double s = at(C, M, K, i, j);
                        for (int p = p0; p < p_max; ++p)
                            s += at(A, M, N, i, p) * at(B, N, K, p, j);
                        at(C, M, K, i, j) = s;
                    }
            }
}

// ====================== Измерение ======================
template<typename Func>
double measure(Func f, int runs = 15)
{
    std::vector<double> times(runs);
    // прогрев
    f();
    for (int r = 0; r < runs; ++r)
    {
        double t0 = omp_get_wtime();
        f();
        double t1 = omp_get_wtime();
        times[r] = (t1 - t0) * 1000.0;
    }
    return AvgTrustedIntervalAVG(times);
}

struct Dataset {
    int M, N, K;
    const char* name;
};

int main()
{
    const int RUNS = 15;
    const int threads_list[] = {2, 3, 4};

    Dataset ds[4] = {
        {1000, 1000, 1000, "DS1: 1000x1000 * 1000x1000"},
        {1320, 1800, 1471, "DS2: 1320x1800 * 1800x1471"},
        {1800, 1800, 1800, "DS3: 1800x1800 * 1800x1800"},
        {2000, 2200, 1800, "DS4: 2000x2200 * 2200x1800"}
    };

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "=== Laboratory Work 2. Task 2.2 — Matrices ===\n";
    std::cout << "Runs per measurement: " << RUNS << "\n";
    std::cout << "Tile size for blocked algorithm: " << BLOCK << "\n";
    std::cout << "Max threads: " << omp_get_max_threads() << "\n\n";

    for (int d = 0; d < 4; ++d)
    {
        int M = ds[d].M, N = ds[d].N, K = ds[d].K;
        std::cout << "========== " << ds[d].name << " ==========\n";

        Matrix A(static_cast<size_t>(M) * N);
        Matrix B(static_cast<size_t>(N) * K);
        Matrix C(static_cast<size_t>(M) * K);

        // ----- Заполнение -----
        std::cout << "\n--- Matrix filling ---\n";

        // Последовательное
        double t_fill_seq = measure([&]() {
            fill_A_seq(A, M, N);
            fill_B_seq(B, N, K);
        }, RUNS);
        std::cout << "Filling (sequential): " << t_fill_seq << " ms\n";

        // omp for
        for (int th : threads_list)
        {
            omp_set_num_threads(th);
            double t = measure([&]() {
                fill_A_for(A, M, N);
                fill_B_for(B, N, K);
            }, RUNS);
            std::cout << "Filling (omp for, " << th << " threads): " << t << " ms"
                      << "  Sp=" << t_fill_seq / t
                      << "  Ep=" << (t_fill_seq / t) / th << "\n";
        }

        // omp sections
        for (int th : threads_list)
        {
            omp_set_num_threads(th);
            double t = measure([&]() {
                fill_A_sections(A, M, N);
                fill_B_sections(B, N, K);
            }, RUNS);
            std::cout << "Filling (omp sections, " << th << " threads): " << t << " ms"
                      << "  Sp=" << t_fill_seq / t
                      << "  Ep=" << (t_fill_seq / t) / th << "\n";
        }

        // Заполняем один раз для умножений
        fill_A_seq(A, M, N);
        fill_B_seq(B, N, K);

        // ----- Classic matrix multiplication -----
        std::cout << "\n--- Classic matrix multiplication ---\n";

        double t_classic_seq = measure([&]() {
            mult_classic_seq(A, B, C, M, N, K);
        }, RUNS);
        std::cout << "Classic (sequential): " << t_classic_seq << " ms\n";

        // Вариант 1: omp for
        for (int th : threads_list)
        {
            omp_set_num_threads(th);
            double t = measure([&]() {
                mult_classic_for(A, B, C, M, N, K);
            }, RUNS);
            std::cout << "Classic (omp for, " << th << " threads): " << t << " ms"
                      << "  Sp=" << t_classic_seq / t
                      << "  Ep=" << (t_classic_seq / t) / th << "\n";
        }

        // Вариант 2: omp sections
        for (int th : threads_list)
        {
            omp_set_num_threads(th);
            double t = measure([&]() {
                mult_classic_sections(A, B, C, M, N, K);
            }, RUNS);
            std::cout << "Classic (omp sections, " << th << " threads): " << t << " ms"
                      << "  Sp=" << t_classic_seq / t
                      << "  Ep=" << (t_classic_seq / t) / th << "\n";
        }

        // ----- Блочное (быстрый) -----
        std::cout << "\n--- Blocked matrix multiplication (fast algorithm) ---\n";

        double t_blocked_seq = measure([&]() {
            mult_blocked_seq(A, B, C, M, N, K);
        }, RUNS);
        std::cout << "Blocked (sequential): " << t_blocked_seq << " ms\n";

        for (int th : threads_list)
        {
            omp_set_num_threads(th);
            double t = measure([&]() {
                mult_blocked_for(A, B, C, M, N, K);
            }, RUNS);
            std::cout << "Blocked (omp for+collapse, " << th << " threads): " << t << " ms"
                      << "  Sp=" << t_blocked_seq / t
                      << "  Ep=" << (t_blocked_seq / t) / th << "\n";
        }

        // Проверка корректности (сравниваем один элемент)
        mult_classic_seq(A, B, C, M, N, K);
        double ref = at(C, M, K, 0, 0);
        mult_blocked_seq(A, B, C, M, N, K);
        double blk = at(C, M, K, 0, 0);
        std::cout << "\nCheck C[0][0]: classic=" << ref << "  blocked=" << blk
                  << "  diff=" << std::abs(ref - blk) << "\n";

        std::cout << "\n";
    }

    std::cout << "Done. You can copy the results into Table 2.2.\n";
    return 0;
}
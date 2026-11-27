// Использование: 4.1 [runs] (по умолчанию 100)
// Прогоняет сетку Таблицы 4.1 для 3 НД (3000, 5000, 7000; тип float)
// и сохраняет результаты замеров в results_4_1.csv.
//
// Механизмы распараллеливания (везде по внешнему циклу строк):
//   - Сложение / перемножение: seq, OpenMP for, OpenMP sections,
//     oneTBB task_group, oneTBB parallel_for (лямбда), oneTBB parallel_for (класс).
//   - Сумма элементов: seq, OpenMP for reduction(+),
//     oneTBB parallel_reduce (структура), oneTBB parallel_reduce (лямбда).
//   - Val_range (max - min): seq, OpenMP for reduction(min, max),
//     oneTBB parallel_reduce (класс), oneTBB parallel_reduce (лямбда + структура).

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include <tbb/tbb.h>

using namespace std;

// MARK: Статистика замеров
static double AvgTrustedIntervalMED(vector<double> t) {
    if (t.size() < 2) return t.empty() ? 0.0 : t[0];
    sort(t.begin(), t.end());
    int cnt = (int)t.size();
    double avg = 0.0;
    for (double v : t) avg += v;
    avg /= cnt;
    double med = t[cnt / 2], sd = 0.0, res = 0.0;
    for (double v : t) sd += (v - avg) * (v - avg);
    sd = sqrt(sd / (cnt - 1.0));
    int n = 0;
    for (double v : t)
        if (fabs(v - med) <= sd) { res += v; n++; }
    return n ? res / n : avg;
}

// MARK: Матрицы
static float** AllocMatrix(int n) {
    float** m = new float*[n];
    m[0] = new float[(size_t)n * n];
    for (int i = 1; i < n; i++)
        m[i] = m[0] + (size_t)i * n;
    return m;
}

static void FreeMatrix(float** m) {
    delete[] m[0];
    delete[] m;
}

static void FillMatrices(float** a, float** b, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] = ((i * 37 + j * 17) % 2001 - 1000) * 0.125f + (i - j) * 0.001f;
            b[i][j] = ((i * 23 + j * 41) % 2001 - 1000) * 0.125f - (i + j) * 0.0005f;
        }
    }
}

// MARK: Сложение матриц
static void AddRows(float** a, float** b, float** c, int r0, int r1, int n) {
    for (int i = r0; i < r1; i++)
        for (int j = 0; j < n; j++)
            c[i][j] = a[i][j] + b[i][j];
}

static void AddSeq(float** a, float** b, float** c, int n) {
    AddRows(a, b, c, 0, n, n);
}

static void AddOmpFor(float** a, float** b, float** c, int n) {
#pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            c[i][j] = a[i][j] + b[i][j];
}

static void AddOmpSections(float** a, float** b, float** c, int n) {
    int p = omp_get_max_threads();
    int s1 = n / p, s2 = 2 * n / p, s3 = 3 * n / p;
#pragma omp parallel sections
    {
#pragma omp section
        AddRows(a, b, c, 0, s1, n);
#pragma omp section
        if (p > 1) AddRows(a, b, c, s1, (p == 2) ? n : s2, n);
#pragma omp section
        if (p > 2) AddRows(a, b, c, s2, (p == 3) ? n : s3, n);
#pragma omp section
        if (p > 3) AddRows(a, b, c, s3, n, n);
    }
}

static void AddTbbTaskGroup(float** a, float** b, float** c, int n, int th) {
    tbb::task_group tg;
    for (int t = 0; t < th; t++) {
        int r0 = t * n / th, r1 = (t + 1) * n / th;
        tg.run([=] { AddRows(a, b, c, r0, r1, n); });
    }
    tg.wait();
}

static void AddTbbForLambda(float** a, float** b, float** c, int n) {
    tbb::parallel_for(tbb::blocked_range<int>(0, n), [&](const tbb::blocked_range<int>& r) {
        AddRows(a, b, c, r.begin(), r.end(), n);
    });
}

class AddBody {
    float** a;
    float** b;
    float** c;
    int n;
public:
    AddBody(float** a_, float** b_, float** c_, int n_) : a(a_), b(b_), c(c_), n(n_) {}
    void operator()(const tbb::blocked_range<int>& r) const {
        AddRows(a, b, c, r.begin(), r.end(), n);
    }
};

static void AddTbbForClass(float** a, float** b, float** c, int n) {
    tbb::parallel_for(tbb::blocked_range<int>(0, n), AddBody(a, b, c, n));
}

// MARK: Поэлементное перемножение матриц
static void MulRows(float** a, float** b, float** c, int r0, int r1, int n) {
    for (int i = r0; i < r1; i++)
        for (int j = 0; j < n; j++)
            c[i][j] = a[i][j] * b[i][j];
}

static void MulSeq(float** a, float** b, float** c, int n) {
    MulRows(a, b, c, 0, n, n);
}

static void MulOmpFor(float** a, float** b, float** c, int n) {
#pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            c[i][j] = a[i][j] * b[i][j];
}

static void MulOmpSections(float** a, float** b, float** c, int n) {
    int p = omp_get_max_threads();
    int s1 = n / p, s2 = 2 * n / p, s3 = 3 * n / p;
#pragma omp parallel sections
    {
#pragma omp section
        MulRows(a, b, c, 0, s1, n);
#pragma omp section
        if (p > 1) MulRows(a, b, c, s1, (p == 2) ? n : s2, n);
#pragma omp section
        if (p > 2) MulRows(a, b, c, s2, (p == 3) ? n : s3, n);
#pragma omp section
        if (p > 3) MulRows(a, b, c, s3, n, n);
    }
}

static void MulTbbTaskGroup(float** a, float** b, float** c, int n, int th) {
    tbb::task_group tg;
    for (int t = 0; t < th; t++) {
        int r0 = t * n / th, r1 = (t + 1) * n / th;
        tg.run([=] { MulRows(a, b, c, r0, r1, n); });
    }
    tg.wait();
}

static void MulTbbForLambda(float** a, float** b, float** c, int n) {
    tbb::parallel_for(tbb::blocked_range<int>(0, n), [&](const tbb::blocked_range<int>& r) {
        MulRows(a, b, c, r.begin(), r.end(), n);
    });
}

class MulBody {
    float** a;
    float** b;
    float** c;
    int n;
public:
    MulBody(float** a_, float** b_, float** c_, int n_) : a(a_), b(b_), c(c_), n(n_) {}
    void operator()(const tbb::blocked_range<int>& r) const {
        MulRows(a, b, c, r.begin(), r.end(), n);
    }
};

static void MulTbbForClass(float** a, float** b, float** c, int n) {
    tbb::parallel_for(tbb::blocked_range<int>(0, n), MulBody(a, b, c, n));
}

// MARK: Сумма всех элементов двух матриц (oneTBB: структура + лямбда)
static double SumSeq(float** a, float** b, int n) {
    double s = 0.0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            s += (double)a[i][j] + (double)b[i][j];
    return s;
}

static double SumOmpFor(float** a, float** b, int n) {
    double s = 0.0;
#pragma omp parallel for reduction(+:s) schedule(static)
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            s += (double)a[i][j] + (double)b[i][j];
    return s;
}

// Вариант 1: структурный объект
struct SumStruct {
    float** a;
    float** b;
    int n;
    double value;
    SumStruct(float** a_, float** b_, int n_) : a(a_), b(b_), n(n_), value(0.0) {}
    SumStruct(SumStruct& s, tbb::split) : a(s.a), b(s.b), n(s.n), value(0.0) {}
    void operator()(const tbb::blocked_range<int>& r) {
        double local = value;
        for (int i = r.begin(); i < r.end(); i++)
            for (int j = 0; j < n; j++)
                local += (double)a[i][j] + (double)b[i][j];
        value = local;
    }
    void join(const SumStruct& rhs) { value += rhs.value; }
};

static double SumTbbStruct(float** a, float** b, int n) {
    SumStruct total(a, b, n);
    tbb::parallel_reduce(tbb::blocked_range<int>(0, n), total);
    return total.value;
}

// Вариант 2: лямбда-выражение
static double SumTbbLambda(float** a, float** b, int n) {
    return tbb::parallel_reduce(
        tbb::blocked_range<int>(0, n),
        0.0,
        [&](const tbb::blocked_range<int>& r, double local) {
            for (int i = r.begin(); i < r.end(); i++)
                for (int j = 0; j < n; j++)
                    local += (double)a[i][j] + (double)b[i][j];
            return local;
        },
        plus<double>()
    );
}

// MARK: Val_range = maxEl - minEl
static float RangeSeq(float** a, float** b, int n) {
    float mn = FLT_MAX, mx = -FLT_MAX;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            float v1 = a[i][j], v2 = b[i][j];
            if (v1 < mn) mn = v1;
            if (v1 > mx) mx = v1;
            if (v2 < mn) mn = v2;
            if (v2 > mx) mx = v2;
        }
    return mx - mn;
}

static float RangeOmpFor(float** a, float** b, int n) {
    float mn = FLT_MAX, mx = -FLT_MAX;
#pragma omp parallel for reduction(min:mn) reduction(max:mx) schedule(static)
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            float v1 = a[i][j], v2 = b[i][j];
            if (v1 < mn) mn = v1;
            if (v1 > mx) mx = v1;
            if (v2 < mn) mn = v2;
            if (v2 > mx) mx = v2;
        }
    return mx - mn;
}

// Вариант 1: класс
class MinMaxClass {
private:
    float* const* a;
    float* const* b;
    int n;
public:
    float minVal;
    float maxVal;
    MinMaxClass(float** a_, float** b_, int n_) : a(a_), b(b_), n(n_), minVal(FLT_MAX), maxVal(-FLT_MAX) {}
    MinMaxClass(MinMaxClass& x, tbb::split) : a(x.a), b(x.b), n(x.n), minVal(FLT_MAX), maxVal(-FLT_MAX) {}
    void operator()(const tbb::blocked_range<int>& r) {
        float mn = minVal, mx = maxVal;
        for (int i = r.begin(); i < r.end(); i++)
            for (int j = 0; j < n; j++) {
                float v1 = a[i][j], v2 = b[i][j];
                if (v1 < mn) mn = v1;
                if (v1 > mx) mx = v1;
                if (v2 < mn) mn = v2;
                if (v2 > mx) mx = v2;
            }
        minVal = mn;
        maxVal = mx;
    }
    void join(const MinMaxClass& y) {
        if (y.minVal < minVal) minVal = y.minVal;
        if (y.maxVal > maxVal) maxVal = y.maxVal;
    }
};

static float RangeTbbClass(float** a, float** b, int n) {
    MinMaxClass mm(a, b, n);
    tbb::parallel_reduce(tbb::blocked_range<int>(0, n), mm);
    return mm.maxVal - mm.minVal;
}

// Вариант 2: лямбда-выражение со структурой результата
struct MinMaxVal {
    float mn;
    float mx;
    MinMaxVal() : mn(FLT_MAX), mx(-FLT_MAX) {}
};

static float RangeTbbLambda(float** a, float** b, int n) {
    MinMaxVal res = tbb::parallel_reduce(
        tbb::blocked_range<int>(0, n),
        MinMaxVal(),
        [&](const tbb::blocked_range<int>& r, MinMaxVal mm) {
            for (int i = r.begin(); i < r.end(); i++)
                for (int j = 0; j < n; j++) {
                    float v1 = a[i][j], v2 = b[i][j];
                    if (v1 < mm.mn) mm.mn = v1;
                    if (v1 > mm.mx) mm.mx = v1;
                    if (v2 < mm.mn) mm.mn = v2;
                    if (v2 > mm.mx) mm.mx = v2;
                }
            return mm;
        },
        [](const MinMaxVal& l, const MinMaxVal& r) {
            MinMaxVal out;
            out.mn = min(l.mn, r.mn);
            out.mx = max(l.mx, r.mx);
            return out;
        }
    );
    return res.mx - res.mn;
}

// MARK: Тестирование
enum class Backend { Seq, Omp, Tbb };

static double Measure(Backend bk, int th, int runs, const function<void()>& fn) {
    vector<double> times;
    times.reserve(runs);
    if (bk == Backend::Seq) {
        for (int r = 0; r < runs; r++) {
            double t0 = omp_get_wtime();
            fn();
            times.push_back((omp_get_wtime() - t0) * 1000.0);
            cout << "+" << flush;
        }
    } else if (bk == Backend::Omp) {
        omp_set_num_threads(th);
#pragma omp parallel
        {}
        for (int r = 0; r < runs; r++) {
            double t0 = omp_get_wtime();
            fn();
            times.push_back((omp_get_wtime() - t0) * 1000.0);
            cout << "+" << flush;
        }
    } else {
        tbb::global_control gc(tbb::global_control::max_allowed_parallelism, th);
        tbb::task_arena arena(th);
        arena.execute([&] {
            tbb::parallel_for(0, th * 4, [](int) {});
            for (int r = 0; r < runs; r++) {
                double t0 = omp_get_wtime();
                fn();
                times.push_back((omp_get_wtime() - t0) * 1000.0);
                cout << "+" << flush;
            }
        });
    }
    cout << endl;
    return AvgTrustedIntervalMED(times);
}

struct Case {
    string name;
    Backend bk;
    function<void(int)> run;
};

static void RunGroup(int ds, int n, int runs, const vector<Case>& cases,
                     const function<void()>& beforeCase, const function<bool()>& checkRef, ofstream& csv) {
    double tSeq = 0.0;
    for (const auto& c : cases) {
        int thMin = (c.bk == Backend::Seq) ? 1 : 2;
        int thMax = (c.bk == Backend::Seq) ? 1 : 4;
        for (int th = thMin; th <= thMax; th++) {
            beforeCase();
            cout << c.name;
            if (th > 1) cout << " [" << th << " threads]";
            cout << "\t" << flush;

            double avg = Measure(c.bk, th, runs, [&] { c.run(th); });
            if (c.bk == Backend::Seq) tSeq = avg;
            double sp = (avg > 0.0) ? (tSeq / avg) : 0.0;
            bool ok = checkRef();

            cout << "  avg: " << avg << " ms, Sp: " << sp << (ok ? "" : "  <-- MISMATCH") << endl;
            csv << ds << ',' << n << ',' << '"' << c.name << '"' << ',' << th << ','
                << avg << ',' << sp << ',' << (ok ? "OK" : "FAIL") << '\n';
            csv.flush();
        }
    }
}

int main(int argc, char** argv) {
    int runs = (argc > 1) ? atoi(argv[1]) : 100;
    if (runs < 1) runs = 1;

    const int sizes[3] = { 3000, 5000, 7000 };

    ofstream csv("results_4_1.csv");
    if (!csv.is_open()) {
        cerr << "Cannot open results_4_1.csv" << endl;
        return 1;
    }
    csv << "Dataset,Size,Function,Threads,Time_ms,Speedup,Check\n";

    cout << "Runs: " << runs << ", OpenMP max threads: " << omp_get_max_threads()
         << ", oneTBB default threads: " << tbb::info::default_concurrency() << endl;

    for (int ds = 0; ds < 3; ds++) {
        int n = sizes[ds];
        size_t bytes = (size_t)n * n * sizeof(float);
        cout << "\n=== Dataset " << (ds + 1) << ": " << n << "x" << n << " (float) ===" << endl;

        float** a   = AllocMatrix(n);
        float** b   = AllocMatrix(n);
        float** c   = AllocMatrix(n);
        float** ref = AllocMatrix(n);
        FillMatrices(a, b, n);

        // 1. Сложение
        AddSeq(a, b, ref, n);
        RunGroup(ds + 1, n, runs, {
            { "Add (seq)",                   Backend::Seq, [&](int)    { AddSeq(a, b, c, n); } },
            { "Add OpenMP FOR",              Backend::Omp, [&](int)    { AddOmpFor(a, b, c, n); } },
            { "Add OpenMP Sections",         Backend::Omp, [&](int)    { AddOmpSections(a, b, c, n); } },
            { "Add oneTBB task_group",       Backend::Tbb, [&](int th) { AddTbbTaskGroup(a, b, c, n, th); } },
            { "Add oneTBB parallel_for lam", Backend::Tbb, [&](int)    { AddTbbForLambda(a, b, c, n); } },
            { "Add oneTBB parallel_for cls", Backend::Tbb, [&](int)    { AddTbbForClass(a, b, c, n); } }
        }, [&] { memset(c[0], 0, bytes); }, [&] { return memcmp(c[0], ref[0], bytes) == 0; }, csv);

        // 2. Перемножение
        MulSeq(a, b, ref, n);
        RunGroup(ds + 1, n, runs, {
            { "Mul (seq)",                   Backend::Seq, [&](int)    { MulSeq(a, b, c, n); } },
            { "Mul OpenMP FOR",              Backend::Omp, [&](int)    { MulOmpFor(a, b, c, n); } },
            { "Mul OpenMP Sections",         Backend::Omp, [&](int)    { MulOmpSections(a, b, c, n); } },
            { "Mul oneTBB task_group",       Backend::Tbb, [&](int th) { MulTbbTaskGroup(a, b, c, n, th); } },
            { "Mul oneTBB parallel_for lam", Backend::Tbb, [&](int)    { MulTbbForLambda(a, b, c, n); } },
            { "Mul oneTBB parallel_for cls", Backend::Tbb, [&](int)    { MulTbbForClass(a, b, c, n); } }
        }, [&] { memset(c[0], 0, bytes); }, [&] { return memcmp(c[0], ref[0], bytes) == 0; }, csv);

        // 3. Сумма всех элементов
        double sumRef = SumSeq(a, b, n), sumRes = 0.0;
        RunGroup(ds + 1, n, runs, {
            { "Sum (seq)",                  Backend::Seq, [&](int) { sumRes = SumSeq(a, b, n); } },
            { "Sum OpenMP FOR reduction",   Backend::Omp, [&](int) { sumRes = SumOmpFor(a, b, n); } },
            { "Sum oneTBB reduce (struct)", Backend::Tbb, [&](int) { sumRes = SumTbbStruct(a, b, n); } },
            { "Sum oneTBB reduce (lambda)", Backend::Tbb, [&](int) { sumRes = SumTbbLambda(a, b, n); } }
        }, [&] { sumRes = 0.0; }, [&] { return fabs(sumRes - sumRef) / (1.0 + fabs(sumRef)) < 1e-9; }, csv);

        // 4. Val_range = maxEl - minEl
        float rngRef = RangeSeq(a, b, n), rngRes = 0.0f;
        RunGroup(ds + 1, n, runs, {
            { "Val_range (seq)",                  Backend::Seq, [&](int) { rngRes = RangeSeq(a, b, n); } },
            { "Val_range OpenMP FOR reduction",   Backend::Omp, [&](int) { rngRes = RangeOmpFor(a, b, n); } },
            { "Val_range oneTBB reduce (class)",  Backend::Tbb, [&](int) { rngRes = RangeTbbClass(a, b, n); } },
            { "Val_range oneTBB reduce (lambda)", Backend::Tbb, [&](int) { rngRes = RangeTbbLambda(a, b, n); } }
        }, [&] { rngRes = 0.0f; }, [&] { return fabs(rngRes - rngRef) <= 1e-5f; }, csv);

        FreeMatrix(a);
        FreeMatrix(b);
        FreeMatrix(c);
        FreeMatrix(ref);
    }

    csv.close();
    cout << "\nResults written to results_4_1.csv" << endl;
    return 0;
}

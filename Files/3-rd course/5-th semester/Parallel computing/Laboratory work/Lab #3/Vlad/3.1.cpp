// TODO: Переписать qsort - одна реализация у посл. и парал., а не 2 разных
// TODO: Вывести переключение потоков из функции сортировки (идёт в зачёт времени) в одно место переключения потоков типа MakeImpls


// Использование: Task3_1_Sorting [runs] (по умолчанию 30)

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <omp.h>

using namespace std;

// MARK: Генерация данных и проверка
// Данные строятся по закономерности от индекса, одинаковые при каждом запуске
template <typename T>
void FillData(vector<T>& a, int n);

template <>
void FillData<int>(vector<int>& a, int n) {
    for (int i = 0; i < n; i++)
        a[i] = (int)(((unsigned long long)i * 2654435761ULL + 12345ULL) % 1000003ULL);
}

template <>
void FillData<double>(vector<double>& a, int n) {
    for (int i = 0; i < n; i++)
        a[i] = sin(i * 0.37) * 1000.0 + cos(i * 0.11) * 500.0 + i * 0.001;
}

template <typename T>
bool SameArray(const vector<T>& a, const vector<T>& ref) {
    return a == ref;
}

// MARK: Статистика замеров
static void BubbleSortD(vector<double>& v) {
    bool ex;
    do {
        ex = false;
        for (size_t i = 0; i + 1 < v.size(); i++)
            if (v[i] > v[i + 1]) { swap(v[i], v[i + 1]); ex = true; }
    } while (ex);
}

static double AvgTrustedIntervalMED(vector<double> t) {
    if (t.size() < 2) return t.empty() ? 0.0 : t[0];
    BubbleSortD(t);
    int cnt = (int)t.size();
    double avg = 0;
    for (double v : t) avg += v;
    avg /= cnt;
    double med = t[cnt / 2], sd = 0, res = 0;
    for (double v : t) sd += (v - avg) * (v - avg);
    sd = sqrt(sd / (cnt - 1.0));
    int n = 0;
    for (double v : t)
        if (fabs(v - med) <= sd) { res += v; n++; }
    return n ? res / n : avg;
}

// MARK: Пузырёк (последовательный)
template <typename T>
void BubbleSortSeq(T* a, int n) {
    bool swapped;
    do {
        swapped = false;
        for (int i = 0; i + 1 < n; i++)
            if (a[i] > a[i + 1]) { swap(a[i], a[i + 1]); swapped = true; }
    } while (swapped);
}

// MARK: Чёт-нечётная перестановка
template <typename T>
void OddEvenSortSeq(T* a, int n) {
    bool swapped;
    do {
        swapped = false;
        for (int i = 0; i + 1 < n; i += 2)
            if (a[i] > a[i + 1]) { swap(a[i], a[i + 1]); swapped = true; }
        for (int i = 1; i + 1 < n; i += 2)
            if (a[i] > a[i + 1]) { swap(a[i], a[i + 1]); swapped = true; }
    } while (swapped);
}

template <typename T>
void OddEvenSortPar(T* a, int n, int threads) {
    int swapped;
    do {
        swapped = 0;
        for (int s = 0; s < 2; s++) {
            long long pairs = ((long long)n - s) / 2;
#pragma omp parallel for num_threads(threads) reduction(| : swapped) schedule(static)
            for (long long k = 0; k < pairs; k++) {
                long long i = s + 2 * k;
                if (a[i] > a[i + 1]) {
                    swap(a[i], a[i + 1]);
                    swapped = 1;
                }
            }
        }
    } while (swapped);
}

// MARK: Шелла
template <typename T>
void ShellSortSeq(T* a, int n) {
    for (int gap = n / 2; gap > 0; gap /= 2)
        for (int i = gap; i < n; i++) {
            T tmp = a[i];
            int j = i;
            while (j >= gap && a[j - gap] > tmp) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = tmp;
        }
}

// Элементы с одинаковым остатком от деления на gap образуют независимые подпоследовательности,
// которые сортируются вставками с шагом gap. Подпоследовательности параллелятся по остаткам.
template <typename T>
void ShellSortPar(T* a, int n, int threads) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
#pragma omp parallel for num_threads(threads) schedule(static)
        for (int r = 0; r < gap; r++) {
            for (int i = r + gap; i < n; i += gap) {
                T tmp = a[i];
                int j = i;
                while (j - gap >= r && a[j - gap] > tmp) {
                    a[j] = a[j - gap];
                    j -= gap;
                }
                a[j] = tmp;
            }
        }
    }
}

// MARK: Быстрая сортировка
template <typename T>
int HoarePartition(T* a, int lo, int hi) {
    T pivot = a[lo + (hi - lo) / 2];
    int i = lo - 1, j = hi + 1;
    while (true) {
        do i++; while (a[i] < pivot);
        do j--; while (a[j] > pivot);
        if (i >= j) return j;
        swap(a[i], a[j]);
    }
}

template <typename T>
void QuickSortRangeSeq(T* a, int lo, int hi) {
    if (lo >= hi) return;
    int p = HoarePartition(a, lo, hi);
    QuickSortRangeSeq(a, lo, p);
    QuickSortRangeSeq(a, p + 1, hi);
}

// Задачи создаются только для достаточно больших отрезков, мелкие сортируются без задач
const int QS_TASK_CUTOFF = 2000;

template <typename T>
void QuickSortRangePar(T* a, int lo, int hi) {
    if (lo >= hi) return;
    if (hi - lo < QS_TASK_CUTOFF) {
        QuickSortRangeSeq(a, lo, hi);
        return;
    }
    int p = HoarePartition(a, lo, hi);
#pragma omp task shared(a) untied
    QuickSortRangePar(a, lo, p);
    QuickSortRangePar(a, p + 1, hi);
#pragma omp taskwait
}

template <typename T>
void QuickSortSeq(T* a, int n) {
    QuickSortRangeSeq(a, 0, n - 1);
}

template <typename T>
void QuickSortPar(T* a, int n, int threads) {
#pragma omp parallel num_threads(threads)
#pragma omp single
    QuickSortRangePar(a, 0, n - 1);
}

// Библиотечный qsort - только для справки
template <typename T>
int CmpT(const void* x, const void* y) {
    T a = *(const T*)x, b = *(const T*)y;
    return (a > b) - (a < b);
}

template <typename T>
void LibQSortSeq(T* a, int n) {
    qsort(a, (size_t)n, sizeof(T), CmpT<T>);
}

// MARK: Тестирование
template <typename T>
struct Impl {
    string name;
    int threads;
    function<void(T*, int)> run;
};

template <typename T>
vector<Impl<T>> MakeImpls() {
    vector<Impl<T>> v;
    v.push_back({ "Bubble (seq)", 1, [](T* a, int n) { BubbleSortSeq(a, n); } });
    v.push_back({ "Odd-even (seq)", 1, [](T* a, int n) { OddEvenSortSeq(a, n); } });
    for (int t = 2; t <= 4; t++)
        v.push_back({ "Odd-even (par)", t, [t](T* a, int n) { OddEvenSortPar(a, n, t); } });
    v.push_back({ "Shell (seq)", 1, [](T* a, int n) { ShellSortSeq(a, n); } });
    for (int t = 2; t <= 4; t++)
        v.push_back({ "Shell (par)", t, [t](T* a, int n) { ShellSortPar(a, n, t); } });
    v.push_back({ "Quick (seq)", 1, [](T* a, int n) { QuickSortSeq(a, n); } });
    for (int t = 2; t <= 4; t++)
        v.push_back({ "Quick (par)", t, [t](T* a, int n) { QuickSortPar(a, n, t); } });
    v.push_back({ "Library qsort (seq, reference)", 1, [](T* a, int n) { LibQSortSeq(a, n); } });
    return v;
}

template <typename T>
void RunType(const string& typeName, const int sizes[], int setCount, int runs, ofstream& csv) {
    for (int s = 0; s < setCount; s++) {
        const int n = sizes[s];
        cout << "\n=== " << typeName << ", Data Pack" << (s + 1) << ": n = " << n << " ===" << endl;

        vector<T> src(n);
        FillData<T>(src, n);

        // Эталон для проверки корректности (вне замеров)
        vector<T> ref = src;
        sort(ref.begin(), ref.end());

        vector<T> work(n);
        for (auto& impl : MakeImpls<T>()) {
            cout << impl.name;
            if (impl.threads > 1) cout << " [" << impl.threads << " threads]";
            cout << "\t" << flush;

            vector<double> times;
            times.reserve(runs);
            bool ok = true;
            for (int r = 0; r < runs; r++) {
                work = src; // копирование не входит в замер
                double t0 = omp_get_wtime();
                impl.run(work.data(), n);
                double t1 = omp_get_wtime();
                times.push_back((t1 - t0) * 1000);
                if (r == 0 && !SameArray(work, ref)) ok = false;
                cout << "+" << flush;
            }
            cout << endl;

            double avg = AvgTrustedIntervalMED(times);
            cout << "  avg: " << avg << " ms" << (ok ? "" : "  <-- RESULT MISMATCH") << endl;

            csv << typeName << ',' << (s + 1) << ',' << n << ',' << '"' << impl.name << '"' << ','
                << impl.threads << ',' << avg << ',' << (ok ? "OK" : "FAIL") << '\n';
            csv.flush();
        }
    }
}

int main(int argc, char** argv) {
    int runs = (argc > 1) ? atoi(argv[1]) : 30;
    if (runs < 1) runs = 1;

    // Три НД
    const int sizes[3] = { 20000, 35000, 50000 };

    ofstream csv("results_3_1.csv");
    if (!csv.is_open()) {
        cerr << "Cannot open results_3_1.csv" << endl;
        return 1;
    }
    csv << "Type,Dataset,Size,Algorithm,Threads,Time_ms,Check" << '\n';

    cout << "Runs per implementation: " << runs << ", max threads: " << omp_get_max_threads() << endl;
    RunType<int>("int", sizes, 3, runs, csv);
    RunType<double>("double", sizes, 3, runs, csv);

    csv.close();
    cout << "\nResults written to results_3_1.csv" << endl;
    return 0;
}
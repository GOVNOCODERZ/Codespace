// Использование: Task3_2_MedianFilter [runs] (по умолчанию 15)
// Автоматически прогоняет сетку Таблицы 3.2 по файлам input_1.bmp .. input_4.bmp
// (отсутствующие файлы пропускаются), сохраняет output_X_sh.bmp / output_X_qs.bmp
// и записывает результаты замеров в results_3_2.csv.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include "3.2.h"

using namespace std;

// MARK: Сортировки окрестности (массив значений одного канала, 0..255)
// Сортировка Шелла, последовательная (как в задании 3.1).
static void ShellSort(uint8_t* a, int n) {
    for (int gap = n / 2; gap > 0; gap /= 2)
        for (int i = gap; i < n; i++) {
            uint8_t tmp = a[i];
            int j = i;
            while (j >= gap && a[j - gap] > tmp) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = tmp;
        }
}

// Быстрая сортировка (разбиение Хоара, как в задании 3.1), последовательная.
static int HoarePartition(uint8_t* a, int lo, int hi) {
    uint8_t pivot = a[lo + (hi - lo) / 2];
    int i = lo - 1, j = hi + 1;
    while (true) {
        do i++; while (a[i] < pivot);
        do j--; while (a[j] > pivot);
        if (i >= j) return j;
        swap(a[i], a[j]);
    }
}

static void QuickSortRange(uint8_t* a, int lo, int hi) {
    if (lo >= hi) return;
    int p = HoarePartition(a, lo, hi);
    QuickSortRange(a, lo, p);
    QuickSortRange(a, p + 1, hi);
}

static void QuickSort(uint8_t* a, int n) {
    if (n > 1) QuickSortRange(a, 0, n - 1);
}

typedef void (*SortFn)(uint8_t*, int);

// MARK: Медианная фильтрация одной строки
// Обрабатывает строку y. Буферы bR, bG, bB принадлежат потоку и не делятся между потоками.
static void MedianRow(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R, int y, uint8_t* bR, uint8_t* bG, uint8_t* bB, SortFn sort) {
    const int N = (2 * R + 1) * (2 * R + 1);
    const int mid = N / 2;
    for (int x = 0; x < W; x++) {
        int m = 0;
        for (int dy = -R; dy <= R; dy++) {
            int ky = y + dy;
            if (ky < 0) ky = 0; // зажим по краям
            if (ky > H - 1) ky = H - 1;
            for (int dx = -R; dx <= R; dx++) {
                int kx = x + dx;
                if (kx < 0) kx = 0;
                if (kx > W - 1) kx = W - 1;
                const RGBTRIPLE& p = in[ky][kx];
                bR[m] = p.rgbtRed;
                bG[m] = p.rgbtGreen;
                bB[m] = p.rgbtBlue;
                m++;
            }
        }
        sort(bR, N);
        sort(bG, N);
        sort(bB, N);
        out[y][x].rgbtRed   = bR[mid];
        out[y][x].rgbtGreen = bG[mid];
        out[y][x].rgbtBlue  = bB[mid];
    }
}

// MARK: Фильтр целиком с замером времени (только обработка, без чтения/записи файла)
// Число потоков задаётся заранее через omp_set_num_threads вне замера.
double MedianFilter(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R, SortFn sort) {
    const int N = (2 * R + 1) * (2 * R + 1);
    double t0 = omp_get_wtime();
    if (omp_get_max_threads() > 1) {
#pragma omp parallel
        {
            // Буферы выделяются один раз на поток, а не на каждый пиксель
            vector<uint8_t> bR(N), bG(N), bB(N);
#pragma omp for schedule(static)
            for (int y = 0; y < H; y++)
                MedianRow(in, out, W, H, R, y, bR.data(), bG.data(), bB.data(), sort);
        }
    } else {
        vector<uint8_t> bR(N), bG(N), bB(N);
        for (int y = 0; y < H; y++)
            MedianRow(in, out, W, H, R, y, bR.data(), bG.data(), bB.data(), sort);
    }
    return (omp_get_wtime() - t0) * 1000;
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

// MARK: Выделение памяти под изображение (один блок + указатели на строки)
static RGBTRIPLE** AllocImage(int W, int H) {
    RGBTRIPLE** m = new RGBTRIPLE*[H];
    m[0] = new RGBTRIPLE[(size_t)W * H];
    for (int y = 1; y < H; y++) m[y] = m[0] + (size_t)y * W;
    return m;
}

static void FreeImage(RGBTRIPLE** m) {
    delete[] m[0];
    delete[] m;
}

// MARK: Тестирование
struct SortAlgo {
    string code;
    string name;
    SortFn fn;
};

static void RunDataset(int dsIndex, const string& inPath, int runs, ofstream& csv) {
    {
        ifstream test(inPath, ios::binary);
        if (!test) {
            cout << "\n[SKIP] Dataset " << dsIndex << ": " << inPath << " not found" << endl;
            return;
        }
    }

    RGBTRIPLE** in = nullptr;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    BMPRead(in, fh, ih, inPath.c_str());
    const int W = ih.biWidth;
    const int H = ih.biHeight;
    if (W <= 0 || H <= 0 || ih.biBitCount != 24) {
        cerr << "Unsupported image: " << inPath << " (" << W << "x" << H << ", bpp=" << ih.biBitCount << ")" << endl;
        FreeImage(in);
        return;
    }

    const string resStr = to_string(W) + "x" + to_string(H);
    cout << "\n=== Dataset " << dsIndex << ": " << inPath << " (" << resStr << ") ===" << endl;

    RGBTRIPLE** out = AllocImage(W, H);
    const size_t pixelCount = (size_t)W * H;
    const size_t byteCount = pixelCount * sizeof(RGBTRIPLE);

    // По методичке: Ksize = 7, 11, 15 (R = 3, 5, 7)
    const int radii[3] = { 3, 5, 7 };
    const SortAlgo algos[2] = {
        { "sh", "Shell", ShellSort },
        { "qs", "Quick", QuickSort }
    };

    // Эталон результата для каждого R (сохраняется на первом прогоне Shell, 1 поток)
    vector<RGBTRIPLE> refs[3];

    for (const auto& algo : algos) {
        for (int th = 1; th <= 4; th++) {
            // Переключаем число потоков и прогреваем пул вне замера времени
            omp_set_num_threads(th);
            if (th > 1) {
#pragma omp parallel
                {}
            }

            const string algLabel = algo.name + (th == 1 ? " (seq)" : " (par)");

            for (int ri = 0; ri < 3; ri++) {
                const int R = radii[ri];
                const int Ksize = 2 * R + 1;

                cout << algLabel;
                if (th > 1) cout << " [" << th << " threads]";
                cout << " Ksize=" << Ksize << "\t" << flush;

                vector<double> times;
                times.reserve(runs);
                for (int r = 0; r < runs; r++) {
                    times.push_back(MedianFilter(in, out, W, H, R, algo.fn));
                    cout << "+" << flush;
                }
                cout << endl;

                bool ok = true;
                if (refs[ri].empty()) {
                    refs[ri].assign(out[0], out[0] + pixelCount);
                } else if (memcmp(out[0], refs[ri].data(), byteCount) != 0) {
                    ok = false;
                }

                // Сохраняем выходной файл output_X_alg.bmp (по методичке) один раз для алгоритма
                if (th == 1 && ri == 0) {
                    const string outPath = "output_" + to_string(dsIndex) + "_" + algo.code + ".bmp";
                    BMPWrite(out, W, H, outPath.c_str());
                    cout << "  saved: " << outPath << endl;
                }

                double avg = AvgTrustedIntervalMED(times);
                cout << "  avg: " << avg << " ms" << (ok ? "" : "  <-- RESULT MISMATCH") << endl;

                csv << dsIndex << ',' << '"' << inPath << '"' << ',' << '"' << resStr << '"' << ','
                    << '"' << algLabel << '"' << ',' << th << ',' << Ksize << ','
                    << avg << ',' << (ok ? "OK" : "FAIL") << '\n';
                csv.flush();
            }
        }
    }

    FreeImage(in);
    FreeImage(out);
}

int main(int argc, char** argv) {
    int runs = (argc > 1) ? atoi(argv[1]) : 15;
    if (runs < 1) runs = 1;

    ofstream csv("results_3_2.csv");
    if (!csv.is_open()) {
        cerr << "Cannot open results_3_2.csv" << endl;
        return 1;
    }
    csv << "Dataset,Image,Resolution,Algorithm,Threads,Ksize,Time_ms,Check" << '\n';

    cout << "Runs per implementation: " << runs << ", max threads: " << omp_get_max_threads() << endl;
    for (int ds = 1; ds <= 4; ds++) {
        RunDataset(ds, "input_" + to_string(ds) + ".bmp", runs, csv);
    }

    csv.close();
    cout << "\nResults written to results_3_2.csv" << endl;
    return 0;
}

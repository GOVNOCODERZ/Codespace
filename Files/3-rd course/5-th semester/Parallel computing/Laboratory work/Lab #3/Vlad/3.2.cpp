// TODO: заменить qsort на Шелла, т.к. второй быстрее будет


// Использование: Task3_2_MedianFilter <input.bmp> <output.bmp> <R> [oe|qs] [threads] [runs]
// R - полуразмер окна, Ksize = 2R+1
// oe|qs - сортировка: чёт-нечётная перестановка или qsort (по умолчанию oe)
// threads - число потоков (по умолчанию максимум системы)
// runs - количество запусков для замера (по умолчанию 15)

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include "3.2.h"

using namespace std;

// MARK: Сортировки окрестности (массив значений одного канала, 0..255)
// Чёт-нечётная перестановка V3, последовательная.
static void OddEvenSort(uint8_t* a, int n) {
    bool swapped;
    do {
        swapped = false;
        for (int i = 0; i < n - 1; i += 2)
            if (a[i] > a[i + 1]) { swap(a[i], a[i + 1]); swapped = true; }
        for (int i = 1; i < n - 1; i += 2)
            if (a[i] > a[i + 1]) { swap(a[i], a[i + 1]); swapped = true; }
    } while (swapped);
}

static int CmpU8(const void* x, const void* y) {
    return (int)*(const uint8_t*)x - (int)*(const uint8_t*)y;
}

// qsort из стандартной библиотеки
static void QSortU8(uint8_t* a, int n) {
    qsort(a, (size_t)n, sizeof(uint8_t), CmpU8);
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
double MedianFilter(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R, SortFn sort, int threads) {
    const int N = (2 * R + 1) * (2 * R + 1);
    double t0 = omp_get_wtime();
#pragma omp parallel num_threads(threads)
    {
        // Буферы выделяются один раз на поток, а не на каждый пиксель
        vector<uint8_t> bR(N), bG(N), bB(N);
#pragma omp for schedule(static)
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
int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <input.bmp> <output.bmp> <R> [oe|qs] [threads] [runs]" << endl;
        return 1;
    }
    const char* inPath = argv[1];
    const char* outPath = argv[2];
    int R = atoi(argv[3]);
    string sortName = (argc > 4) ? argv[4] : "oe";
    int threads = (argc > 5) ? atoi(argv[5]) : omp_get_max_threads();
    int runs = (argc > 6) ? atoi(argv[6]) : 15;

    if (R < 0 || threads < 1 || runs < 1) {
        cerr << "Invalid parameters" << endl;
        return 1;
    }
    SortFn sort = (sortName == "qs") ? QSortU8 : OddEvenSort;

    {
        ifstream test(inPath, ios::binary);
        if (!test) {
            cerr << "Cannot open input file: " << inPath << endl;
            return 1;
        }
    }

    RGBTRIPLE** in = nullptr;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    BMPRead(in, fh, ih, inPath);
    const int W = ih.biWidth;
    const int H = ih.biHeight;
    if (W <= 0 || H <= 0 || ih.biBitCount != 24) {
        cerr << "Unsupported image: " << W << "x" << H << ", bpp=" << ih.biBitCount << endl;
        FreeImage(in);
        return 1;
    }

    RGBTRIPLE** out = AllocImage(W, H);

    omp_set_num_threads(threads);
    cout << "Image: " << W << "x" << H << ", Ksize=" << (2 * R + 1) << ", sort=" << sortName << ", threads=" << threads << ", runs=" << runs << endl;

    vector<double> times;
    times.reserve(runs);
    for (int r = 0; r < runs; r++) {
        times.push_back(MedianFilter(in, out, W, H, R, sort, threads));
        cout << "+" << flush;
    }
    cout << endl;

    double avg = AvgTrustedIntervalMED(times);
    cout << "Average time (trusted interval): " << avg << " ms" << endl;

    BMPWrite(out, W, H, outPath);
    cout << "Saved: " << outPath << endl;

    FreeImage(in);
    FreeImage(out);
    return 0;
}
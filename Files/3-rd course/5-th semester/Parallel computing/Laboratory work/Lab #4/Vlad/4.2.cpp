// Использование: 4.2 [runs] (по умолчанию 15)
// Прогоняет сетку Таблицы 4.2 по input_1.bmp .. input_3.bmp (Ksize = 5, 9, 13),
// сохраняет output_X_seq.bmp / output_X_omp.bmp / output_X_tbb.bmp и results_4_2.csv.
//
// Реализованные механизмы:
//   - Сортировка окна: сортировка Шелла (ShellSort, из лаб. 3).
//   - Распараллеливание (внешний цикл по строкам): seq,
//     OpenMP parallel for (schedule(static)), oneTBB parallel_for (blocked_range, лямбда).

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <omp.h>
#include <tbb/tbb.h>
#include "bmp.h"

using namespace std;

// MARK: Сортировка Шелла и обработка строки (из лаб. 3)
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

static void MedianRow(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R, int y,
                      uint8_t* bR, uint8_t* bG, uint8_t* bB) {
    const int N = (2 * R + 1) * (2 * R + 1);
    const int mid = N / 2;
    for (int x = 0; x < W; x++) {
        int m = 0;
        for (int dy = -R; dy <= R; dy++) {
            int ky = y + dy;
            if (ky < 0) ky = 0;
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
        ShellSort(bR, N);
        ShellSort(bG, N);
        ShellSort(bB, N);
        out[y][x].rgbtRed   = bR[mid];
        out[y][x].rgbtGreen = bG[mid];
        out[y][x].rgbtBlue  = bB[mid];
    }
}

// MARK: Реализации фильтра (распараллеливается только внешний цикл по строкам y)
static void MedianSeq(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R) {
    const int N = (2 * R + 1) * (2 * R + 1);
    vector<uint8_t> bR(N), bG(N), bB(N);
    for (int y = 0; y < H; y++)
        MedianRow(in, out, W, H, R, y, bR.data(), bG.data(), bB.data());
}

static void MedianOmp(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R) {
    const int N = (2 * R + 1) * (2 * R + 1);
#pragma omp parallel
    {
        vector<uint8_t> bR(N), bG(N), bB(N);
#pragma omp for schedule(static)
        for (int y = 0; y < H; y++)
            MedianRow(in, out, W, H, R, y, bR.data(), bG.data(), bB.data());
    }
}

static void MedianTbb(RGBTRIPLE** in, RGBTRIPLE** out, int W, int H, int R) {
    const int N = (2 * R + 1) * (2 * R + 1);
    tbb::parallel_for(tbb::blocked_range<int>(0, H), [&](const tbb::blocked_range<int>& r) {
        vector<uint8_t> bR(N), bG(N), bB(N);
        for (int y = r.begin(); y < r.end(); y++)
            MedianRow(in, out, W, H, R, y, bR.data(), bG.data(), bB.data());
    });
}

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

// MARK: Тестирование
struct FilterImpl {
    string code;
    string name;
    Backend bk;
    void (*fn)(RGBTRIPLE**, RGBTRIPLE**, int, int, int);
};

static void RunDataset(int ds, const string& inPath, int runs, ofstream& csv) {
    if (!ifstream(inPath, ios::binary)) {
        cout << "\n[SKIP] Dataset " << ds << ": " << inPath << " not found" << endl;
        return;
    }

    RGBTRIPLE** in = nullptr;
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER ih;
    BMPRead(in, fh, ih, inPath.c_str());
    const int W = ih.biWidth, H = ih.biHeight;
    if (W <= 0 || H <= 0 || ih.biBitCount != 24) {
        cerr << "Unsupported image: " << inPath << endl;
        FreeImage(in);
        return;
    }

    const string resStr = to_string(W) + "x" + to_string(H);
    cout << "\n=== Dataset " << ds << ": " << inPath << " (" << resStr << ") ===" << endl;

    RGBTRIPLE** out = AllocImage(W, H);
    const size_t pixels = (size_t)W * H;
    const size_t bytes  = pixels * sizeof(RGBTRIPLE);

    // По методичке: Ksize = 5, 9, 13 (R = 2, 4, 6)
    const int radii[3] = { 2, 4, 6 };
    const FilterImpl impls[3] = {
        { "seq", "Median Shell (seq)",   Backend::Seq, MedianSeq },
        { "omp", "Median Shell OpenMP",  Backend::Omp, MedianOmp },
        { "tbb", "Median Shell oneTBB",  Backend::Tbb, MedianTbb }
    };

    vector<RGBTRIPLE> refs[3];
    double tSeq[3] = { 0.0, 0.0, 0.0 };

    for (const auto& impl : impls) {
        int thMin = (impl.bk == Backend::Seq) ? 1 : 2;
        int thMax = (impl.bk == Backend::Seq) ? 1 : 4;
        for (int th = thMin; th <= thMax; th++) {
            for (int ri = 0; ri < 3; ri++) {
                int R = radii[ri], Ksize = 2 * R + 1;
                memset(out[0], 0, bytes);

                cout << impl.name;
                if (th > 1) cout << " [" << th << " threads]";
                cout << " Ksize=" << Ksize << "\t" << flush;

                double avg = Measure(impl.bk, th, runs, [&] { impl.fn(in, out, W, H, R); });

                bool ok = true;
                if (impl.bk == Backend::Seq) {
                    tSeq[ri] = avg;
                    refs[ri].assign(out[0], out[0] + pixels);
                } else if (memcmp(out[0], refs[ri].data(), bytes) != 0) {
                    ok = false;
                }
                double sp = (avg > 0.0) ? (tSeq[ri] / avg) : 0.0;

                // Сохраняем выходной файл output_X_alg.bmp для Ksize = 9 (ri == 1)
                if (ri == 1 && (th == 1 || th == 4)) {
                    string outPath = "output_" + to_string(ds) + "_" + impl.code + ".bmp";
                    BMPWrite(out, W, H, outPath.c_str());
                    cout << "  saved: " << outPath << endl;
                }

                cout << "  avg: " << avg << " ms, Sp: " << sp << (ok ? "" : "  <-- MISMATCH") << endl;
                csv << ds << ',' << '"' << inPath << '"' << ',' << '"' << resStr << '"' << ','
                    << '"' << impl.name << '"' << ',' << th << ',' << Ksize << ','
                    << avg << ',' << sp << ',' << (ok ? "OK" : "FAIL") << '\n';
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

    ofstream csv("results_4_2.csv");
    if (!csv.is_open()) {
        cerr << "Cannot open results_4_2.csv" << endl;
        return 1;
    }
    csv << "Dataset,Image,Resolution,Algorithm,Threads,Ksize,Time_ms,Speedup,Check\n";

    cout << "Runs: " << runs << ", OpenMP max threads: " << omp_get_max_threads()
         << ", oneTBB default threads: " << tbb::info::default_concurrency() << endl;

    for (int ds = 1; ds <= 3; ds++)
        RunDataset(ds, "input_" + to_string(ds) + ".bmp", runs, csv);

    csv.close();
    cout << "\nResults written to results_4_2.csv" << endl;
    return 0;
}

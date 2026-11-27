// Использование: 4.3 [runs] [k1] [k2] [ksize]
//   runs  - число запусков (по умолчанию 10)
//   k1    - нижний порог из [0.25, 0.4] (по умолчанию 0.3)
//   k2    - верхний порог из [0.6, 0.75] (по умолчанию 0.7)
//   ksize - размер окрестности: 5, 7, 9 или 0 для прогона всех трёх (по умолчанию 0)
// Обрабатывает input_1.bmp и input_2.bmp, сохраняет карты output_X_ver_size.bmp и results_4_3.csv.
//
// Механизмы распараллеливания (везде по внешнему циклу строк):
//   - seq: последовательный расчёт и построение карт.
//   - OpenMP: parallel for + parallel for reduction(min, max).
//   - oneTBB: parallel_for + parallel_reduce (класс MinMaxReducer).
//   - oneTBB (fused): parallel_for + совмещённый parallel_reduce (класс FusedFeaturesReducer,
//     расчёт признаков и поиск min/max за один проход).

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
#include <cfloat>
#include <omp.h>
#include <tbb/tbb.h>
#include "bmp.h"

using namespace std;

// MARK: Вспомогательные буферы и структуры
template <typename T>
static T** Alloc2D(int W, int H) {
    T** m = new T*[H];
    m[0] = new T[(size_t)W * H];
    for (int y = 1; y < H; y++) m[y] = m[0] + (size_t)y * W;
    return m;
}

template <typename T>
static void Free2D(T** m) {
    delete[] m[0];
    delete[] m;
}

struct MinMax4 {
    double minM2 = DBL_MAX, maxM2 = -DBL_MAX;
    double minR  = DBL_MAX, maxR  = -DBL_MAX;
    double minU  = DBL_MAX, maxU  = -DBL_MAX;
    double minE  = DBL_MAX, maxE  = -DBL_MAX;

    void Update(double m2, double r, double u, double e) {
        minM2 = min(minM2, m2); maxM2 = max(maxM2, m2);
        minR  = min(minR,  r);  maxR  = max(maxR,  r);
        minU  = min(minU,  u);  maxU  = max(maxU,  u);
        minE  = min(minE,  e);  maxE  = max(maxE,  e);
    }

    void Merge(const MinMax4& o) {
        minM2 = min(minM2, o.minM2); maxM2 = max(maxM2, o.maxM2);
        minR  = min(minR,  o.minR);  maxR  = max(maxR,  o.maxR);
        minU  = min(minU,  o.minU);  maxU  = max(maxU,  o.maxU);
        minE  = min(minE,  o.minE);  maxE  = max(maxE,  o.maxE);
    }
};

struct Thresholds4 {
    double t1M2, t2M2, t1R, t2R, t1U, t2U, t1E, t2E;
};

static Thresholds4 CalcThresholds(const MinMax4& mm, double k1, double k2) {
    return {
        (mm.maxM2 - mm.minM2) * k1 + mm.minM2, (mm.maxM2 - mm.minM2) * k2 + mm.minM2,
        (mm.maxR  - mm.minR)  * k1 + mm.minR,  (mm.maxR  - mm.minR)  * k2 + mm.minR,
        (mm.maxU  - mm.minU)  * k1 + mm.minU,  (mm.maxU  - mm.minU)  * k2 + mm.minU,
        (mm.maxE  - mm.minE)  * k1 + mm.minE,  (mm.maxE  - mm.minE)  * k2 + mm.minE
    };
}

struct WorkBufs {
    int W, H;
    uint8_t** bimg;
    double** M2; double** R; double** U; double** E;
    RGBTRIPLE** mapM2; RGBTRIPLE** mapR; RGBTRIPLE** mapU; RGBTRIPLE** mapE;

    WorkBufs(int w, int h) : W(w), H(h) {
        bimg  = Alloc2D<uint8_t>(W, H);
        M2    = Alloc2D<double>(W, H); R    = Alloc2D<double>(W, H);
        U     = Alloc2D<double>(W, H); E    = Alloc2D<double>(W, H);
        mapM2 = AllocImage(W, H);      mapR = AllocImage(W, H);
        mapU  = AllocImage(W, H);      mapE = AllocImage(W, H);
    }

    ~WorkBufs() {
        Free2D(bimg); Free2D(M2); Free2D(R); Free2D(U); Free2D(E);
        FreeImage(mapM2); FreeImage(mapR); FreeImage(mapU); FreeImage(mapE);
    }

    void ClearMaps() {
        size_t bytes = (size_t)W * H * sizeof(RGBTRIPLE);
        memset(mapM2[0], 0, bytes); memset(mapR[0], 0, bytes);
        memset(mapU[0],  0, bytes); memset(mapE[0], 0, bytes);
    }
};

// MARK: Построчные операции (общие для всех реализаций)
static inline RGBTRIPLE ColorByThreshold(double v, double t1, double t2) {
    if (v <= t1) return { 0, 255, 0 };   // зелёный (BGR)
    if (v < t2)  return { 0, 255, 255 }; // жёлтый  (BGR)
    return { 0, 0, 255 };                // красный (BGR)
}

static void GrayscaleRow(RGBTRIPLE** in, uint8_t** bimg, int W, int y) {
    for (int x = 0; x < W; x++) {
        const RGBTRIPLE& p = in[y][x];
        int val = (int)(p.rgbtRed * 0.299 + p.rgbtGreen * 0.587 + p.rgbtBlue * 0.114);
        bimg[y][x] = (uint8_t)(val > 255 ? 255 : val);
    }
}

static inline void CalcPixelFeatures(uint8_t** bimg, int W, int H, int rad, int y, int x, double invSize,
                                     double& outM2, double& outR, double& outU, double& outE) {
    int hist[256] = {};
    for (int dy = -rad; dy <= rad; dy++) {
        int ky = y + dy;
        if (ky < 0) ky = 0;
        if (ky > H - 1) ky = H - 1;
        const uint8_t* row = bimg[ky];
        for (int dx = -rad; dx <= rad; dx++) {
            int kx = x + dx;
            if (kx < 0) kx = 0;
            if (kx > W - 1) kx = W - 1;
            hist[row[kx]]++;
        }
    }

    int sumI = 0;
    for (int i = 0; i < 256; i++)
        sumI += hist[i] * i;
    double m = sumI * invSize;

    double m2 = 0.0, u = 0.0, e = 0.0;
    for (int i = 0; i < 256; i++) {
        if (hist[i] != 0) {
            double p = hist[i] * invSize;
            double d = i - m;
            m2 += d * d * p;
            u  += p * p;
            e  += p * log2(p);
        }
    }
    outM2 = m2;
    outR  = 1.0 - 1.0 / (1.0 + m2);
    outU  = u;
    outE  = (e == 0.0) ? 0.0 : -e;
}

static void FeaturesRow(WorkBufs& wb, int rad, int y) {
    double invSize = 1.0 / ((2 * rad + 1) * (2 * rad + 1));
    for (int x = 0; x < wb.W; x++)
        CalcPixelFeatures(wb.bimg, wb.W, wb.H, rad, y, x, invSize,
                          wb.M2[y][x], wb.R[y][x], wb.U[y][x], wb.E[y][x]);
}

static void FeaturesRowMinMax(WorkBufs& wb, int rad, int y, MinMax4& mm) {
    double invSize = 1.0 / ((2 * rad + 1) * (2 * rad + 1));
    for (int x = 0; x < wb.W; x++) {
        double m2, r, u, e;
        CalcPixelFeatures(wb.bimg, wb.W, wb.H, rad, y, x, invSize, m2, r, u, e);
        wb.M2[y][x] = m2; wb.R[y][x] = r; wb.U[y][x] = u; wb.E[y][x] = e;
        mm.Update(m2, r, u, e);
    }
}

static void BuildMapsRow(WorkBufs& wb, const Thresholds4& t, int y) {
    for (int x = 0; x < wb.W; x++) {
        wb.mapM2[y][x] = ColorByThreshold(wb.M2[y][x], t.t1M2, t.t2M2);
        wb.mapR[y][x]  = ColorByThreshold(wb.R[y][x],  t.t1R,  t.t2R);
        wb.mapU[y][x]  = ColorByThreshold(wb.U[y][x],  t.t1U,  t.t2U);
        wb.mapE[y][x]  = ColorByThreshold(wb.E[y][x],  t.t1E,  t.t2E);
    }
}

// MARK: 1. Последовательная реализация
static void TextureSeq(RGBTRIPLE** in, WorkBufs& wb, int rad, double k1, double k2) {
    for (int y = 0; y < wb.H; y++) GrayscaleRow(in, wb.bimg, wb.W, y);
    for (int y = 0; y < wb.H; y++) FeaturesRow(wb, rad, y);

    MinMax4 mm;
    for (int y = 0; y < wb.H; y++)
        for (int x = 0; x < wb.W; x++)
            mm.Update(wb.M2[y][x], wb.R[y][x], wb.U[y][x], wb.E[y][x]);

    Thresholds4 t = CalcThresholds(mm, k1, k2);
    for (int y = 0; y < wb.H; y++) BuildMapsRow(wb, t, y);
}

// MARK: 2. Реализация OpenMP
static void TextureOmp(RGBTRIPLE** in, WorkBufs& wb, int rad, double k1, double k2) {
#pragma omp parallel for schedule(static)
    for (int y = 0; y < wb.H; y++) GrayscaleRow(in, wb.bimg, wb.W, y);

#pragma omp parallel for schedule(static)
    for (int y = 0; y < wb.H; y++) FeaturesRow(wb, rad, y);

    double minM2 = DBL_MAX, maxM2 = -DBL_MAX;
    double minR  = DBL_MAX, maxR  = -DBL_MAX;
    double minU  = DBL_MAX, maxU  = -DBL_MAX;
    double minE  = DBL_MAX, maxE  = -DBL_MAX;
#pragma omp parallel for schedule(static) \
    reduction(min:minM2, minR, minU, minE) reduction(max:maxM2, maxR, maxU, maxE)
    for (int y = 0; y < wb.H; y++) {
        for (int x = 0; x < wb.W; x++) {
            double m2 = wb.M2[y][x], r = wb.R[y][x], u = wb.U[y][x], e = wb.E[y][x];
            minM2 = min(minM2, m2); maxM2 = max(maxM2, m2);
            minR  = min(minR,  r);  maxR  = max(maxR,  r);
            minU  = min(minU,  u);  maxU  = max(maxU,  u);
            minE  = min(minE,  e);  maxE  = max(maxE,  e);
        }
    }

    MinMax4 mm = { minM2, maxM2, minR, maxR, minU, maxU, minE, maxE };
    Thresholds4 t = CalcThresholds(mm, k1, k2);

#pragma omp parallel for schedule(static)
    for (int y = 0; y < wb.H; y++) BuildMapsRow(wb, t, y);
}

// MARK: 3. Реализация Intel oneTBB (класс редукции для поиска min/max)
class MinMaxReducer {
    const WorkBufs& wb;
public:
    MinMax4 mm;
    explicit MinMaxReducer(const WorkBufs& wb_) : wb(wb_) {}
    MinMaxReducer(MinMaxReducer& other, tbb::split) : wb(other.wb) {}
    void operator()(const tbb::blocked_range<int>& r) {
        MinMax4 local = mm;
        for (int y = r.begin(); y < r.end(); y++)
            for (int x = 0; x < wb.W; x++)
                local.Update(wb.M2[y][x], wb.R[y][x], wb.U[y][x], wb.E[y][x]);
        mm = local;
    }
    void join(const MinMaxReducer& rhs) { mm.Merge(rhs.mm); }
};

static void TextureTbb(RGBTRIPLE** in, WorkBufs& wb, int rad, double k1, double k2) {
    tbb::parallel_for(tbb::blocked_range<int>(0, wb.H), [&](const tbb::blocked_range<int>& r) {
        for (int y = r.begin(); y < r.end(); y++) GrayscaleRow(in, wb.bimg, wb.W, y);
    });
    tbb::parallel_for(tbb::blocked_range<int>(0, wb.H), [&](const tbb::blocked_range<int>& r) {
        for (int y = r.begin(); y < r.end(); y++) FeaturesRow(wb, rad, y);
    });

    MinMaxReducer red(wb);
    tbb::parallel_reduce(tbb::blocked_range<int>(0, wb.H), red);
    Thresholds4 t = CalcThresholds(red.mm, k1, k2);

    tbb::parallel_for(tbb::blocked_range<int>(0, wb.H), [&](const tbb::blocked_range<int>& r) {
        for (int y = r.begin(); y < r.end(); y++) BuildMapsRow(wb, t, y);
    });
}

// MARK: 4. Доп. вариант: Intel oneTBB совмещённый (признаки + min/max в одном parallel_reduce)
class FusedFeaturesReducer {
    WorkBufs& wb;
    int rad;
public:
    MinMax4 mm;
    FusedFeaturesReducer(WorkBufs& wb_, int rad_) : wb(wb_), rad(rad_) {}
    FusedFeaturesReducer(FusedFeaturesReducer& other, tbb::split) : wb(other.wb), rad(other.rad) {}
    void operator()(const tbb::blocked_range<int>& r) {
        MinMax4 local = mm;
        for (int y = r.begin(); y < r.end(); y++)
            FeaturesRowMinMax(wb, rad, y, local);
        mm = local;
    }
    void join(const FusedFeaturesReducer& rhs) { mm.Merge(rhs.mm); }
};

static void TextureTbbFused(RGBTRIPLE** in, WorkBufs& wb, int rad, double k1, double k2) {
    tbb::parallel_for(tbb::blocked_range<int>(0, wb.H), [&](const tbb::blocked_range<int>& r) {
        for (int y = r.begin(); y < r.end(); y++) GrayscaleRow(in, wb.bimg, wb.W, y);
    });

    FusedFeaturesReducer red(wb, rad);
    tbb::parallel_reduce(tbb::blocked_range<int>(0, wb.H), red);
    Thresholds4 t = CalcThresholds(red.mm, k1, k2);

    tbb::parallel_for(tbb::blocked_range<int>(0, wb.H), [&](const tbb::blocked_range<int>& r) {
        for (int y = r.begin(); y < r.end(); y++) BuildMapsRow(wb, t, y);
    });
}

// MARK: Статистика и замер
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

// MARK: Тестирование и сохранение карт
struct TexImpl {
    string code;
    string name;
    Backend bk;
    void (*fn)(RGBTRIPLE**, WorkBufs&, int, double, double);
};

static void SaveMaps(int ds, const string& ver, int ksize, WorkBufs& wb) {
    string suf = "_" + to_string(ksize) + ".bmp";
    string pfx = "output_" + to_string(ds) + "_" + ver + "_";
    BMPWrite(wb.mapM2, wb.W, wb.H, (pfx + "M2" + suf).c_str());
    BMPWrite(wb.mapR,  wb.W, wb.H, (pfx + "R"  + suf).c_str());
    BMPWrite(wb.mapU,  wb.W, wb.H, (pfx + "U"  + suf).c_str());
    BMPWrite(wb.mapE,  wb.W, wb.H, (pfx + "E"  + suf).c_str());
    cout << "  saved: " << pfx << "{M2,R,U,E}" << suf << endl;
}

static void RunDataset(int ds, const string& inPath, int runs, double k1, double k2,
                       const vector<int>& ksizes, ofstream& csv) {
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

    WorkBufs wb(W, H);
    const size_t pixels = (size_t)W * H;
    const size_t bytes  = pixels * sizeof(RGBTRIPLE);

    const TexImpl impls[4] = {
        { "seq",       "Texture (seq)",            Backend::Seq, TextureSeq },
        { "omp",       "Texture OpenMP",           Backend::Omp, TextureOmp },
        { "tbb",       "Texture oneTBB",           Backend::Tbb, TextureTbb },
        { "tbb_fused", "Texture oneTBB (fused)",   Backend::Tbb, TextureTbbFused }
    };

    const size_t kCnt = ksizes.size();
    vector<vector<RGBTRIPLE>> refM2(kCnt), refR(kCnt), refU(kCnt), refE(kCnt);
    vector<double> tSeq(kCnt, 0.0);

    for (const auto& impl : impls) {
        int thMin = (impl.bk == Backend::Seq) ? 1 : 2;
        int thMax = (impl.bk == Backend::Seq) ? 1 : 4;
        for (int th = thMin; th <= thMax; th++) {
            for (size_t ki = 0; ki < kCnt; ki++) {
                int ksize = ksizes[ki];
                int rad = ksize / 2;
                wb.ClearMaps();

                cout << impl.name;
                if (th > 1) cout << " [" << th << " threads]";
                cout << " " << ksize << "x" << ksize << "\t" << flush;

                double avg = Measure(impl.bk, th, runs, [&] { impl.fn(in, wb, rad, k1, k2); });

                bool ok = true;
                if (impl.bk == Backend::Seq) {
                    tSeq[ki] = avg;
                    refM2[ki].assign(wb.mapM2[0], wb.mapM2[0] + pixels);
                    refR[ki].assign(wb.mapR[0],   wb.mapR[0]  + pixels);
                    refU[ki].assign(wb.mapU[0],   wb.mapU[0]  + pixels);
                    refE[ki].assign(wb.mapE[0],   wb.mapE[0]  + pixels);
                } else {
                    if (memcmp(wb.mapM2[0], refM2[ki].data(), bytes) != 0 ||
                        memcmp(wb.mapR[0],  refR[ki].data(),  bytes) != 0 ||
                        memcmp(wb.mapU[0],  refU[ki].data(),  bytes) != 0 ||
                        memcmp(wb.mapE[0],  refE[ki].data(),  bytes) != 0)
                        ok = false;
                }
                double sp = (avg > 0.0) ? (tSeq[ki] / avg) : 0.0;

                // Сохраняем карты для seq по всем размерам и для параллельных версий при th=4 на первом размере
                if (impl.bk == Backend::Seq || (th == 4 && ki == 0))
                    SaveMaps(ds, impl.code, ksize, wb);

                cout << "  avg: " << avg << " ms, Sp: " << sp << (ok ? "" : "  <-- MISMATCH") << endl;
                csv << ds << ',' << '"' << inPath << '"' << ',' << '"' << resStr << '"' << ','
                    << '"' << impl.name << '"' << ',' << th << ',' << ksize << ','
                    << avg << ',' << sp << ',' << (ok ? "OK" : "FAIL") << '\n';
                csv.flush();
            }
        }
    }

    FreeImage(in);
}

int main(int argc, char** argv) {
    int runs     = (argc > 1) ? atoi(argv[1]) : 10;
    double k1    = (argc > 2) ? atof(argv[2]) : 0.3;
    double k2    = (argc > 3) ? atof(argv[3]) : 0.7;
    int ksizeArg = (argc > 4) ? atoi(argv[4]) : 0;

    if (runs < 1) runs = 1;
    if (k1 < 0.25 || k1 > 0.4) {
        cerr << "Warning: k1 clamped to [0.25, 0.4]" << endl;
        k1 = max(0.25, min(0.4, k1));
    }
    if (k2 < 0.6 || k2 > 0.75) {
        cerr << "Warning: k2 clamped to [0.6, 0.75]" << endl;
        k2 = max(0.6, min(0.75, k2));
    }

    vector<int> ksizes;
    if (ksizeArg == 5 || ksizeArg == 7 || ksizeArg == 9) {
        ksizes = { ksizeArg };
    } else {
        if (ksizeArg != 0) cerr << "Warning: ksize must be 5, 7, 9 or 0; running {5, 7, 9}" << endl;
        ksizes = { 5, 7, 9 };
    }

    ofstream csv("results_4_3.csv");
    if (!csv.is_open()) {
        cerr << "Cannot open results_4_3.csv" << endl;
        return 1;
    }
    csv << "Dataset,Image,Resolution,Algorithm,Threads,Ksize,Time_ms,Speedup,Check\n";

    cout << "Runs: " << runs << ", k1: " << k1 << ", k2: " << k2
         << ", OpenMP max threads: " << omp_get_max_threads()
         << ", oneTBB default threads: " << tbb::info::default_concurrency() << endl;

    for (int ds = 1; ds <= 2; ds++)
        RunDataset(ds, "input_" + to_string(ds) + ".bmp", runs, k1, k2, ksizes, csv);

    csv.close();
    cout << "\nResults written to results_4_3.csv" << endl;
    return 0;
}

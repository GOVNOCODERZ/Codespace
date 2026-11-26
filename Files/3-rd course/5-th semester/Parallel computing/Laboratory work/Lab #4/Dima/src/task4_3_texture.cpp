#include "BMPFileRW.h"
#include "common_stats.h"

#include <Windows.h>
#include <omp.h>
#include <oneapi/tbb.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

static size_t utf8Len(const std::string& s) {
    size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0u) != 0x80u) count++;
    }
    return count;
}
static std::string padRightU8(const std::string& s, size_t w) {
    size_t l = utf8Len(s);
    return (l < w) ? (s + std::string(w - l, ' ')) : s;
}
static std::string padLeftU8(const std::string& s, size_t w) {
    size_t l = utf8Len(s);
    return (l < w) ? (std::string(w - l, ' ') + s) : s;
}

// ============================================================================
// Структуры для хранения текстурных признаков и порогов визуализации
// ============================================================================
struct TextureArrays {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> gray;
    std::vector<double> m2;
    std::vector<double> r;
    std::vector<double> u;
    std::vector<double> e;
    std::vector<RGBTRIPLE> mapM2;
    std::vector<RGBTRIPLE> mapR;
    std::vector<RGBTRIPLE> mapU;
    std::vector<RGBTRIPLE> mapE;

    void allocate(int w, int h) {
        width = w;
        height = h;
        size_t sz = static_cast<size_t>(w) * static_cast<size_t>(h);
        gray.resize(sz);
        m2.resize(sz);
        r.resize(sz);
        u.resize(sz);
        e.resize(sz);
        mapM2.resize(sz);
        mapR.resize(sz);
        mapU.resize(sz);
        mapE.resize(sz);
    }

    bool mapsEqual(const TextureArrays& o) const {
        if (width != o.width || height != o.height) return false;
        size_t bytes = mapM2.size() * sizeof(RGBTRIPLE);
        return std::memcmp(mapM2.data(), o.mapM2.data(), bytes) == 0 &&
               std::memcmp(mapR.data(), o.mapR.data(), bytes) == 0 &&
               std::memcmp(mapU.data(), o.mapU.data(), bytes) == 0 &&
               std::memcmp(mapE.data(), o.mapE.data(), bytes) == 0;
    }
};

struct MinMaxAll {
    double minM2 = std::numeric_limits<double>::max();
    double maxM2 = std::numeric_limits<double>::lowest();
    double minR  = std::numeric_limits<double>::max();
    double maxR  = std::numeric_limits<double>::lowest();
    double minU  = std::numeric_limits<double>::max();
    double maxU  = std::numeric_limits<double>::lowest();
    double minE  = std::numeric_limits<double>::max();
    double maxE  = std::numeric_limits<double>::lowest();
};

// Раскраска пикселя карты текстурного признака по двум порогам T1, T2:
//   TF <= T1           -> зелёный
//   T1 < TF < T2       -> жёлтый
//   TF >= T2           -> красный
static inline RGBTRIPLE colorByThreshold(double val, double t1, double t2) {
    if (val <= t1) {
        return { 0, 200, 0 };     // BGR: зелёный
    } else if (val < t2) {
        return { 0, 220, 255 };   // BGR: жёлтый
    } else {
        return { 30, 30, 220 };   // BGR: красный
    }
}

// Вычисление 4 текстурных признаков (mu2, R, U, e) для одного пикселя (x, y) по локальной гистограмме
static inline void computePixelTextures(const unsigned char* gray, int width, int height,
                                        int x, int y, int rad, int area, double invArea,
                                        const double* pLog2p,
                                        int* histCount, unsigned char* touched,
                                        double& outM2, double& outR, double& outU, double& outE) {
    int nTouched = 0;
    if (y >= rad && y < height - rad && x >= rad && x < width - rad) {
        for (int dy = -rad; dy <= rad; dy++) {
            const unsigned char* row = &gray[static_cast<size_t>(y + dy) * static_cast<size_t>(width) + static_cast<size_t>(x - rad)];
            for (int dx = 0; dx < 2 * rad + 1; dx++) {
                unsigned char v = row[dx];
                if (histCount[v] == 0) {
                    touched[nTouched++] = v;
                }
                histCount[v]++;
            }
        }
    } else {
        for (int dy = -rad; dy <= rad; dy++) {
            int ky = std::clamp(y + dy, 0, height - 1);
            const unsigned char* row = &gray[static_cast<size_t>(ky) * static_cast<size_t>(width)];
            for (int dx = -rad; dx <= rad; dx++) {
                int kx = std::clamp(x + dx, 0, width - 1);
                unsigned char v = row[kx];
                if (histCount[v] == 0) {
                    touched[nTouched++] = v;
                }
                histCount[v]++;
            }
        }
    }

    // Проход 1: средняя яркость m по ненулевым элементам гистограммы
    double m = 0.0;
    for (int k = 0; k < nTouched; k++) {
        unsigned char v = touched[k];
        m += (histCount[v] * invArea) * static_cast<double>(v);
    }

    // Проход 2: центральный момент 2-го порядка m2, однородность u, энтропия e; обнуление затронутых бинов
    double m2 = 0.0;
    double u = 0.0;
    double e = 0.0;
    for (int k = 0; k < nTouched; k++) {
        unsigned char v = touched[k];
        int c = histCount[v];
        histCount[v] = 0;
        double p = c * invArea;
        double diff = static_cast<double>(v) - m;
        m2 += diff * diff * p;
        u += p * p;
        e -= pLog2p[c];
    }

    (void)area;
    outM2 = m2;
    outR = 1.0 - (1.0 / (1.0 + m2));
    outU = u;
    outE = e;
}

static std::vector<double> buildEntropyLUT(int area) {
    std::vector<double> lut(static_cast<size_t>(area) + 1, 0.0);
    double inv = 1.0 / static_cast<double>(area);
    for (int c = 1; c <= area; c++) {
        double p = c * inv;
        lut[static_cast<size_t>(c)] = p * std::log2(p);
    }
    return lut;
}

// ============================================================================
// Класс редукции Intel oneTBB для одновременного поиска min/max по M2, R, U, E
// (рекомендовано на стр. 8–9 методических указаний)
// ============================================================================
class TextureMinMaxReducer {
    const double* m2_;
    const double* r_;
    const double* u_;
    const double* e_;
public:
    MinMaxAll mm;

    TextureMinMaxReducer(const double* m2, const double* r, const double* u, const double* e)
        : m2_(m2), r_(r), u_(u), e_(e), mm() {}

    TextureMinMaxReducer(const TextureMinMaxReducer& o, oneapi::tbb::split)
        : m2_(o.m2_), r_(o.r_), u_(o.u_), e_(o.e_), mm() {}

    void operator()(const oneapi::tbb::blocked_range<size_t>& range) {
        MinMaxAll loc = mm;
        for (size_t i = range.begin(); i < range.end(); i++) {
            double vm = m2_[i], vr = r_[i], vu = u_[i], ve = e_[i];
            if (vm < loc.minM2) loc.minM2 = vm;
            if (vm > loc.maxM2) loc.maxM2 = vm;
            if (vr < loc.minR)  loc.minR  = vr;
            if (vr > loc.maxR)  loc.maxR  = vr;
            if (vu < loc.minU)  loc.minU  = vu;
            if (vu > loc.maxU)  loc.maxU  = vu;
            if (ve < loc.minE)  loc.minE  = ve;
            if (ve > loc.maxE)  loc.maxE  = ve;
        }
        mm = loc;
    }

    void join(const TextureMinMaxReducer& rhs) {
        if (rhs.mm.minM2 < mm.minM2) mm.minM2 = rhs.mm.minM2;
        if (rhs.mm.maxM2 > mm.maxM2) mm.maxM2 = rhs.mm.maxM2;
        if (rhs.mm.minR  < mm.minR)  mm.minR  = rhs.mm.minR;
        if (rhs.mm.maxR  > mm.maxR)  mm.maxR  = rhs.mm.maxR;
        if (rhs.mm.minU  < mm.minU)  mm.minU  = rhs.mm.minU;
        if (rhs.mm.maxU  > mm.maxU)  mm.maxU  = rhs.mm.maxU;
        if (rhs.mm.minE  < mm.minE)  mm.minE  = rhs.mm.minE;
        if (rhs.mm.maxE  > mm.maxE)  mm.maxE  = rhs.mm.maxE;
    }
};

// ============================================================================
// 1. Последовательная реализация расчёта признаков и построения карт
// ============================================================================
static void computeTexturesSeq(RGBTRIPLE** srcImg, TextureArrays& ta, int ksize, double k1, double k2) {
    const int W = ta.width;
    const int H = ta.height;
    const int rad = ksize / 2;
    const int area = ksize * ksize;
    const double invArea = 1.0 / static_cast<double>(area);
    const size_t total = static_cast<size_t>(W) * static_cast<size_t>(H);
    const std::vector<double> pLog2p = buildEntropyLUT(area);

    // Цикл А: преобразование RGB -> матрица яркости BIMAGE
    for (int y = 0; y < H; y++) {
        const RGBTRIPLE* row = srcImg[y];
        size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
        for (int x = 0; x < W; x++) {
            double lum = 0.299 * row[x].rgbtRed + 0.587 * row[x].rgbtGreen + 0.114 * row[x].rgbtBlue;
            ta.gray[base + static_cast<size_t>(x)] = static_cast<unsigned char>(std::clamp(static_cast<int>(lum + 0.5), 0, 255));
        }
    }

    // Цикл Б: гистограмма и расчёт 4 текстурных признаков
    int histCount[256] = { 0 };
    unsigned char touched[81];
    for (int y = 0; y < H; y++) {
        size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
        for (int x = 0; x < W; x++) {
            size_t idx = base + static_cast<size_t>(x);
            computePixelTextures(ta.gray.data(), W, H, x, y, rad, area, invArea, pLog2p.data(),
                                 histCount, touched, ta.m2[idx], ta.r[idx], ta.u[idx], ta.e[idx]);
        }
    }

    // Шаг 1: определение минимумов и максимумов за один проход
    MinMaxAll mm;
    for (size_t i = 0; i < total; i++) {
        if (ta.m2[i] < mm.minM2) mm.minM2 = ta.m2[i];
        if (ta.m2[i] > mm.maxM2) mm.maxM2 = ta.m2[i];
        if (ta.r[i]  < mm.minR)  mm.minR  = ta.r[i];
        if (ta.r[i]  > mm.maxR)  mm.maxR  = ta.r[i];
        if (ta.u[i]  < mm.minU)  mm.minU  = ta.u[i];
        if (ta.u[i]  > mm.maxU)  mm.maxU  = ta.u[i];
        if (ta.e[i]  < mm.minE)  mm.minE  = ta.e[i];
        if (ta.e[i]  > mm.maxE)  mm.maxE  = ta.e[i];
    }

    // Шаг 2: расчёт порогов T1, T2
    double t1M2 = (mm.maxM2 - mm.minM2) * k1 + mm.minM2, t2M2 = (mm.maxM2 - mm.minM2) * k2 + mm.minM2;
    double t1R  = (mm.maxR  - mm.minR)  * k1 + mm.minR,  t2R  = (mm.maxR  - mm.minR)  * k2 + mm.minR;
    double t1U  = (mm.maxU  - mm.minU)  * k1 + mm.minU,  t2U  = (mm.maxU  - mm.minU)  * k2 + mm.minU;
    double t1E  = (mm.maxE  - mm.minE)  * k1 + mm.minE,  t2E  = (mm.maxE  - mm.minE)  * k2 + mm.minE;

    // Шаг 3: формирование карт текстурных признаков
    for (size_t i = 0; i < total; i++) {
        ta.mapM2[i] = colorByThreshold(ta.m2[i], t1M2, t2M2);
        ta.mapR[i]  = colorByThreshold(ta.r[i],  t1R,  t2R);
        ta.mapU[i]  = colorByThreshold(ta.u[i],  t1U,  t2U);
        ta.mapE[i]  = colorByThreshold(ta.e[i],  t1E,  t2E);
    }
}

// ============================================================================
// 2. Параллельная реализация OpenMP (parallel for + reduction(min/max))
// ============================================================================
static void computeTexturesOpenMP(RGBTRIPLE** srcImg, TextureArrays& ta, int ksize, double k1, double k2, int numThreads) {
    const int W = ta.width;
    const int H = ta.height;
    const int rad = ksize / 2;
    const int area = ksize * ksize;
    const double invArea = 1.0 / static_cast<double>(area);
    const size_t total = static_cast<size_t>(W) * static_cast<size_t>(H);
    const std::vector<double> pLog2p = buildEntropyLUT(area);

    double minM2 = std::numeric_limits<double>::max(), maxM2 = std::numeric_limits<double>::lowest();
    double minR  = std::numeric_limits<double>::max(), maxR  = std::numeric_limits<double>::lowest();
    double minU  = std::numeric_limits<double>::max(), maxU  = std::numeric_limits<double>::lowest();
    double minE  = std::numeric_limits<double>::max(), maxE  = std::numeric_limits<double>::lowest();

    #pragma omp parallel num_threads(numThreads) default(none) shared(srcImg, ta, W, H, rad, area, invArea, pLog2p, total, k1, k2, minM2, maxM2, minR, maxR, minU, maxU, minE, maxE)
    {
        // Цикл А: вычисление яркости BIMAGE
        #pragma omp for schedule(static)
        for (int y = 0; y < H; y++) {
            const RGBTRIPLE* row = srcImg[y];
            size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
            for (int x = 0; x < W; x++) {
                double lum = 0.299 * row[x].rgbtRed + 0.587 * row[x].rgbtGreen + 0.114 * row[x].rgbtBlue;
                ta.gray[base + static_cast<size_t>(x)] = static_cast<unsigned char>(std::clamp(static_cast<int>(lum + 0.5), 0, 255));
            }
        }

        // Цикл Б: гистограмма и 4 текстурных признака (приватная гистограмма на каждый поток)
        int histCount[256] = { 0 };
        unsigned char touched[81];
        #pragma omp for schedule(dynamic, 8)
        for (int y = 0; y < H; y++) {
            size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
            for (int x = 0; x < W; x++) {
                size_t idx = base + static_cast<size_t>(x);
                computePixelTextures(ta.gray.data(), W, H, x, y, rad, area, invArea, pLog2p.data(),
                                     histCount, touched, ta.m2[idx], ta.r[idx], ta.u[idx], ta.e[idx]);
            }
        }

        // Шаг 1: параллельная редукция минимумов и максимумов
        #pragma omp for reduction(min:minM2,minR,minU,minE) reduction(max:maxM2,maxR,maxU,maxE) schedule(static)
        for (size_t i = 0; i < total; i++) {
            if (ta.m2[i] < minM2) minM2 = ta.m2[i];
            if (ta.m2[i] > maxM2) maxM2 = ta.m2[i];
            if (ta.r[i]  < minR)  minR  = ta.r[i];
            if (ta.r[i]  > maxR)  maxR  = ta.r[i];
            if (ta.u[i]  < minU)  minU  = ta.u[i];
            if (ta.u[i]  > maxU)  maxU  = ta.u[i];
            if (ta.e[i]  < minE)  minE  = ta.e[i];
            if (ta.e[i]  > maxE)  maxE  = ta.e[i];
        }

        // Шаги 2–3: пороги и параллельное построение цветовых карт
        double t1M2 = (maxM2 - minM2) * k1 + minM2, t2M2 = (maxM2 - minM2) * k2 + minM2;
        double t1R  = (maxR  - minR)  * k1 + minR,  t2R  = (maxR  - minR)  * k2 + minR;
        double t1U  = (maxU  - minU)  * k1 + minU,  t2U  = (maxU  - minU)  * k2 + minU;
        double t1E  = (maxE  - minE)  * k1 + minE,  t2E  = (maxE  - minE)  * k2 + minE;

        #pragma omp for schedule(static)
        for (size_t i = 0; i < total; i++) {
            ta.mapM2[i] = colorByThreshold(ta.m2[i], t1M2, t2M2);
            ta.mapR[i]  = colorByThreshold(ta.r[i],  t1R,  t2R);
            ta.mapU[i]  = colorByThreshold(ta.u[i],  t1U,  t2U);
            ta.mapE[i]  = colorByThreshold(ta.e[i],  t1E,  t2E);
        }
    }
}

// ============================================================================
// 3. Параллельная реализация Intel oneTBB (1D blocked_range + класс редукции)
// ============================================================================
static void computeTexturesTBB1D(RGBTRIPLE** srcImg, TextureArrays& ta, int ksize, double k1, double k2, int numThreads) {
    const int W = ta.width;
    const int H = ta.height;
    const int rad = ksize / 2;
    const int area = ksize * ksize;
    const double invArea = 1.0 / static_cast<double>(area);
    const size_t total = static_cast<size_t>(W) * static_cast<size_t>(H);
    const std::vector<double> pLog2p = buildEntropyLUT(area);

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        // Цикл А: RGB -> яркость
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H, 16),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int y = r.begin(); y < r.end(); y++) {
                    const RGBTRIPLE* row = srcImg[y];
                    size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
                    for (int x = 0; x < W; x++) {
                        double lum = 0.299 * row[x].rgbtRed + 0.587 * row[x].rgbtGreen + 0.114 * row[x].rgbtBlue;
                        ta.gray[base + static_cast<size_t>(x)] = static_cast<unsigned char>(std::clamp(static_cast<int>(lum + 0.5), 0, 255));
                    }
                }
            });

        // Цикл Б: гистограмма и 4 текстурных признака (1D построчный parallel_for)
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H, 8),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                int histCount[256] = { 0 };
                unsigned char touched[81];
                for (int y = r.begin(); y < r.end(); y++) {
                    size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
                    for (int x = 0; x < W; x++) {
                        size_t idx = base + static_cast<size_t>(x);
                        computePixelTextures(ta.gray.data(), W, H, x, y, rad, area, invArea, pLog2p.data(),
                                             histCount, touched, ta.m2[idx], ta.r[idx], ta.u[idx], ta.e[idx]);
                    }
                }
            });

        // Шаг 1: редукция min/max через класс TextureMinMaxReducer
        TextureMinMaxReducer reducer(ta.m2.data(), ta.r.data(), ta.u.data(), ta.e.data());
        oneapi::tbb::parallel_reduce(oneapi::tbb::blocked_range<size_t>(0, total, 16384), reducer);
        const MinMaxAll& mm = reducer.mm;

        // Шаги 2–3: пороги и построение карт
        double t1M2 = (mm.maxM2 - mm.minM2) * k1 + mm.minM2, t2M2 = (mm.maxM2 - mm.minM2) * k2 + mm.minM2;
        double t1R  = (mm.maxR  - mm.minR)  * k1 + mm.minR,  t2R  = (mm.maxR  - mm.minR)  * k2 + mm.minR;
        double t1U  = (mm.maxU  - mm.minU)  * k1 + mm.minU,  t2U  = (mm.maxU  - mm.minU)  * k2 + mm.minU;
        double t1E  = (mm.maxE  - mm.minE)  * k1 + mm.minE,  t2E  = (mm.maxE  - mm.minE)  * k2 + mm.minE;

        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, total, 16384),
            [&](const oneapi::tbb::blocked_range<size_t>& r) {
                for (size_t i = r.begin(); i < r.end(); i++) {
                    ta.mapM2[i] = colorByThreshold(ta.m2[i], t1M2, t2M2);
                    ta.mapR[i]  = colorByThreshold(ta.r[i],  t1R,  t2R);
                    ta.mapU[i]  = colorByThreshold(ta.u[i],  t1U,  t2U);
                    ta.mapE[i]  = colorByThreshold(ta.e[i],  t1E,  t2E);
                }
            });
    });
}

// ============================================================================
// 4. Дополнительная реализация Intel oneTBB: 2D-тайлинг blocked_range2d<int>
//    (улучшает 2D-локальность кэша L1/L2 для скользящего окна)
// ============================================================================
static void computeTexturesTBB2D(RGBTRIPLE** srcImg, TextureArrays& ta, int ksize, double k1, double k2, int numThreads) {
    const int W = ta.width;
    const int H = ta.height;
    const int rad = ksize / 2;
    const int area = ksize * ksize;
    const double invArea = 1.0 / static_cast<double>(area);
    const size_t total = static_cast<size_t>(W) * static_cast<size_t>(H);
    const std::vector<double> pLog2p = buildEntropyLUT(area);

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H, 16),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int y = r.begin(); y < r.end(); y++) {
                    const RGBTRIPLE* row = srcImg[y];
                    size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
                    for (int x = 0; x < W; x++) {
                        double lum = 0.299 * row[x].rgbtRed + 0.587 * row[x].rgbtGreen + 0.114 * row[x].rgbtBlue;
                        ta.gray[base + static_cast<size_t>(x)] = static_cast<unsigned char>(std::clamp(static_cast<int>(lum + 0.5), 0, 255));
                    }
                }
            });

        // 2D-декомпозиция тайлами 32x64 через blocked_range2d<int>
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range2d<int>(0, H, 32, 0, W, 64),
            [&](const oneapi::tbb::blocked_range2d<int>& tile) {
                int histCount[256] = { 0 };
                unsigned char touched[81];
                for (int y = tile.rows().begin(); y < tile.rows().end(); y++) {
                    size_t base = static_cast<size_t>(y) * static_cast<size_t>(W);
                    for (int x = tile.cols().begin(); x < tile.cols().end(); x++) {
                        size_t idx = base + static_cast<size_t>(x);
                        computePixelTextures(ta.gray.data(), W, H, x, y, rad, area, invArea, pLog2p.data(),
                                             histCount, touched, ta.m2[idx], ta.r[idx], ta.u[idx], ta.e[idx]);
                    }
                }
            });

        TextureMinMaxReducer reducer(ta.m2.data(), ta.r.data(), ta.u.data(), ta.e.data());
        oneapi::tbb::parallel_reduce(oneapi::tbb::blocked_range<size_t>(0, total, 16384), reducer);
        const MinMaxAll& mm = reducer.mm;

        double t1M2 = (mm.maxM2 - mm.minM2) * k1 + mm.minM2, t2M2 = (mm.maxM2 - mm.minM2) * k2 + mm.minM2;
        double t1R  = (mm.maxR  - mm.minR)  * k1 + mm.minR,  t2R  = (mm.maxR  - mm.minR)  * k2 + mm.minR;
        double t1U  = (mm.maxU  - mm.minU)  * k1 + mm.minU,  t2U  = (mm.maxU  - mm.minU)  * k2 + mm.minU;
        double t1E  = (mm.maxE  - mm.minE)  * k1 + mm.minE,  t2E  = (mm.maxE  - mm.minE)  * k2 + mm.minE;

        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, total, 16384),
            [&](const oneapi::tbb::blocked_range<size_t>& r) {
                for (size_t i = r.begin(); i < r.end(); i++) {
                    ta.mapM2[i] = colorByThreshold(ta.m2[i], t1M2, t2M2);
                    ta.mapR[i]  = colorByThreshold(ta.r[i],  t1R,  t2R);
                    ta.mapU[i]  = colorByThreshold(ta.u[i],  t1U,  t2U);
                    ta.mapE[i]  = colorByThreshold(ta.e[i],  t1E,  t2E);
                }
            });
    });
}

static void saveFlatRGBBMP(const std::vector<RGBTRIPLE>& flat, int W, int H, const std::string& path) {
    RGBTRIPLE** rgb = BMPAlloc(W, H);
    std::memcpy(rgb[0], flat.data(), flat.size() * sizeof(RGBTRIPLE));
    BMPWrite(rgb, W, H, path.c_str());
    BMPFree(rgb);
}

struct TexRow {
    int imgId = 0;
    std::string res;
    std::string impl;
    int ksize = 0;
    int threads = 1;
    int runs = 0;
    TrustedStats stats;
    double speedup = 1.0;
    double efficiency = 1.0;
    double costMs = 0.0;
};

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    double k1 = 0.30; // k1 из диапазона [0.25, 0.4]
    double k2 = 0.70; // k2 из диапазона [0.6, 0.75]
    for (int i = 1; i + 1 < argc; i++) {
        if (std::string(argv[i]) == "--k1") k1 = std::stod(argv[++i]);
        else if (std::string(argv[i]) == "--k2") k2 = std::stod(argv[++i]);
    }
    k1 = std::clamp(k1, 0.25, 0.40);
    k2 = std::clamp(k2, 0.60, 0.75);

    std::cout << "====================================================================================================\n";
    std::cout << "  Лабораторная работа №4 — Задание 4.3: Расчёт текстурных признаков (OpenMP и Intel oneTBB)\n";
    std::cout << "====================================================================================================\n";
    std::cout << "Стандарт OpenMP:         " << _OPENMP << "\n";
    std::cout << "Intel oneTBB потоков:    " << oneapi::tbb::info::default_concurrency() << "\n";
    std::cout << "Пороговые коэффициенты:  k1 = " << std::fixed << std::setprecision(2) << k1
              << " (из [0.25; 0.40]), k2 = " << k2 << " (из [0.60; 0.75])\n";
    std::cout << "Фильтрация выбросов:     AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n";
    std::cout << "Признаки:                M2 (момент 2-го порядка), R (гладкость), U (однородность), E (энтропия)\n\n";

    struct TexInput { int id; std::string path; RGBTRIPLE** rgb = nullptr; int W = 0; int H = 0; };
    std::vector<TexInput> inputs = {
        { 1, "Lab4/images/input_tex_1.bmp", nullptr, 0, 0 },
        { 2, "Lab4/images/input_tex_2.bmp", nullptr, 0, 0 }
    };
    for (auto& inp : inputs) {
        BITMAPFILEHEADER fh{};
        BITMAPINFOHEADER ih{};
        BMPRead(inp.rgb, fh, ih, inp.path.c_str());
        inp.W = ih.biWidth;
        inp.H = ih.biHeight;
        std::cout << "Загружен НД " << inp.id << ": " << inp.path << " (" << inp.W << "x" << inp.H
                  << ", " << (inp.W * inp.H) << " пикс., 24-bit RGB)\n";
    }

    const std::vector<int> kernels = { 5, 7, 9 };
    const std::vector<int> parThreads = { 2, 3, 4 };
    const int RUNS = 15; // >= 10 запусков по методичке

    std::vector<TexRow> allRows;

    for (const auto& inp : inputs) {
        std::cout << "\n--- НД " << inp.id << " (" << inp.W << "x" << inp.H << ") ---\n";
        std::cout << padRightU8("Вид реализации", 22)
                  << padLeftU8("Окно", 6)
                  << padLeftU8("Пот.", 6)
                  << padLeftU8("Зам.", 6)
                  << padLeftU8("Исх.ср.(мс)", 13)
                  << padLeftU8("Отбр.", 7)
                  << padLeftU8("Итог(мс)", 13)
                  << padLeftU8("Sp(n)", 9)
                  << padLeftU8("Ep(n)", 9)
                  << padLeftU8("Cp(мс)", 13)
                  << "\n";
        std::cout << std::string(104, '-') << "\n";

        TextureArrays ta;
        ta.allocate(inp.W, inp.H);
        TextureArrays refTa;
        refTa.allocate(inp.W, inp.H);

        for (int ksize : kernels) {
            computeTexturesSeq(inp.rgb, refTa, ksize, k1, k2);

            auto runCase = [&](const std::string& impl, int p, double t1Ms, auto fn) -> double {
                fn();
                if (!ta.mapsEqual(refTa)) {
                    throw std::runtime_error("Несовпадение карт текстурных признаков: " + impl);
                }
                double* times = new double[RUNS];
                for (int r = 0; r < RUNS; r++) {
                    double t0 = omp_get_wtime();
                    fn();
                    double t1 = omp_get_wtime();
                    times[r] = (t1 - t0) * 1000.0;
                }

                TexRow row;
                row.imgId = inp.id;
                row.res = std::to_string(inp.W) + "x" + std::to_string(inp.H);
                row.impl = impl;
                row.ksize = ksize;
                row.threads = p;
                row.runs = RUNS;
                row.stats = computeTrustedStats(times, RUNS);
                delete[] times;

                double baseT1 = (p == 1) ? row.stats.trustedAvgMed : t1Ms;
                row.speedup = (row.stats.trustedAvgMed > 0.0) ? (baseT1 / row.stats.trustedAvgMed) : 1.0;
                row.efficiency = row.speedup / static_cast<double>(p);
                row.costMs = static_cast<double>(p) * row.stats.trustedAvgMed;

                std::string winStr = std::to_string(ksize) + "x" + std::to_string(ksize);
                std::cout << padRightU8(impl, 22)
                          << std::right << std::fixed
                          << std::setw(6) << winStr
                          << std::setw(6) << p
                          << std::setw(6) << RUNS
                          << std::setw(13) << std::setprecision(4) << row.stats.rawAvg
                          << std::setw(7) << row.stats.discardedRuns
                          << std::setw(13) << std::setprecision(4) << row.stats.trustedAvgMed
                          << std::setw(9) << std::setprecision(4) << row.speedup
                          << std::setw(9) << std::setprecision(4) << row.efficiency
                          << std::setw(13) << std::setprecision(4) << row.costMs
                          << "\n" << std::flush;

                double resT = row.stats.trustedAvgMed;
                allRows.push_back(std::move(row));
                return resT;
            };

            double t1Ms = runCase("Texture_Seq", 1, 0.0, [&]() { computeTexturesSeq(inp.rgb, ta, ksize, k1, k2); });
            for (int p : parThreads) {
                runCase("Texture_OpenMP", p, t1Ms, [&]() { computeTexturesOpenMP(inp.rgb, ta, ksize, k1, k2, p); });
            }
            for (int p : parThreads) {
                runCase("Texture_oneTBB_1D", p, t1Ms, [&]() { computeTexturesTBB1D(inp.rgb, ta, ksize, k1, k2, p); });
            }
            for (int p : parThreads) {
                runCase("Texture_oneTBB_2D", p, t1Ms, [&]() { computeTexturesTBB2D(inp.rgb, ta, ksize, k1, k2, p); });
            }

            // Сохранение 4 карт текстурных признаков (output_X_ver_size_*.bmp)
            std::string prefix = "Lab4/images/output_tex_" + std::to_string(inp.id) + "_tbb_" + std::to_string(ksize);
            saveFlatRGBBMP(ta.mapM2, inp.W, inp.H, prefix + "_M2.bmp");
            saveFlatRGBBMP(ta.mapR,  inp.W, inp.H, prefix + "_R.bmp");
            saveFlatRGBBMP(ta.mapU,  inp.W, inp.H, prefix + "_U.bmp");
            saveFlatRGBBMP(ta.mapE,  inp.W, inp.H, prefix + "_E.bmp");
        }
    }

    for (auto& inp : inputs) {
        BMPFree(inp.rgb);
    }

    std::ofstream out("Lab4/results/task4_3_summary.csv");
    out << "img_id,resolution,impl_name,ksize,threads,runs,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,speedup,efficiency,cost_ms\n";
    out << std::fixed << std::setprecision(6);
    for (const auto& r : allRows) {
        out << r.imgId << "," << r.res << "," << r.impl << "," << r.ksize << "," << r.threads << ","
            << r.runs << "," << r.stats.rawAvg << "," << r.stats.median << "," << r.stats.sd << ","
            << r.stats.keptRuns << "," << r.stats.discardedRuns << ","
            << r.stats.trustedAvgMed << "," << r.speedup << "," << r.efficiency << "," << r.costMs << "\n";
    }
    out.close();

    std::cout << "\nРезультаты задания 4.3 и карты текстурных признаков (M2, R, U, E) сохранены успешно.\n";
    return 0;
}

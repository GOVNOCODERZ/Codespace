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
#include <stdexcept>
#include <string>
#include <vector>

static constexpr double PI_VAL = 3.14159265358979323846;

static size_t utf8Len(const std::string& s) {
    size_t c = 0;
    for (unsigned char ch : s) {
        if ((ch & 0xC0u) != 0x80u) c++;
    }
    return c;
}
static std::string padRightU8(const std::string& s, size_t w) {
    size_t l = utf8Len(s);
    return (l < w) ? (s + std::string(w - l, ' ')) : s;
}
static std::string padLeftU8(const std::string& s, size_t w) {
    size_t l = utf8Len(s);
    return (l < w) ? (std::string(w - l, ' ') + s) : s;
}

static inline unsigned char clampU8(double v) {
    if (v <= 0.0) return 0;
    if (v >= 255.0) return 255;
    return static_cast<unsigned char>(v + 0.5);
}

// ============================================================================
// Планарное представление RGB-изображения (SoA) для высокой локальности кэша
// ============================================================================
struct PlanarRGB {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> r, g, b;

    void allocate(int w, int h) {
        width = w; height = h;
        size_t sz = static_cast<size_t>(w) * static_cast<size_t>(h);
        r.resize(sz); g.resize(sz); b.resize(sz);
    }

    void fromRGBTRIPLE(RGBTRIPLE** src, int w, int h) {
        allocate(w, h);
        for (int y = 0; y < h; y++) {
            const RGBTRIPLE* row = src[y];
            size_t base = static_cast<size_t>(y) * static_cast<size_t>(w);
            for (int x = 0; x < w; x++) {
                r[base + static_cast<size_t>(x)] = row[x].rgbtRed;
                g[base + static_cast<size_t>(x)] = row[x].rgbtGreen;
                b[base + static_cast<size_t>(x)] = row[x].rgbtBlue;
            }
        }
    }

    void toRGBTRIPLE(RGBTRIPLE** dst) const {
        for (int y = 0; y < height; y++) {
            RGBTRIPLE* row = dst[y];
            size_t base = static_cast<size_t>(y) * static_cast<size_t>(width);
            for (int x = 0; x < width; x++) {
                row[x].rgbtRed   = r[base + static_cast<size_t>(x)];
                row[x].rgbtGreen = g[base + static_cast<size_t>(x)];
                row[x].rgbtBlue  = b[base + static_cast<size_t>(x)];
            }
        }
    }

    int maxAbsDiff(const PlanarRGB& o) const {
        int md = 0;
        for (size_t i = 0; i < r.size(); i++) {
            md = std::max(md, std::abs(static_cast<int>(r[i]) - static_cast<int>(o.r[i])));
            md = std::max(md, std::abs(static_cast<int>(g[i]) - static_cast<int>(o.g[i])));
            md = std::max(md, std::abs(static_cast<int>(b[i]) - static_cast<int>(o.b[i])));
        }
        return md;
    }
};

// ============================================================================
// Построение нормированных ядер Гаусса (2D для классического, 1D для 2x1D)
// ============================================================================
static std::vector<double> buildGaussianKernel2D(int ksize, double sigma) {
    int rad = ksize / 2;
    std::vector<double> kern(static_cast<size_t>(ksize) * static_cast<size_t>(ksize));
    double sum = 0.0;
    double denom = 2.0 * sigma * sigma;
    double normC = 1.0 / (2.0 * PI_VAL * sigma * sigma);
    for (int dy = -rad; dy <= rad; dy++) {
        for (int dx = -rad; dx <= rad; dx++) {
            double cf = normC * std::exp(-static_cast<double>(dx * dx + dy * dy) / denom);
            kern[static_cast<size_t>(dy + rad) * static_cast<size_t>(ksize) + static_cast<size_t>(dx + rad)] = cf;
            sum += cf;
        }
    }
    double invSum = 1.0 / sum;
    for (double& v : kern) v *= invSum;
    return kern;
}

static std::vector<double> buildGaussianKernel1D(int ksize, double sigma) {
    int rad = ksize / 2;
    std::vector<double> kern(static_cast<size_t>(ksize));
    double sum = 0.0;
    double denom = 2.0 * sigma * sigma;
    double normC = 1.0 / (std::sqrt(2.0 * PI_VAL) * sigma);
    for (int dx = -rad; dx <= rad; dx++) {
        double cf = normC * std::exp(-static_cast<double>(dx * dx) / denom);
        kern[static_cast<size_t>(dx + rad)] = cf;
        sum += cf;
    }
    double invSum = 1.0 / sum;
    for (double& v : kern) v *= invSum;
    return kern;
}

// ============================================================================
// ЧАСТЬ 1: ФИЛЬТР ГАУССА — КЛАССИЧЕСКИЙ (2D-свёртка O(K^2))
// ============================================================================
static inline void gaussClassicRow(const unsigned char* __restrict src,
                                   unsigned char* __restrict dstRow,
                                   int y, int W, int H, int rad, int ksize,
                                   const double* __restrict kern) {
    bool innerY = (y >= rad && y < H - rad);
    for (int x = 0; x < W; x++) {
        double acc = 0.0;
        if (innerY && x >= rad && x < W - rad) {
            for (int dy = -rad; dy <= rad; dy++) {
                const unsigned char* sRow = &src[static_cast<size_t>(y + dy) * static_cast<size_t>(W) + static_cast<size_t>(x - rad)];
                const double* kRow = &kern[static_cast<size_t>(dy + rad) * static_cast<size_t>(ksize)];
                for (int kx = 0; kx < ksize; kx++) {
                    acc += static_cast<double>(sRow[kx]) * kRow[kx];
                }
            }
        } else {
            for (int dy = -rad; dy <= rad; dy++) {
                int ky = std::clamp(y + dy, 0, H - 1);
                const unsigned char* sRow = &src[static_cast<size_t>(ky) * static_cast<size_t>(W)];
                const double* kRow = &kern[static_cast<size_t>(dy + rad) * static_cast<size_t>(ksize)];
                for (int dx = -rad; dx <= rad; dx++) {
                    int kx = std::clamp(x + dx, 0, W - 1);
                    acc += static_cast<double>(sRow[kx]) * kRow[dx + rad];
                }
            }
        }
        dstRow[x] = clampU8(acc);
    }
}

static void gaussClassicSeq(const PlanarRGB& src, PlanarRGB& dst, int ksize) {
    double sigma = static_cast<double>(ksize - 1) * 0.25;
    std::vector<double> kern = buildGaussianKernel2D(ksize, sigma);
    int rad = ksize / 2;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };
    for (int ch = 0; ch < 3; ch++) {
        for (int y = 0; y < src.height; y++) {
            gaussClassicRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(src.width)],
                            y, src.width, src.height, rad, ksize, kern.data());
        }
    }
}

static void gaussClassicOpenMP(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    double sigma = static_cast<double>(ksize - 1) * 0.25;
    std::vector<double> kern = buildGaussianKernel2D(ksize, sigma);
    int rad = ksize / 2;
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    #pragma omp parallel num_threads(numThreads) default(none) shared(inCh, outCh, kern, W, H, rad, ksize)
    {
        for (int ch = 0; ch < 3; ch++) {
            #pragma omp for schedule(static) nowait
            for (int y = 0; y < H; y++) {
                gaussClassicRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                y, W, H, rad, ksize, kern.data());
            }
        }
    }
}

static void gaussClassicTBB(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    double sigma = static_cast<double>(ksize - 1) * 0.25;
    std::vector<double> kern = buildGaussianKernel2D(ksize, sigma);
    int rad = ksize / 2;
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, 3 * H, 16),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int idx = r.begin(); idx < r.end(); idx++) {
                    int ch = idx / H;
                    int y = idx % H;
                    gaussClassicRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                    y, W, H, rad, ksize, kern.data());
                }
            });
    });
}

// ============================================================================
// ЧАСТЬ 2: ФИЛЬТР ГАУССА — БЫСТРЫЙ (2x1D сепарабельный с транспонированием TMP[W][H])
// ============================================================================
// Проход 1: горизонтальная 1D-свёртка строки y входного изображения -> запись в столбец y транспонированного буфера TMP[x][y]
static inline void gauss1DPass1Row(const unsigned char* __restrict srcRow,
                                   double* __restrict tmpTransposed,
                                   int y, int W, int H, int rad, int ksize,
                                   const double* __restrict kern1D) {
    for (int x = 0; x < W; x++) {
        double acc = 0.0;
        if (x >= rad && x < W - rad) {
            const unsigned char* s = &srcRow[x - rad];
            for (int k = 0; k < ksize; k++) {
                acc += static_cast<double>(s[k]) * kern1D[k];
            }
        } else {
            for (int dx = -rad; dx <= rad; dx++) {
                int kx = std::clamp(x + dx, 0, W - 1);
                acc += static_cast<double>(srcRow[kx]) * kern1D[dx + rad];
            }
        }
        // Транспонированная запись: TMP[x][y]
        tmpTransposed[static_cast<size_t>(x) * static_cast<size_t>(H) + static_cast<size_t>(y)] = acc;
    }
}

// Проход 2: горизонтальная 1D-свёртка строки x буфера TMP[x][0..H-1] -> обратная запись в IMAGE2[y][x]
static inline void gauss1DPass2Row(const double* __restrict tmpRow,
                                   unsigned char* __restrict dstImg,
                                   int x, int W, int H, int rad, int ksize,
                                   const double* __restrict kern1D) {
    for (int y = 0; y < H; y++) {
        double acc = 0.0;
        if (y >= rad && y < H - rad) {
            const double* t = &tmpRow[y - rad];
            for (int k = 0; k < ksize; k++) {
                acc += t[k] * kern1D[k];
            }
        } else {
            for (int dy = -rad; dy <= rad; dy++) {
                int ky = std::clamp(y + dy, 0, H - 1);
                acc += tmpRow[ky] * kern1D[dy + rad];
            }
        }
        dstImg[static_cast<size_t>(y) * static_cast<size_t>(W) + static_cast<size_t>(x)] = clampU8(acc);
    }
}

static void gaussFastSeq(const PlanarRGB& src, PlanarRGB& dst, int ksize, std::vector<double>& tmpBuf) {
    double sigma = static_cast<double>(ksize - 1) * 0.25;
    std::vector<double> kern1D = buildGaussianKernel1D(ksize, sigma);
    int rad = ksize / 2;
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    for (int ch = 0; ch < 3; ch++) {
        for (int y = 0; y < H; y++) {
            gauss1DPass1Row(&(*inCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                            tmpBuf.data(), y, W, H, rad, ksize, kern1D.data());
        }
        for (int x = 0; x < W; x++) {
            gauss1DPass2Row(&tmpBuf[static_cast<size_t>(x) * static_cast<size_t>(H)],
                            outCh[ch]->data(), x, W, H, rad, ksize, kern1D.data());
        }
    }
}

static void gaussFastOpenMP(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads, std::vector<double>& tmpBuf) {
    double sigma = static_cast<double>(ksize - 1) * 0.25;
    std::vector<double> kern1D = buildGaussianKernel1D(ksize, sigma);
    int rad = ksize / 2;
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    #pragma omp parallel num_threads(numThreads) default(none) shared(inCh, outCh, tmpBuf, kern1D, W, H, rad, ksize)
    {
        for (int ch = 0; ch < 3; ch++) {
            // Проход 1: горизонтальная свёртка + транспонирование (неявный барьер в конце цикла)
            #pragma omp for schedule(static)
            for (int y = 0; y < H; y++) {
                gauss1DPass1Row(&(*inCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                tmpBuf.data(), y, W, H, rad, ksize, kern1D.data());
            }
            // Проход 2: вторая горизонтальная свёртка по транспонированному буферу + возврат
            #pragma omp for schedule(static)
            for (int x = 0; x < W; x++) {
                gauss1DPass2Row(&tmpBuf[static_cast<size_t>(x) * static_cast<size_t>(H)],
                                outCh[ch]->data(), x, W, H, rad, ksize, kern1D.data());
            }
        }
    }
}

static void gaussFastTBB(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads, std::vector<double>& tmpBuf) {
    double sigma = static_cast<double>(ksize - 1) * 0.25;
    std::vector<double> kern1D = buildGaussianKernel1D(ksize, sigma);
    int rad = ksize / 2;
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        for (int ch = 0; ch < 3; ch++) {
            oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H, 16),
                [&](const oneapi::tbb::blocked_range<int>& r) {
                    for (int y = r.begin(); y < r.end(); y++) {
                        gauss1DPass1Row(&(*inCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                        tmpBuf.data(), y, W, H, rad, ksize, kern1D.data());
                    }
                });
            oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, W, 32),
                [&](const oneapi::tbb::blocked_range<int>& r) {
                    for (int x = r.begin(); x < r.end(); x++) {
                        gauss1DPass2Row(&tmpBuf[static_cast<size_t>(x) * static_cast<size_t>(H)],
                                        outCh[ch]->data(), x, W, H, rad, ksize, kern1D.data());
                    }
                });
        }
    });
}

// ============================================================================
// ЧАСТЬ 3: СРЕДНЕАРИФМЕТИЧЕСКИЙ ФИЛЬТР — КЛАССИЧЕСКИЙ (2D-сумма O(K^2))
// ============================================================================
static inline void meanClassicRow(const unsigned char* __restrict src,
                                  unsigned char* __restrict dstRow,
                                  int y, int W, int H, int rad, int ksize, double invArea) {
    bool innerY = (y >= rad && y < H - rad);
    for (int x = 0; x < W; x++) {
        int sumVal = 0;
        if (innerY && x >= rad && x < W - rad) {
            for (int dy = -rad; dy <= rad; dy++) {
                const unsigned char* sRow = &src[static_cast<size_t>(y + dy) * static_cast<size_t>(W) + static_cast<size_t>(x - rad)];
                for (int kx = 0; kx < ksize; kx++) {
                    sumVal += sRow[kx];
                }
            }
        } else {
            for (int dy = -rad; dy <= rad; dy++) {
                int ky = std::clamp(y + dy, 0, H - 1);
                const unsigned char* sRow = &src[static_cast<size_t>(ky) * static_cast<size_t>(W)];
                for (int dx = -rad; dx <= rad; dx++) {
                    int kx = std::clamp(x + dx, 0, W - 1);
                    sumVal += sRow[kx];
                }
            }
        }
        dstRow[x] = clampU8(static_cast<double>(sumVal) * invArea);
    }
}

static void meanClassicSeq(const PlanarRGB& src, PlanarRGB& dst, int ksize) {
    int rad = ksize / 2;
    double invArea = 1.0 / static_cast<double>(ksize * ksize);
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };
    for (int ch = 0; ch < 3; ch++) {
        for (int y = 0; y < src.height; y++) {
            meanClassicRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(src.width)],
                           y, src.width, src.height, rad, ksize, invArea);
        }
    }
}

static void meanClassicOpenMP(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    int rad = ksize / 2;
    double invArea = 1.0 / static_cast<double>(ksize * ksize);
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    #pragma omp parallel num_threads(numThreads) default(none) shared(inCh, outCh, W, H, rad, ksize, invArea)
    {
        for (int ch = 0; ch < 3; ch++) {
            #pragma omp for schedule(static) nowait
            for (int y = 0; y < H; y++) {
                meanClassicRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                               y, W, H, rad, ksize, invArea);
            }
        }
    }
}

static void meanClassicTBB(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    int rad = ksize / 2;
    double invArea = 1.0 / static_cast<double>(ksize * ksize);
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, 3 * H, 16),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int idx = r.begin(); idx < r.end(); idx++) {
                    int ch = idx / H;
                    int y = idx % H;
                    meanClassicRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                   y, W, H, rad, ksize, invArea);
                }
            });
    });
}

// ============================================================================
// ЧАСТЬ 4: СРЕДНЕАРИФМЕТИЧЕСКИЙ ФИЛЬТР — БЫСТРЫЙ (математическая рекурсия O(K))
//   При x = 0: полная сумма по окну [-rad..rad]x[-rad..rad]
//   При x > 0: sum = sum - col(x - rad - 1) + col(x + rad) по dy = -rad..rad
// ============================================================================
static inline void meanRecursiveRow(const unsigned char* __restrict src,
                                    unsigned char* __restrict dstRow,
                                    int y, int W, int H, int rad, int ksize, double invArea) {
    // Предвычисляем указатели на Ksize строк окрестности с учётом границ изображения
    const unsigned char* rows[13]; // Максимум Ksize = 13
    for (int dy = -rad; dy <= rad; dy++) {
        int ky = std::clamp(y + dy, 0, H - 1);
        rows[dy + rad] = &src[static_cast<size_t>(ky) * static_cast<size_t>(W)];
    }

    // x = 0: полный расчёт начальной суммы окна
    int sumVal = 0;
    for (int r = 0; r < ksize; r++) {
        const unsigned char* rowPtr = rows[r];
        for (int dx = -rad; dx <= rad; dx++) {
            int kx = std::clamp(dx, 0, W - 1);
            sumVal += rowPtr[kx];
        }
    }
    dstRow[0] = clampU8(static_cast<double>(sumVal) * invArea);

    // x > 0: рекурсивное обновление (вычитаем уходящий левый столбец KX1, добавляем правый KX2)
    for (int x = 1; x < W; x++) {
        int kx1 = x - rad - 1;
        if (kx1 < 0) kx1 = 0;
        int kx2 = x + rad;
        if (kx2 > W - 1) kx2 = W - 1;

        int delta = 0;
        for (int r = 0; r < ksize; r++) {
            const unsigned char* rowPtr = rows[r];
            delta += static_cast<int>(rowPtr[kx2]) - static_cast<int>(rowPtr[kx1]);
        }
        sumVal += delta;
        dstRow[x] = clampU8(static_cast<double>(sumVal) * invArea);
    }
}

static void meanFastSeq(const PlanarRGB& src, PlanarRGB& dst, int ksize) {
    int rad = ksize / 2;
    double invArea = 1.0 / static_cast<double>(ksize * ksize);
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };
    for (int ch = 0; ch < 3; ch++) {
        for (int y = 0; y < src.height; y++) {
            meanRecursiveRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(src.width)],
                             y, src.width, src.height, rad, ksize, invArea);
        }
    }
}

static void meanFastOpenMP(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    int rad = ksize / 2;
    double invArea = 1.0 / static_cast<double>(ksize * ksize);
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    // Рекурсия выполняется вдоль X независимо для каждой строки Y — идеально распараллеливается по Y
    #pragma omp parallel num_threads(numThreads) default(none) shared(inCh, outCh, W, H, rad, ksize, invArea)
    {
        for (int ch = 0; ch < 3; ch++) {
            #pragma omp for schedule(static) nowait
            for (int y = 0; y < H; y++) {
                meanRecursiveRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                 y, W, H, rad, ksize, invArea);
            }
        }
    }
}

static void meanFastTBB(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    int rad = ksize / 2;
    double invArea = 1.0 / static_cast<double>(ksize * ksize);
    int W = src.width, H = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, 3 * H, 16),
            [&](const oneapi::tbb::blocked_range<int>& r) {
                for (int idx = r.begin(); idx < r.end(); idx++) {
                    int ch = idx / H;
                    int y = idx % H;
                    meanRecursiveRow(inCh[ch]->data(), &(*outCh[ch])[static_cast<size_t>(y) * static_cast<size_t>(W)],
                                     y, W, H, rad, ksize, invArea);
                }
            });
    });
}

static void savePlanarBMP(const PlanarRGB& img, const std::string& path) {
    RGBTRIPLE** rgb = BMPAlloc(img.width, img.height);
    img.toRGBTRIPLE(rgb);
    BMPWrite(rgb, img.width, img.height, path.c_str());
    BMPFree(rgb);
}

struct ConvDataset {
    int id = 0;
    int W = 0, H = 0;
    std::string path;
    PlanarRGB src;
};

struct ConvRow {
    int imgId = 0;
    std::string res;
    std::string filterType; // "Gauss" or "Mean"
    std::string variant;    // "Classic" or "Fast"
    std::string backend;    // "Seq", "OpenMP", "oneTBB"
    int ksize = 0;
    int threads = 1;
    int runs = 0;
    TrustedStats stats;
    double speedupOwn = 1.0;       // Ускорение относительно своего посл. варианта (Classic_Seq или Fast_Seq)
    double speedupVsClassic = 1.0; // Общее ускорение относительно Classic_Seq
    double efficiency = 1.0;
    double costMs = 0.0;
    std::vector<double> rawTimesMs;
};

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    bool fastDemo = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--fast") fastDemo = true;
    }

    const int RUNS = fastDemo ? 3 : 15; // >= 7 запусков по заданию 5.2 (15 в полном режиме, 3 в --fast для защиты)

    std::cout << "====================================================================================================\n";
    std::cout << "  Лабораторная работа №5: Алгоритмы свёртки (Гаусс и Среднеарифметический) в OpenMP и Intel oneTBB\n";
    std::cout << "====================================================================================================\n";
    std::cout << "Стандарт OpenMP:      " << _OPENMP << "\n";
    std::cout << "Intel oneTBB потоков: " << oneapi::tbb::info::default_concurrency() << "\n";
    std::cout << "Точность таймера:     omp_get_wtick() = " << std::fixed << std::setprecision(6)
              << (omp_get_wtick() * 1000.0) << " мс\n";
    std::cout << "Фильтрация выбросов:  AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n";
    std::cout << "Запусков на серию:    " << RUNS << (fastDemo ? " (быстрый режим --fast)\n\n" : " (контрольная серия ДИ: 100)\n\n");

    // Задание 5.1: демонстрация загрузки input.bmp и сохранения output_alg_var.bmp (для Ksize=9)
    {
        RGBTRIPLE** rgb = nullptr;
        BITMAPFILEHEADER fh{};
        BITMAPINFOHEADER ih{};
        BMPRead(rgb, fh, ih, "Lab5/images/input.bmp");
        PlanarRGB inSingle, outSingle;
        inSingle.fromRGBTRIPLE(rgb, ih.biWidth, ih.biHeight);
        BMPFree(rgb);
        outSingle.allocate(inSingle.width, inSingle.height);
        std::vector<double> tmp(static_cast<size_t>(inSingle.width) * static_cast<size_t>(inSingle.height));

        gaussClassicTBB(inSingle, outSingle, 9, 4);
        savePlanarBMP(outSingle, "Lab5/images/output_gauss_classic.bmp");
        gaussFastTBB(inSingle, outSingle, 9, 4, tmp);
        savePlanarBMP(outSingle, "Lab5/images/output_gauss_fast.bmp");
        meanClassicTBB(inSingle, outSingle, 9, 4);
        savePlanarBMP(outSingle, "Lab5/images/output_mean_classic.bmp");
        meanFastTBB(inSingle, outSingle, 9, 4);
        savePlanarBMP(outSingle, "Lab5/images/output_mean_fast.bmp");
        std::cout << "Задание 5.1: обработан Lab5/images/input.bmp -> output_{gauss,mean}_{classic,fast}.bmp\n";
    }

    // Задание 5.2: загрузка четырёх изображений от 1280x720 до 2560x1440
    std::vector<ConvDataset> datasets;
    int numImg = fastDemo ? 2 : 4;
    for (int i = 1; i <= numImg; i++) {
        ConvDataset ds;
        ds.id = i;
        ds.path = "Lab5/images/input_" + std::to_string(i) + ".bmp";
        RGBTRIPLE** rgb = nullptr;
        BITMAPFILEHEADER fh{};
        BITMAPINFOHEADER ih{};
        BMPRead(rgb, fh, ih, ds.path.c_str());
        ds.W = ih.biWidth;
        ds.H = ih.biHeight;
        ds.src.fromRGBTRIPLE(rgb, ds.W, ds.H);
        BMPFree(rgb);
        std::cout << "Загружен НД " << i << ": " << ds.path << " (" << ds.W << "x" << ds.H
                  << ", " << (ds.W * ds.H) << " пикс., 24-bit RGB)\n";
        datasets.push_back(std::move(ds));
    }

    const std::vector<int> kernels = { 5, 9, 13 };
    const std::vector<int> parThreads = { 2, 3, 4 };
    std::vector<ConvRow> allRows;

    auto runFilterSuite = [&](const std::string& filterType, const std::string& titleRu) {
        std::cout << "\n====================================================================================================\n";
        std::cout << "  " << titleRu << "\n";
        std::cout << "====================================================================================================\n";

        for (const auto& ds : datasets) {
            std::cout << "\n--- НД " << ds.id << " (" << ds.W << "x" << ds.H << ") — " << filterType << " ---\n";
            std::cout << padRightU8("Функция (реализация)", 24)
                      << padLeftU8("Ksize", 6)
                      << padLeftU8("Пот.", 5)
                      << padLeftU8("Зам.", 5)
                      << padLeftU8("Исх.ср.(мс)", 12)
                      << padLeftU8("Отбр.", 6)
                      << padLeftU8("Итог(мс)", 12)
                      << padLeftU8("Sp(n)", 8)
                      << padLeftU8("Ep(n)", 8)
                      << padLeftU8("Cp(мс)", 12)
                      << "\n";
            std::cout << std::string(98, '-') << "\n";

            PlanarRGB dst, refClassic, refFast;
            dst.allocate(ds.W, ds.H);
            refClassic.allocate(ds.W, ds.H);
            refFast.allocate(ds.W, ds.H);
            std::vector<double> tmpBuf(static_cast<size_t>(ds.W) * static_cast<size_t>(ds.H));

            for (int ksize : kernels) {
                if (filterType == "Gauss") {
                    gaussClassicSeq(ds.src, refClassic, ksize);
                    gaussFastSeq(ds.src, refFast, ksize, tmpBuf);
                    if (refClassic.maxAbsDiff(refFast) > 1) {
                        throw std::runtime_error("Расхождение Gauss Classic vs Fast > 1!");
                    }
                } else {
                    meanClassicSeq(ds.src, refClassic, ksize);
                    meanFastSeq(ds.src, refFast, ksize);
                    if (refClassic.maxAbsDiff(refFast) > 0) {
                        throw std::runtime_error("Расхождение Mean Classic vs Fast > 0!");
                    }
                }

                // Сохраняем примеры для НД 1 (K=5,9,13) и НД 2-4 (K=9)
                if (!fastDemo && (ds.id == 1 || ksize == 9)) {
                    std::string tag = (filterType == "Gauss") ? "gauss" : "mean";
                    savePlanarBMP(refFast, "Lab5/images/output_" + std::to_string(ds.id) + "_" + tag + "_k" + std::to_string(ksize) + ".bmp");
                }

                double t1Classic = 0.0;
                double t1Fast = 0.0;

                auto bench = [&](const std::string& variant, const std::string& backend, int p,
                                 const PlanarRGB& refImg, auto fn) -> double {
                    // Прогрев + проверка точности
                    fn();
                    if (dst.maxAbsDiff(refImg) > 0) {
                        throw std::runtime_error("Несовпадение результата: " + filterType + "_" + variant + "_" + backend);
                    }

                    // Для графика доверительного интервала (100 замеров): НД 4, K=13, Fast oneTBB p=4
                    int nRuns = RUNS;
                    if (!fastDemo && ds.id == 4 && ksize == 13 && variant == "Fast" && backend == "oneTBB" && p == 4) {
                        nRuns = 100;
                    }

                    double* times = new double[nRuns];
                    std::vector<double> raw(static_cast<size_t>(nRuns));
                    for (int r = 0; r < nRuns; r++) {
                        double t0 = omp_get_wtime();
                        fn();
                        double t1 = omp_get_wtime();
                        double ms = (t1 - t0) * 1000.0;
                        times[r] = ms;
                        raw[static_cast<size_t>(r)] = ms;
                    }

                    ConvRow row;
                    row.imgId = ds.id;
                    row.res = std::to_string(ds.W) + "x" + std::to_string(ds.H);
                    row.filterType = filterType;
                    row.variant = variant;
                    row.backend = backend;
                    row.ksize = ksize;
                    row.threads = p;
                    row.runs = nRuns;
                    row.stats = computeTrustedStats(times, nRuns);
                    row.rawTimesMs = std::move(raw);
                    delete[] times;

                    double ownBase = (variant == "Classic") ? ((p == 1) ? row.stats.trustedAvgMed : t1Classic)
                                                            : ((p == 1) ? row.stats.trustedAvgMed : t1Fast);
                    double clsBase = (variant == "Classic" && p == 1) ? row.stats.trustedAvgMed : t1Classic;
                    row.speedupOwn = (row.stats.trustedAvgMed > 0.0) ? (ownBase / row.stats.trustedAvgMed) : 1.0;
                    row.speedupVsClassic = (row.stats.trustedAvgMed > 0.0) ? (clsBase / row.stats.trustedAvgMed) : 1.0;
                    row.efficiency = row.speedupOwn / static_cast<double>(p);
                    row.costMs = static_cast<double>(p) * row.stats.trustedAvgMed;

                    std::string label = filterType + "_" + variant + "_" + backend;
                    std::cout << padRightU8(label, 24)
                              << std::right << std::fixed
                              << std::setw(6) << ksize
                              << std::setw(5) << p
                              << std::setw(5) << nRuns
                              << std::setw(12) << std::setprecision(4) << row.stats.rawAvg
                              << std::setw(6) << row.stats.discardedRuns
                              << std::setw(12) << std::setprecision(4) << row.stats.trustedAvgMed
                              << std::setw(8) << std::setprecision(4) << row.speedupOwn
                              << std::setw(8) << std::setprecision(4) << row.efficiency
                              << std::setw(12) << std::setprecision(4) << row.costMs
                              << "\n" << std::flush;

                    double res = row.stats.trustedAvgMed;
                    allRows.push_back(std::move(row));
                    return res;
                };

                if (filterType == "Gauss") {
                    t1Classic = bench("Classic", "Seq", 1, refClassic, [&]() { gaussClassicSeq(ds.src, dst, ksize); });
                    for (int p : parThreads) bench("Classic", "OpenMP", p, refClassic, [&]() { gaussClassicOpenMP(ds.src, dst, ksize, p); });
                    for (int p : parThreads) bench("Classic", "oneTBB", p, refClassic, [&]() { gaussClassicTBB(ds.src, dst, ksize, p); });

                    t1Fast = bench("Fast", "Seq", 1, refFast, [&]() { gaussFastSeq(ds.src, dst, ksize, tmpBuf); });
                    for (int p : parThreads) bench("Fast", "OpenMP", p, refFast, [&]() { gaussFastOpenMP(ds.src, dst, ksize, p, tmpBuf); });
                    for (int p : parThreads) bench("Fast", "oneTBB", p, refFast, [&]() { gaussFastTBB(ds.src, dst, ksize, p, tmpBuf); });
                } else {
                    t1Classic = bench("Classic", "Seq", 1, refClassic, [&]() { meanClassicSeq(ds.src, dst, ksize); });
                    for (int p : parThreads) bench("Classic", "OpenMP", p, refClassic, [&]() { meanClassicOpenMP(ds.src, dst, ksize, p); });
                    for (int p : parThreads) bench("Classic", "oneTBB", p, refClassic, [&]() { meanClassicTBB(ds.src, dst, ksize, p); });

                    t1Fast = bench("Fast", "Seq", 1, refFast, [&]() { meanFastSeq(ds.src, dst, ksize); });
                    for (int p : parThreads) bench("Fast", "OpenMP", p, refFast, [&]() { meanFastOpenMP(ds.src, dst, ksize, p); });
                    for (int p : parThreads) bench("Fast", "oneTBB", p, refFast, [&]() { meanFastTBB(ds.src, dst, ksize, p); });
                }
            }
        }
    };

    runFilterSuite("Gauss", "Таблица 1: Фильтр Гаусса (классический 2D и быстрый 2x1D с транспонированием)");
    runFilterSuite("Mean",  "Таблица 2: Среднеарифметический фильтр (классический 2D и быстрый рекурсивный)");

    if (!fastDemo) {
        std::ofstream sumOut("Lab5/results/lab5_summary.csv");
        sumOut << "img_id,resolution,filter_type,variant,backend,ksize,threads,runs,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,speedup_own,speedup_vs_classic,efficiency,cost_ms\n";
        sumOut << std::fixed << std::setprecision(6);
        for (const auto& r : allRows) {
            sumOut << r.imgId << "," << r.res << "," << r.filterType << "," << r.variant << "," << r.backend << ","
                   << r.ksize << "," << r.threads << "," << r.runs << ","
                   << r.stats.rawAvg << "," << r.stats.median << "," << r.stats.sd << ","
                   << r.stats.keptRuns << "," << r.stats.discardedRuns << ","
                   << r.stats.trustedAvgMed << "," << r.speedupOwn << "," << r.speedupVsClassic << ","
                   << r.efficiency << "," << r.costMs << "\n";
        }
        sumOut.close();

        std::ofstream rawOut("Lab5/results/lab5_raw_samples.csv");
        rawOut << "filter_type,variant,backend,ksize,threads,run_idx,time_ms,median_ms,sd_ms,in_interval\n";
        rawOut << std::fixed << std::setprecision(6);
        for (const auto& r : allRows) {
            if (r.runs == 100) {
                for (size_t i = 0; i < r.rawTimesMs.size(); i++) {
                    double v = r.rawTimesMs[i];
                    int inI = (v >= r.stats.median - r.stats.sd && v <= r.stats.median + r.stats.sd) ? 1 : 0;
                    rawOut << r.filterType << "," << r.variant << "," << r.backend << ","
                           << r.ksize << "," << r.threads << "," << (i + 1) << ","
                           << v << "," << r.stats.median << "," << r.stats.sd << "," << inI << "\n";
                }
            }
        }
        rawOut.close();
        std::cout << "\nРезультаты ЛР №5 и отфильтрованные BMP-изображения сохранены успешно.\n";
    }
    return 0;
}

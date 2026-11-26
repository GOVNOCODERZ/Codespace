// ============================================================================
// Лабораторная работа №6: Реализация алгоритмов расчёта особых точек
// (детектор углов Харриса) с использованием OpenMP и Intel oneTBB.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <omp.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/concurrent_vector.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/info.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_reduce.h>
#include <oneapi/tbb/parallel_sort.h>
#include <oneapi/tbb/task_arena.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "BMPFileRW.h"
#include "common_stats.h"

namespace {

// ----------------------------------------------------------------------------
// Вспомогательные функции UTF-8 выравнивания столбцов в консоли
// ----------------------------------------------------------------------------
size_t utf8Len(const std::string& s) {
    size_t cnt = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0u) != 0x80u) ++cnt;
    }
    return cnt;
}

std::string padRightU8(const std::string& s, size_t width) {
    size_t l = utf8Len(s);
    return (l < width) ? (s + std::string(width - l, ' ')) : s;
}

std::string padLeftU8(const std::string& s, size_t width) {
    size_t l = utf8Len(s);
    return (l < width) ? (std::string(width - l, ' ') + s) : s;
}

std::string fmtNum(double v, int prec = 4) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(prec) << v;
    return oss.str();
}

inline int clampCoord(int v, int hi) {
    if (v < 0) return 0;
    if (v > hi) return hi;
    return v;
}

// ----------------------------------------------------------------------------
// Структуры данных изображения и особой точки Харриса
// ----------------------------------------------------------------------------
struct ImageRGB {
    int id = 0;
    int width = 0;
    int height = 0;
    std::string label;
    std::vector<unsigned char> r, g, b;

    bool load(const std::string& path) {
        std::ifstream test(path, std::ios::binary);
        if (!test.good()) return false;
        test.close();
        RGBTRIPLE** rgb = nullptr;
        BITMAPFILEHEADER hdr{};
        BITMAPINFOHEADER bmi{};
        BMPRead(rgb, hdr, bmi, path.c_str());
        width = bmi.biWidth;
        height = bmi.biHeight;
        if (width <= 0 || height <= 0 || !rgb) {
            BMPFree(rgb);
            return false;
        }
        const size_t n = static_cast<size_t>(width) * height;
        r.resize(n);
        g.resize(n);
        b.resize(n);
        for (int y = 0; y < height; ++y) {
            const size_t row = static_cast<size_t>(y) * width;
            for (int x = 0; x < width; ++x) {
                r[row + x] = rgb[y][x].rgbtRed;
                g[row + x] = rgb[y][x].rgbtGreen;
                b[row + x] = rgb[y][x].rgbtBlue;
            }
        }
        BMPFree(rgb);
        return true;
    }

    void save(const std::string& path) const {
        RGBTRIPLE** rgb = BMPAlloc(width, height);
        for (int y = 0; y < height; ++y) {
            const size_t row = static_cast<size_t>(y) * width;
            for (int x = 0; x < width; ++x) {
                rgb[y][x].rgbtRed   = r[row + x];
                rgb[y][x].rgbtGreen = g[row + x];
                rgb[y][x].rgbtBlue  = b[row + x];
            }
        }
        BMPWrite(rgb, width, height, path.c_str());
        BMPFree(rgb);
    }
};

struct HarrisPoint {
    float r;
    int y;
    int x;
};

struct HarrisPointDesc {
    bool operator()(const HarrisPoint& a, const HarrisPoint& b) const {
        if (a.r != b.r) return a.r > b.r;
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    }
};

// Коэффициенты операторов градиента (стр. 5-6 методички)
constexpr float MATR_COEF_X[3][3] = {
    { 1.0f, 0.0f, -1.0f },
    { 1.0f, 0.0f, -1.0f },
    { 1.0f, 0.0f, -1.0f }
};

constexpr float MATR_COEF_Y[3][3] = {
    {  1.0f,  1.0f,  1.0f },
    {  0.0f,  0.0f,  0.0f },
    { -1.0f, -1.0f, -1.0f }
};

constexpr float HARRIS_K = 0.04f;
constexpr float HARRIS_KF = 0.70f;  // kf in [0.6, 0.8] по методичке (стр. 8)
constexpr int   HARRIS_RANG = 20;   // rang >= 20 по методичке (стр. 8)
constexpr int   VIS_RANG = 1;       // rang = 1 (3x3 красный маркер) по методичке (стр. 9)

// 1D ядро Гаусса (Ksize = 5, sigma = 1.0) для предварительного сглаживания (Шаг 1)
std::vector<float> makeGaussian1D(int ksize) {
    int rad = ksize / 2;
    double sigma = (ksize - 1) / 4.0;
    std::vector<float> ker(ksize);
    double sum = 0.0;
    for (int i = -rad; i <= rad; ++i) {
        double v = std::exp(-(i * i) / (2.0 * sigma * sigma));
        ker[i + rad] = static_cast<float>(v);
        sum += v;
    }
    for (int i = 0; i < ksize; ++i) ker[i] = static_cast<float>(ker[i] / sum);
    return ker;
}

// ----------------------------------------------------------------------------
// ШАГ 1: Формирование массива яркости I(y,x) и сглаживание Гауссом (2x1D)
// ----------------------------------------------------------------------------
void step1_seq(const ImageRGB& img, const std::vector<float>& gKer,
               std::vector<float>& rawI, std::vector<float>& tmpT, std::vector<float>& smoothI) {
    const int W = img.width, H = img.height;
    const int rad = static_cast<int>(gKer.size()) / 2;

    for (int y = 0; y < H; ++y) {
        const size_t row = static_cast<size_t>(y) * W;
        for (int x = 0; x < W; ++x) {
            const size_t idx = row + x;
            rawI[idx] = img.r[idx] * 0.299f + img.g[idx] * 0.587f + img.b[idx] * 0.114f;
        }
    }
    // Горизонтальная 1D-свёртка + запись в транспонированный буфер tmpT[W][H]
    for (int y = 0; y < H; ++y) {
        const float* srcRow = &rawI[static_cast<size_t>(y) * W];
        for (int x = 0; x < W; ++x) {
            float acc = 0.0f;
            for (int d = -rad; d <= rad; ++d) {
                int kx = clampCoord(x + d, W - 1);
                acc += srcRow[kx] * gKer[d + rad];
            }
            tmpT[static_cast<size_t>(x) * H + y] = acc;
        }
    }
    // Вертикальная 1D-свёртка по строкам tmpT + обратное транспонирование в smoothI[H][W]
    for (int x = 0; x < W; ++x) {
        const float* colRow = &tmpT[static_cast<size_t>(x) * H];
        for (int y = 0; y < H; ++y) {
            float acc = 0.0f;
            for (int d = -rad; d <= rad; ++d) {
                int ky = clampCoord(y + d, H - 1);
                acc += colRow[ky] * gKer[d + rad];
            }
            smoothI[static_cast<size_t>(y) * W + x] = acc;
        }
    }
}

void step1_omp(const ImageRGB& img, const std::vector<float>& gKer, int threads,
               std::vector<float>& rawI, std::vector<float>& tmpT, std::vector<float>& smoothI) {
    const int W = img.width, H = img.height;
    const int rad = static_cast<int>(gKer.size()) / 2;

    #pragma omp parallel num_threads(threads)
    {
        #pragma omp for schedule(static)
        for (int y = 0; y < H; ++y) {
            const size_t row = static_cast<size_t>(y) * W;
            for (int x = 0; x < W; ++x) {
                const size_t idx = row + x;
                rawI[idx] = img.r[idx] * 0.299f + img.g[idx] * 0.587f + img.b[idx] * 0.114f;
            }
        }
        #pragma omp for schedule(static)
        for (int y = 0; y < H; ++y) {
            const float* srcRow = &rawI[static_cast<size_t>(y) * W];
            for (int x = 0; x < W; ++x) {
                float acc = 0.0f;
                for (int d = -rad; d <= rad; ++d) {
                    int kx = clampCoord(x + d, W - 1);
                    acc += srcRow[kx] * gKer[d + rad];
                }
                tmpT[static_cast<size_t>(x) * H + y] = acc;
            }
        }
        #pragma omp for schedule(static)
        for (int x = 0; x < W; ++x) {
            const float* colRow = &tmpT[static_cast<size_t>(x) * H];
            for (int y = 0; y < H; ++y) {
                float acc = 0.0f;
                for (int d = -rad; d <= rad; ++d) {
                    int ky = clampCoord(y + d, H - 1);
                    acc += colRow[ky] * gKer[d + rad];
                }
                smoothI[static_cast<size_t>(y) * W + x] = acc;
            }
        }
    }
}

void step1_tbb(const ImageRGB& img, const std::vector<float>& gKer, int threads,
               std::vector<float>& rawI, std::vector<float>& tmpT, std::vector<float>& smoothI) {
    const int W = img.width, H = img.height;
    const int rad = static_cast<int>(gKer.size()) / 2;

    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            for (int y = r.begin(); y < r.end(); ++y) {
                const size_t row = static_cast<size_t>(y) * W;
                for (int x = 0; x < W; ++x) {
                    const size_t idx = row + x;
                    rawI[idx] = img.r[idx] * 0.299f + img.g[idx] * 0.587f + img.b[idx] * 0.114f;
                }
            }
        });
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            for (int y = r.begin(); y < r.end(); ++y) {
                const float* srcRow = &rawI[static_cast<size_t>(y) * W];
                for (int x = 0; x < W; ++x) {
                    float acc = 0.0f;
                    for (int d = -rad; d <= rad; ++d) {
                        int kx = clampCoord(x + d, W - 1);
                        acc += srcRow[kx] * gKer[d + rad];
                    }
                    tmpT[static_cast<size_t>(x) * H + y] = acc;
                }
            }
        });
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, W), [&](const auto& r) {
            for (int x = r.begin(); x < r.end(); ++x) {
                const float* colRow = &tmpT[static_cast<size_t>(x) * H];
                for (int y = 0; y < H; ++y) {
                    float acc = 0.0f;
                    for (int d = -rad; d <= rad; ++d) {
                        int ky = clampCoord(y + d, H - 1);
                        acc += colRow[ky] * gKer[d + rad];
                    }
                    smoothI[static_cast<size_t>(y) * W + x] = acc;
                }
            }
        });
    });
}

// ----------------------------------------------------------------------------
// ШАГ 2: Вычисление 1-го и 2-го градиентов (строго по псевдокоду стр. 6-7)
// ----------------------------------------------------------------------------
inline void step2_pass1_row(int y, int W, int H, const std::vector<float>& I,
                            std::vector<float>& tmpX, std::vector<float>& tmpY) {
    const size_t outRow = static_cast<size_t>(y) * W;
    for (int x = 0; x < W; ++x) {
        float linX = 0.0f;
        float linY = 0.0f;
        for (int dy = -1; dy <= 1; ++dy) {
            const int ky = clampCoord(y + dy, H - 1);
            const size_t kRow = static_cast<size_t>(ky) * W;
            for (int dx = -1; dx <= 1; ++dx) {
                const int kx = clampCoord(x + dx, W - 1);
                const float val = I[kRow + kx];
                linX += val * MATR_COEF_X[dy + 1][dx + 1];
                linY += val * MATR_COEF_Y[dy + 1][dx + 1];
            }
        }
        tmpX[outRow + x] = linX;
        tmpY[outRow + x] = linY;
    }
}

inline void step2_pass2_row(int y, int W, int H,
                            const std::vector<float>& tmpX, const std::vector<float>& tmpY,
                            std::vector<float>& diffX, std::vector<float>& diffY, std::vector<float>& diffXY) {
    const size_t outRow = static_cast<size_t>(y) * W;
    for (int x = 0; x < W; ++x) {
        float linXX = 0.0f;
        float linYY = 0.0f;
        float linXY = 0.0f;
        for (int dy = -1; dy <= 1; ++dy) {
            const int ky = clampCoord(y + dy, H - 1);
            const size_t kRow = static_cast<size_t>(ky) * W;
            for (int dx = -1; dx <= 1; ++dx) {
                const int kx = clampCoord(x + dx, W - 1);
                const float vx = tmpX[kRow + kx];
                const float vy = tmpY[kRow + kx];
                linXX += vx * MATR_COEF_X[dy + 1][dx + 1];
                linXY += vx * MATR_COEF_Y[dy + 1][dx + 1];
                linYY += vy * MATR_COEF_Y[dy + 1][dx + 1];
            }
        }
        diffX[outRow + x]  = linXX;
        diffY[outRow + x]  = linYY;
        diffXY[outRow + x] = linXY;
    }
}

void step2_seq(int W, int H, const std::vector<float>& I,
               std::vector<float>& tmpX, std::vector<float>& tmpY,
               std::vector<float>& diffX, std::vector<float>& diffY, std::vector<float>& diffXY) {
    for (int y = 0; y < H; ++y) step2_pass1_row(y, W, H, I, tmpX, tmpY);
    for (int y = 0; y < H; ++y) step2_pass2_row(y, W, H, tmpX, tmpY, diffX, diffY, diffXY);
}

void step2_omp(int W, int H, const std::vector<float>& I, int threads,
               std::vector<float>& tmpX, std::vector<float>& tmpY,
               std::vector<float>& diffX, std::vector<float>& diffY, std::vector<float>& diffXY) {
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp for schedule(static)
        for (int y = 0; y < H; ++y) step2_pass1_row(y, W, H, I, tmpX, tmpY);
        #pragma omp for schedule(static)
        for (int y = 0; y < H; ++y) step2_pass2_row(y, W, H, tmpX, tmpY, diffX, diffY, diffXY);
    }
}

void step2_tbb(int W, int H, const std::vector<float>& I, int threads,
               std::vector<float>& tmpX, std::vector<float>& tmpY,
               std::vector<float>& diffX, std::vector<float>& diffY, std::vector<float>& diffXY) {
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            for (int y = r.begin(); y < r.end(); ++y) step2_pass1_row(y, W, H, I, tmpX, tmpY);
        });
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            for (int y = r.begin(); y < r.end(); ++y) step2_pass2_row(y, W, H, tmpX, tmpY, diffX, diffY, diffXY);
        });
    });
}

// ----------------------------------------------------------------------------
// ШАГ 3: Вычисление функции отклика угла R (массив H[][])
// ----------------------------------------------------------------------------
void step3_seq(int W, int H,
               const std::vector<float>& diffX, const std::vector<float>& diffY,
               const std::vector<float>& diffXY, std::vector<float>& Hmap) {
    const size_t total = static_cast<size_t>(W) * H;
    for (size_t i = 0; i < total; ++i) {
        const float A = diffX[i];
        const float B = diffY[i];
        const float C = diffXY[i];
        const float tr = A + B;
        Hmap[i] = (A * B - C * C) - (HARRIS_K * (tr * tr));
    }
}

void step3_omp(int W, int H, int threads,
               const std::vector<float>& diffX, const std::vector<float>& diffY,
               const std::vector<float>& diffXY, std::vector<float>& Hmap) {
    const int total = W * H;
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < total; ++i) {
        const float A = diffX[i];
        const float B = diffY[i];
        const float C = diffXY[i];
        const float tr = A + B;
        Hmap[i] = (A * B - C * C) - (HARRIS_K * (tr * tr));
    }
}

void step3_tbb(int W, int H, int threads,
               const std::vector<float>& diffX, const std::vector<float>& diffY,
               const std::vector<float>& diffXY, std::vector<float>& Hmap) {
    const int total = W * H;
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, total), [&](const auto& r) {
            for (int i = r.begin(); i < r.end(); ++i) {
                const float A = diffX[i];
                const float B = diffY[i];
                const float C = diffXY[i];
                const float tr = A + B;
                Hmap[i] = (A * B - C * C) - (HARRIS_K * (tr * tr));
            }
        });
    });
}

// ----------------------------------------------------------------------------
// ШАГ 4.1: Поиск min/max(H) в едином цикле, расчёт threshold и сбор HarrisPoints
// Вариант A: классический контейнер STL (std::vector)
// Вариант B: потокобезопасный контейнер Intel oneTBB (concurrent_vector)
// ----------------------------------------------------------------------------
void step4_1_seq_stl(int W, int H, const std::vector<float>& Hmap, float kf,
                     std::vector<HarrisPoint>& outPts) {
    const size_t total = static_cast<size_t>(W) * H;
    float hMin = std::numeric_limits<float>::max();
    float hMax = std::numeric_limits<float>::lowest();
    for (size_t i = 0; i < total; ++i) {
        if (Hmap[i] < hMin) hMin = Hmap[i];
        if (Hmap[i] > hMax) hMax = Hmap[i];
    }
    const float thr = (hMax - hMin) * kf + hMin;
    outPts.clear();
    for (int y = 0; y < H; ++y) {
        const size_t row = static_cast<size_t>(y) * W;
        for (int x = 0; x < W; ++x) {
            float v = Hmap[row + x];
            if (v > thr) outPts.push_back({v, y, x});
        }
    }
}

void step4_1_omp_stl(int W, int H, const std::vector<float>& Hmap, float kf, int threads,
                     std::vector<HarrisPoint>& outPts) {
    const int total = W * H;
    float hMin = std::numeric_limits<float>::max();
    float hMax = std::numeric_limits<float>::lowest();
    #pragma omp parallel for num_threads(threads) reduction(min:hMin) reduction(max:hMax) schedule(static)
    for (int i = 0; i < total; ++i) {
        if (Hmap[i] < hMin) hMin = Hmap[i];
        if (Hmap[i] > hMax) hMax = Hmap[i];
    }
    const float thr = (hMax - hMin) * kf + hMin;

    std::vector<std::vector<HarrisPoint>> localVecs(threads);
    #pragma omp parallel num_threads(threads)
    {
        int tid = omp_get_thread_num();
        auto& loc = localVecs[tid];
        #pragma omp for schedule(static)
        for (int y = 0; y < H; ++y) {
            const size_t row = static_cast<size_t>(y) * W;
            for (int x = 0; x < W; ++x) {
                float v = Hmap[row + x];
                if (v > thr) loc.push_back({v, y, x});
            }
        }
    }
    size_t sumSz = 0;
    for (const auto& lv : localVecs) sumSz += lv.size();
    outPts.clear();
    outPts.reserve(sumSz);
    for (const auto& lv : localVecs) outPts.insert(outPts.end(), lv.begin(), lv.end());
}

void step4_1_omp_tbbvec(int W, int H, const std::vector<float>& Hmap, float kf, int threads,
                        oneapi::tbb::concurrent_vector<HarrisPoint>& outPts) {
    const int total = W * H;
    float hMin = std::numeric_limits<float>::max();
    float hMax = std::numeric_limits<float>::lowest();
    #pragma omp parallel for num_threads(threads) reduction(min:hMin) reduction(max:hMax) schedule(static)
    for (int i = 0; i < total; ++i) {
        if (Hmap[i] < hMin) hMin = Hmap[i];
        if (Hmap[i] > hMax) hMax = Hmap[i];
    }
    const float thr = (hMax - hMin) * kf + hMin;

    outPts.clear();
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int y = 0; y < H; ++y) {
        const size_t row = static_cast<size_t>(y) * W;
        for (int x = 0; x < W; ++x) {
            float v = Hmap[row + x];
            if (v > thr) outPts.push_back({v, y, x});
        }
    }
}

struct MinMaxPair {
    float mn;
    float mx;
};

void step4_1_tbb_stl(int W, int H, const std::vector<float>& Hmap, float kf, int threads,
                     std::vector<HarrisPoint>& outPts) {
    const int total = W * H;
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        MinMaxPair init{std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
        MinMaxPair mm = oneapi::tbb::parallel_reduce(
            oneapi::tbb::blocked_range<int>(0, total), init,
            [&](const oneapi::tbb::blocked_range<int>& r, MinMaxPair cur) {
                for (int i = r.begin(); i < r.end(); ++i) {
                    if (Hmap[i] < cur.mn) cur.mn = Hmap[i];
                    if (Hmap[i] > cur.mx) cur.mx = Hmap[i];
                }
                return cur;
            },
            [](MinMaxPair a, MinMaxPair b) {
                return MinMaxPair{std::min(a.mn, b.mn), std::max(a.mx, b.mx)};
            }
        );
        const float thr = (mm.mx - mm.mn) * kf + mm.mn;

        oneapi::tbb::enumerable_thread_specific<std::vector<HarrisPoint>> ets;
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            auto& loc = ets.local();
            for (int y = r.begin(); y < r.end(); ++y) {
                const size_t row = static_cast<size_t>(y) * W;
                for (int x = 0; x < W; ++x) {
                    float v = Hmap[row + x];
                    if (v > thr) loc.push_back({v, y, x});
                }
            }
        });
        size_t sumSz = 0;
        for (const auto& lv : ets) sumSz += lv.size();
        outPts.clear();
        outPts.reserve(sumSz);
        for (const auto& lv : ets) outPts.insert(outPts.end(), lv.begin(), lv.end());
    });
}

void step4_1_tbb_tbbvec(int W, int H, const std::vector<float>& Hmap, float kf, int threads,
                        oneapi::tbb::concurrent_vector<HarrisPoint>& outPts) {
    const int total = W * H;
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        MinMaxPair init{std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
        MinMaxPair mm = oneapi::tbb::parallel_reduce(
            oneapi::tbb::blocked_range<int>(0, total), init,
            [&](const oneapi::tbb::blocked_range<int>& r, MinMaxPair cur) {
                for (int i = r.begin(); i < r.end(); ++i) {
                    if (Hmap[i] < cur.mn) cur.mn = Hmap[i];
                    if (Hmap[i] > cur.mx) cur.mx = Hmap[i];
                }
                return cur;
            },
            [](MinMaxPair a, MinMaxPair b) {
                return MinMaxPair{std::min(a.mn, b.mn), std::max(a.mx, b.mx)};
            }
        );
        const float thr = (mm.mx - mm.mn) * kf + mm.mn;

        outPts.clear();
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            for (int y = r.begin(); y < r.end(); ++y) {
                const size_t row = static_cast<size_t>(y) * W;
                for (int x = 0; x < W; ++x) {
                    float v = Hmap[row + x];
                    if (v > thr) outPts.push_back({v, y, x});
                }
            }
        });
    });
}

// ----------------------------------------------------------------------------
// ШАГ 4.2: Сортировка HarrisPoints по убыванию R
// ----------------------------------------------------------------------------
template <typename Container>
void ompMergeSortRec(Container& arr, size_t left, size_t right, int depth) {
    if (right - left <= 1024 || depth <= 0) {
        std::sort(arr.begin() + left, arr.begin() + right, HarrisPointDesc());
        return;
    }
    size_t mid = left + (right - left) / 2;
    #pragma omp task shared(arr)
    {
        ompMergeSortRec(arr, left, mid, depth - 1);
    }
    #pragma omp task shared(arr)
    {
        ompMergeSortRec(arr, mid, right, depth - 1);
    }
    #pragma omp taskwait
    std::inplace_merge(arr.begin() + left, arr.begin() + mid, arr.begin() + right, HarrisPointDesc());
}

template <typename Container>
void step4_2_seq(Container& pts) {
    std::sort(pts.begin(), pts.end(), HarrisPointDesc());
}

template <typename Container>
void step4_2_omp(Container& pts, int threads) {
    if (pts.size() <= 1) return;
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp single
        {
            ompMergeSortRec(pts, 0, pts.size(), 4);
        }
    }
}

template <typename Container>
void step4_2_tbb(Container& pts, int threads) {
    if (pts.size() <= 1) return;
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_sort(pts.begin(), pts.end(), HarrisPointDesc());
    });
}

// ----------------------------------------------------------------------------
// ШАГ 4.3: Отсечение точек с меньшим R в заданном радиусе rang (rang >= 20)
// Для каждой точки i проверяем наличие более сильной точки j < i на расстоянии < rang
// ----------------------------------------------------------------------------
template <typename InContainer>
inline bool isSuppressed(const InContainer& pts, size_t i, int rangSq) {
    const int yi = pts[i].y;
    const int xi = pts[i].x;
    for (size_t j = 0; j < i; ++j) {
        const int dy = yi - pts[j].y;
        const int dx = xi - pts[j].x;
        if (dx * dx + dy * dy < rangSq) return true;
    }
    return false;
}

void step4_3_seq_stl(const std::vector<HarrisPoint>& pts, int rang,
                     std::vector<HarrisPoint>& outVs) {
    outVs.clear();
    const int rangSq = rang * rang;
    for (size_t i = 0; i < pts.size(); ++i) {
        if (!isSuppressed(pts, i, rangSq)) outVs.push_back(pts[i]);
    }
}

void step4_3_omp_stl(const std::vector<HarrisPoint>& pts, int rang, int threads,
                     std::vector<HarrisPoint>& outVs) {
    const int rangSq = rang * rang;
    const int n = static_cast<int>(pts.size());
    std::vector<std::vector<HarrisPoint>> localVecs(threads);
    #pragma omp parallel num_threads(threads)
    {
        int tid = omp_get_thread_num();
        auto& loc = localVecs[tid];
        #pragma omp for schedule(dynamic, 32)
        for (int i = 0; i < n; ++i) {
            if (!isSuppressed(pts, static_cast<size_t>(i), rangSq)) loc.push_back(pts[i]);
        }
    }
    outVs.clear();
    for (const auto& lv : localVecs) outVs.insert(outVs.end(), lv.begin(), lv.end());
}

void step4_3_omp_tbbvec(const oneapi::tbb::concurrent_vector<HarrisPoint>& pts, int rang, int threads,
                        oneapi::tbb::concurrent_vector<HarrisPoint>& outVs) {
    const int rangSq = rang * rang;
    const int n = static_cast<int>(pts.size());
    outVs.clear();
    #pragma omp parallel for num_threads(threads) schedule(dynamic, 32)
    for (int i = 0; i < n; ++i) {
        if (!isSuppressed(pts, static_cast<size_t>(i), rangSq)) outVs.push_back(pts[i]);
    }
}

void step4_3_tbb_stl(const std::vector<HarrisPoint>& pts, int rang, int threads,
                     std::vector<HarrisPoint>& outVs) {
    const int rangSq = rang * rang;
    const size_t n = pts.size();
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::enumerable_thread_specific<std::vector<HarrisPoint>> ets;
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, n, 32), [&](const auto& r) {
            auto& loc = ets.local();
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (!isSuppressed(pts, i, rangSq)) loc.push_back(pts[i]);
            }
        });
        outVs.clear();
        for (const auto& lv : ets) outVs.insert(outVs.end(), lv.begin(), lv.end());
    });
}

void step4_3_tbb_tbbvec(const oneapi::tbb::concurrent_vector<HarrisPoint>& pts, int rang, int threads,
                        oneapi::tbb::concurrent_vector<HarrisPoint>& outVs) {
    const int rangSq = rang * rang;
    const size_t n = pts.size();
    outVs.clear();
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, n, 32), [&](const auto& r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                if (!isSuppressed(pts, i, rangSq)) outVs.push_back(pts[i]);
            }
        });
    });
}

// ----------------------------------------------------------------------------
// ШАГ 5: Визуализация особых точек (копирование + закрашивание окрестности rang=1 красным)
// ----------------------------------------------------------------------------
template <typename Container>
void step5_seq(const ImageRGB& src, const Container& pts, int visRang, ImageRGB& dst) {
    const int W = src.width, H = src.height;
    const size_t n = static_cast<size_t>(W) * H;
    dst.width = W;
    dst.height = H;
    dst.r.resize(n);
    dst.g.resize(n);
    dst.b.resize(n);
    std::memcpy(dst.r.data(), src.r.data(), n);
    std::memcpy(dst.g.data(), src.g.data(), n);
    std::memcpy(dst.b.data(), src.b.data(), n);
    for (size_t i = 0; i < pts.size(); ++i) {
        const int cy = pts[i].y;
        const int cx = pts[i].x;
        for (int dy = -visRang; dy <= visRang; ++dy) {
            int py = cy + dy;
            if (py < 0 || py >= H) continue;
            for (int dx = -visRang; dx <= visRang; ++dx) {
                int px = cx + dx;
                if (px < 0 || px >= W) continue;
                size_t idx = static_cast<size_t>(py) * W + px;
                dst.r[idx] = 255;
                dst.g[idx] = 0;
                dst.b[idx] = 0;
            }
        }
    }
}

template <typename Container>
void step5_omp(const ImageRGB& src, const Container& pts, int visRang, int threads, ImageRGB& dst) {
    const int W = src.width, H = src.height;
    const size_t n = static_cast<size_t>(W) * H;
    dst.width = W;
    dst.height = H;
    dst.r.resize(n);
    dst.g.resize(n);
    dst.b.resize(n);
    const int numPts = static_cast<int>(pts.size());
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp for schedule(static)
        for (int y = 0; y < H; ++y) {
            const size_t off = static_cast<size_t>(y) * W;
            std::memcpy(&dst.r[off], &src.r[off], W);
            std::memcpy(&dst.g[off], &src.g[off], W);
            std::memcpy(&dst.b[off], &src.b[off], W);
        }
        #pragma omp for schedule(static)
        for (int i = 0; i < numPts; ++i) {
            const int cy = pts[i].y;
            const int cx = pts[i].x;
            for (int dy = -visRang; dy <= visRang; ++dy) {
                int py = cy + dy;
                if (py < 0 || py >= H) continue;
                for (int dx = -visRang; dx <= visRang; ++dx) {
                    int px = cx + dx;
                    if (px < 0 || px >= W) continue;
                    size_t idx = static_cast<size_t>(py) * W + px;
                    dst.r[idx] = 255;
                    dst.g[idx] = 0;
                    dst.b[idx] = 0;
                }
            }
        }
    }
}

template <typename Container>
void step5_tbb(const ImageRGB& src, const Container& pts, int visRang, int threads, ImageRGB& dst) {
    const int W = src.width, H = src.height;
    const size_t n = static_cast<size_t>(W) * H;
    dst.width = W;
    dst.height = H;
    dst.r.resize(n);
    dst.g.resize(n);
    dst.b.resize(n);
    oneapi::tbb::task_arena arena(threads);
    arena.execute([&]() {
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<int>(0, H), [&](const auto& r) {
            for (int y = r.begin(); y < r.end(); ++y) {
                const size_t off = static_cast<size_t>(y) * W;
                std::memcpy(&dst.r[off], &src.r[off], W);
                std::memcpy(&dst.g[off], &src.g[off], W);
                std::memcpy(&dst.b[off], &src.b[off], W);
            }
        });
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, pts.size()), [&](const auto& r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                const int cy = pts[i].y;
                const int cx = pts[i].x;
                for (int dy = -visRang; dy <= visRang; ++dy) {
                    int py = cy + dy;
                    if (py < 0 || py >= H) continue;
                    for (int dx = -visRang; dx <= visRang; ++dx) {
                        int px = cx + dx;
                        if (px < 0 || px >= W) continue;
                        size_t idx = static_cast<size_t>(py) * W + px;
                        dst.r[idx] = 255;
                        dst.g[idx] = 0;
                        dst.b[idx] = 0;
                    }
                }
            }
        });
    });
}

// ----------------------------------------------------------------------------
// Замер серии запусков с доверительным интервалом AvgTrustedIntervalMed
// Для быстрых шагов (< 20 мс) выполняется пакетирование повторов внутри omp_get_wtime(),
// что устраняет квантование таймера (omp_get_wtick = 1 мс) при сохранении чистого omp_get_wtime().
// ----------------------------------------------------------------------------
template <typename Fn>
int calibrateBatchReps(Fn&& fn, double targetBracketMs = 24.0, int maxReps = 200) {
    fn(); // прогрев
    int reps = 1;
    while (reps < maxReps) {
        double t0 = omp_get_wtime();
        for (int i = 0; i < reps; ++i) fn();
        double dtMs = (omp_get_wtime() - t0) * 1000.0;
        if (dtMs >= targetBracketMs) break;
        if (dtMs < 1.0) {
            reps = std::min(maxReps, reps * 8);
        } else {
            int est = static_cast<int>(std::ceil(reps * (targetBracketMs / dtMs)));
            reps = std::min(maxReps, std::max(reps + 1, est));
        }
    }
    return reps;
}

template <typename Fn>
TrustedStats measureSeries(Fn&& fn, int runs, int batchReps = 1, std::vector<double>* rawOut = nullptr) {
    // Прогревочный запуск
    fn();
    std::vector<double> samples(runs);
    for (int r = 0; r < runs; ++r) {
        double t0 = omp_get_wtime();
        for (int b = 0; b < batchReps; ++b) {
            fn();
        }
        double t1 = omp_get_wtime();
        samples[r] = ((t1 - t0) * 1000.0) / static_cast<double>(batchReps); // мс на 1 вызов
    }
    if (rawOut) *rawOut = samples;
    double* ptr = samples.data();
    return computeTrustedStats(ptr, runs);
}

struct StepRecord {
    int ds_id;
    std::string resolution;
    std::string step_id;
    std::string step_name;
    std::string backend;   // Seq, OpenMP, oneTBB
    std::string container; // None, STL, TBB_ConcVec
    int threads;
    int runs;
    TrustedStats st;
    double speedup;
    double efficiency;
    double cost_ms;
};

struct TotalRecord {
    int ds_id;
    std::string resolution;
    std::string impl_id;
    std::string impl_name;
    int threads;
    double total_ms;
    double speedup;
    double efficiency;
    double cost_ms;
    size_t pts_after_thr;
    size_t pts_final;
};

void printTable61Header(int ds_id, const std::string& res) {
    std::cout << "\n--- НД " << ds_id << " (" << res << ") — Таблица 6.1 (по функциональным блокам) ---\n";
    std::cout << padRightU8("Блок / реализация", 34) << " "
              << padLeftU8("Пот.", 4) << " "
              << padLeftU8("Зам.", 4) << " "
              << padLeftU8("Исх.ср.(мс)", 11) << " "
              << padLeftU8("Отбр.", 5) << " "
              << padLeftU8("Итог(мс)", 10) << " "
              << padLeftU8("Sp(n)", 7) << " "
              << padLeftU8("Ep(n)", 7) << " "
              << padLeftU8("Cp(мс)", 10) << "\n";
    std::cout << std::string(100, '-') << "\n";
}

void printTable61Row(const StepRecord& r) {
    std::string label = r.step_id + "_" + r.backend;
    if (r.container != "None") label += "_" + r.container;
    std::cout << padRightU8(label, 34) << " "
              << padLeftU8(std::to_string(r.threads), 4) << " "
              << padLeftU8(std::to_string(r.runs), 4) << " "
              << padLeftU8(fmtNum(r.st.rawAvg, 4), 11) << " "
              << padLeftU8(std::to_string(r.st.discardedRuns), 5) << " "
              << padLeftU8(fmtNum(r.st.trustedAvgMed, 4), 10) << " "
              << padLeftU8(fmtNum(r.speedup, 4), 7) << " "
              << padLeftU8(fmtNum(r.efficiency, 4), 7) << " "
              << padLeftU8(fmtNum(r.cost_ms, 4), 10) << "\n";
}

void printTable62Header(int ds_id, const std::string& res) {
    std::cout << "\n--- НД " << ds_id << " (" << res << ") — Таблица 6.2 (суммарное время формирования карты особых точек) ---\n";
    std::cout << padRightU8("Полная реализация", 40) << " "
              << padLeftU8("Пот.", 4) << " "
              << padLeftU8("Итог(мс)", 10) << " "
              << padLeftU8("Sp(n)", 7) << " "
              << padLeftU8("Ep(n)", 7) << " "
              << padLeftU8("Cp(мс)", 10) << " "
              << padLeftU8("Точ.(пор/финал)", 16) << "\n";
    std::cout << std::string(100, '-') << "\n";
}

void printTable62Row(const TotalRecord& r) {
    std::string ptsStr = std::to_string(r.pts_after_thr) + "/" + std::to_string(r.pts_final);
    std::cout << padRightU8(r.impl_name, 40) << " "
              << padLeftU8(std::to_string(r.threads), 4) << " "
              << padLeftU8(fmtNum(r.total_ms, 4), 10) << " "
              << padLeftU8(fmtNum(r.speedup, 4), 7) << " "
              << padLeftU8(fmtNum(r.efficiency, 4), 7) << " "
              << padLeftU8(fmtNum(r.cost_ms, 4), 10) << " "
              << padLeftU8(ptsStr, 16) << "\n";
}

std::string resolvePath(const std::string& preferred, const std::string& fallback) {
    std::ifstream f(preferred, std::ios::binary);
    if (f.good()) return preferred;
    return fallback;
}

} // namespace

int main(int argc, char* argv[]) {
    bool fastMode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--fast") fastMode = true;
    }

    const int runsMain = fastMode ? 3 : 12;   // >= 10 запусков по методичке
    const int runsCI   = fastMode ? 15 : 100; // контрольная серия 100 запусков для графика ДИ
    const std::vector<int> threadList = {2, 3, 4};
    const std::vector<float> gKer = makeGaussian1D(5);

    std::cout << "====================================================================================================\n";
    std::cout << "  Лабораторная работа №6: Детектор особых точек Харриса в OpenMP и Intel oneTBB\n";
    std::cout << "====================================================================================================\n";
    std::cout << "Стандарт OpenMP:      " << _OPENMP << "\n";
    std::cout << "Intel oneTBB потоков: " << oneapi::tbb::info::default_concurrency() << "\n";
    std::cout << "Точность таймера:     omp_get_wtick() = " << fmtNum(omp_get_wtick() * 1000.0, 6) << " мс\n";
    std::cout << "Фильтрация выбросов:  AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n";
    std::cout << "Параметры Харриса:    k = 0.04, kf = " << fmtNum(HARRIS_KF, 2)
              << ", rang = " << HARRIS_RANG << " пикс., Гаусс Ksize = 5, маркер rang = " << VIS_RANG << "\n";
    std::cout << "Запусков на серию:    " << runsMain << " (контрольная серия ДИ: " << runsCI << ")\n\n";

    std::vector<ImageRGB> datasets(3);
    const std::string imgNames[3] = {"input_1.bmp", "input_2.bmp", "input_3.bmp"};
    std::string outDir = "Lab6/images/";
    std::string resDir = "Lab6/results/";
    {
        std::ifstream chk("Lab6/images/input_1.bmp", std::ios::binary);
        if (!chk.good()) {
            outDir = "";
            resDir = "";
        }
    }

    for (int i = 0; i < 3; ++i) {
        std::string p = resolvePath("Lab6/images/" + imgNames[i], imgNames[i]);
        if (!datasets[i].load(p)) {
            std::cerr << "Ошибка загрузки " << p << "\n";
            return 1;
        }
        datasets[i].id = i + 1;
        datasets[i].label = std::to_string(datasets[i].width) + "x" + std::to_string(datasets[i].height);
        std::cout << "Загружен НД " << (i + 1) << ": " << p << " (" << datasets[i].label
                  << ", " << (datasets[i].width * datasets[i].height) << " пикс., 24-bit RGB)\n";
    }

    std::vector<StepRecord> allSteps;
    std::vector<TotalRecord> allTotals;
    std::vector<double> ciSamples;

    for (const auto& img : datasets) {
        const int W = img.width;
        const int H = img.height;
        const size_t N = static_cast<size_t>(W) * H;

        std::vector<float> rawI(N), tmpT(N), smoothI(N);
        std::vector<float> tmpX(N), tmpY(N), diffX(N), diffY(N), diffXY(N);
        std::vector<float> Hmap(N);

        std::vector<HarrisPoint> ptsThrSTL, ptsSortedSTL, ptsFinalSTL;
        oneapi::tbb::concurrent_vector<HarrisPoint> ptsThrTBB, ptsSortedTBB, ptsFinalTBB;
        ImageRGB visOut;

        printTable61Header(img.id, img.label);

        auto addStep = [&](const std::string& sid, const std::string& sname,
                           const std::string& backend, const std::string& cont,
                           int th, int runs, const TrustedStats& st, double t1) {
            double sp = (st.trustedAvgMed > 0.0) ? (t1 / st.trustedAvgMed) : 1.0;
            double ep = sp / th;
            double cp = th * st.trustedAvgMed;
            StepRecord rec{img.id, img.label, sid, sname, backend, cont, th, runs, st, sp, ep, cp};
            allSteps.push_back(rec);
            printTable61Row(rec);
            return st.trustedAvgMed;
        };

        // Хранилище времён для сборки Таблицы 6.2
        // Ключ: step_id + "_" + backend + "_" + container + "_" + p
        struct StepTimes {
            double seq = 0.0;
            double omp[5] = {};
            double tbb[5] = {};
        };
        StepTimes tS1, tS2, tS3, tS41_stl, tS41_tbbv, tS42_stl, tS42_tbbv, tS43_stl, tS43_tbbv, tS5;

        // ---------------- ШАГ 1: Яркость + Гаусс ----------------
        {
            auto fnSeq = [&]() { step1_seq(img, gKer, rawI, tmpT, smoothI); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS1.seq = addStep("Step1_GrayGauss", "Шаг 1: Яркость и Гаусс", "Seq", "None", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            for (int p : threadList) {
                auto st = measureSeries([&]() { step1_omp(img, gKer, p, rawI, tmpT, smoothI); }, runsMain, reps);
                tS1.omp[p] = addStep("Step1_GrayGauss", "Шаг 1: Яркость и Гаусс", "OpenMP", "None", p, runsMain, st, tS1.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step1_tbb(img, gKer, p, rawI, tmpT, smoothI); }, runsMain, reps);
                tS1.tbb[p] = addStep("Step1_GrayGauss", "Шаг 1: Яркость и Гаусс", "oneTBB", "None", p, runsMain, st, tS1.seq);
            }
            step1_seq(img, gKer, rawI, tmpT, smoothI);
        }

        // ---------------- ШАГ 2: Градиенты 1-го и 2-го порядка ----------------
        {
            auto fnSeq = [&]() { step2_seq(W, H, smoothI, tmpX, tmpY, diffX, diffY, diffXY); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS2.seq = addStep("Step2_Gradients", "Шаг 2: Градиенты DiffX/Y/XY", "Seq", "None", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            for (int p : threadList) {
                auto st = measureSeries([&]() { step2_omp(W, H, smoothI, p, tmpX, tmpY, diffX, diffY, diffXY); }, runsMain, reps);
                tS2.omp[p] = addStep("Step2_Gradients", "Шаг 2: Градиенты DiffX/Y/XY", "OpenMP", "None", p, runsMain, st, tS2.seq);
            }
            for (int p : threadList) {
                bool collectCI = (img.id == 3 && p == 4);
                int rCnt = collectCI ? runsCI : runsMain;
                auto st = measureSeries([&]() { step2_tbb(W, H, smoothI, p, tmpX, tmpY, diffX, diffY, diffXY); },
                                        rCnt, reps, collectCI ? &ciSamples : nullptr);
                tS2.tbb[p] = addStep("Step2_Gradients", "Шаг 2: Градиенты DiffX/Y/XY", "oneTBB", "None", p, rCnt, st, tS2.seq);
            }
            step2_seq(W, H, smoothI, tmpX, tmpY, diffX, diffY, diffXY);
        }

        // ---------------- ШАГ 3: Отклик угла R (массив H) ----------------
        {
            auto fnSeq = [&]() { step3_seq(W, H, diffX, diffY, diffXY, Hmap); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS3.seq = addStep("Step3_ResponseR", "Шаг 3: Отклик угла R", "Seq", "None", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            for (int p : threadList) {
                auto st = measureSeries([&]() { step3_omp(W, H, p, diffX, diffY, diffXY, Hmap); }, runsMain, reps);
                tS3.omp[p] = addStep("Step3_ResponseR", "Шаг 3: Отклик угла R", "OpenMP", "None", p, runsMain, st, tS3.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step3_tbb(W, H, p, diffX, diffY, diffXY, Hmap); }, runsMain, reps);
                tS3.tbb[p] = addStep("Step3_ResponseR", "Шаг 3: Отклик угла R", "oneTBB", "None", p, runsMain, st, tS3.seq);
            }
            step3_seq(W, H, diffX, diffY, diffXY, Hmap);
        }

        // ---------------- ШАГ 4.1: Порог threshold и сбор HarrisPoints ----------------
        {
            auto fnSeq = [&]() { step4_1_seq_stl(W, H, Hmap, HARRIS_KF, ptsThrSTL); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS41_stl.seq = addStep("Step4_1_Collect", "Шаг 4.1: Порог и сбор точек", "Seq", "STL", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            tS41_tbbv.seq = tS41_stl.seq;

            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_1_omp_stl(W, H, Hmap, HARRIS_KF, p, ptsThrSTL); }, runsMain, reps);
                tS41_stl.omp[p] = addStep("Step4_1_Collect", "Шаг 4.1: Порог и сбор точек", "OpenMP", "STL", p, runsMain, st, tS41_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_1_tbb_stl(W, H, Hmap, HARRIS_KF, p, ptsThrSTL); }, runsMain, reps);
                tS41_stl.tbb[p] = addStep("Step4_1_Collect", "Шаг 4.1: Порог и сбор точек", "oneTBB", "STL", p, runsMain, st, tS41_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_1_omp_tbbvec(W, H, Hmap, HARRIS_KF, p, ptsThrTBB); }, runsMain, reps);
                tS41_tbbv.omp[p] = addStep("Step4_1_Collect", "Шаг 4.1: Порог и сбор точек", "OpenMP", "TBBVec", p, runsMain, st, tS41_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_1_tbb_tbbvec(W, H, Hmap, HARRIS_KF, p, ptsThrTBB); }, runsMain, reps);
                tS41_tbbv.tbb[p] = addStep("Step4_1_Collect", "Шаг 4.1: Порог и сбор точек", "oneTBB", "TBBVec", p, runsMain, st, tS41_stl.seq);
            }
            step4_1_seq_stl(W, H, Hmap, HARRIS_KF, ptsThrSTL);
            ptsThrTBB.assign(ptsThrSTL.begin(), ptsThrSTL.end());
        }

        // ---------------- ШАГ 4.2: Сортировка HarrisPoints по убыванию R ----------------
        {
            auto fnSeq = [&]() { ptsSortedSTL = ptsThrSTL; step4_2_seq(ptsSortedSTL); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS42_stl.seq = addStep("Step4_2_Sort", "Шаг 4.2: Сортировка по R", "Seq", "STL", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            tS42_tbbv.seq = tS42_stl.seq;

            for (int p : threadList) {
                auto st = measureSeries([&]() { ptsSortedSTL = ptsThrSTL; step4_2_omp(ptsSortedSTL, p); }, runsMain, reps);
                tS42_stl.omp[p] = addStep("Step4_2_Sort", "Шаг 4.2: Сортировка по R", "OpenMP", "STL", p, runsMain, st, tS42_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { ptsSortedSTL = ptsThrSTL; step4_2_tbb(ptsSortedSTL, p); }, runsMain, reps);
                tS42_stl.tbb[p] = addStep("Step4_2_Sort", "Шаг 4.2: Сортировка по R", "oneTBB", "STL", p, runsMain, st, tS42_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { ptsSortedTBB = ptsThrTBB; step4_2_omp(ptsSortedTBB, p); }, runsMain, reps);
                tS42_tbbv.omp[p] = addStep("Step4_2_Sort", "Шаг 4.2: Сортировка по R", "OpenMP", "TBBVec", p, runsMain, st, tS42_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { ptsSortedTBB = ptsThrTBB; step4_2_tbb(ptsSortedTBB, p); }, runsMain, reps);
                tS42_tbbv.tbb[p] = addStep("Step4_2_Sort", "Шаг 4.2: Сортировка по R", "oneTBB", "TBBVec", p, runsMain, st, tS42_stl.seq);
            }
            ptsSortedSTL = ptsThrSTL;
            step4_2_seq(ptsSortedSTL);
            ptsSortedTBB.assign(ptsSortedSTL.begin(), ptsSortedSTL.end());
        }

        // ---------------- ШАГ 4.3: Отсечение точек в радиусе rang ----------------
        {
            auto fnSeq = [&]() { step4_3_seq_stl(ptsSortedSTL, HARRIS_RANG, ptsFinalSTL); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS43_stl.seq = addStep("Step4_3_NMS", "Шаг 4.3: Отсечение в радиусе", "Seq", "STL", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            tS43_tbbv.seq = tS43_stl.seq;

            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_3_omp_stl(ptsSortedSTL, HARRIS_RANG, p, ptsFinalSTL); }, runsMain, reps);
                tS43_stl.omp[p] = addStep("Step4_3_NMS", "Шаг 4.3: Отсечение в радиусе", "OpenMP", "STL", p, runsMain, st, tS43_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_3_tbb_stl(ptsSortedSTL, HARRIS_RANG, p, ptsFinalSTL); }, runsMain, reps);
                tS43_stl.tbb[p] = addStep("Step4_3_NMS", "Шаг 4.3: Отсечение в радиусе", "oneTBB", "STL", p, runsMain, st, tS43_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_3_omp_tbbvec(ptsSortedTBB, HARRIS_RANG, p, ptsFinalTBB); }, runsMain, reps);
                tS43_tbbv.omp[p] = addStep("Step4_3_NMS", "Шаг 4.3: Отсечение в радиусе", "OpenMP", "TBBVec", p, runsMain, st, tS43_stl.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step4_3_tbb_tbbvec(ptsSortedTBB, HARRIS_RANG, p, ptsFinalTBB); }, runsMain, reps);
                tS43_tbbv.tbb[p] = addStep("Step4_3_NMS", "Шаг 4.3: Отсечение в радиусе", "oneTBB", "TBBVec", p, runsMain, st, tS43_stl.seq);
            }
            step4_3_seq_stl(ptsSortedSTL, HARRIS_RANG, ptsFinalSTL);
            step4_3_tbb_tbbvec(ptsSortedTBB, HARRIS_RANG, 4, ptsFinalTBB);
        }

        // ---------------- ШАГ 5: Визуализация особых точек ----------------
        {
            auto fnSeq = [&]() { step5_seq(img, ptsFinalSTL, VIS_RANG, visOut); };
            int reps = calibrateBatchReps(fnSeq);
            auto stSeq = measureSeries(fnSeq, runsMain, reps);
            tS5.seq = addStep("Step5_Visualize", "Шаг 5: Визуализация точек", "Seq", "None", 1, runsMain, stSeq, stSeq.trustedAvgMed);
            for (int p : threadList) {
                auto st = measureSeries([&]() { step5_omp(img, ptsFinalSTL, VIS_RANG, p, visOut); }, runsMain, reps);
                tS5.omp[p] = addStep("Step5_Visualize", "Шаг 5: Визуализация точек", "OpenMP", "None", p, runsMain, st, tS5.seq);
            }
            for (int p : threadList) {
                auto st = measureSeries([&]() { step5_tbb(img, ptsFinalSTL, VIS_RANG, p, visOut); }, runsMain, reps);
                tS5.tbb[p] = addStep("Step5_Visualize", "Шаг 5: Визуализация точек", "oneTBB", "None", p, runsMain, st, tS5.seq);
            }
        }

        // Сохранение выходных BMP (output_X_var.bmp) для проверки корректности (Задание 6.1)
        const std::string pfx = outDir + "output_" + std::to_string(img.id) + "_";
        step5_seq(img, ptsFinalSTL, VIS_RANG, visOut);
        visOut.save(pfx + "seq.bmp");
        step5_omp(img, ptsFinalSTL, VIS_RANG, 4, visOut);
        visOut.save(pfx + "omp_stl.bmp");
        step5_omp(img, ptsFinalTBB, VIS_RANG, 4, visOut);
        visOut.save(pfx + "omp_tbbvec.bmp");
        step5_tbb(img, ptsFinalSTL, VIS_RANG, 4, visOut);
        visOut.save(pfx + "tbb_stl.bmp");
        step5_tbb(img, ptsFinalTBB, VIS_RANG, 4, visOut);
        visOut.save(pfx + "tbb_tbbvec.bmp");

        // ---------------- ТАБЛИЦА 6.2: Суммарное время по всем 7 реализациям ----------------
        printTable62Header(img.id, img.label);

        const double seqTotal = tS1.seq + tS2.seq + tS3.seq + tS41_stl.seq + tS42_stl.seq + tS43_stl.seq + tS5.seq;
        const size_t nThr = ptsThrSTL.size();
        const size_t nFinSTL = ptsFinalSTL.size();
        const size_t nFinTBB = ptsFinalTBB.size();

        auto addTotal = [&](const std::string& iid, const std::string& iname, int th, double totMs, size_t nFin) {
            double sp = (totMs > 0.0) ? (seqTotal / totMs) : 1.0;
            double ep = sp / th;
            double cp = th * totMs;
            TotalRecord tr{img.id, img.label, iid, iname, th, totMs, sp, ep, cp, nThr, nFin};
            allTotals.push_back(tr);
            printTable62Row(tr);
        };

        addTotal("Seq", "Sequential (посл.)", 1, seqTotal, nFinSTL);

        for (int p : threadList) {
            double tot = tS1.omp[p] + tS2.omp[p] + tS3.omp[p] + tS41_stl.omp[p] + tS42_stl.omp[p] + tS43_stl.omp[p] + tS5.omp[p];
            addTotal("OpenMP_STL", "OpenMP (Шаг 4: STL vector)", p, tot, nFinSTL);
        }
        for (int p : threadList) {
            double tot = tS1.omp[p] + tS2.omp[p] + tS3.omp[p] + tS41_tbbv.omp[p] + tS42_tbbv.omp[p] + tS43_tbbv.omp[p] + tS5.omp[p];
            addTotal("OpenMP_TBBVec", "OpenMP (Шаг 4: oneTBB conc_vec)", p, tot, nFinTBB);
        }
        for (int p : threadList) {
            double tot = tS1.tbb[p] + tS2.tbb[p] + tS3.tbb[p] + tS41_stl.tbb[p] + tS42_stl.tbb[p] + tS43_stl.tbb[p] + tS5.tbb[p];
            addTotal("oneTBB_STL", "Intel oneTBB (Шаг 4: STL vector)", p, tot, nFinSTL);
        }
        for (int p : threadList) {
            double tot = tS1.tbb[p] + tS2.tbb[p] + tS3.tbb[p] + tS41_tbbv.tbb[p] + tS42_tbbv.tbb[p] + tS43_tbbv.tbb[p] + tS5.tbb[p];
            addTotal("oneTBB_TBBVec", "Intel oneTBB (Шаг 4: oneTBB conc_vec)", p, tot, nFinTBB);
        }
        // Гибрид OpenMP + Intel oneTBB:
        // Шаги 1, 2, 3, 5 — регулярные проходы (OpenMP static), Шаги 4.1, 4.2 (parallel_sort), 4.3 — Intel oneTBB (work-stealing)
        for (int p : threadList) {
            double tot = tS1.omp[p] + tS2.omp[p] + tS3.omp[p] + tS41_stl.tbb[p] + tS42_stl.tbb[p] + tS43_stl.tbb[p] + tS5.omp[p];
            addTotal("Hybrid_STL", "OpenMP+oneTBB (Шаг 4: STL vector)", p, tot, nFinSTL);
        }
        for (int p : threadList) {
            double tot = tS1.omp[p] + tS2.omp[p] + tS3.omp[p] + tS41_tbbv.tbb[p] + tS42_tbbv.tbb[p] + tS43_tbbv.tbb[p] + tS5.omp[p];
            addTotal("Hybrid_TBBVec", "OpenMP+oneTBB (Шаг 4: oneTBB conc_vec)", p, tot, nFinTBB);
        }
    }

    // Сохранение CSV для Excel и графиков
    {
        std::ofstream f(resDir + "lab6_table61.csv");
        f << "ds_id,resolution,step_id,step_name,backend,container,threads,runs,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,speedup,efficiency,cost_ms\n";
        for (const auto& r : allSteps) {
            f << r.ds_id << "," << r.resolution << "," << r.step_id << ",\"" << r.step_name << "\","
              << r.backend << "," << r.container << "," << r.threads << "," << r.runs << ","
              << fmtNum(r.st.rawAvg, 6) << "," << fmtNum(r.st.median, 6) << "," << fmtNum(r.st.sd, 6) << ","
              << r.st.keptRuns << "," << r.st.discardedRuns << ","
              << fmtNum(r.st.trustedAvgMed, 6) << "," << fmtNum(r.speedup, 6) << ","
              << fmtNum(r.efficiency, 6) << "," << fmtNum(r.cost_ms, 6) << "\n";
        }
    }
    {
        std::ofstream f(resDir + "lab6_table62.csv");
        f << "ds_id,resolution,impl_id,impl_name,threads,total_ms,speedup,efficiency,cost_ms,pts_after_thr,pts_final\n";
        for (const auto& r : allTotals) {
            f << r.ds_id << "," << r.resolution << "," << r.impl_id << ",\"" << r.impl_name << "\","
              << r.threads << "," << fmtNum(r.total_ms, 6) << "," << fmtNum(r.speedup, 6) << ","
              << fmtNum(r.efficiency, 6) << "," << fmtNum(r.cost_ms, 6) << ","
              << r.pts_after_thr << "," << r.pts_final << "\n";
        }
    }
    {
        std::ofstream f(resDir + "lab6_raw_samples.csv");
        f << "run_index,time_ms\n";
        for (size_t i = 0; i < ciSamples.size(); ++i) {
            f << (i + 1) << "," << fmtNum(ciSamples[i], 6) << "\n";
        }
    }

    std::cout << "\nРезультаты ЛР №6 и выходные BMP-изображения (output_X_var.bmp) сохранены успешно.\n";
    return 0;
}

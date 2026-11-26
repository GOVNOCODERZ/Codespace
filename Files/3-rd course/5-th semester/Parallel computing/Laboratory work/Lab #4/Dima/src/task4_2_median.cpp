#include "BMPFileRW.h"
#include "common_stats.h"

#include <Windows.h>
#include <omp.h>
#include <oneapi/tbb.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
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

// Быстрая 3-сторонняя сортировка qsort из ЛР №3 для малых окон окрестности (25..169 элементов)
static inline void quickSortU8(unsigned char* __restrict arr, int left, int right) {
    while (left < right) {
        if (right - left <= 12) {
            for (int i = left + 1; i <= right; i++) {
                unsigned char key = arr[i];
                int j = i - 1;
                while (j >= left && arr[j] > key) {
                    arr[j + 1] = arr[j];
                    j--;
                }
                arr[j + 1] = key;
            }
            return;
        }
        unsigned char pivot = arr[left + ((right - left) >> 1)];
        int lt = left;
        int i = left;
        int gt = right;
        while (i <= gt) {
            if (arr[i] < pivot) {
                unsigned char t = arr[lt]; arr[lt] = arr[i]; arr[i] = t;
                lt++; i++;
            } else if (arr[i] > pivot) {
                unsigned char t = arr[i]; arr[i] = arr[gt]; arr[gt] = t;
                gt--;
            } else {
                i++;
            }
        }
        if (lt - left < right - gt) {
            quickSortU8(arr, left, lt - 1);
            left = gt + 1;
        } else {
            quickSortU8(arr, gt + 1, right);
            right = lt - 1;
        }
    }
}

// Планарное представление RGB (SoA) для максимальной локальности кэша
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
                row[x].rgbtRed = r[base + static_cast<size_t>(x)];
                row[x].rgbtGreen = g[base + static_cast<size_t>(x)];
                row[x].rgbtBlue = b[base + static_cast<size_t>(x)];
            }
        }
    }

    bool equals(const PlanarRGB& other) const {
        return width == other.width && height == other.height &&
               r == other.r && g == other.g && b == other.b;
    }
};

// Обработка одной строки канала с приватным буфером окна medMas
static inline void filterRowChannel(const std::vector<unsigned char>& inCh,
                                    std::vector<unsigned char>& outCh,
                                    int y, int width, int height, int radius, int ksize,
                                    unsigned char* medMas) {
    const int area = ksize * ksize;
    const int mid = area / 2;
    unsigned char* outRow = &outCh[static_cast<size_t>(y) * static_cast<size_t>(width)];
    const bool innerY = (y >= radius && y < height - radius);

    for (int x = 0; x < width; x++) {
        if (innerY && x >= radius && x < width - radius) {
            unsigned char* ptr = medMas;
            for (int dy = -radius; dy <= radius; dy++) {
                std::memcpy(ptr,
                            &inCh[static_cast<size_t>(y + dy) * static_cast<size_t>(width) + static_cast<size_t>(x - radius)],
                            static_cast<size_t>(ksize));
                ptr += ksize;
            }
        } else {
            int idx = 0;
            for (int dy = -radius; dy <= radius; dy++) {
                int ky = std::clamp(y + dy, 0, height - 1);
                const unsigned char* inRow = &inCh[static_cast<size_t>(ky) * static_cast<size_t>(width)];
                for (int dx = -radius; dx <= radius; dx++) {
                    int kx = std::clamp(x + dx, 0, width - 1);
                    medMas[idx++] = inRow[kx];
                }
            }
        }
        quickSortU8(medMas, 0, area - 1);
        outRow[x] = medMas[mid];
    }
}

// 1. Последовательный медианный фильтр
static void medianFilterSeq(const PlanarRGB& src, PlanarRGB& dst, int ksize) {
    const int radius = ksize / 2;
    unsigned char medMas[169]; // Максимум 13x13 = 169
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };
    for (int ch = 0; ch < 3; ch++) {
        for (int y = 0; y < src.height; y++) {
            filterRowChannel(*inCh[ch], *outCh[ch], y, src.width, src.height, radius, ksize, medMas);
        }
    }
}

// 2. Параллельный медианный фильтр OpenMP
static void medianFilterOpenMP(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    const int radius = ksize / 2;
    const int width = src.width;
    const int height = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    #pragma omp parallel num_threads(numThreads) default(none) shared(inCh, outCh, width, height, radius, ksize)
    {
        unsigned char medMas[169];
        for (int ch = 0; ch < 3; ch++) {
            #pragma omp for schedule(dynamic, 8) nowait
            for (int y = 0; y < height; y++) {
                filterRowChannel(*inCh[ch], *outCh[ch], y, width, height, radius, ksize, medMas);
            }
        }
    }
}

// 3. Параллельный медианный фильтр Intel oneTBB (parallel_for + task_arena)
static void medianFilterTBB(const PlanarRGB& src, PlanarRGB& dst, int ksize, int numThreads) {
    const int radius = ksize / 2;
    const int width = src.width;
    const int height = src.height;
    const std::vector<unsigned char>* inCh[3] = { &src.r, &src.g, &src.b };
    std::vector<unsigned char>* outCh[3] = { &dst.r, &dst.g, &dst.b };

    oneapi::tbb::task_arena arena(numThreads);
    arena.execute([&]() {
        // Объединяем все 3*height строк в единое пространство задач для балансировки work-stealing
        oneapi::tbb::parallel_for(
            oneapi::tbb::blocked_range<int>(0, 3 * height, 8),
            [&](const oneapi::tbb::blocked_range<int>& range) {
                unsigned char medMas[169];
                for (int idx = range.begin(); idx < range.end(); idx++) {
                    int ch = idx / height;
                    int y = idx % height;
                    filterRowChannel(*inCh[ch], *outCh[ch], y, width, height, radius, ksize, medMas);
                }
            }
        );
    });
}

static void savePlanarBMP(const PlanarRGB& img, const std::string& path) {
    RGBTRIPLE** rgb = BMPAlloc(img.width, img.height);
    img.toRGBTRIPLE(rgb);
    BMPWrite(rgb, img.width, img.height, path.c_str());
    BMPFree(rgb);
}

struct ImgDataset {
    int id = 0;
    int width = 0;
    int height = 0;
    std::string path;
    PlanarRGB src;
};

struct MedRow {
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

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "==================================================================================================\n";
    std::cout << "  Лабораторная работа №4 — Задание 4.2: Медианный фильтр BMP (OpenMP и Intel oneTBB)\n";
    std::cout << "==================================================================================================\n";
    std::cout << "Стандарт OpenMP:      " << _OPENMP << "\n";
    std::cout << "Intel oneTBB потоков: " << oneapi::tbb::info::default_concurrency() << "\n";
    std::cout << "Метод сортировки:     3-сторонний QuickSort (из ЛР №3), планарное представление RGB (SoA)\n";
    std::cout << "Фильтрация выбросов:  AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n\n";

    std::vector<ImgDataset> datasets;
    for (int i = 1; i <= 3; i++) {
        ImgDataset ds;
        ds.id = i;
        ds.path = "Lab4/images/input_" + std::to_string(i) + ".bmp";
        RGBTRIPLE** rgb = nullptr;
        BITMAPFILEHEADER fh{};
        BITMAPINFOHEADER ih{};
        BMPRead(rgb, fh, ih, ds.path.c_str());
        ds.width = ih.biWidth;
        ds.height = ih.biHeight;
        ds.src.fromRGBTRIPLE(rgb, ds.width, ds.height);
        BMPFree(rgb);
        std::cout << "Загружен НД " << i << ": " << ds.path << " (" << ds.width << "x" << ds.height
                  << ", " << (ds.width * ds.height) << " пикс., 24-bit RGB)\n";
        datasets.push_back(std::move(ds));
    }

    const std::vector<int> kernels = { 5, 9, 13 };
    const std::vector<int> parThreads = { 2, 3, 4 };
    const int RUNS = 15;

    std::vector<MedRow> allRows;

    for (const auto& ds : datasets) {
        std::cout << "\n--- НД " << ds.id << " (" << ds.width << "x" << ds.height << ") ---\n";
        std::cout << padRightU8("Реализация", 22)
                  << padLeftU8("Ksize", 6)
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

        PlanarRGB dst;
        dst.allocate(ds.width, ds.height);

        for (int ksize : kernels) {
            PlanarRGB refOut;
            refOut.allocate(ds.width, ds.height);
            medianFilterTBB(ds.src, refOut, ksize, 6);

            auto runCase = [&](const std::string& impl, int p, double t1Ms, auto fn) -> double {
                // Прогрев + верификация побитового совпадения
                fn();
                if (!dst.equals(refOut)) {
                    throw std::runtime_error("Несовпадение выходного BMP: " + impl);
                }

                double* times = new double[RUNS];
                for (int r = 0; r < RUNS; r++) {
                    double t0 = omp_get_wtime();
                    fn();
                    double t1 = omp_get_wtime();
                    times[r] = (t1 - t0) * 1000.0;
                }

                MedRow row;
                row.imgId = ds.id;
                row.res = std::to_string(ds.width) + "x" + std::to_string(ds.height);
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

                std::cout << padRightU8(impl, 22)
                          << std::right << std::fixed
                          << std::setw(6) << ksize
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

            // 1. Последовательная версия
            double t1Ms = runCase("Median_Seq", 1, 0.0, [&]() { medianFilterSeq(ds.src, dst, ksize); });

            // 2. OpenMP (2, 3, 4 потока)
            for (int p : parThreads) {
                runCase("Median_OpenMP", p, t1Ms, [&]() { medianFilterOpenMP(ds.src, dst, ksize, p); });
            }
            savePlanarBMP(dst, "Lab4/images/output_" + std::to_string(ds.id) + "_omp_k" + std::to_string(ksize) + ".bmp");
            if (ksize == 9) {
                savePlanarBMP(dst, "Lab4/images/output_" + std::to_string(ds.id) + "_omp.bmp");
            }

            // 3. Intel oneTBB (2, 3, 4 потока)
            for (int p : parThreads) {
                runCase("Median_oneTBB", p, t1Ms, [&]() { medianFilterTBB(ds.src, dst, ksize, p); });
            }
            savePlanarBMP(dst, "Lab4/images/output_" + std::to_string(ds.id) + "_tbb_k" + std::to_string(ksize) + ".bmp");
            if (ksize == 9) {
                savePlanarBMP(dst, "Lab4/images/output_" + std::to_string(ds.id) + "_tbb.bmp");
            }
        }
    }

    std::ofstream out("Lab4/results/task4_2_summary.csv");
    out << "img_id,resolution,impl_name,ksize,threads,runs,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,speedup,efficiency,cost_ms\n";
    out << std::fixed << std::setprecision(6);
    for (const auto& r : allRows) {
        out << r.imgId << "," << r.res << "," << r.impl << "," << r.ksize << "," << r.threads << ","
            << r.runs << "," << r.stats.rawAvg << "," << r.stats.median << "," << r.stats.sd << ","
            << r.stats.keptRuns << "," << r.stats.discardedRuns << ","
            << r.stats.trustedAvgMed << "," << r.speedup << "," << r.efficiency << "," << r.costMs << "\n";
    }
    out.close();

    std::cout << "\nРезультаты задания 4.2 и отфильтрованные BMP сохранены успешно.\n";
    return 0;
}

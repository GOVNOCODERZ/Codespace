#include "BMPFileRW.h"
#include "common_stats.h"

#include <Windows.h>
#include <omp.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Подсчёт видимой ширины UTF-8 строки для ровного форматирования консольных таблиц
static size_t utf8Len(const std::string& s) {
    size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0u) != 0x80u) {
            count++;
        }
    }
    return count;
}

static std::string padRightU8(const std::string& s, size_t width) {
    size_t len = utf8Len(s);
    return (len < width) ? (s + std::string(width - len, ' ')) : s;
}

static std::string padLeftU8(const std::string& s, size_t width) {
    size_t len = utf8Len(s);
    return (len < width) ? (std::string(width - len, ' ') + s) : s;
}

// ============================================================================
// Алгоритмы сортировки из задания 3.1, адаптированные для канала unsigned char
// ============================================================================

// Сортировка 1: метод Шелла (последовательность шагов Кнута для малых окон 49..225)
static inline void shellSortU8(unsigned char* __restrict arr, int n) {
    static const int gaps[] = { 40, 13, 4, 1 };
    for (int g = 0; g < 4; g++) {
        int h = gaps[g];
        if (h >= n) {
            continue;
        }
        for (int i = h; i < n; i++) {
            if (arr[i - h] > arr[i]) {
                unsigned char key = arr[i];
                int j = i;
                do {
                    arr[j] = arr[j - h];
                    j -= h;
                } while (j >= h && arr[j - h] > key);
                arr[j] = key;
            }
        }
    }
}

// Сортировка 2: быстрая сортировка qsort (3-стороннее разбиение Дейкстры + cutoff вставками)
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
                unsigned char tmp = arr[lt];
                arr[lt] = arr[i];
                arr[i] = tmp;
                lt++;
                i++;
            } else if (arr[i] > pivot) {
                unsigned char tmp = arr[i];
                arr[i] = arr[gt];
                arr[gt] = tmp;
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

// ============================================================================
// Планарное представление RGB-изображения (SoA) для оптимизации кэш-памяти
// ============================================================================
struct PlanarRGB {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> r;
    std::vector<unsigned char> g;
    std::vector<unsigned char> b;

    void allocate(int w, int h) {
        width = w;
        height = h;
        size_t total = static_cast<size_t>(w) * static_cast<size_t>(h);
        r.resize(total);
        g.resize(total);
        b.resize(total);
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

enum class SortAlg {
    Shell,
    QuickSort
};

static inline void sortWindow(unsigned char* buf, int area, SortAlg alg) {
    if (alg == SortAlg::Shell) {
        shellSortU8(buf, area);
    } else {
        quickSortU8(buf, 0, area - 1);
    }
}

// ============================================================================
// Последовательный медианный фильтр по трём цветовым каналам (R, G, B)
// ============================================================================
static void medianFilterSeq(const PlanarRGB& src, PlanarRGB& dst, int ksize, SortAlg alg) {
    const int width = src.width;
    const int height = src.height;
    const int radius = ksize / 2;
    const int area = ksize * ksize;
    const int mid = area / 2;

    unsigned char medMas[225]; // Максимальный размер окрестности 15x15 = 225

    auto filterChannel = [&](const std::vector<unsigned char>& inCh, std::vector<unsigned char>& outCh) {
        for (int y = 0; y < height; y++) {
            unsigned char* outRow = &outCh[static_cast<size_t>(y) * static_cast<size_t>(width)];
            for (int x = 0; x < width; x++) {
                // Быстрая ветвь для внутренних пикселей (без проверки границ)
                if (y >= radius && y < height - radius && x >= radius && x < width - radius) {
                    unsigned char* ptr = medMas;
                    for (int dy = -radius; dy <= radius; dy++) {
                        std::memcpy(ptr,
                                    &inCh[static_cast<size_t>(y + dy) * static_cast<size_t>(width) + static_cast<size_t>(x - radius)],
                                    static_cast<size_t>(ksize));
                        ptr += ksize;
                    }
                } else {
                    // Обработка граничных пикселей с усечением координат (clamp)
                    int masInd = 0;
                    for (int dy = -radius; dy <= radius; dy++) {
                        int ky = y + dy;
                        if (ky < 0) ky = 0;
                        if (ky > height - 1) ky = height - 1;
                        const unsigned char* inRow = &inCh[static_cast<size_t>(ky) * static_cast<size_t>(width)];
                        for (int dx = -radius; dx <= radius; dx++) {
                            int kx = x + dx;
                            if (kx < 0) kx = 0;
                            if (kx > width - 1) kx = width - 1;
                            medMas[masInd++] = inRow[kx];
                        }
                    }
                }
                sortWindow(medMas, area, alg);
                outRow[x] = medMas[mid];
            }
        }
    };

    filterChannel(src.r, dst.r);
    filterChannel(src.g, dst.g);
    filterChannel(src.b, dst.b);
}

// ============================================================================
// Параллельный медианный фильтр OpenMP (распараллеливание циклов по строкам)
// ============================================================================
static void medianFilterPar(const PlanarRGB& src, PlanarRGB& dst, int ksize, SortAlg alg, int numThreads) {
    const int width = src.width;
    const int height = src.height;
    const int radius = ksize / 2;
    const int area = ksize * ksize;
    const int mid = area / 2;

    // Единая параллельная область для обработки всех трёх каналов R, G, B.
    // Каждый поток использует собственный стековый буфер medMas (исключается false sharing)
    // и записывает данные в непересекающиеся диапазоны строк выходного изображения.
    #pragma omp parallel num_threads(numThreads) default(none) shared(src, dst, width, height, radius, ksize, area, mid, alg)
    {
        unsigned char medMas[225];

        const std::vector<unsigned char>* inChannels[3] = { &src.r, &src.g, &src.b };
        std::vector<unsigned char>* outChannels[3] = { &dst.r, &dst.g, &dst.b };

        for (int ch = 0; ch < 3; ch++) {
            const std::vector<unsigned char>& inCh = *inChannels[ch];
            std::vector<unsigned char>& outCh = *outChannels[ch];

            // Динамическое распределение блоков по 8 строк балансирует нагрузку на неоднородных сценах
            #pragma omp for schedule(dynamic, 8) nowait
            for (int y = 0; y < height; y++) {
                unsigned char* outRow = &outCh[static_cast<size_t>(y) * static_cast<size_t>(width)];
                for (int x = 0; x < width; x++) {
                    if (y >= radius && y < height - radius && x >= radius && x < width - radius) {
                        unsigned char* ptr = medMas;
                        for (int dy = -radius; dy <= radius; dy++) {
                            std::memcpy(ptr,
                                        &inCh[static_cast<size_t>(y + dy) * static_cast<size_t>(width) + static_cast<size_t>(x - radius)],
                                        static_cast<size_t>(ksize));
                            ptr += ksize;
                        }
                    } else {
                        int masInd = 0;
                        for (int dy = -radius; dy <= radius; dy++) {
                            int ky = y + dy;
                            if (ky < 0) ky = 0;
                            if (ky > height - 1) ky = height - 1;
                            const unsigned char* inRow = &inCh[static_cast<size_t>(ky) * static_cast<size_t>(width)];
                            for (int dx = -radius; dx <= radius; dx++) {
                                int kx = x + dx;
                                if (kx < 0) kx = 0;
                                if (kx > width - 1) kx = width - 1;
                                medMas[masInd++] = inRow[kx];
                            }
                        }
                    }
                    sortWindow(medMas, area, alg);
                    outRow[x] = medMas[mid];
                }
            }
        }
    }
}

struct ImageDataset {
    int id = 0;
    std::string inputPath;
    int width = 0;
    int height = 0;
    PlanarRGB src;
};

struct MedianRow {
    int imgId = 0;
    std::string resolution;
    int pixels = 0;
    std::string algo;
    int ksize = 0;
    int threads = 1;
    int runs = 0;
    TrustedStats stats;
    double speedup = 1.0;
    double efficiency = 1.0;
    double costMs = 0.0;
    std::vector<double> rawTimesMs;
};

static MedianRow benchmarkMedian(const ImageDataset& ds,
                                 const PlanarRGB& refOut,
                                 SortAlg alg,
                                 const std::string& algoName,
                                 int ksize,
                                 int threads,
                                 int runs) {
    PlanarRGB dst;
    dst.allocate(ds.width, ds.height);

    // Прогрев кэша и потоков OpenMP (не включается в замеры)
    if (threads == 1) {
        medianFilterSeq(ds.src, dst, ksize, alg);
    } else {
        medianFilterPar(ds.src, dst, ksize, alg, threads);
    }
    if (!refOut.r.empty() && !dst.equals(refOut)) {
        throw std::runtime_error("Расхождение результата фильтрации: " + algoName + " th=" + std::to_string(threads));
    }

    double* timesMs = new double[runs];
    std::vector<double> rawCopy(static_cast<size_t>(runs));

    for (int r = 0; r < runs; r++) {
        double tStart = omp_get_wtime();
        if (threads == 1) {
            medianFilterSeq(ds.src, dst, ksize, alg);
        } else {
            medianFilterPar(ds.src, dst, ksize, alg, threads);
        }
        double tEnd = omp_get_wtime();

        double elapsedMs = (tEnd - tStart) * 1000.0;
        timesMs[r] = elapsedMs;
        rawCopy[static_cast<size_t>(r)] = elapsedMs;
    }

    MedianRow row;
    row.imgId = ds.id;
    row.resolution = std::to_string(ds.width) + "x" + std::to_string(ds.height);
    row.pixels = ds.width * ds.height;
    row.algo = algoName;
    row.ksize = ksize;
    row.threads = threads;
    row.runs = runs;
    row.stats = computeTrustedStats(timesMs, runs);
    row.rawTimesMs = std::move(rawCopy);
    delete[] timesMs;
    return row;
}

static void savePlanarBMP(const PlanarRGB& img, const std::string& outPath) {
    RGBTRIPLE** rgb = BMPAlloc(img.width, img.height);
    img.toRGBTRIPLE(rgb);
    BMPWrite(rgb, img.width, img.height, outPath.c_str());
    BMPFree(rgb);
}

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);

    bool demoMode = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--demo") {
            demoMode = true;
        }
    }

    std::cout << "==================================================================================================\n";
    std::cout << "  Лабораторная работа №3 — Задание 3.2: Параллельная медианная фильтрация BMP-изображений (OpenMP)\n";
    std::cout << "==================================================================================================\n";
    std::cout << "Стандарт OpenMP:      " << _OPENMP << "\n";
    std::cout << "Доступно потоков CPU: " << omp_get_max_threads() << "\n";
    std::cout << "Точность таймера:     omp_get_wtick() = " << std::fixed << std::setprecision(6)
              << (omp_get_wtick() * 1000.0) << " мс\n";
    std::cout << "Алгоритмы сортировки: Сортировка Шелла (Shell) и Быстрая сортировка (QuickSort)\n";
    std::cout << "Метод фильтрации:     AvgTrustedIntervalMed (доверительный интервал med +/- SD)\n\n";

    // Загрузка четырёх входных BMP-изображений через модуль BMPFileRW.h
    std::vector<ImageDataset> datasets;
    int numImages = demoMode ? 2 : 4;
    for (int idx = 1; idx <= numImages; idx++) {
        ImageDataset ds;
        ds.id = idx;
        ds.inputPath = "images/input_" + std::to_string(idx) + ".bmp";
        RGBTRIPLE** rgb = nullptr;
        BITMAPFILEHEADER fh{};
        BITMAPINFOHEADER ih{};
        BMPRead(rgb, fh, ih, ds.inputPath.c_str());
        ds.width = ih.biWidth;
        ds.height = ih.biHeight;
        ds.src.fromRGBTRIPLE(rgb, ds.width, ds.height);
        BMPFree(rgb);
        std::cout << "Загружен НД " << idx << ": " << ds.inputPath << " (" << ds.width << "x" << ds.height
                  << ", " << (ds.width * ds.height) << " пикс., 24-bit RGB)\n";
        datasets.push_back(std::move(ds));
    }

    std::vector<int> kernels = { 7, 11, 15 };
    std::vector<int> baseThreads = { 1, 2, 3, 4 };
    std::vector<int> extThreads = { 1, 2, 3, 4, 6, 8, 12, 16 };
    int runsMain = demoMode ? 3 : 15;

    struct AlgEntry { SortAlg alg; std::string name; std::string shortTag; };
    std::vector<AlgEntry> algos = {
        { SortAlg::Shell, "Median_Shell", "shell" },
        { SortAlg::QuickSort, "Median_QuickSort", "qsort" }
    };

    std::vector<MedianRow> allRows;

    for (const auto& ds : datasets) {
        std::cout << "\n--- НД " << ds.id << " (" << ds.width << "x" << ds.height << ") ---\n";
        std::cout << padRightU8("Реализация", 20)
                  << padLeftU8("Ksize", 6)
                  << padLeftU8("Пот.", 6)
                  << padLeftU8("Зам.", 6)
                  << padLeftU8("Исх.ср.(мс)", 14)
                  << padLeftU8("Отбр.", 7)
                  << padLeftU8("Итог(мс)", 14)
                  << padLeftU8("Sp(n)", 9)
                  << padLeftU8("Ep(n)", 9)
                  << padLeftU8("Cp(мс)", 14)
                  << "\n";
        std::cout << std::string(105, '-') << "\n";

        for (int ksize : kernels) {
            // Получаем эталонное отфильтрованное изображение и сохраняем output_X_alg.bmp
            PlanarRGB refOut;
            refOut.allocate(ds.width, ds.height);
            medianFilterPar(ds.src, refOut, ksize, SortAlg::QuickSort, std::min(6, omp_get_max_threads()));

            for (const auto& ae : algos) {
                // Сохраняем выходные BMP-файлы (output_X_alg_kY.bmp и канонические output_X_alg.bmp для K=7)
                if (!demoMode) {
                    PlanarRGB outVerify;
                    outVerify.allocate(ds.width, ds.height);
                    medianFilterPar(ds.src, outVerify, ksize, ae.alg, std::min(6, omp_get_max_threads()));
                    if (!outVerify.equals(refOut)) {
                        throw std::runtime_error("Несовпадение выходных BMP между алгоритмами сортировки!");
                    }
                    std::string outPathK = "images/output_" + std::to_string(ds.id) + "_" + ae.shortTag + "_k" + std::to_string(ksize) + ".bmp";
                    savePlanarBMP(outVerify, outPathK);
                    if (ksize == 7) {
                        std::string outPathMain = "images/output_" + std::to_string(ds.id) + "_" + ae.shortTag + ".bmp";
                        savePlanarBMP(outVerify, outPathMain);
                    }
                }

                // Для НД 4 (1280x720) проводим расширенное исследование масштабируемости до 16 потоков
                const auto& thList = (!demoMode && ds.id == 4) ? extThreads : baseThreads;
                double t1Ms = 0.0;

                for (int p : thList) {
                    // Для графика доверительного интервала (110 замеров) делаем расширенную серию на НД 1, K=7, p=4
                    int runs = runsMain;
                    if (!demoMode && ds.id == 1 && ksize == 7 && p == 4) {
                        runs = 110;
                    }

                    MedianRow row = benchmarkMedian(ds, refOut, ae.alg, ae.name, ksize, p, runs);
                    if (p == 1) {
                        t1Ms = row.stats.trustedAvgMed;
                    }
                    row.speedup = (row.stats.trustedAvgMed > 0.0) ? (t1Ms / row.stats.trustedAvgMed) : 1.0;
                    row.efficiency = row.speedup / static_cast<double>(p);
                    row.costMs = static_cast<double>(p) * row.stats.trustedAvgMed;

                    std::ostringstream oss;
                    oss << std::fixed
                        << padRightU8(row.algo + (p == 1 ? "_Seq" : "_Par"), 20)
                        << std::setw(6) << row.ksize
                        << std::setw(6) << row.threads
                        << std::setw(6) << row.runs
                        << std::setw(14) << std::setprecision(4) << row.stats.rawAvg
                        << std::setw(7) << row.stats.discardedRuns
                        << std::setw(14) << std::setprecision(4) << row.stats.trustedAvgMed
                        << std::setw(9) << std::setprecision(4) << row.speedup
                        << std::setw(9) << std::setprecision(4) << row.efficiency
                        << std::setw(14) << std::setprecision(4) << row.costMs;
                    std::cout << oss.str() << "\n" << std::flush;

                    allRows.push_back(std::move(row));
                }
            }
        }
    }

    if (!demoMode) {
        std::ofstream sumOut("results/task3_2_summary.csv");
        sumOut << "img_id,resolution,pixels,algo,ksize,threads,runs,raw_avg_ms,median_ms,sd_ms,kept_runs,discarded_runs,trusted_avg_med_ms,trusted_avg_mean_ms,speedup,efficiency,cost_ms\n";
        sumOut << std::fixed << std::setprecision(6);
        for (const auto& r : allRows) {
            sumOut << r.imgId << "," << r.resolution << "," << r.pixels << ","
                   << r.algo << "," << r.ksize << "," << r.threads << "," << r.runs << ","
                   << r.stats.rawAvg << "," << r.stats.median << "," << r.stats.sd << ","
                   << r.stats.keptRuns << "," << r.stats.discardedRuns << ","
                   << r.stats.trustedAvgMed << "," << r.stats.trustedAvgMean << ","
                   << r.speedup << "," << r.efficiency << "," << r.costMs << "\n";
        }
        sumOut.close();

        std::ofstream rawOut("results/task3_2_raw_samples.csv");
        rawOut << "img_id,resolution,algo,ksize,threads,run_idx,time_ms,median_ms,sd_ms,in_interval\n";
        rawOut << std::fixed << std::setprecision(6);
        for (const auto& r : allRows) {
            bool saveRaw = (r.imgId == 1 && r.ksize == 7 && r.threads == 4) ||
                           (r.imgId == 4 && r.ksize == 15 && (r.threads == 1 || r.threads == 4));
            if (!saveRaw) continue;
            for (size_t i = 0; i < r.rawTimesMs.size(); i++) {
                double val = r.rawTimesMs[i];
                int inInt = (val >= r.stats.median - r.stats.sd && val <= r.stats.median + r.stats.sd) ? 1 : 0;
                rawOut << r.imgId << "," << r.resolution << "," << r.algo << ","
                       << r.ksize << "," << r.threads << "," << (i + 1) << ","
                       << val << "," << r.stats.median << "," << r.stats.sd << "," << inInt << "\n";
            }
        }
        rawOut.close();
        std::cout << "\nВыходные изображения output_X_alg.bmp и таблицы результатов сохранены успешно.\n";
    }

    return 0;
}

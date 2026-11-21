// main_4_2.cpp
// Задание 4.2. Медианная фильтрация изображения: последовательная версия +
// один параллельный вариант на OpenMP + один параллельный вариант на
// Intel oneTBB (по заданию — "рекомендуется выбрать наиболее быстрый
// вариант" алгоритма сортировки из лабораторной работы 3).
//
// Выбран алгоритм сортировки Шелла (ShellSortPlain): асимптотически
// быстрее чет-нечетной перестановки (O(n log n) против O(n^2) для окна
// размером до 13x13=169 элементов), поэтому ожидается, что он будет
// "наиболее быстрым вариантом". Если по факту замеров из лабораторной
// работы 3 на твоём ПК окажется быстрее чет-нечетная перестановка —
// просто смени OddEvenSortPlain на ShellSortPlain в вызовах ниже (сигнатуры
// идентичны) и пересобери.
//
// Сборка (после инициализации окружения oneAPI в терминале):
//   icx /Qiopenmp /EHsc /std:c++17 main_4_2.cpp /Fe:median_app4.exe ^
//       /I"%TBBROOT%\include" /link "%TBBROOT%\lib\intel64\tbb12.lib"
#include <omp.h>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "common.h"
#include "bmp_io.h"
#include "median_filter_omp.h"
#include "median_filter_tbb.h"

static const int RUNS_PARALLEL = 15; // >= 15 по заданию
static const int RUNS_SEQ      = 5;  // достаточно для эталонного замера
static const int KSIZES[3] = { 5, 9, 13 };
static const int THREADS[3] = { 2, 3, 4 };
static const int NUM_IMAGES = 3; // НД1, НД2, НД3 (по заданию 1280x720..2560x1440)

// Выбранный алгоритм сортировки для фильтра (см. комментарий выше).
static void (*SortFn)(double*, size_t) = ShellSortPlain;
static const char* SortTag = "shell";

static void RunImage(int imgIdx, const std::string& inPath, std::ofstream& csv) {
    RGBTRIPLE** src = nullptr;
    BITMAPFILEHEADER fh{}; BITMAPINFOHEADER ih{};
    BMPRead(src, fh, ih, inPath.c_str());
    if (!src) { std::cerr << "Не удалось открыть " << inPath << "\n"; return; }
    int width = ih.biWidth, height = ih.biHeight;
    std::cout << "\n=== Изображение " << imgIdx << " (" << width << "x" << height << ") ===\n";

    RGBTRIPLE** dst = new RGBTRIPLE*[height];
    dst[0] = new RGBTRIPLE[(size_t)width * height];
    for (int i = 1; i < height; i++) dst[i] = &dst[0][(size_t)width * i];

    for (int Ksize : KSIZES) {
        // --- Последовательная версия (база T1) ---
        std::vector<double> times(RUNS_SEQ);
        for (int r = 0; r < RUNS_SEQ; r++)
            times[r] = MedianFilterPass(src, dst, width, height, Ksize, 0, SortFn);
        double T1 = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
        std::cout << "Ksize=" << Ksize << " (посл., " << SortTag << "): t="
                   << std::fixed << std::setprecision(4) << T1 << " c\n";
        csv << "posl;" << SortTag << ";" << imgIdx << ";" << width << "x" << height
            << ";-;" << Ksize << ";" << T1 << ";1.0\n";

        if (Ksize == 9) {
            std::string outPath = "../images_out/output_" + std::to_string(imgIdx) + "_" + SortTag + "_posl.bmp";
            BMPWrite(dst, width, height, outPath.c_str());
            std::cout << "  -> сохранено " << outPath << "\n";
        }

        // --- OpenMP (парал. по строкам) ---
        for (int th : THREADS) {
            std::vector<double> pt(RUNS_PARALLEL);
            for (int r = 0; r < RUNS_PARALLEL; r++)
                pt[r] = MedianFilterPass(src, dst, width, height, Ksize, th, SortFn);
            double t = AvgTrustedIntervalMed(pt.data(), RUNS_PARALLEL);
            double sp = T1 / t;
            std::cout << "Ksize=" << Ksize << " (OpenMP, p=" << th << "): t=" << t << " c, Sp=" << sp << "\n";
            csv << "OpenMP;" << SortTag << ";" << imgIdx << ";" << width << "x" << height
                << ";" << th << ";" << Ksize << ";" << t << ";" << sp << "\n";
            if (Ksize == 9 && th == THREADS[0]) {
                std::string outPath = "../images_out/output_" + std::to_string(imgIdx) + "_" + SortTag + "_openmp.bmp";
                BMPWrite(dst, width, height, outPath.c_str());
                std::cout << "  -> сохранено " << outPath << "\n";
            }
        }

        // --- oneTBB (парал. по строкам, тот же гранулярный уровень) ---
        for (int th : THREADS) {
            std::vector<double> pt(RUNS_PARALLEL);
            for (int r = 0; r < RUNS_PARALLEL; r++)
                pt[r] = MedianFilterPassTbb(src, dst, width, height, Ksize, th, SortFn);
            double t = AvgTrustedIntervalMed(pt.data(), RUNS_PARALLEL);
            double sp = T1 / t;
            std::cout << "Ksize=" << Ksize << " (oneTBB, p=" << th << "): t=" << t << " c, Sp=" << sp << "\n";
            csv << "oneTBB;" << SortTag << ";" << imgIdx << ";" << width << "x" << height
                << ";" << th << ";" << Ksize << ";" << t << ";" << sp << "\n";
            if (Ksize == 9 && th == THREADS[0]) {
                std::string outPath = "../images_out/output_" + std::to_string(imgIdx) + "_" + SortTag + "_tbb.bmp";
                BMPWrite(dst, width, height, outPath.c_str());
                std::cout << "  -> сохранено " << outPath << "\n";
            }
        }
    }

    BMPFree(src);
    delete[] dst[0];
    delete[] dst;
}

int main() {
    std::cout << "Задание 4.2. Медианная фильтрация (посл. + OpenMP + oneTBB)\n";
    std::cout << "Выбранный алгоритм сортировки окна: " << SortTag << "\n";
    std::cout << "omp_get_num_procs() = " << omp_get_num_procs() << "\n";

    std::ofstream csv("../data/median4_results.csv");
    csv << "mode;sort;image;resolution;threads;Ksize;time_s;Sp\n";

    for (int img = 1; img <= NUM_IMAGES; img++) {
        std::string inPath = "../images_in/input_" + std::to_string(img) + ".bmp";
        RunImage(img, inPath, csv);
    }

    csv.close();
    std::cout << "\nГотово. Результаты сохранены в data/median4_results.csv\n";
    return 0;
}

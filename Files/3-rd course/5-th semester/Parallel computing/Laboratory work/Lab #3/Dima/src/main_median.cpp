// main_median.cpp
// Задание 3.2. Медианная фильтрация RGB-изображения (BMP, 24 бита).
// Сборка: g++ -O3 -march=native -fopenmp -std=c++17 main_median.cpp -o median_app
#include <windows.h>
#include <omp.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "common.h"
#include "bmp_io.h"
#include "median_filter.h"

static const int RUNS_PARALLEL = 15; // >= 15 требуемых по заданию
static const int RUNS_SEQ      = 15; // По заданию "не менее 15 запусков для каждой реализации"
static const int KSIZES[3]     = { 7, 11, 15 }; // По Таблице 3.2 методички: 7, 11, 15!
static const int THREADS[3]    = { 2, 3, 4 };
static const int NUM_IMAGES    = 4;

// Ползунок прогресса в консоли
static void PrintProgress(const std::string& label, int current, int total) {
    int percent = (current * 100) / total;
    const int barWidth = 20;
    int filled = (percent * barWidth) / 100;

    std::cout << "\r" << label << " [";
    for (int i = 0; i < barWidth; i++) {
        std::cout << (i < filled ? '#' : '.');
    }
    std::cout << "] " << std::setw(3) << percent << "% (" << current << "/" << total << ")" << std::flush;

    if (current == total) {
        std::cout << "\r" << std::string(75, ' ') << "\r" << std::flush;
    }
}

template <typename SortFn>
static void RunAlgoOnImage(const char* algoName, const char* algoTag, SortFn sortFn,
                            int imgIdx, const std::string& inPath,
                            std::ofstream& csv, bool saveOutput) {
    RGBTRIPLE** src = nullptr;
    BITMAPFILEHEADER fh{}; BITMAPINFOHEADER ih{};
    BMPRead(src, fh, ih, inPath.c_str());
    if (!src) {
        std::cerr << "Не удалось открыть " << inPath << "\n";
        return;
    }
    int width = ih.biWidth, height = ih.biHeight;
    std::cout << "\n--- Изображение " << imgIdx << " (" << width << "x" << height
              << "), алгоритм: " << algoName << " ---\n";

    RGBTRIPLE** dst = new RGBTRIPLE*[height];
    dst[0] = new RGBTRIPLE[(size_t)width * height];
    for (int i = 1; i < height; i++) dst[i] = &dst[0][(size_t)width * i];

    for (int ks = 0; ks < 3; ks++) {
        int Ksize = KSIZES[ks];

        // --- Последовательный замер (база T1 для расчета Sp) ---
        std::vector<double> times(RUNS_SEQ);
        std::string seqLabel = "Ksize=" + std::to_string(Ksize) + " (посл.)";
        PrintProgress(seqLabel, 0, RUNS_SEQ);
        for (int r = 0; r < RUNS_SEQ; r++) {
            times[r] = MedianFilterPass(src, dst, width, height, Ksize, 0, sortFn);
            PrintProgress(seqLabel, r + 1, RUNS_SEQ);
        }
        double T1 = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
        std::cout << "Ksize=" << Ksize << " (посл.): t=" << std::fixed << std::setprecision(4) << T1 << " c\n";
        csv << algoTag << ";" << imgIdx << ";" << width << "x" << height << ";-;" << Ksize << ";" << T1 << ";1.0\n";

        if (saveOutput && ks == 1) { // сохраняем картинку для Ksize=11 (средний вариант)
            std::string outPath = "../images_out/output_" + std::to_string(imgIdx) + "_" + algoTag + ".bmp";
            BMPWrite(dst, width, height, outPath.c_str());
            std::cout << "  -> сохранено " << outPath << "\n";
        }

        // --- Параллельные замеры ---
        for (int th = 0; th < 3; th++) {
            int nth = THREADS[th];
            std::vector<double> ptimes(RUNS_PARALLEL);
            std::string parLabel = "Ksize=" + std::to_string(Ksize) + " (p=" + std::to_string(nth) + ")";
            PrintProgress(parLabel, 0, RUNS_PARALLEL);
            for (int r = 0; r < RUNS_PARALLEL; r++) {
                ptimes[r] = MedianFilterPass(src, dst, width, height, Ksize, nth, sortFn);
                PrintProgress(parLabel, r + 1, RUNS_PARALLEL);
            }
            double pavg = AvgTrustedIntervalMed(ptimes.data(), RUNS_PARALLEL);
            double sp = T1 / pavg;
            std::cout << "Ksize=" << Ksize << " (парал., p=" << nth << "): t=" << pavg
                      << " c, Sp=" << sp << "\n";
            csv << algoTag << ";" << imgIdx << ";" << width << "x" << height << ";" << nth << ";" << Ksize << ";" << pavg << ";" << sp << "\n";
        }
    }

    BMPFree(src);
    delete[] dst[0];
    delete[] dst;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "Задание 3.2. Медианная фильтрация изображений (OpenMP)\n";
    std::cout << "omp_get_num_procs() = " << omp_get_num_procs() << "\n";
    std::cout << "omp_get_max_threads() = " << omp_get_max_threads() << "\n";

    std::ofstream csv("../data/median_results.csv");
    csv << std::fixed << std::setprecision(4);
    csv << "algo;image;resolution;threads;Ksize;time_s;Sp\n";

// Алгоритм 1: Сортировка Шелла
    for (int img = 1; img <= NUM_IMAGES; img++) {
        std::string inPath = "../images_in/input_" + std::to_string(img) + ".bmp";
        RunAlgoOnImage("Шелла", "shell", ShellSortPlain, img, inPath, csv, true);
    }

    // Алгоритм 2: Быстрая сортировка (qsort)
    for (int img = 1; img <= NUM_IMAGES; img++) {
        std::string inPath = "../images_in/input_" + std::to_string(img) + ".bmp";
        RunAlgoOnImage("Быстрая сортировка", "qsort", QuickSortPlain, img, inPath, csv, true);
    }

    csv.close();
    std::cout << "\nГотово. Результаты сохранены в data/median_results.csv\n";
    return 0;
}
// main_4_3.cpp
// Задание 4.3. Расчёт и визуализация карт текстурных признаков
// (M2 - момент 2-го порядка, R - относительная гладкость, U - однородность,
// E - энтропия) — последовательная версия, OpenMP, Intel oneTBB.
//
// Параметры порогов k1 [0.25,0.4], k2 [0.6,0.75] — задаются константами
// ниже (K1, K2); при желании можно заменить на чтение из std::cin.
//
// Сборка (после инициализации окружения oneAPI в терминале):
//   icx /Qiopenmp /EHsc /std:c++17 main_4_3.cpp /Fe:texture_app.exe ^
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
#include "texture_common.h"
#include "texture_features_omp.h"
#include "texture_features_tbb.h"

static const int RUNS_PARALLEL = 10; // >= 10 по заданию
static const int RUNS_SEQ      = 5;
static const int NEIGH[3] = { 5, 7, 9 }; // окрестности 5x5,7x7,9x9 (RH=RW)
static const int THREADS[3] = { 2, 3, 4 };
static const int NUM_IMAGES = 2; // НД1, НД2 (по заданию)
static const double K1 = 0.30;   // диапазон [0.25,0.4]
static const double K2 = 0.70;   // диапазон [0.6,0.75]

static void SaveFourMaps(const TextureMaps& tm, const MinMaxAll& mm, int width, int height,
                          const std::string& prefix) {
    double T1, T2;
    RGBTRIPLE** mapImg = AllocImage(width, height);

    ComputeThresholds(mm.minM2, mm.maxM2, K1, K2, T1, T2);
    BuildMapSeq(tm.M2, width, height, T1, T2, mapImg);
    BMPWrite(mapImg, width, height, (prefix + "_M2.bmp").c_str());

    ComputeThresholds(mm.minR, mm.maxR, K1, K2, T1, T2);
    BuildMapSeq(tm.R, width, height, T1, T2, mapImg);
    BMPWrite(mapImg, width, height, (prefix + "_R.bmp").c_str());

    ComputeThresholds(mm.minU, mm.maxU, K1, K2, T1, T2);
    BuildMapSeq(tm.U, width, height, T1, T2, mapImg);
    BMPWrite(mapImg, width, height, (prefix + "_U.bmp").c_str());

    ComputeThresholds(mm.minE, mm.maxE, K1, K2, T1, T2);
    BuildMapSeq(tm.E, width, height, T1, T2, mapImg);
    BMPWrite(mapImg, width, height, (prefix + "_E.bmp").c_str());

    FreeImage(mapImg);
}

static void RunImage(int imgIdx, const std::string& inPath, std::ofstream& csv) {
    RGBTRIPLE** src = nullptr;
    BITMAPFILEHEADER fh{}; BITMAPINFOHEADER ih{};
    BMPRead(src, fh, ih, inPath.c_str());
    if (!src) { std::cerr << "Не удалось открыть " << inPath << "\n"; return; }
    int width = ih.biWidth, height = ih.biHeight;
    std::cout << "\n=== Изображение " << imgIdx << " (" << width << "x" << height << ") ===\n";

    std::vector<uint8_t> bimage;
    ToGrayscale(src, width, height, bimage);

    for (int neigh : NEIGH) {
        int RH = neigh / 2, RW = neigh / 2;
        TextureMaps tm; MinMaxAll mm;

        // --- Последовательная версия (база T1 для Sp) ---
        std::vector<double> times(RUNS_SEQ);
        for (int r = 0; r < RUNS_SEQ; r++) {
            double t1 = ComputeTextureMapsSeq(bimage.data(), width, height, RH, RW, tm);
            double t2m = 0.0; { double tt = omp_get_wtime(); mm = ComputeMinMaxSeq(tm); t2m = omp_get_wtime() - tt; }
            times[r] = t1 + t2m;
        }
        double T1time = AvgTrustedIntervalMed(times.data(), RUNS_SEQ);
        std::cout << "Окрестность " << neigh << "x" << neigh << " (посл.): t="
                   << std::fixed << std::setprecision(4) << T1time << " c\n";
        csv << "posl;" << imgIdx << ";" << width << "x" << height << ";-;" << neigh << ";" << T1time << ";1.0\n";

        if (neigh == 9) {
            SaveFourMaps(tm, mm, width, height,
                         "../images_out/output_" + std::to_string(imgIdx) + "_posl_" + std::to_string(neigh));
        }

        // --- OpenMP ---
        for (int th : THREADS) {
            std::vector<double> pt(RUNS_PARALLEL);
            for (int r = 0; r < RUNS_PARALLEL; r++) {
                double t1 = ComputeTextureMapsOmp(bimage.data(), width, height, RH, RW, tm, th);
                double t2 = ComputeMinMaxOmp(tm, th, mm);
                pt[r] = t1 + t2;
            }
            double t = AvgTrustedIntervalMed(pt.data(), RUNS_PARALLEL);
            double sp = T1time / t;
            std::cout << "Окрестность " << neigh << "x" << neigh << " (OpenMP, p=" << th
                       << "): t=" << t << " c, Sp=" << sp << "\n";
            csv << "OpenMP;" << imgIdx << ";" << width << "x" << height << ";" << th << ";" << neigh << ";" << t << ";" << sp << "\n";
            if (neigh == 9 && th == THREADS[0]) {
                SaveFourMaps(tm, mm, width, height,
                             "../images_out/output_" + std::to_string(imgIdx) + "_openmp_" + std::to_string(neigh));
            }
        }

        // --- oneTBB ---
        for (int th : THREADS) {
            std::vector<double> pt(RUNS_PARALLEL);
            for (int r = 0; r < RUNS_PARALLEL; r++) {
                double t1 = ComputeTextureMapsTbb(bimage.data(), width, height, RH, RW, tm, th);
                double t2 = ComputeMinMaxTbb(tm, th, mm);
                pt[r] = t1 + t2;
            }
            double t = AvgTrustedIntervalMed(pt.data(), RUNS_PARALLEL);
            double sp = T1time / t;
            std::cout << "Окрестность " << neigh << "x" << neigh << " (oneTBB, p=" << th
                       << "): t=" << t << " c, Sp=" << sp << "\n";
            csv << "oneTBB;" << imgIdx << ";" << width << "x" << height << ";" << th << ";" << neigh << ";" << t << ";" << sp << "\n";
            if (neigh == 9 && th == THREADS[0]) {
                SaveFourMaps(tm, mm, width, height,
                             "../images_out/output_" + std::to_string(imgIdx) + "_onetbb_" + std::to_string(neigh));
            }
        }
    }

    BMPFree(src);
}

int main() {
    std::cout << "Задание 4.3. Текстурные признаки (посл. + OpenMP + oneTBB)\n";
    std::cout << "omp_get_num_procs() = " << omp_get_num_procs() << "\n";
    std::cout << "k1=" << K1 << ", k2=" << K2 << "\n";

    std::ofstream csv("../data/texture_results.csv");
    csv << "mode;image;resolution;threads;neigh;time_s;Sp\n";

    for (int img = 1; img <= NUM_IMAGES; img++) {
        std::string inPath = "../images_in/input_" + std::to_string(img) + ".bmp";
        RunImage(img, inPath, csv);
    }

    csv.close();
    std::cout << "\nГотово. Результаты сохранены в data/texture_results.csv\n";
    return 0;
}

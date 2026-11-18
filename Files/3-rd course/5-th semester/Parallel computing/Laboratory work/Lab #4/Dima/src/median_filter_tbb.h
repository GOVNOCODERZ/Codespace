// median_filter_tbb.h
// Задание 4.2. Медианный фильтр — параллельная реализация на Intel oneTBB.
//
// По условию лабораторной работы 4: если в OpenMP распараллеливается
// внешний цикл (по строкам изображения), то и в oneTBB распараллеливать
// нужно только внешний цикл — для честного сравнения условий эксперимента.
// Здесь используется tbb::parallel_for по blocked_range<int> на диапазоне
// строк изображения (аналог #pragma omp parallel for по Y в OpenMP-версии
// из median_filter_omp.h).
//
// ВНИМАНИЕ: написано по документированному API oneTBB, не скомпилировано
// в данной песочнице (нет доступа к libtbb-dev/интернету) — собирать и
// проверять нужно в среде с Intel oneAPI Base Toolkit.
#pragma once

#include <tbb/tbb.h>
#include <tbb/global_control.h>
#include <tbb/blocked_range.h>
#include <chrono>
#include <vector>
#include <cstdint>
#include "bmp_io.h"

inline double NowSecondsMF() {
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

// Один проход медианного фильтра, oneTBB, распараллеливание только по
// внешнему циклу (строки изображения). sortSeqFn — функция сортировки
// double-массива размера Ksize*Ksize (переиспользуются OddEvenSortPlain /
// ShellSortPlain из median_filter_omp.h).
template <typename SortSeqFn>
double MedianFilterPassTbb(RGBTRIPLE** src, RGBTRIPLE** dst, int width, int height,
                            int Ksize, int nThreads, SortSeqFn sortSeqFn) {
    int R = Ksize / 2;
    int size = Ksize * Ksize;

    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, (size_t)nThreads);

    double t0 = NowSecondsMF();
    tbb::parallel_for(tbb::blocked_range<int>(0, height),
        [&](const tbb::blocked_range<int>& rows) {
            // Локальные буферы — отдельная копия на каждый вызов тела
            // (каждый диапазон строк обрабатывается одним потоком
            // последовательно), гонки данных нет: пишем только в "свои"
            // строки dst.
            std::vector<double> medR(size), medG(size), medB(size);
            for (int y = rows.begin(); y != rows.end(); ++y) {
                for (int x = 0; x < width; x++) {
                    int idx = 0;
                    for (int dy = -R; dy <= R; dy++) {
                        int ky = y + dy;
                        if (ky < 0) ky = 0;
                        if (ky > height - 1) ky = height - 1;
                        for (int dx = -R; dx <= R; dx++) {
                            int kx = x + dx;
                            if (kx < 0) kx = 0;
                            if (kx > width - 1) kx = width - 1;
                            const RGBTRIPLE& p = src[ky][kx];
                            medR[idx] = p.rgbtRed;
                            medG[idx] = p.rgbtGreen;
                            medB[idx] = p.rgbtBlue;
                            idx++;
                        }
                    }
                    sortSeqFn(medR.data(), (size_t)size);
                    sortSeqFn(medG.data(), (size_t)size);
                    sortSeqFn(medB.data(), (size_t)size);
                    dst[y][x].rgbtRed   = (uint8_t)medR[size / 2];
                    dst[y][x].rgbtGreen = (uint8_t)medG[size / 2];
                    dst[y][x].rgbtBlue  = (uint8_t)medB[size / 2];
                }
            }
        });

    return NowSecondsMF() - t0;
}

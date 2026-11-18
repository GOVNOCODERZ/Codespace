// median_filter.h
// Медианная фильтрация RGB-изображения — один проход фильтра.
// Вынесено в отдельный заголовок, чтобы переиспользовать и в основном
// приложении (main_median.cpp), и в автономных тестах корректности.
#pragma once

#include <omp.h>
#include <vector>
#include <cstdint>
#include "bmp_io.h"

// Один проход медианного фильтра по всем 3 каналам.
// nThreads == 0  => последовательная версия;
// nThreads >= 1  => параллельная версия (распараллеливание по строкам
//                   изображения, #pragma omp parallel for).
// sortSeqFn — функция сортировки double-массива размера Ksize*Ksize
// (используются шаблонные функции сортировки из sorts.h, без собственного
// внутреннего замера времени, чтобы не искажать замер всего прохода).
template <typename SortSeqFn>
double MedianFilterPass(RGBTRIPLE** src, RGBTRIPLE** dst, int width, int height,
                         int Ksize, int nThreads, SortSeqFn sortSeqFn) {
    int R = Ksize / 2; // RH = RW = R
    int size = Ksize * Ksize;

    double t0 = omp_get_wtime();

    if (nThreads == 0) {
        // --- Последовательная версия ---
        std::vector<double> medR(size), medG(size), medB(size);
        for (int y = 0; y < height; y++) {
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
    } else {
        // --- Параллельная версия: распараллеливание по строкам ---
        // Каждый поток владеет собственными локальными буферами
        // medR/medG/medB (объявлены внутри параллельной области) — это
        // предотвращает гонку данных без лишней синхронизации (т.к.
        // каждый поток читает только src и пишет в непересекающиеся
        // строки dst).
        omp_set_num_threads(nThreads);
        #pragma omp parallel default(none) shared(src, dst, width, height, Ksize, R, size, sortSeqFn)
        {
            std::vector<double> medR(size), medG(size), medB(size);
            #pragma omp for schedule(static)
            for (int y = 0; y < height; y++) {
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
        }
    }

    return omp_get_wtime() - t0;
}

// Простые (без собственного замера времени) версии сортировки для
// применения внутри медианного фильтра — переиспользуют логику из sorts.h.
inline void OddEvenSortPlain(double* a, size_t N) {
    bool swapped;
    do {
        swapped = false;
        for (size_t i = 0; i + 1 < N; i += 2) if (a[i] > a[i + 1]) { std::swap(a[i], a[i + 1]); swapped = true; }
        for (size_t i = 1; i + 1 < N; i += 2) if (a[i] > a[i + 1]) { std::swap(a[i], a[i + 1]); swapped = true; }
    } while (swapped);
}
inline void ShellSortPlain(double* a, size_t N) {
    for (size_t gap = N / 2; gap > 0; gap /= 2) {
        for (size_t i = gap; i < N; i++) {
            double temp = a[i];
            size_t j = i;
            while (j >= gap && a[j - gap] > temp) { a[j] = a[j - gap]; j -= gap; }
            a[j] = temp;
        }
    }
}

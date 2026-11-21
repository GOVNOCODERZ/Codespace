// median_filter.h — Оптимальная версия под методичку для компиляции без -O
#pragma once

#include <omp.h>
#include <cstdint>
#include <cstddef>
#include "bmp_io.h"

static const int MAX_MEDMAS_SIZE = 512;

// 1. Сортировка Шелла (последовательность Кнута 1, 4, 13, 40, 121... работает быстрее gap/2)
inline void ShellSortPlain(uint8_t* a, size_t n) {
    int N = (int)n;
    int gap = 1;
    while (gap < N / 3) gap = 3 * gap + 1;
    for (; gap > 0; gap /= 3) {
        for (int i = gap; i < N; i++) {
            uint8_t temp = a[i];
            int j = i;
            while (j >= gap && a[j - gap] > temp) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = temp;
        }
    }
}

// 2. Быстрая сортировка (qsort) без рекурсии (на локальном стеке),
// чтобы при компиляции без флагов -O не тратить время на вызовы функций!
inline void QuickSortPlain(uint8_t* a, size_t n) {
    int N = (int)n;
    if (N <= 1) return;

    int stackLo[32], stackHi[32];
    int top = 0;
    stackLo[0] = 0;
    stackHi[0] = N - 1;

    while (top >= 0) {
        int lo = stackLo[top];
        int hi = stackHi[top];
        top--;

        while (lo < hi) {
            // Для малых отрезков (<= 10) досортировываем вставками прямо на месте
            if (hi - lo <= 10) {
                for (int i = lo + 1; i <= hi; i++) {
                    uint8_t temp = a[i];
                    int j = i;
                    while (j > lo && a[j - 1] > temp) {
                        a[j] = a[j - 1];
                        j--;
                    }
                    a[j] = temp;
                }
                break;
            }

            uint8_t pivot = a[(lo + hi) >> 1];
            int i = lo, j = hi;
            while (i <= j) {
                while (a[i] < pivot) i++;
                while (a[j] > pivot) j--;
                if (i <= j) {
                    uint8_t tmp = a[i];
                    a[i] = a[j];
                    a[j] = tmp;
                    i++;
                    j--;
                }
            }

            // Меньшую часть обрабатываем в цикле, большую кладем в стек
            if (j - lo < hi - i) {
                if (i < hi) { top++; stackLo[top] = i; stackHi[top] = hi; }
                hi = j;
            } else {
                if (lo < j) { top++; stackLo[top] = lo; stackHi[top] = j; }
                lo = i;
            }
        }
    }
}

template <typename SortSeqFn>
double MedianFilterPass(RGBTRIPLE** src, RGBTRIPLE** dst, int width, int height,
                         int Ksize, int nThreads, SortSeqFn sortSeqFn) {
    int RH = Ksize / 2;
    int RW = Ksize / 2;
    size_t size = (size_t)Ksize * Ksize;
    size_t half = size / 2;

    double t0 = omp_get_wtime();

    if (nThreads == 0) {
        uint8_t MEDMAS_R[MAX_MEDMAS_SIZE];
        uint8_t MEDMAS_G[MAX_MEDMAS_SIZE];
        uint8_t MEDMAS_B[MAX_MEDMAS_SIZE];

        for (int y = 0; y < height; y++) {
            RGBTRIPLE* dstRow = dst[y];
            for (int x = 0; x < width; x++) {
                int Masind = 0;
                for (int dy = -RH; dy <= RH; dy++) {
                    int ky = y + dy;
                    if (ky < 0) ky = 0;
                    else if (ky >= height) ky = height - 1;

                    const RGBTRIPLE* srcRow = src[ky];
                    for (int dx = -RW; dx <= RW; dx++) {
                        int kx = x + dx;
                        if (kx < 0) kx = 0;
                        else if (kx >= width) kx = width - 1;

                        const RGBTRIPLE& p = srcRow[kx];
                        MEDMAS_R[Masind] = p.rgbtRed;
                        MEDMAS_G[Masind] = p.rgbtGreen;
                        MEDMAS_B[Masind] = p.rgbtBlue;
                        Masind++;
                    }
                }
                sortSeqFn(MEDMAS_R, size);
                sortSeqFn(MEDMAS_G, size);
                sortSeqFn(MEDMAS_B, size);

                dstRow[x].rgbtRed   = MEDMAS_R[half];
                dstRow[x].rgbtGreen = MEDMAS_G[half];
                dstRow[x].rgbtBlue  = MEDMAS_B[half];
            }
        }
    } else {
        omp_set_num_threads(nThreads);
        #pragma omp parallel default(none) shared(src, dst, width, height, RH, RW, size, half, sortSeqFn)
        {
            uint8_t MEDMAS_R[MAX_MEDMAS_SIZE];
            uint8_t MEDMAS_G[MAX_MEDMAS_SIZE];
            uint8_t MEDMAS_B[MAX_MEDMAS_SIZE];

            #pragma omp for schedule(dynamic, 4)
            for (int y = 0; y < height; y++) {
                RGBTRIPLE* dstRow = dst[y];
                for (int x = 0; x < width; x++) {
                    int Masind = 0;
                    for (int dy = -RH; dy <= RH; dy++) {
                        int ky = y + dy;
                        if (ky < 0) ky = 0;
                        else if (ky >= height) ky = height - 1;

                        const RGBTRIPLE* srcRow = src[ky];
                        for (int dx = -RW; dx <= RW; dx++) {
                            int kx = x + dx;
                            if (kx < 0) kx = 0;
                            else if (kx >= width) kx = width - 1;

                            const RGBTRIPLE& p = srcRow[kx];
                            MEDMAS_R[Masind] = p.rgbtRed;
                            MEDMAS_G[Masind] = p.rgbtGreen;
                            MEDMAS_B[Masind] = p.rgbtBlue;
                            Masind++;
                        }
                    }
                    sortSeqFn(MEDMAS_R, size);
                    sortSeqFn(MEDMAS_G, size);
                    sortSeqFn(MEDMAS_B, size);

                    dstRow[x].rgbtRed   = MEDMAS_R[half];
                    dstRow[x].rgbtGreen = MEDMAS_G[half];
                    dstRow[x].rgbtBlue  = MEDMAS_B[half];
                }
            }
        }
    }

    return omp_get_wtime() - t0;
}
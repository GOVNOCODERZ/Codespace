// texture_features_omp.h
// Задание 4.3. Расчёт карт текстурных признаков — последовательная версия
// и параллельная версия на OpenMP.
#pragma once

#include <omp.h>
#include <vector>
#include "texture_common.h"

// --- Последовательная версия: формирование карты значений признаков ---
inline double ComputeTextureMapsSeq(const uint8_t* bimage, int width, int height,
                                     int RH, int RW, TextureMaps& out) {
    out.alloc(width, height);
    double t0 = omp_get_wtime();
    double hist[256];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            TextureVals v = ComputeTextureAtPixel(bimage, width, height, x, y, RH, RW, hist);
            size_t idx = (size_t)y * width + x;
            out.M2[idx] = v.m2; out.R[idx] = v.r; out.U[idx] = v.u; out.E[idx] = v.e;
        }
    }
    return omp_get_wtime() - t0;
}

// --- OpenMP: параллельный внешний цикл по Y, у каждого потока своя
// локальная гистограмма (объявлена внутри параллельной области). ---
inline double ComputeTextureMapsOmp(const uint8_t* bimage, int width, int height,
                                     int RH, int RW, TextureMaps& out, int nth) {
    out.alloc(width, height);
    omp_set_num_threads(nth);
    double* M2 = out.M2.data(); double* R = out.R.data();
    double* U = out.U.data();   double* E = out.E.data();

    double t0 = omp_get_wtime();
    #pragma omp parallel default(none) shared(bimage, width, height, RH, RW, M2, R, U, E)
    {
        double hist[256];
        #pragma omp for schedule(static)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                TextureVals v = ComputeTextureAtPixel(bimage, width, height, x, y, RH, RW, hist);
                size_t idx = (size_t)y * width + x;
                M2[idx] = v.m2; R[idx] = v.r; U[idx] = v.u; E[idx] = v.e;
            }
        }
    }
    return omp_get_wtime() - t0;
}

// --- Шаг 1 (OpenMP): редукция min/max по всем 4 картам одновременно
// (reduction(min:...) reduction(max:...), OpenMP 4.5+). ---
inline double ComputeMinMaxOmp(const TextureMaps& t, int nth, MinMaxAll& mm) {
    omp_set_num_threads(nth);
    long long n = (long long)t.width * t.height;
    const double* M2 = t.M2.data(); const double* R = t.R.data();
    const double* U = t.U.data();   const double* E = t.E.data();

    double minM2 = 1e300, maxM2 = -1e300, minR = 1e300, maxR = -1e300;
    double minU = 1e300, maxU = -1e300, minE = 1e300, maxE = -1e300;

    double t0 = omp_get_wtime();
    #pragma omp parallel for \
        reduction(min:minM2,minR,minU,minE) reduction(max:maxM2,maxR,maxU,maxE) \
        default(none) shared(M2, R, U, E, n)
    for (long long k = 0; k < n; k++) {
        if (M2[k] < minM2) minM2 = M2[k]; if (M2[k] > maxM2) maxM2 = M2[k];
        if (R[k]  < minR)  minR  = R[k];  if (R[k]  > maxR)  maxR  = R[k];
        if (U[k]  < minU)  minU  = U[k];  if (U[k]  > maxU)  maxU  = U[k];
        if (E[k]  < minE)  minE  = E[k];  if (E[k]  > maxE)  maxE  = E[k];
    }
    double dt = omp_get_wtime() - t0;

    mm.minM2 = minM2; mm.maxM2 = maxM2; mm.minR = minR; mm.maxR = maxR;
    mm.minU  = minU;  mm.maxU  = maxU;  mm.minE = minE; mm.maxE = maxE;
    return dt;
}

// --- Шаг 3 (OpenMP): построение карты по порогам, параллельный 2D цикл. ---
inline double BuildMapOmp(const std::vector<double>& feature, int width, int height,
                           double T1, double T2, RGBTRIPLE** outMap, int nth) {
    omp_set_num_threads(nth);
    const double* f = feature.data();
    double t0 = omp_get_wtime();
    #pragma omp parallel for collapse(2) default(none) shared(f, width, height, T1, T2, outMap)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            outMap[y][x] = ColorByThreshold(f[(size_t)y * width + x], T1, T2);
        }
    }
    return omp_get_wtime() - t0;
}

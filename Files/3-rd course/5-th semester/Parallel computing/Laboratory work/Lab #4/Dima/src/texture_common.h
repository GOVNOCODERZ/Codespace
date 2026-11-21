// texture_common.h
// Задание 4.3. Общие вычисления для текстурных признаков: перевод в
// градации серого, расчёт признаков по гистограмме окрестности,
// построение карт по порогам T1/T2 и сохранение их в виде BMP.
// Не зависит от технологии распараллеливания — переиспользуется и в
// OpenMP-, и в oneTBB-версиях (texture_features_omp.h / _tbb.h).
#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <limits>
#include "bmp_io.h"

// Перевод изображения в градации серого (яркость): BIMAGE[y][x].
inline void ToGrayscale(RGBTRIPLE** src, int width, int height, std::vector<uint8_t>& bimage) {
    bimage.resize((size_t)width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const RGBTRIPLE& p = src[y][x];
            double v = p.rgbtRed * 0.299 + p.rgbtGreen * 0.587 + p.rgbtBlue * 0.114;
            bimage[(size_t)y * width + x] = (uint8_t)(v + 0.5);
        }
    }
}

// Результат расчёта текстурных признаков для одного пикселя.
struct TextureVals { double m2, r, u, e; };

// Расчёт признаков для пикселя (x,y) по гистограмме окрестности
// (RH,RW) — см. псевдокод из методички лабораторной работы 4.
// hist — переиспользуемый буфer на 256 элементов (double), заполняется
// и обнуляется внутри функции (чтобы не выделять память на каждый пиксель).
inline TextureVals ComputeTextureAtPixel(const uint8_t* bimage, int width, int height,
                                          int x, int y, int RH, int RW, double* hist /*[256]*/) {
    std::fill(hist, hist + 256, 0.0);

    for (int dy = -RH; dy <= RH; dy++) {
        int ky = y + dy;
        if (ky < 0) ky = 0;
        if (ky > height - 1) ky = height - 1;
        for (int dx = -RW; dx <= RW; dx++) {
            int kx = x + dx;
            if (kx < 0) kx = 0;
            if (kx > width - 1) kx = width - 1;
            hist[bimage[(size_t)ky * width + kx]] += 1.0;
        }
    }

    double size = (double)((RH * 2 + 1) * (RW * 2 + 1));
    double m = 0.0;
    for (int i = 0; i < 256; i++) {
        hist[i] /= size;
        m += hist[i] * i;
    }

    double m2 = 0.0, e = 0.0, u = 0.0;
    for (int i = 0; i < 256; i++) {
        double d = (double)i - m;
        m2 += d * d * hist[i];
        if (hist[i] != 0.0) e += hist[i] * std::log2(hist[i]);
        u += hist[i] * hist[i];
    }

    TextureVals out;
    out.m2 = m2;
    out.u = u;
    out.r = 1.0 - (1.0 / (1.0 + m2));
    out.e = -e;
    return out;
}

// Четыре карты текстурных признаков (значения) + их размеры.
struct TextureMaps {
    int width = 0, height = 0;
    std::vector<double> M2, R, U, E;
    void alloc(int w, int h) {
        width = w; height = h;
        M2.assign((size_t)w * h, 0.0);
        R.assign((size_t)w * h, 0.0);
        U.assign((size_t)w * h, 0.0);
        E.assign((size_t)w * h, 0.0);
    }
};

// Шаг 1 (последовательный, для проверки корректности параллельных версий):
// минимумы/максимумы по всем 4 картам.
struct MinMaxAll {
    double minM2 = std::numeric_limits<double>::max(), maxM2 = -std::numeric_limits<double>::max();
    double minR  = std::numeric_limits<double>::max(), maxR  = -std::numeric_limits<double>::max();
    double minU  = std::numeric_limits<double>::max(), maxU  = -std::numeric_limits<double>::max();
    double minE  = std::numeric_limits<double>::max(), maxE  = -std::numeric_limits<double>::max();
};

inline MinMaxAll ComputeMinMaxSeq(const TextureMaps& t) {
    MinMaxAll mm;
    size_t n = (size_t)t.width * t.height;
    for (size_t k = 0; k < n; k++) {
        mm.minM2 = std::min(mm.minM2, t.M2[k]); mm.maxM2 = std::max(mm.maxM2, t.M2[k]);
        mm.minR  = std::min(mm.minR,  t.R[k]);  mm.maxR  = std::max(mm.maxR,  t.R[k]);
        mm.minU  = std::min(mm.minU,  t.U[k]);  mm.maxU  = std::max(mm.maxU,  t.U[k]);
        mm.minE  = std::min(mm.minE,  t.E[k]);  mm.maxE  = std::max(mm.maxE,  t.E[k]);
    }
    return mm;
}

// Шаг 2: пороги T1,T2 для одного признака.
inline void ComputeThresholds(double mn, double mx, double k1, double k2, double& T1, double& T2) {
    T1 = (mx - mn) * k1 + mn;
    T2 = (mx - mn) * k2 + mn;
}

// Цвет по порогам: зелёный <=T1, жёлтый (T1,T2), красный >=T2.
inline RGBTRIPLE ColorByThreshold(double val, double T1, double T2) {
    RGBTRIPLE c;
    if (val <= T1)      { c.rgbtRed = 0;   c.rgbtGreen = 255; c.rgbtBlue = 0; }   // зелёный
    else if (val < T2)  { c.rgbtRed = 255; c.rgbtGreen = 255; c.rgbtBlue = 0; }   // жёлтый
    else                { c.rgbtRed = 255; c.rgbtGreen = 0;   c.rgbtBlue = 0; }   // красный
    return c;
}

// Шаг 3 (последовательный): построение одной карты (для проверки
// корректности и как референс для OpenMP/oneTBB вариантов).
inline void BuildMapSeq(const std::vector<double>& feature, int width, int height,
                         double T1, double T2, RGBTRIPLE** outMap) {
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            outMap[y][x] = ColorByThreshold(feature[(size_t)y * width + x], T1, T2);
}

inline RGBTRIPLE** AllocImage(int width, int height) {
    RGBTRIPLE** img = new RGBTRIPLE*[height];
    img[0] = new RGBTRIPLE[(size_t)width * height];
    for (int i = 1; i < height; i++) img[i] = &img[0][(size_t)width * i];
    return img;
}
inline void FreeImage(RGBTRIPLE** img) {
    if (img) { delete[] img[0]; delete[] img; }
}

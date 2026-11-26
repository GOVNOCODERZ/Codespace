#include "../src/BMPFileRW.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

static inline uint32_t hash32(uint32_t x, uint32_t y, uint32_t seed) {
    uint32_t h = x * 374761393u + y * 668265263u + seed * 1442695041u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static inline BYTE clampByte(double v) {
    if (v < 0.0) return 0;
    if (v > 255.0) return 255;
    return static_cast<BYTE>(v + 0.5);
}

// Добавление умеренного аддитивного гауссова (равномерно-суммарного) шума для наглядности сглаживания
static inline void applyGaussianLikeNoise(RGBTRIPLE& px, int x, int y, uint32_t seed, double amp) {
    uint32_t h1 = hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed);
    uint32_t h2 = hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed + 17u);
    uint32_t h3 = hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed + 53u);
    // Сумма трёх равномерных величин аппроксимирует нормальный шум
    double n = (((h1 & 255u) + (h2 & 255u) + (h3 & 255u)) / 382.5 - 1.0) * amp;
    px.rgbtRed   = clampByte(px.rgbtRed   + n);
    px.rgbtGreen = clampByte(px.rgbtGreen + n);
    px.rgbtBlue  = clampByte(px.rgbtBlue  + n);
}

// Сцена 1 (1280x720): закат над горами с мелким аддитивным шумом и чёткими силуэтами
static void genScene1(RGBTRIPLE** img, int W, int H) {
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double r, g, b;
            if (ny < 0.62) {
                double t = ny / 0.62;
                r = 40.0 + 210.0 * t; g = 50.0 + 115.0 * t; b = 135.0 + 40.0 * (1.0 - t);
                double sx = (nx - 0.68) * (static_cast<double>(W) / H);
                double sy = ny - 0.34;
                double d = std::sqrt(sx * sx + sy * sy);
                if (d < 0.11) { r = 255; g = 235; b = 145; }
                else if (d < 0.27) {
                    double gl = (0.27 - d) / 0.16;
                    r = std::min(255.0, r + 85.0 * gl);
                    g = std::min(255.0, g + 65.0 * gl);
                    b = std::min(255.0, b + 20.0 * gl);
                }
            } else {
                double t = (ny - 0.62) / 0.38;
                double wv = 14.0 * std::sin(ny * 130.0 + std::sin(nx * 24.0) * 2.0);
                r = 165.0 * (1.0 - 0.62 * t) + wv;
                g = 98.0 * (1.0 - 0.52 * t) + wv * 0.6;
                b = 135.0 + 35.0 * t + wv * 0.4;
                double dx = std::abs(nx - 0.68);
                if (dx < 0.08 + 0.05 * t) {
                    double s = 1.0 - dx / (0.08 + 0.05 * t);
                    r = std::min(255.0, r + 95.0 * s);
                    g = std::min(255.0, g + 80.0 * s);
                    b = std::min(255.0, b + 35.0 * s);
                }
            }
            double m1 = 0.50 - 0.14 * std::sin(nx * 6.5 + 0.4) - 0.06 * std::cos(nx * 17.0);
            if (ny > m1 && ny < 0.62) { r = 68; g = 55; b = 98; }
            double m2 = 0.56 - 0.10 * std::sin(nx * 9.2 + 2.1) - 0.04 * std::sin(nx * 23.0);
            if (ny > m2 && ny < 0.62) { r = 34; g = 32; b = 56; }
            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applyGaussianLikeNoise(img[y][x], x, y, 101u, 36.0);
        }
    }
}

// Сцена 2 (1600x900): геометрическая мишень и резкие контрастные полосы (отлично показывает размытие Гаусса vs Box)
static void genScene2(RGBTRIPLE** img, int W, int H) {
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double r, g, b;
            int bx = (x / 80) % 2;
            int by = (y / 80) % 2;
            if ((bx ^ by) == 0) {
                r = 35.0 + 75.0 * nx; g = 55.0 + 90.0 * ny; b = 120.0 + 70.0 * (1.0 - nx);
            } else {
                r = 225.0 - 30.0 * ny; g = 230.0 - 25.0 * nx; b = 238.0;
            }
            double cx = (nx - 0.5) * (static_cast<double>(W) / H);
            double cy = ny - 0.48;
            double d = std::sqrt(cx * cx + cy * cy);
            if (d < 0.34) {
                int ring = static_cast<int>(d * 28.0);
                if (ring % 2 == 0) { r = 24; g = 36; b = 68; }
                else { r = 245; g = 185; b = 45; }
                if (d < 0.055) { r = 220; g = 45; b = 55; }
            }
            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applyGaussianLikeNoise(img[y][x], x, y, 202u, 28.0);
        }
    }
}

// Сцена 3 (1920x1080): множество Мандельброта с тонкими деталями
static void genScene3(RGBTRIPLE** img, int W, int H) {
    #pragma omp parallel for schedule(dynamic, 16)
    for (int y = 0; y < H; y++) {
        double ci = (static_cast<double>(y) / H - 0.5) * 2.2;
        for (int x = 0; x < W; x++) {
            double cr = (static_cast<double>(x) / W - 0.5) * 2.2 * (static_cast<double>(W) / H) - 0.55;
            double zr = 0.0, zi = 0.0;
            int iter = 0;
            const int maxIter = 80;
            while (zr * zr + zi * zi <= 4.0 && iter < maxIter) {
                double nzr = zr * zr - zi * zi + cr;
                zi = 2.0 * zr * zi + ci;
                zr = nzr;
                iter++;
            }
            double r, g, b;
            if (iter == maxIter) { r = 14; g = 18; b = 34; }
            else {
                double t = static_cast<double>(iter) / maxIter;
                r = 255.0 * (0.5 + 0.5 * std::sin(6.28318 * (t * 2.2 + 0.00)));
                g = 255.0 * (0.5 + 0.5 * std::sin(6.28318 * (t * 2.2 + 0.33)));
                b = 255.0 * (0.5 + 0.5 * std::sin(6.28318 * (t * 2.2 + 0.67)));
            }
            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applyGaussianLikeNoise(img[y][x], x, y, 303u, 24.0);
        }
    }
}

// Сцена 4 (2560x1440): волновая интерференция и геометрический медальон
static void genScene4(RGBTRIPLE** img, int W, int H) {
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double aspect = static_cast<double>(W) / H;
            double cx = (nx - 0.5) * aspect;
            double cy = ny - 0.5;
            double dist = std::sqrt(cx * cx + cy * cy);
            double angle = std::atan2(cy, cx);
            double mix = (std::sin(nx * 14.0 + ny * 6.0) + std::cos(dist * 26.0 - 4.0 * std::cos(angle * 6.0)) + std::sin((nx - ny) * 18.0)) / 3.0;
            double r = 127.5 + 115.0 * std::sin(mix * 3.14159 + 0.3);
            double g = 127.5 + 115.0 * std::sin(mix * 3.14159 + 2.3);
            double b = 127.5 + 115.0 * std::sin(mix * 3.14159 + 4.2);
            if (std::abs(dist - 0.35) < 0.015 || std::abs(dist - 0.18) < 0.012) { r = g = b = 250; }
            else if (std::abs(cx) < 0.08 && std::abs(cy) < 0.08) { r = 25; g = 35; b = 75; }
            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applyGaussianLikeNoise(img[y][x], x, y, 404u, 26.0);
        }
    }
}

int main() {
    struct Spec { int w; int h; const char* path; void (*fn)(RGBTRIPLE**, int, int); };
    Spec specs[] = {
        { 1280,  720, "Lab5/images/input_1.bmp", genScene1 },
        { 1600,  900, "Lab5/images/input_2.bmp", genScene2 },
        { 1920, 1080, "Lab5/images/input_3.bmp", genScene3 },
        { 2560, 1440, "Lab5/images/input_4.bmp", genScene4 }
    };
    for (const auto& sp : specs) {
        std::cout << "Generating " << sp.path << " (" << sp.w << "x" << sp.h << ")..." << std::flush;
        RGBTRIPLE** img = BMPAlloc(sp.w, sp.h);
        sp.fn(img, sp.w, sp.h);
        BMPWrite(img, sp.w, sp.h, sp.path);
        // Также копируем НД 1 как канонический input.bmp для демонстрации задания 5.1
        if (sp.w == 1280) {
            BMPWrite(img, sp.w, sp.h, "Lab5/images/input.bmp");
        }
        BMPFree(img);
        std::cout << " OK\n";
    }
    return 0;
}

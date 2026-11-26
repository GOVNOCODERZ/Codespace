#include "../src/BMPFileRW.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <algorithm>

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

// Добавление импульсного шума «соль и перец» (~8% пикселей)
static inline void applySaltPepper(RGBTRIPLE& px, int x, int y, uint32_t seed) {
    uint32_t h = hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed) % 1000u;
    if (h < 40u) {
        px.rgbtRed = 255;
        px.rgbtGreen = 255;
        px.rgbtBlue = 255;
    } else if (h < 80u) {
        px.rgbtRed = 0;
        px.rgbtGreen = 0;
        px.rgbtBlue = 0;
    }
}

// Изображение 1 (1280x720): закат над горами и озером
static void generateScene1(RGBTRIPLE** img, int W, int H) {
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double r, g, b;

            // Градиент неба
            if (ny < 0.62) {
                double t = ny / 0.62;
                r = 35.0 + 215.0 * t;
                g = 45.0 + 110.0 * t;
                b = 120.0 + 50.0 * (1.0 - t);

                // Солнце
                double sx = (nx - 0.68) * (static_cast<double>(W) / H);
                double sy = ny - 0.34;
                double d = std::sqrt(sx * sx + sy * sy);
                if (d < 0.11) {
                    r = 255.0; g = 235.0; b = 140.0;
                } else if (d < 0.28) {
                    double glow = (0.28 - d) / 0.17;
                    r = std::min(255.0, r + 90.0 * glow);
                    g = std::min(255.0, g + 70.0 * glow);
                    b = std::min(255.0, b + 20.0 * glow);
                }
            } else {
                // Озеро с отражением
                double t = (ny - 0.62) / 0.38;
                double wave = 12.0 * std::sin(ny * 140.0 + std::sin(nx * 25.0) * 2.0);
                r = 160.0 * (1.0 - 0.65 * t) + wave;
                g = 95.0 * (1.0 - 0.55 * t) + wave * 0.6;
                b = 130.0 + 35.0 * t + wave * 0.4;
                // Дорожка солнца на воде
                double dx = std::abs(nx - 0.68);
                if (dx < 0.08 + 0.05 * t) {
                    double s = 1.0 - dx / (0.08 + 0.05 * t);
                    r = std::min(255.0, r + 95.0 * s);
                    g = std::min(255.0, g + 80.0 * s);
                    b = std::min(255.0, b + 35.0 * s);
                }
            }

            // Дальний горный хребет
            double m1 = 0.50 - 0.14 * std::sin(nx * 6.5 + 0.4) - 0.06 * std::cos(nx * 17.0);
            if (ny > m1 && ny < 0.62) {
                r = 68.0; g = 55.0; b = 98.0;
            }
            // Ближний горный хребет
            double m2 = 0.56 - 0.10 * std::sin(nx * 9.2 + 2.1) - 0.04 * std::sin(nx * 23.0);
            if (ny > m2 && ny < 0.62) {
                r = 34.0; g = 32.0; b = 56.0;
            }

            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applySaltPepper(img[y][x], x, y, 101u);
        }
    }
}

// Изображение 2 (1920x1080): геометрическая тестовая мишень и цветовые полосы
static void generateScene2(RGBTRIPLE** img, int W, int H) {
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double r = 230.0, g = 235.0, b = 240.0;

            // Фоновая шахматная сетка крупных блоков
            int bx = (x / 96) % 2;
            int by = (y / 96) % 2;
            if ((bx ^ by) == 0) {
                r = 42.0 + 70.0 * nx;
                g = 60.0 + 90.0 * ny;
                b = 110.0 + 80.0 * (1.0 - nx);
            } else {
                r = 215.0 - 40.0 * ny;
                g = 225.0 - 30.0 * nx;
                b = 235.0;
            }

            // Нижняя полоса цветных градиентов
            if (ny > 0.80) {
                int band = static_cast<int>(nx * 6.0);
                double lt = (ny - 0.80) / 0.20;
                switch (band) {
                    case 0: r = 240 * (1 - 0.4 * lt); g = 50; b = 50; break;
                    case 1: r = 50; g = 210 * (1 - 0.4 * lt); b = 70; break;
                    case 2: r = 50; g = 90; b = 240 * (1 - 0.4 * lt); break;
                    case 3: r = 240; g = 210; b = 45; break;
                    case 4: r = 45; g = 215; b = 225; break;
                    default: r = 210; g = 60; b = 215; break;
                }
            } else {
                // Центральные концентрические кольца
                double cx = (nx - 0.5) * (static_cast<double>(W) / H);
                double cy = ny - 0.42;
                double d = std::sqrt(cx * cx + cy * cy);
                if (d < 0.34) {
                    int ring = static_cast<int>(d * 24.0);
                    if (ring % 2 == 0) {
                        r = 25.0; g = 35.0; b = 65.0;
                    } else {
                        r = 245.0; g = 185.0; b = 45.0;
                    }
                    if (d < 0.06) {
                        r = 220.0; g = 45.0; b = 55.0;
                    }
                }
            }

            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applySaltPepper(img[y][x], x, y, 202u);
        }
    }
}

// Изображение 3 (2560x1440): множество Мандельброта
static void generateScene3(RGBTRIPLE** img, int W, int H) {
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
            if (iter == maxIter) {
                r = 12.0; g = 16.0; b = 32.0;
            } else {
                double t = static_cast<double>(iter) / maxIter;
                r = 255.0 * (0.5 + 0.5 * std::sin(6.28318 * (t * 2.2 + 0.00)));
                g = 255.0 * (0.5 + 0.5 * std::sin(6.28318 * (t * 2.2 + 0.33)));
                b = 255.0 * (0.5 + 0.5 * std::sin(6.28318 * (t * 2.2 + 0.67)));
            }
            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applySaltPepper(img[y][x], x, y, 303u);
        }
    }
}

// Изображение 4 (3840x2160): волновая интерференция и геометрический медальон (4K UHD)
static void generateScene4(RGBTRIPLE** img, int W, int H) {
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

            double v1 = std::sin(nx * 14.0 + ny * 6.0);
            double v2 = std::cos(dist * 26.0 - 4.0 * std::cos(angle * 6.0));
            double v3 = std::sin((nx - ny) * 18.0);
            double mix = (v1 + v2 + v3) / 3.0;

            double r = 127.5 + 115.0 * std::sin(mix * 3.14159 + 0.3);
            double g = 127.5 + 115.0 * std::sin(mix * 3.14159 + 2.3);
            double b = 127.5 + 115.0 * std::sin(mix * 3.14159 + 4.2);

            // Чёткие геометрические рамки и центральный элемент для проверки сохранения контуров
            if (std::abs(dist - 0.35) < 0.015 || std::abs(dist - 0.18) < 0.012) {
                r = 250.0; g = 250.0; b = 250.0;
            } else if (std::abs(cx) < 0.08 && std::abs(cy) < 0.08) {
                r = 25.0; g = 35.0; b = 75.0;
            }

            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
            applySaltPepper(img[y][x], x, y, 404u);
        }
    }
}

int main() {
    struct Spec { int w; int h; const char* path; void (*gen)(RGBTRIPLE**, int, int); };
    Spec specs[] = {
        {  480,  270, "images/input_1.bmp", generateScene2 },
        {  640,  360, "images/input_2.bmp", generateScene1 },
        {  960,  540, "images/input_3.bmp", generateScene3 },
        { 1280,  720, "images/input_4.bmp", generateScene4 }
    };

    for (const auto& sp : specs) {
        std::cout << "Generating " << sp.path << " (" << sp.w << "x" << sp.h << ")..." << std::flush;
        RGBTRIPLE** img = BMPAlloc(sp.w, sp.h);
        sp.gen(img, sp.w, sp.h);
        BMPWrite(img, sp.w, sp.h, sp.path);
        BMPFree(img);
        std::cout << " OK\n";
    }
    return 0;
}

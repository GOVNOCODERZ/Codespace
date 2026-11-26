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

static inline void applySaltPepper(RGBTRIPLE& px, int x, int y, uint32_t seed) {
    uint32_t h = hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed) % 1000u;
    if (h < 35u) {
        px = { 255, 255, 255 };
    } else if (h < 70u) {
        px = { 0, 0, 0 };
    }
}

// Сцена 1 (1280x720, с шумом для Задания 4.2): закат над горами и озером
static void genMedian1(RGBTRIPLE** img, int W, int H) {
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double r, g, b;
            if (ny < 0.62) {
                double t = ny / 0.62;
                r = 35.0 + 215.0 * t; g = 45.0 + 110.0 * t; b = 120.0 + 50.0 * (1.0 - t);
                double sx = (nx - 0.68) * (static_cast<double>(W) / H);
                double sy = ny - 0.34;
                double d = std::sqrt(sx * sx + sy * sy);
                if (d < 0.11) { r = 255; g = 235; b = 140; }
                else if (d < 0.28) {
                    double glow = (0.28 - d) / 0.17;
                    r = std::min(255.0, r + 90.0 * glow);
                    g = std::min(255.0, g + 70.0 * glow);
                    b = std::min(255.0, b + 20.0 * glow);
                }
            } else {
                double t = (ny - 0.62) / 0.38;
                double wave = 10.0 * std::sin(ny * 120.0 + std::sin(nx * 20.0) * 1.8);
                r = 160.0 * (1.0 - 0.65 * t) + wave;
                g = 95.0 * (1.0 - 0.55 * t) + wave * 0.6;
                b = 130.0 + 35.0 * t + wave * 0.4;
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
            applySaltPepper(img[y][x], x, y, 101u);
        }
    }
}

// Сцена 2 (1920x1080, с шумом для Задания 4.2): множество Мандельброта
static void genMedian2(RGBTRIPLE** img, int W, int H) {
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
            applySaltPepper(img[y][x], x, y, 202u);
        }
    }
}

// Сцена 3 (2560x1440, с шумом для Задания 4.2): волновая интерференция и геометрический центр
static void genMedian3(RGBTRIPLE** img, int W, int H) {
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
            applySaltPepper(img[y][x], x, y, 303u);
        }
    }
}

// Мультитекстурная сцена для Задания 4.3 (четкие области: гладкая, среднетекстурная, высококонтрастная)
static void genTextureScene(RGBTRIPLE** img, int W, int H, uint32_t seed) {
    #pragma omp parallel for schedule(static)
    for (int y = 0; y < H; y++) {
        double ny = static_cast<double>(y) / H;
        for (int x = 0; x < W; x++) {
            double nx = static_cast<double>(x) / W;
            double r = 0, g = 0, b = 0;

            if (ny < 0.33) {
                // Верхняя зона: плавный градиент (низкая энтропия, высокая однородность) + контрастный круг
                r = 70.0 + 90.0 * nx;
                g = 110.0 + 70.0 * ny;
                b = 185.0 - 40.0 * nx;
                double dx = (nx - 0.25) * (static_cast<double>(W) / H);
                double dy = ny - 0.16;
                if (dx * dx + dy * dy < 0.012) {
                    // Внутри круга — мелкая высококонтрастная решётка
                    int pat = ((x / 2) ^ (y / 2)) & 1;
                    r = pat ? 245.0 : 20.0; g = pat ? 235.0 : 25.0; b = pat ? 220.0 : 30.0;
                }
            } else if (ny < 0.67) {
                // Средняя зона: слева — волнистая умеренная текстура, справа — кирпичная кладка / штриховка
                if (nx < 0.5) {
                    double wv = std::sin(x * 0.22 + std::sin(y * 0.12) * 3.0) * std::cos(y * 0.18);
                    double base = 135.0 + 55.0 * wv;
                    r = base * 0.95; g = base * 1.02; b = base * 0.85;
                } else {
                    int row = y / 14;
                    int col = (x + (row % 2) * 16) / 32;
                    int bx = (x + (row % 2) * 16) % 32;
                    int by = y % 14;
                    bool mortar = (bx < 2 || by < 2);
                    double jitter = static_cast<double>(hash32(static_cast<uint32_t>(col), static_cast<uint32_t>(row), seed) % 35u);
                    if (mortar) {
                        r = 210; g = 205; b = 195;
                    } else {
                        double grain = static_cast<double>(hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed + 7u) % 28u) - 14.0;
                        r = 145.0 + jitter + grain;
                        g = 75.0 + jitter * 0.5 + grain * 0.6;
                        b = 55.0 + jitter * 0.3 + grain * 0.4;
                    }
                }
            } else {
                // Нижняя зона: слева — плотная стохастическая зернистая текстура (высокая энтропия и дисперсия),
                // справа — однородные ступенчатые полосы с резкими границами
                if (nx < 0.52) {
                    uint32_t h1 = hash32(static_cast<uint32_t>(x), static_cast<uint32_t>(y), seed + 19u);
                    uint32_t h2 = hash32(static_cast<uint32_t>(x / 2), static_cast<uint32_t>(y / 2), seed + 37u);
                    double val = 25.0 + (h1 % 165u) + (h2 % 65u);
                    r = val; g = val * 0.96; b = val * 0.90;
                } else {
                    int block = static_cast<int>((nx - 0.52) * 14.0);
                    double tone = (block % 2 == 0) ? 45.0 : 225.0;
                    r = tone; g = tone * 0.95; b = 255.0 - tone * 0.4;
                }
            }

            // Чёткая диагональная полоса через всё изображение для визуализации отклика на контуры
            if (std::abs((nx - 0.5) - (ny - 0.5) * 0.9) < 0.012) {
                r = 240; g = 240; b = 240;
            }
            img[y][x] = { clampByte(b), clampByte(g), clampByte(r) };
        }
    }
}

int main() {
    struct Spec { int w; int h; const char* path; int kind; uint32_t seed; };
    Spec specs[] = {
        { 1280,  720, "Lab4/images/input_1.bmp",     1, 101u },
        { 1920, 1080, "Lab4/images/input_2.bmp",     2, 202u },
        { 2560, 1440, "Lab4/images/input_3.bmp",     3, 303u },
        { 1280,  720, "Lab4/images/input_tex_1.bmp", 4, 404u },
        { 2560, 1440, "Lab4/images/input_tex_2.bmp", 4, 505u },
    };
    for (const auto& sp : specs) {
        std::cout << "Generating " << sp.path << " (" << sp.w << "x" << sp.h << ")..." << std::flush;
        RGBTRIPLE** img = BMPAlloc(sp.w, sp.h);
        if (sp.kind == 1) genMedian1(img, sp.w, sp.h);
        else if (sp.kind == 2) genMedian2(img, sp.w, sp.h);
        else if (sp.kind == 3) genMedian3(img, sp.w, sp.h);
        else genTextureScene(img, sp.w, sp.h, sp.seed);
        BMPWrite(img, sp.w, sp.h, sp.path);
        BMPFree(img);
        std::cout << " OK\n";
    }
    return 0;
}

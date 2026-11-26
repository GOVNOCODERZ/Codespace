#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "../src/BMPFileRW.h"

namespace {

inline unsigned char clampU8(double v) {
    if (v < 0.0) return 0;
    if (v > 255.0) return 255;
    return static_cast<unsigned char>(v + 0.5);
}

inline double hashNoise(int x, int y, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + seed * 1442695041u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= (h >> 16);
    return (static_cast<double>(h & 0xFFFFu) / 32767.5) - 1.0; // [-1, 1]
}

struct RGBImage {
    int w, h;
    std::vector<unsigned char> r, g, b;
    RGBImage(int width, int height) : w(width), h(height), r(w * h, 0), g(w * h, 0), b(w * h, 0) {}

    void setPx(int x, int y, double rr, double gg, double bb) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        size_t idx = static_cast<size_t>(y) * w + x;
        r[idx] = clampU8(rr);
        g[idx] = clampU8(gg);
        b[idx] = clampU8(bb);
    }

    void fillRect(int x0, int y0, int x1, int y1, double rr, double gg, double bb) {
        for (int y = std::max(0, y0); y < std::min(h, y1); ++y)
            for (int x = std::max(0, x0); x < std::min(w, x1); ++x)
                setPx(x, y, rr, gg, bb);
    }

    void save(const std::string& path) {
        RGBTRIPLE** rgb = BMPAlloc(w, h);
        for (int y = 0; y < h; ++y) {
            const size_t row = static_cast<size_t>(y) * w;
            for (int x = 0; x < w; ++x) {
                rgb[y][x].rgbtRed   = r[row + x];
                rgb[y][x].rgbtGreen = g[row + x];
                rgb[y][x].rgbtBlue  = b[row + x];
            }
        }
        BMPWrite(rgb, w, h, path.c_str());
        BMPFree(rgb);
    }
};

// НД 1 (1280x720): Калибровочная шахматная доска + геометрические фигуры с чёткими углами
void generateDataset1(const std::string& path) {
    const int W = 1280, H = 720;
    RGBImage img(W, H);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            double n = hashNoise(x, y, 101) * 4.0;
            // Спокойный светлый градиентный фон
            double bgR = 215.0 - 25.0 * (double)y / H;
            double bgG = 222.0 - 20.0 * (double)y / H;
            double bgB = 232.0 - 15.0 * (double)y / H;
            img.setPx(x, y, bgR + n, bgG + n, bgB + n);
        }
    }
    // Шахматная область слева (6x4 клетки по 80 пикс.)
    int ox = 90, oy = 160, cell = 80;
    for (int cy = 0; cy < 5; ++cy) {
        for (int cx = 0; cx < 6; ++cx) {
            bool dark = ((cx + cy) % 2 == 0);
            double c = dark ? 28.0 : 242.0;
            for (int dy = 0; dy < cell; ++dy) {
                for (int dx = 0; dx < cell; ++dx) {
                    int px = ox + cx * cell + dx;
                    int py = oy + cy * cell + dy;
                    double n = hashNoise(px, py, 102) * 3.0;
                    img.setPx(px, py, c + n, c + n, (dark ? c + 12.0 : c) + n);
                }
            }
        }
    }
    // Справа: архитектурный фасад с контрастными окнами
    img.fillRect(680, 120, 1180, 620, 52, 65, 84);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 5; ++col) {
            int wx0 = 715 + col * 90;
            int wy0 = 155 + row * 110;
            bool lit = ((row * 3 + col) % 3 != 1);
            img.fillRect(wx0, wy0, wx0 + 56, wy0 + 70, lit ? 245 : 180, lit ? 232 : 200, lit ? 175 : 225);
        }
    }
    // Треугольник / крыша над фасадом
    for (int y = 40; y < 120; ++y) {
        double t = (double)(y - 40) / 80.0;
        int lx = (int)(930 - t * 260);
        int rx = (int)(930 + t * 260);
        for (int x = lx; x <= rx; ++x) {
            double n = hashNoise(x, y, 103) * 3.0;
            img.setPx(x, y, 155 + n, 62 + n, 52 + n);
        }
    }
    img.save(path);
}

// НД 2 (1920x1080): Городской квартал (многоэтажки с окнами) + контрастная калибровочная сетка
void generateDataset2(const std::string& path) {
    const int W = 1920, H = 1080;
    RGBImage img(W, H);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            double n = hashNoise(x, y, 201) * 4.0;
            double t = (double)y / H;
            img.setPx(x, y, 36 + 70 * t + n, 58 + 90 * t + n, 110 + 95 * t + n);
        }
    }
    // Три высотных здания с контрастными окнами
    struct Building { int x0, y0, x1, y1, rows, cols; };
    Building bs[3] = {
        {120, 260, 600, 1000, 7, 5},
        {690, 140, 1230, 1000, 8, 6},
        {1320, 320, 1800, 1000, 6, 5}
    };
    for (int bi = 0; bi < 3; ++bi) {
        const auto& b = bs[bi];
        img.fillRect(b.x0, b.y0, b.x1, b.y1, 32 + bi * 12, 38 + bi * 10, 48 + bi * 12);
        int bw = b.x1 - b.x0;
        int bh = b.y1 - b.y0;
        int cw = (bw - 40) / b.cols;
        int ch = (bh - 40) / b.rows;
        for (int r = 0; r < b.rows; ++r) {
            for (int c = 0; c < b.cols; ++c) {
                int wx0 = b.x0 + 24 + c * cw;
                int wy0 = b.y0 + 24 + r * ch;
                bool warm = ((r + c + bi) % 2 == 0);
                img.fillRect(wx0, wy0, wx0 + cw - 24, wy0 + ch - 26,
                             warm ? 248 : 195, warm ? 230 : 220, warm ? 155 : 245);
            }
        }
    }
    // Шахматная полоса на переднем плане
    int cell = 64;
    for (int y = 960; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            bool dark = (((x / cell) + ((y - 960) / cell)) % 2 == 0);
            double c = dark ? 22.0 : 238.0;
            double n = hashNoise(x, y, 202) * 3.0;
            img.setPx(x, y, c + n, c + n, c + n);
        }
    }
    img.save(path);
}

// НД 3 (2560x1440): Сцена высокого разрешения — инженерная сетка, ромбы, здания и шахматные блоки
void generateDataset3(const std::string& path) {
    const int W = 2560, H = 1440;
    RGBImage img(W, H);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            double n = hashNoise(x, y, 301) * 4.0;
            double tx = (double)x / W, ty = (double)y / H;
            img.setPx(x, y, 200 - 30 * ty + n, 210 - 25 * tx + n, 224 - 20 * ty + n);
        }
    }
    // Левый блок: контрастная шахматная матрица 8x6
    int cell = 110;
    for (int cy = 0; cy < 6; ++cy) {
        for (int cx = 0; cx < 8; ++cx) {
            bool dark = ((cx + cy) % 2 == 0);
            double c = dark ? 25.0 : 244.0;
            img.fillRect(100 + cx * cell, 140 + cy * cell, 100 + (cx + 1) * cell, 140 + (cy + 1) * cell, c, c, c);
        }
    }
    // Правый блок: архитектурный комплекс с сеткой окон
    img.fillRect(1120, 140, 2440, 1280, 42, 52, 68);
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 9; ++c) {
            int wx0 = 1170 + c * 138;
            int wy0 = 190 + r * 150;
            bool gold = ((r * 2 + c) % 3 == 0);
            img.fillRect(wx0, wy0, wx0 + 88, wy0 + 96,
                         gold ? 250 : 210, gold ? 226 : 230, gold ? 145 : 248);
        }
    }
    // Нижний левый блок: вложенные контрастные ромбы и квадраты
    int cx = 540, cy = 1090;
    for (int y = 850; y < 1330; ++y) {
        for (int x = 140; x < 940; ++x) {
            int dx = std::abs(x - cx);
            int dy = std::abs(y - cy);
            int manh = dx + dy;
            if (manh < 220) {
                bool band = ((manh / 44) % 2 == 0);
                double v = band ? 30.0 : 242.0;
                double n = hashNoise(x, y, 302) * 3.0;
                img.setPx(x, y, v + n, v + n, v + n);
            }
        }
    }
    img.save(path);
}

} // namespace

int main() {
    generateDataset1("Lab6/images/input_1.bmp");
    generateDataset2("Lab6/images/input_2.bmp");
    generateDataset3("Lab6/images/input_3.bmp");
    std::cout << "Generated Lab 6 BMP datasets: input_1 (1280x720), input_2 (1920x1080), input_3 (2560x1440)\n";
    return 0;
}

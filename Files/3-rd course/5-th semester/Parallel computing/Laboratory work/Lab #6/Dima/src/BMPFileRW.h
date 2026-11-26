#pragma once

#include <Windows.h>
#include <fstream>
#include <stdexcept>
#include <string>

void BMPRead(RGBTRIPLE**& rgb, BITMAPFILEHEADER& header, BITMAPINFOHEADER& bmiHeader, const char* fin);
void BMPWrite(RGBTRIPLE**& rgb, int imWidth, int imHeight, const char* fout);
void BMPFree(RGBTRIPLE**& rgb);
RGBTRIPLE** BMPAlloc(int imWidth, int imHeight);

unsigned char get_row_data_padding(unsigned int width);
unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height);

// Определение величины дополнения (padding) на случай, если ширина изображения в байтах не кратна 4
inline unsigned char get_row_data_padding(unsigned int width) {
    return (width % 4 == 0) ? 0 : static_cast<unsigned char>(4 - (width * sizeof(RGBTRIPLE)) % 4);
}

// Вычисление общего размера 24-битного BMP-файла в байтах
inline unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height) {
    return static_cast<unsigned int>(
        sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
        height * (width * sizeof(RGBTRIPLE) + get_row_data_padding(width))
    );
}

// Выделение непрерывного блока памяти для хранения матрицы пикселей RGBTRIPLE[height][width]
inline RGBTRIPLE** BMPAlloc(int imWidth, int imHeight) {
    RGBTRIPLE** rgb = new RGBTRIPLE*[imHeight];
    rgb[0] = new RGBTRIPLE[static_cast<size_t>(imWidth) * static_cast<size_t>(imHeight)];
    for (int i = 1; i < imHeight; i++) {
        rgb[i] = &rgb[0][static_cast<size_t>(imWidth) * static_cast<size_t>(i)];
    }
    return rgb;
}

// Освобождение памяти, выделенной функциями BMPRead / BMPAlloc
inline void BMPFree(RGBTRIPLE**& rgb) {
    if (rgb != nullptr) {
        delete[] rgb[0];
        delete[] rgb;
        rgb = nullptr;
    }
}

// Чтение 24-битного несжатого BMP-файла в двумерный массив структур RGBTRIPLE
inline void BMPRead(RGBTRIPLE**& rgb, BITMAPFILEHEADER& header,
                    BITMAPINFOHEADER& bmiHeader, const char* fin) {
    // Открываем файл BMP в двоичном режиме
    std::ifstream InFile(fin, std::ios::binary);
    if (!InFile.is_open()) {
        throw std::runtime_error(std::string("Не удалось открыть входной BMP-файл: ") + fin);
    }

    // Считываем заголовок файла (14 байт)
    InFile.read(reinterpret_cast<char*>(&header), sizeof(BITMAPFILEHEADER));
    // Считываем заголовочную часть растрового изображения (40 байт)
    InFile.read(reinterpret_cast<char*>(&bmiHeader), sizeof(BITMAPINFOHEADER));

    if (header.bfType != (('M' << 8) + 'B') || bmiHeader.biBitCount != 24 || bmiHeader.biCompression != 0) {
        throw std::runtime_error(std::string("Неподдерживаемый формат BMP (требуется 24-бит без сжатия): ") + fin);
    }

    // Выделяем память под массив указателей строк и единую область данных (оптимизация кэша)
    rgb = BMPAlloc(bmiHeader.biWidth, bmiHeader.biHeight);

    // Перемещаемся на начало пиксельных данных
    InFile.seekg(header.bfOffBits, std::ios::beg);

    // Вычисляем выравнивание строк до кратности 4 байтам
    int padding = get_row_data_padding(static_cast<unsigned int>(bmiHeader.biWidth));
    char tmp[3] = { 0, 0, 0 };

    // Считываем строки снизу вверх (стандартный порядок хранения DIB в BMP)
    for (int i = 0; i < bmiHeader.biHeight; i++) {
        InFile.read(reinterpret_cast<char*>(&rgb[bmiHeader.biHeight - 1 - i][0]),
                    static_cast<std::streamsize>(bmiHeader.biWidth) * static_cast<std::streamsize>(sizeof(RGBTRIPLE)));
        if (padding > 0) {
            InFile.read(tmp, padding);
        }
    }

    InFile.close();
}

// Запись двумерного массива структур RGBTRIPLE в 24-битный BMP-файл
inline void BMPWrite(RGBTRIPLE**& rgb, int imWidth, int imHeight, const char* fout) {
    // Открываем файл для записи изображения в формат BMP
    std::ofstream OutFile(fout, std::ios::binary);
    if (!OutFile.is_open()) {
        throw std::runtime_error(std::string("Не удалось создать выходной BMP-файл: ") + fout);
    }

    // Формируем заголовок файла BMP
    BITMAPFILEHEADER header{};
    header.bfType = ('M' << 8) + 'B';
    header.bfSize = bmp24b_file_size_calc(static_cast<unsigned int>(imWidth), static_cast<unsigned int>(imHeight));
    header.bfOffBits = 54;

    // Формируем заголовок растровой информации
    BITMAPINFOHEADER bmiHeader{};
    bmiHeader.biSize = 40;
    bmiHeader.biWidth = imWidth;
    bmiHeader.biHeight = imHeight;
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = 24;
    bmiHeader.biSizeImage = header.bfSize - static_cast<DWORD>(sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER));

    // Записываем оба заголовка в файл
    OutFile.write(reinterpret_cast<const char*>(&header), sizeof(BITMAPFILEHEADER));
    OutFile.write(reinterpret_cast<const char*>(&bmiHeader), sizeof(BITMAPINFOHEADER));

    // Вычисляем выравнивание строк до кратности 4 байтам
    int padding = get_row_data_padding(static_cast<unsigned int>(bmiHeader.biWidth));
    char tmp[3] = { 0, 0, 0 };

    // Записываем строки снизу вверх
    for (int i = 0; i < bmiHeader.biHeight; i++) {
        OutFile.write(reinterpret_cast<const char*>(&rgb[bmiHeader.biHeight - i - 1][0]),
                      static_cast<std::streamsize>(bmiHeader.biWidth) * static_cast<std::streamsize>(sizeof(RGBTRIPLE)));
        if (padding > 0) {
            OutFile.write(tmp, padding);
        }
    }

    OutFile.close();
}

// bmp_io.h
// Чтение/запись 24-битных BMP-изображений без сжатия.
// Основано на материалах курса (структуры BITMAPFILEHEADER/BITMAPINFOHEADER).
// Доработано: портируемость на Linux/GCC (без <Windows.h>) + отступы
// (padding) строк до кратности 4 байт, как того требует формат BMP.
#pragma once

#include <fstream>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
    #include <Windows.h>
#else
    // Собственные определения структур BMP (бинарно совместимы с Windows.h),
    // используются при сборке вне Windows (например, для тестирования на Linux).
    #pragma pack(push, 1)
    struct BITMAPFILEHEADER {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    };
    struct BITMAPINFOHEADER {
        uint32_t biSize;
        int32_t  biWidth;
        int32_t  biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t  biXPelsPerMeter;
        int32_t  biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };
    struct RGBTRIPLE {
        uint8_t rgbtBlue;
        uint8_t rgbtGreen;
        uint8_t rgbtRed;
    };
    #pragma pack(pop)
#endif

// Количество байт дополнения (padding) на строку изображения, чтобы длина
// строки в файле была кратна 4 байтам (требование формата BMP).
inline unsigned char get_row_data_padding(unsigned int width) {
    unsigned int rowBytes = width * sizeof(RGBTRIPLE);
    return (rowBytes % 4 == 0) ? 0 : (unsigned char)(4 - (rowBytes % 4));
}

// Полный размер BMP-файла (заголовки + данные с учетом padding).
inline unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height) {
    return sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
         + height * (width * sizeof(RGBTRIPLE) + get_row_data_padding(width));
}

// Чтение BMP-файла. rgb[y][x] — после чтения индексация СВЕРХУ ВНИЗ
// (строка 0 — верхняя строка изображения), хотя в самом BMP-файле строки
// хранятся снизу вверх — это учитывается при чтении (см. зеркалирование
// по вертикали ниже).
inline void BMPRead(RGBTRIPLE**& rgb, BITMAPFILEHEADER& header,
                     BITMAPINFOHEADER& bmiHeader, const char* fin)
{
    std::ifstream InFile(fin, std::ios::binary);
    if (!InFile) {
        rgb = nullptr;
        return;
    }
    InFile.read((char*)(&header), sizeof(BITMAPFILEHEADER));
    InFile.read((char*)(&bmiHeader), sizeof(BITMAPINFOHEADER));

    rgb = new RGBTRIPLE * [bmiHeader.biHeight];
    rgb[0] = new RGBTRIPLE[(size_t)bmiHeader.biWidth * (size_t)bmiHeader.biHeight];
    for (int i = 1; i < bmiHeader.biHeight; i++) {
        rgb[i] = &rgb[0][(size_t)bmiHeader.biWidth * i];
    }

    InFile.seekg(header.bfOffBits, std::ios::beg);
    int padding = get_row_data_padding(bmiHeader.biWidth);
    char tmp[3] = { 0, 0, 0 };
    // В BMP строки хранятся снизу вверх, поэтому читаем в rgb[height-1-i],
    // в итоге rgb[0] соответствует верхней строке изображения.
    for (int i = 0; i < bmiHeader.biHeight; i++) {
        InFile.read((char*)(&rgb[bmiHeader.biHeight - 1 - i][0]), bmiHeader.biWidth * sizeof(RGBTRIPLE));
        if (padding > 0) InFile.read((char*)(&tmp[0]), padding);
    }
    InFile.close();
}

// Запись BMP-файла (24 бита на пиксель, без сжатия).
inline void BMPWrite(RGBTRIPLE**& rgb, int imWidth, int imHeight, const char* fout)
{
    std::ofstream OutFile(fout, std::ios::binary);

    BITMAPFILEHEADER header = { 0 };
    header.bfType = ('M' << 8) + 'B';
    header.bfSize = bmp24b_file_size_calc(imWidth, imHeight);
    header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bmiHeader = { 0 };
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = imWidth;
    bmiHeader.biHeight = imHeight;
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = 24;
    bmiHeader.biSizeImage = header.bfSize - sizeof(BITMAPINFOHEADER) - sizeof(BITMAPFILEHEADER);

    OutFile.write((char*)(&header), sizeof(BITMAPFILEHEADER));
    OutFile.write((char*)(&bmiHeader), sizeof(BITMAPINFOHEADER));

    int padding = get_row_data_padding(bmiHeader.biWidth);
    char tmp[3] = { 0, 0, 0 };
    for (int i = 0; i < bmiHeader.biHeight; i++) {
        OutFile.write((char*)&(rgb[bmiHeader.biHeight - i - 1][0]), bmiHeader.biWidth * sizeof(RGBTRIPLE));
        if (padding > 0) OutFile.write((char*)(&tmp[0]), padding);
    }
    OutFile.close();
}

// Освобождение памяти, выделенной BMPRead.
inline void BMPFree(RGBTRIPLE**& rgb) {
    if (rgb) {
        delete[] rgb[0];
        delete[] rgb;
        rgb = nullptr;
    }
}

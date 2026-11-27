#pragma once

#include <cstdint>
#include <cstddef>
#include <fstream>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t bfReserved;
    uint32_t bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)

typedef struct {
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
} BITMAPINFOHEADER;

typedef struct tagRGBTRIPLE {
    uint8_t rgbtBlue;
    uint8_t rgbtGreen;
    uint8_t rgbtRed;
} RGBTRIPLE;

inline RGBTRIPLE** AllocImage(int W, int H) {
    RGBTRIPLE** m = new RGBTRIPLE*[H];
    m[0] = new RGBTRIPLE[(size_t)W * H];
    for (int y = 1; y < H; y++)
        m[y] = m[0] + (size_t)y * W;
    return m;
}

inline void FreeImage(RGBTRIPLE** m) {
    if (!m) return;
    delete[] m[0];
    delete[] m;
}

inline unsigned char get_row_data_padding(unsigned int width) {
    return (width % 4 == 0) ? 0 : (4 - (width * sizeof(RGBTRIPLE)) % 4);
}

inline unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height) {
    return (unsigned int)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
        (size_t)height * ((size_t)width * sizeof(RGBTRIPLE) + get_row_data_padding(width)));
}

inline void BMPRead(RGBTRIPLE**& rgb, BITMAPFILEHEADER& header, BITMAPINFOHEADER& bmiHeader, const char* fin) {
    std::ifstream InFile(fin, std::ios::binary);
    InFile.read((char*)(&header), sizeof(BITMAPFILEHEADER));
    InFile.read((char*)(&bmiHeader), sizeof(BITMAPINFOHEADER));
    rgb = AllocImage(bmiHeader.biWidth, bmiHeader.biHeight);
    InFile.seekg(header.bfOffBits, std::ios::beg);
    int padding = get_row_data_padding(bmiHeader.biWidth);
    char tmp[3] = { 0, 0, 0 };
    for (int i = 0; i < bmiHeader.biHeight; i++) {
        InFile.read((char*)(&rgb[bmiHeader.biHeight - 1 - i][0]), (std::streamsize)bmiHeader.biWidth * sizeof(RGBTRIPLE));
        if (padding > 0)
            InFile.read(tmp, padding);
    }
    InFile.close();
}

inline void BMPWrite(RGBTRIPLE**& rgb, int imWidth, int imHeight, const char* fout) {
    std::ofstream OutFile(fout, std::ios::binary);
    BITMAPFILEHEADER header = {};
    header.bfType = ('M' << 8) + 'B';
    header.bfSize = bmp24b_file_size_calc(imWidth, imHeight);
    header.bfOffBits = 54;

    BITMAPINFOHEADER bmiHeader = {};
    bmiHeader.biSize = 40;
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
        OutFile.write((char*)(&rgb[bmiHeader.biHeight - i - 1][0]), (std::streamsize)bmiHeader.biWidth * sizeof(RGBTRIPLE));
        if (padding > 0)
            OutFile.write(tmp, padding);
    }
    OutFile.close();
}

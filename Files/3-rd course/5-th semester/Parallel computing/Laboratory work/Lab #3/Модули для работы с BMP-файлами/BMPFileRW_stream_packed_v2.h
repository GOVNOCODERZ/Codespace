#include <fstream>


//          ..    
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
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

//
typedef struct tagRGBTRIPLE {
    uint8_t rgbtBlue;
    uint8_t rgbtGreen;
    uint8_t rgbtRed;
} RGBTRIPLE;



void BMPRead(RGBTRIPLE **&, BITMAPFILEHEADER&, BITMAPINFOHEADER&, const char *);
void BMPWrite(RGBTRIPLE**& rgb, int imWidth, int imHeight, const char* fout);

unsigned char get_row_data_padding(unsigned int width);
unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height);

//           4
unsigned char get_row_data_padding(unsigned int width) {
	return (width % 4 == 0) ? 0 : (4 - (width * sizeof(RGBTRIPLE)) % 4);
}

//   BMP 
unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height) {
	return sizeof(BITMAPFILEHEADER)+ sizeof(BITMAPINFOHEADER) +  height * (width* sizeof(RGBTRIPLE) + get_row_data_padding(width));
}


void BMPRead(RGBTRIPLE** &rgb, BITMAPFILEHEADER &header, \
	BITMAPINFOHEADER &bmiHeader, const char* fin)
{
    //   BMP
	std::ifstream InFile(fin, std::ios::binary);
	//   
	InFile.read((char*)(&header), sizeof(BITMAPFILEHEADER));
	//    
	InFile.read((char*)(&bmiHeader), sizeof(BITMAPINFOHEADER));
    //     RGB   RGBTRIPLE
    rgb = new RGBTRIPLE*[bmiHeader.biHeight];
	//       
	rgb[0] = new RGBTRIPLE[bmiHeader.biWidth*bmiHeader.biHeight];
    for (int i = 1; i < bmiHeader.biHeight; i++)
    {   //     
        rgb[i] = &rgb[0][bmiHeader.biWidth*i];
    }
	//     
	InFile.seekg(header.bfOffBits, std::ios::beg);
	//           4
	int padding = get_row_data_padding(bmiHeader.biWidth);
	char tmp[3] = { 0,0,0 };
	//       RGB 
    for (int i = 0; i < bmiHeader.biHeight; i++)
    {
			InFile.read((char*)(&rgb[bmiHeader.biHeight-1-i][0]), bmiHeader.biWidth*sizeof(RGBTRIPLE)); // RGBTRIPLE {Blue Green bRed;}
			if (padding > 0)
				InFile.read((char*)(&tmp[0]), padding);
    }
    //  
	InFile.close();
}

void BMPWrite(RGBTRIPLE**& rgb, int imWidth , int imHeight, const char* fout)
{
	//        BMP
	std::ofstream OutFile(fout, std::ios::binary);
	//      BMP
	BITMAPFILEHEADER header = { 0 };
	header.bfType = ('M' << 8) + 'B';
	header.bfSize = bmp24b_file_size_calc(imWidth, imHeight);;
	header.bfOffBits = 54;
	//       
	BITMAPINFOHEADER bmiHeader = { 0 };
	//   
	bmiHeader.biSize = 40;
	bmiHeader.biWidth = imWidth;
	bmiHeader.biHeight = imHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 24;
	bmiHeader.biSizeImage = header.bfSize - sizeof(BITMAPINFOHEADER)- sizeof(BITMAPFILEHEADER);
	////   
	OutFile.write((char*)(&header), sizeof(BITMAPFILEHEADER));
	////    
	OutFile.write((char*)(&bmiHeader), sizeof(BITMAPINFOHEADER));
	//           4
	int padding = get_row_data_padding(bmiHeader.biWidth);
	char tmp[3] = { 0,0,0 };
	//       RGBTRIPLE   
	for (int i = 0; i < bmiHeader.biHeight; i++)
	{
		OutFile.write((char*)&(rgb[bmiHeader.biHeight - i - 1][0]), bmiHeader.biWidth * sizeof(RGBTRIPLE));
		if (padding > 0)
			OutFile.write((char*)(&tmp[0]), padding);
	}
	//  
	OutFile.close();
}


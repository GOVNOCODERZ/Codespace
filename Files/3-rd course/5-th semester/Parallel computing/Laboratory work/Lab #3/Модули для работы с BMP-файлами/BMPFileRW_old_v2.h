//          ..    
typedef struct {
	unsigned short    bfType;
	unsigned long    bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned long    bfOffBits;
} BITMAPFILEHEADER;



typedef struct {
	unsigned long       biSize;
	long        biWidth;
	long        biHeight;
	unsigned short        biPlanes;
	unsigned short       biBitCount;
	unsigned long       biCompression;
	unsigned long       biSizeImage;
	long        biXPelsPerMeter;
	long        biYPelsPerMeter;
	unsigned long       biClrUsed;
	unsigned long       biClrImportant;
} BITMAPINFOHEADER;

//
typedef struct tagRGBTRIPLE {
	unsigned char rgbtBlue;
	unsigned char     rgbtGreen;
	unsigned char     rgbtRed;
} RGBTRIPLE;



void BMPRead(RGBTRIPLE **&, BITMAPFILEHEADER&, BITMAPINFOHEADER&, const char *);
void BMPWrite(RGBTRIPLE**& rgb, int imWidth, int imHeight, const char* fout);
static unsigned char get_row_data_padding(unsigned int width);
unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height);

//           4
static unsigned char get_row_data_padding(unsigned int width) {
	return (width % 4 == 0) ? 0 : (4 - (width * sizeof(RGBTRIPLE)) % 4);
}

//   BMP 
unsigned int bmp24b_file_size_calc(unsigned int width, unsigned int height) {
	return sizeof(BITMAPFILEHEADER)+ sizeof(BITMAPINFOHEADER) +  height * (width* sizeof(RGBTRIPLE) + get_row_data_padding(width));
}


void BMPRead(RGBTRIPLE** &rgb, BITMAPFILEHEADER &header, \
	BITMAPINFOHEADER &bmiHeader, const char* fin)
{
	//  
	FILE* pFile;
	fopen_s(&pFile, fin, "rb");
	//   
	fread_s(&(header.bfType), sizeof(unsigned short), sizeof(unsigned short), 1, pFile);
	fread_s(&(header.bfSize), sizeof(unsigned long), sizeof(unsigned long), 1, pFile);
	fread_s(&(header.bfReserved1), sizeof(unsigned short), sizeof(unsigned short), 1, pFile);
	fread_s(&(header.bfReserved2), sizeof(unsigned short), sizeof(unsigned short), 1, pFile);
	fread_s(&(header.bfOffBits), sizeof(unsigned long), sizeof(unsigned long), 1, pFile);
	//    
	fread_s(&bmiHeader, sizeof(BITMAPINFOHEADER), sizeof(BITMAPINFOHEADER), 1, pFile);
    //     RGB   RGBTRIPLE
    rgb = new RGBTRIPLE*[bmiHeader.biHeight];
	//       
	rgb[0] = new RGBTRIPLE[bmiHeader.biWidth*bmiHeader.biHeight];
    for (int i = 1; i < bmiHeader.biHeight; i++)
    {   //     
        rgb[i] = &rgb[0][bmiHeader.biWidth*i];
    }
	//     
	fseek(pFile, header.bfOffBits, SEEK_SET);
	//           4
	int padding = get_row_data_padding(bmiHeader.biWidth);
	char tmp[3] = { 0,0,0 };
	//       RGB 
    for (int i = 0; i < bmiHeader.biHeight; i++)
    {
		fread_s(&rgb[bmiHeader.biHeight - 1 - i][0], (bmiHeader.biWidth * sizeof(RGBTRIPLE)), sizeof(RGBTRIPLE), bmiHeader.biWidth, pFile);
		if (padding > 0)
			fread_s(&tmp[0], sizeof(tmp), sizeof(unsigned char), padding, pFile);
    }
    //  
	fclose(pFile);
}

void BMPWrite(RGBTRIPLE**& rgb, int imWidth , int imHeight, const char* fout)
{
	//        BMP
	FILE* oFile;
	fopen_s(&oFile, fout, "wb");
	//      BMP
	BITMAPFILEHEADER header = { 0 }; //  
	header.bfType = ('M' << 8) + 'B';
	header.bfSize = bmp24b_file_size_calc(imWidth, imHeight);;
	header.bfOffBits = 54;
	//       
	BITMAPINFOHEADER bmiHeader = { 0 }; //  
	//   
	bmiHeader.biSize = 40;
	bmiHeader.biWidth = imWidth;
	bmiHeader.biHeight = imHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 24;
	bmiHeader.biSizeImage = header.bfSize - sizeof(BITMAPINFOHEADER)- sizeof(BITMAPFILEHEADER);
	////   
	fwrite(&(header.bfType), sizeof(unsigned short), 1, oFile);
	fwrite(&(header.bfSize), sizeof(unsigned long),  1, oFile);
	fwrite(&(header.bfReserved1), sizeof(unsigned short),  1, oFile);
	fwrite(&(header.bfReserved2), sizeof(unsigned short),  1, oFile);
	fwrite(&(header.bfOffBits), sizeof(unsigned long),  1, oFile);
	////    
	fwrite(&bmiHeader, sizeof(BITMAPINFOHEADER), 1, oFile);
	//           4
	int padding = get_row_data_padding(bmiHeader.biWidth);
	char tmp[3] = { 0,0,0 };
	//       RGBTRIPLE   
	for (int i = 0; i < bmiHeader.biHeight; i++)
	{
		fwrite(&(rgb[bmiHeader.biHeight - i - 1][0]), sizeof(unsigned char), bmiHeader.biWidth * sizeof(RGBTRIPLE), oFile);
		//    
		if (padding > 0)
			fwrite(&tmp[0], sizeof(unsigned char), padding, oFile);
			
	}
	//  
	fclose(oFile);
}


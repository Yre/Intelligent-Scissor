#ifndef BITMAP_H
#define BITMAP_H

#include <stdio.h>
#include <string.h>

#define BMP_BI_RGB        0L


typedef struct { 
	unsigned short	bfType; 
	unsigned int	bfSize; 
	unsigned short	bfReserved1; 
	unsigned short	bfReserved2; 
	unsigned int	bfOffBits; 
} BMP_BITMAPFILEHEADER; 
 
typedef struct { 
	unsigned int	biSize; 
	int	biWidth; 
	int	biHeight; 
	unsigned short	biPlanes; 
	unsigned short	biBitCount; 
	unsigned int	biCompression; 
	unsigned int	biSizeImage; 
	int	biXPelsPerMeter; 
	int	biYPelsPerMeter; 
	unsigned int	biClrUsed; 
	unsigned int	biClrImportant; 
} BMP_BITMAPINFOHEADER; 

// global I/O routines
extern unsigned char* readBMP(const char* fname, int& width, int& height );
extern void writeBMP( char* iname, int width, int height, unsigned char* data ); 

#endif
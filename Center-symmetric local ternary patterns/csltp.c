#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <Windows.h>

#define BitmapFileSize(file) ((unsigned long)((BITMAPFILEHEADER*)file)->bfSize)
#define BitmapImage(file) ((unsigned char*)file + ((BITMAPFILEHEADER*)file)->bfOffBits)
#define BitmapWidth(file) ((long)((BITMAPINFOHEADER*)((unsigned char*)file + sizeof(BITMAPFILEHEADER)))->biWidth)
#define BitmapHeight(file) ((long)((BITMAPINFOHEADER*)((unsigned char*)file + sizeof(BITMAPFILEHEADER)))->biHeight)

typedef void* BitmapFile;
bool BitmapOpenFile(char *pFileName, BitmapFile *pBitmapFile);
bool BitmapWriteFile(char *pFileName, BitmapFile BitmapFile);
bool BitmapCopyFile(BitmapFile *SourceFile, BitmapFile *pCopyFile);

void RGBToGrey(BitmapFile RGBFile, BitmapFile GreyFile);
void GreyToCSLTP(BitmapFile GreyFile, BitmapFile CSLTPFile, int T);

bool HTMLprint(char *pFileName, BitmapFile BitmapFile);
bool CSVprint(char *pFileName, BitmapFile BitmapFile);

int main(int argc, char *argv[])
{
	BitmapFile RGBFile = NULL;
	bool bResult = BitmapOpenFile("lena.bmp", &RGBFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	BitmapFile GreyFile = NULL;
	bResult = BitmapCopyFile(RGBFile, &GreyFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	BitmapFile CSLTPFile = NULL;
	bResult = BitmapCopyFile(RGBFile, &CSLTPFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	RGBToGrey(RGBFile, GreyFile);
	bResult = BitmapWriteFile("lenaGrey.bmp", GreyFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	bResult = CSVprint("lenaGrey.csv", GreyFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	GreyToCSLTP(GreyFile, CSLTPFile, 20);
	bResult = BitmapWriteFile("lenaCSLTP.bmp", CSLTPFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	bResult = CSVprint("lenaCSLTP.csv", CSLTPFile);
	if (false == bResult)
	{
		return EXIT_FAILURE;
	}

	free(RGBFile);
	free(GreyFile);
	free(CSLTPFile);
	return EXIT_SUCCESS;
}


bool BitmapOpenFile(char *pFileName, BitmapFile *pBitmapFile)
{
	FILE *pFILE = fopen(pFileName, "rb");
	if (NULL == pFILE)
	{
		return false;
	}

	int Result = fseek(pFILE, 0L, SEEK_END);
	if (0 != Result)
	{
		return false;
	}

	long FileCurrent = ftell(pFILE);
	if (-1L == FileCurrent)
	{
		return false;
	}
	size_t FileSize = (size_t)FileCurrent;

	void *pBuffer = malloc(FileSize);
	if (NULL == pBuffer)
	{
		return false;
	}

	Result = fseek(pFILE, 0L, SEEK_SET);
	if (0 != Result)
	{
		return false;
	}

	size_t ResultSize = fread(pBuffer, sizeof(byte), FileSize, pFILE);
	if (FileSize != ResultSize)
	{
		return false;
	}

	Result = fclose(pFILE);
	if (0 != Result)
	{
		return false;
	}

	*pBitmapFile = pBuffer;
	return true;
}

bool BitmapWriteFile(char *pFileName, BitmapFile BitmapFile)
{
	FILE *pFILE = fopen(pFileName, "wb");
	if (NULL == pFILE)
	{
		return false;
	}

	size_t FileSize = BitmapFileSize(BitmapFile);

	size_t ResultSize = fwrite(BitmapFile, sizeof(byte), FileSize, pFILE);
	if (FileSize != ResultSize)
	{
		return false;
	}

	int Result = fclose(pFILE);
	if (0 != Result)
	{
		return false;
	}

	return true;
}

bool BitmapCopyFile(BitmapFile *SourceFile, BitmapFile *pCopyFile)
{
	size_t FileSize = BitmapFileSize(SourceFile);

	void *File = malloc(FileSize);
	if (NULL == File)
	{
		return false;
	}

	memcpy(File, SourceFile, FileSize);
	*pCopyFile = File;
	return true;
}

void RGBToGrey(BitmapFile RGBFile, BitmapFile GreyFile)
{
	unsigned char *pRGBImage = BitmapImage(RGBFile);
	unsigned char *pGreyImage = BitmapImage(GreyFile);

	long Count = BitmapWidth(RGBFile) * BitmapHeight(RGBFile) * 3;
	for (long Index = 0; Index < Count; Index += 3)
	{
		int Red = (int)*pRGBImage++;
		int Green = (int)*pRGBImage++;
		int Blue = (int)*pRGBImage++;

		int Grey = (Red * 299 + Green * 587 + Blue * 114 + 500) / 1000;
		memset(pGreyImage + Index, Grey, (size_t)3);
	}
}

void GreyToCSLTP(BitmapFile GreyFile, BitmapFile CSLTPFile, int T)
{
	unsigned char *pGreyImage = BitmapImage(GreyFile);
	unsigned char *pCSLTPImage = BitmapImage(CSLTPFile);
	long Width = BitmapWidth(GreyFile);
	long Height = BitmapHeight(GreyFile);
	memset(pCSLTPImage, 0, Width * Height * 3);

#define ReverseY(y) (Height - 1 - (y))
#define Array2D(x,y) (((x) + ReverseY(y) * Width) * 3)

	for (long y = 1; y < Height - 1; y++)
	{
		int s[4] = { 0 };
		for (long x = 1; x < Width - 1; x++)
		{
			s[0] = pGreyImage[Array2D(x + 1, y)] - pGreyImage[Array2D(x - 1, y)];
			s[1] = pGreyImage[Array2D(x + 1, y + 1)] - pGreyImage[Array2D(x - 1, y - 1)];
			s[2] = pGreyImage[Array2D(x, y + 1)] - pGreyImage[Array2D(x, y - 1)];
			s[3] = pGreyImage[Array2D(x - 1, y)] - pGreyImage[Array2D(x + 1, y)];

			int SCTLP = 0;

			for (int Index = 0; Index < 4; Index++)
			{
				int S = s[Index];
				if (S >= T)
				{
					S = 2;
				}
				else if (S > -T)
				{
					S = 1;
				}
				else
				{
					S = 0;
				}

				switch (Index)
				{
				case 0:
					SCTLP += S;
					break;
				case 1:
					SCTLP += S * 3;
					break;
				case 2:
					SCTLP += S * 3 * 3;
					break;
				case 3:
					SCTLP += S * 3 * 3 * 3;
					break;
				}
			}
			memset(&pCSLTPImage[Array2D(x, y)], SCTLP, 3);
		}
	}
}

bool HTMLprint(char *pFileName, BitmapFile BitmapFile)
{
#pragma pack(push)
#pragma pack(1)
	struct
	{
		unsigned short identifier;      // 0x0000
		unsigned int filesize;          // 0x0002
		unsigned int reserved;          // 0x0006
		unsigned int bitmap_dataoffset; // 0x000A
		unsigned int bitmap_headersize; // 0x000E
		unsigned int width;             // 0x0012
		unsigned int height;            // 0x0016
		unsigned short planes;          // 0x001A
		unsigned short bits_perpixel;   // 0x001C
		unsigned int compression;       // 0x001E
		unsigned int bitmap_datasize;   // 0x0022
		unsigned int hresolution;       // 0x0026
		unsigned int vresolution;       // 0x002A
		unsigned int usedcolors;        // 0x002E
		unsigned int importantcolors;   // 0x0032
		unsigned int palette;           // 0x0036
	}BMP;
#pragma pack(pop)

	memcpy(&BMP, BitmapFile, sizeof(BMP));

	FILE *pFILE = fopen(pFileName, "w");
	if (NULL == pFILE)
	{
		return false;
	}

	fprintf(pFILE, "identifier=%x<br/>\n", BMP.identifier);
	fprintf(pFILE, "filesize=%d<br/>\n", BMP.filesize);
	fprintf(pFILE, "reserved=%d<br/>\n", BMP.reserved);
	fprintf(pFILE, "bitmap_dataoffset=%d<br/>\n", BMP.bitmap_dataoffset);
	fprintf(pFILE, "bitmap_headersize=%d<br/>\n", BMP.bitmap_headersize);
	fprintf(pFILE, "width=%d<br/>\n", BMP.width);
	fprintf(pFILE, "height=%d<br/>\n", BMP.height);
	fprintf(pFILE, "planes=%d<br/>\n", BMP.planes);
	fprintf(pFILE, "bits_perpixel=%d<br/>\n", BMP.bits_perpixel);
	fprintf(pFILE, "compression=%d<br/>\n", BMP.compression);
	fprintf(pFILE, "bitmap_datasize=%d<br/>\n", BMP.filesize - BMP.bitmap_dataoffset);
	fprintf(pFILE, "hresolution=%d<br/>\n", BMP.hresolution);
	fprintf(pFILE, "vresolution=%d<br/>\n", BMP.vresolution);
	fprintf(pFILE, "usedcolors=%d<br/>\n", BMP.usedcolors);
	fprintf(pFILE, "importantcolors=%d<br/>\n", BMP.importantcolors);
	fprintf(pFILE, "palette=%d<br/>\n", BMP.palette);
	fprintf(pFILE, "<br><br><a href=../../index.html>Home</a><br>\n");

	int Result = fclose(pFILE);
	if (0 != Result)
	{
		return false;
	}

	return true;
}

bool CSVprint(char *pFileName, BitmapFile BitmapFile)
{
	unsigned char *pImage = BitmapImage(BitmapFile);
	long Width = BitmapWidth(BitmapFile);
	long Height = BitmapHeight(BitmapFile);

	FILE *pFILE = fopen(pFileName, "w");
	if (NULL == pFILE)
	{
		return false;
	}

#define ReverseY(y) (Height - 1 - (y))
#define Array2D(x,y) (((x) + ReverseY(y) * Width) * 3)

	for (long y = 0; y < Height; y++)
	{
		for (long x = 0; x < Width - 1; x++)
		{
			int Result = fprintf(pFILE, "%d, ", pImage[Array2D(x, y)]);
			if (0 > Result)
			{
				return false;
			}
		}

		int Result = fprintf(pFILE, "%d\n", pImage[Array2D(Width - 1, y)]);
		if (0 > Result)
		{
			return false;
		}
	}

	int Result = fclose(pFILE);
	if (0 != Result)
	{
		return false;
	}

	return true;
}

/**
 * @file  bmp.c
 * @brief 处理bmp文件，对其进行解析
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 7,2020
 */
#include <stdlib.h>
#include <string.h>

#include "include/pic_operation.h"
#include "include/picfmt_manager.h"
#include "include/debug_manager.h"

#pragma pack(push) /* 将当前pack设置压栈保存 */
#pragma pack(1)    /* 必须在结构体定义之前使用 */

/* bmp文件信息头 */
typedef struct tagBITMAPFILEHEADER {
	unsigned short bfType; 			//文件类型
	unsigned long  bfSize;			//文件大小
	unsigned short bfReserved1;		//保留位，必须为0
	unsigned short bfReserved2;		//保留位，必须为0
	unsigned long  bfOffBits;		//文件头开始到实际的图象数据之间的字节的偏移量
} BITMAPFILEHEADER;

/* 位图信息头 */
typedef struct tagBITMAPINFOHEADER { /* bmih */
	unsigned long  biSize;			//文件信息头结构体需要的自身
	unsigned long  biWidth;			//图像宽度
	unsigned long  biHeight;		//图像高度
	unsigned short biPlanes;		//位面数，总被设为1
	unsigned short biBitCount;		//bpp
	unsigned long  biCompression;	//数据压缩的类型
	unsigned long  biSizeImage;		//图象的大小
	unsigned long  biXPelsPerMeter;//水平分辨率
	unsigned long  biYPelsPerMeter;//垂直分辨率
	unsigned long  biClrUsed;		//使用的彩色表中的颜色索引，0-使用所有调色板项
	unsigned long  biClrImportant;	//对图象显示有重要影响的颜色索引的数目，0-都重要
} BITMAPINFOHEADER;

#pragma pack(pop) /* 恢复先前的pack设置 */

/**
 * @Description: 判断是否为bmp文件
 * @param pFileHead - 文件头
 * @return 1 - bmp文件, 0 - 非bmp文件
 */
static int isBMPFormat(PT_FileMap ptFileMap)
{
	unsigned char *pFileHead = ptFileMap->pFileMem;

	/* bmp文件最开始的两字节数据为0x4D42 */
	if ((pFileHead[0] != 0x42) || (pFileHead[1] != 0x4d))
		return 0;
	else
		return 1;
}

/**
 * @Description: 从源数据中按照DstBpp的值，转换到目的数据
 * @param width - bmp位图的宽度，SrcBpp - bmp位图的bpp
 *		  DstBpp - LCD上显示位图的bpp，SrcDatas - bmp位图的一行像素起始地址
 *		  DstDatas - LCD上显示位图的一行像素起始地址
 * @return 0 - 支持转换，-1 - 不支持转换 
 */
// sercBpp必须为24位，目标可以为 24位，32位或者16位
static int CoverOneLine(int width, int SrcBpp, int DstBpp, unsigned char *SrcDatas, unsigned char *DstDatas)
{
	int i;
	int pos;
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int color;
	unsigned short *DstData16Bpp;
	unsigned int   *DstData32Bpp;

	pos = 0;
	DstData16Bpp = (unsigned short *)DstDatas;
	DstData32Bpp = (unsigned int   *)DstDatas;
	
	if (SrcBpp != 24) {
		DebugPrint(APP_NOTICE"can not support %bpp\n", SrcBpp);
		return -1;
	}

	if (DstBpp == 24)
		memcpy(DstDatas, SrcDatas, width * 3);
	else {
		/* 提取有效的数据 */
		for (i = 0; i < width; i++) {
			blue   = SrcDatas[pos++];
			green  = SrcDatas[pos++];
			red    = SrcDatas[pos++];
			
			if (DstBpp == 32) {
				color = (red << 16) | (green << 8) | blue;
				*DstData32Bpp = color;
				DstData32Bpp++;
			} else if (DstBpp == 16) {
				/* RGB:565 */
				red   = red   >> 3;
				green = green >> 2;
				blue  = blue  >> 3;
				color = (red << 11) | (green << 5) | blue;
				*DstData16Bpp= color;
				DstData16Bpp++;
			}
		}
	}

	return 0;
}

/**
 * @Description: 从bmp文件中获取像素数据，并分配缓冲区存放像素信息，供外部文件调用
 * @param pFileHead - 文件头，ptPixelDatas - 位图信息，存储像素信息，其bpp用户设定
 * @return 0 - 成功，-1 - 失败 
 */
 
int GetPixelDatasFrmBMP(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas)
{
	int y;
	int width;
	int height;
	int BMPbpp;
	int LineWidthReal;
	int LineWidthAlign;
	unsigned char *pSrc;
	unsigned char *pDest;
	unsigned char *pFileHead;
	BITMAPFILEHEADER *ptBITMAPFILEHEADER;
	BITMAPINFOHEADER *ptBITMAPINFOHEADER;

	pFileHead = ptFileMap->pFileMem;
	ptBITMAPFILEHEADER = (BITMAPFILEHEADER *)pFileHead;
	ptBITMAPINFOHEADER = (BITMAPINFOHEADER *)(pFileHead + sizeof(BITMAPFILEHEADER));

	/* 获取bmp文件信息 */
	width  = ptBITMAPINFOHEADER->biWidth;
	height = ptBITMAPINFOHEADER->biHeight;
	BMPbpp = ptBITMAPINFOHEADER->biBitCount;
	if (BMPbpp != 24) {
		DebugPrint(APP_ERR"can not support %d bpp\n", BMPbpp);
		return -1;
	}

	/* 设置结构体 */
	ptPixelDatas->height      = height;
	ptPixelDatas->width       = width;
	ptPixelDatas->linebytes   = width * ptPixelDatas->bpp / 8;
	ptPixelDatas->TotalBytes = ptPixelDatas->height * ptPixelDatas->linebytes;
	/* 分配缓冲区 */
	ptPixelDatas->PixelDatas = (unsigned char *)malloc(ptPixelDatas->TotalBytes);
	if (ptPixelDatas->PixelDatas == NULL) {
		DebugPrint(APP_ERR"malloc ptPixelDatas->PixelDatas err\n");
		return -1;
	}

	/* 获取bmp像素信息：存储像素的起始是图片左下角 */
	LineWidthReal  = width * BMPbpp / 8;	//每行像素所占的字节
	LineWidthAlign = (LineWidthReal + 3) & ~0x3;		//向4取整后的每行像素所占的字节

	pSrc = pFileHead + ptBITMAPFILEHEADER->bfOffBits;	//像素信息的源地址
	pSrc = pSrc + LineWidthAlign * (height- 1);			//移动到存储图片左上角的数据地址

	pDest = ptPixelDatas->PixelDatas;	//数据最终的去向
	
	for (y = 0; y < height; y++) {
		//memcpy(pDest, pSrc, LineWidthReal);
		CoverOneLine(width, BMPbpp, ptPixelDatas->bpp, pSrc, pDest);
		pSrc  -= LineWidthAlign;	//bmp二进制数据
		pDest += ptPixelDatas->linebytes;		//LCD显示数据
	}

	return 0;
}

/**
 * @Description: 释放存储bmp文件中的像素信息的内存
 * @param ptPixelDatas - 位图信息，存储像素信息
 * @return 0 - 释放成功
 */
static int FreePixelDatasForBMP(PT_PixelDatas ptPixelDatas)
{
	free(ptPixelDatas->PixelDatas);
	return 0;
}

T_PicFileParser s_tBMPParser = {
	.name 			= "bmp",
	.isSupport 		= isBMPFormat,
	.GetPixelDatas 	= GetPixelDatasFrmBMP,
	.FreePixelDatas = FreePixelDatasForBMP,
};

int BMPParserInit(void)
{
	return RegisterPicFileParser(&s_tBMPParser);
}


/**
 * @file  bmp.c
 * @brief 处理bmp文件的坐标信息，计算实现缩放
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 7,2020
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "include/pic_operation.h"
#include "include/debug_manager.h"

/**
 * @Description: 对图片数据处理进行实现缩放，采用近邻取样插值
 * @param ptOriginPic - 缩放前图片信息, ptZoomPic - 缩放后图片信息
 * @return 0 - 成功缩放，-1 - 不支持缩放
 */
// 缩放比例通过原图片和缩放图片之间的比例相除得知，也就是说，只要设计目标比例大小即可
int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic)
{
	unsigned long x, y;
	unsigned long PixelBytes;
	unsigned long SrcY;
    unsigned long DstWidth;
    unsigned long *pSrcXTable;
	unsigned char *pDest;
	unsigned char *pSrc;

	if (ptOriginPic->bpp != ptZoomPic->bpp) {
		DebugPrint(APP_NOTICE"can not support bpp where the source %d and destination %d are different\n"
					, ptOriginPic->bpp, ptZoomPic->bpp);
		return -1;
	}
	
	DstWidth = ptZoomPic->width;
	PixelBytes = ptOriginPic->bpp / 8;

	/* 分配空间 */
	pSrcXTable = (unsigned long *)malloc(sizeof(unsigned long) * DstWidth);
	if (pSrcXTable == NULL) {
		DebugPrint(APP_ERR"pSrcXTable malloc err\n");
		return -1;
	}

	/* 获取源图片一行中像素点的x坐标 */
    for (x = 0; x < DstWidth; x++)
        pSrcXTable[x]=(x * ptOriginPic->width / ptZoomPic->width);

	/* 计算缩放后的坐标信息 */
    for (y = 0; y < ptZoomPic->height; y++) {
		SrcY = (y * ptOriginPic->height / ptZoomPic->height);		//获取源图片一列中像素点的y坐标

		pSrc  = ptOriginPic->PixelDatas + SrcY * ptOriginPic->linebytes;	//原图每行起始像素在内存的地址
		pDest = ptZoomPic->PixelDatas    + y * ptZoomPic->linebytes;	//缩放图每行起始像素在内存的地址

		/* 原图坐标：(pSrcXTable[x], SrcY)
		 * 缩放坐标：(x, y)
		 */
        for (x = 0; x < DstWidth; x++){
			memcpy(pDest + x * PixelBytes, pSrc + pSrcXTable[x] * PixelBytes, PixelBytes); 
        }
    }

	/* 释放空间 */
    free(pSrcXTable);

	return 0;
}

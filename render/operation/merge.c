/**
 * @file  merge.c
 * @brief 把zoom.c缩放后处理的像素信息存放到Framebuffer中
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 7,2020
 */
#include <stdlib.h>
#include <string.h>
	
#include "include/pic_operation.h"
#include "include/debug_manager.h"

int PicMerge(int x, int y, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic)
{
	int i;
	unsigned char *pDst;
	unsigned char *pSrc;
	
	if ((ptSmallPic->width > ptBigPic->width) ||
		(ptSmallPic->height > ptBigPic->height) ||
		(ptSmallPic->bpp != ptBigPic->bpp)) {
		DebugPrint(APP_ERR"SmallPic > BigPic || SmallPic.bpp != BigPic.bpp\n");
		return -1;
	}
	pSrc = ptSmallPic->PixelDatas;
	pDst = ptBigPic->PixelDatas + y * ptBigPic->linebytes + x * ptBigPic->bpp / 8;

	for (i = 0; i < ptSmallPic->height; i++) {
		memcpy(pDst, pSrc, ptSmallPic->linebytes);
		pSrc += ptSmallPic->linebytes;
		pDst += ptBigPic->linebytes;
	}
	return 0;
}

/* 
 * 把新图片并入老图片
 * iStartXofNewPic, iStartYofNewPic : 从新图片的(iStartXofNewPic, iStartYofNewPic)开始读出数据用于合并
 * iStartXofOldPic, iStartYofOldPic : 合并到老图片的(iStartXofOldPic, iStartYofOldPic)去
 * iWidth, iHeight : 合并区域的大小
 * ptNewPic        : 新图片
 * ptOldPic        : 老图片
 */
int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PT_PixelDatas ptNewPic, PT_PixelDatas ptOldPic)
{
	int i;
	unsigned char *pucSrc;
	unsigned char *pucDst;
    int linebytesCpy = iWidth * ptNewPic->bpp / 8;

	pucSrc = ptNewPic->PixelDatas + iStartYofNewPic * ptNewPic->linebytes + iStartXofNewPic * ptNewPic->bpp / 8;
	pucDst = ptOldPic->PixelDatas + iStartYofOldPic * ptOldPic->linebytes + iStartXofOldPic * ptOldPic->bpp / 8;
	for (i = 0; i < iHeight; i++)
	{
		memcpy(pucDst, pucSrc, linebytesCpy);
		pucSrc += ptNewPic->linebytes;
		pucDst += ptOldPic->linebytes;
	}
	return 0;
}


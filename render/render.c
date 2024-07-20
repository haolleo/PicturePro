#include <include/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <include/render.h>
#include <include/file.h>
#include <include/fonts_manager.h>
#include <include/encoding_manager.h>
#include <include/picfmt_manager.h>
#include <string.h>

void FlushVideoMemToDev(PT_VideoMem ptVideoMem)
{
	//memcpy(GetDefaultDispDev()->pDispMem, ptVideoMem->tPixelDatas.PixelDatas, ptVideoMem.tPixelDatas.iHeight * ptVideoMem.tPixelDatas.linebytes);
	if (!ptVideoMem->isDevFB)
	{
        //将数据刷新到framebuffer中
		GetDefaultDispDev()->ShowPage(ptVideoMem);
	}
}

int GetPixelDatasForIcon(char *strFileName, int DevBpp, PT_PixelDatas ptPixelDatas)
{
    T_FileMap tFileMap;
    int iError;
    int iXres, iYres, Bpp;

    /* 图标存在 /etc/digitpic/icons */
    snprintf(tFileMap.strFileName, 128, "%s/%s", ICON_PATH, strFileName);
    tFileMap.strFileName[127] = '\0';

    iError = MapFile(&tFileMap);
    if (iError)
    {
        DBG_PRINTF("MapFile %s error!\n", strFileName);
        return -1;
    }

    iError = Parser("bmp")->isSupport(&tFileMap);
    if (iError == 0)
    {
        DBG_PRINTF("can't support this file: %s\n", strFileName);
        unMapFile(&tFileMap);
        return -1;
    }

    GetDispResolution(&iXres, &iYres, &Bpp);
    ptPixelDatas->bpp = Bpp;
    iError = Parser("bmp")->GetPixelDatas(&tFileMap, ptPixelDatas);
    if (iError)
    {
        DBG_PRINTF("GetPixelDatas for %s error!\n", strFileName);
        unMapFile(&tFileMap);
        return -1;
    }

    unMapFile(&tFileMap);
    return 0;
}


void FreePixelDatasForIcon(PT_PixelDatas ptPixelDatas)
{
	Parser("bmp")->FreePixelDatas(ptPixelDatas);
}

int isPictureFileSupported(char *strFileName)
{
	T_FileMap tFileMap;
	int iError;

	strncpy(tFileMap.strFileName, strFileName, 256);
	tFileMap.strFileName[255] = '\0';
    iError = MapFile(&tFileMap);
    if (iError)
    {
        DBG_PRINTF("MapFile %s error!\n", strFileName);
        return 0;
    }

    if (GetParser(&tFileMap) == NULL)
    {
        unMapFile(&tFileMap);
        return 0;
    }

    unMapFile(&tFileMap);
    return 1;
}


int GetPixelDatasFrmFile(char *strFileName, PT_PixelDatas ptPixelDatas)
{
	T_FileMap tFileMap;
	int iError;
	int iXres, iYres, Bpp;
    PT_PicFileParser ptParser;

	strncpy(tFileMap.strFileName, strFileName, 256);
	tFileMap.strFileName[255] = '\0';

	iError = MapFile(&tFileMap);
	if (iError)
	{
		DBG_PRINTF("MapFile %s error!\n", strFileName);
		return -1;
	}
	
    ptParser = GetParser(&tFileMap);
    if (ptParser == NULL) //解析失败，释放映射内存
	{
        unMapFile(&tFileMap);
		return -1;
	}

	GetDispResolution(&iXres, &iYres, &Bpp);
	ptPixelDatas->bpp = Bpp;
	iError = ptParser->GetPixelDatas(&tFileMap, ptPixelDatas);
	if (iError)
	{
		DBG_PRINTF("GetPixelDatas for %s error!\n", strFileName);
        unMapFile(&tFileMap);
		return -1;
	}

    unMapFile(&tFileMap);
	return 0;
}

void FreePixelDatasFrmFile(PT_PixelDatas ptPixelDatas)
{
	//Parser("bmp")->FreePixelDatas(ptPixelDatas);
	free(ptPixelDatas->PixelDatas);
}


/* 返回值: 设置了VideoMem中多少字节 */
static int SetColorForPixelInVideoMem(int iX, int iY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	unsigned char *pucVideoMem;
	unsigned short *pwVideoMem16Bpp;
	unsigned int *pdwVideoMem32Bpp;
	unsigned short wColor16Bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;

	pucVideoMem      = ptVideoMem->tPixelDatas.PixelDatas;
	pucVideoMem      += iY * ptVideoMem->tPixelDatas.linebytes + iX * ptVideoMem->tPixelDatas.bpp / 8;
	pwVideoMem16Bpp  = (unsigned short *)pucVideoMem;
	pdwVideoMem32Bpp = (unsigned int *)pucVideoMem;

	//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	//DBG_PRINTF("x = %d, y = %d\n", iX, iY);
	
	switch (ptVideoMem->tPixelDatas.bpp)
	{
		case 8:
		{
			*pucVideoMem = (unsigned char)dwColor;
			return 1;
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16Bpp = (iRed << 11) | (iGreen << 5) | iBlue;
			*pwVideoMem16Bpp	= wColor16Bpp;
			return 2;
			break;
		}
		case 32:
		{
			*pdwVideoMem32Bpp = dwColor;
			return 4;
			break;
		}
		default :
		{			
			return -1;
		}
	}

	return -1;
}

void ClearRectangleInVideoMem(int TopLeftX, int TopLeftY, int BotRightX, int BotRightY, PT_VideoMem ptVideoMem, unsigned int dwColor)
{
	int x, y;
	for (y = TopLeftY; y <= BotRightY; y++)
		for (x = TopLeftX; x <= BotRightX; x++)
			SetColorForPixelInVideoMem(x, y, ptVideoMem, dwColor);
}

static int isFontInArea(int TopLeftX, int TopLeftY, int BotRightX, int BotRightY, PT_FontBitMap ptFontBitMap)
{
    if ((ptFontBitMap->XLeft >= TopLeftX) && (ptFontBitMap->XMax <= BotRightX) && \
         (ptFontBitMap->YTop >= TopLeftY) && (ptFontBitMap->YMax <= BotRightY))
         return 1;
    else
        return 0;
        
}

/* 根据位图中的数据把字符显示到videomem中 */
static int MergeOneFontToVideoMem(PT_FontBitMap ptFontBitMap, PT_VideoMem ptVideoMem)
{
	int i;
	int x, y;
	int bit;
	int iNum;
	unsigned char ucByte;

	if (ptFontBitMap->Bpp == 1)
	{
		for (y = ptFontBitMap->YTop; y < ptFontBitMap->YMax; y++)
		{
			i = (y - ptFontBitMap->YTop) * ptFontBitMap->Pitch;
			for (x = ptFontBitMap->XLeft, bit = 7; x < ptFontBitMap->XMax; x++)
			{
				if (bit == 7)
				{
					ucByte = ptFontBitMap->pBuffer[i++];
				}
				
				if (ucByte & (1<<bit))
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					/* 使用背景色 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				if (iNum == -1)
				{
					return -1;
				}
				bit--;
				if (bit == -1)
				{
					bit = 7;
				}
			}
		}
	}
	else if (ptFontBitMap->Bpp == 8)
	{
		for (y = ptFontBitMap->YTop; y < ptFontBitMap->YMax; y++)
			for (x = ptFontBitMap->XLeft; x < ptFontBitMap->XMax; x++)
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pBuffer[i++]);
				if (ptFontBitMap->pBuffer[i++])
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_FOREGROUND);
				}
				else
				{
					iNum = SetColorForPixelInVideoMem(x, y, ptVideoMem, COLOR_BACKGROUND);
				}
				
				if (iNum == -1)
				{
					return -1;
				}
			}
	}
	else
	{
		DBG_PRINTF("ShowOneFont error, can't support %d Bpp\n", ptFontBitMap->Bpp);
		return -1;
	}
	return 0;
}
	
/*
 * 在videomem的指定矩形中间显示字符串
 * 参考: 03.freetype\02th_arm\06th_show_lines_center
 */
int MergerStringToCenterOfRectangleInVideoMem(int TopLeftX, int TopLeftY, int BotRightX, int BotRightY, unsigned char *pucTextString, PT_VideoMem ptVideoMem)
{
	int iLen;
	int iError;
	unsigned char *pucBufStart;
	unsigned char *pucBufEnd;
	unsigned int dwCode;
	T_FontBitMap tFontBitMap;
	
	int bHasGetCode = 0;

	int iMinX = 32000, iMaxX = -1;
	int iMinY = 32000, iMaxY = -1;

	int iStrTopLeftX , iStrTopLeftY;

	int iWidth, iHeight;

	tFontBitMap.CurOriginX = 0;
	tFontBitMap.CurOriginY = 0;
	pucBufStart = pucTextString;
	pucBufEnd   = pucTextString + strlen((char *)pucTextString);

	/* 0. 清除这个区域 */
	ClearRectangleInVideoMem(TopLeftX, TopLeftY, BotRightX, BotRightY, ptVideoMem, COLOR_BACKGROUND);
	
	/* 1.先计算字符串显示的总体宽度、高度 */
	while (1)
	{
		/* 从字符串中逐个取出字符 */
        iLen = GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode); //跟电子书类似
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图, 位图信息里含有字符显示时的左上角、右下角坐标 */
		iError = GetFontBitmap(dwCode, &tFontBitMap);
		if (0 == iError)
		{									
			if (iMinX > tFontBitMap.XLeft)
			{
				iMinX = tFontBitMap.XLeft;
			}
			if (iMaxX < tFontBitMap.XMax)
			{
				iMaxX = tFontBitMap.XMax;
			}

			if (iMinY > tFontBitMap.YTop)
			{
				iMinY = tFontBitMap.YTop;
			}
			if (iMaxY < tFontBitMap.XMax)
			{
				iMaxY = tFontBitMap.YMax;
			}
			
			tFontBitMap.CurOriginX = tFontBitMap.NextOriginX;
			tFontBitMap.CurOriginY = tFontBitMap.NextOriginY;
		}
		else
		{
			DBG_PRINTF("GetFontBitmap for calc width/height error!\n");
		}
	}	

	iWidth  = iMaxX - iMinX;
	iHeight = iMaxY - iMinY;

    /* 如果字符串过长 */
    if (iWidth > BotRightX - TopLeftX)
    {
        iWidth = BotRightX - TopLeftX;
    }

    /* 如果字符串过高 */
	if (iHeight > BotRightY - TopLeftY)
	{
		DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		//DBG_PRINTF("iHeight = %d, BotRightY - TopLeftX = %d - %d = %d\n", iHeight, BotRightY, TopLeftY, BotRightY - TopLeftY);
		return -1;
	}
	//DBG_PRINTF("iWidth = %d, iHeight = %d\n", iWidth, iHeight);

	/* 2.确定第1个字符的原点 
	 * 2.1 先计算左上角坐标
	 */
	iStrTopLeftX = TopLeftX + (BotRightX - TopLeftX - iWidth) / 2;
	iStrTopLeftY = TopLeftY + (BotRightY - TopLeftY - iHeight) / 2;
	//DBG_PRINTF("iNewFirstFontTopLeftX = %d, iNewFirstFontTopLeftY = %d\n", iNewFirstFontTopLeftX, iNewFirstFontTopLeftY);

	/*	 
	 * 2.2 再计算第1个字符原点坐标
	 * iMinX - 原来的CurOriginX(0) = iStrTopLeftX - 新的CurOriginX
	 * iMinY - 原来的CurOriginY(0) = iStrTopLeftY - 新的CurOriginY
	 */
	tFontBitMap.CurOriginX = iStrTopLeftX - iMinX;
	tFontBitMap.CurOriginY = iStrTopLeftY - iMinY;

	//DBG_PRINTF("CurOriginX = %d, CurOriginY = %d\n", tFontBitMap.CurOriginX, tFontBitMap.CurOriginY);
	
	pucBufStart = pucTextString;	
	bHasGetCode = 0;
	while (1)
	{
		/* 从字符串中逐个取出字符 */
		iLen = GetCodeFrmBuf(pucBufStart, pucBufEnd, &dwCode);
		if (0 == iLen)
		{
			/* 字符串结束 */
			if (!bHasGetCode)
			{
				DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
			else
			{
				break;
			}
		}

		bHasGetCode = 1;
		pucBufStart += iLen;

		/* 获得字符的位图 */
		iError = GetFontBitmap(dwCode, &tFontBitMap);
		if (0 == iError)
		{
			//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			/* 显示一个字符 */
            if (isFontInArea(TopLeftX, TopLeftY, BotRightX, BotRightY, &tFontBitMap))
            {
    			if (MergeOneFontToVideoMem(&tFontBitMap, ptVideoMem))
    			{
    				DBG_PRINTF("MergeOneFontToVideoMem error for code 0x%x\n", dwCode);
    				return -1;
    			}
            }
            else
            {
                return 0;
            }
			//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
			
			tFontBitMap.CurOriginX = tFontBitMap.NextOriginX;
			tFontBitMap.CurOriginY = tFontBitMap.NextOriginY;
		}
		else
		{
			DBG_PRINTF("GetFontBitmap for drawing error!\n");
		}
	}

	return 0;
}

/* 反转图标: 就是把该区域里每个象素的颜色取反 */
static void InvertButton(PT_Layout ptLayout)
{
	int iY;
	int i;
	int iButtonWidthBytes;
	unsigned char *pucVideoMem;
	PT_DispOpr ptDispOpr = GetDefaultDispDev();

	pucVideoMem = ptDispOpr->pDispMem;
	pucVideoMem += ptLayout->TopLeftY * ptDispOpr->LineWidth + ptLayout->TopLeftX * ptDispOpr->Bpp / 8; /* 图标在Framebuffer中的地址 */
	iButtonWidthBytes = (ptLayout->BotRightX - ptLayout->TopLeftX + 1) * ptDispOpr->Bpp / 8;

	for (iY = ptLayout->TopLeftY; iY <= ptLayout->BotRightY; iY++)
	{
		for (i = 0; i < iButtonWidthBytes; i++)
		{
			pucVideoMem[i] = ~pucVideoMem[i];  /* 取反 */
		}
		pucVideoMem += ptDispOpr->LineWidth;
	}
}

/* 松开图标 */
void ReleaseButton(PT_Layout ptLayout)
{
	InvertButton(ptLayout);
}

/* 按下图标 */
void PressButton(PT_Layout ptLayout)
{
	InvertButton(ptLayout);
}



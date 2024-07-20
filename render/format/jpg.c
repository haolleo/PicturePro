#include "include/config.h"
#include "include/pic_operation.h"
#include "include/picfmt_manager.h"
#include "include/file.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>

typedef struct MyErrorMgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
}T_MyErrorMgr, *PT_MyErrorMgr;

static int isJPGFormat(PT_FileMap ptFileMap);
static int GetPixelDatasFrmJPG(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas);
static int FreePixelDatasForJPG(PT_PixelDatas ptPixelDatas);

static T_PicFileParser s_tJPGParser = {
	.name           = "jpg",
	.isSupport      = isJPGFormat,
	.GetPixelDatas  = GetPixelDatasFrmJPG,
	.FreePixelDatas = FreePixelDatasForJPG,	
};

static void MyErrorExit(j_common_ptr ptCInfo)
{
    static char errStr[JMSG_LENGTH_MAX];
    
	PT_MyErrorMgr ptMyErr = (PT_MyErrorMgr)ptCInfo->err;

    /* Create the message */
    (*ptCInfo->err->format_message) (ptCInfo, errStr);
    DBG_PRINTF("%s\n", errStr);

	longjmp(ptMyErr->setjmp_buffer, 1);
}

static int isJPGFormat(PT_FileMap ptFileMap)
{
	struct jpeg_decompress_struct tDInfo;

    /* 默认的错误处理函数是让程序退出
     * 我们参考libjpeg里的bmp.c编写自己的错误处理函数
     */
	//struct jpeg_error_mgr tJErr;   
	T_MyErrorMgr tJerr;
    int ret;

	// 分配和初始化一个decompression结构体
	// tDInfo.err = jpeg_std_error(&tJErr);
	tDInfo.err               = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit     = MyErrorExit;

	if(setjmp(tJerr.setjmp_buffer))
	{
		/* 如果程序能运行到这里, 表示JPEG解码出错 */
        jpeg_destroy_decompress(&tDInfo);
		return 0;;
	}
	
	jpeg_create_decompress(&tDInfo);

	// 用jpeg_read_header获得jpg信息
	jpeg_stdio_src(&tDInfo, ptFileMap->pFileFd);

    ret = jpeg_read_header(&tDInfo, TRUE);
	jpeg_abort_decompress(&tDInfo);

    return (ret == JPEG_HEADER_OK);
}

static int CovertOneLine(int width, int SrcBpp, int DstBpp, unsigned char *pSrcDatas, unsigned char *pDstDatas)
{
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int color;

	unsigned short *pDstDatas16bpp = (unsigned short *)pDstDatas;
	unsigned int   *pDstDatas32bpp = (unsigned int *)pDstDatas;

	int i;
	int pos = 0;

	if (SrcBpp != 24)
	{
		return -1;
	}

	if (DstBpp == 24)
	{
		memcpy(pDstDatas, pSrcDatas, width*3);
	}
	else
	{
		for (i = 0; i < width; i++)
		{
			red   = pSrcDatas[pos++];
			green = pSrcDatas[pos++];
			blue  = pSrcDatas[pos++];
			if (DstBpp == 32)
			{
				color = (red << 16) | (green << 8) | blue;
				*pDstDatas32bpp = color;
				pDstDatas32bpp++;
			}
			else if (DstBpp == 16)
			{
				/* 565 */
				red   = red >> 3;
				green = green >> 2;
				blue  = blue >> 3;
				color = (red << 11) | (green << 5) | (blue);
				*pDstDatas16bpp = color;
				pDstDatas16bpp++;
			}
		}
	}
	return 0;
}


/*
 * ptPixelDatas->bpp 是输入的参数, 它决定从JPG得到的数据要转换为该格式
 */
static int GetPixelDatasFrmJPG(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas)
{
	struct jpeg_decompress_struct tDInfo;
	//struct jpeg_error_mgr tJErr;
    int ret;
    int RowStride;
    unsigned char *LineBuffer = NULL;
    unsigned char *pDest;
	T_MyErrorMgr tJerr;

    fseek(ptFileMap->pFileFd, 0, SEEK_SET);

	// 分配和初始化一个decompression结构体
	//tDInfo.err = jpeg_std_error(&tJErr);

	tDInfo.err               = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit     = MyErrorExit;

	if(setjmp(tJerr.setjmp_buffer))
	{
		/* 如果程序能运行到这里, 表示JPEG解码出错 */
        jpeg_destroy_decompress(&tDInfo);
        if (LineBuffer)
        {
            free(LineBuffer);
        }
        if (ptPixelDatas->PixelDatas)
        {
            free(ptPixelDatas->PixelDatas);
        }
		return -1;
	}

	jpeg_create_decompress(&tDInfo);

	// 用jpeg_read_header获得jpg信息
	jpeg_stdio_src(&tDInfo, ptFileMap->pFileFd);

    ret = jpeg_read_header(&tDInfo, TRUE);

	// 设置解压参数,比如放大、缩小
    tDInfo.scale_num = tDInfo.scale_denom = 1;
    
	// 启动解压：jpeg_start_decompress	
	jpeg_start_decompress(&tDInfo);
    
	// 一行的数据长度
	RowStride = tDInfo.output_width * tDInfo.output_components;
	LineBuffer = malloc(RowStride);

    if (NULL == LineBuffer)
    {
        return -1;
    }

	ptPixelDatas->width  = tDInfo.output_width;
	ptPixelDatas->height = tDInfo.output_height;
	//ptPixelDatas->bpp    = bpp;
	ptPixelDatas->linebytes    = ptPixelDatas->width * ptPixelDatas->bpp / 8;
    ptPixelDatas->TotalBytes   = ptPixelDatas->height * ptPixelDatas->linebytes;
	ptPixelDatas->PixelDatas = (unsigned char *)malloc(ptPixelDatas->TotalBytes);
	if (NULL == ptPixelDatas->PixelDatas)
	{
		return -1;
	}

    pDest = ptPixelDatas->PixelDatas;

	// 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
	while (tDInfo.output_scanline < tDInfo.output_height) 
	{
        /* 得到一行数据,里面的颜色格式为0xRR, 0xGG, 0xBB */
		(void) jpeg_read_scanlines(&tDInfo, &LineBuffer, 1);

		// 转到ptPixelDatas去
		CovertOneLine(ptPixelDatas->width, 24, ptPixelDatas->bpp, LineBuffer, pDest);
		pDest += ptPixelDatas->linebytes;
	}
	
	free(LineBuffer);
	jpeg_finish_decompress(&tDInfo);
	jpeg_destroy_decompress(&tDInfo);

    return 0;
}

static int FreePixelDatasForJPG(PT_PixelDatas ptPixelDatas)
{
	free(ptPixelDatas->PixelDatas);
	return 0;
}

int JPGParserInit(void)
{
	return RegisterPicFileParser(&s_tJPGParser);
}



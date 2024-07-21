#include "include/config.h"
#include "include/pic_operation.h"
#include "include/picfmt_manager.h"
#include "include/file.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <png.h>

static int isPNGFormat(PT_FileMap ptFileMap);
static int GetPixelDatasFrmPNG(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas);
static int FreePixelDatasForPNG(PT_PixelDatas ptPixelDatas);

static T_PicFileParser s_tPNGParser = {
    .name           = "png",
    .isSupport      = isPNGFormat,
    .GetPixelDatas  = GetPixelDatasFrmPNG,
    .FreePixelDatas = FreePixelDatasForPNG,
};

static int isPNGFormat(PT_FileMap ptFileMap)
{
    png_structp png_ptr;
    png_infop info_ptr;
    int ret = 0;
    /* 创建png结构和info结构 */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        goto cleanup; //表示出错，执行错误处理函数
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        goto cleanup;
    /* 设置错误处理 */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* 错误发生，进行清理 */
        goto cleanup;
    }
    /* 设置输入控制 */
    png_init_io(png_ptr, ptFileMap->tFp);
    /* 读取所有PNG控制信息 */
    png_read_info(png_ptr, info_ptr);
    /* 成功读取PNG信息，设置返回值 */
    ret = 1;
cleanup:
    /* 清理 */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return ret;
}

static int CovertOneLine(int width, png_byte SrcBpp, int DstBpp, unsigned char *pSrcDatas, unsigned char *pDstDatas)
{
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int color;

    unsigned short *pDstDatas16bpp = (unsigned short *)pDstDatas;
    unsigned int   *pDstDatas32bpp = (unsigned int *)pDstDatas;

    int i;
    int pos = 0;
    //其实PNG_COLOR_TYPE_RGB_ALPHA就是RBG[ALPHA]32位 PNG_COLOR_TYPE_RGB 24位
    if (SrcBpp != PNG_COLOR_TYPE_RGB_ALPHA ||SrcBpp != PNG_COLOR_TYPE_RGB )
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
            if(SrcBpp == PNG_COLOR_TYPE_RGB_ALPHA) //32位，最后一位是alpha，不使用
                pos++;
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
 * ptPixelDatas->bpp 是输入的参数, 它决定从PNG得到的数据要转换为该格式
 */
static int GetPixelDatasFrmPNG(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas)
{

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    }
    png_init_io(png_ptr, ptFileMap->tFp);

    png_set_sig_bytes(png_ptr, 0);
    png_read_info(png_ptr, info_ptr);
    //解析出来的属性信息
    int bit_depth;
    png_byte color_type;
    int width,height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    // Allocate memory for the image data   png_bytep其实就是unsigned char类型
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++)
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr,info_ptr));

    // Read the pixels
    png_read_update_info(png_ptr, info_ptr);
    png_read_image(png_ptr, row_pointers);

    ptPixelDatas->iWidth  = width;
    ptPixelDatas->iHeight = height;
    //ptPixelDatas->bpp    = bpp;
    ptPixelDatas->iLineBytes    = ptPixelDatas->iWidth * ptPixelDatas->iBpp / 8;
    ptPixelDatas->iTotalBytes   = ptPixelDatas->iHeight * ptPixelDatas->iLineBytes;
    ptPixelDatas->aucPixelDatas = (unsigned char *)malloc(ptPixelDatas->iTotalBytes);
    if (NULL == ptPixelDatas->aucPixelDatas)
    {
        return -1;
    }

    unsigned char* pDest = ptPixelDatas->aucPixelDatas;

    for(int i=0;i<height;i++)
    {
        // 转到ptPixelDatas去 row_pointers是一个二维指针，pDest是一个一维指针
        CovertOneLine(ptPixelDatas->iWidth, color_type, ptPixelDatas->iBpp, row_pointers[i], pDest);
        pDest += ptPixelDatas->iLineBytes;
    }

    // Cleanup
    for (int y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return 0;
}

static int FreePixelDatasForPNG(PT_PixelDatas ptPixelDatas)
{
    free(ptPixelDatas->aucPixelDatas);
    return 0;
}

int PNGParserInit(void)
{
    return RegisterPicFileParser(&s_tPNGParser);
}



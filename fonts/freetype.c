    
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "include/fonts_manager.h"
#include "include/config.h"

static FT_Library	s_tLibrary;
static FT_Face		s_tFace;
static FT_GlyphSlot	s_tSlot;

static int FreeTypeFontInit(char *pFontFile, unsigned int FontSize)
{
	int error;
	
	/* 初始化库 */
	error = FT_Init_FreeType(&s_tLibrary);
	if (error) {
		DBG_PRINTF("FT_Init_FreeType error, err code:%d\n", error);
		return -1;
	}

	/* 装载字体文件 */
	error = FT_New_Face(s_tLibrary, pFontFile, 0, &s_tFace);
	if (error) {
		DBG_PRINTF("FT_New_Face error, err code:%d\n", error);
		return -1;
	}

	/* 设置像素大小：FontSize*FontSize */
	error = FT_Set_Pixel_Sizes(s_tFace, FontSize, 0);
	if (error) {
		DBG_PRINTF("FT_Set_Pixel_Sizes error, err code:%d\n", error);
		return -1;
	}
	
	s_tSlot = s_tFace->glyph;
	return 0;
}

static int FreetyPeGetFontBitMap(unsigned int Code, PT_FontBitMap ptFontBitMap)
{
	int error;
	int PenX;
	int PenY;

	PenX = ptFontBitMap->CurOriginX;
	PenY = ptFontBitMap->CurOriginY;
	
	/* 根据给定的文字信息，加载文字，转换成单色位图 */
	error = FT_Load_Char(s_tFace, Code, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
	if (error) {
		printf("FT_Load_Char error, err code:%d\n", error);
		return -1;
	}

	/* 设置ptFontBitMap的参数 */
	ptFontBitMap->XLeft 		= PenX + s_tSlot->bitmap_left;
	ptFontBitMap->YTop  		= PenY - s_tSlot->bitmap.rows;
	ptFontBitMap->XMax   		= ptFontBitMap->XLeft + s_tSlot->bitmap.width;
	ptFontBitMap->YMax   		= ptFontBitMap->YTop   + s_tSlot->bitmap.rows;
	ptFontBitMap->Bpp    		= 1;
	ptFontBitMap->Pitch  		= s_tSlot->bitmap.pitch;
	ptFontBitMap->NextOriginX	= PenX + s_tSlot->advance.x / 64;
	ptFontBitMap->NextOriginY	= PenY + s_tSlot->advance.y;
	ptFontBitMap->pBuffer		= s_tSlot->bitmap.buffer;
	
	return 0;
}

static void FreeTypeSetFontSize(unsigned int FontSize)
{
	FT_Set_Pixel_Sizes(s_tFace, FontSize, 0);
}

/* 分配、设置、注册结构体 */
static T_FontOpr s_tFreeTypeFontOpr = {
	.name 			= "freetype",
	.FontInit		= FreeTypeFontInit,
	.GetFontBitmap  = FreetyPeGetFontBitMap,
	.SetFontSize    = FreeTypeSetFontSize,
}; 

int FreeTypeInit(void)
{
	return RegisterFontOpr(&s_tFreeTypeFontOpr);
}



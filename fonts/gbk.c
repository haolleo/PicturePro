#include <include/config.h>
#include <include/fonts_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int GBKFontInit(char *pFontFile, unsigned int FontSize);
static int GBKGetFontBitmap(unsigned int Code, PT_FontBitMap ptFontBitMap);

static T_FontOpr s_tGBKFontOpr = {
	.name          = "gbk",
	.FontInit      = GBKFontInit,
	.GetFontBitmap = GBKGetFontBitmap,
};

static int s_FdHZK;
static unsigned char *s_pHZKMem;
static unsigned char *s_pHZKMemEnd;

static int GBKFontInit(char *pFontFile, unsigned int FontSize)
{
	struct stat tStat;

	if (16 != FontSize)
	{
		DBG_PRINTF("GBK can't support %d fontsize\n", FontSize);
		return -1;
	}
	
	s_FdHZK = open(pFontFile, O_RDONLY);
	if (s_FdHZK < 0)
	{
		DBG_PRINTF("can't open %s\n", pFontFile);
		return -1;
	}
	if(fstat(s_FdHZK, &tStat))
	{
		DBG_PRINTF("can't get fstat\n");
		return -1;
	}
	s_pHZKMem = (unsigned char *)mmap(NULL , tStat.st_size, PROT_READ, MAP_SHARED, s_FdHZK, 0);
	if (s_pHZKMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for hzk16\n");
		return -1;
	}
	s_pHZKMemEnd = s_pHZKMem + tStat.st_size;
	return 0;
}

static int GBKGetFontBitmap(unsigned int Code, PT_FontBitMap ptFontBitMap)
{
	int Area;
	int Where;

	int PenX = ptFontBitMap->CurOriginX;
	int PenY = ptFontBitMap->CurOriginY;

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	if (Code & 0xffff0000)
	{
		DBG_PRINTF("don't support this code : 0x%x\n", Code);
		return -1;
	}	

	Area  = (int)(Code & 0xff) - 0xA1;
	Where = (int)((Code >> 8) & 0xff) - 0xA1;

	if ((Area < 0) || (Where < 0))
	{
		DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	ptFontBitMap->XLeft    = PenX;
	ptFontBitMap->YTop     = PenY - 16;
	ptFontBitMap->XMax     = PenX + 16;
	ptFontBitMap->YMax     = PenY;
	ptFontBitMap->Bpp      = 1;
	ptFontBitMap->Pitch    = 2;
	ptFontBitMap->pBuffer = s_pHZKMem + (Area * 94 + Where)*32;;	

	if (ptFontBitMap->pBuffer >= s_pHZKMemEnd)
	{
		return -1;
	}

	ptFontBitMap->NextOriginX = PenX + 16;
	ptFontBitMap->NextOriginY = PenY;
	
	//DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

int GBKInit(void)
{
	return RegisterFontOpr(&s_tGBKFontOpr);
}


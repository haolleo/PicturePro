#include <include/config.h>
#include <include/fonts_manager.h>
#include <string.h>

static PT_FontOpr s_ptFontOprHead = NULL;
static int s_FontSize;


int RegisterFontOpr(PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTmp;

	if (!s_ptFontOprHead)
	{
		s_ptFontOprHead   = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = s_ptFontOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext     = ptFontOpr;
		ptFontOpr->ptNext = NULL;
	}

	return 0;
}


void ShowFontOpr(void)
{
	int i = 0;
	PT_FontOpr ptTmp = s_ptFontOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_FontOpr GetFontOpr(char *pName)
{
	PT_FontOpr ptTmp = s_ptFontOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

void SetFontSize(unsigned int FontSize)
{
	PT_FontOpr ptTmp = s_ptFontOprHead;
	
	s_FontSize = FontSize;

	while (ptTmp)
	{
		if (ptTmp->SetFontSize)
		{
			ptTmp->SetFontSize(FontSize);
		}
		ptTmp = ptTmp->ptNext;
	}
}

int GetFontBitmap(unsigned int Code, PT_FontBitMap ptFontBitMap)
{
	int error = 0;
	PT_FontOpr ptTmp = s_ptFontOprHead;

	while (ptTmp)
	{
		error = ptTmp->GetFontBitmap(Code, ptFontBitMap);
		if (0 == error)
		{
			return 0;
		}
		ptTmp = ptTmp->ptNext;
	}
	return -1;
}

int SetFontsDetail(char *pFontsName, char *pFontsFile, unsigned int FontSize)
{
	int error = 0;
	PT_FontOpr ptFontOpr;

	ptFontOpr = GetFontOpr(pFontsName);
	if (NULL == ptFontOpr)
	{
		return -1;
	}

	s_FontSize = FontSize;

	error = ptFontOpr->FontInit(pFontsFile, FontSize);
	
	return error;
}

int FontsInit(void)
{
	int error;
	
	error = ASCIIInit();
	if (error)
	{
		DBG_PRINTF("ASCIIInit error!\n");
		return -1;
	}

	error = GBKInit();
	if (error)
	{
		DBG_PRINTF("GBKInit error!\n");
		return -1;
	}
	
	error = FreeTypeInit();
	if (error)
	{
		DBG_PRINTF("FreeTypeInit error!\n");
		return -1;
	}

	return 0;
}


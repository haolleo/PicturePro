#include <include/config.h>
#include <include/encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr s_ptEncodingOprHead;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr)
{
	PT_EncodingOpr ptTmp;

	if (!s_ptEncodingOprHead)
	{
		s_ptEncodingOprHead   = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = s_ptEncodingOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext = ptEncodingOpr;
		ptEncodingOpr->ptNext = NULL;
	}

	return 0;
}



void ShowEncodingOpr(void)
{
	int i = 0;
	PT_EncodingOpr ptTmp = s_ptEncodingOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pFileBufHead)
{
	PT_EncodingOpr ptTmp = s_ptEncodingOprHead;

	while (ptTmp)
	{	
		if (ptTmp->isSupport(pFileBufHead))
			return ptTmp;
		else
			ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptFontOprCpy;
	
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptFontOprCpy = malloc(sizeof(T_FontOpr));
		if (!ptFontOprCpy)
		{
			return -1;
		}
		else
		{
			memcpy(ptFontOprCpy, ptFontOpr, sizeof(T_FontOpr));
			ptFontOprCpy->ptNext = ptEncodingOpr->ptFontOprSupportedHead;
			ptEncodingOpr->ptFontOprSupportedHead = ptFontOprCpy;
			return 0;
		}		
	}
}

int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTmp;
	PT_FontOpr ptPre;
		
	if (!ptEncodingOpr || !ptFontOpr)
	{
		return -1;
	}
	else
	{
		ptTmp = ptEncodingOpr->ptFontOprSupportedHead;
		if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
		{
			/* 删除头节点 */
			ptEncodingOpr->ptFontOprSupportedHead = ptTmp->ptNext;
			free(ptTmp);
			return 0;
		}

		ptPre = ptEncodingOpr->ptFontOprSupportedHead;
		ptTmp = ptPre->ptNext;
		while (ptTmp)
		{
			if (strcmp(ptTmp->name, ptFontOpr->name) == 0)
			{
				/* 从链表里取出、释放 */
				ptPre->ptNext = ptTmp->ptNext;
				free(ptTmp);
				return 0;
			}
			else
			{
				ptPre = ptTmp;
				ptTmp = ptTmp->ptNext;
			}
		}

		return -1;
	}
}

PT_EncodingOpr Encode(char *pName)
{
	PT_EncodingOpr ptTmp = s_ptEncodingOprHead;

	while (ptTmp) {
		if (strcmp(ptTmp->name, pName) == 0)
			return ptTmp;

		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}

/* 先写一个简单的版本, 只能处理ASCII字符串 --实际上utf-8可以满足了 */
int GetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode)
{
	PT_EncodingOpr pTmp;
	
    pTmp = Encode("utf-8");
	if (pTmp == NULL) {
		DebugPrint(APP_WARNING"Can not find utf-8 Encode file!\n");
		return -1;
	}

	return pTmp->GetCodeFrmBuf(pBufStart, pBufEnd, pCode);
}

int EncodingInit(void)
{
	int error;

	error = AsciiEncodingInit();
	if (error)
	{
		DBG_PRINTF("AsciiEncodingInit error!\n");
		return -1;
	}

	error = Utf16leEncodingInit();
	if (error)
	{
		DBG_PRINTF("Utf16leEncodingInit error!\n");
		return -1;
	}
	
	error = Utf16beEncodingInit();
	if (error)
	{
		DBG_PRINTF("Utf16beEncodingInit error!\n");
		return -1;
	}
	
	error = Utf8EncodingInit();
	if (error)
	{
		DBG_PRINTF("Utf8EncodingInit error!\n");
		return -1;
	}

	return 0;
}


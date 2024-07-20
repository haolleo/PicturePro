#include <include/config.h>
#include <include/encoding_manager.h>
#include <string.h>

static int isUtf16beCoding(unsigned char *pBufHead);
static int Utf16beGetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode);

static T_EncodingOpr g_tUtf16beEncodingOpr = {
	.name          = "utf-16be",
	.HeadLen	   = 2,
	.isSupport     = isUtf16beCoding,
	.GetCodeFrmBuf = Utf16beGetCodeFrmBuf,
};

static int isUtf16beCoding(unsigned char *pBufHead)
{
	const char StrUtf16be[] = {0xFE, 0xFF, 0};
	
	if (strncmp((const char*)pBufHead, StrUtf16be, 2) == 0)
	{
		/* UTF-16 big endian */
		return 1;
	}
	else
	{
		return 0;
	}
}

static int Utf16beGetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode)
{
	if (pBufStart + 1 < pBufEnd)
	{
		*pCode = (((unsigned int)pBufStart[0])<<8) + pBufStart[1];
		return 2;
	}
	else
	{
		/* 文件结束 */
		return 0;
	}
}

int  Utf16beEncodingInit(void)
{
	AddFontOprForEncoding(&g_tUtf16beEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUtf16beEncodingOpr, GetFontOpr("ascii"));
	
	return RegisterEncodingOpr(&g_tUtf16beEncodingOpr);
}




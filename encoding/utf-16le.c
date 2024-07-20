#include <include/config.h>
#include <include/encoding_manager.h>
#include <string.h>

static int isUtf16leCoding(unsigned char *pBufHead);
static int Utf16leGetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode);

static T_EncodingOpr s_tUtf16leEncodingOpr = {
	.name          = "utf-16le",
	.HeadLen	   = 2,
	.isSupport     = isUtf16leCoding,
	.GetCodeFrmBuf = Utf16leGetCodeFrmBuf,
};

static int isUtf16leCoding(unsigned char *pBufHead)
{
	const char StrUtf16le[] = {0xFF, 0xFE, 0};
	if (strncmp((const char *)pBufHead, StrUtf16le, 2) == 0)
	{
		/* UTF-16 little endian */
		return 1;
	}
	else
	{
		return 0;
	}
}

static int Utf16leGetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode)
{
	if (pBufStart + 1 < pBufEnd)
	{
		*pCode = (((unsigned int)pBufStart[1])<<8) + pBufStart[0];
		return 2;
	}
	else
	{
		/* 文件结束 */
		return 0;
	}
}

int  Utf16leEncodingInit(void)
{
	AddFontOprForEncoding(&s_tUtf16leEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&s_tUtf16leEncodingOpr, GetFontOpr("ascii"));
	
	return RegisterEncodingOpr(&s_tUtf16leEncodingOpr);
}



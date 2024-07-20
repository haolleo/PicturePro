
#include <include/config.h>
#include <include/encoding_manager.h>
#include <string.h>

static int isAsciiCoding(unsigned char *pBufHead);
static int AsciiGetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode);

static T_EncodingOpr s_tAsciiEncodingOpr = {
	.name          = "ascii",
	.HeadLen      = 0,
	.isSupport     = isAsciiCoding,
	.GetCodeFrmBuf = AsciiGetCodeFrmBuf,
};

/* 判断是否支持该编码格式 */
static int isAsciiCoding(unsigned char *pBufHead)
{
	const char StrUtf8[]    = {0xEF, 0xBB, 0xBF, 0};
	const char StrUtf16le[] = {0xFF, 0xFE, 0};
	const char StrUtf16be[] = {0xFE, 0xFF, 0};
	
	if (strncmp((const char*)pBufHead, StrUtf8, 3) == 0) 
	{
		/* UTF-8 */
		return 0;
	}
	else if (strncmp((const char*)pBufHead, StrUtf16le, 2) == 0)
	{
		/* UTF-16 little endian */
		return 0;
	}
	else if (strncmp((const char*)pBufHead, StrUtf16be, 2) == 0)
	{
		/* UTF-16 big endian */
		return 0;
	}
	else
	{
		return 1;
	}
}

/* 从源文件中获取编码数据 */
static int AsciiGetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode)
{
	unsigned char *pBuf = pBufStart;
	unsigned char c = *pBuf;
	
	if ((pBuf < pBufEnd) && (c < (unsigned char)0x80))
	{
		/* 返回ASCII码 */
		*pCode = (unsigned int)c;
		return 1;
	}

	if (((pBuf + 1) < pBufEnd) && (c >= (unsigned char)0x80))
	{
		/* 返回GBK码 */
		*pCode = pBuf[0] + (((unsigned int)pBuf[1])<<8);
		return 2;
	}

	if (pBuf < pBufEnd)
	{
		/* 可能文件有损坏, 但是还是返回一个码, 即使它是错误的 */
		*pCode = (unsigned int)c;
		return 1;
	}
	else
	{
		/* 文件处理完毕 */
		return 0;
	}
}

int AsciiEncodingInit(void)
{
	/* 添加支持该编码文件的点阵数据的结构体 */
	AddFontOprForEncoding(&s_tAsciiEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&s_tAsciiEncodingOpr, GetFontOpr("ascii"));
	AddFontOprForEncoding(&s_tAsciiEncodingOpr, GetFontOpr("gbk"));

	/* 注册该编码格式        */
	return RegisterEncodingOpr(&s_tAsciiEncodingOpr);
}


#include <include/config.h>
#include <include/encoding_manager.h>
#include <string.h>

static int isUtf8Coding(unsigned char *pBufHead);
static int Utf8GetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode);

static T_EncodingOpr s_tUtf8EncodingOpr = {
	.name          = "utf-8",
	.HeadLen	   = 3,
	.isSupport     = isUtf8Coding,
	.GetCodeFrmBuf = Utf8GetCodeFrmBuf,
};

static int isUtf8Coding(unsigned char *pBufHead)
{
	const char StrUtf8[]    = {0xEF, 0xBB, 0xBF, 0};
	if (strncmp((const char*)pBufHead, StrUtf8, 3) == 0)
		return 1;	//UTF-8
	else
		return 0;
}

/* 获得前导为1的位的个数
 * 比如二进制数 11001111 的前导1有2位: 用2个字节表示编码
 *              11100001 的前导1有3位: 用3个字节表示编码
 */
static int GetPreOneBits(unsigned char Val)
{
	int i;
	int j = 0;
	
	for (i = 7; i >= 0; i--) {
		if (!(Val & (1 << i)))
			break;
		else
			j++;
	}
	
	return j;
}

static int Utf8GetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode)
{
#if 0
    对于UTF-8编码中的任意字节B，如果B的第一位为0，则B为ASCII码，并且B独立的表示一个字符;
    如果B的第一位为1，第二位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的一个字节，并且不为字符的第一个字节编码;
    如果B的前两位为1，第三位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由两个字节表示;
    如果B的前三位为1，第四位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由三个字节表示;
    如果B的前四位为1，第五位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由四个字节表示;

    因此，对UTF-8编码中的任意字节，根据第一位，可判断是否为ASCII字符;
    根据前二位，可判断该字节是否为一个字符编码的第一个字节; 
    根据前四位（如果前两位均为1），可确定该字节为字符编码的第一个字节，并且可判断对应的字符由几个字节表示;
    根据前五位（如果前四位为1），可判断编码是否有错误或数据传输过程中是否有错误。
#endif

	int i;	
	int Num;
	unsigned char Val;
	unsigned int Sum = 0;

	/* 判断异常文件结束 */
	if (pBufStart >= pBufEnd)
		return 0;


	Val = pBufStart[0];
	Num  = GetPreOneBits(pBufStart[0]);	//得到前导码，可知用多少字节表示一个字符

	/* 判断文件结束 */
	if ((pBufStart + Num) > pBufEnd)
		return 0;

	/* 前导码为0，为ascii编码 */
	if (Num == 0)
	{
		*pCode = pBufStart[0];
		return 1;
	}
	else
	{
		/* 获取除前导码的数据 */
		Val = Val << Num;
		Val = Val >> Num;	
		Sum += Val;

		/* 根据前导码获取后面字节的信息，从UTF-8编码信息中获取该字符的Unicode值 */
		for (i = 1; i < Num; i++)
		{
			Val = pBufStart[i] & 0x3f;
			Sum = Sum << 6;
			Sum += Val;			
		}
		*pCode = Sum;
		return Num;
	}
}

int  Utf8EncodingInit(void)
{
	/* 添加支持该编码文件的点阵数据的结构体 */
	AddFontOprForEncoding(&s_tUtf8EncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&s_tUtf8EncodingOpr, GetFontOpr("ascii"));

	/* 注册该编码格式        */
	return RegisterEncodingOpr(&s_tUtf8EncodingOpr);
}


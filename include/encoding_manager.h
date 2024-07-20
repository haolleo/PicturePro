
#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

#include <include/fonts_manager.h>
#include <include/disp_manager.h>

typedef struct EncodingOpr {
    char *name;		//编码格式的名字
    int HeadLen;	//编码数据的头部长度，区分是何种编码格式：0—ascii, 2—UTF16, 3—UTF8
    PT_FontOpr ptFontOprSupportedHead;	//添加支持该编码格式的点阵数据的结构体

    int (*isSupport)(unsigned char *pBufHead);	//判断是否支持名为name的编码格式
    int (*GetCodeFrmBuf)(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode);	//获取编码数据

    struct EncodingOpr *ptNext;
}T_EncodingOpr, *PT_EncodingOpr;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr);
void ShowEncodingOpr(void);
PT_DispOpr GetDispOpr(char *pName);
PT_EncodingOpr Encode(char *pName);
PT_EncodingOpr SelectEncodingOprForFile(unsigned char *pFileBufHead);
int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr);
int DelFontOprFrmEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr);
int EncodingInit(void);
int AsciiEncodingInit(void);
int  Utf16beEncodingInit(void);
int  Utf16leEncodingInit(void);
int  Utf8EncodingInit(void);
int GetCodeFrmBuf(unsigned char *pBufStart, unsigned char *pBufEnd, unsigned int *pCode);


#endif /* _ENCODING_MANAGER_H */

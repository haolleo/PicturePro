
#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

typedef struct FontBitMap {
    int XLeft;
    int YTop;
    int XMax;
    int YMax;
    int Bpp;
    int Pitch;   /* 对于单色位图, 两行象素之间的跨度 */
    int CurOriginX;
    int CurOriginY;
    int NextOriginX;
    int NextOriginY;
    unsigned char *pBuffer;
}T_FontBitMap, *PT_FontBitMap;

typedef struct FontOpr {
    char *name;
    int (*FontInit)(char *pFontFile, unsigned int FontSize);
    int (*GetFontBitmap)(unsigned int Code, PT_FontBitMap ptFontBitMap);
    void (*SetFontSize)(unsigned int FontSize);
    struct FontOpr *ptNext;
}T_FontOpr, *PT_FontOpr;


int RegisterFontOpr(PT_FontOpr ptFontOpr);
void ShowFontOpr(void);
int FontsInit(void);
int ASCIIInit(void);
int GBKInit(void);
int FreeTypeInit(void);
PT_FontOpr GetFontOpr(char *pName);
void SetFontSize(unsigned int FontSize);
int GetFontBitmap(unsigned int Code, PT_FontBitMap ptFontBitMap);
int SetFontsDetail(char *pFontsName, char *pFontsFile, unsigned int FontSize);



#endif /* _FONTS_MANAGER_H */

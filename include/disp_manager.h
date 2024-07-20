#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include "pic_operation.h"

/* 图标布局信息 */
typedef struct Layout {
    char *strIconName;			//图标的名字
    int TopLeftX;				//图标左上角x坐标
    int TopLeftY;				//图标左上角y坐标
    int BotRightX;				//图标右下角x坐标
    int BotRightY;				//图标右下角x坐标
}T_Layout, *PT_Layout;

/* 内存页面数据是否被使用 */
typedef enum {
    VMS_FREE = 0,			//空闲
    VMS_USED_FOR_PREPARE,	//子线程占用
    VMS_USED_FOR_CURMAIN,	//当前主线程占用
}E_VideoMemState;

/* 内存页面数据是否已准备好 */
typedef enum {
    PS_BLANK = 0,			//数据空白
    PS_GENERATING,			//数据生成中
    PS_GENERATED,			//数据完毕
}E_PicState;

/* 描述页面内存块 */  
//整个LCD界面的数据，id 页面ID isDevFB 该页面是否FB使用 eState:使用状态，空闲主线程 子线程 epIC是否有数据 tpixelData 整个页面数据
typedef struct VideoMem {
    int id;
    int isDevFB;
    E_VideoMemState eVideoMemState;
    E_PicState ePicState;
    T_PixelDatas tPixelDatas;
    struct VideoMem *ptNext;
}T_VideoMem, *PT_VideoMem;

//fb.c进行初始化
typedef struct DispOpr {
    char *name;
    unsigned char *pDispMem; //映射freembuffer中
    int Xres;
    int Yres;
    int Bpp;
    int LineWidth;
    int (*DeviceInit)(void);
    int (*ShowPixel)(int PenX, int PenY, unsigned int Color);
    int (*CleanScreen)(unsigned int BackColor);
    int (*ShowPage)(PT_VideoMem ptVideoMem);
    struct DispOpr *ptNext;
}T_DispOpr, *PT_DispOpr;

int RegisterDispOpr(PT_DispOpr ptDispOpr);
void ShowDispOpr(void);
int DisplayInit(void);
int FBInit(void);
int GetDispResolution(int *pXres, int *pYres, int *pBpp);
void SelectAndInitDefaultDispDev(char *name);
int AllocVideoMem(int num);
PT_VideoMem GetVideoMem(int id, int isCurMain);
void PutVideoMem(PT_VideoMem ptVideoMem);
PT_DispOpr GetDefaultDispDev(void);
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int color);
PT_VideoMem GetDevVideoMem(void);
void SetFontSize(unsigned int FontSize);
void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor);



#endif /* _DISP_MANAGER_H */

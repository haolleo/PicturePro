

#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include "input_manager.h"
#include "disp_manager.h"

typedef struct PageParams {
    int iPageID;                  /* 页面的ID */
    char strCurPictureFile[256];  /* 要处理的第1个图片文件 */
}T_PageParams, *PT_PageParams;


/* 页面图标信息 */
typedef struct PageLayout {
    int TopLeftX;		  /* 这个区域的左上角、右下角坐标 */
    int TopLeftY;
    int BotRightX;
    int BotRightY;
    int bpp;					//显示页面的bpp
    int MaxTotalBytes;			//最大图标的大小
    PT_Layout ptLayout;			//指向描述该页面图标数组的指针
}T_PageLayout, *PT_PageLayout;

/* 描述页面行为 */
typedef struct PageAction {
    char *name;
    int (*Run)(PT_PageParams ptParentPageParams);
    int (*GetInputEvent)(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent);
    int (*Prepare)(void);
    struct PageAction *ptNext;
}T_PageAction, *PT_PageAction;

typedef struct PageCfg {
    int IntervalSecond;
    char strSeletedDir[256];
}T_PageCfg, *PT_PageCfg;

int PagesInit(void);
PT_PageAction Page(char *pName);
void ShowPages(void);
int RegisterPageAction(PT_PageAction ptPageAction);
int MainPageInit(void);
int SettingPageInit(void);
int IntervalPageInit(void);
int BrowsePageInit(void);
int AutoPageInit(void);
int ManualPageInit(void);
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem);
int GenericGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent);
int TimeMSBetween(struct timeval tTimeStart, struct timeval tTimeEnd);
void GetIntervalTime(int *piIntervalSecond);
void GetSelectedDir(char *strSeletedDir);
int ID(char *strName);
void GetPageCfg(PT_PageCfg ptPageCfg);


#endif /* _PAGE_MANAGER_H */

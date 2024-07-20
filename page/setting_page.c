
/**
 * @file  main_page.c
 * @brief 设置页面显示文件
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 10,2020
 */
#include <stdlib.h> 
#include <string.h>

#include "include/config.h"
#include "include/page_manager.h"
#include "include/input_manager.h"
#include "include/render.h"
#include "include/debug_manager.h"
#include "include/input_manager.h"
#include "include/disp_manager.h"

/**
 * @Description: 显示main_page页面信息
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static T_Layout s_tSettingPageIconsLayout[] = {
	{"select_fold.bmp",  0, 0, 0, 0},
	{"interval.bmp", 	 0, 0, 0, 0},
	{"return.bmp",       0, 0, 0, 0},
	{NULL, 				 0, 0, 0, 0},	//结尾标志
};

static T_PageLayout s_tSettingPageLayout = {
	.MaxTotalBytes  = 0,
	.ptLayout       = s_tSettingPageIconsLayout,
};


/* 计算各图标坐标值 */
static void  CalcSettingPageLayout(PT_PageLayout ptPageLayout)
{
	int startY;
	int width;
	int height;
	int xres, yres, bpp;
	int TmpTotalBytes;
	PT_Layout ptLayout;

	ptLayout = ptPageLayout->ptLayout;
	GetDispResolution(&xres, &yres, &bpp);
	ptPageLayout->bpp = bpp;

	/*   
	 *    ----------------------
	 *                           1/2 * height
	 *          select_fold.bmp  height
	 *                           1/2 * height
	 *          interval.bmp     height
	 *                           1/2 * height
	 *          return.bmp       height
	 *                           1/2 * height
	 *    ----------------------
	 */
	 
	height = yres * 2 / 10;
	width  = height;
	startY = height / 2;
	
	/* select_fold图标 */
	ptLayout[0].TopLeftY  = startY;
	ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height - 1;
	ptLayout[0].TopLeftX  = (xres - width * 2) / 2;
	ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width * 2 - 1;

	TmpTotalBytes = (ptLayout[0].BotRightX - ptLayout[0].TopLeftX + 1) * (ptLayout[0].BotRightY - ptLayout[0].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
	{
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	}


	/* interval图标 */
	ptLayout[1].TopLeftY  = ptLayout[0].BotRightY + height / 2 + 1;
	ptLayout[1].BotRightY = ptLayout[1].TopLeftY + height - 1;
	ptLayout[1].TopLeftX  = (xres - width * 2) / 2;
	ptLayout[1].BotRightX = ptLayout[1].TopLeftX + width * 2 - 1;

	TmpTotalBytes = (ptLayout[1].BotRightX - ptLayout[1].TopLeftX + 1) * (ptLayout[1].BotRightY - ptLayout[1].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
	{
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	}

	/* return图标 */
	ptLayout[2].TopLeftY  = ptLayout[1].BotRightY + height / 2 + 1;
	ptLayout[2].BotRightY = ptLayout[2].TopLeftY + height - 1;
	ptLayout[2].TopLeftX  = (xres - width) / 2;
	ptLayout[2].BotRightX = ptLayout[2].TopLeftX + width - 1;

	TmpTotalBytes = (ptLayout[2].BotRightX - ptLayout[2].TopLeftX + 1) * (ptLayout[2].BotRightY - ptLayout[2].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
	{
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	}

}

static void ShowSettingPage(PT_PageLayout ptPageLayout)
{

	int error;
	PT_VideoMem ptVideoMem;
	PT_Layout ptLayout;
	ptLayout = ptPageLayout->ptLayout;

	/* 获得显存 */
	ptVideoMem = GetVideoMem(ID("Setting"), 1);
	if (ptVideoMem == NULL) {
		DebugPrint(APP_ERR"Can not get video mem for Setting_page!\n");
		return ;
	}
	
	/* 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if (ptLayout[0].TopLeftX == 0)
		CalcSettingPageLayout(ptPageLayout);

	error = GeneratePage(ptPageLayout, ptVideoMem);	
	
	/* 刷新/加载到设备 */
	FlushVideoMemToDev(ptVideoMem);

	/* 设置页面内存为空闲状态 */
	PutVideoMem(ptVideoMem);
}

static int SettingPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}

/**
 * @Description: Setting_page主页面
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static int SettingPageRun(PT_PageParams ptParentPageParams)
{
	int index;
	int pressured;
	int indexPressured;
	T_InputEvent tInputEvent;    
	T_PageParams tPageParams;

	tPageParams.iPageID = ID("setting");
	
	/* 显示页面 */
	ShowSettingPage(&s_tSettingPageLayout);

	/* 创建Prepare线程 */

	index = -1;
	indexPressured = -1;
	pressured = 0;
	/* 调用GetInputEvent(), 获得输入事件，进而处理 */
	while (1) {
		index = SettingPageGetInputEvent(&s_tSettingPageLayout, &tInputEvent);
		
		/* 松开和按下不在同一个图标范围内 */
		if (tInputEvent.pressure == 0) {
			
			/* 曾经有按键按下 */
			if (pressured) {
				ReleaseButton(&s_tSettingPageIconsLayout[indexPressured]);
				pressured = 0;

				/* 松开与按下的按键为同一个 */
				if (indexPressured == index) {

					switch (indexPressured) {
						
					case 0: /* "选择目录"按钮 */
						
						Page("browse")->Run(&tPageParams);
						ShowSettingPage(&s_tSettingPageLayout);
						break;

					case 1: /* interval按钮 */
						
						Page("interval")->Run(&tPageParams);
						ShowSettingPage(&s_tSettingPageLayout);
						break;
						
					case 2:		/* 返回按键 */
						
						return 0;

					default:
						break;
					}
				}
				
				indexPressured = -1;
			}
		} else {
			/* 松开和按下都在同一个图标范围内 */
			/* 按下 */
			if (index != -1) {

				/* 之前未按下 */
				if (!pressured) {
					pressured = 1;
					indexPressured = index;
					PressButton(&s_tSettingPageIconsLayout[indexPressured]);
				}
			}
		}
	}

	return 0;
}

static int SettingPagePrepare()
{
	return 0;
}

static T_PageAction s_tSettingPageAction = {
	.name 			= "setting",
	.Run 			= SettingPageRun,
	.GetInputEvent = SettingPageGetInputEvent,
	.Prepare		= SettingPagePrepare,
};

/**
 * @Description: Setting页面初始化函数，注册该结构体
 * @return 0
 */
int SettingPageInit(void)
{
	return RegisterPageAction(&s_tSettingPageAction);
}


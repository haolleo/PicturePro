
/**
 * @file  main_page.c
 * @brief 主页面显示文件
 * @version 2.0 （版本声明）
 * @author Dk
 * @date  July 11,2020
 */
#include <stdlib.h>
 
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
static T_Layout s_tMainPageIconLayout[] = {
	{"browse_mode.bmp",  0, 0, 0, 0},
	{"continue_mod.bmp", 0, 0, 0, 0},
	{"setting.bmp",      0, 0, 0, 0},
	{NULL, 				 0, 0, 0, 0},	//结尾标志
};

static T_PageLayout s_tMainPageLayout = {
	.MaxTotalBytes = 0,
	.ptLayout       = s_tMainPageIconLayout,
};

/* 计算各图标坐标值 */
// ？？？设计有问题，不可能用 [0][1][2]这样的设计
static void  CalcMainPageLayout(PT_PageLayout ptPageLayout)
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
	 *          browse_mode.bmp  height
	 *                           1/2 * height
	 *         continue_mod.bmp     height
	 *                           1/2 * height
	 *          setting.bmp       height
	 *                           1/2 * height
	 *    ----------------------
	 */
	 
	height = yres * 2 / 10;
    width  = 2*height; //宽是2倍高
	startY = height / 2;
	
	/* select_fold图标 */
	ptLayout[0].TopLeftY  = startY;
	ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height - 1;
    ptLayout[0].TopLeftX  = (xres - width ) / 2;
    ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width  - 1;

	TmpTotalBytes = (ptLayout[0].BotRightX - ptLayout[0].TopLeftY + 1) * (ptLayout[0].BotRightY - ptLayout[0].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
	{
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	}


	/* interval图标 */
	ptLayout[1].TopLeftY  = ptLayout[0].BotRightY + height / 2 + 1;
	ptLayout[1].BotRightY = ptLayout[1].TopLeftY + height - 1;
    ptLayout[1].TopLeftX  = (xres - width) / 2;
    ptLayout[1].BotRightX = ptLayout[1].TopLeftX + width - 1;

	TmpTotalBytes = (ptLayout[1].BotRightX - ptLayout[1].TopLeftX + 1) * (ptLayout[1].BotRightY - ptLayout[1].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
	{
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	}

	/* return图标 */
	ptLayout[2].TopLeftY  = ptLayout[1].BotRightY + height / 2 + 1;
	ptLayout[2].BotRightY = ptLayout[2].TopLeftY + height - 1;
    ptLayout[2].TopLeftX  = (xres - width ) / 2;
    ptLayout[2].BotRightX = ptLayout[2].TopLeftX + width - 1;

	TmpTotalBytes = (ptLayout[2].BotRightX - ptLayout[2].TopLeftX + 1) * (ptLayout[2].BotRightY - ptLayout[2].TopLeftY + 1) * bpp / 8;
	//最大图片的大小在这里赋值
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
	{
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	}

}

static int MainPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}

static void ShowMainPage(PT_PageLayout ptPageLayout)
{
	int error;
	PT_VideoMem ptVideoMem;

	PT_Layout ptLayout;

	/* 获得显存 */
	ptVideoMem = GetVideoMem(ID("main"), 1);
	if (ptVideoMem == NULL) {
		DebugPrint(APP_ERR"Can not get video mem for main_page!\n");
		return ;
	}

	ptLayout = ptPageLayout->ptLayout;
	
	/* 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if (ptLayout[0].TopLeftX == 0)
		CalcMainPageLayout(ptPageLayout);

    //将各个图标的值放入准备的显存空间
	error = GeneratePage(ptPageLayout, ptVideoMem);
	
	/* 刷新/加载到设备 */
	FlushVideoMemToDev(ptVideoMem);

	/* 设置页面内存为空闲状态 */
	PutVideoMem(ptVideoMem);
}

/**
 * @Description: main_page主页面
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static int MainPageRun(PT_PageParams ptParentPageParams)
{
	int index;
	int pressured;
	int indexPressured;
	T_InputEvent tInputEvent;	
    T_PageParams tPageParams;

	
    tPageParams.iPageID = ID("main");
	
	/* 显示页面 */
	ShowMainPage(&s_tMainPageLayout);

	index = -1;
	indexPressured = -1;
	pressured = 0;
	/* 调用GetInputEvent(), 获得输入事件，进而处理 */
	while (1) {
		index = MainPageGetInputEvent(&s_tMainPageLayout, &tInputEvent);
		
		/* 松开和按下不在同一个图标范围内 */
        // 压力pressure为0意味着此时是松开状态
		if (tInputEvent.pressure == 0) {
			
			/* 曾经有按键按下 */
			if (pressured) {
				ReleaseButton(&s_tMainPageIconLayout[indexPressured]);
				pressured = 0;

                /* 松开与按下的按键为同一个 */
                //按下不会有反应，只有松下的时候，进行检测之前是否按下，如果按下，且位置一致，那么认为该松下是按下相对应的
				if (indexPressured == index) {
					switch (indexPressured) {
					case 0: /* 浏览按钮 */
						tPageParams.strCurPictureFile[0] = '\0';
						Page("manual")->Run(&tPageParams);
                        //manual退出后来到这里
						/* 从设置页面返回后显示当首的主页面 */
						ShowMainPage(&s_tMainPageLayout);

						break;
						
					case 1:		/* auto_page */
						Page("auto")->Run(&tPageParams);

						/* 返回主页面 */
						ShowMainPage(&s_tMainPageLayout);
						break;
						
					case 2:		/* setting_page */
						Page("setting")->Run(&tPageParams);

						/* 返回主页面 */
						ShowMainPage(&s_tMainPageLayout);
						break;

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
					PressButton(&s_tMainPageIconLayout[indexPressured]);
				}
			}
		}
	}

	return 0;
}


static T_PageAction s_tMainPageAction = {
	.name 			= "main",
	.Run 			= MainPageRun,
	.GetInputEvent = MainPageGetInputEvent,
};

/**
 * @Description: main页面初始化函数，注册该结构体
 * @return 0
 */
int MainPageInit(void)
{
	return RegisterPageAction(&s_tMainPageAction);
}

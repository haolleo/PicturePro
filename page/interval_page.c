
/**
 * @file  interval_page.c
 * @brief 设置时间间隔页面显示文件
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
#include "include/fonts_manager.h"

static T_Layout s_tIntervalNumberLayout;
static int s_IntervalSecond = 7;

/**
 * @Description: 显示main_page页面信息
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static T_Layout s_tIntervalPageIconLayout[] = {
    {"zoomin.bmp",    0, 0, 0, 0}, //没图片？？
	{"time.bmp",   0, 0, 0, 0},
    {"zooout.bmp",    0, 0, 0, 0}, //没图片？？
	{"ok.bmp",     0, 0, 0, 0},
	{"cancel.bmp", 0, 0, 0, 0},
	{NULL, 		   0, 0, 0, 0},	//结尾标志
};

static T_PageLayout s_tIntervalPageLayout = {
	.MaxTotalBytes = 0,
	.ptLayout       = s_tIntervalPageIconLayout,
};

void GetIntervalTime(int *piIntervalSecond)
{
    *piIntervalSecond = s_IntervalSecond;
}

static void  CalcIntervalPageLayout(PT_PageLayout ptPageLayout)
{
	int startY;
	int width;
	int height;
	int xres, yres, bpp;
	int TmpTotalBytes;
	PT_Layout ptLayout;

	ptLayout = ptPageLayout->ptLayout;

	/* 获取工作中的LCD设备分辨率与bpp */
	GetDispResolution(&xres, &yres, &bpp);
	ptPageLayout->bpp = bpp;

	/*   
	 *    ----------------------
	 *                          1/2 * height
	 *          inc.bmp         height * 28 / 128     
	 *         time.bmp         height * 72 / 128
	 *          dec.bmp         height * 28 / 128     
	 *                          1/2 * height
	 *    ok.bmp     cancel.bmp 1/2 * height
	 *                          1/2 * height
	 *    ----------------------
	 */
	height = yres / 3;
	width  = height;
	startY = height / 2;

	/* inc图标 */
	ptLayout[0].TopLeftY  = startY;
	ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height * 28 / 128 - 1;
	ptLayout[0].TopLeftX  = (xres - width * 52 / 128) / 2;
	ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width * 52 / 128 - 1;

	/* inc图标大小 */
	TmpTotalBytes = (ptLayout[0].BotRightX - ptLayout[0].TopLeftX + 1) * (ptLayout[0].BotRightY - ptLayout[0].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;

	/* time图标 */
	ptLayout[1].TopLeftY  = ptLayout[0].BotRightY + 1;
	ptLayout[1].BotRightY = ptLayout[1].TopLeftY + height * 72 / 128 - 1;
	ptLayout[1].TopLeftX  = (xres - width) / 2;
	ptLayout[1].BotRightX = ptLayout[1].TopLeftX + width - 1;

	/* time图标大小 */
	TmpTotalBytes = (ptLayout[1].BotRightX - ptLayout[1].TopLeftX + 1) * (ptLayout[1].BotRightY - ptLayout[1].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;

	/* dec图标 */
	ptLayout[2].TopLeftY  = ptLayout[1].BotRightY + 1;
	ptLayout[2].BotRightY = ptLayout[2].TopLeftY + height * 28 / 128 - 1;
	ptLayout[2].TopLeftX  = (xres - width * 52 / 128) / 2;
	ptLayout[2].BotRightX = ptLayout[2].TopLeftX + width * 52 / 128 - 1;

	/* dec图标大小 */
	TmpTotalBytes = (ptLayout[2].BotRightX - ptLayout[2].TopLeftX + 1) * (ptLayout[2].BotRightY - ptLayout[2].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;

	/* ok图标 */
	ptLayout[3].TopLeftY  = ptLayout[2].BotRightY + height / 2 + 1;
	ptLayout[3].BotRightY = ptLayout[3].TopLeftY + height / 2 - 1;
	ptLayout[3].TopLeftX  = (xres - width) / 3;
	ptLayout[3].BotRightX = ptLayout[3].TopLeftX + width / 2 - 1;

	/* ok图标大小 */
	TmpTotalBytes = (ptLayout[3].BotRightX - ptLayout[3].TopLeftX + 1) * (ptLayout[3].BotRightY - ptLayout[3].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;
	

	/* ok图标 */
	ptLayout[4].TopLeftY  = ptLayout[3].TopLeftY;
	ptLayout[4].BotRightY = ptLayout[3].BotRightY;
	ptLayout[4].TopLeftX  = ptLayout[3].TopLeftX * 2 + width/2;
	ptLayout[4].BotRightX = ptLayout[4].TopLeftX + width/2 - 1;
	TmpTotalBytes = (ptLayout[4].BotRightX - ptLayout[4].TopLeftX + 1) * (ptLayout[4].BotRightY - ptLayout[4].TopLeftY + 1) * bpp / 8;
	if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
		ptPageLayout->MaxTotalBytes = TmpTotalBytes;

	/* time.bmp原图大小为128x72, 里面的两个数字大小为52x40
	 * 经过CalcIntervalPageLayout后有所缩放
	 */
	width  = ptLayout[1].BotRightX - ptLayout[1].TopLeftX + 1;
	height = ptLayout[1].BotRightY - ptLayout[1].TopLeftY + 1;

	s_tIntervalNumberLayout.TopLeftX  = ptLayout[1].TopLeftX + (128 - 52) / 2 * width / 128;
	s_tIntervalNumberLayout.BotRightX = ptLayout[1].BotRightX - (128 - 52) / 2 * width / 128 + 1;

	s_tIntervalNumberLayout.TopLeftY  = ptLayout[1].TopLeftY + (72 - 40) / 2 * height / 72;
	s_tIntervalNumberLayout.BotRightY = ptLayout[1].BotRightY - (72 - 40) / 2 * height / 72 + 1;

#if 0
	DBG_PRINTF("frame:  width = %d, height = %d\n", width, height);
	DBG_PRINTF("number: width = %d, height = %d\n", s_tIntervalNumberLayout.BotRightX - s_tIntervalNumberLayout.TopLeftX, s_tIntervalNumberLayout.BotRightY - s_tIntervalNumberLayout.TopLeftY);
#endif
			
}

/* 绘制图标中的数字 */
//可以用qt的组件代替，问题是qt组件的加载
static int GenerateIntervalPageSpecialIcon(int number, PT_VideoMem ptVideoMem)
{
	unsigned int FontSize;
	char strNumber[3];
	int error;
	
	FontSize = s_tIntervalNumberLayout.BotRightY - s_tIntervalNumberLayout.TopLeftY;
	SetFontSize(FontSize);

	/* 显示两位数字: 00~59 */
	if (number > 59)
		return -1;

	snprintf(strNumber, 3, "%02d", number);
	//DBG_PRINTF("strNumber = %s, len = %d\n", strNumber, strlen(strNumber));

	/* 把数字显示到当前页面内存的对应图标的中心位置 */
	error = MergerStringToCenterOfRectangleInVideoMem(s_tIntervalNumberLayout.TopLeftX, 
					s_tIntervalNumberLayout.TopLeftY, s_tIntervalNumberLayout.BotRightX, 
					s_tIntervalNumberLayout.BotRightY, (unsigned char *)strNumber, ptVideoMem);

	return error;
}

static int IntervalPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}

static void ShowIntervalPage(PT_PageLayout ptPageLayout)
{
	int error;
	PT_VideoMem  ptVideoMem;
	PT_Layout    ptLayout;

	ptLayout = ptPageLayout->ptLayout;
	
	/* 获得显存 */
	ptVideoMem = GetVideoMem(ID("Interval"), 1);
	if (ptVideoMem == NULL) {
		DebugPrint(APP_ERR"Can not get video mem for Interval_page!\n");
		return ;
	}
	
	/* 描画数据 */
	/* 如果还没有计算过各图标的坐标 */
	if (ptLayout[0].TopLeftX == 0)
		CalcIntervalPageLayout(ptPageLayout);

	/* 绘制图标中的数字 */
	error = GenerateIntervalPageSpecialIcon(s_IntervalSecond, ptVideoMem);
	if (error)
		DBG_PRINTF(APP_ERR"GenerateIntervalPageSpecialIcon error!\n");

	/* 页面数据的描绘 */
	error = GeneratePage(ptPageLayout, ptVideoMem);	
	
	/* 刷新/加载到设备 */
	FlushVideoMemToDev(ptVideoMem);

	/* 设置页面内存为空闲状态 */
	PutVideoMem(ptVideoMem);
}


/**
 * @Description: Interval_page主页面
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static int IntervalPageRun(PT_PageParams ptParentPageParams)
{
	int index;
	int pressured;
	int indexPressured;
	int intervalSecond;
	int fast = 0;  /* 表示快速加减 */
	T_InputEvent tInputEvent;
	T_InputEvent tInputEventPrePress;
	PT_VideoMem ptDevVideoMem;

	intervalSecond = s_IntervalSecond;
	ptDevVideoMem = GetDevVideoMem();

	tInputEventPrePress.time.tv_sec = 0;
	tInputEventPrePress.time.tv_usec = 0;
	
	/* 显示页面 */
	ShowIntervalPage(&s_tIntervalPageLayout);

	/* 创建Prepare线程 */

	index = -1;
	indexPressured = -1;
	pressured = 0;
	/* 调用GetInputEvent(), 获得输入事件，进而处理 */
	while (1) {
		index = IntervalPageGetInputEvent(&s_tIntervalPageLayout, &tInputEvent);
		
		/* 松开和按下不在同一个图标范围内 */
		if (tInputEvent.pressure == 0) {
			
			/* 曾经有按键按下 */
			if (pressured) {
				ReleaseButton(&s_tIntervalPageIconLayout[indexPressured]);
				pressured = 0;

				/* 松开与按下的按键为同一个 */
				if (indexPressured == index) {
					switch (indexPressured) {
					case 0: /* inc按钮 */
						intervalSecond++;
					
						if (intervalSecond == 60)
							intervalSecond = 0;
						
						/* 在指定区域显示数字 */
						GenerateIntervalPageSpecialIcon(intervalSecond, ptDevVideoMem);
						break;

					case 2: /* dec按钮 */
						intervalSecond--;
											
						if (intervalSecond == -1)
							intervalSecond = 59;

						/* 在指定区域显示数字 */
						GenerateIntervalPageSpecialIcon(intervalSecond, ptDevVideoMem);
						break;

					case 3: /* ok按钮 */

						s_IntervalSecond = intervalSecond;
						
						return 0;
						break;

					case 4: /* cancel按钮 */
						
						return 0;
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
				if (!pressured && index != -1) {
					pressured = 1;
					indexPressured = index;
					tInputEventPrePress = tInputEvent;  /* 记录下来 */
					PressButton(&s_tIntervalPageIconLayout[indexPressured]);
				}

				/* 如果按下的是"inc.bmp"或"dec.bmp" 
				 * 连按2秒后, 飞快的递增或减小: 每50ms变化一次
				 */
				if ((indexPressured == 0) || (indexPressured == 2)) {
					if (fast && (TimeMSBetween(tInputEventPrePress.time, tInputEvent.time) > 50))
					{
						intervalSecond = indexPressured ? (intervalSecond - 1) : (intervalSecond + 1);
						if (intervalSecond == 60)
							intervalSecond = 0;
						else if (intervalSecond == -1)
							intervalSecond = 59;

						/* 在指定区域显示数字 */
						GenerateIntervalPageSpecialIcon(intervalSecond, ptDevVideoMem);
						tInputEventPrePress = tInputEvent;
					}
					
					if (TimeMSBetween(tInputEventPrePress.time, tInputEvent.time) > 2000) {
						fast = 1;
						tInputEventPrePress = tInputEvent;
					}
					
				}
			}
		}
	}

	return 0;
}

static int IntervalPagePrepare()
{
	return 0;
}

static T_PageAction s_tIntervalPageAction = {
	.name 			= "interval",
	.Run 			= IntervalPageRun,
	.GetInputEvent = IntervalPageGetInputEvent,
	.Prepare		= IntervalPagePrepare,
};

/**
 * @Description: Interval页面初始化函数，注册该结构体
 * @return 0
 */
int IntervalPageInit(void)
{
	return RegisterPageAction(&s_tIntervalPageAction);
}



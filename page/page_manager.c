
#include <string.h>
#include <stdlib.h>

#include "include/config.h"
#include "include/render.h"
#include "include/page_manager.h"
#include "include/disp_manager.h"

static PT_PageAction s_ptPageActionHead;

/**
 * @Description: 注册函数，把支持的page结构体放入链表，供下层调用
 *				 构建一个链表：把多个拓展文件的结构体“串”起来
 * @param ptPageAction - 所要注册的结构体
 * @return 0
 */
int RegisterPageAction(PT_PageAction ptPageAction)
{
	PT_PageAction ptTmp;

	if (!s_ptPageActionHead) {
		s_ptPageActionHead    = ptPageAction;
		ptPageAction->ptNext = NULL;
	}
	else {
		ptTmp = s_ptPageActionHead;
		while (ptTmp->ptNext)
			ptTmp = ptTmp->ptNext;

		ptTmp->ptNext	      = ptPageAction;
		ptPageAction->ptNext = NULL;
	}

	return 0;
}

/**
 * @Description: 显示所支持的页面，供外部函数调用
 * @return 0
 */
void ShowPages(void)
{
	int i = 0;
	PT_PageAction ptTmp = s_ptPageActionHead;

	while (ptTmp) {
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/**
 * @Description: 获取指定的名字的拓展文件，供外部函数调用，找寻对应结构体
 * @param pName - 寻找结构体的名字
 * @return 0
 */
PT_PageAction Page(char *pName)
{
	PT_PageAction ptTmp = s_ptPageActionHead;
	
	while (ptTmp) {
		if (strcmp(ptTmp->name, pName) == 0)
			return ptTmp;

		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}

//更新page  从pagelayout中取出所有图片，将原始大小换算为显示大小，然后加载入界面 pagelayout是页面布局，即四个按键的显示
int GeneratePage(PT_PageLayout ptPageLayout, PT_VideoMem ptVideoMem)
{
	int error;
	PT_Layout	 ptLayout;
	T_PixelDatas tOriginIconPixelDatas;
	T_PixelDatas tZoomIconPixelDatas;

	ptLayout = ptPageLayout->ptLayout;
	
	if (ptVideoMem->ePicState != PS_GENERATED) {
		/* 清除屏幕 */
		ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

		/* 设置LCD显示的图片的信息 */
		tZoomIconPixelDatas.bpp = ptPageLayout->bpp;

		/* 分配存储LCD显示的图片数据的内存 */
		tZoomIconPixelDatas.PixelDatas = (unsigned char *)malloc(ptPageLayout->MaxTotalBytes);
		if (tZoomIconPixelDatas.PixelDatas == NULL) {
			DebugPrint(APP_ERR"Malloc err! File:%s Line:d\n", __FILE__, __LINE__);
			return -1;
		}

		/* 描绘每个图片的坐标信息 */
		while (ptLayout->strIconName) {

			/* 获取图片像素信息 */
			error = GetPixelDatasForIcon(ptLayout->strIconName, ptPageLayout->bpp, &tOriginIconPixelDatas);
			if (error == -1) {
				DebugPrint(APP_ERR"GetPixelDatasForIcon err! File:%s Line:%d\n", __FILE__, __LINE__);
				free(tZoomIconPixelDatas.PixelDatas);	//释放内存
				return error;
			}
			
			tZoomIconPixelDatas.height     = ptLayout->BotRightY - ptLayout->TopLeftY + 1;
			tZoomIconPixelDatas.width      = ptLayout->BotRightX - ptLayout->TopLeftX+ 1;
			tZoomIconPixelDatas.linebytes  = tZoomIconPixelDatas.width * tZoomIconPixelDatas.bpp / 8;
			tZoomIconPixelDatas.TotalBytes = tZoomIconPixelDatas.linebytes * tZoomIconPixelDatas.height;
			
			/* 对图片的缩放参数进行设置 */
			PicZoom(&tOriginIconPixelDatas, &tZoomIconPixelDatas);

			/* 把缩放后的图片信息整合到ptVideoMem.tPixelDatas中 */
            // x,y为缩放图片所放位置
			PicMerge(ptLayout->TopLeftX, ptLayout->TopLeftY, &tZoomIconPixelDatas, &ptVideoMem->tPixelDatas);

			FreePixelDatasForIcon(&tOriginIconPixelDatas);
			ptLayout++;
		}

		
		free(tZoomIconPixelDatas.PixelDatas);	//释放内存
		ptVideoMem->ePicState = PS_GENERATED;	//更新状态
	}

	return 0;
}

/* 两个时间点的差值:单位ms */
int TimeMSBetween(struct timeval tTimeStart, struct timeval tTimeEnd)
{
	int ms;
	ms = (tTimeEnd.tv_sec - tTimeStart.tv_sec) * 1000 + (tTimeEnd.tv_usec - tTimeStart.tv_usec) / 1000;
	return ms;
}

// 获取
int GenericGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	T_InputEvent tInputEvent;
	int ret;
	int i = 0;
	PT_Layout ptLayout = ptPageLayout->ptLayout;
	
	/* 获得原始的触摸屏数据 
     * 它是调用input_manager.c的函数，此函数会让当前线否休眠
     * 当触摸屏线程获得数据后，会把它唤醒
	 */
    ret = GetInputEvent(&tInputEvent);
	if (ret)
	{
		return -1;
	}

	if (tInputEvent.type != INPUT_TYPE_TOUCHSCREEN)
	{
		return -1;
	}

	*ptInputEvent = tInputEvent;
	
	/* 处理数据 */
	/* 确定触点位于哪一个按钮上 */
	while (ptLayout[i].strIconName)
	{
		if ((tInputEvent.x >= ptLayout[i].TopLeftX) && (tInputEvent.x <= ptLayout[i].BotRightX) && \
			 (tInputEvent.y >= ptLayout[i].TopLeftY) && (tInputEvent.y <= ptLayout[i].BotRightY))
			/* 找到了被点中的按钮 */
			return i;
		else
			i++;	
	}

	/* 触点没有落在按钮上 */
	return -1;
}

void GetPageCfg(PT_PageCfg ptPageCfg)
{
    GetSelectedDir(ptPageCfg->strSeletedDir);
    GetIntervalTime(&ptPageCfg->IntervalSecond);
}

/**
 * @Description: 页面初始化函数，供外部函数调用，向下注册结构体
 * @return 0
 */
int PagesInit(void)
{
	int error;
	
	error = MainPageInit();
	error |= SettingPageInit();
	error |= IntervalPageInit();
	error |= BrowsePageInit();
	error |= AutoPageInit();
	error |= ManualPageInit();

	return error;
}

int ID(char *strName)  //使得根据char*传入后转换的ID不相同
{
	return (int)(strName[0]) + (int)(strName[1]) + (int)(strName[2]) + (int)(strName[3]);
}


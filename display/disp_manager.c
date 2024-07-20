
#include <stdlib.h>

#include "include/config.h"
#include "include/disp_manager.h"
#include "string.h"

static PT_DispOpr s_ptDispOprHead;
static PT_DispOpr s_ptDefaultDispOpr;

static PT_VideoMem s_ptVieoMemHead;		//页面内存链表头

/* 函数名：		 注册函数
 * 函数功能：构建一个链表：把多个拓展文件的结构体“串”起来
 * 函数实现：根据传入的结点，首先判断该链表头是否为空
 *				空则，头结点指向传入的节点，且把节点的ptNext域指向NULL
 *				不空则，尾插法插入链表
 */
int RegisterDispOpr(PT_DispOpr ptDispOpr)
{
	PT_DispOpr ptTmp;

	if (!s_ptDispOprHead)
	{
		s_ptDispOprHead   = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = s_ptDispOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptDispOpr;
		ptDispOpr->ptNext = NULL;
	}

	return 0;
}

/* 显示支持拓展文件的名字 */
void ShowDispOpr(void)
{
	int i = 0;
	PT_DispOpr ptTmp = s_ptDispOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* 获取指定的名字的拓展文件 */
PT_DispOpr GetDispOpr(char *pName)
{
	PT_DispOpr ptTmp = s_ptDispOprHead;
	
	while (ptTmp) {
		if (strcmp(ptTmp->name, pName) == 0)
			return ptTmp;
			
		ptTmp = ptTmp->ptNext;
	}
	
	return NULL;
}

//选择一个显示设备，并初始化，然后清屏  --与Default相关都是该设备
void SelectAndInitDefaultDispDev(char *name)
{
	s_ptDefaultDispOpr = GetDispOpr(name);
	if (s_ptDefaultDispOpr == NULL) {
		DebugPrint(APP_ERR"s_ptDefaultDispOpr is NULL! File:%s Line:%d\n", __FILE__, __LINE__);
		return ;
	}

	s_ptDefaultDispOpr->DeviceInit();
	s_ptDefaultDispOpr->CleanScreen(0);
}

//找到device显存，因为插入显存是头插法，因此device的在最后面
PT_VideoMem GetDevVideoMem(void)
{
	PT_VideoMem ptTmp = s_ptVieoMemHead;
	
	while (ptTmp)
	{
		if (ptTmp->isDevFB)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

PT_DispOpr GetDefaultDispDev(void)
{
	return s_ptDefaultDispOpr;
}

/**
 * @Description: 获得正在使用的显示设备的分辨率与bpp
 * @param pXres - x分辨率, pYres - y分辨率
 * @return 0
 */
int GetDispResolution(int *pXres, int *pYres, int *pbpp)
{
	if (s_ptDefaultDispOpr) {
		*pXres = s_ptDefaultDispOpr->Xres;
		*pYres = s_ptDefaultDispOpr->Yres;
		*pbpp  = s_ptDefaultDispOpr->Bpp;
		return 0;
	} else
		return -1;
}

/**
 * @Description: 为每个页面分配内存，后续用来刷新/加载到显存中
 * @param num - 需分配内存块的数量，当num为0，只描述设备本身Framebuffer信息不进行内存申请
 * @return 0 - 成功，-1 - 失败
 */
int AllocVideoMem(int num)
{
	int i;
	int bpp;
	int linebytes;
	int xres, yres;
	int VMSize;
	PT_VideoMem ptNew;
	PT_VideoMem ptTmp;

	bpp = yres = xres = 0;
	
	GetDispResolution(&xres, &yres, &bpp);
	VMSize    = xres * yres * bpp / 8;
	linebytes = xres * bpp / 8;
	
	/* 1、先把设备本身的Framebuffer放入链表 */
	ptNew = (PT_VideoMem)malloc(sizeof(T_VideoMem));
	if (ptNew == NULL) {
		DebugPrint(APP_ERR"Framebuffer set fail!\n");
		goto fail;
	}

	/* 设置该内存所描述页面的信息 */
	ptNew->id 					  = 0;
	ptNew->isDevFB 				  = 1;  //当前页面是直接放入framebuffer,也就是lcd直接显示  isDeviceFramebuffer,是否为framebuffer
	ptNew->eVideoMemState         = VMS_FREE;
	ptNew->ePicState              = PS_BLANK;
	ptNew->tPixelDatas.bpp        = bpp;
	ptNew->tPixelDatas.height     = yres;
	ptNew->tPixelDatas.width      = xres;
	ptNew->tPixelDatas.linebytes  = linebytes;		
	ptNew->tPixelDatas.PixelDatas = s_ptDefaultDispOpr->pDispMem;
	ptNew->tPixelDatas.TotalBytes = VMSize;

	/* 强制设置设备本身的Framebuffer状态为被占用 */
	if (num != 0)
		ptNew->eVideoMemState = VMS_USED_FOR_CURMAIN;

	/* 头插法插入链表 */
	ptNew->ptNext = s_ptVieoMemHead;
	s_ptVieoMemHead = ptNew;

	/* 2、后分配用于页面管理的内存并设置结构体，组成：页面描述 + 显存 */
	for (i = 0; i < num; i++) {
		ptNew = (PT_VideoMem)malloc(sizeof(T_VideoMem) + VMSize);
		if (ptNew == NULL) {
			DebugPrint(APP_ERR"ptNew malloc fail，already malloc num: %d!\n", i);
			goto fail;
		}

		/* 设置该内存所描述页面的信息 */ //
		ptNew->id 					  = 0; 
		ptNew->isDevFB 				  = 0; //后续都是放在内存中，如果num=0,也就是所有的都在framebuffer操作，因为有些设备内存很小。
		ptNew->eVideoMemState         = VMS_FREE;
		ptNew->ePicState              = PS_BLANK;
		ptNew->tPixelDatas.bpp        = bpp;
		ptNew->tPixelDatas.height     = yres;
		ptNew->tPixelDatas.width      = xres;
		ptNew->tPixelDatas.linebytes  = linebytes;		
		ptNew->tPixelDatas.PixelDatas = (unsigned char *)(ptNew + 1); //结构体指针+1，指加上该结构体的大小  
		ptNew->tPixelDatas.TotalBytes = VMSize;

		/* 头插法插入链表 */
		ptNew->ptNext = s_ptVieoMemHead;
		s_ptVieoMemHead = ptNew;
	}

	return 0;
	
fail:
	while (s_ptVieoMemHead) {
		ptTmp = s_ptVieoMemHead->ptNext;
		free(s_ptVieoMemHead);
		s_ptVieoMemHead = ptTmp;
	}

	return -1;
}

/**
 * @Description: 根据id获得对应的页面内存，后续用来刷新/加载到显存中
 * @param id - 页面内存对应的id, isCurMain - 是否给当前主线程使用：
 *									1-是，0-给对应页面的prepare线程使用
 * @return ptTmp - 满足要求的节点，NULL - 无满足要求的节点
 */
// 主要情况有，1.输入ID存在，则提取该显存数据，确定是主线程需要还是子线程需要 2.输入ID不存在，则为新数据，找到空闲显存（此时数据也为空，方便注入）
//  3.如果找不到空闲的且为空的数据，那么没办法，找到一个空闲的显存，但是已有数据，只能选择将原有数据覆盖（也就是预先分配的内存有限，只能供优先的使用）
PT_VideoMem GetVideoMem(int id, int isCurMain) 
{
	PT_VideoMem ptTmp = s_ptVieoMemHead;

	/* 1. 优先: 取出空闲的、ID相同的videomem */
	while (ptTmp)
	{
		if ((ptTmp->eVideoMemState == VMS_FREE) && (ptTmp->id == id))
		{
			ptTmp->eVideoMemState = isCurMain ? VMS_USED_FOR_CURMAIN : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

	/* 2. 如果前面不成功, 取出一个空闲的并且ptVideoMem->ePicState = PS_BLANK的videomem */
	ptTmp = s_ptVieoMemHead;
	while (ptTmp)
	{
		if ((ptTmp->eVideoMemState == VMS_FREE) && (ptTmp->ePicState == PS_BLANK))
		{
			ptTmp->id = id;
			ptTmp->eVideoMemState = isCurMain ? VMS_USED_FOR_CURMAIN : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}	
	
	/* 3. 如果前面不成功: 取出任意一个空闲的videomem */
	ptTmp = s_ptVieoMemHead;
	while (ptTmp)
	{
		if (ptTmp->eVideoMemState == VMS_FREE)
		{
			ptTmp->id = id;
			ptTmp->ePicState = PS_BLANK;
			ptTmp->eVideoMemState = isCurMain ? VMS_USED_FOR_CURMAIN : VMS_USED_FOR_PREPARE;
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}

    /* 4. 如果没有空闲的videomem并且isCurMain为1, 则取出任意一个videomem(不管它是否空闲) */
    if (isCurMain)
    {
    	ptTmp = s_ptVieoMemHead;
    	ptTmp->id = id;
    	ptTmp->ePicState = PS_BLANK;
    	ptTmp->eVideoMemState = isCurMain ? VMS_USED_FOR_CURMAIN : VMS_USED_FOR_PREPARE;
    	return ptTmp;
    }
    
	return NULL;
}

/**
 * @Description: 设置对应节点的内存为空闲状态
 * @param ptVideoMem - 需要设置的节点
 */
void PutVideoMem(PT_VideoMem ptVideoMem)
{
	ptVideoMem->eVideoMemState = VMS_FREE;
    if (ptVideoMem->id == -1)
    {
        ptVideoMem->ePicState = PS_BLANK;
    }
}


/**
 * @Description: 清屏函数  传入页面 tPixelDatas设置即可。
 * @param ptVideoMem - 需要设置的节点
 */
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int color)
{
	int i;
	unsigned char  *pPen8;
	unsigned short *pPen16;
	unsigned int   *pPen32;
	unsigned int 	color16bpp;
	unsigned int	red, green, blue;

	pPen8  = ptVideoMem->tPixelDatas.PixelDatas;
	pPen16 = (unsigned short *)pPen8;
	pPen32 = (unsigned int   *)pPen8;

	switch (ptVideoMem->tPixelDatas.bpp) {
	case 8:
		memset(pPen8, color, ptVideoMem->tPixelDatas.TotalBytes);
		break;
	case 16:
		/* RGB:565 */
		red      = ((color >> 16) & 0xffff) >> 3;
		green    = ((color >> 8 )  & 0xffff) >> 2;
		blue     = ((color >> 0 )  & 0xffff) >> 3;
		color16bpp    = (red << 11) | (green << 5) | blue;
		
		for (i = 0; i < ptVideoMem->tPixelDatas.TotalBytes;) {
			*pPen16 = color16bpp;
			pPen16++;
			i += 2;
		}
		break;
	case 32:
		for (i = 0; i < ptVideoMem->tPixelDatas.TotalBytes;) {
			*pPen32 = color;
			pPen32++;
			i += 4;
		}
		break;
	default:
		DBG_PRINTF("can not surport %dbpp\n", ptVideoMem->tPixelDatas.bpp);
		break;
	}

}

/* 把显存中某区域全部清为某种颜色 */  
// ptVideoMem 页面 ptLayout 区域 功能为将页面某一区域设置一个颜色。
void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor)
{
	unsigned char *pucVM;
	unsigned short *pwVM16bpp;
	unsigned int *pdwVM32bpp;
	unsigned short wColor16bpp; /* 565 */
	int iRed;
	int iGreen;
	int iBlue;
	int iX;
	int iY;
    int linebytesClear;
    int i;

	pucVM	   = ptVideoMem->tPixelDatas.PixelDatas + ptLayout->TopLeftY * ptVideoMem->tPixelDatas.linebytes + ptLayout->TopLeftX * ptVideoMem->tPixelDatas.bpp / 8;
	pwVM16bpp  = (unsigned short *)pucVM;
	pdwVM32bpp = (unsigned int *)pucVM;

    linebytesClear = (ptLayout->BotRightX - ptLayout->TopLeftX + 1) * ptVideoMem->tPixelDatas.bpp / 8;

	switch (ptVideoMem->tPixelDatas.bpp)
	{
		case 8:
		{
            for (iY = ptLayout->TopLeftY; iY <= ptLayout->BotRightY; iY++)
            {
    			memset(pucVM, dwColor, linebytesClear);
                pucVM += ptVideoMem->tPixelDatas.linebytes;
            }
			break;
		}
		case 16:
		{
			iRed   = (dwColor >> (16+3)) & 0x1f;
			iGreen = (dwColor >> (8+2)) & 0x3f;
			iBlue  = (dwColor >> 3) & 0x1f;
			wColor16bpp = (iRed << 11) | (iGreen << 5) | iBlue;
            for (iY = ptLayout->TopLeftY; iY <= ptLayout->BotRightY; iY++)
            {
                i = 0;
                for (iX = ptLayout->TopLeftX; iX <= ptLayout->BotRightX; iX++)
    			{
    				pwVM16bpp[i++]	= wColor16bpp;
    			}
                pwVM16bpp = (unsigned short *)((unsigned int)pwVM16bpp + ptVideoMem->tPixelDatas.linebytes);
            }
			break;
		}
		case 32:
		{
            for (iY = ptLayout->TopLeftY; iY <= ptLayout->BotRightY; iY++)
            {
                i = 0;
                for (iX = ptLayout->TopLeftX; iX <= ptLayout->BotRightX; iX++)
    			{
    				pdwVM32bpp[i++]	= dwColor;
    			}
                pdwVM32bpp = (unsigned int *)((unsigned int)pdwVM32bpp + ptVideoMem->tPixelDatas.linebytes);
            }
			break;
		}
		default :
		{
			DBG_PRINTF("can't support %d bpp\n", ptVideoMem->tPixelDatas.bpp);
			return;
		}
	}

}

/* 初始化函数 */
int DisplayInit(void)
{
	int error;
	
	error = FBInit();

	return error;
}


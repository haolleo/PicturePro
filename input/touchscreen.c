/**
 * @file  touchscreen.c
 * @brief touchscreen input设备的处初始化，处理过程与取消，参考tslib中的ts_print.c，实现滑动翻页
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 2,2020
 */
#include <stdlib.h>
#include <tslib.h>

#include "include/input_manager.h"
#include "include/disp_manager.h"
#include "include/config.h"
#include "include/draw.h"

static struct tsdev *s_pTSDev;	//touchscreen设备
static int s_Xres;	//LCD x方向的分辨率
static int s_Xres;	//LCD y方向的分辨率

/**
 * @Description: 获取touchscreen输入的初始化，设置模式使其可以采用非阻塞的方式获取输入
 * @return 成功：0 失败: -1
 * @note 由于需要获取到LCD的分辨率，所以这个函数被调用之前，SelectAndInitDisplay必须被调用，才可以获取到数据
 */
static int TouchScreenDevInit(void)
{	
	int bpp;
	char *pTSName = NULL;

	/* 根据环境变量获得设备名，并以阻塞的方式打开 */
	if((pTSName = getenv("TSLIB_TSDEVICE")) != NULL) 
		s_pTSDev = ts_open(pTSName, 0);
	else
		// /dev/input/event1是我的触摸屏地址
		s_pTSDev = ts_open("/dev/input/event1", 1);  

	if (!s_pTSDev) {
		DBG_PRINTF("ts_open error!\n");
		return -1;
	}

	if (ts_config(s_pTSDev)) {
		DBG_PRINTF("ts_config error!\n");
		return -1;
	}

	/* 获取LCD分辨率 */
	if (GetDispResolution(&s_Xres, &s_Xres, &bpp))
		return -1;

	return 0;
}

/**
 * @Description: touchscreen输入的退出，恢复原本的模式
 * @return 成功：0
 */
static int TouchScreenDevExit(void)
{
	return 0;
}

#if 0
/**
 * @Description: 判断上一次事件与此次事件相隔的时间是否超过500ms
 * @param ppreTime - 存储上一次事件时间的结构体指针，pcurTime - 此次存储事件时间的结构体指针 
 * @return 两次事件时间间隔超过500ms：0 无超过：1
 */
static int isOutOf500ms(struct timeval *ppreTime, struct timeval *pcurTime)
{
	int prems;
	int curms;

	/* tv_sec - 秒, tv_usec - 微秒 */
	prems = ppreTime->tv_sec * 1000 + ppreTime->tv_usec / 1000;
	curms = pcurTime->tv_sec * 1000 + pcurTime->tv_usec / 1000;

	return (curms > (prems + 500));
}
#endif

/**
 * @Description: 采用查询的方式获读取touchscreen数据
 * @param ptInputEvent - 表示input设备的结构体.
 * @return 触摸屏被按下：0 无按下：-1
 */
static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	int ret;
	int startflag;
	int endflag;
	struct ts_sample samp;

	endflag = startflag = 0;
	while(1) {
		/* 如果无数据则休眠 */
		ret = ts_read(s_pTSDev, &samp, 1);
		if (ret == 1) {
			/* 按下 */
			ptInputEvent->time 		= samp.tv;
			ptInputEvent->type 		= INPUT_TYPE_TOUCHSCREEN;
			ptInputEvent->x     	= samp.x;
			ptInputEvent->y    		= samp.y;
			ptInputEvent->pressure = samp.pressure;

			return 0;		
		} else 
			return -1;
	}
	
	return 0;
}

static T_InputOpr s_tTouchScreenOpr = {
	.name 			= "touchscreen",
	.DeviceInit 	= TouchScreenDevInit,
	.DeviceExit 	= TouchScreenDevExit,
	.GetInputEvent  = TouchScreenGetInputEvent,
};

/**
 * @Description: touchscreen input设备初始化函数，供上层调用注册设备
 * @return 成功：0
 */
int TouchScreenInit(void)
{
	return RegisterInputOpr(&s_tTouchScreenOpr);
}



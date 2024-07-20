/**
 * @file  stdin.c
 * @brief 标准输入input设备的处初始化，处理过程与取消
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 1,2020
 */
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "include/input_manager.h"

/**
 * @Description: 获取标准串口输入的初始化，设置模式使其可以采用非阻塞的方式获取输入
 * @return 成功：0
 */
static int StdinDevInit(void)
{
	struct termios ttystate;
	 
	/* 获得终端的状态 */
	tcgetattr(STDIN_FILENO, &ttystate);

	/* 关闭标准模式，并设置位为  1，表示接受最小用户数输入为1 */
	ttystate.c_lflag &= ~ICANON;	
	ttystate.c_cc[VMIN] = 1;		
		
	/* 设置术语状态 */
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

	return 0;
}

/**
 * @Description: 获取标准串口输入的退出，恢复原本的模式
 * @return 成功：0
 */
static int StdinDevExit(void)
{
	struct termios ttystate;
	 
	/* 获得终端的状态 */
	tcgetattr(STDIN_FILENO, &ttystate);
 
	/* 打开标准模式 */
	ttystate.c_lflag |= ICANON;

	/* 设置术语状态 */
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

	return 0;
}

/**
 * @Description: 根据标准输入的结果进行相应处理，采用阻塞的方式获取输入，配合子线程
 * @param ptInputEvent - 表示input设备获取输入事件的结构体.
 * @return 0
 */
static int StdinGetInputEvent(PT_InputEvent ptInputEvent)
{
	char c;
	
	/* 处理数据 */
	ptInputEvent->type = INPUT_TYPE_STDIN;

	/* 如果没有数据，则会休眠直到有输入 */
	c = fgetc(stdin);
	gettimeofday(&ptInputEvent->time, NULL);

#if 0
	if (c == 'u') {
		ptInputEvent->val = INPUT_VALUE_UP;
		gettimeofday(&ptInputEvent->time, NULL);
	} else if (c == 'n') {
		ptInputEvent->val = INPUT_VALUE_DOWN;		
		gettimeofday(&ptInputEvent->time, NULL);
	} else if (c == 'q') {
		ptInputEvent->val = INPUT_VALUE_EXIT;
		gettimeofday(&ptInputEvent->time, NULL);
	} else {
		ptInputEvent->val = INPUT_VALUE_UNKONW;
		gettimeofday(&ptInputEvent->time, NULL);
	}
#endif

	return 0;

}

static T_InputOpr s_tStdinOpr = {
	.name 			= "stdin",
	.DeviceInit 	= StdinDevInit,
	.DeviceExit 	= StdinDevExit,
	.GetInputEvent  = StdinGetInputEvent,
};

/**
 * @Description: 标准输入初始化函数，供上层调用注册设备
 * @return 成功：0
 */
int StdinInit(void)
{
	return RegisterInputOpr(&s_tStdinOpr);
}



#include <include/config.h>
#include <string.h>
#include <stdlib.h>

#include "include/input_manager.h"

static PT_InputOpr s_ptInputOprHead;	//链表头
static T_InputEvent s_tInputEvent;		//存储子线程获得的输入事件变量，供子线程共享
static pthread_mutex_t s_tMutex = PTHREAD_MUTEX_INITIALIZER;		//子线程互斥量
static pthread_cond_t s_tCondvar = PTHREAD_COND_INITIALIZER;		//条件变量

/* 函数名：		 注册函数
 * 函数功能：构建一个链表：把多个拓展文件的结构体“串”起来
 * 函数实现：根据传入的结点，首先判断该链表头是否为空
 *				空则，头结点指向传入的节点，且把节点的ptNext域指向NULL
 *				不空则，尾插法插入链表
 */
int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTmp;

	if (!s_ptInputOprHead)
	{
		s_ptInputOprHead   = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = s_ptInputOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}

	return 0;
}

/* 显示支持拓展文件的名字 */
void ShowInputOpr(void)
{
	int i = 0;
	PT_InputOpr ptTmp = s_ptInputOprHead;

	while (ptTmp) {
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* 初始化函数 */
int InputInit(void)
{
	int error;
	
	error = StdinInit();
	error |= TouchScreenInit();

	return error;
}

/* 获取输入事件 */
int GetInputEvent(PT_InputEvent ptInputEvent)
{
	/* 休眠 */
	/* 进入临界资源前，获得互斥量 */
	pthread_mutex_lock(&s_tMutex);	
	/* pthread_cond_wait会先解除之前的pthread_mutex_lock锁定的s_tMutex，
     * 然后阻塞在等待队列里休眠，直到再次被唤醒
     * （大多数情况下是等待的条件成立而被唤醒，唤醒后，该进程会先锁定pthread_mutex_lock(&s_tMutex)
     */
	pthread_cond_wait(&s_tCondvar, &s_tMutex);

	/* 被唤醒之后（执行完子线程对应的InputEventThreadFunction()函数）返回数据 */
	*ptInputEvent = s_tInputEvent;

	/* 释放互斥量 */
	pthread_mutex_unlock(&s_tMutex);	//解锁
	
	return 0;
}

/* 输入事件线程函数 */
static void *InputEventThreadFunction(void *pvoid)
{
	T_InputEvent tmpInputEvent;
	
	/* 定义函数指针 */
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	GetInputEvent = (int (*)(PT_InputEvent))pvoid;

	while(1) {
		/* 调用函数获得输入事件 */
		//调用输入设备的获取输入事件，如果有输入数据，就会给0
		if (GetInputEvent(&tmpInputEvent) == 0) {
			/* 唤醒主线程，把tmpInputEvent的值赋给一个全局变量 */
			/* 访问临界资源前，先获得互斥量 */
			pthread_mutex_lock(&s_tMutex);		//加锁
			s_tInputEvent = tmpInputEvent;
		
			/* 唤醒主线程 */
			pthread_cond_signal(&s_tCondvar);	//发送信号给处于阻塞等待状态的主线程

			/* 释放互斥量 */
			pthread_mutex_unlock(&s_tMutex);	//解锁
		}
	}
	
	return NULL;
}


/* 初始化所有支持的Input设备 */
int AllInputDeviceInit()
{
	int error;
	PT_InputOpr ptTmp;

	error = -1;
	ptTmp = s_ptInputOprHead;
	
	while (ptTmp) {
		if (ptTmp->DeviceInit() == 0) {
			/* 创建子线程 */
			error = pthread_create(&ptTmp->threadId, NULL, InputEventThreadFunction, ptTmp->GetInputEvent);
			if (error != 0) {				
				printf("pthread_creat error ,error code : %d\n", error);
				return error;
			}	
		}
		ptTmp = ptTmp->ptNext;
	}

	return 0;
}

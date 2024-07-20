
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "include/config.h"
#include "include/debug_manager.h"

static PT_DebugOpr s_ptDebugOprHead;
static int s_DebugLevelLimit = 8;	//打印级别，8为最高

/* 函数名：		 注册函数
 * 函数功能：构建一个链表：把多个拓展文件的结构体“串”起来
 * 函数实现：根据传入的结点，首先判断该链表头是否为空
 *				空则，头结点指向传入的节点，且把节点的ptNext域指向NULL
 *				不空则，尾插法插入链表
 */
int RegisterDebugOpr(PT_DebugOpr ptDebugOpr)
{
	PT_DebugOpr ptTmp;

	if (!s_ptDebugOprHead) {
		s_ptDebugOprHead   = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	} else {
		ptTmp = s_ptDebugOprHead;
		while (ptTmp->ptNext)
			ptTmp = ptTmp->ptNext;

		ptTmp->ptNext	    = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}

	return 0;
}

/* 显示支持拓展文件的名字 */
void ShowDebugOpr(void)
{
	int i = 0;
	PT_DebugOpr ptTmp = s_ptDebugOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

/* 设置打印级别
 * precvbuf = "dgblevel=<0-7>"
 */
int SetDebugLevel(char *precvbuf)
{
	s_DebugLevelLimit = precvbuf[9] - '0';
	return 0;
}

/* 打印函数 */  //C中调试信息的写法
int DebugPrint(const char * pFormat, ...)
{
	int num;
	int debuglevel;
	char tmpbuf[1000];
	char *tmpstr;
	va_list args;
	PT_DebugOpr ptTmp = s_ptDebugOprHead;

	//c中关于可变参数列表的用法，args指向pFormat后的可变参数
	va_start(args, pFormat);
	//将pFormat后的可变参数按照pFormat的格式转换后赋给tmpbuf
	num = vsprintf(tmpbuf, pFormat, args);
	va_end(args);
	tmpbuf[num] = '\0';
	
	debuglevel = DEFAULT_DBGLEVEL;
	tmpstr = tmpbuf;
	/* 根据打印级别决定打印何种信息 */ //感觉用正则表达式更好
	if ((tmpbuf[0] == '<') && (tmpbuf[2] == '>')) {
		debuglevel = tmpbuf[1] - '0';

		if ((debuglevel >= 0) && (debuglevel <= 9))
			tmpstr = tmpstr + 3;
		else
			debuglevel = DEFAULT_DBGLEVEL;
	}

	/* 打印级别不同 */
	if (debuglevel > s_DebugLevelLimit)
		return -1;

	/* 调用链表中所有isUsed==1的结构体的DebugPrint() */
	while (ptTmp)
	{
		if (ptTmp->isUsed == 1)
			ptTmp->DebugPrint(tmpstr);
		
		ptTmp = ptTmp->ptNext;
	}

	return 0;
}

/* 获取指定的名字的拓展文件结构体 */
PT_DebugOpr GetDebugOpr(char *pName)
{
	PT_DebugOpr ptTmp = s_ptDebugOprHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

/* 设置打印通道 
 * stdout=0			: 关闭stdout通道
 * stdout=1			: 打开stdout通道
 * netprint=0		: 关闭netprint通道
 * netprint=1		: 打开netprint通道
 */
//这个改为读取外部的配置文件更合适，由配置文件更改
int SetDebugChanel(char *precvbuf)
{
	char *strtmp;
	char strname[100];
	PT_DebugOpr ptTmp;

	memset(strname, 0, 100);
	strtmp = strchr(precvbuf, '='); //寻找precvbuf中第一个=的位置
	if (!strtmp)
		return -1;
	else {
		strncpy(strname, precvbuf, strtmp - precvbuf); //把=前面的赋值给strname
		ptTmp = GetDebugOpr(strname);

		if (ptTmp == NULL)
			return -1;
		//=后面的第一个字母，其实就是0或1
		if (strtmp[1] == '0')
			ptTmp->isUsed = 0;
		else
			ptTmp->isUsed = 1;
		
		return 0;
	}

	return 0;
}

/* 初始化函数 */
//只负责对网络打印和std打印进行注册
int DebugInit(void)
{
	int error;
	
	error  = StdoutInit();
	error |= NetPrintInit();
	
	return error;
}

int InitDebugChanel(void)
{
	PT_DebugOpr ptTmp = s_ptDebugOprHead;
	
	while (ptTmp)
	{
		if (ptTmp->isUsed && ptTmp->DebugInit)
			ptTmp->DebugInit();

		ptTmp = ptTmp->ptNext;
	}
	
	return 0;
}

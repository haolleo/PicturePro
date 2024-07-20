
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>

#include "include/config.h"
#include "include/debug_manager.h"


#define SERVER_PORT 	 	5678
#define PRINT_BUF_SIZE 		(16 * 1024)

static int s_SocketServer;		//服务器端套接字

static int s_isHaveConect = 0;	//服务器端与客户端连接标志，0-未连接，1-连接
static struct sockaddr_in s_tSocketServerAddr;	//服务器端
static struct sockaddr_in s_tSocketClientAddr;	//客户端

static char *s_pNetPrintBuf;	//网络打印缓冲区
static int s_WritePos = 0;		//环形缓冲区写位置
static int s_ReadPos  = 0;		//环形缓冲区读位置

static pthread_t s_tSendThreadId;		//发送线程ID
static pthread_t s_tRecvThreadId;		//接收线程ID
static pthread_cond_t s_tDebugSendCondvar   = PTHREAD_COND_INITIALIZER;		//发送线程条件变量
static pthread_mutex_t s_tNetDebugSendMutex = PTHREAD_MUTEX_INITIALIZER;	//发送线程互斥量

/* 判断缓冲区是否满， 满 - 1 */
static int isFull(void)
{
	return (((s_WritePos + 1) % PRINT_BUF_SIZE) == s_ReadPos);
}

/* 判断缓冲区是否空， 空 - 1 */
static int isEmpty(void)
{
	return (s_WritePos == s_ReadPos);
}

/* 把数据写入环形缓冲区 */
static int PutData(char val)
{
	if (isFull())
		return -1;
	else {
		s_pNetPrintBuf[s_WritePos] = val;		//写入数据
		s_WritePos = (s_WritePos + 1) % PRINT_BUF_SIZE;	//移动写位置
		return 0;
	}
}

/* 从环形缓冲区中读出数据 */
static int GetData(char *val)
{
	if (isEmpty())
		return -1;
	else {
		*val = s_pNetPrintBuf[s_ReadPos];		//读出数据
		s_ReadPos = (s_ReadPos + 1) % PRINT_BUF_SIZE;	//移动读位置
		return 0;
	}		
}

/* 发送线程函数 */
static void *NetDebugSendThreadFunction(void *pvoid)
{
	int i;
	int addrlen;
	int sendlen;
	char val;
	char sendbuf[512];
	
	addrlen = sizeof(struct sockaddr);
	
	while (1) {
		/* 休眠 */
		/* 进入临界资源前，获得互斥量 */
		pthread_mutex_lock(&s_tNetDebugSendMutex);	

		/* pthread_cond_wait会先解除之前的pthread_mutex_lock锁定的s_tNetDebugRecvMutex，
	     * 然后阻塞在等待队列里休眠，直到再次被唤醒
	     * （大多数情况下是等待的条件成立而被唤醒，唤醒后，该进程会先锁定pthread_mutex_lock(&s_tNetDebugRecvMutex)
	     */
		pthread_cond_wait(&s_tDebugSendCondvar, &s_tNetDebugSendMutex);

		/* 释放互斥量 */
		pthread_mutex_unlock(&s_tNetDebugSendMutex);

		/* 被唤醒之后，把环形缓冲区的数据取出来，用sendto()进行网络打印信息给客户端 */
		while ((s_isHaveConect == 1) && (!isEmpty())) {
			i = 0;
			
			/* 从环形缓冲区中取数据 */
			while((i < 512) && (GetData(&val) == 0)) {
				sendbuf[i] = val;
				i++;
			}

			/* 发送数据 */
			sendlen = sendto(s_SocketServer, sendbuf, i, 0, 
								(const struct sockaddr *)&s_tSocketClientAddr, addrlen);
		}
	}

	return NULL;
}

/* 接收线程函数 */
static void *NetDebugRecvThreadFunction(void *pvoid)
{
	int addrlen;
	int recvlen;
	char recvbuf[1000];
	struct sockaddr_in SocketClientAddr;

	memset(recvbuf, 0, 1000);
	addrlen = sizeof(struct sockaddr);
	while (1) {
		recvlen = recvfrom(s_SocketServer, recvbuf, 999, 0, 
							(struct sockaddr *)&SocketClientAddr, (socklen_t *)&addrlen);
		//udp在接收数据的时候会获得发送方和接收方的地址 SocketClientAddr 客户端地址   addrlen：客户端地址长度

		/* 处理数据 */  //可以重新设置客户端地址和调试等级
		if (addrlen > 0) {
			DBG_PRINTF("netprint.c get msg: %s\n", recvbuf);
			if (strcmp(recvbuf, "setclient") == 0)
			{
				/* 设置客户端 */
				s_tSocketClientAddr = SocketClientAddr;
				s_isHaveConect = 1;
			} else if (strncmp(recvbuf, "dbglevel=", 9) == 0)
				SetDebugLevel(recvbuf);		//设置打印级别
			else
				SetDebugChanel(recvbuf);	//设置打印通道
		}
	}

	return NULL;
}

/* socket初始化 */
static int NetPrintDebugInit(void)
{
	int ret;
	int error;

	printf("NetPrintDebugInit start\n");
	
	/* 分配一个套接口的描述字及其所用的资源
 	 * AF_INET：针对Internet的,因而可以允许在远程主机之间通信
 	 * SOCK_DGRAM：使用UDP协议,这样会提供定长的,不可靠,无连接的通信
	 */
	s_SocketServer = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == s_SocketServer)
	{
		printf("Socket error!\n");
		return -1;
	}

	s_tSocketServerAddr.sin_family	      = AF_INET;
	s_tSocketServerAddr.sin_port		  = htons(SERVER_PORT);  /* host to net, short */
	s_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset(s_tSocketServerAddr.sin_zero, 0, 8);

	/* 与socket返回的文件描述符捆绑在一起 */
	ret = bind(s_SocketServer, (const struct sockaddr *)&s_tSocketServerAddr, sizeof(struct sockaddr));
	if (ret == -1) {
		printf("Bind error:%s\n",strerror(errno));
		return -1;
	}

	/* 分配缓冲区 */
	s_pNetPrintBuf = malloc(PRINT_BUF_SIZE);
	if (s_pNetPrintBuf == NULL) {
		close(s_SocketServer);
		printf("s_pNetPrintBuf malloc error!\n");
		return -1;
	}

	/* 创建netprint发送线程：用来发送打印信息给客户端 */
	error = pthread_create(&s_tSendThreadId, NULL, NetDebugSendThreadFunction, NULL);
	if (error != 0) {
		printf("send pthread_creat error ,error code : %d\n", error);
		return error;
	}
	
	/* 创建netprint接收线程：用来接收控制信息，如修改打印级别、修改打印通道 */
	error = pthread_create(&s_tRecvThreadId, NULL, NetDebugRecvThreadFunction, NULL);
	if (error != 0) {
		printf("recv pthread_creat error ,error code : %d\n", error);
		return error;
	}

	printf("NetPrintDebugInit end\n");
	return 0;
}

/* 关闭socket */
static int NetPrinDebugtExit(void)
{
	close(s_SocketServer);
	free(s_pNetPrintBuf);
	
	return 0;
}

/* 网络打印服务器端 */
static int NetPrintDebugPrint(char *strdata)
{
	
	int i;

	/* 把数据放入到环形缓冲区 */
	for (i = 0; i < strlen(strdata); i++) {
		if (PutData(strdata[i]) != 0)
			break;
	}

	/* 进入临界资源前，获得互斥量 */
	pthread_mutex_lock(&s_tNetDebugSendMutex);	

	/* 客户端连接后，数据通过网络发送给客户端，采用线程的方式 */
	/* 唤醒netprint的发送线程 */
	pthread_cond_signal(&s_tDebugSendCondvar);

	/* 释放互斥量 */
	pthread_mutex_unlock(&s_tNetDebugSendMutex);
	
	return i;
}

static T_DebugOpr s_tNetPrintDebugOpr = {
	.name        = "netprint",
    .isUsed      = 0,
	.DebugInit   = NetPrintDebugInit,
	.DebugExit   = NetPrinDebugtExit,
	.DebugPrint  = NetPrintDebugPrint,
};

int NetPrintInit(void)
{
	return RegisterDebugOpr(&s_tNetPrintDebugOpr);
}


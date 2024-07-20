
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/page_manager.h"
#include "include/config.h"
#include "include/render.h"
#include "stdlib.h"
#include "include/file.h"

static pthread_t s_tAutoPlayThreadID;
static pthread_mutex_t s_tAutoPlayThreadMutex  = PTHREAD_MUTEX_INITIALIZER; /* 互斥量 */
static int s_AutoPlayThreadShouldExit = 0;
static T_PageCfg s_tPageCfg;

/* 以深度优先的方式获得目录下的文件 
 * 即: 先获得顶层目录下的文件, 再进入一级子目录A
 *     先获得一级子目录A下的文件, 再进入二级子目录AA, ...
 *     处理完一级子目录A后, 再进入一级子目录B
 *
 * "连播模式"下调用该函数获得要显示的文件
 * 有两种方法获得这些文件:
 * 1. 事先只需要调用一次函数,把所有文件的名字保存到某个缓冲区中
 * 2. 要使用文件时再调用函数,只保存当前要使用的文件的名字
 * 第1种方法比较简单,但是当文件很多时有可能导致内存不足.
 * 我们使用第2种方法:
 * 假设某目录(包括所有子目录)下所有的文件都给它编一个号
 * s_StartNumberToRecord : 从第几个文件开始取出它们的名字
 * s_CurFileNumber       : 本次函数执行时读到的第1个文件的编号
 * s_FileCountHaveGet    : 已经得到了多少个文件的名字
 * s_FileCountTotal      : 每一次总共要取出多少个文件的名字
 * s_NextProcessFileIndex: 在s_pstrFileNames数组中即将要显示在LCD上的文件
 *
 */
static int s_StartNumberToRecord = 0;
static int s_CurFileNumber = 0;
static int s_FileCountHaveGet = 0;
static int s_FileCountTotal = 0;
static int s_NextProcessFileIndex = 0;

#define FILE_COUNT 10 //一次性最多10个
static char s_pstrFileNames[FILE_COUNT][256];

static void ResetAutoPlayFile(void)
{
    s_StartNumberToRecord = 0;
    s_CurFileNumber = 0;
    s_FileCountHaveGet = 0;
    s_FileCountTotal = 0;
    s_NextProcessFileIndex = 0;
}

static int GetNextAutoPlayFile(char *strFileName)
{
    int error;
    
    if (s_NextProcessFileIndex < s_FileCountHaveGet)
    {
        strncpy(strFileName, s_pstrFileNames[s_NextProcessFileIndex], 256);
        s_NextProcessFileIndex++;
        return 0;
    }
    else
    {
        s_CurFileNumber    = 0;
        s_FileCountHaveGet = 0;
        s_FileCountTotal   = FILE_COUNT;
        s_NextProcessFileIndex = 0;
        error = GetFilesIndir(s_tPageCfg.strSeletedDir, &s_StartNumberToRecord, &s_CurFileNumber, &s_FileCountHaveGet, s_FileCountTotal, s_pstrFileNames);
        if (error || (s_NextProcessFileIndex >= s_FileCountHaveGet))
        {
            /* 再次从头读起(连播模式下循环显示) */
            s_StartNumberToRecord = 0;
            s_CurFileNumber    = 0;
            s_FileCountHaveGet = 0;
            s_FileCountTotal = FILE_COUNT;
            s_NextProcessFileIndex = 0;
            
            error = GetFilesIndir(s_tPageCfg.strSeletedDir, &s_StartNumberToRecord, &s_CurFileNumber, &s_FileCountHaveGet, s_FileCountTotal, s_pstrFileNames);
        }
        
        if (error == 0)
        {   
            if (s_NextProcessFileIndex < s_FileCountHaveGet)
            {
                strncpy(strFileName, s_pstrFileNames[s_NextProcessFileIndex], 256);
                s_NextProcessFileIndex++;
                return 0;
            }
        }
    }

    return -1;
}

/* cur = 1 表示必须获得videomem, 因为这是马上就要在LCD上显示出来的
 * cur = 0 表示这是做准备用的, 有可能无法获得videomem
 */
static PT_VideoMem PrepareNextPicture(int cur)
{
	T_PixelDatas tOriginIconPixelDatas;
	T_PixelDatas tPicPixelDatas;
    PT_VideoMem ptVideoMem;
	int error;
	int xres, yres, bpp;
    int TopLeftX, TopLeftY;
    float k;
    char strFileName[256];
    
	GetDispResolution(&xres, &yres, &bpp);
    
	/* 获得显存 */
	ptVideoMem = GetVideoMem(-1, cur);
	if (ptVideoMem == NULL)
	{
		DBG_PRINTF("can't get video mem for browse page!\n");
		return NULL;
	}
    ClearVideoMem(ptVideoMem, COLOR_BACKGROUND);

    while (1) //获得不出错的图片
    {
        //获取s_FileCountTotal数量的图片，并从next的序号开始显示
        error = GetNextAutoPlayFile(strFileName);
        if (error)
        {
            DBG_PRINTF("GetNextAutoPlayFile error\n");
            PutVideoMem(ptVideoMem);
            return NULL;
        }
        
        error = GetPixelDatasFrmFile(strFileName, &tOriginIconPixelDatas);
        if (0 == error)
        {
            break;
        }
    }

    /* 把图片按比例缩放到LCD屏幕上, 居中显示 */
    k = (float)tOriginIconPixelDatas.height / tOriginIconPixelDatas.width;
    tPicPixelDatas.width  = xres;
    tPicPixelDatas.height = xres * k;
    if (tPicPixelDatas.height > yres)
    {
        tPicPixelDatas.width  = yres / k;
        tPicPixelDatas.height = yres;
    }
    tPicPixelDatas.bpp        = bpp;
    tPicPixelDatas.linebytes  = tPicPixelDatas.width * tPicPixelDatas.bpp / 8;
    tPicPixelDatas.TotalBytes = tPicPixelDatas.linebytes * tPicPixelDatas.height;
    tPicPixelDatas.PixelDatas = (unsigned char *)malloc(tPicPixelDatas.TotalBytes);
    if (tPicPixelDatas.PixelDatas == NULL)
    {
        PutVideoMem(ptVideoMem);
        return NULL;
    }

    PicZoom(&tOriginIconPixelDatas, &tPicPixelDatas);

    /* 算出居中显示时左上角坐标 */
    TopLeftX = (xres - tPicPixelDatas.width) / 2;
    TopLeftY = (yres - tPicPixelDatas.height) / 2;
    PicMerge(TopLeftX, TopLeftY, &tPicPixelDatas, &ptVideoMem->tPixelDatas);
    FreePixelDatasFrmFile(&tOriginIconPixelDatas);
    free(tPicPixelDatas.PixelDatas);

    return ptVideoMem;
}


static void *AutoPlayThreadFunction(void *pVoid)
{
    int first = 1;
    int exit;
    PT_VideoMem ptVideoMem;

    ResetAutoPlayFile();
    
    while (1)
    {
        /* 1. 先判断是否要退出 */
        pthread_mutex_lock(&s_tAutoPlayThreadMutex);
        exit = s_AutoPlayThreadShouldExit;
        pthread_mutex_unlock(&s_tAutoPlayThreadMutex);

        if (exit)
        {
            return NULL;
        }

        /* 2. 准备要显示的图片 */
        ptVideoMem = PrepareNextPicture(0);

        /* 3. 时间到后就显示出来 */

        if (!first)
        {
            sleep(s_tPageCfg.IntervalSecond);       /* 先用休眠来代替 */
        }
        first = 0;
        
        if (ptVideoMem == NULL)
        {
            ptVideoMem = PrepareNextPicture(1);
        }

    	/* 刷到设备上去 */
    	FlushVideoMemToDev(ptVideoMem);

    	/* 解放显存 */
    	PutVideoMem(ptVideoMem);        

    }
    return NULL;
}

static int AutoPageRun(PT_PageParams ptParentPageParams)
{
	T_InputEvent tInputEvent;
	int ret;
    
    s_AutoPlayThreadShouldExit = 0;

    /* 获得配置值: 显示哪一个目录, 显示图片的间隔 */
    GetPageCfg(&s_tPageCfg);
        
    /* 1. 启动一个线程来连续显示图片 */
    pthread_create(&s_tAutoPlayThreadID, NULL, AutoPlayThreadFunction, NULL);

    /* 2. 当前线程等待触摸屏输入, 先做简单点: 如果点击了触摸屏, 让线程退出 */
    while (1)
    {
        ret = GetInputEvent(&tInputEvent);
        if (0 == ret)
        {
            pthread_mutex_lock(&s_tAutoPlayThreadMutex);
            s_AutoPlayThreadShouldExit = 1;   /* AutoPlayThreadFunction线程检测到这个变量为1后会退出 */
            pthread_mutex_unlock(&s_tAutoPlayThreadMutex);
            pthread_join(s_tAutoPlayThreadID, NULL);  /* 等待子线程退出 */
            return 0;
        }
    }

}


static T_PageAction g_tAutoPageAction = {
	.name          = "auto",
	.Run           = AutoPageRun,
};

int AutoPageInit(void)
{
	return RegisterPageAction(&g_tAutoPageAction);
}



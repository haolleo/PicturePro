
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "include/file.h"
#include "include/debug_manager.h"

/**
 * @Description: 获取文件信息，并对文件进行映射
 * @param ptFileMap - 需要打开的文件
 * @return 0 - 成功， -1 - 失败
 */
int MapFile(PT_FileMap ptFileMap)
{
	FILE *tFd;
	int fd;	
	struct stat tStat;	
	/* 打开bmp文件 */
    tFd = fopen(ptFileMap->strFileName, "r+"); //tFd为文件流
	if (tFd == NULL) {
		DebugPrint(APP_ERR"can not open %s! File:%s Line:%d \n\n", 
					ptFileMap->strFileName, __FILE__, __LINE__);
		return -1;
	}
    fd = fileno(tFd); //fd为文件描述符

	/* 获得文件的大小 */
	fstat(fd, &tStat);
	
	/* 直接映射到内存 */
	ptFileMap->pFileMem = (unsigned char *)mmap(NULL, tStat.st_size, 
								PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptFileMap->pFileMem == (unsigned char *)-1) {
		DebugPrint(APP_ERR"ptFileMap->pFileMem mmap err! \
							File:%s Line:%d \n", __FILE__, __LINE__);
		return -1;
	}

	/* 设置结构体信息 */
	ptFileMap->pFileFd   = tFd;
	ptFileMap->FileSize = tStat.st_size;
	
	return 0;
}

static int isDir(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

	/* stat()判断文件的属性，S_ISDIR()判断文件是否为目录 */
    if ((stat(strTmp, &tStat) == 0) && S_ISDIR(tStat.st_mode))
        return 1;
    else
        return 0;
	
}
static int isRegDir(char *strDirPath, char *strSubDirName)
{
    static const char *strSpecailDirs[] = {"sbin", "bin", "usr", "lib", "proc", "tmp", "dev", "sys", NULL};
    int i;
    
    /* 如果目录名含有"strSpecailDirs"中的任意一个, 则返回0 */
    if (0 == strcmp(strDirPath, "/"))
    {
        while (strSpecailDirs[i])
        {
            if (0 == strcmp(strSubDirName, strSpecailDirs[i]))
                return 0;
            i++;
        }
    }
    return 1;    
}


static int isRegFile(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

	/* stat()获取文件信息，S_ISREG()判断文件是否为文件 */
    if ((stat(strTmp, &tStat) == 0) && S_ISREG(tStat.st_mode))
        return 1;
    else
        return 0;

}


/* 把某目录下所含的顶层子目录、顶层目录下的文件都记录下来 */
//TODO:将目录文件一起处理，而不是分开处理，效率太低了
int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *pNumber)	
{
    PT_DirContent *ptDirContents;
    struct dirent **aptNameList; //目录指针
	int number;
	int i;
	int j;

	/* 扫描目录,结果按名字排序,存在aptNameList[0],aptNameList[1],... */
	number = scandir(strDirName, &aptNameList, 0, alphasort);
	if (number < 0)
	{
		DebugPrint("scandir error!\n");
		return -1;
	}

    /* 采用二维数组的形式去存储“strDirName”目录下的文件信息 */
	/* 忽略".", ".."这两个目录 */
    ptDirContents = malloc(sizeof(PT_DirContent) * (number - 2));//给二维指针赋地址，避免其成为野指针 *ptDirContents = PT_DirContent
	if (NULL == ptDirContents)
	{
		DebugPrint("malloc error!\n");
		return -1;
	}
    *pptDirContents = ptDirContents;

	for (i = 0; i < number - 2; i++)
	{
		ptDirContents[i] = malloc(sizeof(T_DirContent));
		if (NULL == ptDirContents)
		{
			DebugPrint("malloc error!\n");
			return -1;
		}
	}

	/* 先把目录挑出来存入ptDirContents */
	for (i = 0, j = 0; i < number; i++)
	{
		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
        /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		/* if (aptNameList[i]->d_type == DT_DIR) */
        if (isDir(strDirName, aptNameList[i]->d_name))
		{
			strncpy(ptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			ptDirContents[j]->strName[255] = '\0';
			ptDirContents[j]->eFileType    = FILETYPE_DIR;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
			j++;
		}
	}

	/* 再把常规文件挑出来存入ptDirContents */
	for (i = 0; i < number; i++)
	{
        if (aptNameList[i] == NULL) //因为已经被free了
            continue;
        
		/* 忽略".",".."这两个目录 */
		if ((0 == strcmp(aptNameList[i]->d_name, ".")) || (0 == strcmp(aptNameList[i]->d_name, "..")))
			continue;
        /* 并不是所有的文件系统都支持d_type, 所以不能直接判断d_type */
		/* if (aptNameList[i]->d_type == DT_REG) */
        if (isRegFile(strDirName, aptNameList[i]->d_name))
		{
			strncpy(ptDirContents[j]->strName, aptNameList[i]->d_name, 256);
			ptDirContents[j]->strName[255] = '\0';
			ptDirContents[j]->eFileType    = FILETYPE_FILE;
            free(aptNameList[i]);
            aptNameList[i] = NULL;
			j++;
		}
	}

	/* 释放ptDirContents中未使用的项 */
	for (i = j; i < number - 2; i++)
	{
		free(ptDirContents[i]);
	}

	/* 释放scandir函数分配的内存 */
	for (i = 0; i < number; i++)
	{
        if (aptNameList[i])
        {
    		free(aptNameList[i]);
        }
	}
	free(aptNameList);

    *pNumber = j; //有效目录文件数量
	
	return 0;
}


/**
 * @Description: 取消文件映射
 * @param ptFileUnMap - 文件
 * @return 0 - 成功， -1 - 失败
 */
int unMapFile(PT_FileMap ptFileUnMap)
{
	munmap(ptFileUnMap->pFileMem, ptFileUnMap->FileSize);
	fclose(ptFileUnMap->pFileFd);

	return 0;
}

void FreeDirContents(PT_DirContent *ptDirContents, int number)
{
	int i;
	for (i = 0; i < number; i++)
	{
		free(ptDirContents[i]);
	}
	free(ptDirContents);
}


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
 * pStartNumberToRecord : 从第几个文件开始取出它们的名字  //开始记录的编号
 * pCurFileNumber       : 本次函数执行时读到的第1个文件的编号  //当前编号
 * pFileCountHaveGet    : 已经得到了多少个文件的名字 //记录的编号数量
 * iFileCountTotal       : 总共要取出多少个文件的名字 //准备取出的编号
 *
 */
int GetFilesIndir(char *strDirName, int *pStartNumberToRecord, 
	int *pCurFileNumber, int *pFileCountHaveGet, int iFileCountTotal, char apstrFileNames[][256])
{
    int error;
    PT_DirContent *ptDirContents;  /* 数组:存有目录下"顶层子目录","文件"的名字 */
    int iDirContentsNumber;         /* ptDirContents数组有多少项 */
    int i;
    char strSubDirName[256];

    /* 为避免访问的目录互相嵌套, 设置能访问的目录深度为10 */
#define MAX_DIR_DEEPNESS 10    
    static int iDirDeepness = 0;

    if (iDirDeepness > MAX_DIR_DEEPNESS)
    {
        return -1;
    }

    iDirDeepness++;    

    /* 1、获得当前目录下的内容，使用scandir，以二维数组的形式存储文件/目录的名字和类型 */
    error = GetDirContents(strDirName, &ptDirContents, &iDirContentsNumber);    
    if (error)
    {
        DebugPrint(APP_ERR"GetDirContents error!\n");
        iDirDeepness--;
        return -1;
    }

    /* 先记录文件 */
    for (i = 0; i < iDirContentsNumber; i++)
    {
        if (ptDirContents[i]->eFileType == FILETYPE_FILE)
        {
            if (*pCurFileNumber >= *pStartNumberToRecord)
            {
                snprintf(apstrFileNames[*pFileCountHaveGet], 256, "%s/%s", strDirName, ptDirContents[i]->strName);
                (*pFileCountHaveGet)++;
                (*pCurFileNumber)++;
                (*pStartNumberToRecord)++;
                if (*pFileCountHaveGet >= iFileCountTotal)
                {
                    FreeDirContents(ptDirContents, iDirContentsNumber);
                    iDirDeepness--;
                    return 0;
                }
            }
            else
            {
                (*pCurFileNumber)++;
            }
        }
    }

    /* 递归处理目录 */
    for (i = 0; i < iDirContentsNumber; i++)
    {
        if ((ptDirContents[i]->eFileType == FILETYPE_DIR) && isRegDir(strDirName, ptDirContents[i]->strName))
        {
            snprintf(strSubDirName, 256, "%s/%s", strDirName, ptDirContents[i]->strName);
            GetFilesIndir(strSubDirName, pStartNumberToRecord, pCurFileNumber, pFileCountHaveGet, iFileCountTotal, apstrFileNames);
            if (*pFileCountHaveGet >= iFileCountTotal)
            {
                FreeDirContents(ptDirContents, iDirContentsNumber);
                iDirDeepness--;
                return 0;
            }
        }
    }

    FreeDirContents(ptDirContents, iDirContentsNumber);
    iDirDeepness--;
    return 0;
}


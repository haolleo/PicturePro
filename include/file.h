
#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>	

/* 映射文件的信息 */
typedef struct FileMap {
	char strFileName[256];
	FILE *pFileFd;
	int FileSize;
	unsigned char *pFileMem;
}T_FileMap, *PT_FileMap;

/* 表示是文件还是目录 */
typedef enum {
	FILETYPE_DIR = 0,
	FILETYPE_FILE,
}E_FileType;

/* 描述文件信息 */
typedef struct DirContent {
	char strName[256];
	E_FileType eFileType;	
}T_DirContent, *PT_DirContent;

int MapFile(PT_FileMap ptFileMap);
int unMapFile(PT_FileMap ptFileMap);int GetDirContents(char *strDirName, PT_DirContent **ptDirContents, int *pNumber);
int GetFilesIndir(char *strDirName, int *pStartNumberToRecord, 
int *pCurFileNumber, int *pFileCountHaveGet, int iFileCountTotal, char apstrFileNames[][256]);
void FreeDirContents(PT_DirContent *ptDirContents, int number);

#endif /* _FILE_H */

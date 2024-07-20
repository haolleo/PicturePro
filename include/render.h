
#ifndef _RENDER_H
#define _RENDER_H

#include "pic_operation.h"
#include "disp_manager.h"
#include "page_manager.h"

int PicZoom(PT_PixelDatas ptOriginPic, PT_PixelDatas ptZoomPic);
int PicMerge(int x, int y, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic);
void FlushVideoMemToDev(PT_VideoMem ptVideoMem);
int GetPixelDatasForIcon(char *strFileName, int DevBpp, PT_PixelDatas ptPixelDatas);
void FreePixelDatasForIcon(PT_PixelDatas ptOriginIconPixelDatas);
int isPictureFileSupported(char *strFileName);
void ReleaseButton(PT_Layout ptLayout);
void PressButton(PT_Layout ptLayout);
int GetPixelDatasFrmFile(char *strFileName, PT_PixelDatas ptPixelDatas);
void FreePixelDatasFrmFile(PT_PixelDatas ptPixelDatas);
int MergerStringToCenterOfRectangleInVideoMem(int TopLeftX, int TopLeftY, 
			int BotRightX, int BotRightY, unsigned char *pTextString, PT_VideoMem ptVideoMem);
void ClearRectangleInVideoMem(int TopLeftX, int TopLeftY, 
		int BotRightX, int BotRightY, PT_VideoMem ptVideoMem, unsigned int Color);
int PicMergeRegion(int iStartXofNewPic, int iStartYofNewPic, int iStartXofOldPic, int iStartYofOldPic, int iWidth, int iHeight, PT_PixelDatas ptNewPic, PT_PixelDatas ptOldPic);



#endif /* _RENDER_H */


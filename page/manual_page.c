#include <include/config.h>
#include <include/render.h>
#include <stdlib.h>
#include <include/file.h>
#include <include/fonts_manager.h>
#include <string.h>

/* 放大/缩小系数 */
#define ZOOM_RATIO (0.9)

#define SLIP_MIN_DISTANCE (2*2)

/* 菜单的区域 */
static T_Layout s_atManualMenuIconsLayout[] = {
	{"return.bmp", 0, 0, 0, 0},
	{"zoomout.bmp", 0, 0, 0, 0},
	{"zoomin.bmp", 0, 0, 0, 0},
	{"pre_pic.bmp", 0, 0, 0, 0},
    {"next_pic.bmp", 0, 0, 0, 0},
    {"continue_mod_small.bmp", 0, 0, 0, 0},
	{NULL, 0, 0, 0, 0},
};

static T_PageLayout s_tManualPageMenuIconsLayout = {
	.MaxTotalBytes = 0,
	.ptLayout       = s_atManualMenuIconsLayout,
};

/* 显示在LCD上的图片, 它的中心点, 在g_tZoomedPicPixelDatas里的坐标 */
static int g_iXofZoomedPicShowInCenter;  
static int g_iYofZoomedPicShowInCenter;

static T_Layout s_tManualPictureLayout;

static T_PixelDatas s_tOriginPicPixelDatas;
static T_PixelDatas s_tZoomedPicPixelDatas;

/* 计算菜单中各图标坐标值 */
static void  CalcManualPageMenusLayout(PT_PageLayout ptPageLayout)
{
	int width;
	int height;
	int xres, yres, bpp;
	int TmpTotalBytes;
	PT_Layout ptLayout;
	int i;

	ptLayout = ptPageLayout->ptLayout;
	GetDispResolution(&xres, &yres, &bpp);
	ptPageLayout->bpp = bpp;

	if (xres < yres)
	{			 
		/*	 xres/6
		 *	  --------------------------------------------------------------
		 *	   return	zoomout	zoomin  pre_pic next_pic continue_mod_small
		 *
		 *
		 *
		 *
		 *
		 *
		 *	  --------------------------------------------------------------
		 */
		 
		width  = xres / 6;
		height = width;

		/* return图标 */
		ptLayout[0].TopLeftY  = 0;
		ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height - 1;
		ptLayout[0].TopLeftX  = 0;
		ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width - 1;

        /* 其他5个图标 */
        for (i = 1; i < 6; i++)
        {
    		ptLayout[i].TopLeftY  = 0;
    		ptLayout[i].BotRightY = ptLayout[i].TopLeftY + height - 1;
    		ptLayout[i].TopLeftX  = ptLayout[i-1].BotRightX + 1;
    		ptLayout[i].BotRightX = ptLayout[i].TopLeftX + width - 1;
        }

	}
	else
	{
		/*	 yres/6
		 *	  ----------------------------------
		 *	   up		  
		 *
		 *    zoomout	    
		 *
		 *    zoomin
		 *  
		 *    pre_pic
		 *
		 *    next_pic
		 *
		 *    continue_mod_small
		 *
		 *	  ----------------------------------
		 */
		 
		height  = yres / 6;
		width = height;

		/* return图标 */
		ptLayout[0].TopLeftY  = 0;
		ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height - 1;
		ptLayout[0].TopLeftX  = 0;
		ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width - 1;
		
        /* 其他5个图标 */
        for (i = 1; i < 6; i++)
        {
    		ptLayout[i].TopLeftY  = ptLayout[i-1].BotRightY+ 1;
    		ptLayout[i].BotRightY = ptLayout[i].TopLeftY + height - 1;
    		ptLayout[i].TopLeftX  = 0;
    		ptLayout[i].BotRightX = ptLayout[i].TopLeftX + width - 1;
        }		
	}

	i = 0;
	while (ptLayout[i].strIconName)
	{
		TmpTotalBytes = (ptLayout[i].BotRightX - ptLayout[i].TopLeftX + 1) * (ptLayout[i].BotRightY - ptLayout[i].TopLeftY + 1) * bpp / 8;
		if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
		{
			ptPageLayout->MaxTotalBytes = TmpTotalBytes;
		}
		i++;
	}
}


/* 计算图片的显示区域 */
static void CalcManualPagePictureLayout(void)
{
	int xres, yres, bpp;
	int TopLeftX, TopLeftY;
	int BotRightX, BotRightY;
	
	GetDispResolution(&xres, &yres, &bpp);

	if (xres < yres)
	{
		/*	 xres/6
		 *	  --------------------------------------------------------------
		 *	   return	zoomout	zoomin  pre_pic next_pic continue_mod_small  (图标)
		 *	  --------------------------------------------------------------
		 *
		 *                              图片
		 *
		 *
		 *	  --------------------------------------------------------------
		 */
		TopLeftX  = 0;
		BotRightX = xres - 1;
		TopLeftY  = s_atManualMenuIconsLayout[0].BotRightY + 1;
		BotRightY = yres - 1;
	}
	else
	{
		/*	 yres/6
		 *	  --------------------------------------------------------------
		 *	   up		         |
		 *                       |
		 *    zoomout	         |
		 *                       |
		 *    zoomin             |
		 *                       |
		 *    pre_pic            |                 图片
		 *                       |
		 *    next_pic           |
		 *                       |
		 *    continue_mod_small | 
		 *                       |
		 *	  --------------------------------------------------------------
		 */
		TopLeftX  = s_atManualMenuIconsLayout[0].BotRightX + 1;
		BotRightX = xres - 1;
		TopLeftY  = 0;
		BotRightY = yres - 1;
	}

    s_tManualPictureLayout.TopLeftX   = TopLeftX;
    s_tManualPictureLayout.TopLeftY   = TopLeftY;
    s_tManualPictureLayout.BotRightX  = BotRightX;
    s_tManualPictureLayout.BotRightY  = BotRightY;
    s_tManualPictureLayout.strIconName = NULL;
}


static PT_PixelDatas GetZoomedPicPixelDatas(PT_PixelDatas ptOriginPicPixelDatas, int iZoomedWidth, int iZoomedHeight)
{
    float k;
	int iXres, iYres, bpp;
    
	GetDispResolution(&iXres, &iYres, &bpp);

    if (s_tZoomedPicPixelDatas.PixelDatas)
    {
        free(s_tZoomedPicPixelDatas.PixelDatas);
        s_tZoomedPicPixelDatas.PixelDatas = NULL;
    }
    
    k = (float)ptOriginPicPixelDatas->height / ptOriginPicPixelDatas->width;
    s_tZoomedPicPixelDatas.width  = iZoomedWidth;
    s_tZoomedPicPixelDatas.height = iZoomedWidth * k;
    if (s_tZoomedPicPixelDatas.height > iZoomedHeight)
    {
        s_tZoomedPicPixelDatas.width  = iZoomedHeight / k;
        s_tZoomedPicPixelDatas.height = iZoomedHeight;
    }
    s_tZoomedPicPixelDatas.bpp        = bpp;
    s_tZoomedPicPixelDatas.linebytes  = s_tZoomedPicPixelDatas.width * s_tZoomedPicPixelDatas.bpp / 8;
    s_tZoomedPicPixelDatas.TotalBytes = s_tZoomedPicPixelDatas.linebytes * s_tZoomedPicPixelDatas.height;
    s_tZoomedPicPixelDatas.PixelDatas = malloc(s_tZoomedPicPixelDatas.TotalBytes);
    if (s_tZoomedPicPixelDatas.PixelDatas == NULL)
    {
        return NULL;
    }
    
    PicZoom(ptOriginPicPixelDatas, &s_tZoomedPicPixelDatas);
    return &s_tZoomedPicPixelDatas;
}

static PT_PixelDatas GetOriginPictureFilePixelDatas(char *strFileName)
{
    int iError;

    if (s_tOriginPicPixelDatas.PixelDatas)
    {
        free(s_tOriginPicPixelDatas.PixelDatas);
        s_tOriginPicPixelDatas.PixelDatas = NULL;
    }
    
    /* 获得图片文件的数据 */
    iError = GetPixelDatasFrmFile(strFileName, &s_tOriginPicPixelDatas);
    if (iError)
    {
        return NULL;
    }
    else
    {
        return &s_tOriginPicPixelDatas;
    }
}

/* 计算两个触点的距离, 为简化计算, 返回距离的平方值 */
static int DistanceBetweenTwoPoint(PT_InputEvent ptInputEvent1, PT_InputEvent ptInputEvent2)
{
    return (ptInputEvent1->x - ptInputEvent2->x) * (ptInputEvent1->x - ptInputEvent2->x) + \
           (ptInputEvent1->y- ptInputEvent2->y) * (ptInputEvent1->y - ptInputEvent2->y);
}


static int ShowPictureInManualPage(PT_VideoMem ptVideoMem, char *strFileName)
{
	int iXres, iYres, bpp;
    int iPictureLayoutWidth;
    int iPictureLayoutheight;
    int TopLeftX, TopLeftY;
	PT_PixelDatas ptZoomedPicPixelDatas;
    PT_PixelDatas ptOriginPicPixelDatas;

	GetDispResolution(&iXres, &iYres, &bpp);

    /* 获得图片文件的数据 */
    ptOriginPicPixelDatas =  GetOriginPictureFilePixelDatas(strFileName);
    if (!ptOriginPicPixelDatas)
    {
        return -1;
    }
    
    /* 把图片按比例缩放到LCD屏幕上, 居中显示 */
    iPictureLayoutWidth  = s_tManualPictureLayout.BotRightX - s_tManualPictureLayout.TopLeftX + 1;
    iPictureLayoutheight = s_tManualPictureLayout.BotRightY - s_tManualPictureLayout.TopLeftY + 1;
    ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&s_tOriginPicPixelDatas, iPictureLayoutWidth, iPictureLayoutheight);
    if (!ptZoomedPicPixelDatas)
    {
        return -1;
    }
        
    /* 算出居中显示时左上角坐标 */
    TopLeftX = s_tManualPictureLayout.TopLeftX + (iPictureLayoutWidth - ptZoomedPicPixelDatas->width) / 2;
    TopLeftY = s_tManualPictureLayout.TopLeftY + (iPictureLayoutheight - ptZoomedPicPixelDatas->height) / 2;
    g_iXofZoomedPicShowInCenter = ptZoomedPicPixelDatas->width / 2;
    g_iYofZoomedPicShowInCenter = ptZoomedPicPixelDatas->height / 2;

    ClearVideoMemRegion(ptVideoMem, &s_tManualPictureLayout, COLOR_BACKGROUND);
    PicMerge(TopLeftX, TopLeftY, ptZoomedPicPixelDatas, &ptVideoMem->tPixelDatas);

    return 0;
}


static void ShowManualPage(PT_PageLayout ptPageLayout, char *strFileName)
{
	PT_VideoMem ptVideoMem;
	int error;
	int xres, yres, bpp;


	GetDispResolution(&xres, &yres, &bpp);
	PT_Layout ptLayout = ptPageLayout->ptLayout;
		
	/* 1. 获得显存 */
	ptVideoMem = GetVideoMem(ID("manual"), 1);
	if (ptVideoMem == NULL)
	{
		DBG_PRINTF("can't get video mem for manual page!\n");
		return;
	}

	/* 2. 描画数据 */

	/* 如果还没有计算过各图标的坐标 */
	if (ptLayout[0].TopLeftX == 0)
	{
		CalcManualPageMenusLayout(ptPageLayout);
        CalcManualPagePictureLayout();
	}

    /* 在videomem上生成图标 */
    //显示功能键
	error = GeneratePage(ptPageLayout, ptVideoMem);
	
	 /* 获得图片文件的数据 */
    //显示图片
    error = ShowPictureInManualPage(ptVideoMem, strFileName);
    if (error)
    {
        PutVideoMem(ptVideoMem);
        return;
    }
    

	/* 3. 刷到设备上去 */
	FlushVideoMemToDev(ptVideoMem);

	/* 4. 解放显存 */
	PutVideoMem(ptVideoMem);
}

static int ManualPageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
	return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}


static void ShowZoomedPictureInLayout(PT_PixelDatas ptZoomedPicPixelDatas, PT_VideoMem ptVideoMem)
{
    int iStartXofNewPic, iStartYofNewPic;
    int iStartXofOldPic, iStartYofOldPic;
    int widthPictureInPlay, heightPictureInPlay;
    int iPictureLayoutWidth, iPictureLayoutHeight;
    int iDeltaX, iDeltaY;

    iPictureLayoutWidth  = s_tManualPictureLayout.BotRightX - s_tManualPictureLayout.TopLeftX + 1;
    iPictureLayoutHeight = s_tManualPictureLayout.BotRightY - s_tManualPictureLayout.TopLeftY + 1;
    
    /* 显示新数据 */
    iStartXofNewPic = g_iXofZoomedPicShowInCenter - iPictureLayoutWidth/2;
    if (iStartXofNewPic < 0)
    {
        iStartXofNewPic = 0;
    }
    if (iStartXofNewPic > ptZoomedPicPixelDatas->width)
    {
        iStartXofNewPic = ptZoomedPicPixelDatas->width;
    }

    /* 
     * g_iXofZoomedPicShowInCenter - iStartXofNewPic = PictureLayout中心点X坐标 - iStartXofOldPic
     */
    iDeltaX = g_iXofZoomedPicShowInCenter - iStartXofNewPic;
    iStartXofOldPic = (s_tManualPictureLayout.TopLeftX + iPictureLayoutWidth / 2) - iDeltaX;
    if (iStartXofOldPic < s_tManualPictureLayout.TopLeftX)
    {
        iStartXofOldPic = s_tManualPictureLayout.TopLeftX;
    }
    if (iStartXofOldPic > s_tManualPictureLayout.BotRightX)
    {
        iStartXofOldPic = s_tManualPictureLayout.BotRightX + 1;
    }
        
    if ((ptZoomedPicPixelDatas->width - iStartXofNewPic) > (s_tManualPictureLayout.BotRightX - iStartXofOldPic + 1))
        widthPictureInPlay = (s_tManualPictureLayout.BotRightX - iStartXofOldPic + 1);
    else
        widthPictureInPlay = (ptZoomedPicPixelDatas->width - iStartXofNewPic);
    
    iStartYofNewPic = g_iYofZoomedPicShowInCenter - iPictureLayoutHeight/2;
    if (iStartYofNewPic < 0)
    {
        iStartYofNewPic = 0;
    }
    if (iStartYofNewPic > ptZoomedPicPixelDatas->height)
    {
        iStartYofNewPic = ptZoomedPicPixelDatas->height;
    }

    /* 
     * g_iYofZoomedPicShowInCenter - iStartYofNewPic = PictureLayout中心点Y坐标 - iStartYofOldPic
     */
    iDeltaY = g_iYofZoomedPicShowInCenter - iStartYofNewPic;
    iStartYofOldPic = (s_tManualPictureLayout.TopLeftY + iPictureLayoutHeight / 2) - iDeltaY;

    if (iStartYofOldPic < s_tManualPictureLayout.TopLeftY)
    {
        iStartYofOldPic = s_tManualPictureLayout.TopLeftY;
    }
    if (iStartYofOldPic > s_tManualPictureLayout.BotRightY)
    {
        iStartYofOldPic = s_tManualPictureLayout.BotRightY + 1;
    }
    
    if ((ptZoomedPicPixelDatas->height - iStartYofNewPic) > (s_tManualPictureLayout.BotRightY - iStartYofOldPic + 1))
    {
        heightPictureInPlay = (s_tManualPictureLayout.BotRightY - iStartYofOldPic + 1);
    }
    else
    {
        heightPictureInPlay = (ptZoomedPicPixelDatas->height - iStartYofNewPic);
    }
        
    ClearVideoMemRegion(ptVideoMem, &s_tManualPictureLayout, COLOR_BACKGROUND);
    PicMergeRegion(iStartXofNewPic, iStartYofNewPic, iStartXofOldPic, iStartYofOldPic, widthPictureInPlay, heightPictureInPlay, ptZoomedPicPixelDatas, &ptVideoMem->tPixelDatas);
}

/* manual页面的Run参数含义: 
 * 如果它的父页面是主页面   - 显示browse页面, 
 * 否则 - 显示图片
 */
static int ManualPageRun(PT_PageParams ptParentPageParams)
{
	T_InputEvent tInputEvent;
	T_InputEvent tPreInputEvent;
	int bPressed = 0;
	int iIndexPressed = -1;
    int iIndex;
    T_PageParams tPageParams;
    int iError;
    char strDirName[256];
    char strFileName[256];
    char strFullPathName[256];
    PT_DirContent *aptDirContents;
    int iDirContentsNumber;
    int iPicFileIndex;
    char *pcTmp;
	int ZoomedWidth;
	int ZoomedHeight;
    PT_PixelDatas ptZoomedPicPixelDatas;
	int widthPictureInPlay, heightPictureInPlay;
	int iPictureLayoutWidth, iPictureLayoutHeight;
    PT_VideoMem ptDevVideoMem;
	int iXres, iYres, bpp;
	int bPicSlipping= 0;

	GetDispResolution(&iXres, &iYres, &bpp);

	widthPictureInPlay = 0;
	heightPictureInPlay = 0;
	tPreInputEvent.x = 0;
	tPreInputEvent.y = 0;
	ptZoomedPicPixelDatas = &s_tZoomedPicPixelDatas;

    tPageParams.iPageID = ID("manual");
    
    if (ptParentPageParams->iPageID == ID("main"))//会出现这种情况吗？？
    {
        /* 如果它的父页面是主页面 - 显示browse页面*/
        Page("browse")->Run(ptParentPageParams);
    }
    else
    {
        ptDevVideoMem = GetDevVideoMem();
        strcpy(strFullPathName, ptParentPageParams->strCurPictureFile);
      

		/* 显示菜单和图片文件 */
		ShowManualPage(&s_tManualPageMenuIconsLayout, strFullPathName);

	   iPictureLayoutWidth	= s_tManualPictureLayout.BotRightX - s_tManualPictureLayout.TopLeftX + 1;
	   iPictureLayoutHeight = s_tManualPictureLayout.BotRightY - s_tManualPictureLayout.TopLeftY + 1;

        /* 取出目录名 */
        strcpy(strDirName, ptParentPageParams->strCurPictureFile);
        //找到最后一个/，即当前图片的目录
        pcTmp = strrchr(strDirName, '/');
        *pcTmp = '\0';

        /* 取出文件名 */
        strcpy(strFileName, pcTmp+1);

        /* 获得当前目录下所有目录和文件的名字 */
        iError = GetDirContents(strDirName, &aptDirContents, &iDirContentsNumber);

        /* 确定当前显示的是哪一个文件 */
        // 主要确定点击显示的图片在目录中的序号
        for (iPicFileIndex = 0; iPicFileIndex < iDirContentsNumber; iPicFileIndex++)
        {
            if (0 == strcmp(strFileName, aptDirContents[iPicFileIndex]->strName))
            {
                break;
            }
        }

        while (1)
        {
            /* 先确定是否触摸了菜单图标 */
            iIndex = ManualPageGetInputEvent(&s_tManualPageMenuIconsLayout, &tInputEvent);
            if (tInputEvent.pressure == 0)
            {
                /* 如果是松开 */
                if (bPressed)
                {
                	bPicSlipping = 0;
                    /* 曾经有按钮被按下 */
                    ReleaseButton(&s_atManualMenuIconsLayout[iIndexPressed]);
                    bPressed = 0;

                    if (iIndexPressed == iIndex) /* 按下和松开都是同一个按钮 */
                    {
                        switch (iIndexPressed)
                        {
                            case 0: /* 返回按钮 */
                            {
                                return 0;
                                break;
                            }
                           case 1: /* 缩小按钮 */
                           {
								/* 获得缩小后的数据 */
								ZoomedWidth  = (float)s_tZoomedPicPixelDatas.width * ZOOM_RATIO;
								ZoomedHeight = (float)s_tZoomedPicPixelDatas.height * ZOOM_RATIO;
								ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&s_tOriginPicPixelDatas, ZoomedWidth, ZoomedHeight);

								/* 重新计算中心点 */
								g_iXofZoomedPicShowInCenter = (float)g_iXofZoomedPicShowInCenter * ZOOM_RATIO;
								g_iYofZoomedPicShowInCenter = (float)g_iYofZoomedPicShowInCenter * ZOOM_RATIO;

								/* 显示新数据 */
							   ShowZoomedPictureInLayout(ptZoomedPicPixelDatas, ptDevVideoMem);
								break;
							}
							case 2: /* 放大按钮 */
							{
								/* 获得放大后的数据 */
								ZoomedWidth  = (float)s_tZoomedPicPixelDatas.width / ZOOM_RATIO;
								ZoomedHeight = (float)s_tZoomedPicPixelDatas.height / ZOOM_RATIO;
								ptZoomedPicPixelDatas = GetZoomedPicPixelDatas(&s_tOriginPicPixelDatas, ZoomedWidth, ZoomedHeight);
								
								/* 重新计算中心点 */
								g_iXofZoomedPicShowInCenter = (float)g_iXofZoomedPicShowInCenter / ZOOM_RATIO;
								g_iYofZoomedPicShowInCenter = (float)g_iYofZoomedPicShowInCenter / ZOOM_RATIO;

								 /* 显示新数据 */
                                ShowZoomedPictureInLayout(ptZoomedPicPixelDatas, ptDevVideoMem);
								break;
                            }
                            case 3: /* "上一张"按钮 */
                            {
                                while (iPicFileIndex > 0)
                                {
                                    iPicFileIndex--;
                                    snprintf(strFullPathName, 256, "%s/%s", strDirName, aptDirContents[iPicFileIndex]->strName);
                                    strFullPathName[255] = '\0';
                                    
                                    if (isPictureFileSupported(strFullPathName))
                                    {
                                        ShowPictureInManualPage(ptDevVideoMem, strFullPathName);
                                        break;
                                    }
                                }
                                
                                break;
                            }
                            case 4: /* "下一张"按钮 */
                            {
                                while (iPicFileIndex < iDirContentsNumber - 1)
                                {
                                    iPicFileIndex++;
                                    snprintf(strFullPathName, 256, "%s/%s", strDirName, aptDirContents[iPicFileIndex]->strName);
                                    strFullPathName[255] = '\0';

                                    if (isPictureFileSupported(strFullPathName))
                                    {
                                        ShowPictureInManualPage(ptDevVideoMem, strFullPathName);
                                        break;
                                    }
                                }
                                break;
                            }
                            case 5: /* "连播"按钮 */
                            {
                                /* Manual页面的触发有两个方法: 在主页面按"浏览模式"进入"浏览页面"->"选中某个文件" 2.在"连播页面"里点击正在显示的图片
                                 * 如果是后者, 直接return就可以了:因为return后是返回到"连播页面"的, 它会继续"连播"
                                 */
                                DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                                if (ptParentPageParams->iPageID == ID("browse"))  /* 触发自"浏览页面" */
                                {
                                    DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
                                    strcpy(tPageParams.strCurPictureFile, strFullPathName);
                                    Page("auto")->Run(&tPageParams);  /* auto页面返回前,会把它正在显示的文件存在tPageParams.strCurPictureFile */
                                    ShowManualPage(&s_tManualPageMenuIconsLayout, tPageParams.strCurPictureFile);
                                }
                                else /* 当前manual页面的父页面是auto页面, 直接返回即可 */
                                {
                                  
                                    return 0;
                                }
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                    }
                    
                    iIndexPressed = -1;
                }
            }
            else
            {
                /* 按下状态 */
                if (iIndex != -1)
                {
                    if (!bPressed)
                    {
                        /* 未曾按下按钮 */
                        bPressed = 1;
                        iIndexPressed = iIndex;
                        PressButton(&s_atManualMenuIconsLayout[iIndexPressed]);
                    }
                }
				else  /* 点击的是图片显示区域, 滑动图片 */
				{
					/* 如果没有按钮被按下 */
					if (!bPressed && !bPicSlipping)
					{
						bPicSlipping = 1;
                        tPreInputEvent = tInputEvent;
					}

					if (bPicSlipping)
					{
                        /* 如果触点滑动距离大于规定值, 则挪动图片 */
                        if (DistanceBetweenTwoPoint(&tInputEvent, &tPreInputEvent) > SLIP_MIN_DISTANCE)
                        {                            
                            /* 重新计算中心点 */
                            g_iXofZoomedPicShowInCenter -= (tInputEvent.x - tPreInputEvent.x);
                            g_iYofZoomedPicShowInCenter -= (tInputEvent.y - tPreInputEvent.y);
                            
                            /* 显示新数据 */
                            ShowZoomedPictureInLayout(ptZoomedPicPixelDatas, ptDevVideoMem);
                            
                            /* 记录滑动点 */
                            tPreInputEvent = tInputEvent;                            
                        }
					}
				}
            }       

            
        }
    }
	return 0;
}

static T_PageAction s_tManualPageAction = {
	.name          = "manual",
	.Run           = ManualPageRun,
};

int ManualPageInit(void)
{
	return RegisterPageAction(&s_tManualPageAction);
}


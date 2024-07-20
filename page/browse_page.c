
/**
 * @file  browse_page.c
 * @brief 图片显示文件
 * @version 1.0 （版本声明）
 * @author Dk
 * @date  July 6,2020
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "include/config.h"
#include "include/render.h"
#include "include/page_manager.h"
#include "include/input_manager.h"
#include "include/file.h"

#define DIR_FILE_ICON_WIDTH    40
#define DIR_FILE_ICON_HEIGHT   DIR_FILE_ICON_WIDTH
#define DIR_FILE_NAME_HEIGHT   20
#define DIR_FILE_NAME_WIDTH   (DIR_FILE_ICON_HEIGHT + DIR_FILE_NAME_HEIGHT) //为了能保证填下？设计最大长度？
#define DIR_FILE_ALL_WIDTH    DIR_FILE_NAME_WIDTH
#define DIR_FILE_ALL_HEIGHT   DIR_FILE_ALL_WIDTH

/*
 * browse页面有两个区域: 菜单图标, 目录和文件图标
 * 为统一处理, "菜单图标"的序号为0,1,2,3,..., "目录和文件图标"的序号为1000,1001,1002,....
 *
 */
#define DIRFILE_ICON_INDEX_BASE 1000

/* 用来描述某目录里的内容 */
static PT_DirContent *s_ptDirContents;  /* 数组:存有目录下"顶层子目录","文件"的名字 */
static int s_DirContentsNumber;         /* s_ptDirContents数组有多少项 */ //因为链表无法得知大小
static int s_StartIndex = 0;            /* 在屏幕上显示的第1个"目录和文件"是s_ptDirContents数组里的哪一项 */

static int s_DirFileNumPerCol, s_DirFileNumPerRow;

/* 目录或文件的区域 */
static char *s_strDirClosedIconName  = "fold_closed.bmp";
static char *s_strDirOpenedIconName  = "fold_opened.bmp";
static char *g_strFileIconName = "file.bmp";

static T_Layout *s_ptDirAndFileLayout; //0 图标 1： 文字  目录和文件的layout显示

//将图标提取出来，并根据实际显示大小赋予内容
static T_PixelDatas s_tDirClosedIconPixelDatas;
static T_PixelDatas s_tDirOpenedIconPixelDatas;
static T_PixelDatas s_tFileIconPixelDatas;

/* 当前显示的目录 */
static char s_strCurDir[256] = DEFAULT_DIR;
// 被选择的目录 ：： 用来进行连播模式
static char s_strSelectedDir[256] = DEFAULT_DIR;

static T_Layout s_tBrowsePageMenusLayout[] = {
//	{"return.bmp",    0, 0, 0, 0},
    { "up.bmp", 	  0, 0, 0, 0},
    {"select.bmp",    0, 0, 0, 0},
    {"pre_page.bmp",  0, 0, 0, 0},
    {"next_page.bmp", 0, 0, 0, 0},
    {NULL,			  0, 0, 0, 0},
};

// 菜单界面是固定的
static T_PageLayout s_tBrowsePageLayout = {
    .MaxTotalBytes = 0,
    .ptLayout      = s_tBrowsePageMenusLayout,
};

// 文件和目录界面不固定
static T_PageLayout s_tBrowsePageDirAndFileLayout = {
    .MaxTotalBytes = 0,
    //.ptLayout      = s_tBrowsePageMenusLayout,
};

void GetSelectedDir(char *strSeletedDir)
{
    strncpy(strSeletedDir, s_strSelectedDir, 256);
    strSeletedDir[255] = '\0';
}


/**
 * @Description: browse页面输入事件函数
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static int BrowsePageGetInputEvent(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
    return GenericGetInputEvent(ptPageLayout, ptInputEvent);
}

static int GetInputPositionInPageLayout(PT_PageLayout ptPageLayout, PT_InputEvent ptInputEvent)
{
    int i = 0;
    PT_Layout ptLayout = ptPageLayout->ptLayout;

    /* 处理数据 */
    /* 确定触点位于哪一个按钮上 */
    while (ptLayout[i].BotRightY)
    {
        if ((ptInputEvent->x >= ptLayout[i].TopLeftX) && (ptInputEvent->x <= ptLayout[i].BotRightX) && \
             (ptInputEvent->y >= ptLayout[i].TopLeftY) && (ptInputEvent->y   <= ptLayout[i].BotRightY))
        {
            /* 找到了被点中的按钮 */
            return i;
        }
        else
        {
            i++;
        }
    }

    /* 触点没有落在按钮上 */
    return -1;
}


/* 计算各图标坐标值 */
static void  CalcBrowsePageLayout(PT_PageLayout ptPageLayout)
{	
    int i;
    int width;
    int height;
    int xres, yres, bpp;
    int TmpTotalBytes;
    PT_Layout ptLayout;

    ptLayout = ptPageLayout->ptLayout;
    GetDispResolution(&xres, &yres, &bpp);
    ptPageLayout->bpp = bpp;

    /*   xres/4
     *    ----------------------------------
     *     up   select  pre_page  next_page
     *
     *
     *
     *
     *
     *
     *    ----------------------------------
     */
     //取短边作为放置图标的地方
    if (xres < yres) {
        width  = xres / 4;
        height = width;

        /* return图标 */
        ptLayout[0].TopLeftY  = 0;
        ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height - 1;
        ptLayout[0].TopLeftX  = 0;
        ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width - 1;

        /* up图标 */
        ptLayout[1].TopLeftY  = 0;
        ptLayout[1].BotRightY = ptLayout[1].TopLeftY + height - 1;
        ptLayout[1].TopLeftX  = ptLayout[0].BotRightX + 1;
        ptLayout[1].BotRightX = ptLayout[1].TopLeftX + width - 1;

        /* select图标 */
        ptLayout[2].TopLeftY  = 0;
        ptLayout[2].BotRightY = ptLayout[2].TopLeftY + height - 1;
        ptLayout[2].TopLeftX  = ptLayout[1].BotRightX + 1;
        ptLayout[2].BotRightX = ptLayout[2].TopLeftX + width - 1;

        /* pre_page图标 */
        ptLayout[3].TopLeftY  = 0;
        ptLayout[3].BotRightY = ptLayout[3].TopLeftY + height - 1;
        ptLayout[3].TopLeftX  = ptLayout[2].BotRightX + 1;
        ptLayout[3].BotRightX = ptLayout[3].TopLeftX + width - 1;

    } else {
        height  = yres / 4;
        width = height;

        /* return图标 */
        ptLayout[0].TopLeftY  = 0;
        ptLayout[0].BotRightY = ptLayout[0].TopLeftY + height - 1;
        ptLayout[0].TopLeftX  = 0;
        ptLayout[0].BotRightX = ptLayout[0].TopLeftX + width - 1;

        /* up图标 */
        ptLayout[1].TopLeftY  = ptLayout[0].BotRightY+ 1;
        ptLayout[1].BotRightY = ptLayout[1].TopLeftY + height - 1;
        ptLayout[1].TopLeftX  = 0;
        ptLayout[1].BotRightX = ptLayout[1].TopLeftX + width - 1;

        /* select图标 */
        ptLayout[2].TopLeftY  = ptLayout[1].BotRightY + 1;
        ptLayout[2].BotRightY = ptLayout[2].TopLeftY + height - 1;
        ptLayout[2].TopLeftX  = 0;
        ptLayout[2].BotRightX = ptLayout[2].TopLeftX + width - 1;

        /* pre_page图标 */
        ptLayout[3].TopLeftY  = ptLayout[2].BotRightY + 1;
        ptLayout[3].BotRightY = ptLayout[3].TopLeftY + height - 1;
        ptLayout[3].TopLeftX  = 0;
        ptLayout[3].BotRightX = ptLayout[3].TopLeftX + width - 1;
    }

    i = 0;
    //有一点进步
    while (ptLayout[i].strIconName) {
        TmpTotalBytes = (ptLayout[i].BotRightX - ptLayout[i].TopLeftX + 1) *
                    (ptLayout[i].BotRightY - ptLayout[i].TopLeftY + 1) * bpp / 8;
        if (ptPageLayout->MaxTotalBytes < TmpTotalBytes)
            ptPageLayout->MaxTotalBytes = TmpTotalBytes;

        i++;
    }

}

/* 计算目录和文件的显示区域 */
//将整块区域进行分割，而不是根据实际的文件和目录数量进行
static int CalcBrowsePageDirAndFilesLayout(void)
{
    int xres, yres, bpp;
    int TopLeftX, TopLeftY;
    int TopLeftXBak;
    int BotRightX, BotRightY;
    int IconWidth, IconHeight;
    int NumPerCol, NumPerRow;
    int DeltaX, DeltaY;
    int i, j, k = 0;

    GetDispResolution(&xres, &yres, &bpp);

    if (xres < yres)
    {
        /* --------------------------------------
         *    up select pre_page next_page 图标
         * --------------------------------------
         *
         *           目录和文件
         *
         *
         * --------------------------------------
         */
        TopLeftX  = 0;
        BotRightX = xres - 1;
        TopLeftY  = s_tBrowsePageMenusLayout[0].BotRightY + 1;
        BotRightY = yres - 1;
    }
    else
    {
        /*	 yres/4
         *	  ----------------------------------
         *	   up      |
         *             |
         *    select   |
         *             |     目录和文件
         *    pre_page |
         *             |
         *   next_page |
         *             |
         *	  ----------------------------------
         */
        TopLeftX  = s_tBrowsePageMenusLayout[0].BotRightX + 1;
        BotRightX = xres - 1;
        TopLeftY  = 0;
        BotRightY = yres - 1;
    }

    /* 确定一行显示多少个"目录或文件", 显示多少行 */
    IconWidth  = DIR_FILE_NAME_WIDTH;
    IconHeight = IconWidth;

    /* 图标之间的间隔要大于10个象素 */
    NumPerRow = (BotRightX - TopLeftX + 1) / IconWidth; //计算能放置的图标数量
    while (1)
    {
        DeltaX  = (BotRightX - TopLeftX + 1) - IconWidth * NumPerRow;
        if ((DeltaX / (NumPerRow + 1)) < 10)//剩余的空间足够负担NumPerRow + 1个图标的间隔
            NumPerRow--;
        else
            break;
    }

    NumPerCol = (BotRightY - TopLeftY + 1) / IconHeight;
    while (1)
    {
        DeltaY  = (BotRightY - TopLeftY + 1) - IconHeight * NumPerCol;
        if ((DeltaY / (NumPerCol + 1)) < 10)
            NumPerCol--;
        else
            break;
    }

    /* 每个图标之间的间隔 */
    DeltaX = DeltaX / (NumPerRow + 1);
    DeltaY = DeltaY / (NumPerCol + 1);

    s_DirFileNumPerRow = NumPerRow;
    s_DirFileNumPerCol = NumPerCol;

    //DebugPrint("s_DirFileNumPerRow = %d, s_DirFileNumPerCol = %d\n", s_DirFileNumPerRow, s_DirFileNumPerCol);

    /* 可以显示 NumPerRow * NumPerCol个"目录或文件"
     * 分配"两倍+1"的T_Layout结构体: 一个用来表示图标,另一个用来表示名字
     * 最后一个用来存NULL,借以判断结构体数组的末尾
     */
    s_ptDirAndFileLayout = malloc(sizeof(T_Layout) * (2 * NumPerRow * NumPerCol + 1));
    if (NULL == s_ptDirAndFileLayout)
    {
        DebugPrint("malloc error!\n");
        return -1;
    }

    /* "目录和文件"整体区域的左上角、右下角坐标 */
    s_tBrowsePageDirAndFileLayout.TopLeftX      = TopLeftX;
    s_tBrowsePageDirAndFileLayout.BotRightX     = BotRightX;
    s_tBrowsePageDirAndFileLayout.TopLeftY      = TopLeftY;
    s_tBrowsePageDirAndFileLayout.BotRightY     = BotRightY;
    s_tBrowsePageDirAndFileLayout.bpp           = bpp;
    s_tBrowsePageDirAndFileLayout.ptLayout       = s_ptDirAndFileLayout;
    s_tBrowsePageDirAndFileLayout.MaxTotalBytes = DIR_FILE_ALL_WIDTH * DIR_FILE_ALL_HEIGHT * bpp / 8;

    //DebugPrint("s_tBrowsePageDirAndFileLayout: \n");
    //DebugPrint("(%d, %d) ~ (%d, %d)\n", TopLeftX, TopLeftY, BotRightX, BotRightY);
    
    /* 确定图标和名字的位置
     *
     * 图标是一个正方体, "图标+名字"也是一个正方体
     *   --------
     *   |  图  |
     *   |  标  |
     * ------------
     * |   名字   |
     * ------------
     */
    TopLeftX += DeltaX;
    TopLeftY += DeltaY;
    TopLeftXBak = TopLeftX;
    for (i = 0; i < NumPerCol; i++)
    {
        for (j = 0; j < NumPerRow; j++)
        {
            /* 图标 */
            s_ptDirAndFileLayout[k].TopLeftX  = TopLeftX + (DIR_FILE_NAME_WIDTH - DIR_FILE_ICON_WIDTH) / 2;
            s_ptDirAndFileLayout[k].BotRightX = s_ptDirAndFileLayout[k].TopLeftX + DIR_FILE_ICON_WIDTH - 1;
            s_ptDirAndFileLayout[k].TopLeftY  = TopLeftY;
            s_ptDirAndFileLayout[k].BotRightY = TopLeftY + DIR_FILE_ICON_HEIGHT - 1;

            /* 名字 */
            s_ptDirAndFileLayout[k+1].TopLeftX  = TopLeftX;
            s_ptDirAndFileLayout[k+1].BotRightX = TopLeftX + DIR_FILE_NAME_WIDTH - 1;
            s_ptDirAndFileLayout[k+1].TopLeftY  = s_ptDirAndFileLayout[k].BotRightY + 1; //同一排的字在其对应图标下面1个像素
            s_ptDirAndFileLayout[k+1].BotRightY = s_ptDirAndFileLayout[k+1].TopLeftY + DIR_FILE_NAME_HEIGHT - 1;

            TopLeftX += DIR_FILE_ALL_WIDTH + DeltaX;
            k += 2;
        }
        TopLeftX = TopLeftXBak;
        TopLeftY += DIR_FILE_ALL_HEIGHT + DeltaY;
    }

    /* 结尾 */
    s_ptDirAndFileLayout[k].TopLeftX   = 0;
    s_ptDirAndFileLayout[k].BotRightX  = 0;
    s_ptDirAndFileLayout[k].TopLeftY   = 0;
    s_ptDirAndFileLayout[k].BotRightY  = 0;
    s_ptDirAndFileLayout[k].strIconName = NULL;

    return 0;
}

/* aptDirContents数组中有iDirContentNumber项
 * 从第iStartIndex项开始显示, 显示满一页
 */
static int GenerateBrowsePageDirAndFile(int iStartIndex, int iDirContentsNumber, PT_DirContent *aptDirContents, PT_VideoMem ptVideoMem)
{
    int error;
    int i, j, k = 0;
    int iDirContentIndex = iStartIndex;
    PT_PageLayout ptPageLayout = &s_tBrowsePageDirAndFileLayout;
    PT_Layout ptLayout = ptPageLayout->ptLayout;

    ClearRectangleInVideoMem(ptPageLayout->TopLeftX, ptPageLayout->TopLeftY, ptPageLayout->BotRightX, ptPageLayout->BotRightY, ptVideoMem, COLOR_BACKGROUND);

    SetFontSize(ptLayout[1].BotRightY - ptLayout[1].TopLeftY - 5);

    for (i = 0; i < s_DirFileNumPerCol; i++)
    {
        for (j = 0; j < s_DirFileNumPerRow; j++)
        {
            if (iDirContentIndex < iDirContentsNumber)
            {
                /* 显示目录或文件的图标 */
                if (aptDirContents[iDirContentIndex]->eFileType == FILETYPE_DIR)
                {
                    PicMerge(ptLayout[k].TopLeftX, ptLayout[k].TopLeftY, &s_tDirClosedIconPixelDatas, &ptVideoMem->tPixelDatas);
                }
                else
                {
                    PicMerge(ptLayout[k].TopLeftX, ptLayout[k].TopLeftY, &s_tFileIconPixelDatas, &ptVideoMem->tPixelDatas);
                }

                k++;
                /* 显示目录或文件的名字 */
                //DBG_PRINTF("MergerStringToCenterOfRectangleInVideoMem: %s\n", aptDirContents[iDirContentIndex]->strName);
                //显示图标下面的文字
                error = MergerStringToCenterOfRectangleInVideoMem(ptLayout[k].TopLeftX, ptLayout[k].TopLeftY, ptLayout[k].BotRightX, ptLayout[k].BotRightY, (unsigned char *)aptDirContents[iDirContentIndex]->strName, ptVideoMem);
                //文字也是p_layout，也要+1
                k++;

                iDirContentIndex++;
            }
            else
            {
                break;
            }
        }
        if (iDirContentIndex >= iDirContentsNumber)
        {
            break;
        }
    }
    return 0;

}

static int GenerateDirAndFileIcons(PT_PageLayout ptPageLayout)
{
    T_PixelDatas tOriginIconPixelDatas;
    int error;
    int xres, yres, bpp;
    PT_Layout ptLayout = ptPageLayout->ptLayout;

    GetDispResolution(&xres, &yres, &bpp);

    /* 给目录图标、文件图标分配内存 */
    s_tDirClosedIconPixelDatas.bpp          = bpp;
    s_tDirClosedIconPixelDatas.PixelDatas = malloc(ptPageLayout->MaxTotalBytes); //好像大小是确定的
    if (s_tDirClosedIconPixelDatas.PixelDatas == NULL)
    {
        return -1;
    }

    s_tDirOpenedIconPixelDatas.bpp          = bpp;
    s_tDirOpenedIconPixelDatas.PixelDatas = malloc(ptPageLayout->MaxTotalBytes);
    if (s_tDirOpenedIconPixelDatas.PixelDatas == NULL)
    {
        return -1;
    }

    s_tFileIconPixelDatas.bpp          = bpp;
    s_tFileIconPixelDatas.PixelDatas = malloc(ptPageLayout->MaxTotalBytes);
    if (s_tFileIconPixelDatas.PixelDatas == NULL)
    {
        return -1;
    }

    /* 从BMP文件里提取图像数据 */
    /* 1. 提取"fold_closed图标" */
    error = GetPixelDatasForIcon(s_strDirClosedIconName, s_tFileIconPixelDatas.bpp, &tOriginIconPixelDatas);
    if (error)
    {
        DBG_PRINTF("GetPixelDatasForIcon %s error!\n", s_strDirClosedIconName);
        return -1;
    }
    s_tDirClosedIconPixelDatas.height = ptLayout[0].BotRightY - ptLayout[0].TopLeftY + 1;
    s_tDirClosedIconPixelDatas.width  = ptLayout[0].BotRightX - ptLayout[0].TopLeftX + 1;
    s_tDirClosedIconPixelDatas.linebytes  = s_tDirClosedIconPixelDatas.width * s_tDirClosedIconPixelDatas.bpp / 8;
    s_tDirClosedIconPixelDatas.TotalBytes = s_tDirClosedIconPixelDatas.linebytes * s_tDirClosedIconPixelDatas.height;
    PicZoom(&tOriginIconPixelDatas, &s_tDirClosedIconPixelDatas);//把原始大小缩放至设置大小
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    /* 2. 提取"fold_opened图标" */
    error = GetPixelDatasForIcon(s_strDirOpenedIconName,  s_tFileIconPixelDatas.bpp, &tOriginIconPixelDatas);
    if (error)
    {
        DBG_PRINTF("GetPixelDatasForIcon %s error!\n", s_strDirOpenedIconName);
        return -1;
    }
    s_tDirOpenedIconPixelDatas.height = ptLayout[0].BotRightY - ptLayout[0].TopLeftY + 1;
    s_tDirOpenedIconPixelDatas.width  = ptLayout[0].BotRightX - ptLayout[0].TopLeftX + 1;
    s_tDirOpenedIconPixelDatas.linebytes  = s_tDirOpenedIconPixelDatas.width * s_tDirOpenedIconPixelDatas.bpp / 8;
    s_tDirOpenedIconPixelDatas.TotalBytes = s_tDirOpenedIconPixelDatas.linebytes * s_tDirOpenedIconPixelDatas.height;
    PicZoom(&tOriginIconPixelDatas, &s_tDirOpenedIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    /* 3. 提取"file图标" */
    error = GetPixelDatasForIcon(g_strFileIconName, s_tFileIconPixelDatas.bpp, &tOriginIconPixelDatas);
    if (error)
    {
        DBG_PRINTF("GetPixelDatasForIcon %s error!\n", g_strFileIconName);
        return -1;
    }
    s_tFileIconPixelDatas.height = ptLayout[0].BotRightY - ptLayout[0].TopLeftY + 1;
    s_tFileIconPixelDatas.width  = ptLayout[0].BotRightX - ptLayout[0].TopLeftX+ 1;
    s_tFileIconPixelDatas.linebytes  = s_tDirClosedIconPixelDatas.width * s_tDirClosedIconPixelDatas.bpp / 8;
    s_tFileIconPixelDatas.TotalBytes = s_tFileIconPixelDatas.linebytes * s_tFileIconPixelDatas.height;
    PicZoom(&tOriginIconPixelDatas, &s_tFileIconPixelDatas);
    FreePixelDatasForIcon(&tOriginIconPixelDatas);

    return 0;
}

/* 对于目录图标, 把它改为"file_opened图标"
 * 对于文件图标, 只是反选
 */
static void SelectDirFileIcon(int DirFileIndex)
{
    int DirFileContentIndex;
    PT_VideoMem ptDevVideoMem;

    ptDevVideoMem = GetDevVideoMem();

    DirFileIndex = DirFileIndex & ~1;
    DirFileContentIndex = s_StartIndex + DirFileIndex/2;
    
    if (s_ptDirContents[DirFileContentIndex]->eFileType == FILETYPE_DIR)
    {
        PicMerge(s_ptDirAndFileLayout[DirFileIndex].TopLeftX, s_ptDirAndFileLayout[DirFileIndex].TopLeftY,
                    &s_tDirClosedIconPixelDatas, &ptDevVideoMem->tPixelDatas);
    }
    else
    {
        PressButton(&s_ptDirAndFileLayout[DirFileIndex]);
        PressButton(&s_ptDirAndFileLayout[DirFileIndex + 1]);
    }
}

/* 对于目录图标, 把它改为"file_closeed图标"
 * 对于文件图标, 只是反选
 */
static void DeSelectDirFileIcon(int DirFileIndex)
{
    int DirFileContentIndex;
    PT_VideoMem ptDevVideoMem;
    
    ptDevVideoMem = GetDevVideoMem();

    DirFileIndex = DirFileIndex & ~1; // 最后一位强行变为0 将DirFileIndex变为向下取为2的整数 如3-》2 5-》4
    DirFileContentIndex = s_StartIndex + DirFileIndex/2;
    char str[10];
    snprintf(str, sizeof(str), "<3>%d", DirFileContentIndex);
    printf(str);
    
    if (s_ptDirContents[DirFileContentIndex]->eFileType == FILETYPE_DIR)
    {
        PicMerge(s_ptDirAndFileLayout[DirFileIndex].TopLeftX, s_ptDirAndFileLayout[DirFileIndex].TopLeftY,
                    &s_tDirClosedIconPixelDatas, &ptDevVideoMem->tPixelDatas);
    }
    else
    {
        ReleaseButton(&s_ptDirAndFileLayout[DirFileIndex]);
        ReleaseButton(&s_ptDirAndFileLayout[DirFileIndex + 1]);
    }
}

static void ShowBrowsePage(PT_PageLayout ptPageLayout)
{
    int error;
    PT_VideoMem ptVideoMem;
    PT_Layout ptLayout;
    ptLayout = ptPageLayout->ptLayout;

    /* 获得显存 */
    ptVideoMem = GetVideoMem(ID("browse"), 1);
    if (ptVideoMem == NULL) {
        DebugPrint(APP_ERR"Can not get video mem for main_page!\n");
        return ;
    }

    /* 描画数据 */
    /* 如果还没有计算过各图标的坐标 */
    // 计算页面和图标的位置 ：： 所有空间计算，不根据实际的文件和目录数量来，假设所有区域都有
    if (ptLayout[0].TopLeftX == 0) {
        CalcBrowsePageLayout(ptPageLayout);
        CalcBrowsePageDirAndFilesLayout();
    }

     /* 如果还没有生成"目录和文件"的图标 */
    // 从icon中提取开闭目录，文件的图标，并给PixelData对象
    if (!s_tDirClosedIconPixelDatas.PixelDatas)
    {
        GenerateDirAndFileIcons(&s_tBrowsePageDirAndFileLayout);
    }

    error = GeneratePage(ptPageLayout, ptVideoMem);

    error = GenerateBrowsePageDirAndFile(s_StartIndex, s_DirContentsNumber, s_ptDirContents, ptVideoMem);
    /* 刷新/加载到设备 */
    FlushVideoMemToDev(ptVideoMem);

    /* 设置页面内存为空闲状态 */
    // 因为已经由framebuffer显示，页面内存就可以释放了
    PutVideoMem(ptVideoMem);
}


/**
 * @Description: 获取指定的名字的拓展文件，供外部函数调用，找寻对应结构体
 * @param pName - 寻找结构体的名字
 * @return 0
 */
static int BrowsePageRun(PT_PageParams ptParentPageParams)
{
    int index;
    int error;
    int pressured;
    int indexPressured;
    int UsedToSelectDir = 0;
    char *pcTmp;
    char strTmp[256];
    T_InputEvent tInputEvent;
    T_InputEvent tInputEventPrePress;
    PT_VideoMem ptDevVideoMem;
    int	DirFileContentIndex;
    T_PageParams tPageParams;

    /* "连续播放图片"时, 是从哪一个目录读取文件呢? 这是由"选择"按钮来选择该目录的
     * 点击目录图标将进入下一级目录, 哪怎样选择目录呢? //感觉改成双击进去目录更合适
     * 1. 先点击"选择"按钮
     * 2. 再点击某个目录图标
     */
    int HaveClickSelectIcon = 0;
    

    ptDevVideoMem = GetDevVideoMem();

    /* 0. 获得要显示的目录的内容 */
    error = GetDirContents(s_strCurDir, &s_ptDirContents, &s_DirContentsNumber); //默认是根目录
    if (error) {
        DebugPrint("GetDirContents error!\n");
        return error;
    }
    
    //DebugPrint("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    //DebugPrint("s_DirContentsNumber = %d\n", s_DirContentsNumber);

    /* 显示页面 */
    ShowBrowsePage(&s_tBrowsePageLayout);

    /* 创建Prepare线程 */

    index = -1;
    indexPressured = -1;
    pressured = 0;
    /* 调用GetInputEvent(), 获得输入事件，进而处理 */
    while (1) {

        /* 先确定是否触摸了菜单图标 */
        index = BrowsePageGetInputEvent(&s_tBrowsePageLayout, &tInputEvent);

        /* 如果触点不在菜单图标上, 则判断是在哪一个"目录和文件"上 */
        if (index == -1)
        {
            index = GetInputPositionInPageLayout(&s_tBrowsePageDirAndFileLayout, &tInputEvent);
            if (index != -1)
            {
                if (s_StartIndex + index / 2 < s_DirContentsNumber)  /* 判断这个触点上是否有图标 */ // /2是因为图标和文字加起来是一个布局
                    index += DIRFILE_ICON_INDEX_BASE; /* 这是"目录和文件图标" */
                else
                    index = -1;
            }
        }

        /* 松开和按下不在同一个图标范围内 */
        // 松开的处理逻辑： 按下松开 -- 则可以进行操作  对操作的解析是放在松开之后处理
        if (tInputEvent.pressure == 0) {
            /* 曾经有按键按下 */
            if (pressured) {

                if (indexPressured < DIRFILE_ICON_INDEX_BASE) {  /* 菜单图标 */

                     /* 释放图标 */
                    if (!(UsedToSelectDir && (indexPressured == 1))) /* "选择"图标单独处理 */
                    {
                        ReleaseButton(&s_tBrowsePageMenusLayout[indexPressured]);
                    }

                    pressured = 0;

                /* 松开与按下的按键为同一个 */
                if (indexPressured == index) {
                    switch (indexPressured) {
                        case 0: 	/* "向上"按钮 */
                            if (0 == strcmp(s_strCurDir, "/")) {  /* 已经是顶层目录 */
                                FreeDirContents(s_ptDirContents, s_DirContentsNumber);
                                return 0;
                            }

                            // 如果此时目录在/book/file 那么 / 变 \0 则为 /book 之后重新来一遍GetDirContents获取/book的内容，然后显示， 此时不用计算，因为计算是一次性计算所有的区域
                            pcTmp = strrchr(s_strCurDir, '/'); /* 找到最后一个'/', 把它去掉 */
                            *pcTmp = '\0';
                            FreeDirContents(s_ptDirContents, s_DirContentsNumber);
                            error = GetDirContents(s_strCurDir, &s_ptDirContents, &s_DirContentsNumber);
                            if (error)
                            {
                                DBG_PRINTF("GetDirContents error!\n");
                                return error;
                            }
                            s_StartIndex = 0;
                            error = GenerateBrowsePageDirAndFile(s_StartIndex, s_DirContentsNumber, s_ptDirContents, ptDevVideoMem);

                            break;

                        case 1:		/* 选择 */
                            if (!UsedToSelectDir)
                            {
                                /* 如果不是用于"选择目录", 该按钮无用处 */
                                break;
                            }

                            if (!HaveClickSelectIcon)  /* 第1次点击"选择"按钮 */
                            {
                                HaveClickSelectIcon = 1;
                            }
                            else
                            {
                                ReleaseButton(&s_tBrowsePageMenusLayout[indexPressured]);
                                pressured    = 0;
                                HaveClickSelectIcon = 0;
                            }
                            break;

                        case 2:		/* 上一页 */ //是一个循环，可以一直上一页，如果是第一页，则会变成最后一页
                            s_StartIndex -=s_DirFileNumPerCol * s_DirFileNumPerRow;
                            if (s_StartIndex >= 0)
                                error = GenerateBrowsePageDirAndFile(s_StartIndex, s_DirContentsNumber,
                                                s_ptDirContents, ptDevVideoMem);
                            else
                                s_StartIndex +=s_DirFileNumPerCol * s_DirFileNumPerRow;
                            break;

                        case 3:		/* 下一页 */
                            s_StartIndex +=s_DirFileNumPerCol * s_DirFileNumPerRow;
                            if (s_StartIndex <= s_DirContentsNumber )
                                error = GenerateBrowsePageDirAndFile(s_StartIndex, s_DirContentsNumber,
                                                s_ptDirContents, ptDevVideoMem);
                            else
                                s_StartIndex -=s_DirFileNumPerCol * s_DirFileNumPerRow;

                            break;

                        default:
                            break;
                    }
                }
            }
            else /* "目录和文件图标" */
            {
                DeSelectDirFileIcon(indexPressured - DIRFILE_ICON_INDEX_BASE);
                pressured      = 0;
                /*
                 * 如果按下和松开时, 触点不处于同一个图标上, 则释放图标
                 */
                if (indexPressured != index)
                {
                    return 0;
                }
                else if (HaveClickSelectIcon) /* 按下和松开都是同一个按钮, 并且"选择"按钮是按下状态 */
                {
                    /* 如果是目录, 记录这个目录 */
                    DirFileContentIndex = s_StartIndex + (indexPressured - DIRFILE_ICON_INDEX_BASE)/2;
                    if (s_ptDirContents[DirFileContentIndex]->eFileType == FILETYPE_DIR)
                    {
                        ReleaseButton(&s_tBrowsePageMenusLayout[1]);  /* 同时松开"选择按钮" */
                        HaveClickSelectIcon = 0;

                        /* 记录目录名 */
                        snprintf(strTmp, 256, "%s/%s", s_strCurDir, s_ptDirContents[DirFileContentIndex]->strName);
                        strTmp[255] = '\0';
                        strcpy(s_strSelectedDir, strTmp);
                    }
                }
                else  /* "选择"按钮不被按下时, 单击目录则进入 */
                {
                    /* 如果是目录, 进入这个目录 */
                    DirFileContentIndex = s_StartIndex + (indexPressured - DIRFILE_ICON_INDEX_BASE)/2;
                    if (s_ptDirContents[DirFileContentIndex]->eFileType == FILETYPE_DIR)
                    {
                        snprintf(strTmp, 256, "%s/%s", s_strCurDir, s_ptDirContents[DirFileContentIndex]->strName);
                        strTmp[255] = '\0';
                        strcpy(s_strCurDir, strTmp);
                        FreeDirContents(s_ptDirContents, s_DirContentsNumber);
                        error = GetDirContents(s_strCurDir, &s_ptDirContents, &s_DirContentsNumber);
                        if (error)
                        {
                            DBG_PRINTF("GetDirContents error!\n");
                            return -1;
                        }
                        s_StartIndex = 0;
                        error = GenerateBrowsePageDirAndFile(s_StartIndex, s_DirContentsNumber, s_ptDirContents, ptDevVideoMem);
                    }
                    else if (UsedToSelectDir == 0) /* UsedToSelectDir为0时单击文件时显示它 */
                    {
                        snprintf(tPageParams.strCurPictureFile, 256, "%s/%s", s_strCurDir, s_ptDirContents[DirFileContentIndex]->strName);
                        tPageParams.strCurPictureFile[255] = '\0';
                        //如果浏览模式点击图片，则会进入manual界面
                        if (isPictureFileSupported(tPageParams.strCurPictureFile))
                        {
                            tPageParams.iPageID = ID("browse");
                            Page("manual")->Run(&tPageParams);
                            ShowBrowsePage(&s_tBrowsePageLayout);
                        }
                    }
                }
            }
            }
        }
        else {
            /* 松开和按下都在同一个图标范围内 */
            /* 按下 */
            // 按下的处理逻辑
            if (index != -1) {

                /* 之前未按下 */
                if (!pressured) {
                    pressured = 1;
                    indexPressured = index;
                    tInputEventPrePress = tInputEvent;  /* 记录下来 */
                    if (index < DIRFILE_ICON_INDEX_BASE)  /* 菜单图标 */
                    {
                        if (UsedToSelectDir) {
                            if (!(HaveClickSelectIcon && (indexPressured == 1)))  /* 如果已经按下"选择"按钮, 自然不用再次反转该图标 */
                                PressButton(&s_tBrowsePageMenusLayout[index]);
                        }else  {
                            if (!HaveClickSelectIcon)
                                PressButton(&s_tBrowsePageMenusLayout[index]);
                        }
                    }
                    else   /* 目录和文件图标 */
                    {
                        SelectDirFileIcon(index - DIRFILE_ICON_INDEX_BASE);
                    }
                }

                /* 长按"向上"按钮, 返回 */
                if (indexPressured == 0)
                {
                    if (TimeMSBetween(tInputEventPrePress.time, tInputEvent.time) > 2000) {
                        FreeDirContents(s_ptDirContents, s_DirContentsNumber);
                        return 0;
                    }

                }
            }
        }
    }

    return 0;
}

static int BrowsePagePrepare()
{
    return 0;
}

static T_PageAction s_tBrowsePageAction = {
    .name 			= "browse",
    .Run 			= BrowsePageRun,
    .GetInputEvent = BrowsePageGetInputEvent,
    .Prepare		= BrowsePagePrepare,
};

/**
 * @Description: browse页面初始化函数，注册该结构体
 * @return 0
 */
int BrowsePageInit(void)
{
    return RegisterPageAction(&s_tBrowsePageAction);
}



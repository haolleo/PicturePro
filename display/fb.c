#include <include/config.h>
#include <include/disp_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>

static int s_FBFD;
static int s_ScreenSize;
static int s_LineWidth;
static int s_PixelWidth;
static struct fb_var_screeninfo s_tVar;
static struct fb_fix_screeninfo s_tFix;
static unsigned char *s_pFbmem;

static int FBDeviceInit(void);
static int FBShowPixel(int PenX, int PenY, unsigned int Color);
static int FBCleanScreen(unsigned int BackColor);
static int FBShowPage(PT_VideoMem ptVideoMem);

static T_DispOpr s_tFBDispOpr = {
	.name         = "fb",
	.DeviceInit   = FBDeviceInit,
	.ShowPixel    = FBShowPixel,
	.CleanScreen  = FBCleanScreen,
	.ShowPage	  = FBShowPage,
};

/* LCD设备初始化函数 */
static int FBDeviceInit(void)
{
	/* 打开设备：支持读写 */
	s_FBFD = open(FB_DEVICE_NAME, O_RDWR);
	write(s_FBFD,"\033[9;0]",8);//加入解决黑屏问题，原理还不清楚
	if (s_FBFD < 0) {
		DBG_PRINTF("can not open %s , err code :%d\n", FB_DEVICE_NAME, s_FBFD);
		return -1;
	}

	/* 获得可变信息 */
	if (ioctl(s_FBFD, FBIOGET_VSCREENINFO, &s_tVar)) {
		DBG_PRINTF("can not get s_tVar\n");
		return -1;
	}

	/* 获得固定信息 */
	if (ioctl(s_FBFD, FBIOGET_FSCREENINFO, &s_tFix)) {
		DBG_PRINTF("can not get s_tVar\n");
		return -1;
	}
	
	s_ScreenSize = s_tVar.xres * s_tVar.yres * s_tVar.bits_per_pixel / 8; // 屏幕总像素所占的字节数
	s_LineWidth   = s_tVar.xres * s_tVar.bits_per_pixel / 8;	// 每行像素所占的字节数
	s_PixelWidth  = s_tVar.bits_per_pixel / 8;	// 每个像素所占字节数

	s_tFBDispOpr.Xres 		= s_tVar.xres;
	s_tFBDispOpr.Yres 		= s_tVar.yres;
	s_tFBDispOpr.Bpp   		= s_tVar.bits_per_pixel;
	s_tFBDispOpr.LineWidth = s_LineWidth;
    DBG_PRINTF("xres %d yres %d \n",s_tFBDispOpr.Xres,s_tFBDispOpr.Yres);
	/* 直接映射到内存的Framebuffer */
	s_pFbmem = (unsigned char *)mmap(NULL, s_ScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, s_FBFD, 0);
	if (s_pFbmem == (unsigned char *)-1) {
		DBG_PRINTF("can not mmap\n");
		return -1;
	}
	s_tFBDispOpr.pDispMem = s_pFbmem;

	return 0;
}

/* LCD像素显示函数 */
static int FBShowPixel(int PenX, int PenY, unsigned int Color)
{
	unsigned char  *pPen8;
	unsigned short *pPen16;
	unsigned int   *pPen32;
	unsigned int	red, green, blue;
	
	if ((PenX >= s_tVar.xres) || (PenY >= s_tVar.yres))
	{
		DBG_PRINTF("out of region\n");
		return -1;
	}

	/* 该坐标在内存中对应像素的位置 */
	pPen8  = s_pFbmem + PenY * s_LineWidth + PenX * s_PixelWidth;
	pPen16 = (unsigned short *)pPen8;
	pPen32 = (unsigned int   *)pPen8;

	switch (s_tFBDispOpr.Bpp) {
	case 8:
		*pPen8 = (unsigned char)Color;
		break;
	case 16:
		/* RGB:565 */
		red      = ((Color >> 16) & 0xff) >> 3;
		green    = ((Color >> 8 )  & 0xff) >> 2;
		blue     = ((Color >> 0 )  & 0xff) >> 3;
		*pPen16  = (red << 11) | (green << 5) | blue;
		break;
	case 32:
		*pPen32 = Color;
		break;
	default:
		DBG_PRINTF("can not surport %d bpp\n", s_tFBDispOpr.Bpp);
		return -1;
	}

	return 0;
}

/* 清屏函数 */
static int FBCleanScreen(unsigned int BackColor)
{
	int i;
	unsigned char  *pPen8;
	unsigned short *pPen16;
	unsigned int   *pPen32;
	unsigned int	red, green, blue;

	pPen8  = s_pFbmem;
	pPen16 = (unsigned short *)pPen8;
	pPen32 = (unsigned int   *)pPen8;

	switch (s_tFBDispOpr.Bpp) {
	case 8:
		memset(pPen8, BackColor, s_ScreenSize);
		break;
	case 16:
		/* RGB:565 */
		red      = ((BackColor >> 16) & 0xffff) >> 3;
		green    = ((BackColor >> 8 )  & 0xffff) >> 2;
		blue     = ((BackColor >> 0 )  & 0xffff) >> 3;
		BackColor = (red << 11) | (green << 5) | blue;
		
		for (i = 0; i < s_ScreenSize;) {
			*pPen16 = BackColor;
			pPen16++;
			i += 2;
		}
		break;
	case 32:
		for (i = 0; i < s_ScreenSize;) {
			*pPen32 = BackColor;
			pPen32++;
			i += 4;
		}
		break;
	default:
		DBG_PRINTF("can not surport %dbpp\n", s_tFBDispOpr.Bpp);
		return -1;
	}

	return 0;
}

//针对界面显示，把ptVideoMem中像素数据复制到ptVideoMem,这个又是mmap到framebuffer中，所以可以直接显示
static int FBShowPage(PT_VideoMem ptVideoMem)
{
	memcpy(s_tFBDispOpr.pDispMem, ptVideoMem->tPixelDatas.PixelDatas, 
			ptVideoMem->tPixelDatas.TotalBytes);

	return 0;
}

int FBInit(void)
{
	return RegisterDispOpr(&s_tFBDispOpr);
}


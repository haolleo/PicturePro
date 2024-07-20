#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "include/config.h"
#include "include/page_manager.h"
#include "include/encoding_manager.h"
#include "include/fonts_manager.h"
#include "include/disp_manager.h"
#include "include/input_manager.h"
#include "include/debug_manager.h"
#include "include/pic_operation.h"
#include "include/picfmt_manager.h"
#include "include/render.h"

const char* kFontFile = "./MSYH.TTF"; //调试使用
int main(int argc, char **argv)
{
	int error;
	/* 初始化调试系统 */
	error = DebugInit();
	if (error) {
		printf(APP_ERR"DebugInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}
	
	error = InitDebugChanel();
	if (error) {
		printf(APP_ERR"InitDebugChanel error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

    if (argc != 1)
	{
		DBG_PRINTF("Usage:\n");
        DBG_PRINTF("%s <freetype_file>\n", argv[0]);
		return 0;
	}

	/* 初始化显示设备 */
	error = DisplayInit();
	if (error) {
		DebugPrint(APP_ERR"DisplayInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	/* 选择和初始默认的设备 */
	SelectAndInitDefaultDispDev("fb");
	
	/* 初始化输入子系统 */
	error = InputInit();
	if (error) {
		DebugPrint(APP_ERR"InputInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	/* 初始化输入设备 */
	error = AllInputDeviceInit();
	if (error) {
		DebugPrint(APP_ERR"AllInputDeviceInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	/* 初始化字符编码系统 */
	error = FontsInit();
	if (error)
	{
		DBG_PRINTF("FontsInit error!\n");
	}

	/* 初始化具体的字符编码文件 */
    error = SetFontsDetail("freetype", kFontFile, 24);
	if (error)
	{
		DBG_PRINTF("SetFontsDetail error!\n");
	}

	/* 分配页面内存 */
	error = AllocVideoMem(5);
	if (error) {
		DebugPrint(APP_ERR"AllocVideoMem error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	/* 初始化页面系统 */
	error = EncodingInit();
	if (error) {
		DebugPrint(APP_ERR"EncodingInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	ShowEncodingOpr();
	
	/* 初始化页面系统 */
	error = PagesInit();
	if (error) {
		DebugPrint(APP_ERR"PagesInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	
	/*  */
	error = PicFmtsInit();
	if (error) {
		DebugPrint(APP_ERR"PagesInit error! File:%s Line:%d\n", __FILE__, __LINE__);
		return -1;
	}

	//ShowPages();
	/* 显示主页面 */
	Page("main")->Run(NULL);
	return 0;
}


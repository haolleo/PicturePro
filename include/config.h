
#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include <include/debug_manager.h>

#define FB_DEVICE_NAME "/dev/fb0"
#define DEFAULT_DIR "/"

#define ICON_PATH 		"/etc/digitpic/icons"

#define COLOR_BACKGROUND   0xE7DBB5  /* ???? */
#define COLOR_FOREGROUND   0x514438  /* ???? */

//#define DBG_PRINTF(...)
// 由于引入了不同的DEBUG输入方式，因此要符合DEBUG格式
// <2> *** 2: debug level ***:内容
#define DBG_PRINTF DebugPrint

#endif /* _CONFIG_H */

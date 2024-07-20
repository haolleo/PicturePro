
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "include/config.h"
#include "include/debug_manager.h"

static int StdoutDebugPrint(char *strdata)
{
	/* 直接把输出信息用printf打印出来 */
	printf("%s", strdata);
	
	return strlen(strdata);
}

static T_DebugOpr s_tStdoutDebugOpr = {
	.name        = "stdout",
	.isUsed      = 1,
	.DebugPrint  = StdoutDebugPrint,
};

int StdoutInit(void)
{
	return RegisterDebugOpr(&s_tStdoutDebugOpr);
}

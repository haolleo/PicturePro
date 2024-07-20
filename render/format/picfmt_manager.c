

#include "include/config.h"
#include "include/pic_operation.h"
#include "include/picfmt_manager.h"
#include <string.h>

static PT_PicFileParser s_ptPicFileParserHead;

int RegisterPicFileParser(PT_PicFileParser ptPicFileParser)
{
	PT_PicFileParser ptTmp;

	if (!s_ptPicFileParserHead)
	{
		s_ptPicFileParserHead   = ptPicFileParser;
		ptPicFileParser->ptNext = NULL;
	}
	else
	{
		ptTmp = s_ptPicFileParserHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptPicFileParser;
		ptPicFileParser->ptNext = NULL;
	}

	return 0;
}

void ShowPicFmts(void)
{
	int i = 0;
	PT_PicFileParser ptTmp = s_ptPicFileParserHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

PT_PicFileParser Parser(char *pName)
{
	PT_PicFileParser ptTmp = s_ptPicFileParserHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

PT_PicFileParser GetParser(PT_FileMap ptFileMap)
{
	PT_PicFileParser ptTmp = s_ptPicFileParserHead;
	
	while (ptTmp)
	{
		if (ptTmp->isSupport(ptFileMap))
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}


int PicFmtsInit(void)
{
	int error;

	error = BMPParserInit();
	error |= JPGParserInit();
		
	return error;
}


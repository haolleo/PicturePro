
#ifndef _DRAW_H
#define _DRAW_H
int OpenTextFile(char *pFileName);
int SetFontsDetail(char *pHZKFile, char *pFileFreetype, unsigned int FontSize);
int SelectAndInitDisplay(char *pName);
int ShowNextPage(void);
int ShowPrePage(void);
int DrawInit(void);

#endif /* _DRAW_H */


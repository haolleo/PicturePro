
#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H
#include <sys/time.h>
#include <pthread.h>

#define INPUT_TYPE_STDIN 			0
#define INPUT_TYPE_TOUCHSCREEN 		1

#define INPUT_VALUE_UP 				0
#define INPUT_VALUE_DOWN 			1
#define INPUT_VALUE_EXIT 			2
#define INPUT_VALUE_UNKONW 		   -1

/* 输入事件信息结构体 */
typedef struct InputEvent {
    struct timeval time;
    int type;		/* stdin, touchscreen */
    int pressure;
    int x;
    int y;
    int key;
}T_InputEvent, *PT_InputEvent;

/* 输入事件结构体 */
typedef struct InputOpr {
    char *name;
    pthread_t threadId;
    int (*DeviceInit)(void);
    int (*DeviceExit)(void);
    int (*GetInputEvent)(PT_InputEvent ptInputEvent);
    struct InputOpr *ptNext;
}T_InputOpr, *PT_InputOpr;

int RegisterInputOpr(PT_InputOpr ptFontOpr);
int InputInit(void);
void ShowInputOpr(void);
int GetInputEvent(PT_InputEvent ptInputEvent);
int AllInputDeviceInit(void);
int StdinInit(void);
int TouchScreenInit(void);


#endif /* _INPUT_MANAGER_H */

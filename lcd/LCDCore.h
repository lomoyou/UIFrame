/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDCore.h
Author		: tanghong
Version	: 0.0.1
Date		: 2022/11/21
Description	: None
History	: LCD Display (1080*1920) Driver for APS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDCORE_H_
#define _LCDCORE_H_

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "LCDWindow.h"
#include "SDL2/SDL.h"
#include "driver.h"

/* Exported define -----------------------------------------------------------*/
/* LCD 事件类型定义                            */
#define LCD_EVENT_TIMEOUT                 0             // 页面超时
#define LCD_EVENT_TS_PRESS                1             // 用户点击按钮
#define LCD_EVENT_TS_RELEASE              2             // 用户释放按钮
#define LCD_EVENT_UPDATE_TIME             3             // 时间刷新
#define LCD_EVENT_UPDATE_DATE             4             // 日期刷新
#define LCD_EVENT_UPDATE_TIMEOUT          5             // 超时时间刷新

/* LCD 事件类型定义                            */
#define LCD_OBJ_HIDE                      0x00          // LCD 对象显示控制
#define LCD_OBJ_SHOW                      0x01          // LCD 对象显示控制
#define LCD_OBJ_DISABLE                   0x00          // LCD 对象(仅 BUTTON/KEYPAD )功能控制
#define LCD_OBJ_ENABLE                    0x02          // LCD 对象(仅 BUTTON/KEYPAD )功能控制


/* LCD 窗口切换事件类型 */
#define LCD_EVENT_SHOW                  0x01        // 窗口显示
#define LCD_EVENT_HIDE                  0x02        // 窗口隐藏
#define LCD_EVENT_RAISE                 0x03        // 窗口提升

/* Exported types ------------------------------------------------------------*/
/* LCD_EventCb_t
   LCD 底层事件回调函数
   输入参数 
   id 触发该事件的对象ID
       @event=LCD_EVENT_TIMEOUT    id 为页面ID
       @event=LCD_EVENT_TS_XXXX    id 为按键/键盘ID
   event 该事件类型 参考 LCD_EVENT_XXX
   ts 该事件发生的时间戳 精确至毫秒
   data 事件数据，使用完毕后需要释放资源
*/
typedef void ( *LCD_EventCb_t ) ( int id, int event, long long ts, void * data );


typedef struct                          // LCD 全局控制结构体
{
    SDL_Window * window;                // 全局SDL窗口
    SDL_Renderer * render;              // 全局SDL渲染器
    int xRel;                           // 窗口宽, 用于计算多窗口触摸位置
    int yRel;                           // 窗口高, 用于计算多窗口触摸位置
    pthread_mutex_t lock;               // 全局互斥锁

    sem_t sem;                          // 全局更新显示信号量
    char pgupdate;                      // 页更新标记
    char renew;							// 允许屏幕重绘标志
    int page;                           // 当前显示页面的编号
    int mask;							// 蒙版效果标志
    int able;                           // 触摸窗口控制 1 - 检测此窗口数据，0 - 不检测此窗口数据。
    int id;                             // 窗口ID
    LcdWin_t * win;                     // 窗口显示核心数据
    LCD_EventCb_t pEventCbFun;          // 回调函数
    pthread_t tTid;                           //
    pthread_t dTid;                           //
} LcdWindow_t ;




/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

int LCD_Init                    ( void                                 );
LcdWindow_t *LCD_CreateWindow   ( char *name, SDL_Rect rect            );
int LCD_DestroyWindow           ( LcdWindow_t *win                     );
int LCD_SetCallback    		    ( LcdWindow_t *win, LCD_EventCb_t cb   );
int LCD_SelectPage              ( LcdWindow_t *win, int id             );
int LCD_DisplayText             ( LcdWindow_t *win, int page, int id, char * val );
int LCD_DisplayImage            ( LcdWindow_t *win, int page, int id, char * file);
int LCD_DisplayQRCode           ( LcdWindow_t *win, int page, int id, char * val );
int LCD_DisplayVideo            ( LcdWindow_t *win, int page, int id, int w, int h, char *data, int size );
int LCD_DisplayButtonIcon       ( LcdWindow_t *win, int page, int id, char *value, char * file );
int LCD_ObjectControl           ( LcdWindow_t *win, int id, int ctrl   );
int LCD_RenewEnable   	        ( LcdWindow_t *win, int renew          );
int LCD_MaskEnable              ( LcdWindow_t *win, int mask           );
int LCD_WindowControl           ( LcdWindow_t *win, int show           );
int32_t LCD_WindowCheck         ( LcdWindow_t *win, int able           );
int32_t LCD_RaisaWindow         ( LcdWindow_t *win                     );
void SDLEventThread( void );

#endif /* _LCDCORE_H_ */
/******************************** END OF FILE *********************************/

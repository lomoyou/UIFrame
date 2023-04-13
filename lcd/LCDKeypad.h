/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDKeypad.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDKEYPAD_H_
#define _LCDKEYPAD_H_

/* Includes ------------------------------------------------------------------*/

#include "LCDObject.h"
#include "LCDButton.h"
#include "SDL2/SDL.h"

/* Exported define -----------------------------------------------------------*/

#define KEYPAD_PRESSED              1
#define KEYPAD_RELEASED             0

/* Exported types ------------------------------------------------------------*/

typedef struct                      // 键盘中按键布局
{
    const char * keyval;            // 键值字符串 
    SDL_Rect area;                  // 按键区域 

} LcdKeyLayout_t;

typedef struct                      // 键盘
{    
    LcdObject_t * obj;              // 关联的LCD对象
    char state;                     // 键盘状态
    SDL_Rect rect;                  // 键盘所在区域
    SDL_Texture * texture;          // 键盘纹理
    int activekey;                  // 被按下的按键
    int keycnt;                     // 键盘包含的按键数量
    LcdButton_t ** keys;            // 按键列表
    SDL_Renderer * renderer;        // 渲染器

} LcdKeypad_t;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdKeypad_t * CreateLcdKeypad ( SDL_Rect rect, char * icon, int keycnt, LcdKeyLayout_t * layout, SDL_Renderer * renderer );
int ChangeLcdKeypadState ( LcdKeypad_t * kp, int ackey );
int LcdKeypadPonitCheck ( LcdKeypad_t * kp, int x, int y );
int LcdKeypadControl ( LcdKeypad_t * kp, int show, int enable );

#endif /* _LCDKEYPAD_H_ */
/******************************** END OF FILE *********************************/

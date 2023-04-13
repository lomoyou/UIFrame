/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDButton.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDBUTTON_H_
#define _LCDBUTTON_H_

/* Includes ------------------------------------------------------------------*/

#include "LCDObject.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

/* Exported define -----------------------------------------------------------*/

#define KEY_PRESSED                 0
#define KEY_RELEASED                1

/* Exported types ------------------------------------------------------------*/

/* 自定义按钮数据 */ 
typedef struct {
    SDL_Color bColor;                 //  按钮前景色
    SDL_Color Color;                  //  按钮文本颜色
    int pt;                           //  按钮文本字体大小
    char align;                         // 显示文本对齐， 0 - 默认居中， 1 - 左对齐  2- 右对齐
    char content[128];                //  按钮文本内容。
} newButton_t;

typedef struct                      // 按钮定义       
{
    LcdObject_t * obj;              // 关联的LCD对象
    char value[32];                 // 按钮键值
    //void * data;                    // 按钮额外数据
    char state;                     // 按钮状态


    SDL_Rect rect;                  // 按钮所在区域
    SDL_Texture * texture;          // 按键当前显示的纹理
    SDL_Texture * init;             // 按钮默认纹理
    SDL_Texture * active;           // 按钮激活纹理
    SDL_Renderer * renderer;        // 渲染器

} LcdButton_t;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdButton_t * CreateLcdButton ( SDL_Rect rect, char * value, void * data, char * icon, SDL_Renderer * renderer );
int ChangeLcdButtonState ( LcdButton_t * btn, char state );
int UpdateLcdButton (LcdButton_t * btn, char *icon );
int LcdButtonPointCheck (LcdButton_t * btn, int x, int y );
int LcdButtonControl ( LcdButton_t * btn, int show, int enable );

#endif /* _LCDBUTTON_H_ */
/******************************** END OF FILE *********************************/

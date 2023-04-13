/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDText.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDTEXT_H_
#define _LCDTEXT_H_

/* Includes ------------------------------------------------------------------*/

#include "LCDObject.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

/* Exported define -----------------------------------------------------------*/

#define TEXT_ALIGN_LEFT             0
#define TEXT_ALIGN_CENTER           1
#define TEXT_ALIGN_RIGHT            2


#define TEXT_UPDATE_DISABLED 0
#define TEXT_UPDATE_ENABLED 1

/* Exported types ------------------------------------------------------------*/

typedef struct                      // 文本定义
{
    LcdObject_t * obj;              // 关联的LCD对象
    char align;                     // 对齐模式
    TTF_Font * font;                // 文本字体/样式
    SDL_Color color;                // 文本颜色
    SDL_Color bColor;               // 背景颜色
    SDL_Rect rect;                  // 文本显示区域
    SDL_Texture * texture;          // 文本字符串纹理
    SDL_Renderer * renderer;        // 渲染器

} LcdText_t;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdText_t * CreateLcdText ( char align, SDL_Rect rect, int pt, char * font, SDL_Color color, SDL_Renderer * renderer );
int UpdateLcdText ( LcdText_t * text, char * content, char opt );
int LcdTextControl ( LcdText_t * txt, int show );

#endif /* _LCDTEXT_H_ */
/******************************** END OF FILE *********************************/

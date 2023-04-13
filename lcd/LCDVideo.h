/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDVideo.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDVIDEO_H_
#define _LCDVIDEO_H_

/* Includes ------------------------------------------------------------------*/

#include "LCDObject.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

/* Exported define -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

typedef struct                      // 文本定义
{
    LcdObject_t * obj;              // 关联的LCD对象
    SDL_Rect rect;                  // 图像显示区域
    SDL_Texture * texture;          // 图像纹理
    SDL_Renderer * renderer;        // 渲染器
	SDL_Color color;				// 关灯颜色
} LcdVideo_t;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdVideo_t *CreateLcdVideo(SDL_Rect rect, SDL_Color color, SDL_Renderer *renderer);
int LcdVideoControl(LcdVideo_t *video, int show);
int UpdateLcdVideo(LcdVideo_t *video, int width, int height, char *data, int size);

#endif /* _LCDTEXT_H_ */
/******************************** END OF FILE *********************************/

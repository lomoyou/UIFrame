/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDImage.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDIMAGE_H_
#define _LCDIMAGE_H_

/* Includes ------------------------------------------------------------------*/

#include "LCDObject.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

/* Exported define -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct                      // 图像
{
    LcdObject_t * obj;              // 关联的LCD对象
    char defimg[256];               // 默认图片文件路径
    SDL_Rect rect;                  // 图片显示区域
    SDL_Texture * texture;          // 图片纹理
    SDL_Renderer * renderer;        // 渲染器

} LcdImage_t;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdImage_t * CreateLcdImage ( SDL_Rect rect, char * file, SDL_Renderer * renderer );
int UpdateLcdImage ( LcdImage_t * image, char * file );
int BindLcdImageToObject ( LcdImage_t * image, LcdObject_t * obj );
int LcdImageControl ( LcdImage_t * image, int show );

#endif /* _LCDIMAGE_H_ */
/******************************** END OF FILE *********************************/

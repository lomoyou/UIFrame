/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDQRCode.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDQRCODE_H_
#define _LCDQRCODE_H_

/* Includes ------------------------------------------------------------------*/

#include "LCDObject.h"
#include "SDL2/SDL.h"
#include "qrencode.h"

/* Exported define -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct                      // 二维码定义
{
    LcdObject_t * obj;              // 关联的LCD对象
    char version;                   // 二维码版本
                                    // 0 自动根据二维码文本生成
                                    // >0 指定二维码版本
    SDL_Rect rect;                  // 二维码显示区域
    SDL_Texture * texture;          // 二维码纹理
    SDL_Renderer * renderer;        // 渲染器

} LcdQRCode_t;
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdQRCode_t * CreateLcdQRCode ( char version, SDL_Rect rect, SDL_Renderer * renderer );
int UpdateLcdQRCode ( LcdQRCode_t * qr, char * content );
int LcdQRcodeControl ( LcdQRCode_t * qr, int show );

#endif /* _LCDQRCODE_H_ */
/******************************** END OF FILE *********************************/

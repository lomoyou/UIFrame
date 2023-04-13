/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDWindow.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDWINDOW_H_
#define _LCDWINDOW_H_

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "SDL2/SDL.h"
#include "LCDObject.h"

/* Exported types ------------------------------------------------------------*/

typedef struct                      // LCD窗口类型定义
{
    char name[64];                  // 名称

    int w;                          // 窗口宽度
    int h;                          // 窗口高度
    unsigned int lmap;              // 对象层位表
    unsigned int umap;              // 对象层更新位表
    sem_t * sem;                    // 该页对象更新信号量
    struct LcdObject * objs;        // 对象双向链表
    void * data;                    // 页面私有数据

    SDL_Texture * texture[8];      // 每个显示层的纹理
    SDL_Renderer * renderer;         // 渲染器

} LcdWin_t ;

/* Exported define -----------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdWin_t * CreateLcdWindow ( char * name, sem_t * sem, void * data, int w, int h, SDL_Renderer * renderer );
void LcdWindowAddObject ( LcdWin_t * win, LcdObject_t * obj );
void LcdWindowPresent ( LcdWin_t * win, int page, int option );

#endif /* _LCDWINDOW_H_ */
/******************************** END OF FILE *********************************/

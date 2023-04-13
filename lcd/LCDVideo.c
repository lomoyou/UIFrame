/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd.
FileName	: LCDText.c
Author		: tanghong
Version		: 0.1.0
Date		: 2019/8/26
Description	: None
History		: None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "LCDVideo.h"

/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: CreateLcdVideo
Description	: 创建一个video
Input		: rect - 显示区域
              color - 颜色
              renderer - 渲染器
Output		: None
Return		: NULL - 创建失败
Note		: None
------------------------------------------------------------------------------*/
LcdVideo_t * CreateLcdVideo ( SDL_Rect rect, SDL_Color color, SDL_Renderer * renderer )
{
    LcdVideo_t * video = NULL;

    video = (LcdVideo_t *)malloc(sizeof(LcdVideo_t));
    if( video == NULL )   return NULL;

    memset( video, 0x00, sizeof(LcdVideo_t) );

    video->rect = rect;
    video->color = color;
    video->renderer = renderer;

    return video;
}

/*-----------------------------------------------------------------------------
Function	: LcdVideoControl
Description	: 图像控制
Input		: video - 图像
              show - 文本否显示
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int  LcdVideoControl( LcdVideo_t * video, int show )
{
    // 参数检查
    if( video == NULL )           return -1;

    // 更改按键状态
    if( show ) video->obj->hide = 0;
    else video->obj->hide = 1;
    
    // 通知LCD对象状态已更新
    if( video->obj ) LcdObjectUpdate( video->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: UpdateLcdVideo
Description	: 更新video显示内容
Input		: video - video
              width - 宽度
              height - 高度
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int UpdateLcdVideo ( LcdVideo_t * video, int width, int height, char * data, int size)
{
    SDL_Texture * texture = NULL;
    SDL_Texture * target = NULL;
    SDL_Rect rect;
    SDL_Rect dRect;
    SDL_Surface *surface = NULL;

        // 参数检查
        if (video == NULL)
            return -1;

        // 视频帧纹理
        texture = SDL_CreateTexture(video->renderer,
                                    SDL_PIXELFORMAT_IYUV,   // YUV420P
                                    SDL_TEXTUREACCESS_STREAMING,
                                    width, height);
        rect.x = 0;
        rect.y = 0;
        rect.w = width;
        rect.h = height;

        // 将YUV数据填充到纹理上
        SDL_UpdateTexture(texture, &rect, data, size);

        target = SDL_CreateTexture(video->renderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_TARGET,
                                   video->rect.w, video->rect.h);
        SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);   // 设置为透明模式
        SDL_SetTextureBlendMode(target, 255);

        SDL_RenderClear( video->renderer );
        SDL_SetRenderTarget( video->renderer, target );

        // 裁剪视频帧
        if(video->rect.w >= width) {
            rect.x = 0;
            rect.w = width;
            dRect.x = (video->rect.w - width) / 2;
            dRect.w = width;

        } else {

            rect.x = (width - video->rect.w) / 2;
            rect.w = video->rect.w;

            dRect.x = 0;
            dRect.w = video->rect.w;
        }


        if(video->rect.h >= height) {
            rect.y = 0;
            rect.h = height;

            dRect.y = (video->rect.h - height) / 2;
            dRect.h = height;
        }else {
            rect.y = (height - video->rect.h) / 2;
            rect.h = video->rect.h;

            dRect.y = 0;
            dRect.h = video->rect.h;
        }


        SDL_RenderCopy( video->renderer, texture, &rect, &dRect);

        SDL_DestroyTexture(texture);

        SDL_SetRenderTarget(video->renderer, NULL);

        if (video->texture)  SDL_DestroyTexture(video->texture);
        video->texture = target;
     
    // 更改渲染器目标
    SDL_SetRenderTarget( video->renderer, NULL );

    // 对象状态已更新
    if( video->obj)    LcdObjectUpdate( video->obj );

    return 0;
}

/******************************* END OF FILE **********************************/

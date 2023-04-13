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
#include "LCDText.h"

/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: CreateLcdText
Description	: 创建一个文本
Input		: align - 对齐模式
              rect - 显示区域
              font - 字体
              color - 颜色
              renderer - 渲染器
Output		: None
Return		: NULL - 创建失败
Note		: None
------------------------------------------------------------------------------*/
LcdText_t * CreateLcdText ( char align, SDL_Rect rect, int pt, char * font, SDL_Color color, SDL_Renderer * renderer )
{
    LcdText_t * text = NULL;

    text = (LcdText_t *)malloc(sizeof(LcdText_t));
    if( text == NULL )   return NULL;

    memset( text, 0x00, sizeof(LcdText_t) );

    text->align = align;
    text->rect = rect;
    text->font = TTF_OpenFont( font, pt );
    text->color = color;
    text->renderer = renderer;

    return text;
}

/*-----------------------------------------------------------------------------
Function	: LcdTextControl
Description	: 控制文本
Input		: txt - 文本
              show - 文本是否显示
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int LcdTextControl ( LcdText_t * txt, int show )
{
    // 参数检查
    if( txt == NULL )           return -1;

    // 更改按键状态
    if( show ) txt->obj->hide = 0;
    else txt->obj->hide = 1;
    
    // 通知LCD对象状态已更新
    if( txt->obj ) LcdObjectUpdate( txt->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: UpdateLcdText
Description	: 更新文本显示内容
Input		: text - 文本
              content - 文本内容
              otp - 1 立即刷新显示
                    0 暂不刷新显示
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int UpdateLcdText ( LcdText_t * text, char * content, char opt )
{
    SDL_Texture * texture = NULL;
    SDL_Texture * target = NULL;
    SDL_Rect rect = { 0 };
    SDL_Surface * surface = NULL;

    // 参数检查
    if( text == NULL ) return -1;
    // 如果文本不为空则创建文本纹理
    if( content != NULL && strlen(content) )
    {
        // 生成文本纹理 自动换行实现 TTF_RenderUTF8_Blended_Wrapped
        surface = TTF_RenderUTF8_Blended( text->font, content, text->color );
        if (surface)
        {
            texture = SDL_CreateTextureFromSurface( text->renderer, surface );
            SDL_QueryTexture( texture, NULL, NULL, &rect.w, &rect.h  );
            SDL_FreeSurface(surface);
            if( rect.w > text->rect.w ) rect.w = text->rect.w;

            if( rect.h > text->rect.h )
            {
                rect.h = text->rect.h;
                rect.y = 0;
            }
            else
            {
                // 上下居中
                rect.y = text->rect.h/2 - rect.h/2;
            }

            // 修正文本显示的位置
            switch( text->align )
            {
                case TEXT_ALIGN_LEFT    :
                {
                    rect.x = 0;
                    //rect.y = 0;
                }break;
                case TEXT_ALIGN_CENTER  :
                {
                    rect.x = text->rect.w/2 - rect.w/2;
                    rect.y = 0;
                }break;
                case TEXT_ALIGN_RIGHT  :
                {
                    rect.x = text->rect.w - rect.w;
                    //rect.y = 0;
                }break;
                default :
                {
                    rect.x = 0;
                    //rect.y = 0;
                }break;
            }
        }
    }

    // 创建目标纹理
    target = SDL_CreateTexture( text->renderer,
                                SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_TARGET,
                                text->rect.w, 
                                text->rect.h );
    // 设置纹理混合模式
    SDL_SetTextureBlendMode( target, SDL_BLENDMODE_BLEND );
    // 清空渲染器并指向目标纹理
    SDL_RenderClear( text->renderer );
    SDL_SetRenderTarget( text->renderer, target );
    SDL_SetRenderDrawColor( text->renderer, 255, 255, 255, 0 );
    SDL_RenderFillRect( text->renderer, NULL );
    // 渲染文本纹理
    if( texture)
    {
        SDL_RenderCopy( text->renderer, texture, NULL, &rect );
        SDL_DestroyTexture( texture );
    }
    // 更改渲染器目标
    SDL_SetRenderTarget( text->renderer, NULL );
    // 更新纹理
    if( text->texture ) SDL_DestroyTexture( text->texture );
    text->texture = target;

    // 对象状态已更新
    if( text->obj && opt )    LcdObjectUpdate( text->obj );

    return 0;
}

/******************************* END OF FILE **********************************/

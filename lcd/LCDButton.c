/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd.
FileName	: LCDButton.c
Author		: tanghong
Version		: 0.1.0
Date		: 2019/8/26
Description	: None
History		: None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "LCDButton.h"
#include "SDL2/SDL_ttf.h"
/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static char * MicrosoftYaHei = (char*)"./fonts/ArialMT.ttf";
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: 

Description	: 创建一个按钮
Input		: rect - 按钮区域
              cls - 按钮类型
              value - 按钮键值
              data - 按钮附属数据
              icon - 按钮图标
              renderer - 渲染器
Output		: None
Return		: NULL - 创建失败
Note		: None
------------------------------------------------------------------------------*/
LcdButton_t * CreateLcdButton ( SDL_Rect rect, char * value, void * data, char * icon, SDL_Renderer * renderer )
{
    SDL_Texture * texture = NULL;
    SDL_Surface * surface = NULL;
    SDL_Texture * target = NULL;
    LcdButton_t * btn = NULL;
    SDL_Rect r = {0};

    btn = (LcdButton_t *)malloc(sizeof(LcdButton_t));
    if( btn == NULL )   return NULL;
    memset( btn, 0, sizeof(LcdButton_t) );

    btn->rect = rect;
    if (value) strcpy( btn->value, value );
   // btn->data = data;
    btn->renderer = renderer;

    if( icon != NULL &&  renderer != NULL )
    {
        // 生成图标纹理
        surface = IMG_Load( icon );
        if (surface)
        {
            texture = SDL_CreateTextureFromSurface( renderer, surface );
            SDL_FreeSurface( surface );
        }

        // 渲染默认显示纹理
        if( texture )
        {
            // 创建目标纹理
            target = SDL_CreateTexture( renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        rect.w, 
                                        rect.h );
            // 清空渲染器并指向目标纹理
            SDL_RenderClear( renderer );
            SDL_SetRenderTarget( renderer, target );
            SDL_RenderCopy( renderer, texture, NULL, NULL );
            // 更改渲染器目标
            SDL_SetRenderTarget( renderer, NULL );
            // 更新纹理
            SDL_SetTextureBlendMode( target, SDL_BLENDMODE_BLEND );
            btn->init = target;   
        }

        // 渲染按钮按下显示纹理
        if( texture )
        {
            // 创建目标纹理
            target = SDL_CreateTexture( renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        rect.w, 
                                        rect.h );
            // 清空渲染器并指向目标纹理
            SDL_RenderClear( renderer );
            SDL_SetRenderTarget( renderer, target );
            SDL_RenderCopy( renderer, texture, NULL, NULL );
            
            // 更改渲染器目标
            SDL_SetRenderTarget( renderer, NULL );
            // 更新纹理
            SDL_SetTextureBlendMode( target, SDL_BLENDMODE_ADD );
            SDL_SetTextureColorMod( target, 255, 255, 255 );
            btn->active = target;   
        }

        // 释放图标纹理
        if( texture )  SDL_DestroyTexture( texture );
        btn->texture = btn->init;
    }
    else if( icon == NULL && data != NULL && renderer != NULL) // 根据data生成按钮纹理
    {
        newButton_t *b = (newButton_t *)data;

        // 打开字体文件
        TTF_Font *font = TTF_OpenFont(MicrosoftYaHei, b->pt);

        if (font)
        {
            // 生成文本纹理，
            surface = TTF_RenderUTF8_Blended(font, b->content, b->Color);
            TTF_CloseFont(font);
        }


        if (surface)
        {
            texture = SDL_CreateTextureFromSurface(btn->renderer, surface);
            SDL_FreeSurface( surface );

            SDL_QueryTexture(texture, NULL, NULL, &r.w, &r.h);
            if (r.w > rect.w)
                r.w = rect.w;

            // 上下居中
            if (r.h > rect.h)
            {
                r.h = rect.h;
                r.y = 0;
            }
            else
            {
                r.y = rect.h / 2 - r.h / 2 ;
            }

            if (b->align == 1)
            {
                // 左对齐
                r.x = 30;
            }
            else if (b->align == 2)
            {
                // 右对齐
                r.x = rect.w - r.w - 30;
            }
            else
            {
                // 左右居中
                r.x = rect.w/2 - r.w/2;

            }
        }

        // 生成初始纹理
        if(texture != NULL)
        {

            // 创建目标纹理
            target = SDL_CreateTexture(btn->renderer,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    rect.w,
                                    rect.h);
            // 设置纹理混合模式
            SDL_RenderClear(btn->renderer);
            SDL_SetRenderTarget(btn->renderer, target);
            SDL_SetRenderDrawColor(btn->renderer, b->bColor.r, b->bColor.g, b->bColor.b, b->bColor.a);
            SDL_RenderFillRect(btn->renderer, NULL);


            SDL_RenderCopy(btn->renderer, texture, NULL, &r);
            //SDL_DestroyTexture(texture);

            // 更改渲染器目标
            SDL_SetRenderTarget(btn->renderer, NULL);
            // 更新纹理
            SDL_SetTextureBlendMode( target, SDL_BLENDMODE_BLEND );
            btn->init = target;
        }

        // 生成按键按下的纹理
        if (texture != NULL)
        {
            // 创建目标纹理
            target = SDL_CreateTexture(btn->renderer,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    rect.w,
                                    rect.h);
            // 设置纹理混合模式
            SDL_RenderClear(btn->renderer);
            SDL_SetRenderTarget(btn->renderer, target);
            SDL_SetRenderDrawColor(btn->renderer, 0x28, 0x28, 0x28, 0xFF);
            SDL_RenderFillRect(btn->renderer, NULL);

            SDL_RenderCopy(btn->renderer, texture, NULL, &r);
            SDL_DestroyTexture(texture);
            // 更改渲染器目标
            SDL_SetRenderTarget(btn->renderer, NULL);
            // 更新纹理
            SDL_SetTextureBlendMode( target, SDL_BLENDMODE_BLEND );

            btn->active = target;
        }
        // 释放图标纹理
        if( texture )  SDL_DestroyTexture( texture );
        btn->texture = btn->init;
    }

    return btn;
}

/*-----------------------------------------------------------------------------
Function	: LcdButtonControl
Description	: 控制按钮
Input		: btn - 按钮
              show - 按钮是否显示
              enable - 按钮是否接受输入
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int LcdButtonControl ( LcdButton_t * btn, int show, int enable )
{
    // 参数检查
    if( btn == NULL )           return -1;

    // 更改按键状态
    if( show ) btn->obj->hide = 0;
    else btn->obj->hide = 1;
    if( enable ) btn->obj->disable = 0;
    else btn->obj->disable = 1;
    
    // 通知LCD对象状态已更新
    if( btn->obj ) LcdObjectUpdate( btn->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: ChangeLcdButtonState
Description	: 改变按钮状态
Input		: btn - 按钮
              state - 状态
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int ChangeLcdButtonState ( LcdButton_t * btn, char state )
{
    // 参数检查
    if( btn == NULL )           return -1;
    if( state != KEY_RELEASED &&
        state != KEY_PRESSED )  return -1;

    // 更改按钮状态并切换显示纹理
    btn->state = state;
    if( state == KEY_PRESSED )  btn->texture = btn->active; 
    else btn->texture = btn->init;

    // 通知LCD对象状态已更新
    if( btn->obj ) LcdObjectUpdate( btn->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: UpdateLcdButton
Description	: 更新按钮纹理
Input		: btn - 按钮
              icon - 图标文件
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int UpdateLcdButton (LcdButton_t * btn,  char *icon)
{
    SDL_Texture * texture = NULL;
    SDL_Surface * surface = NULL;
    SDL_Texture * target = NULL;
    SDL_Rect rect = {0};

    if (!btn)  return -1;

    // 生成图标纹理
    if (icon != NULL && strlen(icon))
    {
        surface = IMG_Load(icon );
        if (surface)
        {
            texture = SDL_CreateTextureFromSurface( btn->renderer, surface );
            SDL_FreeSurface( surface );
        }
    }

    // 创建的激活纹理，否则使用默认纹理
    if (texture)
    {
        SDL_QueryTexture( texture, NULL, NULL, &rect.w, &rect.h );
        target = SDL_CreateTexture( btn->renderer, 
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    rect.w,
                                    rect.h );
        // 清空渲染器并指向目标纹理
        SDL_RenderClear( btn->renderer );
        SDL_SetRenderTarget( btn->renderer, target );
        SDL_RenderCopy( btn->renderer, texture, NULL, NULL );
        
        // 更改渲染器目标
        SDL_SetRenderTarget( btn->renderer, NULL );
        // 更新纹理
        SDL_SetTextureBlendMode( target, SDL_BLENDMODE_ADD );
        SDL_SetTextureColorMod( target, 255, 255, 255 );

        if (btn->active) SDL_DestroyTexture( btn->active);
        btn->active = target;  

        if (btn->texture) SDL_DestroyTexture( btn->texture);
        btn->texture = texture;
        btn->init = btn->texture;
    }

    // 通知LCD对象状态已更新
    if( btn->obj ) LcdObjectUpdate( btn->obj );
}
/*-----------------------------------------------------------------------------
Function	: LcdButtonPointCheck
Description	: 检测指定坐标值在该按钮中
Input		: btn - 按钮
              x - X坐标
              y - Y坐标
Output		: None
Return		: 0 - miss
              1 - hit
              -1 - error
Note		: None
------------------------------------------------------------------------------*/
int LcdButtonPointCheck (LcdButton_t * btn, int x, int y )
{
    if( !btn )          return -1;

    if( x >= btn->rect.x &&
        x <= btn->rect.x + btn->rect.w &&
        y >= btn->rect.y &&
        y <= btn->rect.y + btn->rect.h  )   return 1;
    else return 0;
}

/******************************* END OF FILE **********************************/
/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd.
FileName	: LCDKeypad.c
Author		: tanghong
Version		: 0.1.0
Date		: 2019/8/26
Description	: None
History		: None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "LCDButton.h"
#include "LCDKeypad.h"

/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: CreateLcdKeypad
Description	: 创建一个键盘
Input		: rect - 键盘区域 - 实际只取 x, y 值 确定键盘放置位置 键盘高度/宽度均由图标确定
              icon - 键盘图标
              keycnt - 按键数量
              layout - 按键布局数据
              renderer - 渲染器
Output		: None
Return		: NULL - 创建失败
Note		: None
------------------------------------------------------------------------------*/
LcdKeypad_t * CreateLcdKeypad ( SDL_Rect rect, char * icon, int keycnt, LcdKeyLayout_t * layout, SDL_Renderer * renderer )
{
    SDL_Texture * texture = NULL;
    SDL_Surface * surface = NULL;
    SDL_Texture * target = NULL;
    LcdKeypad_t * kp = NULL;
    SDL_Rect nrect = { 0 };
    int idx = 0;

    kp = (LcdKeypad_t *)malloc(sizeof(LcdKeypad_t));
    if( kp == NULL )    return NULL;
    memset( kp, 0, sizeof(LcdKeypad_t) );

    if( !layout )       return NULL;    
    if( !renderer )     return NULL;   

    nrect.x = rect.x;
    nrect.y = rect.y;
    kp->keycnt = keycnt;
    kp->renderer = renderer;

    // 创建键盘纹理
    if( icon != NULL )
    {
        // 生成图标纹理
        surface = IMG_Load( icon );
        if (surface)
        {
            texture = SDL_CreateTextureFromSurface( renderer, surface );
            SDL_FreeSurface( surface );
            SDL_QueryTexture( texture, NULL, NULL, &nrect.w, &nrect.h );

            // 渲染默认显示纹理
            if( texture )
            {
                // 创建目标纹理
                target = SDL_CreateTexture( renderer,
                                            SDL_PIXELFORMAT_RGBA8888,
                                            SDL_TEXTUREACCESS_TARGET,
                                            nrect.w,
                                            nrect.h );
                // 设置纹理混合模式
                SDL_SetTextureBlendMode( target, SDL_BLENDMODE_BLEND );
                // 清空渲染器并指向目标纹理
                SDL_RenderClear( renderer );
                SDL_SetRenderTarget( renderer, target );
                SDL_RenderCopy( renderer, texture, NULL, NULL );
                // 更改渲染器目标
                SDL_SetRenderTarget( renderer, NULL );
                // 更新纹理
                kp->texture = target;
                kp->rect = nrect;
                SDL_DestroyTexture( texture );
            }
            else
            {
                goto err1;
            }
        }
    }
    else
    {
        goto err1;
    }

    // 创建每一个按键
    kp->keys = ( LcdButton_t ** )malloc( sizeof(LcdButton_t *)*keycnt );
    if( kp->keys == NULL ) goto err1;
    else memset( kp->keys, 0, sizeof(LcdButton_t *)*keycnt );
    
    for( idx = 0; idx < keycnt; idx ++ )
    {
        kp->keys[idx] = CreateLcdButton( layout[idx].area, (char *)layout[idx].keyval, NULL, NULL, NULL );
        if( kp->keys[idx] == NULL )
        {
            goto err2;
        }
        else
        {
            // 创建按键被按下的纹理
            target = SDL_CreateTexture( renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        kp->keys[idx]->rect.w, 
                                        kp->keys[idx]->rect.h );
            // 设置纹理混合模式
            SDL_SetTextureBlendMode( target, SDL_BLENDMODE_NONE );
            // 清空渲染器并指向目标纹理
            SDL_RenderClear( renderer );
            SDL_SetRenderTarget( renderer, target );
            SDL_RenderCopy( renderer, kp->texture, &kp->keys[idx]->rect, NULL );
            // 更改渲染器目标
            SDL_SetRenderTarget( renderer, NULL );
            // 更新纹理
            SDL_SetTextureBlendMode( target, SDL_BLENDMODE_ADD );
            SDL_SetTextureColorMod( target, 255, 255, 255 );
            kp->keys[idx]->texture = target;   
        }

        kp->keys[idx]->rect.x += kp->rect.x;
        kp->keys[idx]->rect.y += kp->rect.y;
    }

    return kp;

  err2:
    for( int i = 0; i < idx; i ++ )
        free( kp->keys[i] );
    free( kp->keys );
  err1:
    free(kp);
    return NULL;
}

/*-----------------------------------------------------------------------------
Function	: LcdKeypadControl
Description	: 控制键盘
Input		: kp - 键盘
              show - 键盘是否显示
              enable - 键盘是否接受输入
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int LcdKeypadControl ( LcdKeypad_t * kp, int show, int enable )
{
    // 参数检查
    if( kp == NULL )           return -1;

    // 更改键盘状态
    if( show ) kp->obj->hide = 0;
    else kp->obj->hide = 1;
    if( enable ) kp->obj->disable = 0;
    else kp->obj->disable = 1;
    
    // 通知LCD对象状态已更新
    if( kp->obj ) LcdObjectUpdate( kp->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: ChangeLcdKeypadState
Description	: 改变按钮状态
Input		: kp - 键盘
              key - 被按下的按键序号 -1 表示没有按键被按下
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int ChangeLcdKeypadState ( LcdKeypad_t * kp, int key )
{
    // 参数检查
    if( kp == NULL )           return -1;

    // 更改键盘状态
    if( key >= 0 ) kp->state = KEYPAD_PRESSED;
    else kp->state = KEYPAD_RELEASED;
    kp->activekey = key;
    
    // 通知LCD对象状态已更新
    if( kp->obj ) LcdObjectUpdate( kp->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LcdKeypadPonitCheck 
Description	: 检测指定坐标值在该键盘中
Input		: kp - 键盘
              x - X坐标
              y - Y坐标
Output		: None
Return		: >= 0  被命中的按键序号
              -1 - 没有按键被命中
Note		: None
------------------------------------------------------------------------------*/
int LcdKeypadPonitCheck ( LcdKeypad_t * kp, int x, int y )
{
    int idx = 0;
    if( kp == NULL )        return -1;

    for( idx = 0; idx < kp->keycnt; idx ++ ){
        if(LcdButtonPointCheck(kp->keys[idx], x, y) )
            break;
    }


    if( idx < kp->keycnt )  return idx;
    else return -1;
}

/******************************* END OF FILE **********************************/
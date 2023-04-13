/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd.
FileName	: LCDPage.c
Author		: tanghong
Version		: 0.1.0
Date		: 2019/8/26
Description	: None
History		: None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "SDL2/SDL.h"

#include "LCDWindow.h"
#include "LCDObject.h"
#include "LCDText.h"
#include "LCDQRCode.h"
#include "LCDImage.h"
#include "LCDButton.h"
#include "LCDKeypad.h"
#include "LCDVideo.h"
#include "logging.h"
#include "lcd.h"

/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: CreateLcdWindow
Description	: 创建一个LCD显示窗口
Input		: name - 页名称
              sem - 窗口更新信号量
              data - 窗口私有数据
              w - width
              h - heigth
Output		: None
Return		: NULL - 创建窗口失败
Note		: None
------------------------------------------------------------------------------*/
LcdWin_t * CreateLcdWindow ( char * name, sem_t * sem, void * data, int w, int h, SDL_Renderer * renderer )
{
    LcdWin_t * win = NULL;

    // 分配LCD对象空间
    win = (LcdWin_t *)malloc(sizeof(LcdWin_t));
    if( win == NULL )               return NULL;
    memset( win, 0x00, sizeof(LcdWin_t) );

    // 初始化LCD对象
    strcpy( win->name, name );
    win->lmap = 0;
    win->umap = 0;
    win->sem = sem;
    win->data = data;
    win->objs = CreateLcdObject( (char*)"LISTSTART", -1, 0, 0, NULL );
    win->renderer = renderer;
    win->objs->next = NULL;
    win->objs->prev = NULL;
    win->w = w;
    win->h = h;

    // 每一层都生成一个页面纹理
    for( int idx = 0; idx < 8; idx ++ )
    {
        win->texture[idx] = SDL_CreateTexture( renderer,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        w,
                                        h );
    }
    
    return win;
}

/*-----------------------------------------------------------------------------
Function	: LcdWindowAddObject
Description	: 添加一个对象
Input		: window - 窗口
              obj - 对象
Output		: None
Return		: None
Note		: None
------------------------------------------------------------------------------*/
void LcdWindowAddObject ( LcdWin_t * win, LcdObject_t * obj )
{
    struct LcdObject * o = NULL;

    if( win != NULL && obj != NULL )
    {
        o = win->objs;
        while( o && o->next != NULL ) o = o->next;

        o->next = obj;
        obj->prev = o;
        obj->next = NULL;
        obj->win = (void *)win;
        win->lmap |= 1 << obj->layer;
    }
}

/*-----------------------------------------------------------------------------
Function	: LcdWindowPresent
Description	: 将页面显示到LCD屏幕上
Input		: window - 窗口
              page - 页
              option - 选项 bit0 0 全部重绘 1 自动重绘
							bit1 0 正常显示 1 蒙板效果
Output		: None
Return		: None
Note		: None
------------------------------------------------------------------------------*/
void LcdWindowPresent ( LcdWin_t * win, int page, int option )
{
    struct LcdObject * o = NULL;
    SDL_Renderer * render = NULL;
    int layer = 0; 
    int top = -1;
    int prev = 0;   
    char start = 0;

    if( win == NULL )    return;
    if( option&0x00000001 )     win->umap |= 0x00000001;

    render = win->renderer;
    for( layer = 0; layer < 8; layer ++ )
    {
        if( win->lmap & ( 1 << layer ))
        {
            //plog(LOG_LEVEL_INFO, "layer valid:%#x\n",layer);
            prev = top;
            top = layer;
            if( start || win->umap & ( 1 << layer ) )
            {
               // plog(LOG_LEVEL_INFO,"layer need change : %d\n",layer);
                start = 1;
                win->umap &= ~(1 << layer);
               // SDL_RenderClear( render );
                SDL_SetRenderTarget( render, win->texture[top] );
                if( prev >= 0 ) SDL_RenderCopy( render, win->texture[prev], NULL, NULL );

                o = win->objs;
                while (o){
                    o = o->next;
                   //if( o && o->layer == layer && !o->disable && ( o->page == page || o->page == 0xFF ))
                    if( o && o->layer == layer && ( o->type == OBJ_TYPE_BUTTON || !o->disable) &&( o->page == page || o->page == 0xFF ))
                    {
                        o->update = 0;
                       // if (o->page == LCD_PAGE_SMALL) plog(LOG_LEVEL_INFO, "obj change : %08x\n", o->id );

                        switch ( o->type )
                        {
                            case OBJ_TYPE_TEXT      :
                            {
                                LcdText_t * text = (LcdText_t *)o->data;
                                if( !o->hide )
                                    if (text && text->texture) SDL_RenderCopy( render, text->texture, NULL, &text->rect );
                            } break;
                            case OBJ_TYPE_QRCODE    :
                            {
                                LcdQRCode_t * qr = (LcdQRCode_t *)o->data;
                                if( !o->hide ) 
                                    if (qr && qr->texture ) SDL_RenderCopy( render, qr->texture, NULL, &qr->rect );
                            } break;
                            case OBJ_TYPE_IMAGE     :
                            {
                                LcdImage_t * img = (LcdImage_t *)o->data;
                                if( !o->hide ) 
                                	if (img && img->texture) SDL_RenderCopy( render, img->texture, NULL, &img->rect );
                            } break;
                            case OBJ_TYPE_BUTTON    :
                            {
                                LcdButton_t * btn = (LcdButton_t *)o->data;
                                if (!o->hide)   // 未隐藏
                                {
                                    if(o->disable) // 未失能
                                    {
                                        // 按钮失能，显示按下状态
                                        if (btn && btn->active) SDL_RenderCopy( render, btn->active, NULL, &btn->rect );
                                    }
                                    else
                                    {
                                        if (btn && btn->texture) SDL_RenderCopy( render, btn->texture, NULL, &btn->rect );
                                    }
                                }
                            } break;
                            case OBJ_TYPE_KEYPAD    :
                            {   
                                LcdKeypad_t * kp = (LcdKeypad_t *)o->data;
                                if (kp && !o->disable) SDL_RenderCopy( render, kp->texture, NULL, &kp->rect );
                                if( kp->state == KEYPAD_PRESSED )
                                {
                                    LcdButton_t * key = kp->keys[kp->activekey];
                                    if (key && key->texture && !o->disable) SDL_RenderCopy( render, key->texture, NULL, &key->rect );
                                }
 
                            } break;
                            case OBJ_TYPE_VIDEO     :
                            {
                                LcdVideo_t *v = (LcdVideo_t *)o->data;
                                if (v && v->texture) SDL_RenderCopy( render, v->texture, NULL, &v->rect );
                            }break;
                            default : break;
                        }
                    }
                }

                //SDL_RenderClear( render );
                SDL_SetRenderTarget( render, NULL );
            }
        } 
    }

	SDL_Texture * target = NULL;
	if( option & 0x00000002 )
	{

		SDL_SetTextureBlendMode( win->texture[top], SDL_BLENDMODE_ADD );
		
		// 创建目标纹理
		target = SDL_CreateTexture( render,
									SDL_PIXELFORMAT_RGBA8888,
									SDL_TEXTUREACCESS_TARGET,
									win->w,
									win->h );
		// 清空渲染器并指向目标纹理
		SDL_RenderClear( render );
		SDL_SetRenderTarget( render, target );
		SDL_SetRenderDrawColor( render, 128, 128, 128, 128 );
		SDL_RenderFillRect( render, NULL );
		SDL_RenderCopy( render, win->texture[top], NULL, NULL );
		
		// 更改渲染器目标
		SDL_SetRenderTarget( render, NULL );
	}
	else
	{
		SDL_SetTextureBlendMode( win->texture[top], SDL_BLENDMODE_NONE );
		target = win->texture[top];
	}

//	SDL_SetRenderTarget( render, NULL );
//	SDL_RenderClear( render );
	SDL_Rect rect = { 0 };
    rect.w = win->w;
    rect.h = win->h;
	SDL_RenderCopy( render, target, NULL, &rect );
    SDL_RenderPresent( render );
	if(  option & 0x00000002  ) SDL_DestroyTexture( target );
}

/******************************* END OF FILE **********************************/

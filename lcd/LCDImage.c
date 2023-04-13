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

#include "LCDImage.h"
#include "LCDObject.h"

/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: CreateLcdImage
Description	: 创建一个图像
Input		: rect - 图像显示区域
              renderer - 渲染器
Output		: None
Return		: NULL - 创建失败
Note		: None
------------------------------------------------------------------------------*/
LcdImage_t * CreateLcdImage ( SDL_Rect rect, char * file, SDL_Renderer * renderer )
{
    LcdImage_t * img = NULL;

    img = (LcdImage_t *)malloc(sizeof(LcdImage_t));
    if( img == NULL )   return NULL;

    memset( img, 0x00, sizeof(LcdImage_t) );
    strcpy( img->defimg, file );
    img->rect = rect;
    img->renderer = renderer;

    return img;
}

/*-----------------------------------------------------------------------------
Function	: LcdImageControl
Description	: 控制图片
Input		: img - 图片
              show - 图片是否显示
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int LcdImageControl ( LcdImage_t * image, int show )
{
    // 参数检查
    if( image == NULL )           return -1;

    // 更改按键状态
    if( show ) image->obj->hide = 0;
    else image->obj->hide = 1;
    
    // 通知LCD对象状态已更新
    if( image->obj ) LcdObjectUpdate( image->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: UpdateLcdImage
Description	: 更新图片显示内容
Input		: remote - 图片对象
              content - 二维码内容
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int UpdateLcdImage ( LcdImage_t * image, char * file )
{
    SDL_Texture * texture = NULL;
    SDL_Surface * surface = NULL;
    SDL_Rect rect = { 0 };

    // 参数检查
    if( image == NULL ) return -1;

    // 如果图片文件路径存在则创建图像纹理
    if( file != NULL && strlen(file) )
    {
        // 生成图片纹理
        surface = IMG_Load( file );
        if( surface )
        {
            texture = SDL_CreateTextureFromSurface( image->renderer, surface );
            SDL_FreeSurface( surface );
        } 
    }

    // 如果图像纹理创建失败(大概率是传入参数的文件不存在) 则创建默认的图像纹理
    if( !texture )
    {
        // 生成图片纹理
        surface = IMG_Load( image->defimg );
        if( surface )
        {
            texture = SDL_CreateTextureFromSurface( image->renderer, surface );
            SDL_FreeSurface( surface );
        } 

        // TODO: 转换格式 
        // SDL_ConvertSurfaceFormat(surface, )
    }

    // 更新纹理
    if( image->texture ) SDL_DestroyTexture( image->texture );
    image->texture = texture;

    // 对象状态已更新
    if( image->obj )    LcdObjectUpdate( image->obj );

    return 0;
}

/******************************* END OF FILE **********************************/

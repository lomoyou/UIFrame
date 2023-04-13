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
#include "LCDQRCode.h"

/* Private types -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: CreateLcdQRCode
Description	: 创建一个二维码
Input		: version - 二维码版本和二维码黑白块数量相关
                        0 自动选择合适版本
              rect - 二维码显示区域
Output		: None
Return		: NULL - 创建失败
Note		: None
------------------------------------------------------------------------------*/
LcdQRCode_t * CreateLcdQRCode ( char version, SDL_Rect rect, SDL_Renderer * renderer )
{
    LcdQRCode_t * qr = NULL;

    qr = (LcdQRCode_t *)malloc(sizeof(LcdQRCode_t));
    if( qr == NULL )   return NULL;

    memset( qr, 0x00, sizeof(LcdQRCode_t) );
    qr->rect = rect;
    qr->renderer = renderer;

    return qr;
}

/*-----------------------------------------------------------------------------
Function	: LcdQRcodeControl
Description	: 控制图片
Input		: qr - 二维码
              show - 是否显示
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int LcdQRcodeControl ( LcdQRCode_t * qr, int show )
{
    // 参数检查
    if( qr == NULL )           return -1;

    // 更改按键状态
    if( show ) qr->obj->hide = 0;
    else qr->obj->hide = 1;
    
    // 通知LCD对象状态已更新
    if( qr->obj ) LcdObjectUpdate( qr->obj );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: UpdateLcdQRCode
Description	: 更新二维码显示内容
Input		: qr - 二维码
              content - 二维码内容
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int UpdateLcdQRCode ( LcdQRCode_t * qr, char * content )
{
    SDL_Texture * texture = NULL;
    SDL_Texture * target = NULL;
    SDL_Rect rect = { 0 };
    QRcode * pQRC = NULL;
    

    // 参数检查
    if( qr == NULL ) return -1;

    // 如果二维码不为空则创建文本纹理
    if( content != NULL && strlen(content) )
    {
        pQRC = QRcode_encodeString( content, qr->version, QR_ECLEVEL_Q, QR_MODE_8, 1 );
        if( pQRC == NULL )
        {
        //    plog( LOG_LEVEL_ERROR, "QRcode_encodeString error\n");
            return -1;
        } 
        else
        {
            int dotWidth, pixWidth, bufferLineWidth, i, j;
            unsigned int qrdot = 0;
            unsigned char *  pRGBData, *pSourceData, *pDestData;

            // 每一个黑白块占用像素宽度
            qrdot = (qr->rect.w*9/10 + ( pQRC->width/2 )) / (pQRC->width);
        //    printf("QR Version : %d, Width %d, Dotsize %d\n", pQRC->version, pQRC->width, qrdot );

            // 横纵有多少个黑白块
            dotWidth = pQRC->width;
            // 横纵有多少个像素点
            pixWidth = pQRC->width*qrdot;
            bufferLineWidth = pixWidth*sizeof(SDL_Color);
            
            // 分配ARGB内存并填充全白色
            pRGBData = (unsigned char *)malloc(bufferLineWidth * pixWidth);
            memset( pRGBData, 0xFF, bufferLineWidth * pixWidth );  

            // 解析图像
            pSourceData = pQRC->data;
            for( i = 0; i < dotWidth; i ++ )
            {                               
                pDestData = pRGBData + bufferLineWidth*i*qrdot;
                for( j = 0; j < dotWidth; j ++ )
                {
                    if ( *pSourceData & 0x01 )
                    {
                        // 将 dot 转换为 qrdot*qrdot 的黑块
                        for (unsigned int m = 0; m < qrdot; m ++)
                        {
                            for ( unsigned int n = 0; n < qrdot; n++)
                            {
                                *(pDestData + 0 + n*sizeof(SDL_Color) + bufferLineWidth*m) = 0x00;
                                *(pDestData + 1 + n*sizeof(SDL_Color) + bufferLineWidth*m) = 0x00;
                                *(pDestData + 2 + n*sizeof(SDL_Color) + bufferLineWidth*m) = 0x00;
                                *(pDestData + 3 + n*sizeof(SDL_Color) + bufferLineWidth*m) = 0x00;
                            }
                        }
                    }
                    
                    pDestData += sizeof(SDL_Color)*qrdot;
                    pSourceData++;
                }
            }

            // 释放 
            QRcode_free( pQRC );

            // 生成二维码图像纹理
            texture = SDL_CreateTexture( qr->renderer, 
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STATIC,
                                         pixWidth, 
                                         pixWidth );
            SDL_UpdateTexture( texture, NULL, pRGBData, bufferLineWidth );
            free( pRGBData );

            // 生成二维码显示区域    
            if ( pixWidth > qr->rect.w ) pixWidth = qr->rect.w;                      
            rect.x = qr->rect.w/2 - pixWidth/2;
            rect.y = qr->rect.h/2 - pixWidth/2;
            rect.w = pixWidth;
            rect.h = pixWidth;
        }
    }

    // 创建目标纹理
    target = SDL_CreateTexture( qr->renderer,
                                SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_TARGET,
                                qr->rect.w, 
                                qr->rect.h );

    // 清空渲染器并指向目标纹理
    SDL_RenderClear( qr->renderer );
    SDL_SetRenderTarget( qr->renderer, target );
    SDL_SetRenderDrawColor( qr->renderer, 255, 255, 255, 255 );
    SDL_RenderFillRect( qr->renderer, NULL );

    // 渲染文本纹理
    if( texture != NULL )
    {
        SDL_RenderCopy( qr->renderer, texture, NULL, &rect );
        SDL_DestroyTexture( texture );
    }
        
    // 更改渲染器目标
    SDL_SetRenderTarget( qr->renderer, NULL );

    // 更新纹理
    if( qr->texture ) SDL_DestroyTexture( qr->texture );
    qr->texture = target;

    // 通知LCD对象状态已更新
    if( qr->obj ) LcdObjectUpdate( qr->obj );

    return 0;
}

/******************************* END OF FILE **********************************/

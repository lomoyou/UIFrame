/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd.
FileName	: LCDObject.c
Author		: tanghong
Version		: 0.1.0
Date		: 2019/8/26
Description	: None
History		: None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "LCDWindow.h"
#include "LCDObject.h"
#include "LCDText.h"
#include "LCDQRCode.h"
#include "LCDImage.h"
#include "LCDButton.h"
#include "LCDKeypad.h"
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
Function	: CreateLcdObject
Description	: 创建一个LCD对象
Input		: name - 对象名称
              id - 对象ID
              type - 对象类型
              layer - 对象所属层
              data - 对象数据
Output		: None
Return		: NULL - 创建对象失败
Note		: None
------------------------------------------------------------------------------*/
LcdObject_t * CreateLcdObject ( char * name, unsigned int id, char type, char layer, void * data )
{
    LcdObject_t * obj = NULL;

    // 输入参数检查
    if( type > OBJ_TYPE_KEYPAD )    return NULL;
    if( layer >= 8 )                return NULL;

    // 分配LCD对象空间
    obj = (LcdObject_t *)malloc(sizeof(LcdObject_t));
    if( obj == NULL )               return NULL;
    memset( obj, 0x00, sizeof(LcdObject_t) );

    // 初始化LCD对象
    strcpy( obj->name, name );
    obj->id = id;
    obj->type = type;
    obj->layer = layer;
    obj->data = data;
    obj->page = ( id >> 16 ) & 0x00FF;

    if( data != NULL )
    switch ( type )
    {
        case OBJ_TYPE_TEXT      :
        {
            LcdText_t * txt = ( LcdText_t * )data;
            txt->obj = obj;
        }break;
        case OBJ_TYPE_IMAGE     :
        {
            LcdImage_t * img = ( LcdImage_t * )data;
            img->obj = obj;
        }break;
        case OBJ_TYPE_QRCODE    :
        {
            LcdQRCode_t * qr = ( LcdQRCode_t * )data;
            qr->obj = obj;
        }break;
        case OBJ_TYPE_BUTTON    :
        {
            LcdButton_t * bt = ( LcdButton_t * )data;
            bt->obj = obj;
        }break;
        case OBJ_TYPE_KEYPAD    :
        {
            LcdKeypad_t * kp = ( LcdKeypad_t * )data;
            kp->obj = obj;
        }break;
        case OBJ_TYPE_VIDEO     :
        {
            LcdVideo_t * vd = ( LcdVideo_t * )data;
            vd->obj = obj;
        }break;
        default : break;
    }

    return obj;
}

/*-----------------------------------------------------------------------------
Function	: LcdObjectUpdate
Description	: 通知页面该对象发生更新
Input		: obj - 对象
Output		: None
Return		: None
Note		: None
------------------------------------------------------------------------------*/
void LcdObjectUpdate ( LcdObject_t * obj )
{
    LcdWin_t * win = NULL;

    if( obj )
    {
        obj->update = 1;
        if( obj->win ) 
        {
            win = (LcdWin_t *)obj->win;
            win->umap |= ( 1 << obj->layer );
            sem_post( win->sem );
        }
    }
}

/******************************* END OF FILE **********************************/

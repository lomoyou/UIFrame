/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd. 
FileName	: LCDObject.h
Author		: tanghong
Version		: 0.0.1
Date		: 2022/11/21
Description	: None
History		: LCD Display (1080*1920) Driver for OverseasAPS
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LCDOBJECT_H_
#define _LCDOBJECT_H_

/* Includes ------------------------------------------------------------------*/

#include "includes.h"

/* Exported types ------------------------------------------------------------*/

struct LcdObject                    // LCD对象通用属性
{
    char name[64];                  // 对象名称
    int id;                         // 对象ID
    char type;                      // 对象类型
    char layer;                     // 对象所属层
    int page;                       // 对象所属页
    char hide;                      // 对象隐藏标记
    char disable;                   // 对象禁用标记
    char update;                    // 对象状态更新标记
    void * data;                    // 对象数据
    void * win;                     // 对象所属窗口

    struct LcdObject * prev;        // 对象双向链表
    struct LcdObject * next;   
};

typedef struct LcdObject LcdObject_t;

/* Exported define -----------------------------------------------------------*/

#define OBJ_TYPE_TEXT               0
#define OBJ_TYPE_QRCODE             1
#define OBJ_TYPE_IMAGE              2
#define OBJ_TYPE_BUTTON             3
#define OBJ_TYPE_KEYPAD             4 
#define OBJ_TYPE_VIDEO              5

// 对象层级定义：背景图-0，公共固定显示内容-1，页面特有文本-2，页面特有图像-3，页面特有按钮-4，页面特有键盘-5，页面特有二维码-6
#define OBJ_LAYER_0     0       // 背景图
#define OBJ_LAYER_1     1       // 公共固定显示内容
#define OBJ_LAYER_2     2       // 页面特有文本
#define OBJ_LAYER_3     3       // 页面特有图像
#define OBJ_LAYER_4     4       // 页面特有按钮
#define OBJ_LAYER_5     5       // 页面特有键盘
#define OBJ_LAYER_6     6       // 页面特有视频
#define OBJ_LAYER_7     7       //  

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

LcdObject_t * CreateLcdObject ( char * name, unsigned int id, char type, char layer, void * data );
void LcdObjectUpdate ( LcdObject_t * obj );

#endif /* _LCDOBJECT_H_ */
/******************************** END OF FILE *********************************/

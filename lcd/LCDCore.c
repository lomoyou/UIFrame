/*******************************************************************************
Copyright (C) 2018, Cytel (Shanghai) Ltd.
FileName	: LCDCore.c
Author		: tanghong
Version		: 0.1.0
Date		: 2019/8/26
Description	: None
History		: None
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include "includes.h"
#include "logging.h"

#include "LCDCore.h"
#include "LCDWindow.h"
#include "LCDObject.h"
#include "LCDText.h"
#include "LCDQRCode.h"
#include "LCDImage.h"
#include "LCDButton.h"
#include "LCDKeypad.h"
#include "LCDVideo.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

/* Private define ------------------------------------------------------------*/

#define DUMMY_STRING                (char*)""
#define RECT(rect,X,Y,W,H)          { rect.x = X; rect.y = Y; rect.w = W; rect.h = H;       }
#define COLOR(color,R,G,B,A)        { color.r = R; color.g = G; color.b = B; color.a = A;   }
#define SYS_OBJ_ID( t, p, s)        ( t << 24 | p << 16 | 1 << 8 | s )
#define SDL_LCD_UPDATE              ( SDL_USEREVENT +1 )

// 最大窗口个数
#define MAX_WINDOWS_COUNT               10
#define SDL_EVENT_LCD_USER               (SDL_USEREVENT +1)
#define SDL_EVENT_LCD_UPDATE             (SDL_USEREVENT +2)
/* Private types -------------------------------------------------------------*/



typedef struct                          // LCD 事件信息结构体
{
    int obj;                            // 对象ID
    int event;                          // 类型
    void * data;                        // 事件数据
    unsigned long long ts;              // 时间戳
    LCD_EventCb_t callback;             // 回调函数
} LcdEvent_t;

// 全局窗口资源
typedef struct {
    int count;
    LcdWindow_t  *list[MAX_WINDOWS_COUNT];
} LcdGlobalWindows_t;

typedef struct {
    int page;
    int option;
    LcdWin_t *win;
} SDL_LCDEvent;

/* Public variables ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static LcdGlobalWindows_t global = {0};

/* Private function prototypes -----------------------------------------------*/

static void * LcdEventCbThread      ( void * arg );
static void * LcdDisplayThread      ( void * arg );

/* Private functions ---------------------------------------------------------*/

static void PutOutLog(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    plog(LOG_LEVEL_WARN, "%s\n", message);
}

/*-----------------------------------------------------------------------------
Function	: EventTriggerCb
Description	: 触发事件回调
Input		: event - 事件代码
              data - 事件数据
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
static int EventTriggerCb       (LCD_EventCb_t callback, int id, int event, void * data  )
{
    struct timeval tv = { 0 };
    LcdEvent_t * pEvent = NULL;
    pthread_t  evcbTid;

    gettimeofday( &tv, NULL );

    // 填充回调参数
    pEvent = (LcdEvent_t *)malloc( sizeof(LcdEvent_t) );
    memset(pEvent, 0x00, sizeof(LcdEvent_t));

    if( pEvent == NULL )
    {
        plog( LOG_LEVEL_WARN, "事件数据创建失败！\n" );
        return -1;
    }

    pEvent->obj = id;
    pEvent->event = event;
    pEvent->ts = (unsigned long long)(tv.tv_sec)*1000 + (tv.tv_usec/1000);
    pEvent->data = data;

    if (callback) pEvent->callback = callback;

    // 创建回调线程
    // 该线程为动态创建 需要注意使用 detach 或 join
    if ( pthread_create( &evcbTid,
                         NULL,
                         LcdEventCbThread,
                         (void*)pEvent ))
    {
        plog( LOG_LEVEL_WARN, "创建回调线程失败！\n" );
        return -1;
    }

    // 分离回调线程 因为不希望阻塞
    pthread_detach( evcbTid );

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: SDLEventThread
Description	: SDL事件响应线程
Input		: arg - NC
Output		: None
Return		: None 
Note		: None
------------------------------------------------------------------------------*/
//static void * SDLEventThread ( void * arg )
void SDLEventThread( void )
{
    SDL_Event event = { 0 };
    SDL_TouchFingerEvent tfEvent = { 0 };
    SDL_MouseButtonEvent mbEvent = { 0 };
    SDL_UserEvent uev = { 0 };
    char quit = 0;
    int ts, x, y;
	char touchFlag, showCursor = 0;

    int hit = 0;
    LcdButton_t * hbtn = NULL;
    LcdKeypad_t * hkp = NULL;

    plog(LOG_LEVEL_NOTICE, "Thread '%s' is ready\n", __func__ );
    sleep(2);
    while( !quit )
    {
        while( SDL_WaitEvent( &event ) )
        {
            // 确定触摸事件类型
            // ts 0 - 不关心
            //    1 - 触摸或鼠标单击
            //    2 - 释放
            // 清空当前记录的页面ID
            switch ( event.type )
            {
                case SDL_QUIT :
                {
                    printf("QUIT\n");
                    quit = 1;
                    ts = 0;
                    exit(1);
                }break;
                case SDL_MOUSEBUTTONUP :
                {
                    mbEvent = event.button;
                    ts = 2;
                    touchFlag = 1;
                }break;
                case SDL_MOUSEBUTTONDOWN :
                {
                    mbEvent = event.button;
                    ts = 1;
                    x = mbEvent.x;
                    y = mbEvent.y;
                    touchFlag = 0;

					if( !showCursor )
					{
						SDL_ShowCursor(1);
						showCursor = 1;
					}					
                }break;
                case SDL_FINGERUP :
                {
                    tfEvent = event.tfinger;
                    ts = 2;
                    touchFlag = 0;
                }break;
                case SDL_FINGERDOWN :
                {
                    tfEvent = event.tfinger;
                    ts = 1;
                    touchFlag = 1;
                }break;
                case SDL_EVENT_LCD_USER:
                {
                    uev = event.user;
                    pthread_mutex_lock(&((LcdWindow_t *)uev.data1)->lock);

                    if (uev.code == LCD_EVENT_SHOW) SDL_ShowWindow((SDL_Window *)((LcdWindow_t *)(uev.data1))->window);
                    if (uev.code == LCD_EVENT_HIDE) SDL_HideWindow((SDL_Window *)((LcdWindow_t *)(uev.data1))->window);
                    if (uev.code == LCD_EVENT_RAISE) SDL_RaiseWindow((SDL_Window *)((LcdWindow_t *)(uev.data1))->window);
                    pthread_mutex_unlock(&((LcdWindow_t *)uev.data1)->lock);
                    ts = 0;
                }break;
                case SDL_EVENT_LCD_UPDATE:
                 {
                    uev = event.user;
                    pthread_mutex_lock(&((LcdWindow_t *)uev.data1)->lock);
                    LcdWindowPresent(((LcdWindow_t *)uev.data1)->win, uev.code, uev.windowID);
                    pthread_mutex_unlock(&((LcdWindow_t *)uev.data1)->lock);

                    ts = 0;
                }break;
                default :
                {
                    ts = 0;
                }break;
            }

            for (int i = 0; i < global.count; i++)
            {

                // 当前窗口是否使能触摸控制
                if (global.list[i]->able)
                {
                    // 鼠标点击或触摸事件响应
                    if (ts == 1)
                    {
                        LcdObject_t *o = global.list[i]->win->objs;
                        LcdButton_t *btn = NULL;
                        LcdKeypad_t *kp = NULL;

                        if (o == NULL)
                            continue;

                        while(o){
                            o = o->next;
                            if (o && o->type == OBJ_TYPE_BUTTON &&
                                !o->disable && o->page == global.list[i]->page)
                            {
                                btn = (LcdButton_t *)o->data;

                                if (touchFlag)
                                {
                                    // 由于有多个窗口存在，且存在窗口提升，因此应该以提升后的窗口来计算。
                                    x = (int)(tfEvent.x * global.list[i]->xRel);
                                    y = (int)(tfEvent.y * global.list[i]->yRel);
                                }


                                hit = LcdButtonPointCheck(btn, x, y);
                                if (hit)
                                {
                                    hbtn = btn;
                                    ChangeLcdButtonState(btn, KEY_PRESSED);
                                    EventTriggerCb(global.list[i]->pEventCbFun, o->id, LCD_EVENT_TS_PRESS, btn->value);
                                    break;
                                }
                            }

                            if (o && o->type == OBJ_TYPE_KEYPAD &&
                                !o->disable &&
                                o->page == global.list[i]->page)
                            {
                                kp = (LcdKeypad_t *)o->data;
                                if (touchFlag)
                                {
                                    x = (int)(tfEvent.x * global.list[i]->xRel);
                                    y = (int)(tfEvent.y * global.list[i]->yRel);
                                }

                                hit = LcdKeypadPonitCheck(kp, x, y);
                                if (hit >= 0)
                                {
                                    hkp = kp;
                                    ChangeLcdKeypadState(kp, hit);
                                    EventTriggerCb(global.list[i]->pEventCbFun, o->id, LCD_EVENT_TS_PRESS,
                                                   kp->keys[hit]->value);
                                    break;
                                }
                            }

                        }
                    }

                    // 鼠标或触摸释放事件响应
                    if( ts == 2 )
                    {
                        if( hbtn )
                        {
                            // EventTriggerCb(global.list[i]->pEventCbFun, hbtn->obj->id, LCD_EVENT_TS_RELEASE, NULL);
                            ChangeLcdButtonState( hbtn, KEY_RELEASED );
                            hbtn = NULL;
                        }

                        if( hkp )
                        {
                            //EventTriggerCb(global.list[i]->pEventCbFun, hkp->obj->id, LCD_EVENT_TS_RELEASE, NULL);
                            ChangeLcdKeypadState( hkp, -1 );
                            hkp = NULL;
                        }
                    }
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------
Function	: LcdDisplayThread
Description	: 刷新LCD显示线程
Input		: arg - NC
Output		: None
Return		: None 
Note		: None
------------------------------------------------------------------------------*/
void * LcdDisplayThread ( void * arg )
{
    LcdWindow_t *p = NULL;

    p = (LcdWindow_t *)arg;
    plog(LOG_LEVEL_NOTICE, "Thread '%s' is ready\n", __func__ );

    //sleep(2);
    while( 1 )
    {
        sem_wait(&p->sem);
        if( p->page < 0 )  continue;

        pthread_mutex_lock( &p->lock );
		if( p->renew )
		{
			uint32_t option = 0;
			if( p->pgupdate )	option |= 0x00000001;       // 如果是页面更新，则全部重绘，否则只重绘目标对象
			if( p->mask )	option |= 0x00000002;           // 是否启用蒙版效果，针对按钮
#if 0
            LcdWindowPresent( p->win, p->page, option );
#else
            SDL_Event ev = {0};
            SDL_UserEvent uev = {0};
            uev.type = SDL_EVENT_LCD_UPDATE;

            uev.code = (Sint32)p->page;
            uev.data1 = (void *)p;
            uev.windowID = option;
            ev.user = uev;
            SDL_PushEvent(&ev);
#endif
            p->pgupdate = 0;
		}

        pthread_mutex_unlock( &p->lock );
    }
}

/*-----------------------------------------------------------------------------
Function	: LcdEventCbThread
Description	: LCD事件回调线程
Input		: arg - NC
Output		: None
Return		: None 
Note		: 当 LCD 发生特定事件后通过回调函数 将事件信息传递出去
------------------------------------------------------------------------------*/
static void * LcdEventCbThread ( void * arg )
{
    LcdEvent_t * pEvent = ( LcdEvent_t* ) arg;

    if( pEvent == NULL )
    {
        plog( LOG_LEVEL_WARN, "事件回调参数错误！\n" );
        return NULL;
    }

	//plog( LOG_LEVEL_NOTICE, "事件回调线程 /0x%08x/0x%08x/%lld 就绪\n", pEvent->obj, pEvent->event, pEvent->ts  );

    if( pEvent->callback != NULL )
        (pEvent->callback)(pEvent->obj, pEvent->event, pEvent->ts, pEvent->data );
    else    plog( LOG_LEVEL_WARN, "事件回调函数未注册！\n" );

	//plog( LOG_LEVEL_NOTICE, "事件回调线程 0x%08x/0x%08x/%lld 退出.\n", pEvent->obj, pEvent->event, pEvent->ts );
    free( pEvent );
    
    return NULL;
}

/* Public functions ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/*-----------------------------------------------------------------------------
Function	: LCD_Init
Description	: 初始化LCD
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_Init ( void )
{
    int ret;

    // 初始化图形库和相关资源
//    SDL_LogSetOutputFunction(PutOutLog, NULL); //PutOutLog打印日志函数
//    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    // 初始化 SDL VIDEO
    ret = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_HAPTIC);
    if( ret < 0 )
    {
        plog( LOG_LEVEL_ERROR, "初始化 SDL 失败：%s\n", SDL_GetError() );
        return -1;
    } 

    // 初始化 SDL_image
    ret = IMG_Init( IMG_INIT_JPG | IMG_INIT_PNG );
    if( ret < 0 )
    {
        plog( LOG_LEVEL_ERROR, "初始化 SDL_image 失败：%s\n", SDL_GetError() );
        return -1;
    } 

    // 初始化 SDL_ttf
    ret = TTF_Init();
    if( ret < 0 )
    {
        plog( LOG_LEVEL_ERROR, "初始化 SDL_ttf 失败：%s\n", SDL_GetError() );
        return -1;
    } 



    SDL_ShowCursor(0);
    return 0;
}


/*-----------------------------------------------------------------------------
Function	: LCD_CreateWindow
Description	: 创建一个窗口
Input		: name - 窗口名称
              rect - 窗口区域
Output		: None
Return		: 全局窗口对象
Note		: 该函数调用之前，需先调用LCD_Init() 初始化相关资源
------------------------------------------------------------------------------*/
LcdWindow_t *LCD_CreateWindow(char *name, SDL_Rect rect)
{
    LcdWindow_t *w = NULL;

    w = (LcdWindow_t *)malloc(sizeof(LcdWindow_t));
    memset(w, 0x00, sizeof(LcdWindow_t));


    // 创建全局 window
    w->window = SDL_CreateWindow( name,
                                      rect.x,
                                      rect.y,
                                      rect.w, rect.h,
                                  SDL_WINDOW_BORDERLESS);
    if( w->window == NULL )
    {
        plog(LOG_LEVEL_ERROR, "Failed to create SDL window, err：%s\n",SDL_GetError() );
        return NULL;
    }
    w->xRel = rect.w;
    w->yRel = rect.h;


    // 创建全局 renderer
    w->render = SDL_CreateRenderer( w->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if( w->render == NULL ) {
        plog(LOG_LEVEL_ERROR, "Failed to create SDL renderer, err：%s\n", SDL_GetError());
        return NULL;
    }

    // 画背景色
    SDL_SetRenderDrawColor(w->render, 0x35, 0x34, 0x36, 0xFF);
    SDL_RenderClear(w->render);
    SDL_RenderPresent(w->render);

    sem_init(&w->sem, 0, 0);
    pthread_mutex_init( &w->lock, 0 );


    w->win = CreateLcdWindow(DUMMY_STRING, &w->sem, NULL, rect.w, rect.h, w->render);
    if ( w->win == NULL ) return NULL;

    // 初始化LCD刷新显示线程
    if( pthread_create( &w->dTid, NULL, LcdDisplayThread, (void *)w ) )
    {
        plog( LOG_LEVEL_ERROR, "Failed to create thread 'LcdDisplayThread'\n" );
        return NULL;
    }

    w->renew = 1;

    // 将创建的窗口绑定到全局窗口资源。
    w->id = global.count;
    global.list[global.count] = w;

    global.count++;
    return w;
}

/*-----------------------------------------------------------------------------
Function	: LCD_DestroyWindow
Description	: 创建一个窗口
Input		: name - 窗口名称
              rect - 窗口区域
Output		: None
Return		: 全局窗口对象
Note		: 该函数调用之前，需先调用LCD_Init() 初始化相关资源
------------------------------------------------------------------------------*/
int LCD_DestroyWindow( LcdWindow_t * win )
{
    int idx;
    for (int i = 0; i < global.count; i++)
    {
        // 查找对应窗口
        if(global.list[i]->id == win->id)
        {
            idx = i;
            break;
        }
    }

    for (int i = 0; i < global.count - idx; i++)
    {
        global.list[i+idx] = global.list[i+idx+1];
    }

    global.count--;

    // 回收资源
    pthread_cancel(win->dTid);
    pthread_cancel(win->tTid);

    SDL_DestroyRenderer(win->render);
    SDL_DestroyWindow(win->window);
    sem_destroy(&win->sem);
    pthread_mutex_destroy(&win->lock);
    
    free(win->win);
    free(win);

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCD_SetCallback
Description	: 设置LCD事件回调函数
Input		: cb - 回调函数地址
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_SetCallback    		( LcdWindow_t *win, LCD_EventCb_t cb )
{
   if (win)
   {
       pthread_mutex_lock(&win->lock);
       win->pEventCbFun = cb;
       pthread_mutex_unlock(&win->lock);
   }
    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCD_SelectPage
Description	: 选择LCD当前显示的页面
Input		: id - 页面ID, 参考 LCD_PAGE_XXX 定义
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_SelectPage          ( LcdWindow_t *win, int id)
{
    if( id >= 0x00 &&
        id <= 0x11 )
    {

        pthread_mutex_lock( &win->lock );
        win->page = id;
        win->pgupdate = 1;
		win->mask = 0;

        sem_post( &win->sem );
        pthread_mutex_unlock( &win->lock );
        return 0;
    }

    return -1;
}

/*-----------------------------------------------------------------------------
Function	: LCD_DisplayText
Description	: 更新显示文本
Input		: 
              page - 页面ID
              id - 文本ID
              val - 变量字符串
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_DisplayText         ( LcdWindow_t *win, int page, int id, char * val )
{
    LcdObject_t * o = NULL;
    LcdText_t * txt = NULL;
 
    pthread_mutex_lock( &win->lock );
    // 遍历所有的对象
    o = win->win->objs;
    while(o){
        o = o->next;
        if (o && o->page == page && o->type == OBJ_TYPE_TEXT && o->id == id) {
            txt = (LcdText_t *) (o->data);
            if (txt != NULL) {
                UpdateLcdText(txt, val, 1);
                pthread_mutex_unlock(&win->lock);
                return 0;
            }else{
                break;
            }
        }
    }
    pthread_mutex_unlock( &win->lock );
    return -1;
}

/*-----------------------------------------------------------------------------
Function	: LCD_DisplayImage
Description	: 更新显示图片
              file - 文件路径
Input		: id - 图片ID
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_DisplayImage        ( LcdWindow_t *win, int page,  int id, char * file )
{
    LcdObject_t * o = NULL;
    LcdImage_t * img = NULL;
 
    pthread_mutex_lock( &win->lock );
    // 遍历所有的对象
    o = win->win->objs;
    while(o)
    {
        o = o->next;
        if( o && o->page == page && o->type == OBJ_TYPE_IMAGE && o->id == id )
        {
            img = (LcdImage_t *)(o->data);
            if( img != NULL ){
                UpdateLcdImage( img, file );
                pthread_mutex_unlock( &win->lock );
                return 0;
            }else{
                break;
            }
        }
    }

    pthread_mutex_unlock( &win->lock );
    return -1;
}

/*-----------------------------------------------------------------------------
Function	: LCD_DisplayImage
Description	: 更新显示图片
              file - 文件路径
Input		: id - 图片ID
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int LCD_DisplayVideo        ( LcdWindow_t *win, int page, int id, int w, int h, char *data, int size )
{
    LcdObject_t * o = NULL;
    LcdVideo_t * video = NULL;

    pthread_mutex_lock( &win->lock );
    // 遍历所有的对象
    o = win->win->objs;
    while(o)
    {
        o = o->next;
        if( o && o->page == page && o->type == OBJ_TYPE_VIDEO && o->id == id )
        {
            video = (LcdVideo_t *)(o->data);
            if( video != NULL ){
                UpdateLcdVideo(video, w, h, data, size);
                pthread_mutex_unlock( &win->lock );
                return 0;
            }else{
                break;
            }
        }
    }

    pthread_mutex_unlock( &win->lock );
    return -1;
}
/*-----------------------------------------------------------------------------
Function	: LCD_DisplayQRCode
Description	: 更新显示二维码
Input		: qr - 二维码ID
              val - 变量字符串
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_DisplayQRCode       ( LcdWindow_t *win, int page, int id, char * val )
{
    LcdObject_t * o = NULL;
    LcdQRCode_t * qr = NULL;
 
    pthread_mutex_lock( &win->lock );
    // 遍历所有的对象
    o = win->win->objs;

    while(o)
    {
        o = o->next;
        if( o && o->page == page && o->type == OBJ_TYPE_QRCODE && o->id == id )
        {
            qr = (LcdQRCode_t *)(o->data);if( qr != NULL )
            {
                UpdateLcdQRCode( qr, val );
                pthread_mutex_unlock( &win->lock );
                return 0;
            }else{
                break;
            }
        }
    }

    pthread_mutex_unlock( &win->lock );
    return -1;
}

/*-----------------------------------------------------------------------------
Function	: LCD_DisplayButtonIcon
Description	: 更新按钮图片
Input		: page - 页面
              id - 按钮ID
              value - 按键值
              icon - 图标文件路径
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_DisplayButtonIcon        ( LcdWindow_t *win, int page, int id, char *value, char * file )
{
    LcdObject_t * o = NULL;
    LcdButton_t * btn = NULL;
 
    pthread_mutex_lock( &win->lock );
    // 遍历所有的对象
    o = win->win->objs;
    while(o){
        o = o->next;
        if( o && o->page == page && o->type == OBJ_TYPE_BUTTON && o->id == id )
        {
            btn = (LcdButton_t *)(o->data);
            if( btn != NULL ){
                // 更新键值
                if (value){
                    memset( btn->value, 0, sizeof( btn->value ) );
                    memcpy( btn->value, value, strlen(value));
                }

                UpdateLcdButton(btn, file);
                pthread_mutex_unlock( &win->lock );
                return 0;
            }
        }
    }

    pthread_mutex_unlock( &win->lock );
    return -1;
}

/*-----------------------------------------------------------------------------
Function	: LCD_ObjectControl
Description	: 控制LCD对象
Input		: id - 对象ID
              ctrl - 控制方式 LCD_OBJ_XXXX
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_ObjectControl        ( LcdWindow_t *win, int id, int ctrl   )
{
    LcdObject_t * o = NULL;
    LcdButton_t * btn = NULL;
    LcdImage_t * img = NULL;
    LcdText_t * txt = NULL;
    LcdQRCode_t * qr = NULL;
    LcdKeypad_t * kp = NULL;
    LcdVideo_t * video = NULL;

    pthread_mutex_lock( &win->lock );
    // 遍历所有的对象
    o = win->win->objs;
    while(o){
        o = o->next;
        if( o && o->id == id ){
            if( id >> 24 == 0 ){
                txt = (LcdText_t *)o->data;
                LcdTextControl( txt, ctrl );
            }

            if( id >> 24 == 1 ){
                qr = (LcdQRCode_t *)o->data;
                LcdQRcodeControl( qr, ctrl );
            }

            if( id >> 24 == 2 ){
                img = (LcdImage_t *)o->data;
                LcdImageControl( img, ctrl );
            }

            if( id >> 24 == 3 ){
                btn = (LcdButton_t *)o->data;
                LcdButtonControl( btn, ctrl & LCD_OBJ_SHOW, ctrl & LCD_OBJ_ENABLE );
            }

            if( id >> 24 == 4 ){
                kp = (LcdKeypad_t *)o->data;
                LcdKeypadControl( kp, ctrl & LCD_OBJ_SHOW, ctrl & LCD_OBJ_ENABLE );
            }

            if( id >> 24 == 5){
                video = (LcdVideo_t *)o->data;
                LcdVideoControl( video, ctrl & LCD_OBJ_SHOW);
            }
            break;
        }
    }

    pthread_mutex_unlock( &win->lock );
    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCD_DisplayLock
Description	: 是否允许LCD刷新显示
Input		: lock - 0 允许刷新
					 1 禁止刷新
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_RenewEnable   	    ( LcdWindow_t *win, int renew         )
{
    pthread_mutex_lock(&win->lock);

    win->renew = renew;
    if( renew ) sem_post( &win->sem );

    pthread_mutex_unlock( &win->lock );
    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCD_MaskEnable
Description	: 激活LCD蒙板效果
Input		: mask - 0 正常显示
					 1 蒙板
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
int LCD_MaskEnable			( LcdWindow_t *win, int mask 		 	 )
{
    pthread_mutex_lock(&win->lock);

    win->mask = mask;
    sem_post( &win->sem );

    pthread_mutex_unlock( &win->lock );
    return 0;
}
/*-----------------------------------------------------------------------------
Function	: LCD_WindowControl
Description	: 控制LCD窗口显示控制
Input		: window - LCD窗口对象
              show - 0 不显示该窗口
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCD_WindowControl            ( LcdWindow_t *win, int show)
{
    SDL_Event ev = {0};
    SDL_UserEvent uev = {0};

    pthread_mutex_lock(&win->lock);
#if 1
    uev.type = SDL_EVENT_LCD_USER;

    if (show) uev.code = LCD_EVENT_SHOW;
    else uev.code = LCD_EVENT_HIDE;

    uev.data1 = (void *)win;
    uev.data2 = NULL;
    ev.type = SDL_EVENT_LCD_USER;
    ev.user = uev;

    SDL_PushEvent(&ev);
#else
    if (!show)
    {
        if (win) SDL_HideWindow(win->window);
    }
    else
    {
        if (win) SDL_ShowWindow(win->window);
    }

#endif
    pthread_mutex_unlock(&win->lock);

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCD_WindowCheck
Description	: 控制LCD窗口触摸检测
Input		: window - LCD窗口对象
              able - 0 不检测触摸数据 1 检测触摸数据
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCD_WindowCheck            ( LcdWindow_t *win, int able)
{
    pthread_mutex_lock(&win->lock);
    if (!able)
    {
        win->able = 0;
    }
    else
    {
        win->able = 1;
    }
    pthread_mutex_unlock(&win->lock);
    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCD_RaisaWindow
Description	: 显示多个窗口时，聚焦到新的窗口。
Input		: window - LCD窗口对象
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCD_RaisaWindow ( LcdWindow_t *win)
{
#if 1
    pthread_mutex_lock(&win->lock);
    SDL_Event ev = {0};
    SDL_UserEvent uev = {0};

    uev.type = SDL_EVENT_LCD_USER;
    uev.code = LCD_EVENT_RAISE;

    uev.data1 = (void *)win;
    uev.data2 = NULL;
    ev.type = SDL_EVENT_LCD_USER;
    ev.user = uev;

    SDL_PushEvent(&ev);
    pthread_mutex_unlock(&win->lock);
#else
    pthread_mutex_lock(&window->lock);
    if (window) SDL_RaiseWindow(window->window);
    pthread_mutex_unlock(&window->lock);
#endif
    return 0;
}
/******************************* END OF FILE **********************************/

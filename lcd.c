/*******************************************************************************
Copyright (C) 2021, Cytel (Shanghai) Ltd. 
FileName	: mWinLcd->c
Author		: 唐红
Version		: 0.0.1
Date		: 2022/12/8
Description	: None 
History		: None
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include "LCDCore.h"
#include "LCDImage.h"
#include "LCDButton.h"
#include "LCDKeypad.h"
#include "LCDObject.h"
#include "LCDText.h"
#include "logging.h"
#include "language.h"
#include "config.h"
#include "subin.h"
#include "LCDVideo.h"

/* Private define ------------------------------------------------------------*/
#define DUMMY_STRING                (char*)""
#define RECT(rect,X,Y,W,H)          { rect.x = X; rect.y = Y; rect.w = W; rect.h = H;       }
#define COLOR(color,R,G,B,A)        { color.r = R; color.g = G; color.b = B; color.a = A;   }
#define SYS_OBJ_ID( t, p, s)        ( t << 24 | p << 16 | 1 << 8 | s )
#define CALCULATENOTE(x)            (x*1.0*syscfg.park.dRate)/syscfg.park.fRate
/* Private types -------------------------------------------------------------*/
typedef struct {
	uint8_t buffer[32];
	uint8_t pos[32];
	uint8_t cnt;
} KeyInput_t;

typedef struct
{
	uint32_t state;
	uint8_t page;
	uint8_t eCnt;	// Enter按键次数 0 - 输入车牌或者登录ID, 1 - 输入登录密码
	LCDResultList_t * list;
	LCDSelfCheck_t selfCheckList[15];
	uint32_t selfCheckCount;
	uint32_t count;
	uint32_t index;
	
	KeyInput_t keyInput;
	uint32_t change;

	LCDPayment_t pay;
	LCDFeedPay_t feed;
    LCDCashBox_t box;
	DrvEventCb_t cb;
    pthread_t tTid;
	pthread_mutex_t evlock;
    int timeout;
    int windowID;
    LcdWindow_t *window;

}Window_t;
/* Public variables ----------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/

static void * UserCbThread     		( void * arg );
static int32_t TriggerUserCb 		( void *callback, uint32_t event, void * data, uint32_t len  );
static int LCDQueryPage	            ( uint32_t page);
// 资源文件
#if 1
char * bgImageFile = (char*)"./images/background.png";
char * buttonClearFile = (char*)"./images/clear.png";
char * keypadEnUpperFile = (char*)"./images/keypad-en-upper.png";
char * keypadEnLowerFile = (char*)"./images/keypad-en-lower.png";
char * keypadSymbolFile = (char*)"./images/keypad-symbol.png";
char * keypadNumberFile = (char*)"./images/keypad-number.png";
char * keypadLPNFile = (char*)"./images/keypad-LPN.png";
char * defLargeFile = (char*)"./images/def-local.png";
char * defSmallFile = (char*)"./images/def-small.png";
char * defNoticeFile = (char*)"./images/def-notice.png";
char * defImgSuccessFile = (char*)"./images/pay-success.png";
char * defImgDisconnectFile = (char*)"./images/disconnect.png";
char * defImgChangeFile = (char*)"./images/change.png";
char * defImgCheckSuccessFile = (char*)"./images/check-success.png";
char * defImgCheckFailFile = (char*)"./images/check-fail.png";
char * defImgCheckingFie = (char*)"./images/checking.png";
char * inputBackgroundFile = (char*)"./images/input-background.png";
char * instructionsBackground = (char*)"./images/instructions-background.png";
char * idPasswordBackground = (char*)"./images/id-passwd-background.png";
char * smallWindowBackground = (char*)"./images/small-window-background.png";
char * maintainCashFeedBackground = (char*)"./images/maintain-cashfeed-background.png";
char * maintainInputBackground = (char*)"./images/maintain-input-background.png";
char * defSmallNoticeFile = (char*)"./images/def-small-notice.png";
char * defAdFile = (char*) "./images/def-ad-file.png";
char * MicrosoftYaHei = (char*)"./fonts/ArialMT.ttf";
#endif

// 键盘布局数据
LcdKeyLayout_t enKeyLayout[] =
{
        {   "1",      0,     0,       72, 72 },
        {   "2",      92,    0,       72, 72 },
        {   "3",      184,   0,       72, 72 },
        {   "4",      276,   0,       72, 72 },
        {   "5",      368,   0,       72, 72 },
        {   "6",      460,   0,       72, 72 },
        {   "7",      552,   0,       72, 72 },
        {   "8",      644,   0,       72, 72 },
        {   "9",      736,   0,       72, 72 },
        {   "0",      828,   0,       72, 72 },
        {   "Q",      0,     92,      72, 72 },
        {   "W",      92,    92,      72, 72 },
        {   "E",      184,   92,      72, 72 },
        {   "R",      276,   92,      72, 72 },
        {   "T",      368,   92,      72, 72 },
        {   "Y",      460,   92,      72, 72 },
        {   "U",      552,   92,      72, 72 },
        {   "I",      644,   92,      72, 72 },
        {   "O",      736,   92,      72, 72 },
        {   "P",      828,   92,      72, 72 },
        {   "A",      0,     184,     72, 72 },
        {   "S",      92,    184,     72, 72 },
        {   "D",      184,   184,     72, 72 },
        {   "F",      276,   184,     72, 72 },
        {   "G",      368,   184,     72, 72 },
        {   "H",      460,   184,     72, 72 },
        {   "J",      552,   184,     72, 72 },
        {   "K",      644,   184,     72, 72 },
        {   "L",      736,   184,     72, 72 },
        {   "<-",     828,   184,     72, 72 },
        {   "Z",      0,     276,     72, 72 },
        {   "X",      92,    276,     72, 72 },
        {   "C",      184,   276,     72, 72 },
        {   "V",      276,   276,     72, 72 },
        {   "B",      368,   276,     72, 72 },
        {   "N",      460,   276,     72, 72 },
        {   "M",      552,   276,     72, 72 },
        {   "Enter",  644,   276,     256, 72 },

};

// 符号键盘
LcdKeyLayout_t enSymbolKeyLayout[] =
{
        {   "[",      0,     0,       95, 72 },
        {   "]",      115,   0,       95, 72 },
        {   "{",      230,   0,       95, 72 },
        {   "}",      345,   0,       95, 72 },
        {   "(",      460,   0,       95, 72 },
        {   ")",      575,   0,       95, 72 },
        {   "<",      690,   0,       95, 72 },
        {   ">",      805,   0,       95, 72 },
        {   "+",      0,     92,      95, 72 },
        {   "-",      115,   92,      95, 72 },
        {   "_",      230,   92,      95, 72 },
        {   "=",      345,   92,      95, 72 },
        {   "~",      460,   92,      95, 72 },
        {   "/",      575,   92,      95, 72 },
        {   "\\",     690,   92,      95, 72 },
        {   "|",      805,   92,      95, 72 },
        {   "`",      0,     184,     95, 72 },
        {   ",",      115,   184,     95, 72 },
        {   ";",      230,   184,     95, 72 },
        {   ":",      345,   184,     95, 72 },
        {   "'",      460,   184,     95, 72 },
        {   "\"",     575,   184,     95, 72 },
        {   "*",      690,   184,     95, 72 },
        {   "?",      805,   184,     95, 72 },
        {   "Upper",  0,     276,     118,72 },
        {   "!",      138,   276,     72, 72 },
        {   "@",      230,   276,     72, 72 },
        {   "#",      322,   276,     72, 72 },
        {   "$",      414,   276,     72, 72 },
        {   "%",      506,   276,     72, 72 },
        {   "^",      598,   276,     72, 72 },
        {   "&",      690,   276,     72, 72 },
        {   "<-",     782,   276,     118, 72 },

};

// 键盘布局数据
LcdKeyLayout_t enUpperKeyLayout[] =
{
        {   "1",      0,     0,       72, 72 },
        {   "2",      92,    0,       72, 72 },
        {   "3",      184,   0,       72, 72 },
        {   "4",      276,   0,       72, 72 },
        {   "5",      368,   0,       72, 72 },
        {   "6",      460,   0,       72, 72 },
        {   "7",      552,   0,       72, 72 },
        {   "8",      644,   0,       72, 72 },
        {   "9",      736,   0,       72, 72 },
        {   "0",      828,   0,       72, 72 },
        {   "Q",      0,     92,      72, 72 },
        {   "W",      92,    92,      72, 72 },
        {   "E",      184,   92,      72, 72 },
        {   "R",      276,   92,      72, 72 },
        {   "T",      368,   92,      72, 72 },
        {   "Y",      460,   92,      72, 72 },
        {   "U",      552,   92,      72, 72 },
        {   "I",      644,   92,      72, 72 },
        {   "O",      736,   92,      72, 72 },
        {   "P",      828,   92,      72, 72 },
        {   ".?=",    0,     184,     72, 72 },
        {   "A",      92,    184,     72, 72 },
        {   "S",      184,   184,     72, 72 },
        {   "D",      276,   184,     72, 72 },
        {   "F",      368,   184,     72, 72 },
        {   "G",      460,   184,     72, 72 },
        {   "H",      552,   184,     72, 72 },
        {   "J",      644,   184,     72, 72 },
        {   "K",      736,   184,     72, 72 },
        {   "L",      828,   184,     72, 72 },
        {   "Lower",  0,     276,     118, 72 },
        {   "Z",      138,    276,    72, 72 },
        {   "X",      230,   276,     72, 72 },
        {   "C",      322,   276,     72, 72 },
        {   "V",      414,   276,     72, 72 },
        {   "B",      506,   276,     72, 72 },
        {   "N",      598,   276,     72, 72 },
        {   "M",      690,   276,     72, 72 },
		{   "<-",   782,   276,     118, 72 },

};

// 键盘布局数据
LcdKeyLayout_t enLowerKeyLayout[] =
{
        {   "1",      0,     0,       72, 72 },
        {   "2",      92,    0,       72, 72 },
        {   "3",      184,   0,       72, 72 },
        {   "4",      276,   0,       72, 72 },
        {   "5",      368,   0,       72, 72 },
        {   "6",      460,   0,       72, 72 },
        {   "7",      552,   0,       72, 72 },
        {   "8",      644,   0,       72, 72 },
        {   "9",      736,   0,       72, 72 },
        {   "0",      828,   0,       72, 72 },
        {   "q",      0,     92,      72, 72 },
        {   "w",      92,    92,      72, 72 },
        {   "e",      184,   92,      72, 72 },
        {   "r",      276,   92,      72, 72 },
        {   "t",      368,   92,      72, 72 },
        {   "y",      460,   92,      72, 72 },
        {   "u",      552,   92,      72, 72 },
        {   "i",      644,   92,      72, 72 },
        {   "o",      736,   92,      72, 72 },
        {   "p",      828,   92,      72, 72 },
        {   ".?=",    0,     184,     72, 72 },
        {   "a",      92,    184,     72, 72 },
        {   "s",      184,   184,     72, 72 },
        {   "d",      276,   184,     72, 72 },
        {   "f",      368,   184,     72, 72 },
        {   "g",      460,   184,     72, 72 },
        {   "h",      552,   184,     72, 72 },
        {   "j",      644,   184,     72, 72 },
        {   "k",      736,   184,     72, 72 },
        {   "l",      828,   184,     72, 72 },
        {   "Upper",  0,     276,     118, 72 },
        {   "z",      138,   276,     72, 72 },
        {   "x",      230,   276,     72, 72 },
        {   "c",      322,   276,     72, 72 },
        {   "v",      414,   276,     72, 72 },
        {   "b",      506,   276,     72, 72 },
        {   "n",      598,   276,     72, 72 },
        {   "m",      690,   276,     72, 72 },
        {   "<-",     782,   276,     118, 72 },
};

// 数字键盘布局数据
LcdKeyLayout_t numberKeyLayout[] =
{
        {   "1",      0,     0,         163, 72 },
        {   "2",      183,   0,         163, 72 },
        {   "3",      366,   0,         163, 72 },
        {   "4",      0,     92,        163, 72 },
        {   "5",      183,   92,        163, 72 },
		{   "6",      366,   92,        163, 72 },
		{   "7",      0,     184,       163, 72 },
		{   "8",      183,   184,       163, 72 },
		{   "9",      366,   184,       163, 72 },
		{   ".",      0,     276,       163, 72 },
		{   "0",      183,   276,       163, 72 },
		{   "<-",     366,   276,       163, 72 },
};

// 功能键键值
#if 1
char * KEY_VAL_HOME = (char*)"HOME";
char * KEY_VAL_CANCEL = (char*)"CANCEL";
char * KEY_VAL_CLEAR = (char*)"CLEAR";
char * KEY_VAL_CONFIRM = (char*)"CONFIRM";
char * KEY_VAL_CONTINUE = (char*)"CONTINUE";
char * KEY_VAL_ENLARGE = (char*)"ENLARGE";
char * KEY_VAL_NEXT = (char*)"NEXT";
char * KEY_VAL_PAYLATER = (char*)"PAYLATER";
char * KEY_VAL_PREV = (char*)"PREV";
char * KEY_VAL_PRINT = (char*)"PRINT";
char * KEY_VAL_QUERY = (char*)"QUERY";
char * KEY_VAL_START = (char*)"START";
char * KEY_VAL_OKAY = (char*)"OKAY";
char * KEY_VAL_RETRY = (char*)"RETRY";
char * KEY_VAL_FINISH = (char*)"FINISH";
char * KEY_VAL_RETURN = (char*)"RETURN";
char * KEY_VAL_REPORT = (char*)"REPORT";
char * KEY_VAL_MENU1 = (char*)"MENU1";
char * KEY_VAL_MENU2 = (char*)"MENU2";
char * KEY_VAL_MENU3 = (char*)"MENU3";
char * KEY_VAL_MENU4 = (char*)"MENU4";
char * KEY_VAL_MENU5 = (char*)"MENU5";
char * KEY_VAL_FEED = (char*)"FEED";
char * KEY_VAL_MORE = (char*)"MORE";
char * KEY_VAL_CLEAN = (char*)"CLEAN";
char * KEY_VAL_EXIT = (char *)"EXIT";
char * KEY_VAL_ENTER = (char *)"ENTER";
#endif
/* Private variables ---------------------------------------------------------*/
//static LcdWindow_t *mWinLcd->window = NULL;	// 主窗口
//static LcdWindow_t *sWinLcd->window = NULL;	// 弹出小窗口
//static LcdWindow_t *mfWinLcd->window = NULL;	// 投币弹出小窗
static Window_t *mWinLcd = NULL;
static Window_t *sWinLcd = NULL;
static Window_t *mfWinLcd = NULL;

/* Private functions ---------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function	: createBackgroundPage
Description	: 创建背景页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createBackgroundPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
    SDL_Rect rect = { 0 };
    int ret;
	char buffer[256] = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;
    LcdVideo_t * video = NULL;

	if (mWinLcd->window == NULL)
		return -1;
	
	// 背景图
	RECT( rect, 0, 0, 1080, 1920 );
	img = CreateLcdImage ( rect, (char*)bgImageFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_IMG_BACKGROUND, OBJ_TYPE_IMAGE, OBJ_LAYER_0, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 底部提示
	RECT( rect, 90, 1872, 900, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, (char *)MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageCommon.tBottomTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_BTIP, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 停车场名称
	RECT( rect, 90, 510, 360, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, (char *)MicrosoftYaHei, color, mWinLcd->window->render );
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s%s", lang.pageCommon.tParkName, syscfg.park.ParkName);
	UpdateLcdText( txt, buffer, TEXT_UPDATE_DISABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_PARKING, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 设备名称
	RECT( rect, 450, 510, 200, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, (char *)MicrosoftYaHei, color, mWinLcd->window->render );
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s%s", lang.pageCommon.tDeviceName, syscfg.park.DeviceName);
	UpdateLcdText( txt, buffer, TEXT_UPDATE_DISABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_DEVICE, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 日期
	RECT( rect, 680, 510, 190, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, (char *)MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, (char*)"Jan 1, 1970", TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_DATE, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 时间
	RECT( rect, 880, 510, 150, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, (char *)MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, (char*)"00:00:00", TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_TIME, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	
	// 页面倒计时
	RECT( rect, 890, 535, 100, 100 );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, (char *)MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, DUMMY_STRING, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_TIMEOUT, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

    //图片广告
    RECT( rect, 0, 0, 1080, 500);
    img = CreateLcdImage( rect, DUMMY_STRING, mWinLcd->window->render );
    UpdateLcdImage( img, defAdFile);
    obj = CreateLcdObject( DUMMY_STRING, SYS_IMG_ADIMG, OBJ_TYPE_IMAGE, OBJ_LAYER_3, img);
    LcdWindowAddObject(mWinLcd->window->win, obj);

    // 视频广告
    RECT( rect, 0, 0, 1080, 500)
    COLOR( color, 0x00, 0x00, 0x00, 0xFF)
    video = CreateLcdVideo(rect, color, mWinLcd->window->render);
    obj = CreateLcdObject( DUMMY_STRING, SYS_VIDEO_ADVIDEO, OBJ_TYPE_VIDEO, OBJ_LAYER_5, video);
    LcdWindowAddObject(mWinLcd->window->win, obj);

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createQueryPage
Description	: 创建查询页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createQueryPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;
	
	// 提示
	RECT( rect, 90, 580, 500, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageQuery.tInputTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 输入框背景
	RECT( rect, 90, 630, 900, 180 );
	img = CreateLcdImage( rect,  inputBackgroundFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_IMG_IMG, OBJ_TYPE_IMAGE, OBJ_LAYER_0, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	
	// 车牌显示文本
	RECT( rect, 120, 672, 764, 100 );
	COLOR( color, 0xe5, 0xe5, 0xe5, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 96, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, DUMMY_STRING, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, QUERY_TEXT_INPUT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 清除键
	RECT( rect, 896, 695, 60, 60 );
	btn = CreateLcdButton( rect, KEY_VAL_CLEAR, NULL, buttonClearFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, QUERY_BUTTON_CLEAR, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 注意
	RECT( rect, 90, 830, 500, 32 );
	COLOR( color, 0xf0, 0x20, 0x20, 0xff );
	txt = CreateLcdText(TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render);
	UpdateLcdText( txt, DUMMY_STRING, TEXT_UPDATE_DISABLED );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_NOTICE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 英文键盘
	RECT( rect, 90, 930, 899, 348 );
	kp = CreateLcdKeypad( rect, keypadLPNFile, sizeof(enKeyLayout)/sizeof(LcdKeyLayout_t), enKeyLayout, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, QUERY_KEYPAD_EN, OBJ_TYPE_KEYPAD, OBJ_LAYER_5, kp );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// instructions background
	RECT( rect, 90, 1374, 899, 348 );
	img = CreateLcdImage( rect,  instructionsBackground, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_IMG_INSTRUCTIONS, OBJ_TYPE_IMAGE, OBJ_LAYER_0, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// instructions text
	RECT( rect, 116, 1382, 899, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageQuery.tInstructions, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_INSTRUCTIONS, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TIP1
	RECT( rect, 116, 1422, 899, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TIP2
	RECT( rect, 116, 1462, 899, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TIP3
	RECT( rect, 116, 1502, 899, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TIP4
	RECT( rect, 116, 1542, 899, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TIP5
	RECT( rect, 116, 1582, 899, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP5, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TIP6
	RECT( rect, 116, 1622, 899, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff);
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject( DUMMY_STRING, QUERY_TEXT_TIP6, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

}

/*-----------------------------------------------------------------------------
Function	: createPayInfoPage
Description	: 创建信息显示页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createPayInfoPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = {0};
	LcdWin_t * win = NULL;
    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;

	// 车辆图提示
	RECT( rect, 90, 615, 200, 30 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 25, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tCarImage, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_CAR, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车辆图
	RECT( rect, 90, 670, 900, 500 );
	img = CreateLcdImage ( rect, defLargeFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_IMG_IMG, OBJ_TYPE_IMAGE, OBJ_LAYER_3, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// "车牌号码提示"
	RECT( rect, 90, 1190, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tLPN, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TLPN, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号
	RECT( rect, 400, 1190, 300, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_LPN, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

//	// 入场时间提示
	RECT( rect, 90, 1240, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tEnterTime, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TETIME, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间
	RECT( rect, 400, 1240, 300, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_ETIME, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 停车时长提示
	RECT( rect, 90, 1290, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tParkingPeriod, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TDURATION, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 停车时长
	RECT( rect, 400, 1290, 300, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_DURATION, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 停车费用提示
	RECT( rect, 90, 1340, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tParkingPrice, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TFEE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 停车费用
	RECT( rect, 400, 1340, 300, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_FEE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 折扣费用提示
	RECT( rect, 90, 1390, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tDiscount, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TDISCOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 折扣费用
	RECT( rect, 400, 1390, 300, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_DISCOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 已支付提示
	RECT( rect, 90, 1440, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tPaid, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TPAID, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 已支付
	RECT( rect, 400, 1440, 300, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_PAID, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 还需缴费提示
	RECT( rect, 90, 1490, 260, 32 );
	COLOR( color,  0x0f, 0xb6, 0x3e, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tBalance, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TBALANCE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 还需缴费
	RECT( rect, 400, 1490, 300, 32 );
	COLOR( color, 0x0f, 0xb6, 0x3e, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_BALANCE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 税率提示
	RECT( rect, 90, 1540, 260, 32 );
	COLOR( color,  0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pagePayInfo.tVAT, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TVAT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 税率
	RECT( rect, 400, 1540, 300, 32);
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_VAT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

//	// 截止出场时间提示
//	RECT( rect, 90, 1590, 260, 32 );
//	COLOR( color,  0x99, 0x99, 0x99, 0xff);
//	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
//	UpdateLcdText( txt, lang.pagePayInfo.tPeriod, TEXT_UPDATE_ENABLED );
//	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_TPERIOD, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
//	LcdWindowAddObject( mWinLcd->window->win, obj );
//
//	// 截止出场时间
//	RECT( rect, 400, 1590, 300, 32 );
//	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
//	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
//	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_TEXT_PERIOD, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
//	LcdWindowAddObject( mWinLcd->window->win, obj );

	// 稍后支付按键
	RECT( rect, 270, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pagePayInfo.bPayLater);
	btn = CreateLcdButton( rect,  KEY_VAL_PAYLATER, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_BUTTON_PAYLATER, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );


	// 确认支付按键
	RECT( rect, 555, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pagePayInfo.bConfirm);
	btn = CreateLcdButton( rect,   KEY_VAL_CONFIRM, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, PAYMENT_BUTTON_PAY, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createLPNListPage
Description	: 创建车辆结果列表展示页
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createLPNListPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;

	// 提示1
	RECT( rect, 90, 580, 800, 30 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 25, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tTopTip1, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject( DUMMY_STRING, RESULT_TEXT_TIP1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt);
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 提示2
	RECT( rect, 90, 610, 600, 30 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText( TEXT_ALIGN_LEFT, rect, 25, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLPNList.tTopTip2, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject( DUMMY_STRING, RESULT_TEXT_TIP2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt);
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 图片0
	RECT( rect, 90, 658, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG0, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片1
	RECT( rect, 410, 658, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG1, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片2
	RECT( rect, 730, 658, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG2, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片3
	RECT( rect, 90, 984, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG3, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片4
	RECT( rect, 410, 984, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG4, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片5
	RECT( rect, 730, 984, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG5, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片6
	RECT( rect, 90, 1310, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG6, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片7
	RECT( rect, 410, 1310, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG7, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 图片8
	RECT( rect, 730, 1310, 260, 206 );
	btn = CreateLcdButton( rect, NULL, NULL, defSmallFile, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_IMG8, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T0
	RECT( rect, 90, 880, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN0, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号0
	RECT( rect, 240, 880, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN0, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T1
	RECT( rect, 410, 880, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号1
	RECT( rect, 560, 880, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T2
	RECT( rect, 730, 880, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号2
	RECT( rect, 880, 880, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T3
	RECT( rect, 90, 1206, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号3
	RECT( rect, 240, 1206, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T4
	RECT( rect, 410, 1206, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号4
	RECT( rect, 560, 1206, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T5
	RECT( rect, 730, 1206, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN5, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号5
	RECT( rect, 880, 1206, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN5, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T6
	RECT( rect, 90, 1532, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN6, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号6
	RECT( rect, 240, 1532, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN6, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T7
	RECT( rect, 410, 1532, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN7, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号7
	RECT( rect, 560, 1532, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN7, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号T8
	RECT( rect, 730, 1532, 150, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tLPN, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TLPN8, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 车牌号8
	RECT( rect, 880, 1532, 100, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_LPN8, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T0
	RECT( rect, 90, 900, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME0, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间0
	RECT( rect, 190, 900, 150, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME0, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T1
	RECT( rect, 410, 900, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间1
	RECT( rect, 510, 900, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_CENTER, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T2
	RECT( rect, 730, 900, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间2
	RECT( rect, 830, 900, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T3
	RECT( rect, 90, 1226, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间3
	RECT( rect, 190, 1226, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T4
	RECT( rect, 410, 1226, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间4
	RECT( rect, 510, 1226, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T5
	RECT( rect, 730, 1226, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME5, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间5
	RECT( rect, 830, 1226, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME5, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T6
	RECT( rect, 90, 1554, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME6, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间6
	RECT( rect, 190, 1554, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME6, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T7
	RECT( rect, 410, 1554, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME7, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间7
	RECT( rect, 510, 1554, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME7, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间T8
	RECT( rect, 730, 1554, 100, 20 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageLPNList.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_TETIME8, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 入场时间8
	RECT( rect, 830, 1554, 170, 20 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 18, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_TEXT_ETIME8, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	

	// 上页键
	RECT( rect, 127, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageLPNList.bPrevPage);
	btn = CreateLcdButton( rect,   KEY_VAL_PREV, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_PREV, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// HOME键
	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageLPNList.bHome);
	btn = CreateLcdButton( rect,   KEY_VAL_HOME, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_HOME, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// 下页键
	RECT( rect, 697, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageLPNList.bNextPage);
	btn = CreateLcdButton( rect,   KEY_VAL_NEXT, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, RESULT_BUTTON_NEXT, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}


/*-----------------------------------------------------------------------------
Function	: createRevisePage
Description	: 创建车辆结果列表展示也
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createRevisePage(void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;

	// 车辆图提示
	RECT( rect, 90, 580, 200, 30 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageEnlarge.tCarImage , TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_CAR, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车辆图
	RECT( rect, 90, 620, 900, 500 );
	img = CreateLcdImage ( rect, defSmallFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_IMG_IMG, OBJ_TYPE_IMAGE, OBJ_LAYER_3, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号提示
	RECT( rect, 90, 1140, 200, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageEnlarge.tLPN , TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_TLPN, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 车牌号
	RECT( rect, 300, 1140, 100, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_LPN, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间提示
	RECT( rect, 90, 1190, 200, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageEnlarge.tEnterTime, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_TETIME, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 入场时间
	RECT( rect, 300, 1190, 250, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_ETIME, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// REVISE_TEXT_TLINE1
	RECT( rect, 90, 1290, 900, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageEnlarge.tTip1,  TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_TLINE1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// REVISE_TEXT_TLINE2
	RECT( rect, 90, 1338, 900, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageEnlarge.tTip2, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_TLINE2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// REVISE_TEXT_TLINE3
	RECT( rect, 90, 1386, 900, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageEnlarge.tTip3, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_TLINE3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// REVISE_TEXT_TLINE4
	RECT( rect, 90, 1434, 900, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageEnlarge.tTip4, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_TEXT_TLINE4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// REVISE_BUTTON_RFETURN
	RECT( rect, 270, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bReturn);
	btn = CreateLcdButton( rect, KEY_VAL_RETURN, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_BUTTON_RETURN, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// REVISE_BUTTON_REPORT
	RECT( rect, 555, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageEnlarge.bReport);
	btn = CreateLcdButton( rect, KEY_VAL_REPORT, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, REVISE_BUTTON_REPORT, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createFeedPage
Description	: 创建车辆结果列表展示页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createFeedPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;

	// 支付提示
	RECT( rect, 90, 580, 900, 30 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 25, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageFeed.tPleasePay, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// amount
	RECT( rect, 90, 640, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageFeed.tAmount, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_TAMOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 340, 640, 250, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_AMOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// paid 
	RECT( rect, 90, 690, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageFeed.tPaid, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_TPAID, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 340, 690, 250, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_PAID, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	
	// balance 
	RECT( rect, 90, 740, 250, 32 );
	COLOR( color, 0x0f, 0xb6, 0x3e, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageFeed.tBalance, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_TBALANCE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 340, 740, 250, 32 );
	COLOR( color, 0x0f, 0xb6, 0x3e, 0xff);
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_BALANCE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Notice
	RECT( rect, 90, 890, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageFeed.tNotice, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_TEXT_NOTICE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// remote
	RECT( rect, 120, 940, 840, 600 );
	img = CreateLcdImage ( rect, defNoticeFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_IMG_IMG, OBJ_TYPE_IMAGE, OBJ_LAYER_3, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 取消按钮
	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageFeed.bCancel);
	btn = CreateLcdButton( rect, KEY_VAL_CANCEL, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, FEED_BUTTON_CANCEL, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createPayResultPage
Description	: 创建支付结果页面,包含网络异常显示页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createPayResultPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;
	
	// 支付结果页面
	// 支付结果图片
	RECT( rect, 385, 658, 310, 310 );
	img = CreateLcdImage ( rect, defImgSuccessFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, PAYRESULT_IMG_TIP, OBJ_TYPE_IMAGE, OBJ_LAYER_3, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 290, 988, 500, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_CENTER, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageSuccess.tTip1, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYRESULT_TEXT_TIP1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 290, 1038, 500, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_CENTER, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageSuccess.tTip2, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYRESULT_TEXT_TIP2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

    // 网络异常显示图片
	RECT( rect, 371, 688, 338, 338 );
	img = CreateLcdImage (rect, defImgDisconnectFile, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, PAYRESULT_IMG_NET, OBJ_TYPE_IMAGE, OBJ_LAYER_3, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

    // 网络异常提示信息
	RECT( rect, 290, 1056, 500, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_CENTER, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageFailed.tTip1, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, PAYRESULT_TEXT_TIP3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createLoginPage
Description	: 创建维护登陆页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createLoginPage(void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;

	// 提示
	RECT( rect, 90, 580, 800, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLogin.tTip1, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TID
	RECT( rect, 180, 698, 170, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLogin.tID, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_TEXT_TID, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// ID
	RECT( rect, 380, 698, 546, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_TEXT_ID, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 356, 680, 546, 64 );
	img = CreateLcdImage( rect, idPasswordBackground, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_IMG_ID, OBJ_TYPE_IMAGE, OBJ_LAYER_1, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TPWSSWORD
	RECT( rect, 180, 802, 200, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
    UpdateLcdText( txt, lang.pageLogin.tPwd, TEXT_UPDATE_ENABLED );
    obj = CreateLcdObject ( DUMMY_STRING, MLOG_TEXT_TPASSWORD, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(mWinLcd->window->win, obj );
	
	// PWSSWORD
	RECT( rect, 380, 802, 546, 32 );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_TEXT_PASSWOED, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 356, 784, 546, 64 );
	img = CreateLcdImage( rect, idPasswordBackground, mWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_IMG_PASSWORD, OBJ_TYPE_IMAGE, OBJ_LAYER_1, img );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 英文键盘大写
	RECT( rect, 90, 930, 900, 348 );
	kp = CreateLcdKeypad( rect, keypadEnUpperFile, sizeof(enUpperKeyLayout)/sizeof(LcdKeyLayout_t), enUpperKeyLayout, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_KEYPAD_UPPER_EN, OBJ_TYPE_KEYPAD, OBJ_LAYER_5, kp );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 英文键盘小写
	RECT( rect, 90, 930, 900, 348 );
	kp = CreateLcdKeypad( rect, keypadEnLowerFile, sizeof(enLowerKeyLayout)/sizeof(LcdKeyLayout_t), enLowerKeyLayout, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_KEYPAD_LOWER_EN, OBJ_TYPE_KEYPAD, OBJ_LAYER_5, kp );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 英文键盘符号
	RECT( rect, 90, 930, 900, 348 );
	kp = CreateLcdKeypad( rect, keypadSymbolFile, sizeof(enSymbolKeyLayout)/sizeof(LcdKeyLayout_t), enSymbolKeyLayout, mWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MLOG_KEYPAD_SYMBOL, OBJ_TYPE_KEYPAD, OBJ_LAYER_5, kp );
	LcdWindowAddObject(mWinLcd->window->win, obj );

    // Enter
    RECT( rect, 165, 1300, 750, 72 );
    memset(&nb, 0x00, sizeof(newButton_t));
    nb.pt = 30;
    COLOR(nb.bColor, 0xFF, 0xD6, 0x31, 0xff);
    COLOR(nb.Color, 0x40, 0x40, 0x40, 0xff);
    strcpy(nb.content, lang.pageLogin.bEnter);
    btn = CreateLcdButton( rect, KEY_VAL_ENTER, &nb, NULL, mWinLcd->window->render );
    obj = CreateLcdObject ( DUMMY_STRING, MLOG_BUTTON_ENTER, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
    LcdWindowAddObject(mWinLcd->window->win, obj );

	// home 
	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageLogin.bHome);
	btn = CreateLcdButton( rect, KEY_VAL_HOME, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MLOG_BUTTON_HOME, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createMaintainPage
Description	: 创建维护菜单页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createMaintainPage(void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };


    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;

	// main menu
	RECT( rect, 90, 600, 200, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageMenu.tMenuTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAIN_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// button1 
	RECT( rect, 90, 664, 900, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
    nb.align = 1;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageMenu.bButton1);
	btn = CreateLcdButton( rect,   KEY_VAL_MENU1, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MAINTAIN_BUTTON_LAST, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// button2 
	RECT( rect, 90, 766, 900, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
    nb.align = 1;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageMenu.bButton2);
	btn = CreateLcdButton( rect,   KEY_VAL_MENU2, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MAINTAIN_BUTTON_SLEFCHECK, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// button3 
	RECT( rect, 90, 868, 900, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
    nb.align = 1;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageMenu.bButton3);
	btn = CreateLcdButton( rect,   KEY_VAL_MENU3, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MAINTAIN_BUTTON_STATEMENT, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// button4 
	RECT( rect, 90, 970, 900, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
    nb.align = 1;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageMenu.bButton4);
	btn = CreateLcdButton( rect,   KEY_VAL_MENU4, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MAINTAIN_BUTTON_HISTORY, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
	// button5 
	RECT( rect, 90, 1072, 900, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
    nb.align = 1;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageMenu.bButton5);
	btn = CreateLcdButton( rect,   KEY_VAL_MENU5, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MAINTAIN_BUTTON_LOGOUT, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createLastTransactionPage
Description	: 创建上次交易详情页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createLastTransactionPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };


    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mWinLcd->window == NULL)
		return -1;
	
	// 页面提示
	RECT( rect, 90, 580, 900, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TLPN
	RECT( rect, 90, 640, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tLPN, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TLPN, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// LPN
	RECT( rect, 340, 640, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_LPN, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TNPay
	RECT( rect, 90, 690, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tNPay, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TNEEDPAY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// NPay
	RECT( rect, 340, 690, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_PAY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TCPay
	RECT( rect, 90, 740, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tCPay, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TCOINPAY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// CPay
	RECT( rect, 340, 740, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_COINPAY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TBPay
	RECT( rect, 90, 790, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tBPayed, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TBILLPAY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// BPay
	RECT( rect, 340, 790, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_BILLPAY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TNChange
	RECT( rect, 90, 840, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tNChange, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TNEEDCHANGE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Change
	RECT( rect, 340, 840, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_NEEDCHANGE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TAlready Change
	RECT( rect, 90, 890, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tAChange, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TALCHANGE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// already change
	RECT( rect, 340, 890, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_ALCHANGE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// TBalance
	RECT( rect, 90, 940, 250, 32 )
	COLOR( color, 0x99, 0x99, 0x99, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLT.tBalance, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_TBALANCE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Balance
	RECT( rect, 340, 940, 300, 32 )
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff )
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_TEXT_BALANCE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// more details
	RECT( rect, 90, 1050, 250, 72 )
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff)
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff)
	strcpy(nb.content, lang.pageLT.bMDetails);
	btn = CreateLcdButton( rect, KEY_VAL_MORE, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_BUTTON_MORE, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// return
	RECT( rect, 412, 1760, 255, 72 )
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff)
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff)
	strcpy(nb.content, lang.pageCommon.bReturn);
	btn = CreateLcdButton( rect, KEY_VAL_RETURN, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LASTTRANSACTION_BUTTON_RETURN, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createSelfCheck
Description	: 创建自检相关页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createSelfCheckPage(void)
{
	SDL_Rect rect = {0};
    SDL_Color color = {0};
    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    newButton_t nb = { 0 };
    int id = 0;

	// 页面提示
	RECT( rect, 90, 580, 900, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageSelfCheck.tTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, SELFCHECK_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

    id = (SELFCHECK_TEXT_TIP & 0x000000FF) +1;
    for( int idx = 0; idx < 15; idx ++)
    {
        // 创建自检选项-文本
        RECT( rect, 90, 640 + 56*idx, 300, 36 ); 
        COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
        txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
        obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(0, LCD_PAGE_SELFCHECK, id), OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
        LcdWindowAddObject(mWinLcd->window->win, obj );

        // 创建自检选项-结果
        RECT( rect, 400, 636 + 56*idx, 44, 44 );
        img = CreateLcdImage ( rect, defImgCheckSuccessFile, mWinLcd->window->render );
        obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(2, LCD_PAGE_SELFCHECK, id ++), OBJ_TYPE_IMAGE, OBJ_LAYER_3, img );
        LcdWindowAddObject(mWinLcd->window->win, obj );
    }

	// return
	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bReturn);
	btn = CreateLcdButton( rect, KEY_VAL_RETURN, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, SELFCHECK_BUTTON_RETURN, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: createLTDetails
Description	: 创建上次交易细节页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createLTDetailsPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
	newButton_t nb = { 0 };
	int idx = 1;


    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdButton_t * btn = NULL;
	// 页面提示
	RECT( rect, 90, 580, 900, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageLTDetails.tLTTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, LTDETAIL_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

    idx = (LTDETAIL_TEXT_TIP & 0x000000FF) + 1;
	for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            // 第一行显示指定文本
            if (i == 0 )
			{
				char *p = NULL;

				if (j == 0)	p = lang.pageLTDetails.tActionTime;
				if (j == 1) p = lang.pageLTDetails.tEvent;
				if (j == 2) p = lang.pageLTDetails.tValue;

                if (j == 0 )
                {
                    RECT( rect,  90, 640 + i*60, 400, 60 )
                } else {
                    RECT( rect,  490 + (j-1)*300, 640 + i*60, 300, 60 )
                }

				COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
				txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
				UpdateLcdText( txt, p, TEXT_UPDATE_ENABLED );
				obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, idx++), OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
				LcdWindowAddObject(mWinLcd->window->win, obj);
			}
			else
            {
                if (j == 0)
                {
                    RECT( rect, 90, 640 + i*60, 400, 60 )
                } else {
                    RECT( rect, 490 + (j-1)*300, 640 + i*60, 300, 60 )
                }
				COLOR( color, 0x99, 0x99, 0x99, 0xff );
				txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
				obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, idx++), OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
				LcdWindowAddObject(mWinLcd->window->win, obj);
			}
		}
	}

	// return
	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bReturn);
	btn = CreateLcdButton( rect, KEY_VAL_RETURN, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, LTDETAIL_BUTTON_RETURN, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );
}

/*-----------------------------------------------------------------------------
Function	: createCashboxTable
Description	: 创建上次交易细节页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createStatementPage( void )
{
    SDL_Rect rect = {0};
    SDL_Color color = {0};
    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdButton_t * btn = NULL;
    uint32_t id = 1;
    uint32_t btnID = 0;
	newButton_t nb = {0}; 
	int width[] = {200, 150, 100, 150, 100, 200};
    char temp[64] = {0};

	// billBox statement
	RECT( rect, 90, 580, 600, 32 ); 
	COLOR( color,  0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_CASHBOX, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// CUrrency
	RECT( rect, 90, 630, 600, 32 ); 
	COLOR( color,  0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
    sprintf(temp, "%s%s", lang.pageStatement.tCurrency, syscfg.park.currency);
	UpdateLcdText( txt, temp, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_CURRENCY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Boxes
	RECT( rect, 90, 740, 200, 50 ); 
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbBoxes, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_BOXES, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// p Value
	RECT( rect, 290, 740, 150, 50 ); 
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbPValue,TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_VALUES, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Qty.
	RECT( rect, 440, 740, 100, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageStatement.tbQty, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_QTY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Amount
	RECT( rect, 540, 740, 150, 50 ); 
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbAmount, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_AMOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Cap.
	RECT( rect, 690, 740, 100, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbCap, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_CAP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Operate
	RECT( rect, 790, 740, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_CENTER, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbOperate, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_OPERATE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Bill Receiver
	RECT( rect, 90, 795, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbBReceiver, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_BRECEIVER, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// Bill DispenserA
	RECT( rect, 90, 1045, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbBDispenser1,TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject (DUMMY_STRING, STATEMENT_TEXT_BDISPENSER1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
    // Bill Dispenser2
    RECT( rect, 90, 1095, 200, 50 );
    COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
    txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
    UpdateLcdText( txt, lang.pageStatement.tbBDispenser2,TEXT_UPDATE_ENABLED );
    obj = CreateLcdObject (DUMMY_STRING, STATEMENT_TEXT_BDISPENSER2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(mWinLcd->window->win, obj );
	// Bill Dispenser3
	RECT( rect, 90, 1145, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText(txt, lang.pageStatement.tbBDispenser3, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject (DUMMY_STRING, STATEMENT_TEXT_BDISPENSER3, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );
    // Bill Dispenser4
    RECT( rect, 90, 1195, 200, 50 );
    COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
    txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
    UpdateLcdText( txt, lang.pageStatement.tbBDispenser4,TEXT_UPDATE_ENABLED );
    obj = CreateLcdObject (DUMMY_STRING, STATEMENT_TEXT_BDISPENSER4, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(mWinLcd->window->win, obj );

	// coin Hooper
	RECT( rect, 90, 1245, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbCHopper, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_HOOPER, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// coin store
	RECT( rect, 90, 1295, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbCStore, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_STORE, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// total
	RECT( rect, 90, 1380, 200, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageStatement.tbTotal, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_TOTAL, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// total amount
	RECT( rect, 540, 1380, 150, 50 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_TEXT_TOTAL_AMOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// exit
	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageStatement.bExit);
	btn = CreateLcdButton( rect, KEY_VAL_EXIT, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, STATEMENT_BUTTON_EXIT, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// 已经分配了15个
	id = (STATEMENT_TEXT_TOTAL_AMOUNT & 0x000000FF) + 1;
    btnID = STATEMENT_BUTTON_CLEAN1;
	for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            // 最后一列，显示按钮 2, 5,6,7,8,9,10
            if ( j == 4 && (i == 2 || i == 5 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10))
            {
                RECT( rect, 830, 795 + i*50, 140, 40 );
                memset(&nb, 0x00, sizeof(newButton_t));
                nb.pt = 30;
                COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
                COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);

                char *p = (i == 2 || i == 10) ? lang.pageStatement.bClean : lang.pageStatement.bFeed;
                char *v = (i == 2 || i == 10) ? KEY_VAL_CLEAN : KEY_VAL_FEED;
                strcpy(nb.content, p);
                btn = CreateLcdButton( rect,  v, &nb, NULL, mWinLcd->window->render );
                obj = CreateLcdObject ( DUMMY_STRING, btnID++, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
                LcdWindowAddObject(mWinLcd->window->win, obj );
            }
            else
            {
				int flag = 0;

				if (j == 0)
				{
					RECT( rect, 290, 795 + i*50, 150, 50 );
					flag = 1;
				}
				else if (j == 1)
				{
					RECT( rect, 440, 795 + i*50, 100, 50 );
					flag = 1;
				}
				else if (j == 2)
				{
					RECT(rect, 540, 795 + i*50, 150, 50);
					flag = 1;
				}
                else if (j == 3)
                {
                    if (i == 2 || i == 5 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10)
                    {
                        RECT(rect, 690, 795 + i * 50, 100, 50);
                        flag = 1;
                    }
                }
				else
				{

				}

				if (flag)
				{
					COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
					txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
					obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id++), OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
					LcdWindowAddObject(mWinLcd->window->win, obj );
				}
            }
		}
    }

    return 0;
}


/*-----------------------------------------------------------------------------
Function	: createHistoryList
Description	: 历史记录页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createHistoryListPage(void)
{
	SDL_Rect rect = {0};
    SDL_Color color = {0};
    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdButton_t * btn = NULL;
    int id = 1;
	newButton_t nb = {0}; 
	char buffer[64] = {0};

	// 页面提示
	RECT( rect, 90, 580, 900, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageHList.tTip, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, HISTORY_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// shift
	RECT( rect, 90, 640, 150, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageHList.tbShift, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, HISTORY_TEXT_SHIFT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// operator
	RECT( rect, 290, 640, 200, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageHList.tbOperator, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, HISTORY_TEXT_OPERATOR, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// time
	RECT( rect, 490, 640, 350, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageHList.tbTime, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, HISTORY_TEXT_TIME, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	// print
	RECT( rect, 840, 640, 150, 32 );
	COLOR( color, 0xcc, 0xcc, 0xcc, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, mWinLcd->window->render );
	UpdateLcdText( txt, lang.pageHList.tbPrint, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, HISTORY_TEXT_PRINT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	RECT( rect, 412, 1760, 255, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bReturn);
	btn = CreateLcdButton( rect, KEY_VAL_RETURN, &nb, NULL, mWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, HISTORY_BUTTON_RETURN, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mWinLcd->window->win, obj );

	id = (HISTORY_TEXT_PRINT & 0x000000FF) + 1;

	/* LCD_PAGE_HISTORY  table*/
	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (j == 3)
			{
                RECT( rect, 840, 695 + i*50, 100, 40 );
                memset(&nb, 0x00, sizeof(newButton_t));
                nb.pt = 28;
                COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
                COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"PRINT%d", i);
                strcpy(nb.content, lang.pageHList.bPrint);
                btn = CreateLcdButton( rect,  buffer, &nb, NULL, mWinLcd->window->render );
                obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(3, LCD_PAGE_HISTORY, id++), OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
                LcdWindowAddObject(mWinLcd->window->win, obj );
			}
			else
			{
				if (j == 0)
				{
					RECT( rect, 90, 690 + i*50, 150, 50 );
				}
				else if (j == 1)
				{
					RECT( rect, 290, 690 + i*50, 200, 50 );
				}
				else 
				{
					RECT( rect, 490, 690 + i*50, 350, 50 );
				}

                COLOR( color, 0x99, 0x99, 0x99, 0xff );
				txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mWinLcd->window->render );
				obj = CreateLcdObject ( DUMMY_STRING, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id++), OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
				LcdWindowAddObject(mWinLcd->window->win, obj );
			}
		}
	}

    return 0;
}
/*-----------------------------------------------------------------------------
Function	: createSmallWindow
Description	: 创建弹出小窗口
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createSmallWindowPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };

    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (sWinLcd->window == NULL)
		return -1;
	
	// 页面背景
	RECT( rect, 0, 0, 600, 384 );
	img = CreateLcdImage( rect,  smallWindowBackground, sWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject( DUMMY_STRING, SMALL_IMG_BACKGROUND, OBJ_TYPE_IMAGE, OBJ_LAYER_0, img );
	LcdWindowAddObject(sWinLcd->window->win, obj );
	
	// 图标 
	RECT( rect, 30, 20, 36, 36 );
	img = CreateLcdImage( rect,  defSmallNoticeFile, sWinLcd->window->render );
	UpdateLcdImage( img, NULL );
	obj = CreateLcdObject( DUMMY_STRING, SMALL_IMG_TIP, OBJ_TYPE_IMAGE, OBJ_LAYER_1, img );
	LcdWindowAddObject(sWinLcd->window->win, obj );

	// head text
	RECT( rect, 80, 25, 400, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, sWinLcd->window->render );
    UpdateLcdText(txt, DUMMY_STRING, TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject (DUMMY_STRING, SMALL_TEXT_HEAD, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(sWinLcd->window->win, obj );

	// timeout
	RECT( rect, 500, 20, 80, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_RIGHT, rect, 30, MicrosoftYaHei, color, sWinLcd->window->render );
    UpdateLcdText(txt, "180", TEXT_UPDATE_ENABLED);
	obj = CreateLcdObject ( DUMMY_STRING, SYS_TEXT_TIMEOUT, OBJ_TYPE_TEXT, OBJ_LAYER_1, txt );
	LcdWindowAddObject(sWinLcd->window->win, obj );

	// L1
	RECT( rect, 30, 100, 540,  32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, sWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, SMALL_TEXT_LINE1, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(sWinLcd->window->win, obj );
    // L2
    RECT( rect, 30, 150, 540,  32 );
    COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
    txt = CreateLcdText ( TEXT_ALIGN_LEFT, rect, 30, MicrosoftYaHei, color, sWinLcd->window->render );
    obj = CreateLcdObject (DUMMY_STRING, SMALL_TEXT_LINE2, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(sWinLcd->window->win, obj );

	// left button 
	RECT( rect, 30, 280, 250, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bCancel);
	btn = CreateLcdButton( rect, KEY_VAL_CANCEL, &nb, NULL, sWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, SMALL_BUTTON_LEFT_CANCEL, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(sWinLcd->window->win, obj );

	// right
	RECT( rect, 320, 280, 250, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bContinue);
	btn = CreateLcdButton( rect, KEY_VAL_CONTINUE, &nb, NULL, sWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, SMALL_BUTTON_RIGHT_CONTINUE, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(sWinLcd->window->win, obj );

    RECT( rect, 320, 280, 250, 72 );
    memset(&nb, 0x00, sizeof(newButton_t));
    nb.pt = 30;
    COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
    COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
    strcpy(nb.content, lang.pageCommon.bRetry);
    btn = CreateLcdButton( rect, KEY_VAL_RETRY, &nb, NULL, sWinLcd->window->render );
    obj = CreateLcdObject ( DUMMY_STRING, SMALL_BUTTON_RIGHT_RETRY, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
    LcdWindowAddObject(sWinLcd->window->win, obj );

    // center_cancel
    RECT( rect, 175, 280, 250, 72 );
    memset(&nb, 0x00, sizeof(newButton_t));
    nb.pt = 30;
    COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
    COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
    strcpy(nb.content, lang.pageCommon.bCancel);
    btn = CreateLcdButton( rect, KEY_VAL_CANCEL, &nb, NULL, sWinLcd->window->render );
    obj = CreateLcdObject ( DUMMY_STRING, SMALL_BUTTON_CENTER_CANCEL, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
    LcdWindowAddObject(sWinLcd->window->win, obj );

    // center_enter
    RECT( rect, 175, 280, 250, 72 );
    memset(&nb, 0x00, sizeof(newButton_t));
    nb.pt = 30;
    COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
    COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
    strcpy(nb.content, lang.pageLogin.bEnter);
    btn = CreateLcdButton( rect, KEY_VAL_ENTER, &nb, NULL, sWinLcd->window->render );
    obj = CreateLcdObject ( DUMMY_STRING, SMALL_BUTTON_CENTER_ENTER, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
    LcdWindowAddObject(sWinLcd->window->win, obj );


	return 0;
}


/*-----------------------------------------------------------------------------
Function	: createMaintainFeed
Description	: 创建维护投币弹出页面
Input		: None
Output		: None
Return		: 0 - success 
              -1 - fail 
Note		: None
------------------------------------------------------------------------------*/
static int createCashFeedPage( void )
{
	SDL_Color color = { 0xCC, 0xCC, 0xCC, 0xFF };
	char buffer[256] = { 0 };
    SDL_Rect rect = { 0 };
    int ret;
	newButton_t nb = { 0 };


    LcdObject_t * obj = NULL;
    LcdText_t * txt = NULL;
    LcdImage_t * img = NULL;
    LcdButton_t * btn = NULL;
    LcdKeypad_t * kp = NULL;

	if (mfWinLcd->window == NULL)
		return -1;

    // 页面背景
    RECT( rect, 0, 0, 600, 890 );
    img = CreateLcdImage(rect, maintainCashFeedBackground, mfWinLcd->window->render );
    UpdateLcdImage( img, NULL );
    obj = CreateLcdObject( DUMMY_STRING, MAINTAINFEED_IMG_BACKGROUND, OBJ_TYPE_IMAGE, OBJ_LAYER_0, img );
    LcdWindowAddObject(mfWinLcd->window->win, obj );

    RECT( rect, 30, 20, 450, 32 );
    COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
    txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
    UpdateLcdText( txt, lang.pageCashFeed.tTip,  TEXT_UPDATE_ENABLED);
    obj = CreateLcdObject (DUMMY_STRING, MAINTAINFEED_TEXT_TIP, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(mfWinLcd->window->win, obj );

    RECT( rect, 470, 20, 100, 32 );
    COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
    txt = CreateLcdText (TEXT_ALIGN_RIGHT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
    obj = CreateLcdObject (DUMMY_STRING, MAINTAINFEED_TEXT_TIMEOUT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(mfWinLcd->window->win, obj );

	// Currency
	RECT( rect, 30, 140, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	UpdateLcdText( txt, lang.pageCashFeed.tCurrency, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_TCURRENCY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	RECT( rect, 280, 140, 200, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_CURRENCY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	// donamination
	RECT( rect, 30, 190, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	UpdateLcdText( txt, lang.pageCashFeed.tDenomination, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_TDONAMINATION, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	RECT( rect, 280, 190, 200, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_DONAMINATION, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	// quantity
	RECT( rect, 30, 240, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	UpdateLcdText( txt, lang.pageCashFeed.tQuantity, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_TQUANTITY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

    // 输入框背景
    RECT( rect, 280, 232, 235, 48 );
    img = CreateLcdImage(rect, maintainInputBackground, mfWinLcd->window->render );
    UpdateLcdImage( img, NULL );
    obj = CreateLcdObject( DUMMY_STRING, MAINTAINFEED_IMG_INPUT, OBJ_TYPE_IMAGE, OBJ_LAYER_0, img );
    LcdWindowAddObject(mfWinLcd->window->win, obj );

	RECT( rect, 283, 240, 200, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_QUANTITY, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	// amount
	RECT( rect, 30, 290, 250, 32 );
	COLOR( color, 0x99, 0x99, 0x99, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	UpdateLcdText( txt, lang.pageCashFeed.tAmount, TEXT_UPDATE_ENABLED );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_TAMOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	RECT( rect, 280, 290, 200, 32 );
	COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
	txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_AMOUNT, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

    RECT( rect, 30, 478, 300, 32 );
    COLOR( color, 0xe6, 0xe6, 0xe6, 0xff );
    txt = CreateLcdText (TEXT_ALIGN_LEFT, rect, 28, MicrosoftYaHei, color, mfWinLcd->window->render );
    UpdateLcdText( txt, lang.pageCashFeed.tKeyboardName, TEXT_UPDATE_ENABLED );
    obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_TEXT_KEYPAD, OBJ_TYPE_TEXT, OBJ_LAYER_2, txt );
    LcdWindowAddObject(mfWinLcd->window->win, obj );

	// cancel
	RECT( rect, 30, 378, 250, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCommon.bCancel);
	btn = CreateLcdButton(rect, KEY_VAL_CANCEL, &nb, NULL, mfWinLcd->window->render );
	obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_BUTTON_CANCEL, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

	//confirm
	RECT( rect, 300, 378, 250, 72 );
	memset(&nb, 0x00, sizeof(newButton_t));
	nb.pt = 30;
	COLOR(nb.bColor, 0x46, 0x46, 0x46, 0xff);
	COLOR(nb.Color, 0xFF, 0xFF, 0xFF, 0xff);
	strcpy(nb.content, lang.pageCashFeed.bConfirm);
	btn = CreateLcdButton(rect, KEY_VAL_CONTINUE, &nb, NULL, mfWinLcd->window->render );
	obj = CreateLcdObject (DUMMY_STRING, MAINTAINFEED_BUTTON_CONFIRM, OBJ_TYPE_BUTTON, OBJ_LAYER_4, btn );
	LcdWindowAddObject(mfWinLcd->window->win, obj );

    // 数字键盘
    RECT( rect, 30, 528, 529, 348 );
    kp = CreateLcdKeypad(rect, keypadNumberFile, sizeof(numberKeyLayout)/sizeof(LcdKeyLayout_t), numberKeyLayout, mfWinLcd->window->render );
    obj = CreateLcdObject ( DUMMY_STRING, MAINTAINFEED_KEYPAD_NUM1, OBJ_TYPE_KEYPAD, OBJ_LAYER_5, kp );
    LcdWindowAddObject(mfWinLcd->window->win, obj );
	return 0;
}

/*-----------------------------------------------------------------------------
Function	: mainWindowEventCb
Description	: 主窗口事件回调函数
Input		: id - 触发事件对象的ID
			  event - 事件类型
			  ts - 事件发生时间戳
			  data - 事件数据
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
static void mainWindowEventCb 		( int id, int event, long long ts, void * data )
{
	uint8_t page = 0;
	LCDButtonEvent_t button = {0};
    const char * str = "*******************";

	pthread_mutex_lock( &mWinLcd->evlock );


	// 事件发生的页面
	if( event == LCD_EVENT_TIMEOUT ) page = id;
	else page = ( id & 0x00FF0000 ) >> 16;
	// 页面是否变化
	if(page != mWinLcd->page)
	{
		// 记录当前页面ID
		if (page != 0)
        {
            mWinLcd->page = page;
            // 清零按键缓存
            memset(&mWinLcd->keyInput, 0, sizeof(KeyInput_t));

            // 清零Enter次数
            mWinLcd->eCnt = 0;
        }

	}
	
	// 键盘输入
	if( event == LCD_EVENT_TS_PRESS && (id >> 24) == 4 ) 
	{
		if( !strcmp( data, "Enter" ) ) // 查询按钮绑定在键盘上
		{
			// 车牌查询页面Enter
			if (id == QUERY_KEYPAD_EN )
			{
				if (mWinLcd->keyInput.cnt >= 1 )
				{
					TriggerUserCb( (void *)mWinLcd->cb, LCD_EVENT_LICENSE, mWinLcd->keyInput.buffer, strlen(mWinLcd->keyInput.buffer) );
				}
			}		
		}
		// 退格 "<-"
		else if( !strcmp( data, "<-" ) )
		{
			// 退格，删除一个
			if( mWinLcd->keyInput.cnt >= 1 )
			{
				memset( mWinLcd->keyInput.buffer + mWinLcd->keyInput.pos[mWinLcd->keyInput.cnt-1],
						0x00,
						sizeof( mWinLcd->keyInput.buffer ) - mWinLcd->keyInput.pos[mWinLcd->keyInput.cnt-1] );
				mWinLcd->keyInput.pos[mWinLcd->keyInput.cnt-1] = 0;
				mWinLcd->keyInput.cnt --;
				
				if ( id == QUERY_KEYPAD_EN )
				{
					LCD_DisplayText( mWinLcd->window, LCD_PAGE_QUERY, QUERY_TEXT_INPUT, mWinLcd->keyInput.buffer );
				}
				else if ( id == MLOG_KEYPAD_UPPER_EN  || id == MLOG_KEYPAD_LOWER_EN || id == MLOG_KEYPAD_SYMBOL)
				{
					if(mWinLcd->eCnt == 0)
                    {
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_MLOG, MLOG_TEXT_ID, mWinLcd->keyInput.buffer );
                    }
					else
                    {
                        char temp[32] = {0};
                        memcpy(temp, str, mWinLcd->keyInput.cnt);
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_MLOG, MLOG_TEXT_PASSWOED, temp );
                    }
				}

			}
			
		} 
		else if( !strcmp( data, "Upper"))	// 切换至大写键盘
		{
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_UPPER_EN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_LOWER_EN, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_SYMBOL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
		}
		else if( !strcmp( data, "Lower"))	// 切换至小写键盘
		{
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_LOWER_EN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_UPPER_EN, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_SYMBOL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
		}
		else if( !strcmp( data, ".?="))		// 切换至符号键盘
		{
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_SYMBOL, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_LOWER_EN, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
            LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_LOWER_EN, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
		}
		else 
		{
			// 输入车牌号, 最多显示13位
			if( mWinLcd->keyInput.cnt < 13 )
			{
				mWinLcd->keyInput.cnt ++;
				mWinLcd->keyInput.pos[mWinLcd->keyInput.cnt-1] = strlen(mWinLcd->keyInput.buffer);
				memcpy( mWinLcd->keyInput.buffer + mWinLcd->keyInput.pos[mWinLcd->keyInput.cnt-1],
						data,
						strlen(data) );
#if 0
                button.page = page;
                button.id = 0;
                strcpy(button.data, data);
                TriggerUserCb( (void *)mWinLcd->cb, LCD_EVENT_BUTTON, &button, sizeof(LCDButtonEvent_t) );
#endif
				if ( id == QUERY_KEYPAD_EN )
				{
					LCD_DisplayText( mWinLcd->window, LCD_PAGE_QUERY, QUERY_TEXT_INPUT, mWinLcd->keyInput.buffer );
				}
				else if ( id == MLOG_KEYPAD_UPPER_EN  || id == MLOG_KEYPAD_LOWER_EN || id == MLOG_KEYPAD_SYMBOL)
				{
                    if(mWinLcd->eCnt == 0)
                    {
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_MLOG, MLOG_TEXT_ID, mWinLcd->keyInput.buffer );
                    }
                    else
                    {
                        char temp[32] = {0};
                        memcpy(temp, str, mWinLcd->keyInput.cnt);
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_MLOG, MLOG_TEXT_PASSWOED, temp );
                    }
				}
			}
		}
	}

	// 用户按键
	if( event == LCD_EVENT_TS_PRESS && (id >> 24) == 3 )
	{
		
		if( id == QUERY_BUTTON_CLEAR ) // 清除车牌输入
		{
            memset(&mWinLcd->keyInput, 0, sizeof(KeyInput_t));
			LCD_DisplayText( mWinLcd->window, LCD_PAGE_QUERY, QUERY_TEXT_INPUT, "" );
		}
		else if (id == MLOG_BUTTON_ENTER )	// 维护登陆页面Enter键
		{
			if (mWinLcd->eCnt == 0)
			{
				TriggerUserCb((void *)mWinLcd->cb, LCD_EVENT_STAFF_ID, mWinLcd->keyInput.buffer, strlen(mWinLcd->keyInput.buffer));
				mWinLcd->eCnt = 1;
                memset(&mWinLcd->keyInput, 0, sizeof(KeyInput_t));
			}
            else
			{
				TriggerUserCb((void *)mWinLcd->cb, LCD_EVENT_PASSWORD, mWinLcd->keyInput.buffer, strlen(mWinLcd->keyInput.buffer));
				mWinLcd->eCnt = 0;
                memset(&mWinLcd->keyInput, 0, sizeof(KeyInput_t));
			}
		}
		else if (id == RESULT_BUTTON_PREV)	// 上一页按键
		{
			if (mWinLcd->index > 0)
			{
                mWinLcd->index--;
				LCDQueryPage(mWinLcd->index);
			}
		}
		else if (id == RESULT_BUTTON_NEXT)	// 下一页按键
		{

			if (mWinLcd->index < mWinLcd->count/9)
			{
                mWinLcd->index++;
				LCDQueryPage(mWinLcd->index);
			}
		}
		else	// 其他按键
		{
            // 如果是查询列表页面
            if (page == LCD_PAGE_RESULT && id >= RESULT_BUTTON_IMG0 && id <= RESULT_BUTTON_IMG8)
            {
                    LCDRevise_t lpn = {0};

                    strcpy(lpn.license, mWinLcd->list[id - RESULT_BUTTON_IMG0].license);
                    strcpy(lpn.enterTime, mWinLcd->list[id - RESULT_BUTTON_IMG0].enterTime);
                    strcpy(lpn.image, mWinLcd->list[id - RESULT_BUTTON_IMG0].large);

                    LCDRevisePage(&lpn);
            }
            else if (page == LCD_PAGE_REVISE)
            {
                if (id == REVISE_BUTTON_RETURN) // 修正页面返回按钮
                {
                    button.page = page;
                    button.id = id;
                    strcpy(button.data, (uint8_t *)data);

                    TriggerUserCb((void *)mWinLcd->cb, LCD_EVENT_BUTTON, &button, sizeof(LCDButtonEvent_t));
                }

                if (id == REVISE_BUTTON_REPORT) // 修正report按钮
                 {
                    TriggerUserCb((void *)mWinLcd->cb, LCD_EVENT_REVISE, data, strlen((char *)data));
                }
            }
            else
            {
                button.page = page;
                button.id = id;
                strcpy(button.data, (uint8_t *)data);

                TriggerUserCb((void *)mWinLcd->cb, LCD_EVENT_BUTTON, &button, sizeof(LCDButtonEvent_t));
            }
		}
	}

	pthread_mutex_unlock( &mWinLcd->evlock );
}

/*-----------------------------------------------------------------------------
Function	: smallWindowEventCb
Description	: LCD底层事件回调函数
Input		: id - 触发事件对象的ID
			  event - 事件类型
			  ts - 事件发生时间戳
			  data - 事件数据
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
static void smallWindowEventCb 		( int id, int event, long long ts, void * data )
{
	LCDButtonEvent_t button = {0};

	pthread_mutex_lock( &sWinLcd->evlock );

	// 用户按键
	if( event == LCD_EVENT_TS_PRESS && (id >> 24) == 3 )
	{

		button.page = LCD_PAGE_SMALL;
		button.id = id;
		strcpy(button.data, (char *)data); // 拷贝键值
		TriggerUserCb((void *)sWinLcd->cb, LCD_EVENT_BUTTON, &button, sizeof(LCDButtonEvent_t));
	}
	pthread_mutex_unlock( &sWinLcd->evlock );
}

static void maintainFeedWindowEventCb ( int id, int event, long long ts, void * data )
{
	LCDButtonEvent_t button = {0};
    char temp[32] = {0};

	pthread_mutex_lock( &mfWinLcd->evlock );

	if (event == LCD_EVENT_TIMEOUT )
	{
		TriggerUserCb((void *)mfWinLcd->cb, LCD_EVENT_CASHFEED_TIMEOUT, NULL, 0);
	}

	if (event == LCD_EVENT_UPDATE_TIMEOUT)
	{
		LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_TIMEOUT, (char *)data );
	}

	// 用户按键
	if( event == LCD_EVENT_TS_PRESS && (id >> 24) == 3 )
	{
        if (id == MAINTAINFEED_BUTTON_CONFIRM)
        {
            button.page = LCD_PAGE_MAINTAIN_FEED;
            button.id = id;
            strcpy(button.data, mfWinLcd->keyInput.buffer);
            TriggerUserCb((void *)mfWinLcd->cb, LCD_EVENT_BUTTON, &button, sizeof(LCDButtonEvent_t));
        }
        else {
            button.page = LCD_PAGE_MAINTAIN_FEED;
            button.id = id;

            strcpy(button.data, data); // 拷贝键值
            TriggerUserCb((void *)mfWinLcd->cb, LCD_EVENT_BUTTON, &button, sizeof(LCDButtonEvent_t));
        }

	}

    // 键盘输入
    if( event == LCD_EVENT_TS_PRESS && (id >> 24) == 4 ) {

        // 退格 "<-"
        if (!strcmp(data, "<-")) {
            // 退格，删除一个
            if (mfWinLcd->keyInput.cnt >= 1) {
                memset(mfWinLcd->keyInput.buffer + mfWinLcd->keyInput.pos[mfWinLcd->keyInput.cnt - 1],
                       0x00,
                       sizeof(mfWinLcd->keyInput.buffer) - mfWinLcd->keyInput.pos[mfWinLcd->keyInput.cnt - 1]);
                mfWinLcd->keyInput.pos[mfWinLcd->keyInput.cnt - 1] = 0;
                mfWinLcd->keyInput.cnt--;

                LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_QUANTITY, mfWinLcd->keyInput.buffer);
                memset(temp, 0, sizeof(temp));
                sprintf(temp, "%llu", mfWinLcd->box.denomination * (unsigned long long)atoi(mfWinLcd->keyInput.buffer));
                LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_AMOUNT, temp);

                if (atoi(mfWinLcd->keyInput.buffer) <= 0)
                {
                    LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_BUTTON_CONFIRM, LCD_OBJ_DISABLE | LCD_OBJ_SHOW);
                }
            }
        } else {
            if (strcmp(data, ".")) // 小数点无效
            {
                // 输入投币数量, 最多显示10位
                if (mfWinLcd->keyInput.cnt < 9) {
                    mfWinLcd->keyInput.cnt++;
                    mfWinLcd->keyInput.pos[mfWinLcd->keyInput.cnt - 1] = strlen(mfWinLcd->keyInput.buffer);
                    memcpy(mfWinLcd->keyInput.buffer + mfWinLcd->keyInput.pos[mfWinLcd->keyInput.cnt - 1],
                           data,
                           strlen(data));

                    LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_QUANTITY, mfWinLcd->keyInput.buffer);
                    memset(temp, 0, sizeof(temp));
                    sprintf(temp, "%llu", mfWinLcd->box.denomination * (unsigned long long)atoi(mfWinLcd->keyInput.buffer));
                    LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_AMOUNT, temp);

                    if (atoi(mfWinLcd->keyInput.buffer) > 0)
                    {
                        LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_BUTTON_CONFIRM, LCD_OBJ_ENABLE | LCD_OBJ_SHOW);
                    }
                }
            }


        }
    }
	pthread_mutex_unlock( &mfWinLcd->evlock );
}

/*-----------------------------------------------------------------------------
Function	: TriggerUserCb
Description	: 触发用户事件回调
Input		: event - 事件代码
              data - 事件数据
			  len - 事件数据长度
Output		: None
Return		: 0 - success
              -1 - fail
Note		: None
------------------------------------------------------------------------------*/
static int32_t TriggerUserCb 		( void *callback, uint32_t event, void * data, uint32_t len )
{
    struct timeval tv = { 0 };
    DrvEvent_t * pEvent = NULL;
    pthread_t  tid;
    pthread_attr_t attr;

    // 填充回调参数
    pEvent = (DrvEvent_t *)malloc( sizeof(DrvEvent_t) );
    if( pEvent == NULL )
    {
        plog( LOG_LEVEL_WARN, "事件数据创建失败！\n" );
        return -1;
    } 
	else
	{
		pEvent->event = event;
		gettimeofday( &tv, NULL );
		pEvent->ts = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec/1000);

		if( data != NULL && len )
		{
			pEvent->dlen = len;
			pEvent->data = malloc( len + 1 );
			pEvent->callback = callback;
			memset( pEvent->data, 0, len + 1 );
            memcpy(pEvent->data, data, len);
		}
		else
		{
            pEvent->callback = callback;
			pEvent->dlen = 0;
			pEvent->data = NULL;
		}
	}

    // 创建回调线程
   // pthread_attr_init( &attr );
    //pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
    if ( pthread_create( &tid, 
                         NULL, //&attr,
                         UserCbThread, 
                         (void*)pEvent ))
    {
        plog( LOG_LEVEL_WARN, "创建回调线程失败！\n" );
        return -1;
    }
    pthread_detach(tid);
    return 0;
}
/*-----------------------------------------------------------------------------
Function	: UserCbThread
Description	: LCD用户接口回调线程
Input		: arg - DrvEvent_t *
Output		: None
Return		: None 
Note		: 当设备事件发生后通过回调函数 将事件信息传递出去
------------------------------------------------------------------------------*/
static void * UserCbThread     		( void * arg )
{
    DrvEvent_t * pEvent = ( DrvEvent_t* ) arg;
    if( pEvent == NULL )
    {
        plog( LOG_LEVEL_WARN, "事件回调参数错误！\n" );
        return NULL;
    }

    // plog( LOG_LEVEL_NOTICE, "事件回调线程 %s.%d 就绪 [%lld]\n", __func__, pEvent->event, pEvent->ts  );
    if( pEvent->callback != NULL ) ((DrvEventCb_t)pEvent->callback)(pEvent);
    else    plog( LOG_LEVEL_WARN, "事件回调函数未注册！\n" );
    // plog( LOG_LEVEL_NOTICE, "事件回调线程 %s.%d 退出.\n", __func__, pEvent->event );

    if( pEvent->data ) free( pEvent->data );
    free( pEvent );
	
    return NULL;
}


/*-----------------------------------------------------------------------------
Function	: LCDQueryPage
Description	: 车牌查询结果设置展示页
Input		: page - 查询结果页编码从0开始
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
static int LCDQueryPage	( uint32_t page)
{
	uint32_t pos = 0;
	uint32_t idx = 0;
	uint8_t buffer[64] = { 0 };
	uint8_t value[32] = { 0 };

	if( page * 9 >= mWinLcd->count )	return -1;

    LCD_RenewEnable(mWinLcd->window, 0 );
	for( idx = 0; idx < 9; idx ++ )
	{
		pos = page*9 + idx;

		if( pos < mWinLcd->count )
		{
			// 更新车辆图片
			LCD_DisplayButtonIcon(mWinLcd->window, LCD_PAGE_RESULT, RESULT_BUTTON_IMG0 + idx, mWinLcd->list[pos].license, mWinLcd->list[pos].small);
            LCD_ObjectControl(mWinLcd->window, RESULT_BUTTON_IMG0 + idx, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

			// 更新车牌
			LCD_DisplayText(mWinLcd->window, LCD_PAGE_RESULT, RESULT_TEXT_LPN0 + idx, mWinLcd->list[pos].license );
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_LPN0 + idx, LCD_OBJ_SHOW);
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_TLPN0 + idx, LCD_OBJ_SHOW);

			// 更新入场时间
			LCD_DisplayText(mWinLcd->window, LCD_PAGE_RESULT, RESULT_TEXT_ETIME0 + idx, mWinLcd->list[pos].enterTime );
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_ETIME0 + idx, LCD_OBJ_SHOW);
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_TETIME0 + idx, LCD_OBJ_SHOW);
		}
		else
		{
            LCD_ObjectControl(mWinLcd->window, RESULT_BUTTON_IMG0 + idx, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_TLPN0 + idx, LCD_OBJ_HIDE);
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_LPN0 + idx, LCD_OBJ_HIDE);
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_TETIME0 + idx, LCD_OBJ_HIDE);
            LCD_ObjectControl(mWinLcd->window, RESULT_TEXT_ETIME0 + idx, LCD_OBJ_HIDE);
		}
	}

    LCD_ObjectControl(mWinLcd->window, RESULT_BUTTON_PREV, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, RESULT_BUTTON_NEXT, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    if (page == 0) {
        LCD_ObjectControl(mWinLcd->window, RESULT_BUTTON_PREV, LCD_OBJ_SHOW | LCD_OBJ_DISABLE);
    }

    if ((page + 1) * 9 >= mWinLcd->count)
    {
        LCD_ObjectControl(mWinLcd->window, RESULT_BUTTON_NEXT, LCD_OBJ_SHOW | LCD_OBJ_DISABLE);
    }

    LCD_RenewEnable(mWinLcd->window, 1);
	return 0;
}

static void * WinTimerThread (void *arg)
{
    char buffer[64] = { 0 };
    time_t tv;
    int mday = -1;
    int sec = -1;
    struct tm * tp = NULL;
    Window_t *p = (Window_t *)arg;
    int event = 0;

    plog(LOG_LEVEL_NOTICE, "Thread '%s' is ready\n", __func__ );
    while( 1 ) {
        // 获取当前时间
        tv = time(NULL) + syscfg.park.TZDiff;
        tp = gmtime(&tv);

        if (sec != tp->tm_sec) {
            // 刷新时间i
            sec = tp->tm_min;
            memset(buffer, 0x00, sizeof(buffer));
            strftime(buffer, sizeof(buffer), lang.timeDateDisplay.tTime, tp);

            LCD_DisplayText(p->window, LCD_PAGE_COMMON, SYS_TEXT_TIME, buffer);
        }

        if (mday != tp->tm_mday)
        {
            mday = tp->tm_mday;
            memset(buffer, 0x00, sizeof(buffer));
            strftime(buffer, sizeof(buffer), lang.timeDateDisplay.tDate, tp);
            LCD_DisplayText(p->window, LCD_PAGE_COMMON, SYS_TEXT_DATE, buffer);
        }

        if (p->timeout > 0) // 倒计时>= 0 时显示
        {
            p->timeout--;
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%ds", p->timeout);
            LCD_DisplayText(p->window, LCD_PAGE_COMMON, SYS_TEXT_TIMEOUT, buffer);

            if (p->timeout == 0)
            {
                if (p->windowID == LCD_WINDOW_MAIN)  event = LCD_EVENT_MAIN_TIMEOUT;
                if (p->windowID == LCD_WINDOW_SMALL)  event = LCD_EVENT_SMALL_TIMEOUT;
                if (p->windowID == LCD_WINDOW_MAINTAINFEED)  event = LCD_EVENT_CASHFEED_TIMEOUT;

                TriggerUserCb( (void *)p->cb, event, NULL, 0);
            }
        }
        sleep(1);
    }
}


/* Public functions ----------------------------------------------------------*/
/*-----------------------------------------------------------------------------
Function	: InitMainWindow
Description	: 初始化主窗口
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t InitMainWindow(void)
{
	int ret;
    pthread_t tid;

	/* 1. 初始化 LCD */
	ret = LCD_Init();
	if (ret != 0)
	{
		plog(LOG_LEVEL_ERROR, "LCD 初始化失败\n");
		return -1;
	}

    mWinLcd = (Window_t *)malloc(sizeof(Window_t));
    memset(mWinLcd, 0x00, sizeof(Window_t));

	/* 2. 创建主窗口 */
	SDL_Rect rect = {0, 0, 1080, 1920};
	mWinLcd->window = LCD_CreateWindow("APS window", rect);
	if (mWinLcd->window == NULL)
	{
		plog(LOG_LEVEL_ERROR, "APS window 创建失败\n");
		return -1;
	}
    mWinLcd->window->able = 1;

    LCD_SetCallback( mWinLcd->window, mainWindowEventCb );

    LCD_RenewEnable(mWinLcd->window, 0);

    pthread_mutex_lock(&mWinLcd->window->lock);

    // 初始化背景页面
    createBackgroundPage();
    // 创建查询页面
    createQueryPage();
    // 创建车牌费用显示页面
    createPayInfoPage();
    // 创建车牌修改列表
    createLPNListPage();
    createRevisePage();
    createFeedPage();
    createPayResultPage();
    createLoginPage();

    createMaintainPage();
    createLastTransactionPage();
    createSelfCheckPage();
    createLTDetailsPage();
    createStatementPage();
    createHistoryListPage();

    pthread_mutex_unlock(&mWinLcd->window->lock);

    LCD_RenewEnable(mWinLcd->window, 1);
    pthread_mutex_init(&mWinLcd->evlock, 0);

    mWinLcd->windowID = LCD_WINDOW_MAIN;
    mWinLcd->timeout = 0;
    pthread_create(&mWinLcd->tTid, NULL, WinTimerThread, (void *) mWinLcd);

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: MainWindowSetUserCallback
Description	: 设置主窗口用户回调函数
Input		: None
Output		: None
Return		: 0  success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t MainWindowSetUserCallback(DrvEventCb_t cb )
{
	if(mWinLcd)
	{
		mWinLcd->cb = cb;
	}
	else
	{
		return -1;
	}

	return 0;
}
/*-----------------------------------------------------------------------------
Function	: InitSmallWindow
Description	: 初始化提示小窗
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t InitSmallWindow(void)
{
	SDL_Rect rect = { 0 };

	/* . 创建弹出的子窗口 */
	rect.x = SDL_WINDOWPOS_CENTERED;
	rect.y = SDL_WINDOWPOS_CENTERED;
	rect.w = 600;
	rect.h = 384;

    if (sWinLcd == NULL) sWinLcd = (Window_t *)malloc(sizeof(Window_t));
    memset(sWinLcd, 0, sizeof(Window_t));
    pthread_mutex_init( &sWinLcd->evlock, 0 );

    sWinLcd->window = LCD_CreateWindow("Notice Window", rect);
    if (sWinLcd->window == NULL)
    {
        plog(LOG_LEVEL_ERROR, "Notice Window 创建失败\n");
        return -1;
    }


    LCD_RenewEnable(sWinLcd->window, 0);
    pthread_mutex_lock(&sWinLcd->window->lock);
    createSmallWindowPage();
    pthread_mutex_unlock(&sWinLcd->window->lock);
    LCD_RenewEnable(sWinLcd->window, 1);
    LCD_SetCallback(sWinLcd->window, smallWindowEventCb );

    sWinLcd->windowID = LCD_WINDOW_SMALL;
//    pthread_create(&sWinLcd->tTid, NULL, WinTimerThread, (void *) sWinLcd);

	return 0;
}
/*-----------------------------------------------------------------------------
Function	: DestroySmallWindow
Description	: 关闭提示小窗
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t DestroySmallWindow(void)
{
	// 释放窗口资源
	if (sWinLcd->window)
	{
		LCD_DestroyWindow( sWinLcd->window );
	}

	if (sWinLcd)
	{
		if (sWinLcd->list) free(sWinLcd->list);
		pthread_mutex_destroy(&sWinLcd->evlock);
		free(sWinLcd);
        sWinLcd = NULL;
	}
}

/*-----------------------------------------------------------------------------
Function	: MainWindowSetUserCallback
Description	: 设置主窗口用户回调函数
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t SmallWindowSetUserCallback(DrvEventCb_t cb )
{
	if(sWinLcd)
	{
		sWinLcd->cb = cb;
	}
	else
	{
		return -1;
	}

	return 0;
}
/*-----------------------------------------------------------------------------
Function	: InitMaintainFeedWindow
Description	: 初始化投币窗口
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t InitMaintainFeedWindow(void)
{
	SDL_Rect rect = { 0 };

	rect.x = SDL_WINDOWPOS_CENTERED;
	rect.y = SDL_WINDOWPOS_CENTERED;
	rect.w = 600;
	rect.h = 890;

    mfWinLcd = (Window_t *)malloc(sizeof(Window_t));
    memset(mfWinLcd, 0, sizeof(Window_t));
    pthread_mutex_init(&mfWinLcd->evlock, 0 );

	if (mfWinLcd->window == NULL)
	{
        mfWinLcd->window = (LcdWindow_t *)malloc(sizeof(LcdWindow_t));
        mfWinLcd->window = LCD_CreateWindow("CASH FEED Window", rect);
		if (mfWinLcd->window == NULL)
		{
			plog(LOG_LEVEL_ERROR, "CASH FEED Window Create failed\n");
			return -1;
		}
	}

	LCD_SetCallback(mfWinLcd->window, maintainFeedWindowEventCb );
    LCD_RenewEnable(mfWinLcd->window, 0);
    pthread_mutex_lock(&mfWinLcd->window->lock);
    createCashFeedPage();
    pthread_mutex_unlock(&mfWinLcd->window->lock);
    LCD_RenewEnable(mfWinLcd->window, 1);
    mfWinLcd->windowID = LCD_WINDOW_MAINTAINFEED;
	return 0;
}
/*-----------------------------------------------------------------------------
Function	: DestroyMaintainFeedWindow
Description	: 关闭提示小窗
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t DestroyMaintainFeedWindow(void)
{
	// 释放窗口资源
	if (mfWinLcd->window)
	{
		LCD_DestroyWindow(mfWinLcd->window );
	}

	if (mfWinLcd)
	{
		if (mfWinLcd->list) free(mfWinLcd->list);
		pthread_mutex_destroy(&mfWinLcd->evlock);
		free(mfWinLcd);
        mfWinLcd = NULL;
	}

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: CashFeedWinodwSetUserCallback
Description	: 设置主窗口用户回调函数
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t MaintainFeedWindowSetUserCallback(DrvEventCb_t cb )
{
	if(mfWinLcd)
	{
        mfWinLcd->cb = cb;
	}
	else
	{
		return -1;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: CashFeedWinodwSetUserCallback
Description	: 设置主窗口用户回调函数
Input		: None
Output		: None
Return		: 0 - success
			  -1 - fail
Note		: None
------------------------------------------------------------------------------*/
int32_t SetLcdWindowTimeout(Window_t  * window, int timeout)
{
    if(window == NULL) return -1;

    window->timeout = timeout;

    return 0;
}
/*-----------------------------------------------------------------------------
Function	: LCDWindowControl
Description	: 切换窗口控制
Input		: id - 窗口ID
              able - 控制使能 0 - 失能; 1 - 使能
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDWindowControl(int windowID)
{
    if (windowID == LCD_WINDOW_MAIN)
    {
        // 隐藏其他子窗口
        if (sWinLcd && sWinLcd->window) {
            LCD_WindowControl(sWinLcd->window, 0);
            LCD_WindowCheck(sWinLcd->window, 0);
        }

        if (mfWinLcd && mfWinLcd->window) {
            LCD_WindowControl(mfWinLcd->window, 0);
            LCD_WindowCheck(mfWinLcd->window, 0);
        }

        if (mWinLcd->window) {
            LCD_RaisaWindow(mWinLcd->window);
            LCD_WindowCheck(mWinLcd->window, 1);
        }
    }

    if (windowID == LCD_WINDOW_SMALL)
    {
        if (mfWinLcd && mfWinLcd->window) {
            LCD_WindowControl(mfWinLcd->window, 0);
            LCD_WindowCheck(mfWinLcd->window, 0);
        }

        if (sWinLcd && sWinLcd->window) {
            LCD_WindowControl(sWinLcd->window, 1);
            LCD_WindowCheck(sWinLcd->window, 1);
        }

        if (mWinLcd->window) {
            LCD_WindowCheck(mWinLcd->window, 0);
        }

        if (sWinLcd && sWinLcd->window) LCD_RaisaWindow(sWinLcd->window);
    }

    if (windowID == LCD_WINDOW_MAINTAINFEED)
    {
        if (sWinLcd && sWinLcd->window) {
            LCD_WindowControl(sWinLcd->window, 0);
            LCD_WindowCheck(sWinLcd->window, 0);
        }

        if (mfWinLcd && mfWinLcd->window) {
            LCD_WindowControl(mfWinLcd->window, 1);
            LCD_WindowCheck(mfWinLcd->window, 1);
        }

        if (mWinLcd->window) {
            LCD_WindowCheck(mWinLcd->window, 0);
        }

        if (mfWinLcd && mfWinLcd->window) LCD_RaisaWindow(mfWinLcd->window);
    }

    return 0;
}
/*-----------------------------------------------------------------------------
Function	: LCDLpnQueryPage
Description	: 切换至车牌查询页面
Input		: notice - 注意消息
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDLpnQueryPage			( char *notice )
{
	char buffer[100] = {0};
	char list[6][512] = {0};
	int ret = 0;

	memset( &mWinLcd->keyInput, 0x00, sizeof( KeyInput_t ));

	if(notice)
	{
		sprintf(buffer, "Notice: %s", notice);
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_QUERY, QUERY_TEXT_NOTICE, buffer );
        LCD_ObjectControl(mWinLcd->window, QUERY_TEXT_NOTICE, LCD_OBJ_SHOW);
	}
	else
	{
        LCD_ObjectControl(mWinLcd->window, QUERY_TEXT_NOTICE, LCD_OBJ_HIDE);
	}

    LCD_ObjectControl(mWinLcd->window, QUERY_BUTTON_CLEAR, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, QUERY_KEYPAD_EN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, QUERY_TEXT_INSTRUCTIONS, LCD_OBJ_SHOW);
	if (strlen(syscfg.park.instructionsTips))
	{
		ret = ParseLFOfText(syscfg.park.instructionsTips, (char *)list, 512);

		if (ret > 0)
		{
			for (int i = 0; i < ret; i ++)
			{
				LCD_DisplayText(mWinLcd->window, LCD_PAGE_QUERY, QUERY_TEXT_TIP1 + i, list[i]);
                LCD_ObjectControl(mWinLcd->window, QUERY_TEXT_TIP1 + i, LCD_OBJ_SHOW);
			}
		}
	}

    // 更新车牌为空
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_QUERY, QUERY_TEXT_INPUT, "");
    // 隐藏超时时间
    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);

    SetLcdWindowTimeout(mWinLcd, -1);
    LCD_SelectPage( mWinLcd->window, LCD_PAGE_QUERY);

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDResultListPage
Description	: 切换至车牌查询结果列表选择页面
Input		: list - 查询结果列表
			  count - 查询结果条目数
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDResultListPage	    (LCDResultList_t *list, int count )
{
	if (mWinLcd->list != NULL)
    {
        free(mWinLcd->list);
        mWinLcd->list = NULL;
    }

	if( count > 0 && list )
	{
		mWinLcd->list = (LCDResultList_t *)malloc(sizeof(LCDResultList_t) * count);
		memcpy( mWinLcd->list, list, sizeof(LCDResultList_t) * count );
		mWinLcd->index = 0;
		mWinLcd->count = count;

		LCDQueryPage(mWinLcd->index);
        LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_SHOW);
        SetLcdWindowTimeout(mWinLcd, 180);
        LCD_SelectPage(mWinLcd->window, LCD_PAGE_RESULT);
	}
	else
    {
		return -1;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDPaymentInfoPage
Description	: 切换至费用展示场景
Input		: payment - 支付信息
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDPaymentInfoPage       (LCDPayment_t *lpn )
{
	struct tm tp = {0};
	char buffer[128] = { 0 };
	time_t duration = 0;
	time_t tempEntryTime = 0;
    int day, hour, min, t, f = 0;

	if( lpn != NULL  )
	{
		// 车牌图片
		LCD_DisplayImage( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_IMG_IMG, lpn->image );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_IMG_IMG, LCD_OBJ_SHOW);

		// 车牌号
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_LPN, lpn->license );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_LPN, LCD_OBJ_SHOW);

		// 入场时间
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_ETIME, lpn->enterTime );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_ETIME, LCD_OBJ_SHOW);

		// 停车时长
		memset( buffer, 0, sizeof(buffer) );
        if ((day = lpn->duration/(24*60)) > 0)
        {
            sprintf(buffer, "%d%s ", day, lang.timeDateDisplay.tDay);
            t = lpn->duration%(24*60);
            f = 1;
        } else {
            t = lpn->duration;
            f = 0;
        }

        if ((hour = t/60) > 0){
            t = lpn->duration%60;
            sprintf(&buffer[strlen(buffer)], "%d%s ", hour, lang.timeDateDisplay.tHour);
            f = 1;
        } else {
            if (f) sprintf(&buffer[strlen(buffer)], "0%s ", lang.timeDateDisplay.tHour);
            t = lpn->duration;
            f = 0;
        }

        if (t > 0)
        {
            sprintf(&buffer[strlen(buffer)], "%d%s", t, lang.timeDateDisplay.tMinute);
        }

		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT,  PAYMENT_TEXT_DURATION, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_DURATION, LCD_OBJ_SHOW);

		// 停车费用
		memset( buffer, 0, sizeof(buffer) );
		sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->price) );
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_FEE, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_FEE, LCD_OBJ_SHOW);
		
		// 折扣
		memset(buffer, 0, sizeof(buffer));
		sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->discount));
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_DISCOUNT, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_DISCOUNT, LCD_OBJ_SHOW);

		// 已付
		memset( buffer, 0, sizeof(buffer) );
		sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->paid));
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_PAID, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_PAID, LCD_OBJ_SHOW);
		
		// 需付
		memset( buffer, 0, sizeof(buffer) );
		sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->balance) );
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_BALANCE, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_BALANCE, LCD_OBJ_SHOW);

		// 税率
		memset( buffer, 0, sizeof(buffer) );
		sprintf( buffer, "%s %0.2f%%", syscfg.park.currency, lpn->vat);
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_VAT, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_VAT, LCD_OBJ_SHOW);

		// 立场截止时间
		memset( buffer, 0, sizeof(buffer) );
		sprintf( buffer, "%s", lpn->periodTime );
		LCD_DisplayText( mWinLcd->window, LCD_PAGE_PAYMENT, PAYMENT_TEXT_PERIOD, buffer );
        LCD_ObjectControl(mWinLcd->window, PAYMENT_TEXT_PERIOD, LCD_OBJ_SHOW);

		// 稍后支付按钮
        LCD_ObjectControl(mWinLcd->window, PAYMENT_BUTTON_PAYLATER, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        if (lpn->balance > 0)
        {
            LCD_ObjectControl(mWinLcd->window, PAYMENT_BUTTON_PAY, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        } else {
            LCD_ObjectControl(mWinLcd->window, PAYMENT_BUTTON_PAY, LCD_OBJ_SHOW | LCD_OBJ_DISABLE);
        }

        LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_SHOW);

        SetLcdWindowTimeout(mWinLcd, 180);
		LCD_SelectPage( mWinLcd->window, LCD_PAGE_PAYMENT);


	}

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDRevisePage
Description	: 切换至车牌修正页面
Input		: license - 车牌信息
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDRevisePage	    (LCDRevise_t *lpn )
{
	// 显示车牌
	LCD_DisplayImage( mWinLcd->window, LCD_PAGE_REVISE, REVISE_IMG_IMG, lpn->image );
    LCD_ObjectControl(mWinLcd->window, REVISE_IMG_IMG, LCD_OBJ_SHOW);

	// 显示车牌号
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_REVISE, REVISE_TEXT_LPN, lpn->license );
    LCD_ObjectControl(mWinLcd->window, REVISE_TEXT_LPN, LCD_OBJ_SHOW);

	// 显示入场时间
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_REVISE, REVISE_TEXT_ETIME, lpn->enterTime );
    LCD_ObjectControl(mWinLcd->window, REVISE_TEXT_ETIME, LCD_OBJ_SHOW);

    // 修改report键值
    LCD_DisplayButtonIcon(mWinLcd->window, LCD_PAGE_REVISE, REVISE_BUTTON_REPORT, lpn->license, NULL);
    // 修改按钮状态
    LCD_ObjectControl( mWinLcd->window, REVISE_BUTTON_RETURN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl( mWinLcd->window, REVISE_BUTTON_REPORT, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_SHOW);

    SetLcdWindowTimeout(mWinLcd, 180);
	LCD_SelectPage( mWinLcd->window, LCD_PAGE_REVISE);

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDCashFeedPage
Description	: 切换至用户投币页面
Input		: LCDLpnFeed - 0 用户支付成功
					   1 用户支付失败
			  change - 找零/退款金额 单位分
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDCashFeedPage			(LCDFeedPay_t *lpn )
{
	char buffer[128] = {0};

	memset( &mWinLcd->feed, 0, sizeof( LCDFeedPay_t ) );
	memcpy( &mWinLcd->feed, lpn, sizeof( LCDFeedPay_t) );

	// 需支付的总金额
	memset( buffer, 0, sizeof(buffer) );
	sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->amount));
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_FEED, FEED_TEXT_AMOUNT, buffer );
    LCD_ObjectControl(mWinLcd->window, FEED_TEXT_AMOUNT, LCD_OBJ_SHOW);

	// 已投币金额
	memset( buffer, 0, sizeof(buffer) );
	sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->paid));
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_FEED, FEED_TEXT_PAID, buffer );
    LCD_ObjectControl(mWinLcd->window, FEED_TEXT_PAID, LCD_OBJ_SHOW);
	
	// 还需支付金额
	memset( buffer, 0, sizeof(buffer) );
	sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->balance));
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_FEED, FEED_TEXT_BALANCE, buffer );
    LCD_ObjectControl(mWinLcd->window, FEED_TEXT_BALANCE, LCD_OBJ_SHOW);


//	// 提示图片
//	LCD_DisplayImage( mWinLcd->window, LCD_PAGE_FEED, FEED_IMG_IMG, lpn->remote );
//	LCD_ObjectControl(mWinLcd->window, FEED_IMG_IMG, LCD_OBJ_SHOW);

	// 取消按钮
    LCD_ObjectControl(mWinLcd->window, FEED_BUTTON_CANCEL, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_SHOW);

    SetLcdWindowTimeout(mWinLcd, 180);
	// 切换页面
	LCD_SelectPage(mWinLcd->window, LCD_PAGE_FEED);



	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDUpdateCashFeedPage
Description	: 更新投币页面信息显示
Input		: LCDFeedPay_t - 0 用户支付成功
					   1 用户支付失败
			  change - 找零/退款金额 单位分
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDUpdateCashFeedPage(LCDFeedPay_t *lpn)
{
    char buffer[128] = {0};


    // 还需支付金额
	memset( buffer, 0, sizeof(buffer) );
	sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->balance));
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_FEED, FEED_TEXT_BALANCE, buffer );
    LCD_ObjectControl(mWinLcd->window, FEED_TEXT_BALANCE, LCD_OBJ_SHOW);

	// 已付金额
    memset( buffer, 0, sizeof(buffer) );
    sprintf( buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->paid));
    LCD_DisplayText( mWinLcd->window, LCD_PAGE_FEED, FEED_TEXT_PAID, buffer );
    LCD_ObjectControl(mWinLcd->window, FEED_TEXT_PAID, LCD_OBJ_SHOW);


	return 0;
}
/*-----------------------------------------------------------------------------
Function	: LCDNoticePage
Description	: 提示页面
Input		: result 0 - 支付失败， 1 - 支付成功， 2 - 网络异常  3 - 找零
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDNoticePage(int result )
{
    if(result == LCD_NOTICE_PAGE_DISCONNECT)
    {
        // 支付失败 or pay failed
        LCD_DisplayImage(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_IMG_NET, defImgDisconnectFile);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_IMG_NET, LCD_OBJ_SHOW);

        LCD_DisplayText(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_TEXT_TIP3, lang.pageFailed.tTip1);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP3, LCD_OBJ_SHOW);

        LCD_ObjectControl(mWinLcd->window, PAYRESULT_IMG_TIP, LCD_OBJ_HIDE);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP1, LCD_OBJ_HIDE);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP2, LCD_OBJ_HIDE);
        SetLcdWindowTimeout(mWinLcd, 5);
    }
    else if (result == LCD_NOTICE_PAGE_SUCCESS)
    {
        // pay success
        LCD_DisplayImage(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_IMG_TIP, defImgSuccessFile);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_IMG_TIP, LCD_OBJ_SHOW);

        LCD_DisplayText(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_TEXT_TIP1, lang.pageSuccess.tTip1);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP1, LCD_OBJ_SHOW);
        LCD_DisplayText(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_TEXT_TIP2, lang.pageSuccess.tTip2);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP2, LCD_OBJ_SHOW);

        LCD_ObjectControl(mWinLcd->window, PAYRESULT_IMG_NET, LCD_OBJ_HIDE);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP3, LCD_OBJ_HIDE);
        SetLcdWindowTimeout(mWinLcd, 5);
    }
    else if (result == LCD_NOTICE_PAGE_CHANGE)
    {
        // pay success
        LCD_DisplayImage(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_IMG_TIP, defImgChangeFile);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_IMG_TIP, LCD_OBJ_SHOW);

        LCD_DisplayText(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_TEXT_TIP1, lang.pageChange.tTip1);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP1, LCD_OBJ_SHOW);
        LCD_DisplayText(mWinLcd->window, LCD_PAGE_PAYRESULT, PAYRESULT_TEXT_TIP2, lang.pageChange.tTip2);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP2, LCD_OBJ_SHOW);

        LCD_ObjectControl(mWinLcd->window, PAYRESULT_IMG_NET, LCD_OBJ_HIDE);
        LCD_ObjectControl(mWinLcd->window, PAYRESULT_TEXT_TIP3, LCD_OBJ_HIDE);

        SetLcdWindowTimeout(mWinLcd, 60);
    }
    else
    {
        return -1;
    }

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_SHOW);
    LCD_SelectPage(mWinLcd->window, LCD_PAGE_PAYRESULT);

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDLoginPage
Description	: 切换至维护登录页面
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDLoginPage			(void )
{

    // 清零密码和账号
    memset(&mWinLcd->keyInput, 0, sizeof (KeyInput_t));

    LCD_DisplayText(mWinLcd->window, LCD_PAGE_MLOG, MLOG_TEXT_ID, "");
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_MLOG, MLOG_TEXT_PASSWOED, "");
    LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_UPPER_EN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_LOWER_EN, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
    LCD_ObjectControl(mWinLcd->window, MLOG_KEYPAD_SYMBOL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

    LCD_ObjectControl(mWinLcd->window, MLOG_BUTTON_ENTER, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_SHOW);

    SetLcdWindowTimeout(mWinLcd, 180);
	LCD_SelectPage( mWinLcd->window, LCD_PAGE_MLOG);

	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDMaintainPage
Description	: 切换至维护选项页面
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDMaintainPage			(void )
{
	// 显示
    LCD_ObjectControl(mWinLcd->window, MAINTAIN_BUTTON_LAST, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, MAINTAIN_BUTTON_SLEFCHECK, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, MAINTAIN_BUTTON_STATEMENT, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, MAINTAIN_BUTTON_HISTORY, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, MAINTAIN_BUTTON_LOGOUT, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);

    SetLcdWindowTimeout(mWinLcd, -1);
	LCD_SelectPage( mWinLcd->window, LCD_PAGE_MAINTAIN);



	return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDLastTransaction
Description	: 切换上次交易界面
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDLastTransaction		(LCDLastTransaction_t *lpn )
{
    char buffer[128] = {0};

    if (!lpn) return -1;

    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_LPN, (char *)lpn->license);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_LPN, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->needPay));
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_PAY, buffer);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_PAY, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->coinPay));
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_COINPAY, buffer);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_COINPAY, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->billPay));
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_BILLPAY, buffer);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_BILLPAY, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->change));
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_NEEDCHANGE, buffer);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_NEEDCHANGE, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->aChange));
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_ALCHANGE, buffer);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_ALCHANGE, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%s %0.2f", syscfg.park.currency, CALCULATENOTE(lpn->balance));
    LCD_DisplayText(mWinLcd->window, LCD_PAGE_LASTTRANSACTION, LASTTRANSACTION_TEXT_BALANCE, buffer);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_TEXT_BALANCE, LCD_OBJ_SHOW);

    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_BUTTON_MORE, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, LASTTRANSACTION_BUTTON_RETURN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);

    SetLcdWindowTimeout(mWinLcd, -1);
    LCD_SelectPage( mWinLcd->window, LCD_PAGE_LASTTRANSACTION);


    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDSelfCheckPage
Description	: 切换至自检页面选项
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDSelfCheckPage(LCDSelfCheck_t * list, int count )
{
	if (list == NULL) return -1;
    if (count < 1 || count > 15) return -1;

    int id = (SELFCHECK_TEXT_TIP & 0x000000FF) + 1;
	for(int i = 0; i < 15; i++)
	{
        if (i < count)
        {
            LCD_DisplayText( mWinLcd->window, LCD_PAGE_SELFCHECK, SYS_OBJ_ID(0, LCD_PAGE_SELFCHECK, id), list[i].name);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_SELFCHECK, id), LCD_OBJ_SHOW);

            LCD_DisplayImage( mWinLcd->window, LCD_PAGE_SELFCHECK, SYS_OBJ_ID(2, LCD_PAGE_SELFCHECK, id), defImgCheckingFie);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(2, LCD_PAGE_SELFCHECK, id), LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
            id ++;
        }
        else
        {
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_SELFCHECK, id++), LCD_OBJ_HIDE);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(2, LCD_PAGE_SELFCHECK, id++), LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        }

	}

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);

    SetLcdWindowTimeout(mWinLcd, -1);
	LCD_SelectPage(mWinLcd->window, LCD_PAGE_SELFCHECK);

	return 0;
}


/*-----------------------------------------------------------------------------
Function	: LCDUpdateSelfCheckResult
Description	: 更新自检页面结果
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDUpdateSelfCheckResult(int num, int result)
{
    int id = (SELFCHECK_TEXT_TIP & 0x000000FF) + 1;

    char *p = result ? defImgCheckSuccessFile : defImgCheckFailFile;
    LCD_DisplayImage( mWinLcd->window, LCD_PAGE_SELFCHECK, SYS_OBJ_ID(2, LCD_PAGE_SELFCHECK, id + num), p);
}

/*-----------------------------------------------------------------------------
Function	: LCDLTDetailsPage
Description	: 切换至自检页面选项
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDLTDetailsPage(LCDLTDetails_t * list, int count )
{
	int id = 0;
    char buffer[128] = {0};

	if (list == NULL) return -1;
    if (count < 1 || count > 15) return -1;



    id = (LTDETAIL_TEXT_TIP & 0x000000FF) + 4;

	for(int i = 0; i < 14; i++)
	{
        if (i < count)
        {
            LCD_DisplayText( mWinLcd->window, LCD_PAGE_LTDETAIL, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id), (list+i)->actionTime);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id++), LCD_OBJ_SHOW);

            LCD_DisplayText( mWinLcd->window, LCD_PAGE_LTDETAIL, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id), (list+i)->event);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id++), LCD_OBJ_SHOW);

            sprintf(buffer, "%d", (list+i)->val);
            LCD_DisplayText( mWinLcd->window, LCD_PAGE_LTDETAIL, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id), buffer);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id++), LCD_OBJ_SHOW);
        }
        else
        {
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id++), LCD_OBJ_HIDE);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id++), LCD_OBJ_HIDE);
            LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_LTDETAIL, id++), LCD_OBJ_HIDE);
        }

	}

    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);
    LCD_ObjectControl(mWinLcd->window, LTDETAIL_BUTTON_RETURN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    SetLcdWindowTimeout(mWinLcd, -1);
	LCD_SelectPage(mWinLcd->window, LCD_PAGE_LTDETAIL);

	return 0;
}
/*-----------------------------------------------------------------------------
Function	: LCDLTDetails
Description	: 切换至自检页面选项
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDStatementPage(LCDStatement_t * statement)
{
	int pos = 4;
	int id = 0, btnID = 0;
	char buffer[64] = { 0 };

	if (statement == NULL) return -1;

    id = (STATEMENT_TEXT_TOTAL_AMOUNT & 0x000000FF) + 1;
    btnID = STATEMENT_BUTTON_CLEAN1;
	for(int i = 0; i < 11; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (j == 4 && (i ==2 || i== 5 || i == 6 || i == 7 || i == 8 || i == 9 || i == 10)) 
			{}
			else
			{
				if (j == 0)
				{
					memset(buffer, 0, sizeof(buffer));
                    (statement->value[i] > 0) ?
                    sprintf(buffer, "%d", statement->value[i]) : strcpy(buffer, "-");
					LCD_DisplayText( mWinLcd->window, LCD_PAGE_STATEMENT, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), buffer);
                    LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), LCD_OBJ_SHOW);
					id ++;
				}
				else if (j == 1)
				{
					memset(buffer, 0, sizeof(buffer));
                    (statement->value[i] > 0) ?
                    sprintf(buffer, "%d", statement->qty[i])  : strcpy(buffer, "-");
					LCD_DisplayText( mWinLcd->window, LCD_PAGE_STATEMENT, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), buffer);
                    LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), LCD_OBJ_SHOW);
					id ++;
				}
				else if (j == 2)
				{
					memset(buffer, 0, sizeof(buffer));
                    (statement->value[i] > 0) ?
                    sprintf(buffer, "%d", statement->amount[i]) : strcpy(buffer, "-");
					LCD_DisplayText( mWinLcd->window, LCD_PAGE_STATEMENT, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), buffer);
                    LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), LCD_OBJ_SHOW);
					id ++;
				}
				else if (j == 3)
				{
					int flag = 0;

					memset(buffer, 0, sizeof(buffer));
					if (i == 2)
					{
						sprintf(buffer, "%d%%", statement->bReceiverCap);
						flag = 1;
					}
					else if (i == 5)
					{
                        if (statement->BDispenserCap[0] >= 0) {
                            sprintf(buffer, "%d%%", statement->BDispenserCap[0]);
                        } else {
                            strcpy(buffer, "-");
                        }
						flag = 1;
					}
					else if ( i == 6)
					{
                        if (statement->BDispenserCap[1] >= 0) {
                            sprintf(buffer, "%d%%", statement->BDispenserCap[1]);
                        } else {
                            strcpy(buffer, "-");
                        }
						flag = 1;
					}
					else if (i == 7)
					{
                        if (statement->BDispenserCap[2] >= 0) {
                            sprintf(buffer, "%d%%", statement->BDispenserCap[2]);
                        } else {
                            strcpy(buffer, "-");
                        }
                        flag = 1;
					}
					else if (i == 8)
					{
                        if (statement->BDispenserCap[3] >= 0) {
                            sprintf(buffer, "%d%%", statement->BDispenserCap[3]);
                        } else {
                            strcpy(buffer, "-");
                        }
						flag = 1;
					}
					else if (i == 9)
					{
						sprintf(buffer, "%d%%", statement->coinHopperCap);
						flag = 1;
					}
					else if (i == 10)
					{
                        sprintf(buffer, "%d%%", statement->coinStoreCap);
                        flag = 1;
					}

					if (flag)
					{
						LCD_DisplayText( mWinLcd->window, LCD_PAGE_STATEMENT, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), buffer);
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_STATEMENT, id), LCD_OBJ_SHOW);
						id ++;
					}

				}
			}
		}

	}

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%d", statement->totalAmount);
	LCD_DisplayText( mWinLcd->window, LCD_PAGE_STATEMENT, STATEMENT_TEXT_TOTAL_AMOUNT, buffer);
    LCD_ObjectControl(mWinLcd->window, STATEMENT_TEXT_TOTAL_AMOUNT, LCD_OBJ_SHOW);

    LCD_ObjectControl(mWinLcd->window, STATEMENT_BUTTON_EXIT, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);

    SetLcdWindowTimeout(mWinLcd, -1);
    LCD_SelectPage(mWinLcd->window, LCD_PAGE_STATEMENT);

	return 0;
}


/*-----------------------------------------------------------------------------
Function	: LCDHistoryPage
Description	: 切换至历史记录页面
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDHistoryPage(LCDHistoryList_t * list, int count)
{	
	uint32_t id = 0;
	char buffer[64] = { 0 };

	if (list == NULL) return -1;
	if (count < 0 || count >15)
		return -1;

	id = (HISTORY_TEXT_PRINT & 0x000000FF) + 1;
	for (int i = 0; i < 15; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if (j == 3)
			{
                if (i < count)
                    LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(3, LCD_PAGE_HISTORY, id++), LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
                else LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(3, LCD_PAGE_HISTORY, id++), LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
			}
			else
			{
				if (j == 0)
				{
                    if (i < count)
                    {
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_HISTORY, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), (list + i)->shift);
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), LCD_OBJ_SHOW);
                    }
                    else
                    {
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), LCD_OBJ_HIDE);
                    }
				}
				else if(j == 1)
				{
                    if (i < count)
                    {
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_HISTORY, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), (list + i)->operatorStr);
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), LCD_OBJ_SHOW);
                    }
                    else
                    {
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), LCD_OBJ_HIDE);
                    }
				}
				else
				{
                    if (i < count)
                    {
                        LCD_DisplayText( mWinLcd->window, LCD_PAGE_HISTORY, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), (list + i)->time);
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), LCD_OBJ_SHOW);
                    }
                    else
                    {
                        LCD_ObjectControl(mWinLcd->window, SYS_OBJ_ID(0, LCD_PAGE_HISTORY, id), LCD_OBJ_HIDE);
                    }
				}

				id++;
			}
		}
	}

    LCD_ObjectControl(mWinLcd->window, HISTORY_BUTTON_RETURN, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    LCD_ObjectControl(mWinLcd->window, SYS_TEXT_TIMEOUT, LCD_OBJ_HIDE);

    SetLcdWindowTimeout(mWinLcd, -1);
	LCD_SelectPage(mWinLcd->window, LCD_PAGE_HISTORY);

    return 0;
}
/*-----------------------------------------------------------------------------
Function	: LCDHistoryPage
Description	: 切换至历史记录页面
Input		: None
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDSmallWindowPage(int  flag)
{
    char list[2][128] = {0};
    int ret = {0};

    memset(&sWinLcd->keyInput, 0, sizeof(KeyInput_t));


    if (flag == LCD_SMALL_WINDOW_NO_NOTE)
    {
        LCD_RenewEnable(sWinLcd->window, 0);

        // 图标
        LCD_DisplayImage(sWinLcd->window, LCD_PAGE_SMALL, SMALL_IMG_TIP, defSmallNoticeFile);
        LCD_ObjectControl(sWinLcd->window, SMALL_IMG_TIP, LCD_OBJ_SHOW);

        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pageNotice.tTip);

        ret = ParseLFOfText(lang.pageNotice.tContent, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }

        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

        SetLcdWindowTimeout(sWinLcd, 10);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }

    if (flag == LCD_SMALL_WINDOW_REPORT)
    {
        LCD_RenewEnable(sWinLcd->window, 0);
        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pageReportWrong.tTip);

        ret = ParseLFOfText(lang.pageReportWrong.tContent, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }

        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

        SetLcdWindowTimeout(sWinLcd, 20);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }

    if (flag == LCD_SMALL_WINDOW_VALID_LPN)  // 车牌有效提示
    {
        LCD_RenewEnable(sWinLcd->window, 0);
        // 图标
       // LCD_DisplayImage(sWinLcd->window, LCD_PAGE_SMALL, SMALL_IMG_TIP, defSmallNoticeFile);
        //LCD_ObjectControl(sWinLcd->window, SMALL_IMG_TIP, LCD_OBJ_SHOW);

        // 标题
        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pageLPNNotice.tValidLPNTitle);

        // 提示文本
        ret = ParseLFOfText(lang.pageLPNNotice.tValidLPN, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }

        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

        SetLcdWindowTimeout(sWinLcd, 10);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }

    if (flag == LCD_SMALL_WINDOW_NOTFOUND_LPN)
    {
        LCD_RenewEnable(sWinLcd->window, 0);
        // 图标
        LCD_DisplayImage(sWinLcd->window, LCD_PAGE_SMALL, SMALL_IMG_TIP, defSmallNoticeFile);
        LCD_ObjectControl(sWinLcd->window, SMALL_IMG_TIP, LCD_OBJ_SHOW);

        // 标题
        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pageLPNNotice.tValidLPNTitle);
        LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_HEAD, LCD_OBJ_SHOW);

        // 提示文本
        ret = ParseLFOfText(lang.pageLPNNotice.tValidLPN, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }


        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

        SetLcdWindowTimeout(sWinLcd, 10);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }

    if (flag == LCD_SMALL_WINDOW_TIMEOUT) // 超时
    {
        LCD_RenewEnable(sWinLcd->window, 0);
        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pageTimeout.tTip);
       // LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_HEAD, LCD_OBJ_SHOW);

        ret = ParseLFOfText(lang.pageTimeout.tContent, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }

        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

        SetLcdWindowTimeout(sWinLcd, 30);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }

    if (flag == LCD_SMALL_WINDOW_PASSWD_ERROR) // 超时
    {
        LCD_RenewEnable(sWinLcd->window, 0);
        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pagePwdError.tTip);

        ret = ParseLFOfText(lang.pagePwdError.tContent, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }

        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

        SetLcdWindowTimeout(sWinLcd, 20);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }

    if (flag == LCD_SMALL_WINDOW_RETURN_COIN)   // 退币重投
    {
        LCD_RenewEnable(sWinLcd->window, 0);
        LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_HEAD, lang.pageReturnCoin.tTip);

        ret = ParseLFOfText(lang.pageReturnCoin.tContent, (char *)list, 128);
        for (int i = 0; i < ret; i++)
        {
            if (i < 2)
            {
                LCD_DisplayText(sWinLcd->window, LCD_PAGE_SMALL, SMALL_TEXT_LINE1 + i, list[i]);
                LCD_ObjectControl(sWinLcd->window, SMALL_TEXT_LINE1 + i, LCD_OBJ_SHOW);
            }
        }

        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_ENTER, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_RETRY, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_LEFT_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_RIGHT_CONTINUE, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);
        LCD_ObjectControl(sWinLcd->window, SMALL_BUTTON_CENTER_CANCEL, LCD_OBJ_HIDE | LCD_OBJ_DISABLE);

        SetLcdWindowTimeout(sWinLcd, 20);
        LCD_RenewEnable(sWinLcd->window, 1);
        LCD_SelectPage(sWinLcd->window, LCD_PAGE_SMALL);
    }
    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDMaintainCashFeed
Description	: 更新维护投币页面
Input		: cf - 投币信息
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDMaintainCashFeed(LCDCashBox_t *cf)
{
    char buffer[32];

    memset(&mfWinLcd->box, 0, sizeof(LCDCashBox_t));
    memcpy(&mfWinLcd->box, cf, sizeof(LCDCashBox_t));

    // 更新一下keybuffer
    memset(&mfWinLcd->keyInput, 0, sizeof(KeyInput_t));
    sprintf(mfWinLcd->keyInput.buffer, "%d", cf->quantity);
    mfWinLcd->keyInput.cnt = strlen(mfWinLcd->keyInput.buffer);

    LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_CURRENCY, syscfg.park.currency);
    LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_TEXT_CURRENCY, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d", cf->denomination);
    LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_DONAMINATION, buffer);
    LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_TEXT_DONAMINATION, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d", cf->quantity);
    LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_QUANTITY, buffer);
    LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_TEXT_QUANTITY, LCD_OBJ_SHOW);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d", cf->amount);
    LCD_DisplayText(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED, MAINTAINFEED_TEXT_AMOUNT, buffer);
    LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_TEXT_AMOUNT, LCD_OBJ_SHOW);

    LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_KEYPAD_NUM1, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);

    if (cf->amount > 0)
    {
        LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_BUTTON_CONFIRM, LCD_OBJ_SHOW | LCD_OBJ_ENABLE);
    } else {
        LCD_ObjectControl(mfWinLcd->window, MAINTAINFEED_BUTTON_CONFIRM, LCD_OBJ_SHOW | LCD_OBJ_DISABLE);
    }

    SetLcdWindowTimeout(mfWinLcd, -1);
    LCD_SelectPage(mfWinLcd->window, LCD_PAGE_MAINTAIN_FEED);

    return 0;
}

/*-----------------------------------------------------------------------------
Function	: LCDMaintainCashFeed
Description	: 更新维护投币页面
Input		: cf - 投币信息
Output		: None
Return		: 0 success
			  -1 fail
Note		: None
------------------------------------------------------------------------------*/
int32_t LCDUpdateAdvertising(int type, void * data)
{
    LCDADVideo_t  *adVideo =  NULL;
    if (data == NULL) return -1;

    if (type == LCD_AD_TYPE_IMAGE)
    {
        LCD_DisplayImage(mWinLcd->window->win, LCD_PAGE_COMMON, SYS_IMG_ADIMG, (char *)data);
    }
    else {
        adVideo = (LCDADVideo_t *)data;
        LCD_DisplayVideo(mWinLcd->window->win, LCD_PAGE_COMMON, SYS_VIDEO_ADVIDEO,
                         adVideo->w, adVideo->h, adVideo->data, adVideo->size);
    }

    return 0;
}
/******************************** END OF FILE *********************************/
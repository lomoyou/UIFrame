/*******************************************************************************
Copyright (C) 2021, Cytel (Shanghai) Ltd.
FileName	: lcd.h
Author		: 唐红
Version		: 0.0.1
Date		: 2022/12/8
Description	: None
History		: None
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OVERSEASAPS_LCD_H /* OVERSEASAPS_LCD_H */
#define OVERSEASAPS_LCD_H /* OVERSEASAPS_LCD_H */
/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "LCDCore.h"
/* Exported types ------------------------------------------------------------*/

typedef struct {
    uint32_t page;           // 页面
    uint32_t id;             // 按钮ID
    uint8_t data[128];       // 按钮键值
} LCDButtonEvent_t;


typedef struct {
	char name[128];
	char result;
} LCDSelfCheck_t;

typedef struct {
    char actionTime[64];
    char event[64];
    uint32_t val;
} LCDLTDetails_t;

typedef struct {
    int value[11];
    int qty[11];
    int amount[11];
    int bReceiverCap;
    int BDispenserCap[4];
    int coinHopperCap;
    int coinStoreCap;
    int totalAmount;
} LCDStatement_t;

typedef struct {
    char shift[64];
    char operatorStr[64];
    char time[128];
} LCDHistoryList_t;

// 车牌模糊查询结果
typedef struct
{
    char license[32];   // 车牌号
    char soldid[64];
    char enterTime[64]; // 入场时间
    char remote[128];   // 图片云端路径
    char small[128];    // 车辆图片路径-小
    char large[128];    // 车辆图片路径-大
} LCDResultList_t;

// 车牌修正信息
typedef struct {
    char license[32];
    char image[128];
    char enterTime[64]; // 入场时间
} LCDRevise_t;

// 车牌费用信息
typedef struct
{
    char license[32];
    char image[128];
    char enterTime[64];     // 入场时间
    char periodTime[64];    // 立场截止时间
    uint32_t price;         // 总停车费用
    uint32_t discount;      // 折扣
    uint32_t paid;          // 已付
    uint32_t balance;       // 需付 =  price - paid - discount
    uint32_t duration;	    // 停车时长, 分钟
    double vat;             // 税率
} LCDPayment_t;

// 投币支付信息
typedef struct {
    uint32_t amount;	// 实现需要投币的金额
    uint32_t paid;	    // 每次投币的金额
    uint32_t balance;	// 还需投币金额
    uint32_t change;	// 找零金额
    char image[128];    // 提示图片地址
} LCDFeedPay_t;

// 上次交易
typedef struct {
    char license[32];    // 车牌
    char soldid[64];
    uint32_t needPay;
    uint32_t coinPay;
    uint32_t billPay;
    uint32_t change;
    uint32_t aChange;   // already change;
    uint32_t balance;
} LCDLastTransaction_t;

// 零钱箱装载
typedef struct {
    uint32_t denomination;
    uint32_t quantity;
    uint32_t amount;
} LCDCashBox_t;

// LCD视频广告数据
typedef struct {
    int w;
    int h;
    int size;
    char *data;
}LCDADVideo_t;

/* Exported define -----------------------------------------------------------*/
/* LCD 页面ID定义                              */
#define LCD_PAGE_INIT                     0x00          // 初始化
#define LCD_PAGE_STANDBY                  0x01          // 设备就绪
#define LCD_PAGE_QUERY                    0x02          // 费用查询输入车牌
#define LCD_PAGE_RESULT                   0x03          // 查询结果列表
#define LCD_PAGE_REVISE                   0x04          // 车牌修正
#define LCD_PAGE_PAYMENT                  0x05          // 停车费展示&电子支付
#define LCD_PAGE_FEED                     0x06          // 投币
#define LCD_PAGE_MLOG                     0x07          // 维护登录
#define LCD_PAGE_MAINTAIN                 0x08          // 维护
#define LCD_PAGE_LASTTRANSACTION          0x09          // 上次交易页面
#define LCD_PAGE_LTDETAIL                 0x0A          // 上次交易详情
#define LCD_PAGE_SELFCHECK                0x0B          // 自检页面
#define LCD_PAGE_PAYRESULT                0x0C          // 支付结果页面
#define LCD_PAGE_SMALL                    0x0D          // 弹出小窗提示页面
#define LCD_PAGE_MAINTAIN_FEED            0x0E          // 维护投币页面
#define LCD_PAGE_MAINTAIN_FEED_KEYPAD     0x0F          // 维护投币键盘
#define LCD_PAGE_STATEMENT                0x10          // 投币维护预览界面
#define LCD_PAGE_HISTORY                  0x11          // 历史记录
#define LCD_PAGE_COMMON                   0xFF          // 公共页面
/* LCD 事件类型定义
#define LCD_EVENT_TS_PRESS                1             // 用户点击按钮
#define LCD_EVENT_TS_RELEASE              2             // 用户释放按钮
#define LCD_EVENT_TS_MULTI                3             // 三点触控

/* LCD 事件类型定义                            */
#define LCD_OBJ_HIDE                      0x00          // LCD 对象显示控制
#define LCD_OBJ_SHOW                      0x01          // LCD 对象显示控制
#define LCD_OBJ_DISABLE                   0x00          // LCD 对象(仅 BUTTON/KEYPAD )功能控制
#define LCD_OBJ_ENABLE                    0x02          // LCD 对象(仅 BUTTON/KEYPAD )功能控制

/* 以下显示LCD对象ID规则
   0x 00 00 00 00
       |  |  |  |_____ 序号 0-255
       |  |  |_____ 保留
       |  |_______ 所属页面 ID 参考 LCD_PAGE_XXXX , 255为所有页面
       |_________ 类型 0 文本 1 二维码 2 图片 3 按键  4 键盘 5 视频 FF 保留
*/

/* 全局内容                 */
#define SYS_TEXT_PARKING                  0x00FF0000                // 停车场名称
#define SYS_TEXT_DEVICE                   0x00FF0001                // 设备名称
#define SYS_TEXT_DATE                     0x00FF0002                // 日期
#define SYS_TEXT_TIME                     0x00FF0003                // 时间
#define SYS_TEXT_TIMEOUT                  0x00FF0004                // 超时时间
#define SYS_TEXT_BTIP                     0x00FF0005                // 底部备注
#define SYS_IMG_BACKGROUND                0x02FF0001                // 背景图
#define SYS_IMG_ADIMG                     0x02FF0002                // 图片图
#define SYS_VIDEO_ADVIDEO                 0x05FF0000                // 视频广告

/* LCD_PAGE_QUERY          */
#define QUERY_TEXT_TIP                    0x00020000        // 按键输入提示
#define QUERY_TEXT_INPUT                  0x00020001        // 车牌显示
#define QUERY_TEXT_NOTICE                 0x00020002        // notice
#define QUERY_TEXT_INSTRUCTIONS           0x00020003        // 说明提示文本
#define QUERY_TEXT_TIP1                   0x00020004
#define QUERY_TEXT_TIP2                   0x00020005
#define QUERY_TEXT_TIP3                   0x00020006
#define QUERY_TEXT_TIP4                   0x00020007
#define QUERY_TEXT_TIP5                   0x00020008
#define QUERY_TEXT_TIP6                   0x00020009

#define QUERY_IMG_IMG                     0x02020000        // 输入框背景图
#define QUERY_IMG_INSTRUCTIONS            0x02020001        // 说明背景
#define QUERY_BUTTON_CLEAR                0x03020000        // 回退
#define QUERY_KEYPAD_EN                   0x04020004        // 英文



/* LCD_PAGE_RESULT         */
#define RESULT_TEXT_TLPN0                 0x00030000
#define RESULT_TEXT_TLPN1                 0x00030001
#define RESULT_TEXT_TLPN2                 0x00030002
#define RESULT_TEXT_TLPN3                 0x00030003
#define RESULT_TEXT_TLPN4                 0x00030004
#define RESULT_TEXT_TLPN5                 0x00030005
#define RESULT_TEXT_TLPN6                 0x00030006
#define RESULT_TEXT_TLPN7                 0x00030007
#define RESULT_TEXT_TLPN8                 0x00030008

#define RESULT_TEXT_LPN0                  0x00030009
#define RESULT_TEXT_LPN1                  0x0003000A
#define RESULT_TEXT_LPN2                  0x0003000B
#define RESULT_TEXT_LPN3                  0x0003000C
#define RESULT_TEXT_LPN4                  0x0003000D
#define RESULT_TEXT_LPN5                  0x0003000E
#define RESULT_TEXT_LPN6                  0x0003000F
#define RESULT_TEXT_LPN7                  0x00030010
#define RESULT_TEXT_LPN8                  0x00030011

#define RESULT_TEXT_TIP1                  0x00030012
#define RESULT_TEXT_TIP2                  0x00030013

#define RESULT_TEXT_TETIME0               0x00030014
#define RESULT_TEXT_TETIME1               0x00030015
#define RESULT_TEXT_TETIME2               0x00030016
#define RESULT_TEXT_TETIME3               0x00030017
#define RESULT_TEXT_TETIME4               0x00030018
#define RESULT_TEXT_TETIME5               0x00030019
#define RESULT_TEXT_TETIME6               0x0003001A
#define RESULT_TEXT_TETIME7               0x0003001B
#define RESULT_TEXT_TETIME8               0x0003001C

#define RESULT_TEXT_ETIME0                0x0003001D
#define RESULT_TEXT_ETIME1                0x0003001E
#define RESULT_TEXT_ETIME2                0x0003001F
#define RESULT_TEXT_ETIME3                0x00030020
#define RESULT_TEXT_ETIME4                0x00030021
#define RESULT_TEXT_ETIME5                0x00030022
#define RESULT_TEXT_ETIME6                0x00030023
#define RESULT_TEXT_ETIME7                0x00030024
#define RESULT_TEXT_ETIME8                0x00030025


#define RESULT_BUTTON_IMG0                0x03030000
#define RESULT_BUTTON_IMG1                0x03030001
#define RESULT_BUTTON_IMG2                0x03030002
#define RESULT_BUTTON_IMG3                0x03030003
#define RESULT_BUTTON_IMG4                0x03030004
#define RESULT_BUTTON_IMG5                0x03030005
#define RESULT_BUTTON_IMG6                0x03030006
#define RESULT_BUTTON_IMG7                0x03030007
#define RESULT_BUTTON_IMG8                0x03030008

#define RESULT_BUTTON_PREV                0x03030009
#define RESULT_BUTTON_NEXT                0x0303000A
#define RESULT_BUTTON_HOME                0x0303000B

/* LCD_PAGE_REVISE          */
#define REVISE_TEXT_CAR                   0x00040000
#define REVISE_TEXT_TLPN                  0x00040001
#define REVISE_TEXT_LPN                   0x00040002
#define REVISE_TEXT_TETIME                0x00040003
#define REVISE_TEXT_ETIME                 0x00040004
#define REVISE_TEXT_TLINE1                0x00040005
#define REVISE_TEXT_TLINE2                0x00040006
#define REVISE_TEXT_TLINE3                0x00040007
#define REVISE_TEXT_TLINE4                0x00040008


#define REVISE_IMG_IMG                    0x02040000

#define REVISE_BUTTON_RETURN              0x03040000
#define REVISE_BUTTON_REPORT              0x03040001

/* LCD_PAGE_PAYMENT        */
#define PAYMENT_TEXT_CAR                  0x00050000
#define PAYMENT_TEXT_TLPN                 0x00050001
#define PAYMENT_TEXT_LPN                  0x00050002
#define PAYMENT_TEXT_TETIME               0x00050003
#define PAYMENT_TEXT_ETIME                0x00050004            // 入场时间
#define PAYMENT_TEXT_TDURATION            0x00050005
#define PAYMENT_TEXT_DURATION             0x00050006            // 停车时长
#define PAYMENT_TEXT_TFEE                 0x00050007
#define PAYMENT_TEXT_FEE                  0x00050008
#define PAYMENT_TEXT_TDISCOUNT            0x00050009
#define PAYMENT_TEXT_DISCOUNT             0x0005000A
#define PAYMENT_TEXT_TPAID                0x0005000B
#define PAYMENT_TEXT_PAID                 0x0005000C
#define PAYMENT_TEXT_TBALANCE             0x0005000D
#define PAYMENT_TEXT_BALANCE              0x0005000E            // 结欠
#define PAYMENT_TEXT_TPERIOD              0x0005000F            // 出场有效时间
#define PAYMENT_TEXT_PERIOD               0x00050010
#define PAYMENT_TEXT_TVAT                 0x00050013
#define PAYMENT_TEXT_VAT                  0x00050014
#define PAYMENT_IMG_IMG                   0x02050000

#define PAYMENT_BUTTON_PAYLATER           0x03050000
#define PAYMENT_BUTTON_PAY                0x03050001

/* LCD_PAGE_FEED        */
#define FEED_TEXT_TIP                     0x00060000
#define FEED_TEXT_TAMOUNT                 0x00060001
#define FEED_TEXT_AMOUNT                  0x00060002
#define FEED_TEXT_TPAID                   0x00060003
#define FEED_TEXT_PAID                    0x00060004
#define FEED_TEXT_TBALANCE                0x00060005
#define FEED_TEXT_BALANCE                 0x00060006
#define FEED_TEXT_NOTICE                  0x00060009

#define FEED_IMG_IMG                      0x02060000

#define FEED_BUTTON_CANCEL                0x03060000

/* LCD_PAGE_MLOG           */
#define MLOG_TEXT_TIP                     0x00070000
#define MLOG_TEXT_TID                  	  0x00070001
#define MLOG_TEXT_ID                   	  0x00070002
#define MLOG_TEXT_TPASSWORD     	      0x00070003
#define MLOG_TEXT_PASSWOED          	  0x00070004
#define MLOG_IMG_ID                       0x02070005
#define MLOG_IMG_PASSWORD                 0x02070006
#define MLOG_BUTTON_ENTER                 0x03070000
#define MLOG_BUTTON_HOME                  0x03070001

#define MLOG_KEYPAD_UPPER_EN              0x04070000
#define MLOG_KEYPAD_LOWER_EN              0x04070001
#define MLOG_KEYPAD_SYMBOL                0x04070002

/* LCD_PAGE_MAINTAIN        */
#define MAINTAIN_TEXT_TIP                 0x00080000
#define MAINTAIN_BUTTON_LAST              0x03080001
#define MAINTAIN_BUTTON_SLEFCHECK         0x03080002
#define MAINTAIN_BUTTON_STATEMENT         0x03080003
#define MAINTAIN_BUTTON_HISTORY           0x03080004
#define MAINTAIN_BUTTON_LOGOUT            0x03080005
#define MAINTAIN_BUTTON_6                 0x03080006
#define MAINTAIN_BUTTON_7                 0x03080007

/* LCD_PAGE_LASTTRANSACTION */
#define LASTTRANSACTION_TEXT_TIP               0x00090000
#define LASTTRANSACTION_TEXT_TLPN              0x00090001
#define LASTTRANSACTION_TEXT_LPN               0x00090002
#define LASTTRANSACTION_TEXT_TNEEDPAY          0x00090003
#define LASTTRANSACTION_TEXT_PAY               0x00090004
#define LASTTRANSACTION_TEXT_TCOINPAY          0x00090005
#define LASTTRANSACTION_TEXT_COINPAY           0x00090006
#define LASTTRANSACTION_TEXT_TBILLPAY          0x00090007
#define LASTTRANSACTION_TEXT_BILLPAY           0x00090008
#define LASTTRANSACTION_TEXT_TNEEDCHANGE       0x00090009
#define LASTTRANSACTION_TEXT_NEEDCHANGE        0x0009000A
#define LASTTRANSACTION_TEXT_TALCHANGE         0x0009000B
#define LASTTRANSACTION_TEXT_ALCHANGE          0x0009000C
#define LASTTRANSACTION_TEXT_TBALANCE          0x0009000D
#define LASTTRANSACTION_TEXT_BALANCE           0x0009000E

#define LASTTRANSACTION_BUTTON_MORE            0x03090000
#define LASTTRANSACTION_BUTTON_RETURN          0x03090001

/* LCD_PAGE_LTDETAIL */
#define LTDETAIL_TEXT_TIP                      0x000A0000
#define LTDETAIL_BUTTON_RETURN                 0x030A0000

/* LCD_PAGE_SELFCHECK */
#define SELFCHECK_TEXT_TIP                     0x000B0000
#define SELFCHECK_BUTTON_RETURN                0x030B0000

/* LCD_PAGE_STATEMENT */
#define STATEMENT_TEXT_CASHBOX                 0x00100000
#define STATEMENT_TEXT_CURRENCY                0x00100001
#define STATEMENT_TEXT_BOXES                   0x00100002
#define STATEMENT_TEXT_VALUES                  0x00100003
#define STATEMENT_TEXT_QTY                     0x00100004
#define STATEMENT_TEXT_AMOUNT                  0x00100005
#define STATEMENT_TEXT_CAP                     0x00100006
#define STATEMENT_TEXT_OPERATE                 0x00100007
#define STATEMENT_TEXT_BRECEIVER               0x00100008
#define STATEMENT_TEXT_BDISPENSER1             0x00100009
#define STATEMENT_TEXT_BDISPENSER2             0x0010000A
#define STATEMENT_TEXT_BDISPENSER3             0x0010000B
#define STATEMENT_TEXT_BDISPENSER4             0x0010000C
#define STATEMENT_TEXT_HOOPER                  0x0010000D
#define STATEMENT_TEXT_STORE                   0x0010000E
#define STATEMENT_TEXT_TOTAL                   0x0010000F
#define STATEMENT_TEXT_TOTAL_AMOUNT            0x00100010
#define STATEMENT_BUTTON_EXIT                  0x03100000
#define STATEMENT_BUTTON_CLEAN1                0x03100001
#define STATEMENT_BUTTON_FEED1                 0x03100002
#define STATEMENT_BUTTON_FEED2                 0x03100003
#define STATEMENT_BUTTON_FEED3                 0x03100004
#define STATEMENT_BUTTON_FEED4                 0x03100005
#define STATEMENT_BUTTON_FEED5                 0x03100006
#define STATEMENT_BUTTON_CLEAN2                0x03100007


/* LCD_PAGE_HISTORY */
#define HISTORY_TEXT_TIP                       0x00110000
#define HISTORY_TEXT_SHIFT                     0x00110001
#define HISTORY_TEXT_OPERATOR                  0x00110002
#define HISTORY_TEXT_TIME                      0x00110003
#define HISTORY_TEXT_PRINT                     0x00110004
#define HISTORY_BUTTON_RETURN                  0x031100FF

/* LCD_PAGE_PAYRESULT */
#define PAYRESULT_IMG_TIP                      0x020C0000
#define PAYRESULT_TEXT_TIP1                    0x000C0000
#define PAYRESULT_TEXT_TIP2                    0x000C0001
#define PAYRESULT_IMG_NET                      0x020C0001
#define PAYRESULT_TEXT_TIP3                    0x000C0002




// LCD_PAGE_MAINTAIN_FEED
#define MAINTAINFEED_IMG_BACKGROUND                0x02FF0000
#define MAINTAINFEED_IMG_INPUT                     0x02FF0001
#define MAINTAINFEED_TEXT_TIP                      0x000E0000
#define MAINTAINFEED_TEXT_TIMEOUT                  0x000E0001
#define MAINTAINFEED_TEXT_TCURRENCY                0x000E0002
#define MAINTAINFEED_TEXT_CURRENCY                 0x000E0003
#define MAINTAINFEED_TEXT_TDONAMINATION            0x000E0004
#define MAINTAINFEED_TEXT_DONAMINATION             0x000E0005
#define MAINTAINFEED_TEXT_TQUANTITY                0x000E0006
#define MAINTAINFEED_TEXT_QUANTITY                 0x000E0007
#define MAINTAINFEED_TEXT_TAMOUNT                  0x000E0008
#define MAINTAINFEED_TEXT_AMOUNT                   0x000E0009
#define MAINTAINFEED_TEXT_KEYPAD                   0x000E000A
#define MAINTAINFEED_BUTTON_CANCEL                 0x030E0000
#define MAINTAINFEED_BUTTON_CONFIRM                0x030E0001
#define MAINTAINFEED_KEYPAD_NUM1                   0x040E0000

enum {
        LCD_EVENT_MAIN_TIMEOUT = 0x01,         // 主页面超时
        LCD_EVENT_SMALL_TIMEOUT,               // 提示小窗超时
        LCD_EVENT_CASHFEED_TIMEOUT,            // 现金投币维护页面超时
        LCD_EVENT_BUTTON,                      // 按钮按下
        LCD_EVENT_LICENSE,                     // 车牌输入完成
        LCD_EVENT_STAFF_ID,                    // 账号输入完成
        LCD_EVENT_PASSWORD ,                   // 密码输入完成
        LCD_EVENT_REVISE,                      // 车牌修正
        LCD_EVENT_CASHFEED                     // 维护现金投币
};


// LCD_PAGE_SMALL
#define SMALL_IMG_BACKGROUND                   0x02FF0000
#define SMALL_IMG_TIP                          0x02FF0001
#define SMALL_TEXT_HEAD                        0x000D0000
#define SMALL_TEXT_LINE1                       0x000D0001
#define SMALL_TEXT_LINE2                       0x000D0002
#define SMALL_BUTTON_LEFT_CANCEL               0x030D0000
#define SMALL_BUTTON_RIGHT_CONTINUE            0x030D0001
#define SMALL_BUTTON_CENTER_CANCEL             0x030D0002
#define SMALL_BUTTON_RIGHT_RETRY               0x030D0003
#define SMALL_BUTTON_CENTER_ENTER              0x030D0004

// 智能小窗提示
enum smallWindowType {
    LCD_SMALL_WINDOW_REPORT  = 0x01,        // 修正提示
    LCD_SMALL_WINDOW_TIMEOUT,               // 修正超时提示
    LCD_SMALL_WINDOW_NO_NOTE,               // 无币找零提示
    LCD_SMALL_WINDOW_VALID_LPN,             // 车牌有效位数不符提示
    LCD_SMALL_WINDOW_NOTFOUND_LPN,          // 车牌查询不到提示
    LCD_SMALL_WINDOW_PASSWD_ERROR,          // 密码错误提示
    LCD_SMALL_WINDOW_RETURN_COIN,           // 退币重投
};
enum lcdWindowID {
    LCD_WINDOW_MAIN = 0,
    LCD_WINDOW_SMALL,
    LCD_WINDOW_MAINTAINFEED,
};

enum noticePageID {
    LCD_NOTICE_PAGE_SUCCESS = 0x01,         // 支付成功
    LCD_NOTICE_PAGE_DISCONNECT,             // 断开连接
    LCD_NOTICE_PAGE_CHANGE,                 // 找零
};

enum lcdADType{
    LCD_AD_TYPE_IMAGE = 0,
    LCD_AD_TYPE_VIDEO,
};  /* LCD 广告类型 */

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
int32_t InitMainWindow                      (void);
int32_t MainWindowSetUserCallback           (DrvEventCb_t cb);
int32_t InitSmallWindow                     (void);
int32_t DestroySmallWindow                  (void);
int32_t SmallWindowSetUserCallback          (DrvEventCb_t cb);
int32_t InitMaintainFeedWindow              (void);
int32_t DestroyMaintainFeedWindow           (void);
int32_t MaintainFeedWindowSetUserCallback   (DrvEventCb_t cb);
int32_t InitKeypadWindow                    (void);
int32_t DestroyKeypadWindow                 (void);
int32_t KeypadWindowSetUserCallback         (DrvEventCb_t cb);

int32_t LCDLpnQueryPage			(char *notice);
int32_t LCDResultListPage	    (LCDResultList_t *list, int count);
int32_t LCDPaymentInfoPage      (LCDPayment_t *lpn);
int32_t LCDRevisePage	        (LCDRevise_t *lpn);
int32_t LCDCashFeedPage			(LCDFeedPay_t *lpn);
int32_t LCDUpdateCashFeedPage   (LCDFeedPay_t *lpn);
int32_t LCDLoginPage			(void);
int32_t LCDMaintainPage			(void);
int32_t LCDSelfCheckPage        (LCDSelfCheck_t * list, int count);
int32_t LCDUpdateSelfCheckResult(int num, int result);
int32_t LCDLTDetailsPage        (LCDLTDetails_t * list, int count);
int32_t LCDStatementPage        (LCDStatement_t *statement);
int32_t LCDHistoryPage          (LCDHistoryList_t *list, int count);
int32_t LCDNoticePage           (int result);
int32_t LCDLastTransaction		(LCDLastTransaction_t *lpn);
int32_t LCDSmallWindowPage      (int  flag);
int32_t LCDUpdateAdvertising    (int type, void * data);
int32_t LCDMaintainCashFeed     (LCDCashBox_t *cf);
int32_t LCDWindowControl        (int windowID);
#endif /* OVERSEASAPS_LCD_H */
/******************************** END OF FILE *********************************/
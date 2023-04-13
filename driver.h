/*******************************************************************************
Copyright (C) 2021, Cytel (Shanghai) Ltd.
FileName	: driver.h
Author		: 唐红
Version		: 0.0.1
Date		: 2022/11/9
Description	: None
History		: None
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OVERSEASAPS_DRIVER_H /* OVERSEASAPS_DRIVER_H */
#define OVERSEASAPS_DRIVER_H /* OVERSEASAPS_DRIVER_H */
/* Includes ------------------------------------------------------------------*/
#include "includes.h"
/* Exported types ------------------------------------------------------------*/
typedef struct
{
    uint32_t event;     /* 事件类型 */
    uint64_t ts;        /* 事件戳   */
    uint32_t dlen;      /* 数据长度 */
    void * data;        /* 数据    */
    void * callback;    /* 回调函数本身*/
} DrvEvent_t;


// 现金支付信息
typedef struct
{
    uint32_t fee;		// 总停车费用
    uint32_t amount;	// 本次应交费用
    uint32_t paid;		// 已投金额
    uint32_t change;	// 找零金额
} CashPay_t;

// 出钞设备事件信息
typedef struct {
    uint8_t id;             // 出钞设备ID
    uint8_t need;           // 应该出钞数量
    uint8_t actual;         // 实际出钞数量
    uint8_t data[128];      // 故障和状态信息
} BillDispenseEvent_t;

typedef void ( *DrvEventCb_t ) ( DrvEvent_t * ev );
/* Exported define -----------------------------------------------------------*/

/* 打印机事件 */
#define PRINTER_EVENT_WARN            	0x1000        // 打印机警告
#define PRINTER_EVENT_FAULT             0x1001        // 打印机故障
#define PRINTER_EVENT_NORMAL            0x1002        // 打印机正常
/* 纸币机事件 */
#define BILL_EVENT_COMPLETE             0x1003        // 投币成功
#define BILL_EVENT_WARN                 0x1004        // 纸币机警告
#define BILL_EVENT_FAULT        	    0x1005        // 纸币机故障
#define BILL_EVENT_NORMAL               0x1006        // 纸币识别器正常

/* 硬币机事件 */
#define DISPENSER_EVENT_COMPLETE        0x1007        // 纸币找零器A 吐币成功
#define DISPENSER_EVENT_NORMAL          0x1008        // 纸币找零器A 正常
#define DISPENSER_EVENT_FAULT           0x1009        // 纸币找零器A 故障
#define EXTENDER_EVENT_FAULT            0x100A        // IO拓展板故障
#define EXTENDER_EVENT_DATA             0x100B        // IO拓展板响应数据
#define EXTENDER_EVENT_CHANGE           0x100C        // 硬币找零完成
#define EXTENDER_EVENT_RETURN           0x100D        // 打开退币器，页面提示重新投币

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
int DriverInit(void);
#endif /* OVERSEASAPS_DRIVER_H */
/******************************** END OF FILE *********************************/
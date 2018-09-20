#ifndef __POWER_INNER_H__
#define __POWER_INNER_H__

#include <linux/rtc.h>
#include <drv_chg.h>
#include <drv_onoff.h>
#include <power_com.h>
#include <bsp_softtimer.h>
#if (MBB_CHG_PLATFORM_V7R2 == FEATURE_ON)
#include "drv_chg.h"
#endif
/*******************************************************************************
* 开机结构体、枚举定义
********************************************************************************/
typedef enum power_on_property {
	POWER_ON_PROP_STATUS = 0,
	POWER_ON_PROP_REASON,
	POWER_ON_PROP_TIME
}power_sysinfo;

typedef struct
{
    DRV_START_MODE           mode;
    DRV_START_REASON         reason;
    struct rtc_time         time;
    struct softtimer_list   on_timer;
}power_on_ctrl_s;

/********************************************************************************
* 开机函数声明
*********************************************************************************/

/* stub */
extern BATT_LEVEL_ENUM chg_get_batt_level(void);

#if (MBB_CHG_PLATFORM_V7R2 == FEATURE_OFF)
extern int chg_get_batt_level(void);
#endif
/********************************************************************************
* 关机结构体、枚举定义
*********************************************************************************/

typedef struct
{
    DRV_SHUTDOWN_REASON_ENUM    reason;
    struct rtc_time            time;
	struct softtimer_list      off_timer;
}power_off_ctrl_s;

/* 开关机宏定义 */
#if (MBB_CHG_PLATFORM_V7R2 == FEATURE_ON)
#define POWER_OFF_MONOTER_TIMEROUT      (25 * 1000)/*超时关机时间*/
#else
#define POWER_OFF_MONOTER_TIMEROUT      (5 * 1000)
#endif
#define POWER_ON_MONITOR_TIMEROUT       (60*1000)

#define pr_dbg printk

#define EXCH_RESET_LOG_PATH     "/modem_log/poweroff.txt"
#define EXCH_POWER_LOG_PATH     "/modem_log/poweron.txt"
#define EXCH_ONOFF_LOG_MAX      (0x400*16)

/********************************************************************************
* 关机函数声明
*********************************************************************************/

#endif


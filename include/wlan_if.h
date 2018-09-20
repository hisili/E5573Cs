


#ifndef _wlan_if_h_
#define _wlan_if_h_

#include <linux/signal.h>
#include "wlan_utils.h"
#include <linux/interrupt.h>
#if (FEATURE_ON == MBB_WIFI_CHIP_REGULAR)
#include "wlan_drv_if.h"
#endif

#if ((rtl8192 == MBB_WIFI_CHIP1) || (rtl8189 == MBB_WIFI_CHIP1))
#define    WLAN_CACHE_CFGFILE_SIZE         4
typedef struct wlan_mem_cache{
    void *recv;                                  /* 驱动中接收文件缓存 */
    void *cfgfile[WLAN_CACHE_CFGFILE_SIZE];      /* 驱动中的cfgfile缓存 */
}wlan_mem_cache_st;
#endif

#define    WLAN_TRACE_INFO(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define    WLAN_TRACE_ERROR(fmt, ...)    printk(fmt, ##__VA_ARGS__)

/* 判断是否采用SDIO接口的WiFi芯片 */
#define   IS_SDIO_CHIP(chip) ((chip == bcm43362) \
                            || (chip == bcm4354)   \
                            || (chip == rtl8189)   \
                            || (chip == rtl8192))  
#define  CONFIG_WIFI_SDIO (IS_SDIO_CHIP(MBB_WIFI_CHIP1) || IS_SDIO_CHIP(MBB_WIFI_CHIP2)) 

/* signal report */
typedef enum
{
    WIFI_SIGNAL_UPDATE_STA_LIST    = SIGIO,   /* WiFi STA列表更新 */
    WIFI_SIGNAL_AUTO_SHUTDOWN      = SIGUSR2,   /* WiFi自动关闭信号 */
}WLAN_SIGNAL_EVENT_ENUM;

/* event report */
typedef enum _WLAN_EVENT_TYPE
{
    USER_WIFI_TIMEOUT_EVENT         = 1,         /* WiFi自动关闭消息 */
    USER_WIFI_UPDATE_STA_LIST       = 2,         /* STA接入个数更新事件 */
    USER_WIFI_DATA_DEAD_EVENT       = 32,        /* 控制导致FW异常上报 */
    USER_WIFI_CTRL_DEAD_EVENT       = 33,        /* 数传导致FW异常上报 */
    USER_WIFI_NULL_EVENT = ((unsigned int)-1),   /* 空事件 */
}WLAN_EVENT_TYPE;

typedef struct _wlan_user_event
{
    WLAN_EVENT_TYPE eventId;
    unsigned int eventVal;
} WLAN_USER_EVENT;

 /* WiFi芯片状态检测结构体,DHD中还有相同结构体的定义*/
 typedef struct
 {
     unsigned int rxerror;
     unsigned int txerror;
     unsigned int cmderror;
 }WLAN_STATUS_STU;


 /*===========================================================================
 
                         函数声明部分
 
 ===========================================================================*/

/*****************************************************************************
 函数名称  : WLAN_RETURN_TYPE wlan_signal_report(WiFi_SIGNAL_EVENT_ENUM signal)
 功能描述  : 向应用层发送信号
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 当前加载WiFi类型值
*****************************************************************************/
WLAN_RETURN_TYPE wlan_signal_report(WLAN_SIGNAL_EVENT_ENUM signal);

/*****************************************************************************
 函数名称  : WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event)
 功能描述  : 向应用层发送事件
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 当前加载WiFi类型值
*****************************************************************************/
WLAN_RETURN_TYPE wlan_event_report(WLAN_USER_EVENT *event);

/*****************************************************************************
 函数名称  : wlan_nv_read_pow
 功能描述  : 同平台获取POW的NV配置
 输入参数  : pstPow: power结构，uLen: 结构长度
 输出参数  : pstPow: power结构信息
 返 回 值  : NA
*****************************************************************************/
int wlan_nv_read_pow(void * pstPow, unsigned uLen);
/* 巴龙V7平台支持通过NV方式配置Power */
#define WLAN_NV_READ_POW(pstPow, uLen) wlan_nv_read_pow(pstPow, uLen)

/*****************************************************************************
 函数名称  : wlan_request_wakeup_irq
 功能描述  : WiFi唤醒BB 中断注册接口
 输入参数  : handler: 中断回调函数，devname: 设备名称，dev:中断相应参数
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
unsigned int wlan_request_wakeup_irq(irq_handler_t handler, const char *devname, void *dev);

/*****************************************************************************
 函数名称  : wlan_free_irq
 功能描述  : WiFi唤醒BB 中断释放接口
 输入参数  : irq: 中断id
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
void wlan_free_irq(unsigned int irq, void *dev);

/*****************************************************************************
 函数名称  : wlan_set_driver_lock
 功能描述  : WiFi drv 投票接口
 输入参数  : locked: 是否锁定系统，不允许休眠
 输出参数  : NA
 返 回 值  : locked
*****************************************************************************/
void wlan_set_driver_lock(int locked);

#if (FEATURE_ON == MBB_WIFI_CHIP_REGULAR)
/**********************************************************************
函 数 名  :bsp_get_product_wifi_chip_type
功能描述  : 判别单板的WiFi芯片类型
输入参数  : 无。
输出参数  : 无。
返 回 值     : WIFI_CHIP_TYPE 类型
注意事项  : 无。
***********************************************************************/
WIFI_CHIP_TYPE wlan_get_product_wifi_chip_type();
/**********************************************************************
函 数 名  :wlan_get_product_wifi_hw_id
功能描述  : 获取单板的硬件id
输入参数  : 无。
输出参数  : 无。
返 回 值     : 硬件id 
注意事项  : 无。
***********************************************************************/
u32   wlan_get_product_wifi_hw_id();
#endif
#endif



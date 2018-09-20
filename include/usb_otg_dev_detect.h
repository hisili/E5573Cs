

#ifndef _USB_OTG_DEV_DETECT
#define _USB_OTG_DEV_DETECT

#include <linux/workqueue.h>
#include <linux/gpio.h>
#include "usb_platform_comm.h"
#include <linux/wakelock.h>
#include "mbb_usb_adp.h"

#define GPIO_0_17                     (17)
#define GPIO_2_19                     (83)
#define GPIO_2_20                     (84)

#define GPIO_2_23                       (87)
#define GPIO_2_31	                  (95)


/*
#if (FEATURE_ON == MBB_CHG_PLATFORM_9X25)
#define GPIO_OTG_ID_DET      GPIO_0_17
#define GPIO_OTG_ID_SET      GPIO_2_19
#define GPIO_DMDP_CONNECT    GPIO_2_20
#endif
*/

#ifdef MBB_USB_UNITARY_Q
/*****9x30************/
#define GPIO_OTG_ID_DET   51
/*9x25*/
//#define GPIO_OTG_ID_DET      GPIO_0_17
//#define GPIO_OTG_ID_SET      GPIO_2_19
//#define GPIO_DMDP_CONNECT    GPIO_2_20
#else
/*此处配置 balong   GPIO */
#define GPIO_OTG_ID_DET      OTG_ID_DET_GPIO
#define GPIO_OTG_ID_SET      OTG_ID_SET_GPIO
#define GPIO_DMDP_CONNECT    DMDP_CONNECT_GPIO
#endif

#define INVALID_STATE  -1
#define USB_OTG_CONNECT_DP_DM    0x0001  /*短接 D+,D-*/
#define USB_OTG_DISCONNECT_DP_DM 0x0002  /*直连基带，拉低HS_ID*/
#define USB_OTG_ID_PULL_OUT      0x0003  /*直连基带，拉高HS_ID*/
#define USB_OTG_FAST_OFF   0x0004        /*假关机*/
#define USB_OTG_FAST_ON    0x0005       /*假开机*/

/*OTG 设备支持*/
#define USB_OTG_FEATURE_NONE 0x00
#define USB_OTG_FEATURE_CRADLE 0x01
#define USB_OTG_FEATURE_EXTCHG 0x10
#define USB_OTG_FEATURE_CRADLE_EXTCHG 0x11
#define USB_OTG_FEATURE_CRADLE_MASK 0x0f
#define USB_OTG_FEATURE_EXTCHG_MASK 0xf0


#define GPIO_HIGH   1
#define GPIO_LOW    0

#define HOST_ON 0
#define HOST_OFF 1

#define ID_FLOAT 1
#define ID_GROUND 0

#define VBUS_UP   1
#define VBUS_DOWN 0

#define PLATFORM_QUALCOMM   0
#define PLATFORM_BALONG     1

#define OTG_INSERT 1
#define OTG_REMOVE 0

#define OTG_DEV_EVENT_PROC   MBB_USB_TRUE
#define OTG_DEV_EVENT_NONPROC MBB_USB_FALSE

#if(MBB_CHG_EXTCHG == FEATURE_ON)
#define OTG_DET_LONG_DELAY_TIME        HZ  /*OTG  ID 中断防抖中断检测延时*/
#else
#define OTG_DET_LONG_DELAY_TIME        HZ/2/*USB  ID 中断防抖中断检测延时*/
#endif

#define OTG_DET_SHORT_DELAY_TIME       HZ/20  /*不要再延长*/

/*
 记录OTG 设备拔出流程，
 避免拔出时充电消息重复上报
*/
enum otg_dev_remove_process
{
    OTG_DEV_INVALID_FLAG,
    OTG_DEV_REMOVE_PROC,    /*开始处理otg 设备拔出流程*/
    OTG_DEV_REMOVE_DONE     /*otg 设备拔出引起的vbus 下电
                                                            流程中设置改标志*/
};

typedef enum _otg_device
{
    OTG_DEVICE_UNDEFINED = 0,
    OTG_DEVICE_EXTCHAGER = 1,
    OTG_DEVICE_CRADLE = 2, /*单网卡类型，实现整机充电*/
    OTG_DEVICE_CRADLE_CHARGE = 3, /*产品不支持cradle，仅充电*/
    OTG_DEVICE_MAX,
} OTG_DEVICE_TYPE;
typedef enum _cradle_type
{
    CRADLE_NULL = 0,
    CRADLE_AF35 = 1
} OTG_CRADLE_TYPE;
struct otg_debug
{
    USB_UINT stat_usb_id_insert;                   /*防抖前插入记数*/
    USB_UINT stat_usb_id_insert_proc;               /*防抖后 处理前插入记数*/
    USB_UINT stat_usb_id_insert_proc_end;          /*防抖后 处完成后插入记数*/
    USB_UINT stat_usb_id_remove;                         /*防抖前拔出记数*/
    USB_UINT stat_usb_id_remove_proc;               /*防抖后处理前拔出记数*/
    USB_UINT stat_usb_id_remove_proc_end;        /*防抖后处理后拔出记数*/
    USB_UINT stat_usb_id_no_trigger;             /*ID 电平变化没有触发中断的计数*/
    USB_UINT stat_usb_dpdm_connect;                    /*D+ D-直连*/
    USB_UINT stat_usb_dpdm_disconnect;                /*D+ D-断开*/
    USB_UINT stat_usb_id_remove_chain;                 /*ID 移除*/
    USB_UINT stat_usb_otg_fast_off;                      /*进入假关机禁用中断*/
    USB_UINT stat_usb_otg_fast_on;                       /*退出假关机使能中断*/
    USB_UINT stat_usb_kick_timeout ;                        /*防抖动jiffies*/
};
struct otg_dev
{
    OTG_DEVICE_TYPE dev_type;
    USB_INT status;
};
struct otg_dev_det
{
    USB_INT platform;                   /*平台适配*/
    /*otg detect basic elements*/
    USB_INT id;                         /*USB ID状态*/
    USB_INT old_id;
    USB_INT vbus;                       /*VUBS 状态*/
    USB_INT host;                       /*单板是否处于host 模式*/
    USB_INT phy_id;                     /*标记是否要切换到host*/

    USB_INT id_det_gpio;                /*检测OTG 设备硬件中断*/
    USB_ULONG id_irqflags;              /*id 中断触发方式设置*/
    USB_INT id_irq_shared;              /*id 中断号是否共享*/

    USB_INT notify_event;               /*上报给应用的事件*/
    USB_INT charge_type;
    USB_INT otg_remove_flag;         /*记录otg 设备拔出到vbus 下电过程*/
    // USB_INT phy_in_lpm;                 /*usb 芯片是否处于低电模式*/
    struct wake_lock id_wake_lock;
    struct delayed_work otg_id_detect_work;   /*OTG 设备检测中断下半部*/


    /* protects detect process*/
    //spinlock_t            lock;
    //spinlock_t            notify_lock;
    //wait_queue_head_t wait_wq;

    struct otg_debug debug;
    USB_UINT32 otg_feature;
    struct otg_dev  dev;
    usb_adp_ctx_t* contex;
};

int otg_dev_id_state(void);
void usb_notify_event(unsigned long val, void* v);
void otg_dev_set_remove_flags(int remove_flag);
int otg_dev_get_remove_flags(void);

USB_INT usb_otg_device_detect_init(USB_VOID);
USB_VOID usb_otg_device_detect_exit(USB_VOID);
OTG_CRADLE_TYPE usb_otg_get_cradle_type();
OTG_DEVICE_TYPE usb_otg_get_device_type();
USB_VOID usb_otg_set_device_type();
USB_VOID usb_otg_pogopin_set();
USB_VOID usb_otg_pogopin_clean();
#endif


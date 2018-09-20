



#ifndef _DWC_OTG_EXTERN_CHARGE_H
#define _DWC_OTG_EXTERN_CHARGE_H

#include <linux/workqueue.h>
#include <linux/gpio.h>
#include "usb_platform_comm.h"
#include <product_config.h>
#include <linux/wakelock.h>
#define GPIO_0_17                     (17)
#define GPIO_2_19                     (83)
#define GPIO_2_20                     (84)

#define GPIO_2_23                       (87)
#define GPIO_2_31	                  (95) 
#if (FEATURE_ON == MBB_CHG_PLATFORM_9X25)
/*高通平台GPIO端口号*/
#define GPIO_OTG_ID_DET      GPIO_0_17
#define GPIO_OTG_ID_SET      GPIO_2_19
#define GPIO_DMDP_CONNECT    GPIO_2_20
#else
/*balong平台GPIO端口号*/


#define GPIO_OTG_ID_DET      OTG_ID_DET_GPIO
#define GPIO_OTG_ID_SET      OTG_ID_SET_GPIO
#define GPIO_DMDP_CONNECT    DMDP_CONNECT_GPIO
#define GPIO_DMDP_SWITCH     OTG_ID_SWITCH_GPIO
#define GPIO_OTG_VBUS_DET    OTG_VBUS_DET_GPIO
#define CHECK_ID_VOLT_LOW       (150)
#define CHECK_ID_VOLT_HIG       (250)
#endif

#define GPIO_HIGH 1
#define GPIO_LOW 0
#define USB_OTG_FEATURE_NONE 0x00
#define USB_OTG_FEATURE_AF18 0x01
#define USB_OTG_FEATURE_EXTCHG 0x10
#define USB_OTG_FEATURE_AF18_EXTCHG 0x11
#define USB_OTG_FEATURE_AF18_MASK 0x0f
#define USB_OTG_FEATURE_EXTCHG_MASK 0xf0

#define USB_OTG_ID_INSERT     1
#define USB_OTG_ID_REMOVE    0
typedef struct otg_charger
{
    USB_BOOL  ext_chg_id_ins;
    wait_queue_head_t wait_wq;
    struct wake_lock otg_chg_wakelock;
    struct work_struct otg_chg_notify_work;
} otg_charger_st;
typedef enum _cradle_type
{
    CRADLE_NULL = 0,
    CRADLE_AF35 = 1
} OTG_CRADLE_TYPE;
typedef enum _otg_device
{
    OTG_DEVICE_NORMAL = 0,
    OTG_DEVICE_EXTCHAGER = 1,
    OTG_DEVICE_USBNET = 2, /*单网卡类型，实现整机充电*/
    OTG_DEVICE_AP_CRADLE = 3, /*带AP CRADLE类型，带wifi芯片，并实现整机充电*/
    OTG_DEVICE_MAX,
} OTG_DEVICE_TYPE;
USB_VOID otg_wake_lock(USB_VOID);
USB_VOID otg_wake_unlock(USB_VOID);
USB_VOID otg_detect_id_thread( USB_INT action );
USB_VOID usb_otg_extern_charge_event_notify( USB_BOOL  usbid_on_off);
USB_VOID usb_otg_extern_charge_init(USB_VOID);
USB_VOID usb_otg_extern_charge_remove(USB_VOID);
OTG_CRADLE_TYPE usb_otg_get_cradle_type();
OTG_DEVICE_TYPE usb_otg_get_device_type();
USB_VOID usb_otg_set_device_type();
USB_VOID usb_otg_pogopin_set();
USB_VOID usb_otg_pogopin_clean();



#endif
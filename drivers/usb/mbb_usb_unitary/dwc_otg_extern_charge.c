


#include <linux/timer.h>
#include <linux/notifier.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/usb/usb_interface_external.h>
#include "usb_vendor.h"
#include "bsp_version.h"
#include "drv_version.h"
#include "dwc_otg_extern_charge.h"
#include "usb_charger_manager.h"
#include "mbb_usb_adp.h"
#include "usb_platform_comm.h"
#include "usb_debug.h"
#include "usb_notify.h"
#include <product_config.h>
#include <mbb_config.h>
#include <bsp_hkadc.h>
#include <linux/netlink.h>
#ifdef  MBB_FAST_ON_OFF
#include "mbb_fast_on_off.h"
#else
USB_INT usb_fast_on_off_stat(USB_VOID)
{
    return  0;
}
#endif

#define DWC_OTG_AB      0xAB
#define OTG_DETECT_OVERTIME 2000
#define  INVALID_STATE  (-1)

static USB_INT  otg_old_event_id = INVALID_STATE;
USB_INT extern_otg_dev = DWC_OTG_AB;

extern struct blocking_notifier_head usb_balong_notifier_list;
static  OTG_CRADLE_TYPE g_cradle_type = CRADLE_NULL;
static OTG_DEVICE_TYPE g_device_type = OTG_DEVICE_NORMAL;
static int g_usbnet_usb_state = USB_AF35_REMOVE; //AF35插入状态
static struct class* lan_class;
static struct device* lan_dev;
struct device* get_cradle_dev(void);
void set_cradle_dev(struct device* dev);
static ssize_t get_usb_cradle_state(struct device* dev, struct device_attribute* attr,
        char* buf, size_t size);
static DEVICE_ATTR(cradle_state, S_IRUGO | S_IWUSR, get_usb_cradle_state, NULL);
void mbb_usbnet_usb_state_notify(USB_EVENT eventcode);
static USB_INT usb_host_charger_current_notify(struct notifier_block* self,
        USB_ULONG action, USB_PVOID dev);
USB_VOID usb_otg_set_support_feature(USB_VOID);
USB_UINT32 usb_otg_get_support_feature(USB_VOID);

static struct notifier_block usb_host_charger_current_nb =
{
    .notifier_call = usb_host_charger_current_notify
};

typedef struct usb_otg_accesory_ctx
{
    /*USB OTG ID状态*/
    USB_INT usb_id_status;                         /*USB ID当前状态*/
    USB_INT usb_old_id_status;                      /*USB ID 上次状态*/
    USB_UINT stat_usb_id_insert;                   /*防抖前插入记数*/
    USB_UINT stat_usb_id_insert_proc;               /*防抖后 处理前插入记数*/
    USB_UINT stat_usb_id_insert_proc_end;          /*防抖后 处完成后插入记数*/
    USB_UINT stat_usb_id_remove;                         /*防抖前拔出记数*/
    USB_UINT stat_usb_id_remove_proc;               /*防抖后处理前拔出记数*/
    USB_UINT stat_usb_id_remove_proc_end;        /*防抖后处理后拔出记数*/
    USB_UINT stat_usb_id_no_need_notify;             /*抖动计数*/
    USB_UINT stat_usb_dpdm_connect;                    /*D+ D-直连*/
    USB_UINT stat_usb_dpdm_disconnect;                /*D+ D-断开*/
    USB_UINT stat_usb_id_remove_chain;                 /*ID 移除*/
    USB_UINT stat_usb_id_disable_irq;                      /*进入假关机禁用中断*/
    USB_UINT stat_usb_id_enable_irq;                       /*退出假关机使能中断*/
    USB_UINT stat_usb_kick_timeout ;                        /*防抖动jiffies*/
    struct delayed_work usb_notify_accesory_wk; /*USB配件处理任务*/
    USB_INT  stat_host; /*是否host模式0:非host模式，1:host模式*/
    USB_BOOL  ext_chg_id_ins;
    wait_queue_head_t wait_wq;
    struct wake_lock otg_chg_wakelock;
    struct work_struct otg_chg_notify_work;
    /* protects detect process*/
    spinlock_t            lock;
    spinlock_t                  notify_lock;
    USB_UINT32 otg_feature;

} usb_otg_accesory_ctx_t;

usb_otg_accesory_ctx_t g_otg_accesory_h;

#define USB_ID_DETECT_DELAY        2000/*USB  ID 中断防抖中断检测延时*/
#define USB_ID_DETECT_REMOVE_DELAY  50

#define OTG_INSERT_EVENT 1
#define OTG_REMOVE_EVENT 0


/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB插拔入口
*****************************************************************/
void usb_id_status_change(USB_INT status)
{
    USB_ULONG timeout = USB_ID_DETECT_DELAY;
    g_otg_accesory_h.stat_usb_kick_timeout = timeout;
    switch (status)
    {

        case USB_OTG_ID_INSERT:

            otg_wake_lock();
            g_otg_accesory_h.stat_usb_id_insert++;

            break;
        case USB_OTG_ID_REMOVE:
            g_otg_accesory_h.stat_usb_id_remove++;
            timeout = USB_ID_DETECT_REMOVE_DELAY;
            break;
        default:
            DBG_E(MBB_OTG_CHARGER, "%s: error status:%d\n", __FUNCTION__, status);
    }
    /*插入配件时对外充电线，AF18 ,Cradle等*/
    /*涉及OTG线程切换，充电模块状态切换，OTG ID防抖经验值设置为2s */
   g_otg_accesory_h.usb_id_status = status;
    if(delayed_work_pending(&g_otg_accesory_h.usb_notify_accesory_wk))
    {
        cancel_delayed_work(&g_otg_accesory_h.usb_notify_accesory_wk);
    }
    adp_usb_queue_delay_work(&g_otg_accesory_h.usb_notify_accesory_wk, timeout);

 

}

/*****************************************************************
Parameters    :
Return        :    无
Description   :  usb配件插拔工作队列
*****************************************************************/

static void usb_accesory_handler(struct work_struct* work)
{
    USB_INT cur_status = 0;
    USB_ULONG flags = 0 ;
    USB_INT status = -1;
    DBG_E(MBB_OTG_CHARGER, "%s:old_id_status %d,cur_id_status%d\n",
          __FUNCTION__, g_otg_accesory_h.usb_old_id_status, cur_status);
    
        if (g_otg_accesory_h.usb_old_id_status == cur_status)
            {
                g_otg_accesory_h.stat_usb_id_no_need_notify++;
            }
    /**/
    cur_status = g_otg_accesory_h.usb_old_id_status;

recheck:
    local_irq_save(flags);
       status  = g_otg_accesory_h.usb_id_status;
    local_irq_restore(flags);
    //while(  g_otg_accesory_h.usb_id_status != cur_status)
    if( status != cur_status)
    {
          local_irq_save(flags);
        cur_status = g_otg_accesory_h.usb_id_status;
        g_otg_accesory_h.usb_old_id_status = cur_status;
        local_irq_restore( flags);
        switch (cur_status)
        {
            case USB_OTG_ID_INSERT:
            //spin_lock_irqsave(&g_otg_accesory_h.lock, flags);
                g_otg_accesory_h.stat_usb_id_insert_proc++;
                set_id_delta_time(USB_ID_DETECT_DELAY);
                //mlog_print("USB", mlog_lv_info, "stat_usb_id_insert_proc:%d\n"
                //    , g_otg_accesory_h.stat_usb_id_insert_proc);
            //status = g_otg_accesory_h.usb_old_id_status;
                g_otg_accesory_h.usb_old_id_status = USB_OTG_ID_INSERT;

                /*启动检测任务*/
                otg_detect_id_thread(USB_OTG_ID_INSERT);

            g_otg_accesory_h.stat_usb_id_insert_proc_end++;
            //mlog_print("USB", mlog_lv_info, "stat_usb_id_insert_proc_end:%d\n"
            //           , g_otg_accesory_h.stat_usb_id_insert_proc_end);
            //spin_unlock_irqrestore(&g_otg_accesory_h.lock, flags);
            break;
        case USB_OTG_ID_REMOVE:
            //spin_lock_irqsave(&g_otg_accesory_h.lock, flags);
                DBG_E(MBB_OTG_CHARGER, " usb otg id remove.\n");
                g_otg_accesory_h.stat_usb_id_remove_proc++;
                //mlog_print("USB", mlog_lv_info, "stat_usb_id_remove_proc:%d\n"
                //    , g_otg_accesory_h.stat_usb_id_remove_proc);
                usb_set_vbus_status(0);    //拔除时清除上次的vbus标志位
                /*启动检测任务*/
                otg_detect_id_thread(USB_OTG_ID_REMOVE);
                /*投赞成票*/
                otg_wake_unlock();

                g_otg_accesory_h.stat_usb_id_remove_proc_end++;
                //mlog_print("USB", mlog_lv_info, "stat_usb_perip_remove_proc_end:%d\n"
                //           , g_otg_accesory_h.stat_usb_id_remove_proc_end);
            //spin_unlock_irqrestore(&g_otg_accesory_h.lock, flags);
                break;
            default:
                DBG_E(MBB_OTG_CHARGER, "%s, invalid status:%d\n",
                        __FUNCTION__, cur_status);
            break;

        }
goto recheck;
    }

    DBG_E(MBB_OTG_CHARGER, "%s:task of %d end.\n", __FUNCTION__, cur_status);
    g_otg_accesory_h.usb_old_id_status = cur_status;
}

OTG_DEVICE_TYPE mbb_usb_identify_otg_device(USB_VOID)
{
    OTG_DEVICE_TYPE type = OTG_DEVICE_NORMAL;
    USB_INT vbus_gpio_value = 1;
    USB_BOOL vbus_state = MBB_USB_FALSE;
    vbus_gpio_value = gpio_get_value(GPIO_OTG_VBUS_DET);
    vbus_state = usb_get_vbus_status();
    DBG_T(MBB_OTG_CHARGER, "mbb_usb_identify_otg_device%d,%d\n",vbus_gpio_value,vbus_state);
    if ((!vbus_gpio_value) || (MBB_USB_TRUE == vbus_state))
    {
        type = OTG_DEVICE_USBNET;
    }

    else
    {
        type = OTG_DEVICE_EXTCHAGER;
    }

    return type;
}
USB_INT mbb_usb_otg_charger(OTG_DEVICE_TYPE type)
{
    USB_INT ret = MBB_USB_FALSE;
    if (g_otg_accesory_h.otg_feature & USB_OTG_FEATURE_AF18_MASK)
    {
        if ((OTG_DEVICE_USBNET == type) || (OTG_DEVICE_AP_CRADLE == type))
        {
            ret = MBB_USB_TRUE;
        }
    }
    return ret;
}
USB_INT mbb_usb_otg_ex_charger(OTG_DEVICE_TYPE type)
{
    USB_INT ret = MBB_USB_FALSE;
    USB_INT fast_off = MBB_USB_FALSE;
    USB_INT power_off = MBB_USB_FALSE;

    fast_off = usb_fast_on_off_stat();
    power_off = usb_power_off_chg_stat();

    /*判断是否为假关机,不进行对外充电*/
    if (fast_off  || power_off)
    {
        return ret;
    }
    if (g_otg_accesory_h.otg_feature & USB_OTG_FEATURE_EXTCHG)
    {
        if (OTG_DEVICE_EXTCHAGER == type)
        {
            ret = MBB_USB_TRUE;
        }
    }
    return ret;
}

USB_INT mbb_usb_otg_enum(OTG_DEVICE_TYPE type)
{
    USB_INT ret = MBB_USB_FALSE;
    USB_INT fast_off = MBB_USB_FALSE;
    USB_INT power_off = MBB_USB_FALSE;

    fast_off = usb_fast_on_off_stat();
    power_off = usb_power_off_chg_stat();

    /*判断是否为假关机,不进行枚举*/
    if (fast_off || power_off)
    {
        return ret;
    }

    if (g_otg_accesory_h.otg_feature & USB_OTG_FEATURE_AF18_MASK)
    {
        if ((OTG_DEVICE_USBNET == type) || (OTG_DEVICE_AP_CRADLE == type))
        {
            ret = MBB_USB_TRUE;
        }
    }
    return ret;
}

USB_VOID otg_detect_id_thread( USB_INT action )
{
    usb_adp_ctx_t* ctx = NULL;
    static USB_INT otg_first_report_chg_type = 1;
    static chg_chgr_type_t g_otg_charger_type = CHG_CHGR_INVALID;
    OTG_DEVICE_TYPE type = OTG_DEVICE_NORMAL;
    int tmp_ret = -1;
    u16 tmp_rtn_vbat_val = 0;
    USB_INT fast_off = MBB_USB_FALSE;
    USB_INT power_off = MBB_USB_FALSE;
    ctx = usb_get_adp_ctx();
    fast_off = usb_fast_on_off_stat();
    power_off = usb_power_off_chg_stat();
    if (NULL == ctx)
    {
        DBG_E(MBB_OTG_CHARGER, "--otg_detect_id_thread:  ctx == NULL -- \n");
        return;
    }

    DBG_I(MBB_OTG_CHARGER, "notify usb id is %8s.\n", USB_OTG_ID_INSERT == action ?
        "inserted" : "removed");
    tmp_ret = bsp_hkadc_convert(HKADC_CHANNEL_2, &tmp_rtn_vbat_val);
    if (tmp_ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER,"fail to convert, return value %d\n", tmp_ret);
    }
    else
    { 
        if((CHECK_ID_VOLT_LOW<tmp_rtn_vbat_val) && (tmp_rtn_vbat_val<CHECK_ID_VOLT_HIG))
        {
            g_cradle_type = CRADLE_AF35;
        }
        else
        {
            g_cradle_type = CRADLE_NULL;
        }
        DBG_E(MBB_OTG_CHARGER, "CHG:VBAT = %d\n", tmp_rtn_vbat_val);
    } 
    switch (action)
    {
            /******************检测到USB ID插入*******************************/
        case USB_OTG_ID_INSERT:

            /*识别当前的OTG设备类型*/
            type = mbb_usb_identify_otg_device();
            g_device_type = type;
            if (MBB_USB_TRUE == mbb_usb_otg_charger(type))            /*是否支持整机充电*/
            {
                g_otg_charger_type =  CHG_USB_OTG_CRADLE;
                if(CRADLE_AF35 == g_cradle_type)
                {
                     gpio_set_value(GPIO_DMDP_SWITCH, GPIO_HIGH); 
                     if (fast_off || power_off)
                     {
                         DBG_E(MBB_OTG_CHARGER, "-----now is poweroff----\n");
                     }
                     else
                     {
                         mbb_usbnet_usb_state_notify(USB_AF35_ATTACH);
                     }
                     DBG_E(MBB_OTG_CHARGER, "-----now is H3G CRADLE----\n");
                }
                else
                {
                     gpio_set_value(GPIO_2_5, GPIO_LOW); 
                     DBG_I(MBB_OTG_CHARGER, "-----now is not H3G CRADLE----\n");
                     break;
                }
            }
            else if (MBB_USB_TRUE == mbb_usb_otg_ex_charger(type))   /*是否支持对外充电*/
            {
                if(CRADLE_AF35 == g_cradle_type)
                {
                     break;
                }
                g_otg_charger_type =  CHG_EXGCHG_CHGR;
            }

            /*上报充电事件*/
            if (CHG_CHGR_INVALID != g_otg_charger_type)
            {
                /*第一次上报，直接置充电类型，防止充电没起*/
                if (ctx->stm_set_chg_type_cb && otg_first_report_chg_type)
                {
                    DBG_I(MBB_OTG_CHARGER, "-----first notify charger event----\n");
                    if (CHG_CHGR_INVALID != g_otg_charger_type)
                    {
                        ctx->stm_set_chg_type_cb(g_otg_charger_type);
                    }
                    otg_first_report_chg_type = 0;
                }
                ctx->battery_notifier_call_chain_cb(OTG_INSERT_EVENT, g_otg_charger_type);
            }

            /*OTG设备枚举断开D+ D-*/
            if (MBB_USB_TRUE == mbb_usb_otg_enum(type))
            {
                usb_notify_event(USB_OTG_DISCONNECT_DP_DM, NULL);
            }

            break;

            /***********************检测到USB ID 拔出*********************************/
        case USB_OTG_ID_REMOVE:

            if (CHG_CHGR_INVALID != g_otg_charger_type)
            {
                ctx->battery_notifier_call_chain_cb(OTG_REMOVE_EVENT, g_otg_charger_type);
            }
            g_otg_charger_type = CHG_CHGR_INVALID;
            gpio_set_value(GPIO_DMDP_SWITCH, GPIO_LOW); 
            if (fast_off || power_off)
            {
                DBG_E(MBB_OTG_CHARGER, "-----now is poweroff----\n");
            }
             else
            {
                mbb_usbnet_usb_state_notify(USB_AF35_REMOVE);
                usb_notify_event(USB_OTG_ID_PULL_OUT, NULL);
            }
            break;

        default:
            DBG_I(MBB_OTG_CHARGER, "-----invalid action:%d----\n", action);
            break;
    }
}
USB_INT dwc_otg_is_on(USB_VOID)
{
    return g_otg_accesory_h.stat_host;
}
EXPORT_SYMBOL(dwc_otg_is_on);
static irqreturn_t dwc_otg_det_irq(USB_INT irq, USB_PVOID dev_id)
{
    USB_INT otg_det_gpio_value = 1;
    DBG_I(MBB_OTG_CHARGER, "dwc_otg_det_irq:enter\n");
    USB_ULONG flags = 0;
    local_irq_save(flags);

    if (0 == gpio_int_state_get((unsigned)GPIO_OTG_ID_DET))
    {
        local_irq_restore(flags);
        return IRQ_NONE;
    }
    otg_det_gpio_value = gpio_get_value(GPIO_OTG_ID_DET);

    if (otg_det_gpio_value)
    {
        gpio_int_trigger_set(GPIO_OTG_ID_DET, IRQ_TYPE_LEVEL_LOW);
        /* ID 线拔出 */
        usb_id_status_change(USB_OTG_ID_REMOVE);
        DBG_I(MBB_OTG_CHARGER, "OTG_ID pull out!\n");
    }
    else
    {
        gpio_int_trigger_set(GPIO_OTG_ID_DET, IRQ_TYPE_LEVEL_HIGH);

        //gpio_set_value(GPIO_OTG_POWERON, 0);

        /* ID 线插入*/
        usb_id_status_change(USB_OTG_ID_INSERT);
        DBG_I(MBB_OTG_CHARGER, "OTG_ID pull in!\n");

    }
    local_irq_restore(flags);

    return IRQ_HANDLED;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  获取USB 系统锁
*****************************************************************/
USB_VOID otg_wake_lock(USB_VOID)
{
    USB_ULONG flags = 0;
   // local_irq_save(flags);
    wake_lock(&g_otg_accesory_h.otg_chg_wakelock);
    //local_irq_restore(flags);
    return;
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  USB系统解锁
*****************************************************************/
USB_VOID otg_wake_unlock(USB_VOID)
{
    USB_ULONG flags = 0;
   // local_irq_save(flags);
    wake_unlock(&g_otg_accesory_h.otg_chg_wakelock);
    //local_irq_restore(flags);
    return;
}

static USB_INT otg_register_otg_det_irq(USB_VOID)
{
    const char* desc = "OTG_DET_GPIO";
    irq_handler_t isr = NULL;
    unsigned long irqflags = 0;
    USB_INT irq = -1;
    USB_INT error = -1;
    if (gpio_is_valid(GPIO_OTG_ID_DET))
    {
        DBG_I(MBB_OTG_CHARGER, "gipo is valid\n");
        error = gpio_request(GPIO_OTG_ID_DET, "otg_int");
        if (error < 0)
        {
            DBG_E(MBB_OTG_CHARGER, "Failed to request GPIO %d, error %d\n", GPIO_OTG_ID_DET, error);
            return error;
        }
        gpio_int_mask_set(GPIO_OTG_ID_DET);
        gpio_int_state_clear(GPIO_OTG_ID_DET);
        error = gpio_direction_input(GPIO_OTG_ID_DET);
        if (error < 0)
        {
            DBG_E(MBB_OTG_CHARGER, "Failed to configure direction for GPIO %d, error s%d\n", GPIO_OTG_ID_DET, error);
            goto fail;
        }

        gpio_set_function(GPIO_OTG_ID_DET, GPIO_INTERRUPT);

        /*这里是处理，插着usb id线上电时，需要判断一下当前的id状态，通知充电模块是否充电*/
        dwc_otg_det_irq(NULL, NULL);

        irq = gpio_to_irq(GPIO_OTG_ID_DET);
        if (irq < 0)
        {
            error = irq;
            DBG_E(MBB_OTG_CHARGER, "Unable to get irq number for GPIO %d, error %d\n", GPIO_OTG_ID_DET, error);
            goto fail;
        }

        isr = dwc_otg_det_irq;
        irqflags = IRQF_NO_SUSPEND | IRQF_SHARED;

    }


    error = request_irq(irq, isr, irqflags, desc, &extern_otg_dev);
    if (error < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "Unable to claim irq %d; error %d\n", irq, error);

        goto fail;
    }
    gpio_int_state_clear(GPIO_OTG_ID_DET);
    gpio_int_unmask_set(GPIO_OTG_ID_DET);

    return MBB_USB_OK;

fail:
    if (gpio_is_valid(GPIO_OTG_ID_DET))
    {
        gpio_free(GPIO_OTG_ID_DET);
    }

    return error;
}

static USB_INT otg_unregister_otg_det_irq(USB_VOID)
{
    if (gpio_is_valid(GPIO_OTG_ID_DET))
    {
        gpio_free(GPIO_OTG_ID_DET);
    }
    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        gpio_free(GPIO_DMDP_CONNECT);
    }
    if (gpio_is_valid(GPIO_OTG_ID_SET))
    {
        gpio_free(GPIO_OTG_ID_SET);
    }

    return MBB_USB_OK;
}

/*********************************************************************
函数  :otg_exchg_status_clean
功能  :进入假关机后，清除对外充电状态
参数  :
返回值:
*********************************************************************/
static USB_VOID otg_exchg_status_clean(USB_VOID)
{
    USB_INT charger_type = USB_CHARGER_TYPE_INVALID;

    /*断开D+,D-,拉高 phy ID */
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    gpio_set_value(GPIO_OTG_ID_SET, GPIO_HIGH);

    if (MBB_USB_TRUE == g_otg_accesory_h.stat_host)
    {
        bsp_usb_status_change_ex(USB_BALONG_PERIP_REMOVE);
    }
    g_otg_accesory_h.stat_host = MBB_USB_FALSE;

    otg_wake_unlock();

}

/*********************************************************************
函数  :otg_switch_to_host_mode
功能  :对外充电切换到 OTG 模式充电，500mA
参数  :
返回值:
*********************************************************************/
static USB_VOID otg_switch_to_host_mode(USB_VOID)
{

    g_otg_accesory_h.stat_host = MBB_USB_TRUE;
    /*初始化 USB 驱动，进入  OTG  host mode*/
    bsp_usb_status_change_ex(USB_BALONG_PERIP_INSERT);

    /*断开 D+, D- , 拉低 ID */
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    gpio_set_value(GPIO_OTG_ID_SET, GPIO_LOW);
    //gpio_set_value(GPIO_OTG_POWERON, 0);
}

/*********************************************************************
函数  :otg_exchg_remove
功能  :对外充电线拔出处理
参数  :
返回值:
*********************************************************************/
static USB_VOID otg_exchg_remove(USB_VOID)
{
    USB_INT charger_type = USB_CHARGER_TYPE_INVALID;

    /*断开D+,D-,拉高 phy ID */
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    gpio_set_value(GPIO_OTG_ID_SET, GPIO_HIGH);

    if (MBB_USB_TRUE == g_otg_accesory_h.stat_host)
    {
        bsp_usb_status_change_ex(USB_BALONG_PERIP_REMOVE);
    }
    g_otg_accesory_h.stat_host = MBB_USB_FALSE;

}
/*********************************************************************
函数  :usb_check_event_is_valid
功能  :检测event事件，是否有重复，是否为非法event，
               不予理会，注意( event_id >= 0)
参数  :
返回值:
*********************************************************************/
USB_BOOL  usb_check_event_is_valid(USB_INT event_id)
{
    USB_BOOL  ret = MBB_USB_FALSE;

    DBG_E(MBB_OTG_CHARGER , "recieve event id = %d \n", event_id);

    if (event_id < 0)
    {
        DBG_E(MBB_OTG_CHARGER , " illegal event id=%d < 0 \n", event_id);
    }

    if (otg_old_event_id == event_id)
    {
        DBG_E(MBB_OTG_CHARGER , "recieve repeated event id = %d , ignore it\n", event_id);
        return MBB_USB_FALSE;

    }

    switch (otg_old_event_id)
    {
        case USB_OTG_CONNECT_DP_DM:
            if (USB_OTG_ENABLE_ID_IRQ != event_id)
            {
                ret = MBB_USB_TRUE;
            }
            break;
        case USB_OTG_DISCONNECT_DP_DM:
            if ((USB_OTG_ID_PULL_OUT == event_id) || (USB_OTG_DISABLE_ID_IRQ == event_id))
            {
                ret = MBB_USB_TRUE;
            }
            break;
        case USB_OTG_ID_PULL_OUT:
            otg_old_event_id = INVALID_STATE;
            break;
        case USB_OTG_DISABLE_ID_IRQ:
            if ((USB_OTG_ID_PULL_OUT == event_id) || (USB_OTG_ENABLE_ID_IRQ == event_id))
            {
                ret = MBB_USB_TRUE;
            }
            break;

        case USB_OTG_ENABLE_ID_IRQ:
            if (USB_OTG_DISABLE_ID_IRQ == event_id)
            {
                ret = MBB_USB_TRUE;
            }
            break;
        case INVALID_STATE:
        {
            ret = MBB_USB_TRUE;
        }
        break;
        default:
            break;
    }
    return ret;
}

/*********************************************************************
函数  :usb_otg_current_notify
功能  :根据输入参数，设置USB状态
参数  :action:
返回值:
*********************************************************************/
static USB_INT usb_host_charger_current_notify(struct notifier_block* self,
        USB_ULONG action, USB_PVOID dev)
{
    USB_ULONG flags = 0 ;

    if (!usb_check_event_is_valid(action))
    {
        DBG_E(MBB_OTG_CHARGER, "repeated action = %ld ,do nothing.\n", action);
        return MBB_USB_ERROR;
    }
   local_irq_save( flags);

    gpio_int_mask_set(GPIO_DMDP_CONNECT);
    gpio_int_state_clear(GPIO_DMDP_CONNECT);
    gpio_direction_output(GPIO_DMDP_CONNECT, 0);


    gpio_int_mask_set(GPIO_OTG_ID_SET);
    gpio_int_state_clear(GPIO_OTG_ID_SET);
    gpio_direction_output(GPIO_OTG_ID_SET, 1);

    DBG_I(MBB_OTG_CHARGER, "usb_otg_current_notify action is %ld.\n", action);

    switch (action)
    {
        case USB_OTG_CONNECT_DP_DM:
            /*短接 D+ D- , 进行 1 A 充电*/
            g_otg_accesory_h.stat_usb_dpdm_connect++;
            gpio_set_value(GPIO_DMDP_CONNECT, GPIO_HIGH);
            break;
        /* 断开D+ D-连接以后，拉低 ID 线 进入Host 模式，进行otg充电 */
        case USB_OTG_DISCONNECT_DP_DM:
            g_otg_accesory_h.stat_usb_dpdm_disconnect++;
            otg_switch_to_host_mode();
            break;
            /*对外充电线，拔出*/
        case USB_OTG_ID_PULL_OUT:
            g_otg_accesory_h.stat_usb_id_remove_chain++;
            otg_exchg_remove();
            break;
            /*进入假关机处理流程*/
        case USB_OTG_DISABLE_ID_IRQ:
            /*模拟真关机流程，使假关机恢复到初始状态 */
            g_otg_accesory_h.stat_usb_id_disable_irq++;
            g_otg_accesory_h.usb_old_id_status = 0;
            otg_exchg_status_clean();
            break;
            /*退出假关机处理流程*/
        case USB_OTG_ENABLE_ID_IRQ:
            /* 获取当前ID GPIO 状态，如果是低ID 在位上报ID插入事件,
            重新进入对外充电流程 */
            g_otg_accesory_h.stat_usb_id_enable_irq++;
            g_otg_accesory_h.stat_host = MBB_USB_FALSE;
            g_otg_accesory_h.usb_old_id_status = 0;
            dwc_otg_det_irq(NULL, NULL);
            gpio_int_state_clear(GPIO_OTG_ID_DET);
            gpio_int_unmask_set(GPIO_OTG_ID_DET);
            break;
        default:
            break;
    }
    local_irq_restore(flags);
    return MBB_USB_OK;
}

USB_INT chager_test(USB_ULONG action)
{
    gpio_int_mask_set(GPIO_DMDP_CONNECT);
    gpio_int_state_clear(GPIO_DMDP_CONNECT);
    gpio_direction_output(GPIO_DMDP_CONNECT, 0);

    gpio_int_mask_set(GPIO_OTG_ID_SET);
    gpio_int_state_clear(GPIO_OTG_ID_SET);
    gpio_direction_output(GPIO_OTG_ID_SET, 1);

    DBG_I(MBB_OTG_CHARGER, "usb_otg_current_notify action is %ld.\n", action);

    switch (action)
    {
        case USB_OTG_CONNECT_DP_DM:
            gpio_set_value(GPIO_DMDP_CONNECT, GPIO_HIGH);
            break;

        case USB_OTG_DISCONNECT_DP_DM:
            /* 断开D+ D-连接以后，拉低 ID 线 进入Host 模式，进行otg充电 */
            gpio_set_value(GPIO_DMDP_CONNECT, GPIO_HIGH);
            gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
            gpio_set_value(GPIO_OTG_ID_SET, GPIO_LOW);
            break;

        case USB_OTG_ID_PULL_OUT:
            /* ID拔出 断开D+ D-连接，拉高 ID 线 恢复USB 模式 */
            gpio_set_value(GPIO_OTG_ID_SET, GPIO_HIGH);
            gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
            break;

        default:
            break;
    }

    return MBB_USB_OK;
}

EXPORT_SYMBOL(chager_test);
/*********************************************************************
函数  : usb_otg_extern_charge_init
功能  : 对外充电事件初始化
参数  : void
返回值: void
*********************************************************************/
USB_VOID usb_otg_extern_charge_init(USB_VOID)
{
    USB_INT ret = 0;
    int rc = 0;
    usb_otg_set_support_feature();

    if (USB_OTG_FEATURE_NONE == g_otg_accesory_h.otg_feature)
    {

        //gpio_direction_output(GPIO_OTG_POWERON, GPIO_LOW);
        gpio_direction_output(GPIO_OTG_ID_SET, GPIO_HIGH);
        gpio_direction_output(GPIO_DMDP_CONNECT, GPIO_LOW);

        DBG_E(MBB_OTG_CHARGER, "NOT SUPPORT OTG.\r\n");
        return;
    }

    ret = gpio_request(GPIO_DMDP_CONNECT, "OTG_DP_DM_CONNECT_GPIO");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for OTG_DP_DM_CONNECT_GPIO\n");
        gpio_free(GPIO_DMDP_CONNECT);
    }
    ret = gpio_request(GPIO_OTG_ID_SET, "GPIO_OTG_ID_SET");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for GPIO_OTG_ID_SET\n");
        gpio_free(GPIO_OTG_ID_SET);
    }
    ret = gpio_request(GPIO_DMDP_SWITCH , "GPIO_DMDP_SWITCH");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for GPIO_DMDP_SWITCH \n");
        gpio_free(GPIO_DMDP_SWITCH);
    }
    gpio_direction_output(GPIO_DMDP_SWITCH, GPIO_LOW);
    ret = gpio_request(GPIO_OTG_VBUS_DET , "GPIO_OTG_VBUS_DET");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for GPIO_OTG_VBUS_DET \n");
        gpio_free(GPIO_OTG_VBUS_DET);
    }
    gpio_direction_input(GPIO_OTG_VBUS_DET);
   /* ret = gpio_request(GPIO_OTG_POWERON, "GPIO_OTG_POWERON");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for GPIO_OTG_POWERON\n");
        gpio_free(GPIO_OTG_POWERON);
    }
    gpio_direction_output(GPIO_OTG_POWERON, GPIO_LOW);*/
    lan_dev = get_cradle_dev();
    if (NULL == lan_dev)
    {

        lan_class = class_create(THIS_MODULE, "cradle_usb");
        lan_dev = device_create(lan_class, NULL, MKDEV(0, 0), NULL, "cradle");
        set_cradle_dev(lan_dev);
    }
    /*创建设备节点供应用模块查看*/
    rc = device_create_file(lan_dev, &dev_attr_cradle_state);
    if (rc)
    {    
        printk(KERN_ERR "failed to create file cradle_state\n");
    }
    printk(KERN_ERR "sucess to create file cradle_state\n");
    wake_lock_init(&g_otg_accesory_h.otg_chg_wakelock, WAKE_LOCK_SUSPEND, "otg-wakelock");
    INIT_DELAYED_WORK(&g_otg_accesory_h.usb_notify_accesory_wk, (void*)usb_accesory_handler);
    init_waitqueue_head(&g_otg_accesory_h.wait_wq);
    spin_lock_init(&g_otg_accesory_h.lock);
    spin_lock_init(&g_otg_accesory_h.notify_lock);

    /*初始化对外充事件电接口，供充电模块使用*/
    usb_register_otg_notify(&usb_host_charger_current_nb);
    otg_register_otg_det_irq();

}

/*********************************************************************
函数  : wakeup_wait_wq
功能  : 唤醒等待事件的接口
参数  : void
返回值: void
*********************************************************************/
USB_VOID wakeup_wait_wq(USB_VOID)
{
    wake_up_interruptible(&g_otg_accesory_h.wait_wq);
}
EXPORT_SYMBOL_GPL(wakeup_wait_wq);

/*********************************************************************
函数  : usb_otg_extern_charge_remove
功能  : 对外充电事件卸载
参数  : viod
返回值: void
*********************************************************************/
USB_VOID usb_otg_extern_charge_remove(USB_VOID)
{
    if (USB_OTG_FEATURE_NONE == g_otg_accesory_h.otg_feature)
    {
        DBG_I(MBB_OTG_CHARGER, "NOT SUPPORT OTG.\r\n");
        return;
    }
    /*移除对外充事件电接口*/
    usb_unregister_otg_notify(&usb_host_charger_current_nb);
    otg_unregister_otg_det_irq();
    wake_lock_destroy(&g_otg_accesory_h.otg_chg_wakelock);

}
/*********************************************************************
函数  : usb_otg_set_support_feature
功能  :设置当前otg特性
参数  : viod
返回值: void
*********************************************************************/
USB_VOID usb_otg_set_support_feature(USB_VOID)
{
g_otg_accesory_h.otg_feature = USB_OTG_FEATURE_AF18_EXTCHG;
}

/*********************************************************************
函数  : usb_otg_get_support_feature
功能  : 获取当前otg特性
参数  : viod
返回值: void
*********************************************************************/
USB_UINT32 usb_otg_get_support_feature(USB_VOID)
{
    return g_otg_accesory_h.otg_feature;
}
static ssize_t get_usb_cradle_state(struct device* dev,
                                    struct device_attribute* attr, char* buf, size_t size)
{
    int usb_state = 0;

    if (USB_AF35_ATTACH  == g_usbnet_usb_state )
    {
        usb_state = 1;
    }
    return snprintf(buf, sizeof(int), "%0x\n", usb_state);
}
/*****************************************************************
Function  name:mbb_usbnet_set_usb_state
Description   : set usb cradle insert state
Parameters    :  USB_EVENT state
Return        :    
*****************************************************************/
void mbb_usbnet_set_usb_state(USB_EVENT state)
{
    g_usbnet_usb_state = state;
}
/*****************************************************************
Function  name:mbb_usbnet_usb_state_notify
Description   : usb cradle insert event notify to app
Parameters    :  USB_EVENTs eventcode
Return        :    
*****************************************************************/
void mbb_usbnet_usb_state_notify(USB_EVENT eventcode)
{
    DEVICE_EVENT stusbEvent = {0};
    stusbEvent.device_id = DEVICE_ID_USB;
    stusbEvent.event_code = eventcode;
    stusbEvent.len = 0;

    (void)device_event_report(&stusbEvent, sizeof(DEVICE_EVENT));
    
    mbb_usbnet_set_usb_state(eventcode);

}
struct device * get_cradle_dev(void)
{
    return lan_dev;
}
void set_cradle_dev(struct device *dev)
{
    lan_dev = dev;    
}
EXPORT_SYMBOL(get_cradle_dev);
EXPORT_SYMBOL(set_cradle_dev);
OTG_CRADLE_TYPE usb_otg_get_cradle_type()
{
    return  g_cradle_type;
}
OTG_DEVICE_TYPE usb_otg_get_device_type()
{
    return g_device_type;
}
USB_VOID usb_otg_set_device_type()
{
    g_device_type = OTG_DEVICE_EXTCHAGER;
}

USB_VOID usb_otg_pogopin_set()
{
    usb_adp_ctx_t* ctx = NULL;
    chg_chgr_type_t g_otg_charger_type =  CHG_USB_OTG_CRADLE;
    ctx = usb_get_adp_ctx();
    if (NULL == ctx)
    {
        DBG_E(MBB_OTG_CHARGER, "--usb_otg_pogopin_set:  ctx == NULL -- \n");
        return;
    }
    ctx->battery_notifier_call_chain_cb(OTG_INSERT_EVENT, g_otg_charger_type);
    gpio_set_value(GPIO_DMDP_SWITCH, GPIO_HIGH); 
    mbb_usbnet_usb_state_notify(USB_AF35_ATTACH);
    usb_notify_event(USB_OTG_DISCONNECT_DP_DM, NULL);
}
USB_VOID usb_otg_pogopin_clean()
{
    usb_adp_ctx_t* ctx = NULL;
    chg_chgr_type_t g_otg_charger_type =  CHG_CHGR_INVALID;
    ctx = usb_get_adp_ctx();
    if (NULL == ctx)
    {
        DBG_E(MBB_OTG_CHARGER, "--usb_otg_pogopin_clean:  ctx == NULL -- \n");
        return;
    }
    ctx->battery_notifier_call_chain_cb(OTG_REMOVE_EVENT, g_otg_charger_type);
    gpio_set_value(GPIO_DMDP_SWITCH, GPIO_LOW); 
    mbb_usbnet_usb_state_notify(USB_AF35_REMOVE);
    usb_notify_event(USB_OTG_ID_PULL_OUT, NULL);
}
/*****************************************************************
Parameters    :
Return        :    无
Description   :  可维可测拔插设备信息
*****************************************************************/
void usb_accesory_dump(void)
{

    DBG_E(MBB_OTG_CHARGER, "usb_id_status:    %d\n", g_otg_accesory_h.usb_id_status);
    DBG_E(MBB_OTG_CHARGER, "usb_old_id_status:    %d\n", g_otg_accesory_h.usb_old_id_status);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_insert:    %d\n", 
        g_otg_accesory_h.stat_usb_id_insert);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_insert_proc:    %d\n", 
        g_otg_accesory_h.stat_usb_id_insert_proc);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_insert_proc_end:    %d\n", 
        g_otg_accesory_h.stat_usb_id_insert_proc_end);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_remove:    %d\n", 
        g_otg_accesory_h.stat_usb_id_remove);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_remove_proc:    %d\n", 
        g_otg_accesory_h.stat_usb_id_remove_proc);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_remove_proc_end:    %d\n", 
        g_otg_accesory_h.stat_usb_id_remove_proc_end);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_no_need_notify:    %d\n",
        g_otg_accesory_h.stat_usb_id_no_need_notify);
    DBG_E(MBB_OTG_CHARGER, "stat_host:    %d\n", g_otg_accesory_h.stat_host);
    
    DBG_E(MBB_OTG_CHARGER, "stat_usb_dpdm_connect:    %d\n", 
        g_otg_accesory_h.stat_usb_dpdm_connect);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_dpdm_disconnect:    %d\n", 
        g_otg_accesory_h.stat_usb_dpdm_disconnect);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_remove_chain:    %d\n", 
        g_otg_accesory_h.stat_usb_id_remove_chain);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_disable_irq:    %d\n", 
        g_otg_accesory_h.stat_usb_id_disable_irq);
    DBG_E(MBB_OTG_CHARGER, "stat_usb_id_enable_irq:    %d\n", 
        g_otg_accesory_h.stat_usb_id_enable_irq);
    DBG_E(MBB_OTG_CHARGER, "otg_feature:    %d\n", g_otg_accesory_h.otg_feature);

}
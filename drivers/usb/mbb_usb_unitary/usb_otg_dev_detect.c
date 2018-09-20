

#include <linux/timer.h>
#include <linux/notifier.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include "usb_otg_dev_detect.h"
#include "usb_charger_manager.h"
#include "usb_platform_comm.h"
#include "usb_debug.h"
#include "usb_notify.h"

#include <bsp_hkadc.h>
#ifdef  USB_FAST_ON_OFF
#include "mbb_fast_on_off.h"
#endif

/*文件不独立编译，会有重复定义warning*/
#if 0
USB_INT  usb_fast_on_off_stat(USB_VOID)
{
    return  0;
}
USB_INT usb_power_off_chg_stat(USB_VOID)
{
    return 0;
}
#endif

//static BLOCKING_NOTIFIER_HEAD (otg_dev_notify_list);

struct otg_dev_det* otg_device;

extern void usb_notify_syswatch(int deviceid, int eventcode);
static OTG_DEVICE_TYPE g_device_type = OTG_DEVICE_MAX;
static OTG_CRADLE_TYPE g_cradle_type = CRADLE_NULL;
static int g_usbcradle_usb_state = USB_AF35_REMOVE; //AF35插入状态
static struct class* cradle_class;
static struct device* cradle_dev;
struct device* get_cradle_dev(void);
USB_VOID  set_cradle_dev(struct device* dev);
static ssize_t get_otg_cradle_state(struct device* dev, struct device_attribute* attr,
        char* buf, size_t size);
static DEVICE_ATTR(cradle_state, S_IRUGO | S_IWUSR, get_otg_cradle_state, NULL);
USB_VOID  mbb_cradle_state_notify(USB_EVENT eventcode);
USB_BOOL  otg_check_event_is_valid(USB_INT event_id);

/*******otg 适配接口**********/

#include "otg_device_adp.c"


int otg_dev_cradle_state(void)
{
    return otg_device->notify_event;
}
EXPORT_SYMBOL(otg_dev_cradle_state);

int otg_dev_id_state(void)
{
#ifdef USB_OTG_DEV_DETECT
    return gpio_get_value(GPIO_OTG_ID_DET);
#else
    return ID_FLOAT;
#endif/*USB_OTG_DEV_DETECT*/
}
EXPORT_SYMBOL(otg_dev_id_state);

void otg_dev_set_remove_flags(int remove_flag)
{
    if (otg_device)
    {
        otg_device->otg_remove_flag = remove_flag;
    }
}

int otg_dev_get_remove_flags(void)
{
    if (otg_device)
    {
        return otg_device->otg_remove_flag;
    }
    else
    {
        return OTG_DEV_INVALID_FLAG;
    }
}

static USB_PCHAR dev_type_to_string(struct otg_dev_det* otg)
{

    switch (otg->dev.dev_type)
    {
        case OTG_DEVICE_UNDEFINED:
            return "OTG_DEVICE_UNDEFINED";
        case OTG_DEVICE_EXTCHAGER:
            return "OTG_DEVICE_EXTCHAGER";
        case OTG_DEVICE_CRADLE:
            return "OTG_DEVICE_CRADLE";
        case  OTG_DEVICE_CRADLE_CHARGE:
            return "OTG_DEVICE_CRADLE_CHARGE";
        default:
            return "sorry~~";
    }
}

static USB_VOID otg_cradle_insert(struct otg_dev_det *otg)
{
    usb_adp_ctx_t*ctx = otg->contex;

    DBG_I(MBB_OTG_CHARGER,"cradle insert!\n");
    otg->charge_type = CHG_WALL_CHGR;
    otg_first_report_charge_type(otg);
    ctx->battery_notifier_call_chain_cb(OTG_INSERT, otg->charge_type);
    if(OTG_DEVICE_CRADLE == otg->dev.dev_type)
    {

#if defined(BSP_CONFIG_BOARD_E5575S_210) || defined(BSP_CONFIG_BOARD_E5575S_320)
        if(CRADLE_AF35 == g_cradle_type)
        {
              gpio_set_value(GPIO_DMDP_SWITCH, GPIO_HIGH); 
              mbb_cradle_state_notify(USB_AF35_ATTACH);
              DBG_E(MBB_OTG_CHARGER, "-----now is H3G CRADLE----\n");
        }
        else
        {
               gpio_set_value(GPIO_2_5, GPIO_LOW); 
               DBG_E(MBB_OTG_CHARGER, "-----now is not H3G CRADLE----\n");
               return;
        }
#else
         otg->notify_event = USB_CRADLE_PLUGIN_NOTIFY;
         usb_notify_syswatch(DEVICE_ID_USB, otg->notify_event);
#endif
        otg->phy_id = HOST_ON;
        otg_host_on_off(otg);
    }
    else
    {
        DBG_I(MBB_OTG_CHARGER,"cradle charge only!\n");
    }
}
static USB_VOID otg_cradle_remove(struct otg_dev_det *otg)
{
    usb_adp_ctx_t*ctx = otg->contex;
    DBG_I(MBB_OTG_CHARGER,"cradle remove!\n");
    ctx->battery_notifier_call_chain_cb(OTG_REMOVE, otg->charge_type);
    otg->charge_type = CHG_CHGR_INVALID;
    if(MBB_USB_TRUE == otg->host)
    {
#if defined(BSP_CONFIG_BOARD_E5575S_210) || defined(BSP_CONFIG_BOARD_E5575S_320)
        if(CRADLE_AF35 == g_cradle_type)
        {
            mbb_cradle_state_notify(USB_AF35_REMOVE);
            gpio_set_value(GPIO_2_5, GPIO_LOW); 
            g_cradle_type = CRADLE_NULL;
        }
        else
        {
            DBG_E(MBB_OTG_CHARGER, "--otg_cradle_remove  not H3G CRADLE-\n");
            return;
        }
#else
        otg->notify_event = USB_CRADLE_UNPLUG_NOTIFY;
        usb_notify_syswatch(DEVICE_ID_USB, otg->notify_event);
#endif
        otg->phy_id = HOST_OFF;
        otg_host_on_off(otg);
    }
}
static USB_VOID otg_excharge_insert(struct otg_dev_det* otg)
{
    usb_adp_ctx_t* ctx = otg->contex;

    DBG_I(MBB_OTG_CHARGER, "excharge insert!\n");
    otg->charge_type = CHG_EXGCHG_CHGR;
    otg_first_report_charge_type(otg);
    ctx->battery_notifier_call_chain_cb(OTG_INSERT, otg->charge_type);

}

static USB_VOID otg_excharge_remove(struct otg_dev_det* otg)
{
    usb_adp_ctx_t* ctx = otg->contex;

    DBG_I(MBB_OTG_CHARGER, "excharge remove!\n");
    /*先报对外充电拔出，vbus 快速跌落*/
    ctx->battery_notifier_call_chain_cb(OTG_REMOVE, otg->charge_type);
    otg->charge_type = CHG_CHGR_INVALID;
    otg_exchg_disconnect_dpdm(otg);
    otg_check_event_is_valid(USB_OTG_ID_PULL_OUT);
    if (MBB_USB_TRUE == otg->host)
    {
        otg->phy_id = HOST_OFF;
        otg_host_on_off(otg);
    }
}

#if  defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5577S_321) \
     || defined(BSP_CONFIG_BOARD_E5577S_324) 
int g_otg_cradle_falg  = 0;
EXPORT_SYMBOL(g_otg_cradle_falg);
#endif

static USB_VOID otg_dev_insert(struct otg_dev_det* otg)
{
    USB_INT dev = otg->dev.dev_type;

    DBG_T(MBB_OTG_CHARGER, "[ %s ] insert\n", dev_type_to_string(otg));
    switch (dev)
    {
        case OTG_DEVICE_CRADLE:
        case OTG_DEVICE_CRADLE_CHARGE:
    #if defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5577S_321) \
    || defined(BSP_CONFIG_BOARD_E5577S_324) 
            g_otg_cradle_falg = 1 ;
    #endif  
            otg_cradle_insert(otg);
            break;
        case OTG_DEVICE_EXTCHAGER:
    #if defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5577S_321) \
    || defined(BSP_CONFIG_BOARD_E5577S_324) 
            g_otg_cradle_falg = 0;
    #endif
            otg_excharge_insert(otg);
            break;
        default:
            DBG_E(MBB_OTG_CHARGER, "undefined otg-device insert!\n");
            break;
    }
}

static USB_VOID otg_dev_remove(struct otg_dev_det* otg)
{
    USB_INT dev = otg->dev.dev_type;

    DBG_T(MBB_OTG_CHARGER, "[ %s ] remove!\n", dev_type_to_string(otg));
    otg_dev_set_remove_flags(OTG_DEV_REMOVE_PROC);
    switch (dev)
    {
        case OTG_DEVICE_CRADLE:
        case OTG_DEVICE_CRADLE_CHARGE:
            otg_cradle_remove(otg);
            break;
        case OTG_DEVICE_EXTCHAGER:
            otg_excharge_remove(otg);
            break;
        default:
            DBG_E(MBB_OTG_CHARGER, "undefined otg-device pull out!\n");
            break;
    }
    otg->dev.dev_type = OTG_DEVICE_UNDEFINED;
}

/*
    判断产品是否支持该设备
    otg_feature 标记产品是否打开对应功能
    关机、假关机情况 均不支持cradle与对外充。
*/
USB_INT otg_dev_support(struct otg_dev_det* otg, USB_UINT32 mask)
{

    USB_INT fast_off = MBB_USB_FALSE;
    USB_INT power_off = MBB_USB_FALSE;
    USB_INT ret = MBB_USB_FALSE;
#ifdef USB_FAST_ON_OFF
    fast_off = usb_fast_on_off_stat();
#endif
    power_off = usb_power_off_chg_stat();
    DBG_I(MBB_OTG_CHARGER,
          "otg_feature = %ld,fast_off = %d,power_off = %d\n",
          otg->otg_feature, fast_off, power_off);

    if (!(otg->otg_feature & mask) || fast_off || power_off)
    {
        ret = MBB_USB_FALSE;
    }
    else
    {
        ret = MBB_USB_TRUE;
    }
    return ret;
}

static USB_VOID otg_dev_identify(struct otg_dev_det* otg)
{
    USB_UINT32 mask;
    USB_INT vbus_gpio_value = 1;
    /*id 中断 1 s 后获取 vubs 状态*/
    otg->vbus = otg_get_vbus_state(otg);
#ifndef BSP_CONFIG_BOARD_E5577BS_937
    vbus_gpio_value = gpio_get_value(GPIO_OTG_VBUS_DET);
#endif
    /*cradle */
    if ((VBUS_UP == otg->vbus) ||(!vbus_gpio_value) )
    {
        mask = USB_OTG_FEATURE_CRADLE_MASK;
        if (otg_dev_support(otg, mask))
        {
            otg->dev.dev_type = OTG_DEVICE_CRADLE;
        }
        /*如果单板此时不支持对cradle 的枚举，
                则为 仅充电cradle*/
        else
        {
            otg->dev.dev_type = OTG_DEVICE_CRADLE_CHARGE;
        }
    }
    /*excharge*/
    else
    {
        mask = USB_OTG_FEATURE_EXTCHG_MASK;
        if (otg_dev_support(otg, mask))
        {
            otg->dev.dev_type = OTG_DEVICE_EXTCHAGER;
        }
        else
        {
            otg->dev.dev_type = OTG_DEVICE_UNDEFINED;
            wake_unlock(&otg->id_wake_lock);
        }
    }
    g_device_type = otg->dev.dev_type;
    otg_dev_insert(otg);
}
static USB_VOID otg_id_adc_detect()
{
     USB_INT tmp_ret = -1;
     u16 tmp_rtn_vbat_val = 0;
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
}
static USB_VOID otg_id_detect_work_func( struct work_struct *w )
{
    USB_INT current_id = ID_FLOAT;
    USB_ULONG flags = 0;
    struct otg_dev_det* otg = container_of(w, struct otg_dev_det,
                                           otg_id_detect_work.work);
recheck:
    local_irq_save(flags);
    current_id = otg->id;
    /*重新读一下  ID_GPIO*/
    otg->id = gpio_get_value(otg->id_det_gpio);
    /*
            实际读到ID 值 与 中断获取的不同
            更新 id 状态为实际读到的状态
        */
    if (otg->id != current_id)
    {
        current_id = otg->id;
        otg->debug.stat_usb_id_no_trigger++;
    }
    local_irq_restore(flags);


    if (current_id == otg->old_id)
    {
        return;
    }
    DBG_T(MBB_OTG_CHARGER, "current_id = %d,old_id = %d\n", current_id, otg->old_id);

    otg->old_id = current_id;
    if (!current_id)
    {

        otg_id_adc_detect();
        otg->debug.stat_usb_id_insert_proc++;
        otg->dev.status = OTG_INSERT;
        otg_dev_identify(otg);
    }
    else
    {
        otg->debug.stat_usb_id_remove_proc++;
        otg->dev.status = OTG_REMOVE;
        otg_dev_remove(otg);
        wake_unlock(&otg->id_wake_lock);

    }
    goto recheck;

}

static USB_VOID otg_id_status_change(struct otg_dev_det* otg)
{
    USB_INT delay_time = 0;

    DBG_T(MBB_OTG_CHARGER, " -------irg---id = %d -------\n", otg->id);
    switch (otg->id)
    {
        case ID_GROUND:
            wake_lock(&otg->id_wake_lock);
            otg->debug.stat_usb_id_insert++;
            delay_time = OTG_DET_LONG_DELAY_TIME;
            break;
        case ID_FLOAT:
            otg->debug.stat_usb_id_remove++;
            delay_time = OTG_DET_SHORT_DELAY_TIME;
            break;
    }
    otg->debug.stat_usb_kick_timeout = delay_time;

    g_device_type = OTG_DEVICE_MAX;
    if (delayed_work_pending(&otg->otg_id_detect_work))
    {
        cancel_delayed_work(&otg->otg_id_detect_work);
    }
    /* 用USB  自己的线程*/
    queue_delayed_work(system_nrt_wq, &otg->otg_id_detect_work, delay_time);

}


static irqreturn_t otg_dev_det_irq(USB_INT irq, USB_PVOID dev_id)
{
    USB_INT id_val;
    USB_ULONG flags = 0;
    struct otg_dev_det* otg = dev_id;

    DBG_I(MBB_OTG_CHARGER, "otg id trigger!\n");
    if (MBB_USB_TRUE == otg->id_irq_shared)
    {
        local_irq_save(flags);
        if (0 == otg_id_irq_share_protect(otg))
        {
            local_irq_restore(flags);
            return IRQ_NONE;
        }
    }
    id_val = gpio_get_value(otg->id_det_gpio);
    otg->id = id_val;
    otg_id_trigger_set(otg);
    otg_id_status_change(otg);

    if (MBB_USB_TRUE == otg->id_irq_shared)
    {
        local_irq_restore(flags);
    }
    return IRQ_HANDLED;
}

/*
该函数会判断的四个事件
USB_OTG_CONNECT_DP_DM
USB_OTG_DISCONNECT_DP_DM
USB_OTG_FAST_OFF
USB_OTG_FAST_ON
*/
USB_BOOL  otg_check_event_is_valid(USB_INT event_id)
{
    USB_BOOL  ret = OTG_DEV_EVENT_PROC;
    static USB_INT  otg_old_event_id = INVALID_STATE;

    DBG_I(MBB_OTG_CHARGER, "recieve event id = %d ，old_event = %d\n",
          event_id, otg_old_event_id);
    /*无效*/
    if (event_id < 0)
    {
        DBG_E(MBB_OTG_CHARGER , " illegal event id=%d < 0 \n", event_id);
        return OTG_DEV_EVENT_NONPROC;
    }

    /*重复事件*/
    if (otg_old_event_id == event_id)
    {
        DBG_E(MBB_OTG_CHARGER , "recieve repeated event id = %d , ignore it\n", event_id);
        return OTG_DEV_EVENT_NONPROC;

    }

    /*判断当前状态 (otg_old_event_id)，
        是否可以执行收到的事件 (event_id)*/
    switch (otg_old_event_id)
    {
        case USB_OTG_CONNECT_DP_DM:
        case USB_OTG_DISCONNECT_DP_DM:
            if (USB_OTG_FAST_ON == event_id)
            {
                ret = OTG_DEV_EVENT_NONPROC;
            }
            else
            {
                ret = OTG_DEV_EVENT_PROC;
            }
            break;
        case USB_OTG_FAST_OFF:
            if (USB_OTG_FAST_ON == event_id)
            {
                ret = OTG_DEV_EVENT_PROC;
            }
            else
            {
                ret = OTG_DEV_EVENT_NONPROC;
            }
            break;

        case USB_OTG_FAST_ON:
        default:
            /*假开机与默认值时
                    处理所有其他事件*/
            ret = OTG_DEV_EVENT_PROC;
            break;
    }
    if ( OTG_DEV_EVENT_PROC == ret)
    {
        otg_old_event_id = event_id;
    }
    return ret;
}

static USB_VOID otg_dev_fast_on(struct otg_dev_det* otg)
{
    otg->debug.stat_usb_otg_fast_on++;
    otg->host = MBB_USB_FALSE;
    otg->old_id = ID_FLOAT;
    otg_dev_det_irq(0, (USB_PVOID)otg);
    otg_gpio_clear_set(otg->id_det_gpio);

}

static USB_VOID otg_dev_fast_off(struct otg_dev_det* otg)
{
    otg->debug.stat_usb_otg_fast_off++;
    otg->old_id = ID_FLOAT;
    if (otg->host)
    {
        otg->phy_id = HOST_OFF;
        otg_host_on_off(otg);
    }
}


/*********************************************************************
函数  :otg_dev_event_chain_func
功能  :根据输入参数，设置USB状态
参数  :action:
返回值:
*********************************************************************/
static USB_INT otg_dev_event_chain_func(struct notifier_block* self,
                                        USB_ULONG action, USB_PVOID dev)
{
    USB_ULONG flags = 0 ;
    struct otg_dev_det* otg = otg_device;

    if (!otg_check_event_is_valid(action))
    {
        DBG_E(MBB_OTG_CHARGER, "invalid event");
        return MBB_USB_ERROR;
    }

    local_irq_save( flags);

    otg_dev_source_set();

    DBG_I(MBB_OTG_CHARGER, "action is %ld.\n", action);

    switch (action)
    {
        case USB_OTG_CONNECT_DP_DM:
            otg_exchg_connect_dpdm(otg);
            break;
            /* 断开D+ D-连接以后，拉低 ID 线 进入Host 模式，进行otg充电 */
        case USB_OTG_DISCONNECT_DP_DM:
            otg_exchg_disconnect_dpdm_to_host(otg);
            break;
        case USB_OTG_FAST_OFF:
            otg_dev_fast_off(otg);
            break;
        case USB_OTG_FAST_ON:
            otg_dev_fast_on(otg);
            break;
        default:
            DBG_E(MBB_OTG_CHARGER, "do not support this event!\n");
            break;
    }
    local_irq_restore(flags);
    return MBB_USB_OK;
}

static struct notifier_block usb_otg_dev_event_nb =
{
    .notifier_call = otg_dev_event_chain_func
};

static USB_INT otg_dev_register_id_irq(struct otg_dev_det* otg)
{
    const USB_PCHAR irq_desc = "OTG_DET_GPIO";
    irq_handler_t isr = NULL;
    USB_INT irq = -1;
    USB_INT id_gpio = -1;
    USB_INT ret = MBB_USB_ERROR;

    id_gpio = otg->id_det_gpio;
    isr = otg_dev_det_irq;
    DBG_T(MBB_OTG_CHARGER, " id_det_gpio %d\n", id_gpio);

    if (!gpio_is_valid(id_gpio))
    {
        DBG_E(MBB_OTG_CHARGER, "usb_id_det_gpio is invalid\n");
        return MBB_USB_ERROR;
    }
    otg_id_irq_flags_set(otg);
    otg_id_gpio_irq_set(otg->id_det_gpio);

    ret = gpio_request(id_gpio, irq_desc);
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "can't request id_det_gpio %d\n", id_gpio);
        return ret;
    }

    ret = gpio_direction_input(id_gpio);
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "can't request input direction id_det_gpio %d\n",
              id_gpio);
        return ret;
    }


    irq = gpio_to_irq(id_gpio);
    if (irq < 0)
    {
        return MBB_USB_ERROR;
    }
    ret = request_irq(irq, isr, otg->id_irqflags, irq_desc, otg);
    if (ret)
    {
        DBG_E(MBB_OTG_CHARGER, "irqreq ID INT failed\n");
        goto fail;
    }
    otg_gpio_clear_set(otg->id_det_gpio);
    return MBB_USB_OK;
fail:
    if (gpio_is_valid(id_gpio))
    {
        gpio_free(id_gpio);
    }
    return MBB_USB_ERROR;

}

static USB_VOID otg_unregister_id_irq(struct otg_dev_det* otg)
{
    USB_INT id_gpio;

    id_gpio = otg->id_det_gpio;
    if (gpio_is_valid(id_gpio))
    {
        gpio_free(id_gpio);
    }
}


/*********************************************************************
函数  : usb_otg_get_support_feature
功能  : 获取当前otg特性
参数  : viod
返回值: void
*********************************************************************/
USB_UINT32 usb_otg_get_support_feature(USB_VOID)
{
    return otg_device->otg_feature;
}
#if 0
/*********************************************************************
函数  : otg_wakeup_wait_wq
功能  : 唤醒等待事件的接口
参数  : void
返回值: void
*********************************************************************/
USB_VOID otg_wakeup_wait_wq(struct otg_dev_det* otg)
{
    wake_up_interruptible(&otg->wait_wq);
}
#endif
//void usb_notify_event(unsigned long val, void* v)
//{
//    DBG_T(MBB_OTG_CHARGER," usb_notify_event: val = %ld \n",val);
//    blocking_notifier_call_chain(&otg_dev_notify_list, val, v);
//}
//EXPORT_SYMBOL_GPL(usb_notify_event);

USB_INT usb_otg_device_detect_init(USB_VOID)
{
    struct otg_dev_det* otg = NULL;

    int rc = 0;
    otg = (struct otg_dev_det*)
          kzalloc(sizeof(struct otg_dev_det), GFP_KERNEL);

    if (!otg)
    {
        DBG_E(MBB_OTG_CHARGER,
              "cradle driver st kmalloc fail \n");

        return MBB_USB_ERROR;
    }

    otg_device = otg;
    otg_dev_set_platform(otg);
    product_set_otg_dev_support_feature();
    otg_dev_set_remove_flags(OTG_DEV_INVALID_FLAG);
    otg->contex = usb_get_adp_ctx();
    otg->charge_type = CHG_CHGR_INVALID;
    otg->host = MBB_USB_FALSE;
    otg->id = ID_FLOAT;
    otg->old_id = ID_FLOAT;
    otg->id_det_gpio = GPIO_OTG_ID_DET;
    otg_dev_request_source();

    cradle_dev = get_cradle_dev();
    if (NULL == cradle_dev)
    {

        cradle_class = class_create(THIS_MODULE, "cradle_usb");
        cradle_dev = device_create(cradle_class, NULL, MKDEV(0, 0), NULL, "cradle");
        set_cradle_dev(cradle_dev);
    }
    /*创建设备节点供应用模块查看*/
    rc = device_create_file(cradle_dev, &dev_attr_cradle_state);
    if (rc)
    {    
        printk(KERN_ERR "failed to create file cradle_state\n");
    }
    printk(KERN_ERR "sucess to create file cradle_state\n");

    wake_lock_init(&otg->id_wake_lock,
                   WAKE_LOCK_SUSPEND, "otg_id_wakelock");
    INIT_DELAYED_WORK(&otg->otg_id_detect_work,
                      otg_id_detect_work_func);
    //init_waitqueue_head(&otg->wait_wq);
    //spin_lock_init(&otg->lock);
    //spin_lock_init(&otg->notify_lock);
    usb_register_otg_notify(&usb_otg_dev_event_nb);
    /*otg 开机自动检测*/
    otg_dev_det_irq(0, (USB_PVOID)otg);
    otg_dev_register_id_irq(otg);
    return 0;
}

USB_VOID usb_otg_device_detect_exit(USB_VOID)
{
    struct otg_dev_det* otg = otg_device;

    if (!otg)
    {
        DBG_E(MBB_OTG_CHARGER, "otg NULL\n");
        return;
    }
    usb_unregister_otg_notify(&usb_otg_dev_event_nb);
    cancel_delayed_work_sync(&otg->otg_id_detect_work);
    otg_unregister_id_irq(otg);
    wake_lock_destroy(&otg->id_wake_lock);
    otg_dev_free_source();
    kfree(otg);
}

USB_INT chager_test(USB_ULONG action)
{
    struct otg_dev_det* otg = otg_device;


    DBG_I(MBB_OTG_CHARGER, "test action %ld.\n", action);
    otg_dev_source_set();
    switch (action)
    {
        case USB_OTG_CONNECT_DP_DM:
            otg_exchg_connect_dpdm(otg);
            break;

        case USB_OTG_DISCONNECT_DP_DM:
            /* 断开D+ D-连接以后，拉低 ID 线 进入Host 模式，进行otg充电 */
            otg_exchg_connect_dpdm(otg);
            break;
        case USB_OTG_ID_PULL_OUT:
            /* ID拔出 断开D+ D-连接，拉高 ID 线 恢复USB 模式 */
            otg->phy_id = HOST_OFF;
            otg_host_on_off(otg);
            break;
        default:
            break;
    }

    return MBB_USB_OK;
}

static ssize_t get_otg_cradle_state(struct device* dev,
                                    struct device_attribute* attr, char* buf, size_t size)
{
    int usb_state = 0;
    if (USB_AF35_ATTACH  == g_usbcradle_usb_state )
    {
        usb_state = 1;
    }
    return snprintf(buf, sizeof(int), "%0x\n", usb_state);
}

/*****************************************************************
Function  name:mbb_cradle_set_usb_state
Description   : set usb cradle insert state
Parameters    :  USB_EVENT state
Return        :    
*****************************************************************/
USB_VOID mbb_cradle_set_usb_state(USB_EVENT state)
{

    g_usbcradle_usb_state = state;
}

/*****************************************************************
Function  name:mbb_cradle_state_notify
Description   : usb cradle insert event notify to app
Parameters    :  USB_EVENTs eventcode
Return        :    
*****************************************************************/
USB_VOID mbb_cradle_state_notify(USB_EVENT eventcode)
{
    DEVICE_EVENT stusbEvent = {0};
    stusbEvent.device_id = DEVICE_ID_USB;
    stusbEvent.event_code = eventcode;
    stusbEvent.len = 0;

    (void)device_event_report(&stusbEvent, sizeof(DEVICE_EVENT));
    
    mbb_cradle_set_usb_state(eventcode);

}

struct device * get_cradle_dev(void)
{
    return cradle_dev;
}

USB_VOID set_cradle_dev(struct device *dev)
{
    cradle_dev = dev;    
}
EXPORT_SYMBOL(get_cradle_dev);
EXPORT_SYMBOL(set_cradle_dev);

OTG_CRADLE_TYPE usb_otg_get_cradle_type()
{
    return g_cradle_type;
}

OTG_DEVICE_TYPE usb_otg_get_device_type()
{
    return g_device_type;
}

USB_VOID usb_otg_set_device_type()
{
    g_device_type = OTG_DEVICE_UNDEFINED;
}

USB_VOID usb_otg_pogopin_set()
{
    usb_adp_ctx_t* ctx = NULL;
    chg_chgr_type_t g_otg_charger_type =  CHG_WALL_CHGR;
    struct otg_dev_det *otg = otg_device;
    ctx = otg->contex;
    if (NULL == ctx)
    {
        DBG_E(MBB_OTG_CHARGER, "--usb_otg_pogopin_set:  ctx == NULL -- \n");
        return;
    }

    ctx->battery_notifier_call_chain_cb(OTG_INSERT, g_otg_charger_type);
    gpio_set_value(GPIO_DMDP_SWITCH, GPIO_HIGH); 
    mbb_cradle_state_notify(USB_AF35_ATTACH);
    otg->dev.dev_type = OTG_DEVICE_CRADLE;
    otg->phy_id = HOST_ON;
    otg_host_on_off(otg);
}

USB_VOID usb_otg_pogopin_clean()
{
    usb_adp_ctx_t* ctx = NULL;
    chg_chgr_type_t g_otg_charger_type =  CHG_CHGR_INVALID;
    struct otg_dev_det *otg = otg_device;
    ctx = otg->contex;
    if (NULL == ctx)
    {
        DBG_E(MBB_OTG_CHARGER, "--usb_otg_pogopin_clean:  ctx == NULL -- \n");
        return;
    }
    ctx->battery_notifier_call_chain_cb(OTG_REMOVE, g_otg_charger_type);
    gpio_set_value(GPIO_DMDP_SWITCH, GPIO_LOW); 
    mbb_cradle_state_notify(USB_AF35_REMOVE);
    otg->dev.dev_type = OTG_DEVICE_UNDEFINED;
    otg->phy_id = HOST_OFF;
    otg_host_on_off(otg);
}


USB_VOID usb_otg_dump(USB_VOID)
{

    DBG_T(MBB_OTG_CHARGER, "usb_id_status:    %d\n", otg_device->id);
    DBG_T(MBB_OTG_CHARGER, "usb_old_id_status:    %d\n", otg_device->old_id);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_insert:    %d\n", otg_device->debug.stat_usb_id_insert);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_insert_proc:    %d\n", otg_device->debug.stat_usb_id_insert_proc);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_insert_proc_end:    %d\n", otg_device->debug.stat_usb_id_insert_proc_end);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_remove:    %d\n", otg_device->debug.stat_usb_id_remove);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_remove_proc:    %d\n", otg_device->debug.stat_usb_id_remove_proc);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_remove_proc_end:    %d\n", otg_device->debug.stat_usb_id_remove_proc_end);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_no_need_notify:    %d\n", otg_device->debug.stat_usb_id_no_trigger);
    DBG_T(MBB_OTG_CHARGER, "stat_host:    %d\n", otg_device->host);

    DBG_T(MBB_OTG_CHARGER, "stat_usb_dpdm_connect:    %d\n", otg_device->debug.stat_usb_dpdm_connect);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_dpdm_disconnect:    %d\n", otg_device->debug.stat_usb_dpdm_disconnect);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_remove_chain:    %d\n", otg_device->debug.stat_usb_id_remove_chain);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_disable_irq:    %d\n", otg_device->debug.stat_usb_otg_fast_off);
    DBG_T(MBB_OTG_CHARGER, "stat_usb_id_enable_irq:    %d\n", otg_device->debug.stat_usb_otg_fast_on);
    DBG_T(MBB_OTG_CHARGER, "otg_feature:    %ld\n", otg_device->otg_feature);

}


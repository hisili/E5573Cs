

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/platform_device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <product_config.h>
#include "gadget_chips.h"
#include <linux/proc_fs.h>
#include <linux/netlink.h>
#include <linux/notifier.h>
#include <mbb_config.h>

#include "usb_platform_comm.h"
#include "hw_pnp.h"
#include "u_ether.h"
#include "adapt/hw_net_dev.h"
#include "usb_nv_get.h"
#include "mbb_fast_on_off.h"
#include "u_serial.h"
#include "f_acm.h"
#include "u_modem.h"
#include "u_cdev.h"
#include "f_mbb_storage.h"
#include <product_config.h>
#ifdef USB_CHARGE
#include <drv_chg.h>
#include <drv_onoff.h>
#endif
#include "usb_debug.h"
#include "usb_platform_comm.h"
#include "mbb_usb_adp.h"
#if (MBB_CHG_EXTCHG == FEATURE_ON)
#include "usb_otg_dev_detect.h"
#endif
#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
#include "hi_gpio.h"

//#define GPIO_USB_SELECT  GPIO_SLEEP_4_10
//#define GPIO_USB_ID      GPIO_SLEEP_4_23
extern int direction_flags;
#endif
#ifdef MBB_Q_9x_PLATFORM
//#include "xxx.h"

#endif /*MBB_Q_PLATFORM*/

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "hw_pnp.c"
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"
#include "composite.c"
//#include "f_sd.c"

#include "f_adb.c"

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
#include "bsp_sram.h"
#include "hi_gpio.h"
#include <linux/gpio.h>

#define FTEN_USB_GPIO GPIO_1_27

static struct delayed_work ften_usb_work;
struct workqueue_struct* ften_usb_wq;
#endif

#include "f_ncm_mbb.h"
#include "f_ecm_mbb.h"
#include "rndis.h"


MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");


/* 高通平台初始化相关操作*/


/* 巴龙平台初始化相关操作*/

static USB_INT g_is_usb_mbb_init = 0;
static USB_INT g_is_usb_balong_init = 0;

extern USB_INT usb_dwc3_platform_dev_init(USB_VOID);
extern USB_VOID usb_dwc3_platform_dev_exit(USB_VOID);
extern USB_INT usb_dwc3_platform_drv_init(USB_VOID);
extern USB_VOID usb_dwc3_platform_drv_exit(USB_VOID);
#ifdef CONFIG_USB_OTG
extern int dwc_otg_init(void);
extern void dwc_otg_exit(void);
#endif

USB_INT usb_balong_init(USB_VOID)
{
    struct android_dev* android_dev;
    USB_INT ret = 0;
    USB_INT err;

    DBG_T(MBB_PNP, "enter");

    if (g_is_usb_balong_init || is_in_dload)
    {
        DBG_E(MBB_PNP, "%s: balong usb init is %d and is_in_dload is %d.\n", __FUNCTION__,
              g_is_usb_balong_init, is_in_dload);
        return -EPERM;
    }

    while (0 == g_is_usb_mbb_init)
    {
        msleep(100); //PMU中断有时较早，需要同步等待USB初始化完毕
    }
#ifdef CONFIG_USB_OTG
    ret = dwc_otg_init();
    if (ret)
    {
        DBG_E(MBB_PNP, "fail:%d\n", ret);
        return ret;
    }
#endif

    ret = usb_dwc3_platform_dev_init();
    if (ret)
    {
        DBG_E(MBB_PNP, "usb_dwc3_platform_dev_init fail:%d\n", ret);
        goto ret_exit;
    }
    ret = usb_dwc3_platform_drv_init();
    if (ret)
    {
        DBG_E(MBB_PNP, "usb_dwc3_platform_drv_init fail:%d\n", ret);
        goto ret_dev_exit;
    }

    if (!android_class)
    {
        android_class = class_create(THIS_MODULE, "android_usb");

        if (IS_ERR(android_class))
        { return PTR_ERR(android_class); }
    }

    android_dev = kzalloc(sizeof(*android_dev), GFP_KERNEL);
    if (!android_dev)
    {
        DBG_E(MBB_PNP, "Failed to alloc memory for android_dev\n");
        ret = -ENOMEM;
        goto ret_drv_exit;
    }

    android_dev->name = "android_usb";
    android_dev->disable_depth = 1;
    android_dev->functions = supported_functions;
    android_dev->configs_num = 0;
    android_dev->irq_ctl_port = 1;
    INIT_LIST_HEAD(&android_dev->android_configs);
    INIT_WORK(&android_dev->work, android_work);
    mutex_init(&android_dev->mutex);

    err = android_create_device(android_dev, 0);
    if (err)
    {
        class_destroy(android_class);
        kfree(android_dev);
        return err;
    }
    _android_dev = android_dev;
    /*
        if (NULL == composite_setup_func)
        {
            composite_setup_func = composite_driver.setup;
        }

        if (NULL == composite_suspend_func)
        {
            composite_suspend_func = composite_driver.suspend;
        }

        if (NULL == composite_resume_func)
        {
            composite_resume_func = composite_driver.resume;
        }

        composite_driver.setup = android_setup;
        // composite_driver.disconnect = android_disconnect;
        composite_driver.suspend = android_suspend;
        composite_driver.resume = android_resume;
    */
    ret =  usb_composite_probe(&android_usb_driver, android_bind);

    if (ret)
    {
        DBG_E(MBB_PNP, "usb_composite_probe fail:%d\n", ret);
        kfree(android_dev);
        _android_dev = NULL;
        goto ret_drv_exit;
    }

    /*pnp start*/
    pnp_probe();

    g_is_usb_balong_init = 1;

    mod_timer(&g_soft_reconnect_timer, jiffies + msecs_to_jiffies(g_soft_reconnect_loop));

    DBG_I(MBB_PNP, "ok\n");

    return 0;

ret_drv_exit:
    usb_dwc3_platform_drv_exit();
ret_dev_exit:
    usb_dwc3_platform_dev_exit();
ret_exit:
#ifdef CONFIG_USB_OTG
    dwc_otg_exit();
#endif

    g_is_usb_balong_init = 0;
    return ret;
}

USB_VOID usb_balong_exit(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();

    DBG_T(MBB_PNP, "enter");

    if (!g_is_usb_balong_init || is_in_dload)
    {
        DBG_E(MBB_PNP, "balong usb init is %d, is_in_dload is %d.\n",
              g_is_usb_balong_init, is_in_dload);
        return;
    }
    /*拔出时清除定时器*/
    del_timer_sync(&g_soft_reconnect_timer);
#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
    if(0 == direction_flags)
    {
        usb_notify_syswatch(DEVICE_ID_USB, USB_REMOVE_NOTIFY);
    }
    direction_flags = 0;
#else
    usb_notify_syswatch(DEVICE_ID_USB, USB_REMOVE_NOTIFY);
#endif
    mutex_lock(&dev->mutex);
    pnp_remove();
    usb_composite_unregister(&android_usb_driver);
    mutex_unlock(&dev->mutex);
    android_destroy_device(dev);
    usb_dwc3_platform_drv_exit();
    usb_dwc3_platform_dev_exit();
#ifdef CONFIG_USB_OTG
    dwc_otg_exit();
#endif
    g_is_usb_balong_init = 0;

    class_destroy(android_class);
    android_class = NULL;
    kfree(dev);
    _android_dev = NULL;
    huawei_set_usb_enum_state(USB_ENUM_NONE);
    DBG_I(MBB_PNP, "ok\n");
    return;
}
#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
USB_VOID usb_mode_init(USB_VOID)
{
    int error;
#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if (!gpio_is_valid(GPIO_USB_SELECT))
    {
        printk("usb_mode_init: set GPIO_USB_SELECT GPIO54 failed!!\r\n ");
        return -1;
    }
    if (!gpio_is_valid(GPIO_USB_ID))
    {
        printk("usb_mode_init: set GPIO_USB_ID GPIO54 failed!!\r\n ");
        return -1;
    }
    if (gpio_request(GPIO_USB_SELECT, "GPIO_USB_SELECT"))
    {
        printk("usb_mode_init: request GPIO_USB_SELECT GPIO55 failed!!\r\n ");
        return -1;
    }
    if (gpio_request(GPIO_USB_ID, "GPIO_USB_ID"))
    {
        printk("usb_mode_init: request GPIO_USB_ID GPIO55 failed!!\r\n ");
        return -1;
    }
    return;
#else
    error = gpio_direction_output(GPIO_USB_SELECT,1);
    if (error < 0)
    {
        DBG_I(MBB_PNP,"%s:Failed to request GPIO error: %d.\n", __FUNCTION__, error);
        return;
    }
    error = gpio_direction_output(GPIO_USB_ID,1);
    if (error < 0)
    {
        DBG_I(MBB_PNP,"%s:Failed to request GPIO error: %d.\n", __FUNCTION__, error);
        return;
    }
    return;
#endif
}
#endif

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
USB_VOID ften_usb_switch(int sel)
{
    int error;
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    huawei_smem_info* smem_data = NULL;
    USB_INT dload_flag = MBB_USB_FALSE;
    smem_data = (huawei_smem_info*)SRAM_DLOAD_ADDR;
    dload_flag = pnp_get_dload_flag_ften();
    DBG_E(MBB_PNP, "enter!\n");

    error = gpio_direction_output(FTEN_USB_GPIO, sel);
    if (error < 0)
    {
        DBG_T(MBB_PNP, "switch to %s USB fail\n", sel ? "Micro" : "Ften");
        return;
    }
    set_ften_cur_usb(sel);

    if ((FTEN_USB_SWITCH_FTEN == sel) && (FEATURE_OFF == MBB_FACTORY) && (FTEN_USB_TRUE == is_ften_custom_usb()))
    {
        DBG_T(MBB_PNP, "report ften desc\n");
        set_ften_desc_flag(FTEN_USB_DESC_FTEN);
    }
    else
    {
        DBG_T(MBB_PNP, "report hw desc\n");
        set_ften_desc_flag(FTEN_USB_DESC_HUAWEI);
    }
    if (( PORT_DLOAD != dload_flag ) && ( PORT_NV_RES != dload_flag ))
    {
        if (NULL != ften_usb_wq)
        {
            queue_delayed_work(ften_usb_wq, &ften_usb_work, msecs_to_jiffies(DETECT_FTEN_USB_ENUM_TIME));
        }
        else
        {
            DBG_T(MBB_PNP, "ften_usb_wq is NULL\n");
        }
    }
    return;
}

USB_VOID ften_switch_usb_func(struct work_struct* work)
{
    USB_INT error = 0;
    USB_INT sel = 0;
    huawei_smem_info* smem_data = NULL;
    smem_data = (huawei_smem_info*)SRAM_DLOAD_ADDR;
    DBG_E(MBB_PNP, "enter\n");
    if (FTEN_USB_ENUM_DONE == get_ften_enum_flag())
    {
        DBG_T(MBB_PNP, "switch correct!\n");
        smem_data->smem_ften_reboot_flag = 0;
        set_ften_supply_type(get_ften_cur_usb());
        return;
    }
    else
    {
        if (SMEM_FTEN_USB_SWITCH == smem_data->smem_ften_reboot_flag)
        {
            DBG_T(MBB_PNP, "AC adapter is connected\n");
            set_ften_supply_type(FTEN_USB_SWITCH_ADAPTER);
            return;
        }
        else
        {
            if (FTEN_USB_SWITCH_MICRO == get_ften_cur_usb())
            {
                DBG_T(MBB_PNP, "auto switch to ften usb\n");
                smem_data->smem_ften_usb_flag = SMEM_FTEN_USB_FLAG;
            }
            else
            {
                DBG_T(MBB_PNP, "auto switch to micro usb\n");
                smem_data->smem_ften_usb_flag = SMEM_MICRO_USB_FLAG;
            }
            DBG_T(MBB_PNP, "going to reboot and switch usb\n");
            smem_data->smem_ften_reboot_flag = SMEM_FTEN_USB_SWITCH;
            if (FTEN_USB_TRUE == is_ften_custom_usb())
            {
                bsp_drv_power_reboot();
            }
            else
            {
                if (FTEN_USB_SWITCH_MICRO == get_ften_cur_usb())
                {
                    sel = FTEN_USB_SWITCH_FTEN;
                }
                else
                {
                    sel = FTEN_USB_SWITCH_MICRO;
                }
                error = gpio_direction_output(FTEN_USB_GPIO, sel);
                if (error < 0)
                {
                    DBG_T(MBB_PNP, "switch to %s USB fail\n", sel ? "Micro" : "Ften");
                    return;
                }
                set_ften_cur_usb(sel);
                if (NULL != ften_usb_wq)
                {
                    queue_delayed_work(ften_usb_wq, &ften_usb_work, msecs_to_jiffies(DETECT_FTEN_USB_ENUM_TIME));
                }
                else
                {
                    DBG_T(MBB_PNP, "ften_usb_wq is NULL\n");
                }
            }
        }
    }
}

USB_VOID ften_usb_switch_init(USB_VOID)
{
    USB_INT error;
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    huawei_smem_info* smem_data = NULL;

    if (gpio_request(FTEN_USB_GPIO, "usb_switch"))
    {
        DBG_T(MBB_PNP, "usb switch gpio request error\n");
    }
    INIT_DELAYED_WORK(&ften_usb_work, ften_switch_usb_func);
    ften_usb_wq = create_singlethread_workqueue("ften_usb");
    DBG_E(MBB_PNP, "enter\n");
    smem_data = (huawei_smem_info*)SRAM_DLOAD_ADDR;
    if (SMEM_MICRO_USB_FLAG == smem_data->smem_ften_usb_flag)
    {
        DBG_T(MBB_PNP, "smem to micro\n");
        ften_usb_switch(FTEN_USB_SWITCH_MICRO);
    }
    else
    {
        if (SMEM_FTEN_USB_FLAG == smem_data->smem_ften_usb_flag)
        {
            DBG_T(MBB_PNP, "smem to ften\n");
            ften_usb_switch(FTEN_USB_SWITCH_FTEN);
        }
        else
        {
            if (0 == usb_nv_info->usb_ften_switch_sel.nv_status)
            {
                DBG_T(MBB_PNP, "not support usb switch\n");
                return;
            }
            if (FTEN_USB_SWITCH_MICRO == usb_nv_info->usb_ften_switch_sel.usb_sel_flag)
            {
                DBG_T(MBB_PNP, "nv to micro\n");
                ften_usb_switch(FTEN_USB_SWITCH_MICRO);
            }
            else
            {
                DBG_T(MBB_PNP, "nv to ften\n");
                ften_usb_switch(FTEN_USB_SWITCH_FTEN);
            }
        }
    }
}
#endif
static USB_INT __init init(USB_VOID)
{
    //USB 日志管理初始化
    usb_debug_init();

    //初始化NV
    usb_nv_init();
    usb_nv_get();
    //app 启动标识模块注册
    
#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
    ften_usb_switch_init();
#endif

    /*USB热拔插*/
#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
    usb_mode_init();
#endif
    usb_hotplug_init();
    bsp_usb_adapter_init();
    usb_mass_storage_init();
    usb_serial_init();

    usb_rndis_init();
    usb_ncm_init();
    usb_ecm_init();

    usb_pnp_init();
    usb_acm_init();
    usb_cdev_init();
    usb_modem_init();
#ifdef USB_CHARGE
    usb_charger_init();
#endif
    fast_on_off_init();
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*初始化对外充事件电接口*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#else
    usb_otg_device_detect_init();
#endif
#endif
    g_is_usb_mbb_init = 1;

    return 0;
}
module_init(init);

static USB_VOID __exit cleanup(USB_VOID)
{
#ifdef USB_CHARGE
    //模块移除的收尾工作
    usb_charger_exit();
#endif
    fast_on_off_exit();
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*移除对外充事件电接口*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#else
    usb_otg_device_detect_exit();
#endif
#endif
    g_is_usb_mbb_init = 0;
}
module_exit(cleanup);



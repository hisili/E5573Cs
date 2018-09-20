/*
该文件包含了所有OTG 设备检测所需的平台差异接口

MBB_USB_UNITARY_Q/MBB_USB_UNITARY_B

后续如果有平台差异修改，请添加到这个文件

usb_otg_dev_detect.c 不要出现平台差异判断

*/
#ifndef MBB_USB_UNITARY_Q
#include <linux/usb/usb_interface_external.h>
#include "usb_vendor.h"
#include "bsp_version.h"
#include "drv_version.h"
#include "mbb_usb_adp.h"
#include "usb_notify.h"
#include <product_config.h>
#include <mbb_config.h>
#else
#include "usb_config.h"
#include <linux/huawei_feature.h>
#include <linux/huawei_netlink.h>
#endif

#ifdef MBB_USB_UNITARY_Q
extern void qc_otg_host_on_off(int id);
extern int qc_otg_get_vbus_state(void);
#endif

#define DMDP_CONNECT_GPIO    (GPIO_2_26)  /*dmdp connect*/
#define OTG_ID_DET_GPIO      (GPIO_0_5)  /*otg detect*/
#if defined(BSP_CONFIG_BOARD_E5577BS_937)
#define OTG_ID_SET_GPIO      (GPIO_2_2)  /*otg set*/
#else
#define OTG_ID_SET_GPIO      (GPIO_2_3)  /*otg set*/
#endif
#define OTG_ID_SWITCH_GPIO   (GPIO_2_5)
#define OTG_VBUS_DET_GPIO    (GPIO_1_27)


#define GPIO_OTG_ID_DET      OTG_ID_DET_GPIO
#define GPIO_OTG_ID_SET      OTG_ID_SET_GPIO
#define GPIO_DMDP_CONNECT    DMDP_CONNECT_GPIO


#define GPIO_DMDP_SWITCH     OTG_ID_SWITCH_GPIO
#define GPIO_OTG_VBUS_DET    OTG_VBUS_DET_GPIO

#define CHECK_ID_VOLT_LOW       (150)
#define CHECK_ID_VOLT_HIG       (250)


#define USB_OTG_DEV_DETECT
static USB_VOID otg_dev_request_source(USB_VOID)
{
#ifndef MBB_USB_UNITARY_Q
    USB_INT ret;

    printk("GPIO_DMDP_CONNECT = %d\n", GPIO_DMDP_CONNECT);
    ret = gpio_request(GPIO_DMDP_CONNECT, "OTG_DP_DM_CONNECT_GPIO");
    if (ret < 0)
    {
        DBG_E(MBB_OTG_CHARGER, "gpio request failed for OTG_DP_DM_CONNECT_GPIO\n");
        gpio_free(GPIO_DMDP_CONNECT);
    }

    printk("GPIO_OTG_ID_SET = %d\n", GPIO_OTG_ID_SET);
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
    gpio_direction_output(GPIO_DMDP_CONNECT, 0);
    gpio_direction_output(GPIO_OTG_ID_SET, 1);
#else
    /*qualcomm*/
#endif
}
static USB_VOID otg_dev_free_source(USB_VOID)
{
#ifndef MBB_USB_UNITARY_Q

    if (gpio_is_valid(GPIO_DMDP_CONNECT))
    {
        gpio_free(GPIO_DMDP_CONNECT);
    }
    if (gpio_is_valid(GPIO_OTG_ID_SET))
    {
        gpio_free(GPIO_OTG_ID_SET);
    }
#else
    /*qualcomm*/
#endif

}
static USB_VOID otg_dev_set_platform(struct otg_dev_det* otg)
{
#ifdef MBB_USB_UNITARY_Q
    otg->platform = PLATFORM_QUALCOMM;
#else
    otg->platform = PLATFORM_BALONG;
#endif
}

/*编译问题，该函数用宏报，
如果后续，高通用，可去掉宏*/

static USB_VOID  otg_gpio_clear_set(USB_INT gpio)
{
#ifndef MBB_USB_UNITARY_Q
    gpio_int_unmask_set(gpio);
    gpio_int_state_clear(gpio);
#else
    /*qualcomm operation*/
#endif
}
static USB_VOID  otg_gpio_set(USB_INT gpio)
{
#ifndef MBB_USB_UNITARY_Q
    gpio_int_mask_set(gpio);
    gpio_int_state_clear(gpio);
#else
    /*qualcomm operation*/
#endif
}


USB_VOID otg_id_gpio_irq_set(USB_INT id_gpio)
{
#ifndef MBB_USB_UNITARY_Q
    gpio_int_mask_set(id_gpio);
    gpio_int_state_clear(id_gpio);
    gpio_set_function(id_gpio, GPIO_INTERRUPT);
#endif
}

USB_VOID otg_id_trigger_set(struct otg_dev_det* otg)
{
#ifndef MBB_USB_UNITARY_Q
    gpio_int_trigger_set(otg->id_det_gpio,
                         otg->id ? IRQ_TYPE_LEVEL_LOW : IRQ_TYPE_LEVEL_HIGH);
#endif
}

static USB_VOID otg_host_on_off(struct otg_dev_det* otg)
{
#ifdef MBB_USB_UNITARY_Q
    qc_otg_host_on_off(otg->phy_id);
#else/*balong*/
    if (!otg->phy_id)
    {
        bsp_usb_status_change_ex(USB_BALONG_PERIP_INSERT);
    }
    else
    {
        bsp_usb_status_change_ex(USB_BALONG_PERIP_REMOVE);
    }

    /*断开 D+, D- , 拉低 ID */
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
    gpio_set_value(GPIO_OTG_ID_SET, otg->phy_id);
#endif

    otg->host = otg->phy_id ? MBB_USB_FALSE : MBB_USB_TRUE;
}

static USB_INT otg_get_vbus_state(struct otg_dev_det* otg)
{
#ifdef MBB_USB_UNITARY_Q
    return qc_otg_get_vbus_state();
#else
    return usb_get_vbus_status();
#endif
}

static USB_VOID otg_id_irq_flags_set(struct otg_dev_det* otg)
{
#ifndef MBB_USB_UNITARY_Q
    otg->id_irqflags = IRQF_NO_SUSPEND | IRQF_SHARED;
    otg->id_irq_shared = MBB_USB_TRUE;
#else/*MBB_USB_UNITARY_Q*/
    otg->id_irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
    otg->id_irq_shared = MBB_USB_FALSE;
#endif
}

static USB_INT otg_id_irq_share_protect(struct otg_dev_det* otg)
{
#ifndef MBB_USB_UNITARY_Q
    return gpio_int_state_get((unsigned)otg->id_det_gpio);
#else
    return 1;
#endif
}
static USB_VOID otg_first_report_charge_type(struct otg_dev_det* otg)
{
    //static USB_INT first_report = MBB_USB_TRUE;
#ifndef MBB_USB_UNITARY_Q
    static USB_INT first_report = MBB_USB_TRUE;
    usb_adp_ctx_t* ctx = otg->contex;
    if (ctx->stm_set_chg_type_cb && first_report)
    {
        DBG_I(MBB_OTG_CHARGER, "-----first notify charger event----\n");
        ctx->stm_set_chg_type_cb(otg->charge_type);
        first_report = MBB_USB_FALSE;
    }
#else
    /*qualcomm*/
#endif
}

static USB_VOID otg_dev_source_set(USB_VOID)
{
#ifndef  MBB_USB_UNITARY_Q
    otg_gpio_set(GPIO_DMDP_CONNECT);
    //gpio_direction_output(GPIO_DMDP_CONNECT, 0);

    otg_gpio_set(GPIO_OTG_ID_SET);
    //gpio_direction_output(GPIO_OTG_ID_SET, 1);
#else
    /*qualcommm operation*/
#endif
}
static USB_VOID otg_exchg_connect_dpdm(struct otg_dev_det* otg)
{
    /*短接 D+ D- , 进行 1 A 充电*/
    otg->debug.stat_usb_dpdm_connect++;
#ifndef MBB_USB_UNITARY_Q
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_HIGH);
#else
    /*qualcomm op*/
#endif
}
static USB_VOID otg_exchg_disconnect_dpdm(struct otg_dev_det* otg)
{
    otg->debug.stat_usb_dpdm_disconnect++;
#ifndef MBB_USB_UNITARY_Q
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
#else
    /*qualcomm op */
#endif
}
static USB_VOID otg_exchg_disconnect_dpdm_to_host(struct otg_dev_det* otg)
{
    otg->debug.stat_usb_dpdm_disconnect++;
#ifndef MBB_USB_UNITARY_Q
    gpio_set_value(GPIO_DMDP_CONNECT, GPIO_LOW);
#else
    /*qualcomm op */
#endif
    otg->phy_id = HOST_ON;
    otg_host_on_off(otg);
}










/*********************************************************************
函数  : usb_otg_set_support_feature
功能  :设置当前otg特性
参数  : viod
返回值: void
*********************************************************************/
USB_VOID usb_otg_set_support_feature(USB_UINT32 feature)
{
    otg_device->otg_feature = feature;
}

/*不同产品对 OTG 设备的设置,
    该函数由产品自行设置
*/
USB_VOID  product_set_otg_dev_support_feature(USB_VOID)
{
#ifdef MBB_USB_UNITARY_B
    /*E5 */

    USB_UINT32 product_type = 0;
    product_type = bsp_version_get_board_type();
    switch (product_type)
    {
        case HW_VER_PRODUCT_E5575S_210:
        case HW_VER_PRODUCT_E5575S_210_VB:
        case HW_VER_PRODUCT_E5575S_320:
            usb_otg_set_support_feature( USB_OTG_FEATURE_CRADLE);
            break;
        case HW_VER_PRODUCT_E5577S_321:
        case HW_VER_PRODUCT_E5577S_324:
            usb_otg_set_support_feature( USB_OTG_FEATURE_EXTCHG);
            break;
        case HW_VER_PRODUCT_E5577S_932:
        case HW_VER_PRODUCT_E5577BS_932:
        case HW_VER_PRODUCT_E5577BS_937:
            usb_otg_set_support_feature( USB_OTG_FEATURE_EXTCHG);
            break;
        default:
            usb_otg_set_support_feature( USB_OTG_FEATURE_NONE);
            break;
    }
    
#else
    /*case KD01:*/
    /*usb_otg_set_support_feature(USB_OTG_FEATURE_NONE);*/

    /*case KD02:*/
    usb_otg_set_support_feature(USB_OTG_FEATURE_CRADLE);
#endif
}



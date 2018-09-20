




/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "chg_config.h"
#if ( FEATURE_ON == MBB_MLOG )
#include <linux/mlog_lib.h>
#endif
#include "bsp_coul.h"
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#include <bsp_hkadc.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input/key_balong.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include "../../usb/mbb_usb_unitary/usb_hotplug.h"
#endif

#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#include <product_nv_id.h>
#endif
#ifndef CHG_STUB
#include <linux/usb/usb_interface_external.h>
#else
enum chg_current
{
    CHG_CURRENT_HS =0,    /*usb2.0 for 500mA*/
    CHG_CURRENT_SS ,        /*usb3.0 for 900mA */
    CHG_CURRENT_NO,        /*invalid usb for no current*/
};
int usb_speed_work_mode(void)
{
    return 0;
}

#define USB_OTG_CONNECT_DP_DM               (0x0001)
#define USB_OTG_DISCONNECT_DP_DM            (0x0002)  /*直连基带，拉低HS_ID*/
#define USB_OTG_ID_PULL_OUT                 (0x0003)  /*直连基带，拉高HS_ID*/
#define USB_OTG_DISABLE_ID_IRQ              (0x0004)
#define USB_OTG_ENABLE_ID_IRQ               (0x0005)
#endif
#if defined(BSP_CONFIG_BOARD_E5771H_937)
#define  HW_VER_DISTINGUISH      950
#define  RSIISTANCE_CENT_COUNT   7
#endif
/*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#define TEMP_INITIAL_VALUE                  (0xFFFF)   /*温度初始化值*/
#define USB_TEMP_DETECT_COUNT               (4)        /*USB温度连续检测次数+1*/
#endif
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern struct chargeIC_chip *g_chip;
extern struct softtimer_list g_chg_sleep_timer;
extern int32_t is_5577cs_falg;    /*FALSE:不是5577Cs-321;TRUE:是5577Cs-321*/
uint32_t  mbb_fastonoff_chg_flag = false;

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern int battery_monitor_blocking_notifier_call_chain(unsigned long val, unsigned long v);

struct chg_hardware_data *g_chg_hardware_data = NULL;

struct chg_batt_data *g_chg_batt_data = NULL;

int32_t is_bat_3000 = FALSE;    /*FALSE:不是5577S-321的3000mA电池;TRUE:是5577S-321的3000mA电池*/
int32_t is_vbus_unkonwn = FALSE;  /*FALSE:充电IC的STAT寄存器VBUS不为unkonwn;TRUE:充电IC的STAT寄存器VBUS为unkonwn*/
#if defined(BSP_CONFIG_BOARD_E5) || defined(BSP_CONFIG_BOARD_E5573S_156) \
    || defined(BSP_CONFIG_BOARD_E5573S_856) || defined(BSP_CONFIG_BOARD_E5573S_853) \
    || defined(BSP_CONFIG_BOARD_E5573S_607) || defined(BSP_CONFIG_BOARD_E5577CS_603) \
    || defined(BSP_CONFIG_BOARD_E5575S_320)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3638,3638,3721,3805,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 887,/*batt id volt*/
        .id_voltage_max = 1137,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3620,3620,3708,3779,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data chg_batt_data_lishen = {
        .id_voltage_min = 102,/*batt id volt*/
        .id_voltage_max = 352,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3621,3621,3709,3798,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN,
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
    &chg_batt_data_lishen,

};


#elif defined(BSP_CONFIG_BOARD_E5573CS)|| defined(BSP_CONFIG_BOARD_E5573BS) \
    || defined(BSP_CONFIG_BOARD_R218h)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3638,3638,3721,3805,3970,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 1542,/*batt id volt*/
        .id_voltage_max = 1792,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3620,3620,3708,3779,3970,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data chg_batt_data_lishen = {
        .id_voltage_min = 102,/*batt id volt*/
        .id_voltage_max = 352,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3621,3621,3709,3798,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN,
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
    &chg_batt_data_lishen,

};

#elif defined(BSP_CONFIG_BOARD_E5573CS_933)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3638,3638,3721,3805,3970,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 1542,/*batt id volt*/
        .id_voltage_max = 1792,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3620,3620,3708,3779,3970,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data chg_batt_data_lishen = {
        .id_voltage_min = 102,/*batt id volt*/
        .id_voltage_max = 352,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3621,3621,3709,3798,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN,
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
    &chg_batt_data_lishen,

};

/**E5577S-603 使用3000大电池*/
#elif defined(BSP_CONFIG_BOARD_E5577S_603)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1185,  /*batt id volt*/
        .id_voltage_max = 1435,  /*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3606,3606,3669,3752,3836,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 1542, /*batt id volt*/
        .id_voltage_max = 1792, /*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3629,3629,3674,3746,3826,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data chg_batt_data_lishen = {
        .id_voltage_min = 577, /*batt id volt*/
        .id_voltage_max = 827, /*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3600,3600,3652,3740,3829,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
    &chg_batt_data_lishen,

};
#elif defined(BSP_CONFIG_BOARD_E5577S_321)|| defined(BSP_CONFIG_BOARD_E5577S_324)\
   || defined(BSP_CONFIG_BOARD_E5577BS_937)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3638,3638,3721,3805,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 887,/*batt id volt*/
        .id_voltage_max = 1137,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3620,3620,3708,3779,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data chg_batt_data_lishen = {
        .id_voltage_min = 102,/*batt id volt*/
        .id_voltage_max = 352,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3621,3621,3709,3798,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN,
};
static struct chg_batt_data chg_batt_data_xingwangda_3000 = {
        .id_voltage_min = 1185,  /*batt id volt*/
        .id_voltage_max = 1435,  /*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3606,3606,3669,3752,3836,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_3000,
};
static struct chg_batt_data chg_batt_data_feimaotui_3000 = {
        .id_voltage_min = 1542, /*batt id volt*/
        .id_voltage_max = 1792, /*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3629,3629,3674,3746,3826,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_3000,
};
static struct chg_batt_data chg_batt_data_lishen_3000 = {
        .id_voltage_min = 577, /*batt id volt*/
        .id_voltage_max = 827, /*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3600,3600,3652,3740,3829,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN_3000,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
    &chg_batt_data_lishen,
    &chg_batt_data_xingwangda_3000,
    &chg_batt_data_feimaotui_3000,
    &chg_batt_data_lishen_3000,

};
#elif defined(BSP_CONFIG_BOARD_E5577S_932)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda_3000 = {
        .id_voltage_min = 1185, // 1937,/*batt id volt*/
        .id_voltage_max = 1435, // 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3638,3638,3721,3805,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA_3000,
};
static struct chg_batt_data chg_batt_data_feimaotui_3000 = {
        .id_voltage_min = 1542, // 887,/*batt id volt*/
        .id_voltage_max = 1792, // 1137,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3620,3620,3708,3779,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI_3000,
};
static struct chg_batt_data chg_batt_data_lishen_3000 = {
        .id_voltage_min = 577, // 102,/*batt id volt*/
        .id_voltage_max = 827, // 352,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3621,3621,3709,3798,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN_3000,
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda_3000,
    &chg_batt_data_feimaotui_3000,
    &chg_batt_data_lishen_3000,

};
#elif defined(BSP_CONFIG_BOARD_E5_E5578) 
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1000,/*batt id volt*/
        .id_voltage_max = 1200,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4060,3655,3655,3745,3815,4060,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 300,/*batt id volt*/
        .id_voltage_max = 500,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4050,3655,3655,3740,3815,4050,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,

};
#elif defined(BSP_CONFIG_BOARD_E5575S_210)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3754,3754,3805,4030,4120,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_lishen = {
        .id_voltage_min = 102,/*batt id volt*/
        .id_voltage_max = 352,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3754,3754,3805,4030,4120,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_LISHEN,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3754,3754,3805,4030,4120,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 887,/*batt id volt*/
        .id_voltage_max = 1137,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3754,3754,3805,4030,4120,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_lishen,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
};
#elif defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
   || defined(BSP_CONFIG_BOARD_506HW_2)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4050,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4150,4030,3630,3630,3720,3800,4030,4250,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
};
#elif defined(BSP_CONFIG_BOARD_607HW)
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4220,4240,4100,4000,3550,3550,3610,3670,3770,4150,4100},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1937,/*batt id volt*/
        .id_voltage_max = 2187,/*batt id volt*/
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4250,4000,3638,3638,3721,3805,3970,4280,4200},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
};
#elif defined(BSP_CONFIG_BOARD_E5771S_852) 
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3300,3100,4220,4240,4100,4050,3336,3336,3500,3650,3900,4200,4050},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1000,/*batt id volt*/
        .id_voltage_max = 1200,/*batt id volt*/
        .chg_batt_volt_paras        = {3300,3100,4220,4240,4100,4050,3336,3336,3500,3650,3900,4200,4050},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 300,/*batt id volt*/
        .id_voltage_max = 500,/*batt id volt*/
        .chg_batt_volt_paras        = {3300,3100,4220,4240,4100,4050,3336,3336,3500,3650,3900,4200,4050},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
};
#elif defined(BSP_CONFIG_BOARD_E5771H_937) /* E5771h-937无假关机，开关机门限与E5771s-852不一致 */
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3300,3300,4220,4240,4100,4050,3336,3336,3500,3650,3900,4200,4050},/*batt data*/
        .batt_id                    = CHG_BATT_ID_DEF,
};
static struct chg_batt_data chg_batt_data_xingwangda = {
        .id_voltage_min = 1000,/*batt id volt*/
        .id_voltage_max = 1200,/*batt id volt*/
        .chg_batt_volt_paras        = {3300,3300,4220,4240,4100,4050,3336,3336,3500,3650,3900,4200,4050},/*batt data*/
        .batt_id                    = CHG_BATT_ID_XINGWANGDA,
};
static struct chg_batt_data chg_batt_data_feimaotui = {
        .id_voltage_min = 300,/*batt id volt*/
        .id_voltage_max = 500,/*batt id volt*/
        .chg_batt_volt_paras        = {3300,3300,4220,4240,4100,4050,3336,3336,3500,3650,3900,4200,4050},/*batt data*/
        .batt_id                    = CHG_BATT_ID_FEIMAOTUI,
};
static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,
    &chg_batt_data_xingwangda,
    &chg_batt_data_feimaotui,
};
#else
static struct chg_batt_data chg_batt_data_default = {
        .chg_batt_volt_paras        = {3450,3450,4370,4390,4144,4100,3559,3559,3620,3710,3860,0,0},/*batt data*/
};

static struct chg_batt_data *chg_batt_data_array[] = {
    &chg_batt_data_default,

};
#endif

#define BATTERY_DATA_ARRY_SIZE sizeof(chg_batt_data_array) / sizeof(chg_batt_data_array[0])

#if defined(BSP_CONFIG_BOARD_E5_E5578)
#define CALI_VLOT_FEIMAOTUI     (20)
#define CALI_VLOT_XINGWANGDA    (5)
#endif
#if defined(BSP_CONFIG_BOARD_E5573S_856) || defined(BSP_CONFIG_BOARD_E5573S_853) \
    || defined(BSP_CONFIG_BOARD_E5573S_607) || defined(BSP_CONFIG_BOARD_E5573CS) \
    || defined(BSP_CONFIG_BOARD_E5573BS) || defined(BSP_CONFIG_BOARD_E5573CS_933)\
    || defined(BSP_CONFIG_BOARD_607HW) || defined(BSP_CONFIG_BOARD_E5577BS_937) \
    || defined(BSP_CONFIG_BOARD_R218h)
#define CALI_VLOT_LISHEN        (50)
#define CALI_VLOT_FEIMAOTUI     (20)
#define CALI_VLOT_XINGWANGDA    (5)
#endif
/*----------------------------------------------*
 * 数据类型定义说明                             *
 *----------------------------------------------*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#define DEV_OTG_AB            0xBB
#define NUM_OF_LIMIT          (30)
#define BATT_VOLTAGE_3600     (3600)
#define BATT_VOLTAGE_3500     (3500)
#define BATT_VOLTAGE_3350     (3350)
#define BATT_VOLTAGE_3300     (3300)
#define BATT_VOLTAGE_3100     (3100)
#define BATT_VOLTAGE_3000     (3000)
#define BATT_VOLTAGE_2940     (2940)
#define DELAY_TIME_OF_DEBOUNCE (200)
#define DELAY_TIME_20MS        (20)
#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#define USB_SWITCH_APORT_LEVEL      (1)
#define USB_SWITCH_MPORT_LEVEL      (0)
#else
#define USB_SWITCH_APORT_LEVEL      (0)
#define USB_SWITCH_MPORT_LEVEL      (1)
#endif
int ext_dev_id = DEV_OTG_AB;
struct work_struct extchg_plug_in_work;//外充插入工作队列
struct work_struct extchg_plug_out_work;//外充拔出工作队列
#endif

#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#define BATTERY_SN (32)
typedef struct
{
    unsigned char  battery_sn[BATTERY_SN];
    unsigned int  coul_firmware_update_status;
}nv_huawei_coul_firmware_update_info;

#define BATT_CURRENT_I_3000            (-3000)
#define BATT_CURRENT_I_2500            (-2500)
#define BATT_CURRENT_I_1500            (-1500)
#endif

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*0:-Charge Transition State TRIO function.*/
static void chg_transit_state_entry_func(void);
static void chg_transit_state_period_func(void);
static void chg_transit_state_exit_func(void);

/*1:-Fast Charge State TRIO function.*/
static void chg_fastchg_state_entry_func(void);
static void chg_fastchg_state_period_func(void);
static void chg_fastchg_state_exit_func(void);

/*2:-Battery Maintenance State TRIO function.*/
static void chg_maint_state_entry_func(void);
static void chg_maint_state_period_func(void);
static void chg_maint_state_exit_func(void);

/*3:-Invalid Charge Temperature State TRIO function.*/
static void chg_invalid_chg_temp_state_entry_func(void);
static void chg_invalid_chg_temp_state_period_func(void);
static void chg_invalid_chg_temp_state_exit_func(void);

/*4:-Battery Only State TRIO function.*/
static void chg_batt_only_state_entry_func(void);
static void chg_batt_only_state_period_func(void);
static void chg_batt_only_state_exit_func(void);


//static void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param);

static void chg_stm_set_cur_state(chg_stm_state_type new_state);


#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*5:-Warmchg State TRIO function.*/
static void chg_warmchg_state_entry_func(void);
static void chg_warmchg_state_period_func(void);
static void chg_warmchg_state_exit_func(void);
#endif /* MBB_CHG_WARM_CHARGE */

void chg_sleep_batt_check_timer(struct work_struct *work);
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
int32_t chg_batt_temp_revise(int32_t batt_temp);
#endif
/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
static uint32_t g_real_factory_mode; //是否工厂模式
static boolean chg_batt_high_temp_58_flag = FALSE; //高温58度标志
static boolean chg_batt_low_battery_flag = FALSE;  //低电标志
static boolean chg_batt_condition_error_flag = FALSE;

static boolean chg_limit_supply_current_flag = FALSE; //温保限流标识

#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
static int32_t battery_current = 0;
static int32_t battery_current_prev = 0;
int32_t tbatt_supply_volt_threshold = 0;
int32_t tbatt_dischg_volt_threshold = 0;
int32_t tbatt_supply_stop_threshold = 0;
int32_t tbatt_dischg_stop_threshold = 0;
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
static int32_t count_for_short_det = 0;
static int32_t poll_round_for_det = 0;
static boolean extchg_short_flag = FALSE;
int32_t extchg_limit_1a_capacity = 0;
int32_t extchg_limit_500ma_capacity = 0;
int32_t extchg_stop_threshold_powon = 0;
int32_t extchg_stop_threshold_powdown = 0;
int32_t extchg_recharge_threshold = 0;
int32_t battery_voltage = 0xFFFF;
int32_t battery_temperature = 0;
boolean g_extchg_shutoff_flag = FALSE;
static boolean usb_set_direction_flag = FALSE;
#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
static EXTCHG_ILIM ext_chg_status_old = RE_ILIM_NA;
static EXTCHG_ILIM ext_chg_status_new = RE_ILIM_STOP;
static EXTCHG_ILIM ext_chg_status_temp = RE_ILIM_NA;
#else
static EXTCHG_ILIM ext_chg_status_old = RE_ILIM_2A;
static EXTCHG_ILIM ext_chg_status_new = RE_ILIM_2A;
static EXTCHG_ILIM ext_chg_status_temp = RE_ILIM_2A;
#endif
static boolean usb_err_flag = FALSE;
#endif

/*充电超时标志位*/
static boolean chg_time_expired_flag = FALSE;

#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
uint32_t up_over_temp_shutoff_falg = FALSE;
uint32_t low_over_temp_shutoff_falg = FALSE;
#endif

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
/*电池保护标识*/
static boolean chg_battery_protect_flag = FALSE;
/*过温过压保护标识*/
static boolean g_chg_over_temp_volt_protect_flag = FALSE;
/*满电停充电源长时间在位状态标识*/
static boolean g_chg_longtime_nocharge_protect_flag = FALSE;
/*保存进入 满电停充且充电器长时间在位状态前常温复充门限*/
static unsigned int g_batt_normal_temp_recherge_threshold = 0;
/*电池膨胀，满电停冲充电器长时间在位，常温复充门限为4.0V标志*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
static boolean g_chg_batt_expand_change_normal_rechg_flag = FALSE;
#endif
#endif

/*充放电相关NV*/
CHG_BATTERY_OVER_TEMP_PROTECT_NV   g_chgBattOverTempPeotect;  //NV50016
CHG_BATTERY_LOW_TEMP_PROTECT_NV    g_chgBattLowTempPeotect;   //NV52005
CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE   g_chgShutOffTempProtect;   //NV50385
CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE   g_chgBattVoltProtect;      //NV50386
POWERUP_MODE_TYPE                  g_real_powerup_mode_value; //NV50364

/***************************begin NV50016 数据宏定义 ************************/
/*SHUTOFF 高温关机使能开关*/
#define SHUTOFF_OVER_TEMP_PROTECT_ENABLE g_chgBattOverTempPeotect.ulIsEnable
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD g_chgBattOverTempPeotect.lCloseAdcThreshold
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_OVER_TEMP_SHUTOFF_CHECK_TIMES g_chgBattOverTempPeotect.ulTempOverCount
/***************************end   NV50016 数据宏定义 ************************/

/***************************begin NV52005 数据宏定义 ************************/
/*SHUTOFF 高温关机使能开关*/
#define SHUTOFF_LOW_TEMP_PROTECT_ENABLE g_chgBattLowTempPeotect.ulIsEnable
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD g_chgBattLowTempPeotect.lCloseAdcThreshold
/*SHUTOFF 高温关机温度门限*/
#define SHUTOFF_LOW_TEMP_SHUTOFF_CHECK_TIMES g_chgBattLowTempPeotect.ulTempLowCount
/***************************end  NV52005 数据宏定义 ************************/


/************************************begin NV50385 数据宏定义 *******************************/
/*CHG 充电温保护使能开关*/
#define CHG_TEMP_PROTECT_ENABLE g_chgShutOffTempProtect.ulChargeIsEnable
/*CHG 充电高温/低温保护门限*/
#define CHG_OVER_TEMP_STOP_THRESHOLD g_chgShutOffTempProtect.overTempchgStopThreshold
#define CHG_LOW_TEMP_STOP_THRESHOLD g_chgShutOffTempProtect.lowTempChgStopThreshold
/*CHG 充电高温/低温恢复温度门限*/
#define CHG_OVER_TEMP_RESUME_THRESHOLD g_chgShutOffTempProtect.overTempChgResumeThreshold
#define CHG_LOW_TEMP_RESUME_THRESHOLD g_chgShutOffTempProtect.lowTempChgResumeThreshold
/*CHG 充电停充轮询次数*/
#define CHG_TEMP_PROTECT_CHECK_TIMES g_chgShutOffTempProtect.chgTempProtectCheckTimes
/*CHG 充电复充轮询次数*/
#define CHG_TEMP_RESUME_CHECK_TIMES g_chgShutOffTempProtect.chgTempResumeCheckTimes
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
/*进入高温充电电池温度门限*/
#define CHG_WARM_CHARGE_ENTER_THRESHOLD g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold
/*由高温充电恢复到常温充电电池温度门限*/
#define CHG_WARM_CHARGE_EXIT_THRESHOLD g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold
#endif /* HUAWEI_CHG_WARM_CHARGE */
/**********************************end NV50385 数据宏定义 *******************************/


/************************************begin NV50386 数据宏定义 *******************************/
/*开机电压门限*/
#define BATT_VOLT_POWER_ON_THR g_chgBattVoltProtect.battVoltPowerOnThreshold
/*关机电压门限*/
#define BATT_VOLT_POWER_OFF_THR g_chgBattVoltProtect.battVoltPowerOffThreshold
/*充电过压保护门限(平滑值)*/
#define BATT_CHG_OVER_VOLT_PROTECT_THR g_chgBattVoltProtect.battOverVoltProtectThreshold
/*充电过压保护门限(单次采集值)*/
#define BATT_CHG_OVER_VOLT_PROTECT_ONE_THR g_chgBattVoltProtect.battOverVoltProtectOneThreshold
/*区分高温停充和正常停充的判断门限*/
#define BATT_CHG_TEMP_MAINT_THR g_chgBattVoltProtect.battChgTempMaintThreshold
/*充电二次复充门限*/
#define BATT_HIGH_TEMP_RECHARGE_THR g_chgBattVoltProtect.battChgRechargeThreshold

/*低电上限门限*/
#define BATT_VOLT_LEVELLOW_MAX g_chgBattVoltProtect.VbatLevelLow_MAX
/*0格电压上限门限*/
#define BATT_VOLT_LEVEL0_MAX g_chgBattVoltProtect.VbatLevel0_MAX
/*1格电压上限门限*/
#define BATT_VOLT_LEVEL1_MAX g_chgBattVoltProtect.VbatLevel1_MAX
/*2格电压上限门限*/
#define BATT_VOLT_LEVEL2_MAX g_chgBattVoltProtect.VbatLevel2_MAX
/*3格电压上限门限 */
#define BATT_VOLT_LEVEL3_MAX g_chgBattVoltProtect.VbatLevel3_MAX
/*初始判断是否启动充电电压门限*/
#define BATT_CHG_FIRST_MAINT_THR g_chgBattVoltProtect.battChgFirstMaintThreshold
/*常温二次复充门限*/
#define BATT_NORMAL_TEMP_RECHARGE_THR g_chgBattVoltProtect.battNormalTempChgRechargeThreshold
/************************************begin NV50386 数据宏定义 *******************************/
/* 当前是否为工厂模式: true ---工厂模式 ; false ---非工厂模式**/
static boolean chg_current_ftm_mode = FALSE;
/*记录充电温保护标志,初始化为温度正常**/
static uint32_t chg_temp_protect_flag = FALSE;

static uint32_t fact_release_flag = FALSE;

/*记录电池温度状态*/
TEMP_EVENT chg_batt_temp_state = TEMP_BATT_NORMAL;

static uint32_t chg_print_flag = FALSE;
static uint32_t coul_print_flag = FALSE;

/*记录实际的充电状态信息**/
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
CHG_PROCESS_INFO chg_real_info = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0, BATT_LEVEL_0, 0,0xFFFF, 0xFFFF};
#else
CHG_PROCESS_INFO chg_real_info = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0, BATT_LEVEL_0, 0,0xFFFF};
#endif
/*记录USB端口温度保护信息*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
static USB_TEMP_PROTECT_INFO chg_usb_temp_info = {TEMP_INITIAL_VALUE, FALSE, FALSE};
#endif

static chg_stm_type chg_stm_state_machine[CHG_STM_MAX_ST + 1] =
{
    /* 0 Transition State, which is also the default after system boot-up.*/
    {
        (chg_stm_func_type)chg_transit_state_entry_func,
        (chg_stm_func_type)chg_transit_state_period_func,
        (chg_stm_func_type)chg_transit_state_exit_func,
    },

    /* 1 Fast Charge State, battery is charged during this state.*/
    {
        (chg_stm_func_type)chg_fastchg_state_entry_func,
        (chg_stm_func_type)chg_fastchg_state_period_func,
        (chg_stm_func_type)chg_fastchg_state_exit_func,
    },

    /* 2 Maintenance State, battery has been charged to full, system was supplied by
         external charger preferentially.*/
    {
        (chg_stm_func_type)chg_maint_state_entry_func,
        (chg_stm_func_type)chg_maint_state_period_func,
        (chg_stm_func_type)chg_maint_state_exit_func,
    },

    /* 3 Invalid Charge Temperature State, external charger is present, while battery
         is too hot/cold to charge.*/
    {
        (chg_stm_func_type)chg_invalid_chg_temp_state_entry_func,
        (chg_stm_func_type)chg_invalid_chg_temp_state_period_func,
        (chg_stm_func_type)chg_invalid_chg_temp_state_exit_func,
    },

    /* 4 Battery Only State, external charger is absent, or removed, system was supplied
         by battery.*/
    {
        (chg_stm_func_type)chg_batt_only_state_entry_func,
        (chg_stm_func_type)chg_batt_only_state_period_func,
        (chg_stm_func_type)chg_batt_only_state_exit_func,
    },

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /* 5 warm chg State, battery is charged during this state when the battery temp is in
      warm chg area*/
    {
        (chg_stm_func_type)chg_warmchg_state_entry_func,
        (chg_stm_func_type)chg_warmchg_state_period_func,
        (chg_stm_func_type)chg_warmchg_state_exit_func,
    },
#endif /* MBB_CHG_WARM_CHARGE */


    {
        NULL,NULL,NULL
    }
};
/*----------------------------------------------*
 * 全局变量                                   *
 *----------------------------------------------*/
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0,0
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
,0
#endif
};
#else
chg_stm_state_info_t chg_stm_state_info = {0,0,0,0,0,0,0,CHG_BAT_ONLY_MODE,0,0,0,0
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT ) && (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
,0,0
#elif ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
,0
#else ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
,0
#endif
};
#endif


/* 充电使能标志提供给USB驱动用于弱充检测1:使能充电 0:没有使能*/
int chg_en_flag = 0;
unsigned long chg_en_timestamp = 0;
#define CHG_EN_TIME_SLOT    (50)
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/* 0 - 10 ℃充电相关参数设定，针对1900mah电池 */
int chg_hw_para_updated = 0;
int chg_sub_low_temp_changed = 0;
#define CHG_SUB_LOW_TEMP_TOP                (10)
#define CHG_TEMP_RESUM                      ( 3)
#if defined(BSP_CONFIG_BOARD_E5_E5578) 
#define CHG_SUB_LOW_TEMP_CURR_WALL               (576)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (576)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (576)
#elif defined(BSP_CONFIG_BOARD_E5) || defined(BSP_CONFIG_BOARD_E5575S_210) \
    ||defined(BSP_CONFIG_BOARD_E5573S_156) || defined(BSP_CONFIG_BOARD_E5573S_856) \
    || defined(BSP_CONFIG_BOARD_E5573S_853) || defined(BSP_CONFIG_BOARD_E5573S_607) \
    || defined(BSP_CONFIG_BOARD_E5577CS_603) || defined(BSP_CONFIG_BOARD_E5575S_320) \
    || defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS) \
    || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW) \
    || defined(BSP_CONFIG_BOARD_R218h)
#define CHG_SUB_LOW_TEMP_CURR_WALL             (448)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (448)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (448)
#elif defined(BSP_CONFIG_BOARD_E5577S_603)
#define CHG_SUB_LOW_TEMP_CURR_WALL             (896)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (576)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (896)
#elif defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
    ||defined(BSP_CONFIG_BOARD_E5577S_324) || defined(BSP_CONFIG_BOARD_E5577BS_937)
#define CHG_SUB_LOW_TEMP_CURR_WALL             (448)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (448)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (448)
#define CHG_SUB_LOW_TEMP_CURR_WALL_H             (896)
#define CHG_SUB_LOW_TEMP_CURR_USB2_H             (576)
#define CHG_SUB_LOW_TEMP_CURR_USB3_H             (896)
#elif defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
   || defined(BSP_CONFIG_BOARD_506HW_2)
#define CHG_SUB_LOW_TEMP_CURR_WALL             (448)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (448)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (448)
#elif defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#define CHG_SUB_LOW_TEMP_CURR_WALL             (2048)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (576)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (1024)
#else
#define CHG_SUB_LOW_TEMP_CURR_WALL             (576)
#define CHG_SUB_LOW_TEMP_CURR_USB2             (576)
#define CHG_SUB_LOW_TEMP_CURR_USB3             (576)
#endif

#endif
#ifdef BSP_CONFIG_BOARD_E5_DCM
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4200,  /*mV, CV Voltage setting.*/
        256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/

    },

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
        500,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

/*标充高温停充限流参数*/
static const chg_hw_param_t chg_std_chgr_over_temp_hw_paras =  /*lint !e10*/
{
        100,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if (MBB_CHG_WARM_CHARGE ==FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

/*usb高温停充限流参数*/
static const chg_hw_param_t chg_usb_chgr_over_temp_hw_paras =  /*lint !e10*/
{
         100, /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
};
static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5) || defined(BSP_CONFIG_BOARD_E5575S_210) \
    || defined(BSP_CONFIG_BOARD_E5573S_156) || defined(BSP_CONFIG_BOARD_E5573S_856) \
    || defined(BSP_CONFIG_BOARD_E5573S_853) || defined(BSP_CONFIG_BOARD_E5573S_607) \
    || defined(BSP_CONFIG_BOARD_E5577CS_603) || defined(BSP_CONFIG_BOARD_E5575S_320)
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

#elif defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
        || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_R218h)
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5577S_603)
/* 3000mA电池参数 */
static chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577BS_937)
//1500mA电池参数
static chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
//3000mA电池参数
static chg_hw_param_t chg_std_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb3_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5577S_324)
//1500mA电池参数
static chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4304,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
//3000mA电池参数
static chg_hw_param_t chg_std_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb3_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4304,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5577S_932)
//default电池参数
static chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
//3000mA电池参数
static const chg_hw_param_t chg_std_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       2000,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        896,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras_h[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5_E5578)
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) 
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #if defined(MBB_CHG_WARM_CHARGE)
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #if defined(MBB_CHG_WARM_CHARGE)
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #if defined(MBB_CHG_WARM_CHARGE)
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_506HW_2) 
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4352,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #if defined(MBB_CHG_WARM_CHARGE)
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #if defined(MBB_CHG_WARM_CHARGE)
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #if defined(MBB_CHG_WARM_CHARGE)
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4112,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_607HW)
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4350,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
       FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        900,  /*mA, Power Supply front-end Current limit.*/
        512,  /*mA, Charge Current limit.*/
       4100,  /*mV, CV Voltage setting.*/
        128,  /*mA, Taper(Terminate) current.*/
       TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4350,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5_E5770s)
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         704,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4352,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4144,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#elif defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
        1500,  /*mA, Power Supply front-end Current limit.*/
        1536,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
         4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
         4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
         4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
         4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if defined(MBB_CHG_WARM_CHARGE)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
         4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#else
static const chg_hw_param_t chg_std_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
        2000,  /*mA, Power Supply front-end Current limit.*/
        2048,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
       1200,  /*mA, Power Supply front-end Current limit.*/
        576,  /*mA, Charge Current limit.*/
       4200,  /*mV, CV Voltage setting.*/
        256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/

    },

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
        1200,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

/*usb2.0限流参数*/
static const chg_hw_param_t chg_usb_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         704,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};

/*usb3.0限流参数*/
static const chg_hw_param_t chg_usb3_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         1024,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
         TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         512,  /*mA, Charge Current limit.*/
        4208,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         128,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
#endif


#if (MBB_CHG_WIRELESS == FEATURE_ON)
/*无线充电器在位标志，用于无线充电和对外充电互斥切换*/
boolean g_wireless_online_flag = OFFLINE;
/*通过BQ24196来控制无线充电是否充电而非控制无线充电芯片的管脚*/
static const chg_hw_param_t chg_wireless_chgr_hw_paras[CHG_STM_MAX_ST] =
{
    /*0:-Transition State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*1:-Fast Charge State*/
    {
         900,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*2:-Maintenance State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
        1024,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    /*3:-Invalid Charge Temperature State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    /*4:-Battery Only State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4200,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        FALSE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    /*5:-Warm charge State*/
    {
         500,  /*mA, Power Supply front-end Current limit.*/
         576,  /*mA, Charge Current limit.*/
        4100,  /*mV, CV Voltage setting.*/
         256,  /*mA, Taper(Terminate) current.*/
        TRUE   /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
    },
    #endif /* MBB_CHG_WARM_CHARGE */
};
/*关机无线充电的限流参数*/
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
static const chg_hw_param_t chg_wireless_poweroff_warmchg_paras =
{
     900,  /*mA, Power Supply front-end Current limit.*/
    1024,  /*mA, Charge Current limit.*/
    4100,  /*mV, CV Voltage setting.*/
     256,  /*mA, Taper(Terminate) current.*/
    TRUE  /*If charge enabled: FALSE:-Disable, TRUE:-Enable.*/
};
#endif /* MBB_CHG_WARM_CHARGE */
#endif /* MBB_CHG_WIRELESS */

/*0:-PwrOff Charging; 1:-Normal Charging.*/
/*0:-USB/NoStd Chgr;  1:-Wall/Standard Chgr.*/
static const uint32_t chg_fastchg_timeout_value_in_sec[2][2] =
{
    /*Power-off charge.*/
    {
        CHG_POWEROFF_FAST_USB_TIMER_VALUE,           /*USB/NoStd Chgr*/
        CHG_POWEROFF_FAST_CHG_TIMER_VALUE,           /*Wall/Standard Chgr*/
    },
    /*Normal charge.*/
    {
        CHG_FAST_USB_TIMER_VALUE,                    /*USB/NoStd Chgr*/
        CHG_FAST_CHG_TIMER_VALUE,                    /*Wall/Standard Chgr*/
    },
};

#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
typedef enum
{
    RE_BATT_ONLY,
    RE_POWER_ON_CHARGE,
    RE_POWER_ON_EXTCHG, 
    RE_POWER_DOWN_CHARGE, 
    RE_POWER_DOWN_EXTCHG,
}BATT_REVISE_STAT;

static const uint32_t chg_batt_revise_table[5][3] =
{
    {2, 2, 2},//单电池在位
    {2, 5, 6},//开机对内充电+内外并发充电
    {2,-2,-2},//开机单对外充电
    {2, 4, 5},//关机对内充电+内外并发充电
    {2,-2,-2},//关机单对外充电
};
#endif

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
static const char* chg_stm_name_table[CHG_STM_MAX_ST] =
{
    "Transition",                 //过渡状态
    "Fast Charge",                //快充状态
    "Maintenance",                //满电状态
    "Invalid Charge Temperature", //过温状态
    "Battery Only",               //电池单独在位状态
    "Warm charge"                 //高温充电状态
};

static const char* chg_chgr_type_name_table[CHG_CHGR_INVALID + 1] =
{
    "Unknown",                      //充电器类型未知
    "Wall Charger",                 //标准充电器
    "USB HOST PC",                  //USB
    "Non-Standard Chgr",            //第三方充电器
    "wireless chgr"                 //无线充电器
    "extchg chgr"                   //对外充电器
    "weak_chgr"                     //弱充
    "Invalid",                      //充电器不可用
};

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*Lookup charge state machine name from chg_stm_name_table.*/
/*BE CAREFUL: Pls DO make sure "i" MUSTN'T exceed (CHG_STM_INIT_ST, CHG_STM_MAX_ST) range.*/
#define TO_STM_NAME(i)   ((i >= CHG_STM_TRANSIT_ST && i < CHG_STM_MAX_ST) \
                         ? chg_stm_name_table[i] : "NULL")

/*Lookup charger type name from chg_chgr_type_name_table*/
/*BE CAREFUL: Pls DO make sure "i" MUSTN'T exceed [CHG_CHGR_UNKNOWN, CHG_CHGR_INVALID] range.*/
#define TO_CHGR_NAME(i)  ((i <= CHG_CHGR_INVALID) \
                         ? chg_chgr_type_name_table[i] : "NULL")

/*电池温度平滑的长度个数:快轮询长度为30，满轮询长度为5*/
#define  CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST   (30)
#define  CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW   (5)

/*电池电压平滑的长度个数:快轮询长度为30，满轮询长度为5*/
#define  CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST    (30)
#define  CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW    (5)


/*过温关机底层等待时间，若APP在该时间段内仍未关机，则底层执行关机*/
#define  OVER_TEMP_SHUTOFF_DURATION         (15000)
/*低电关机底层等待时间，若APP在该时间段内仍未关机，则底层执行关机*/
#define  LOW_BATT_SHUTOFF_DURATION          (15000)

/*开机后异常电池检测*/
#define CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN      (-30)
/* 开机后坏电池检测的电压门限***/
#define CHG_SHORT_CIRC_BATTERY_THRES        (2700)

/*高温关机的温度告警回滞带，在达到关机门限前的该范围，提示高温*/
#define SHUTOFF_HIGH_TEMP_WARN_LEN          (2)
/*高温提示的恢复回滞带，在温度恢复到关机门限的该范围以下，撤销高温告警提示*/
#define SHUTOFF_HIGH_TEMP_RESUME_LEN        (5)

#define LIMIT_SUPPLY_CURR_TEMP              (58)
#define LIMIT_SUPPLY_CURR__RESUME_LEN       (1)

/*电池过温后给应用上报过温事件的次数*/
#define BATTERY_EVENT_REPORT_TIMES          (5)

#define CHG_BATT_REVISE    1 
#define CHG_USE_COUL_REVISE    0
static int32_t g_chg_revise_count = 0;

/*USB温保门限值,硬件测试完成后提供*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
#define CHG_USB_TEMP_LIMIT      (90)     /*USB温保机制触发门限*/
#define CHG_USB_TEMP_RESUME     (70)    /*USB温度机制恢复门限*/
#endif

/*开始充电前的电池电压，用于充电时的电压补偿防反转*/
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
//#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (4280)    //充电电池电压补偿上限
#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (is_batttemp_in_warm_chg_area()?4080:4280)  //充电电池电压补偿上限
#else
//#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (4150)    //充电电池电压补偿上限
#define CHG_BATT_VOLT_REVISE_LIMIT_UP       (is_batttemp_in_warm_chg_area()?4080:4150)     //充电电池电压补偿上限
#endif
#define CHG_BATT_VOLT_REVISE_LIMIT_DOWN     (3350)    //放电电池电压补偿上限
#define CHG_BATT_VOLT_REVISE_WINDOW         (35)    //电压虚高变化太小则不补补偿
#define CHG_BATT_VOLT_REVISE_LIMIT          (200)    //虚高补偿限制值
#define CHG_BATT_VOLT_REVISE_COUNT          (15)    //首次进行电池电压虚高虚低值计算的次数，30个快轮询共1min

/* 需要上报事件给应用时延时5秒休眠，保证应用能够收到事件*/
#define ALARM_REPORT_WAKELOCK_TIMEOUT (5000)
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
#define SHUTDOWN_WAKELOCK_TIMEOUT (1000 * 120)
#endif

#if defined(BSP_CONFIG_BOARD_E5_E5770s)
#define CHG_TERMI_VOLT_FOR_COUL             (4192)
#define CHG_TERMI_CURR_FOR_COUL             (120)
#define CHG_BAT_TEMP_REVISE_SAMPLE_FAST     (6)
#define CHG_BAT_TEMP_REVISE_SAMPLE_SLOW     (3)
#define CHG_BAT_TEMP_REVISE_TEM_OF_NOREVISE (5)
#define LG_TBATT_SUPPLY_VOLT                (3580)
#define LG_TBATT_DISCHG_VOLT                (3950)
#define LG_TBATT_SUPPLY_STOP_VOLT           (3625)
#define LG_TBATT_DISCHG_STOP_VOLT           (3915)
#define SY_TBATT_SUPPLY_VOLT                (3750)
#define SY_TBATT_DISCHG_VOLT                (3986)
#define SY_TBATT_SUPPLY_STOP_VOLT           (3785)
#define SY_TBATT_DISCHG_STOP_VOLT           (3956)
#define EXTCHG_STOP_THRESHOLD_POWON         (10)
#define EXTCHG_STOP_THRESHOLD_POWDOWN       (2)
#define EXTCHG_RECHG_THRESHOLD              (20)
#endif

#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#define CHG_TERMI_VOLT_FOR_COUL             (4165)
#define CHG_TERMI_WARMCHG_VOLT_FOR_COUL     (4065)
#define CHG_TERMI_CURR_FOR_COUL             (256)
#define CHG_BAT_TEMP_REVISE_SAMPLE_FAST     (6)
#define CHG_BAT_TEMP_REVISE_SAMPLE_SLOW     (3)
#define CHG_BAT_TEMP_REVISE_TEM_OF_NOREVISE (5)
#define LG_TBATT_SUPPLY_VOLT                (3580)
#define LG_TBATT_DISCHG_VOLT                (3950)
#define LG_TBATT_SUPPLY_STOP_VOLT           (3625)
#define LG_TBATT_DISCHG_STOP_VOLT           (3915)
#define SY_TBATT_SUPPLY_VOLT                (3460)
#define SY_TBATT_DISCHG_VOLT                (3900)
#define SY_TBATT_SUPPLY_STOP_VOLT           (3540)
#define SY_TBATT_DISCHG_STOP_VOLT           (3860)
#define EXTCHG_STOP_THRESHOLD_POWON         (10)
#define EXTCHG_STOP_THRESHOLD_POWDOWN       (2)
#define EXTCHG_RECHG_THRESHOLD              (30)
#endif

#if (MBB_CHG_EXTCHG == FEATURE_ON)
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
/* 外充限流的高温门限 */
#define EXTCHG_OVER_TEMP_STOP_THRESHOLD     (61)
#define EXTCHG_OVER_TEMP_RESUME_THRESHOLD   (58)
#define EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD  (55)
#define EXTCHG_WARM_CHARGE_RESUME_THRESHOLD (52)
#define EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD   (-20)
#define EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD (-17)
#define EXTCHG_LOW_TEMP_STOP_THRESHOLD      (0)
#define EXTCHG_LOW_TEMP_RESUME_THRESHOLD    (3)
#define EXTCHG_SHORT_VOLT_THRESHOLD         (2000)
#define TIME_OF_SHORT_DET                   (10)
#define NUM_OF_SHORT_DET_POLL               (3)
#else
#define EXTCHG_EXTREME_HIGH_TEMP            (77)
#define EXTCHG_OVER_TEMP_STOP_THRESHOLD     (61)
#define EXTCHG_OVER_TEMP_RESUME_THRESHOLD   (60)
#define EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD  (55)
#define EXTCHG_WARM_CHARGE_RESUME_THRESHOLD (45)
#define EXTCHG_LOW_VOLTAGE_LIMIT_THRESHOLD  (3600)
#define EXTCHG_LOW_VOLTAGE_STOP_THRESHOLD   (3450)
#define EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD   (-20)
#define EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD (-17)
#define EXTCHG_LOW_TEMP_STOP_THRESHOLD      (0)
#define EXTCHG_LOW_TEMP_RESUME_THRESHOLD    (3)
#define EXTCHG_LOW_VOLTAGE_RESUME_THRESHOLD (3700)
#define EXTCHG_SHORT_VOLT_THRESHOLD         (2000)
#define TIME_OF_SHORT_DET                   (10)
#define NUM_OF_SHORT_DET_POLL               (3)
#define EXTCHG_STOP_CHG_CAPACITY            (18)
#endif
#else
#define EXTCHG_LOW_TEMP_STOP_THRESHOLD      (0)
#define EXTCHG_LOW_TEMP_RESUME_THRESHOLD    (3)
#define EXTCHG_OVER_TEMP_STOP_THRESHOLD     (55)
#define EXTCHG_OVER_TEMP_RESUME_THRESHOLD   (51)
#define EXTCHG_LOW_VOLTAGE_RESUME_THRESHOLD (3550)
#endif
static boolean is_otg_extchg    = FALSE;
static boolean is_extchg_ovtemp = FALSE;


static int32_t g_extchg_revise_count = 0;
#define EXTCHG_BATT_VOLT_REVISE_LIMIT       (200)
#define EXTCHG_THRESHOLD_PATH   "/data/userdata/extchg_config/extchg_threshold"
#define EXTCHG_DISABLE_PATH     "/data/userdata/extchg_config/extchg_disable_st"
#define EXTCHG_DEFAULT_STOP_THRESHOLD       (5)
#define EXTCHG_STOP_CAPACITY_TEN            (10)

#define EXTCHG_STOP_CAPACITY_FIFTHTEEN      (15)
#define EXTCHG_STOP_CAPACITY_EIGHTEEN       (18)

#define EXTCHG_STOP_CAPACITY_TWENTY         (20)
#define EXTCHG_STOP_CAPACITY_THIRTY         (30)

#define EXTCHG_STOP_CAPACITY_THIRTYSIX      (36)
#define EXTCHG_STOP_CAPACITY_TWENTYFIVE      (25)

#define EXTCHG_STOP_CAPACITY_FORTY          (40)
#define EXTCHG_STOP_CAPACITY_FIFTY          (50)
#define EXTCHG_STOP_CAPACITY_SIXTY          (60)
#define EXTCHG_STOP_CAPACITY_SEVENTY        (70)
#define EXTCHG_STOP_CAPACITY_EIGHTY         (80)
#define EXTCHG_STOP_CAPACITY_NINETY         (90)
#define EXTCHG_STOP_CAPACITY_HUNDRED        (100)

#if defined(BSP_CONFIG_BOARD_E5577S_932)
#define EXTCHG_DEFAULT_STOP_VOLTAGE         (3626)
#define EXTCHG_STOP_VOLTAGE_TEN             (3667)
#define EXTCHG_STOP_VOLTAGE_TWENTY          (3724)
#define EXTCHG_STOP_VOLTAGE_THIRTY          (3757)
#define EXTCHG_STOP_VOLTAGE_FORTY           (3791)
#define EXTCHG_STOP_VOLTAGE_FIFTY           (3838)
#define EXTCHG_STOP_VOLTAGE_SIXTY           (3904)
#define EXTCHG_STOP_VOLTAGE_SEVENTY         (3987)
#define EXTCHG_STOP_VOLTAGE_EIGHTY          (4087)
#define EXTCHG_STOP_VOLTAGE_NINETY          (4201)
#define EXTCHG_STOP_VOLTAGE_HUNDRED         (4353)
#elif defined(BSP_CONFIG_BOARD_E5577S_321)
#define EXTCHG_DEFAULT_STOP_VOLTAGE         (3630)
#define EXTCHG_STOP_VOLTAGE_TEN             (3669)
#define EXTCHG_STOP_VOLTAGE_TWENTY          (3722)
#define EXTCHG_STOP_VOLTAGE_THIRTY          (3752)
#define EXTCHG_STOP_VOLTAGE_FORTY           (3788)
#define EXTCHG_STOP_VOLTAGE_FIFTY           (3836)
#define EXTCHG_STOP_VOLTAGE_SIXTY           (3900)
#define EXTCHG_STOP_VOLTAGE_SEVENTY         (3981)
#define EXTCHG_STOP_VOLTAGE_EIGHTY          (4081)
#define EXTCHG_STOP_VOLTAGE_NINETY          (4191)
#define EXTCHG_STOP_VOLTAGE_HUNDRED         (4347)
#elif defined(BSP_CONFIG_BOARD_E5577S_324)||defined(BSP_CONFIG_BOARD_E5577BS_937)
#define EXTCHG_DEFAULT_STOP_VOLTAGE         (3630)
#define EXTCHG_STOP_VOLTAGE_TEN             (3669)
#define EXTCHG_STOP_VOLTAGE_TWENTY          (3722)
#define EXTCHG_STOP_VOLTAGE_THIRTY          (3752)
#define EXTCHG_STOP_VOLTAGE_FORTY           (3788)
#define EXTCHG_STOP_VOLTAGE_FIFTY           (3836)
#define EXTCHG_STOP_VOLTAGE_SIXTY           (3900)
#define EXTCHG_STOP_VOLTAGE_SEVENTY         (3981)
#define EXTCHG_STOP_VOLTAGE_EIGHTY          (4081)
#define EXTCHG_STOP_VOLTAGE_NINETY          (4191)
#define EXTCHG_STOP_VOLTAGE_HUNDRED         (4347)
#else
#define EXTCHG_DEFAULT_STOP_VOLTAGE         (3470)
#define EXTCHG_STOP_VOLTAGE_TEN             (3579)
#define EXTCHG_STOP_VOLTAGE_TWENTY          (3637)
#define EXTCHG_STOP_VOLTAGE_THIRTY          (3670)
#define EXTCHG_STOP_VOLTAGE_FORTY           (3696)
#define EXTCHG_STOP_VOLTAGE_FIFTY           (3728)
#define EXTCHG_STOP_VOLTAGE_SIXTY           (3769)
#define EXTCHG_STOP_VOLTAGE_SEVENTY         (3830)
#define EXTCHG_STOP_VOLTAGE_EIGHTY          (3902)
#define EXTCHG_STOP_VOLTAGE_NINETY          (3985)
#define EXTCHG_STOP_VOLTAGE_HUNDRED         (4130)
#endif

/*停止对外充电门电池电量值默认为低电5%->3.55V*/
int32_t g_extchg_voltage_threshold = 0;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
int32_t g_extchg_stop_soc_threshold = 0;
#endif
/*是否永久停止对外充电标志1:永久停止 0:永久开启*/
int32_t g_extchg_diable_st = 0;
/*记录用户上次设置的永久停止对外充电标志1：永久停止，0：永久开启*/
static int32_t g_last_extchg_diable_st = 0;
#define EXTCHG_TEMP_COMPENSATE_VALUE        (4)
/*用户通过TOUCH UI选择的对外充电的模式，1:仅对外充电 2:对外充电加数据业务*/
int32_t g_ui_choose_exchg_mode = 0;
boolean g_exchg_enable_flag = FALSE;
/*对外充电USB ID线在位标志，用于对外充电和无线充电的互斥切换*/
boolean g_exchg_online_flag = OFFLINE;
#endif /*MBB_CHG_EXTCHG */

#if ((MBB_CHG_EXTCHG == FEATURE_ON) || (MBB_CHG_WIRELESS == FEATURE_ON))
/*下边函数不同平台实现方式不同，需要进行适配*/

extern void usb_notify_event(unsigned long val, void *v);
#endif/*defined(MBB_CHG_EXTCHG) || defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
extern void mlog_set_statis_info(char *item_name,unsigned int item_value);
#endif
/**********************************************************************
函 数 名  : void load_factory_mode_flag_init(void)
功能描述  : 读取工厂模式标志NV参数
输入参数  : none
输出参数  : 无。
返 回 值  : 无。
注意事项  : 无。
***********************************************************************/
void load_ftm_mode_init(void)
{
    if(CHG_OK == chg_config_para_read(NV_FACTORY_MODE_I, (char *) &g_real_factory_mode, sizeof(uint32_t)))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:read factory mode ok,mode =  %d\n ", g_real_factory_mode);

        if(0 == g_real_factory_mode)
        {
            chg_current_ftm_mode = TRUE;
        }
        else
        {
            chg_current_ftm_mode = FALSE;
        }
    }
    else/*若NV读失败，则默认为升级版本*/
    {
#ifndef MBB_FACTORY_FEATURE
        chg_current_ftm_mode = FALSE;
#else
        chg_current_ftm_mode = TRUE;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM: In ftm mode now \n ");
#endif
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:read factory mode nv fail \n ");
    }
}

/*******************************************************************
Function:      chg_batt_temp_init
Description:   电池温度门限值初始化函数，读取NV项，将读取的值存到全局变量中
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_batt_temp_init(void)
{
    if(CHG_OK != chg_config_para_read(NV_OVER_TEMP_SHUTOFF_PROTECT,&g_chgBattOverTempPeotect, \
    sizeof(CHG_BATTERY_OVER_TEMP_PROTECT_NV)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read fail \n ");
        g_chgBattOverTempPeotect.ulIsEnable =  1;
        g_chgBattOverTempPeotect.lCloseAdcThreshold = 61; /*power off temp +1 */
        g_chgBattOverTempPeotect.ulTempOverCount = 1;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read success \n ");
    }

    if(CHG_OK != chg_config_para_read(NV_LOW_TEMP_SHUTOFF_PROTECT,&g_chgBattLowTempPeotect,\
    sizeof(CHG_BATTERY_LOW_TEMP_PROTECT_NV)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_LOW_TEMP_SHUTOFF_PROTECT read fail \n ");
        g_chgBattLowTempPeotect.ulIsEnable =  1;
        g_chgBattLowTempPeotect.lCloseAdcThreshold = -20; /* */
        g_chgBattLowTempPeotect.ulTempLowCount = 1;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_LOW_TEMP_SHUTOFF_PROTECT read success \n ");
    }

    if(CHG_OK != chg_config_para_read(NV_BATT_TEMP_PROTECT_I,&g_chgShutOffTempProtect,\
    sizeof(CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_BATT_TEMP_PROTECT_I read fail \n ");
        #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
        g_chgShutOffTempProtect.ulChargeIsEnable = 1;           //充电温保护使能
        g_chgShutOffTempProtect.overTempchgStopThreshold = 55;  //充电高温保护门限
        g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold = 45; //高温充电入口电池温度门限
        g_chgShutOffTempProtect.lowTempChgStopThreshold = 0;    //充电低温保护门限
        g_chgShutOffTempProtect.overTempChgResumeThreshold = 52; //充电高温恢复温度门限
        g_chgShutOffTempProtect.lowTempChgResumeThreshold = 3;   //充电低温恢复温度门限
        g_chgShutOffTempProtect.chgTempProtectCheckTimes = 1;    //充电停充轮询次数
        g_chgShutOffTempProtect.chgTempResumeCheckTimes = 1;     //充电复充轮询次数
        g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold = 42; //高温充电恢复到常温充电温度门限
        g_chgShutOffTempProtect.reserved2 = 0;                   //预留
        #else
        g_chgShutOffTempProtect.ulChargeIsEnable = 1;           //充电温保护使能
        g_chgShutOffTempProtect.overTempchgStopThreshold = 45;  //充电高温保护门限
        g_chgShutOffTempProtect.subTempChgLimitCurrentThreshold = 38; //2A充电器3000mA电池的限流充电门限
        g_chgShutOffTempProtect.lowTempChgStopThreshold = 0;    //充电低温保护门限
        g_chgShutOffTempProtect.overTempChgResumeThreshold = 42;//充电高温恢复温度门限
        g_chgShutOffTempProtect.lowTempChgResumeThreshold = 3;  //充电低温恢复温度门限
        g_chgShutOffTempProtect.chgTempProtectCheckTimes = 1;   //充电停充轮询次数
        g_chgShutOffTempProtect.chgTempResumeCheckTimes = 1;    //充电复充轮询次数
        g_chgShutOffTempProtect.exitWarmChgToNormalChgThreshold = 0; //高温充电恢复到常温充电温度门限
        g_chgShutOffTempProtect.reserved2 = 0;                  //预留
        #endif /* MBB_CHG_WARM_CHARGE */
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_BATT_TEMP_PROTECT_I read success \n ");
    }
}

/**********************************************************************
函 数 名  : void load_on_off_mode_parameter(void)
功能描述  :  读取硬件测试开机模式NV
输入参数  :none
输出参数  : 无。
返 回 值      : 无。
注意事项  : 无。
***********************************************************************/
void load_on_off_mode_parameter(void)
{
    uint32_t ret_val = CHG_ERROR;
    memset( (void *)&g_real_powerup_mode_value, 0, sizeof(POWERUP_MODE_TYPE) );

    /* 从nv中读取硬件测试开机模式标志*/
    /* 如果读取失败不设置，设置默认值*/
    ret_val = chg_config_para_read(NV_POWERUP_MODE,&g_real_powerup_mode_value, \
        sizeof(POWERUP_MODE_TYPE) );
    if ( CHG_OK == ret_val )
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:\r\nno_battery_powerup_enable=%d,\r\nexception_poweroff_poweron_enable=%d,\
            \r\nlow_battery_poweroff_disable=%d,\r\n ",\
            g_real_powerup_mode_value.no_battery_powerup_enable,
            g_real_powerup_mode_value.exception_poweroff_poweron_enable,
            g_real_powerup_mode_value.low_battery_poweroff_disable);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:read hw test powerup mode nv error, ret_val = %d \n ", ret_val, 0, 0);

        g_real_powerup_mode_value.no_battery_powerup_enable = FALSE;
        g_real_powerup_mode_value.exception_poweroff_poweron_enable = FALSE;
        g_real_powerup_mode_value.low_battery_poweroff_disable = FALSE;
    }

}

/*******************************************************************
Function:      chg_batt_volt_init
Description:   电池电压门限值初始化函数，读取NV项，将读取的值存到全局变量中
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_batt_volt_init(void)
{
#if defined(BSP_CONFIG_BOARD_E5) || defined(BSP_CONFIG_BOARD_E5_E5578) \
    || defined(BSP_CONFIG_BOARD_E5575S_210) || defined(BSP_CONFIG_BOARD_E5577S_321) \
	|| defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_E5575S_320) \
    || defined(BSP_CONFIG_BOARD_E5573S_156) || defined(BSP_CONFIG_BOARD_E5577S_932) \
    || defined(BSP_CONFIG_BOARD_E5573S_856) || defined(BSP_CONFIG_BOARD_E5573S_853) \
    || defined(BSP_CONFIG_BOARD_E5573S_607) || defined(BSP_CONFIG_BOARD_E5577CS_603) \
    || defined(BSP_CONFIG_BOARD_E5577S_603) || defined(BSP_CONFIG_BOARD_E5771S_852) \
    || defined(BSP_CONFIG_BOARD_E5771H_937) || defined(BSP_CONFIG_BOARD_E5573CS) \
    || defined(BSP_CONFIG_BOARD_506HW) || defined(BSP_CONFIG_BOARD_506HW_2) \
    || defined(BSP_CONFIG_BOARD_E5573BS) || defined(BSP_CONFIG_BOARD_E5573CS_933)\
    || defined(BSP_CONFIG_BOARD_E5577S_324) || defined(BSP_CONFIG_BOARD_607HW)\
    || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
    /*get volt para by battery id*/
    if(chg_batt_volt_paras_init())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt data init fail!\n");
    }
#else
/*  当前E5771 的电池ID未定 ，获取到电池ID后看是否要用电池ID区分不同厂家电池门限  */
    if(CHG_OK != chg_config_para_read(NV_BATT_VOLT_PROTECT_I,&g_chgBattVoltProtect,\
        sizeof(CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE)))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read fail \n ");
#if defined(BSP_CONFIG_BOARD_E5_E5770s)
        g_chgBattVoltProtect.battVoltPowerOnThreshold = 3300;     //开机电压门限
        g_chgBattVoltProtect.battVoltPowerOffThreshold = 3300;    //关机电压门限
#else
        g_chgBattVoltProtect.battVoltPowerOnThreshold = 3450;     //开机电压门限
        g_chgBattVoltProtect.battVoltPowerOffThreshold = 3450;    //关机电压门限
#endif
        g_chgBattVoltProtect.battOverVoltProtectThreshold = 4370; //平滑充电过压保护门限(平滑值)
        g_chgBattVoltProtect.battOverVoltProtectOneThreshold = 4390; //单次充电过压保护门限(单次值)
        g_chgBattVoltProtect.battChgTempMaintThreshold = 4144;    //区分高温停充和正常停充的判断门限
        g_chgBattVoltProtect.battChgRechargeThreshold = 4100;     //充电二次复充门限
        g_chgBattVoltProtect.VbatLevelLow_MAX = 3550;             //低电上限门限
        g_chgBattVoltProtect.VbatLevel0_MAX = 3550;               //0格电压上限门限
        g_chgBattVoltProtect.VbatLevel1_MAX = 3610;               //1格电压上限门限
        g_chgBattVoltProtect.VbatLevel2_MAX = 3670;               //2格电压上限门限
        g_chgBattVoltProtect.VbatLevel3_MAX = 3770;               //3格电压上限门限
        g_chgBattVoltProtect.battChgFirstMaintThreshold = 4280;
        g_chgBattVoltProtect.battNormalTempChgRechargeThreshold = 4200;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NV_OVER_TEMP_SHUTOFF_PROTECT read success \n ");
    }
#endif
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    g_batt_normal_temp_recherge_threshold = g_chgBattVoltProtect.battNormalTempChgRechargeThreshold;
    /* 开了膨胀保护宏，统一将产品高温复充门限改为4V  */
    g_chgBattVoltProtect.battChgRechargeThreshold = BATT_EXP_HIGH_TEMP_RECHARGE_THR;
#endif
    
}

/**********************************************************************
函 数 名  : boolean chg_is_no_battery_powerup_enable(void)
功能描述  : 判断目前是否为非工厂模式下的电池不在位开机使能模式
输入参数  : 无。
输出参数  : 无。
返 回 值  : 目前是否为非工厂模式电池不在位开机使能
                           TRUE : 使能
                           FALSE: 非使能
注意事项  : 无。
***********************************************************************/
boolean chg_is_no_battery_powerup_enable(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_no_battery_powerup_enable %d !\n",
                            g_real_powerup_mode_value.no_battery_powerup_enable);
    return g_real_powerup_mode_value.no_battery_powerup_enable;
}

/**********************************************************************
函 数 名  : boolean chg_is_low_battery_poweroff_disable(void)
功能描述  : 判断目前是否为低电关机禁止模式
输入参数  : 无。
输出参数  : 无。
返 回 值  : 是否为低电关机禁止模式
           TRUE : 低电关机不使能
           FALSE: 低电关机使能
注意事项  : 无。
***********************************************************************/
boolean chg_is_low_battery_poweroff_disable(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_low_battery_poweroff_disable %d !\n",
                            g_real_powerup_mode_value.low_battery_poweroff_disable);
    return g_real_powerup_mode_value.low_battery_poweroff_disable;
}

/**********************************************************************
函 数 名  : boolean chg_is_exception_poweroff_poweron_mode(void)
功能描述  :  判断目前是否为异常关机后，下次插入电源后进入开机模式使能开机使能模式
输入参数  : 无。
输出参数  : 无。
返 回 值  : 是否为异常关机后，下次插入电源后进入开机模式使能
           TRUE : 是
           FALSE: 否
注意事项  : 无。
***********************************************************************/
boolean chg_is_exception_poweroff_poweron_mode(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_exception_poweroff_poweron_mode %d !\n",
                            g_real_powerup_mode_value.exception_poweroff_poweron_enable);
    return g_real_powerup_mode_value.exception_poweroff_poweron_enable;
}

/**********************************************************************
函 数 名  :chg_get_batt_level
功能描述  : 获取当前电池电量格数的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量格数
注意事项  : 无。
***********************************************************************/
BATT_LEVEL_ENUM chg_get_batt_level(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_get_batt_level=%d !\n",chg_real_info.bat_volt_lvl);

    return chg_real_info.bat_volt_lvl;
}

/**********************************************************************
函 数 名  : chg_is_ftm_mode
功能描述  : 判断目前是否为ftm模式
输入参数  : 无。
输出参数  : 无。
返 回 值  : 目前是否为工厂模式
           TRUE : 工厂模式
           FALSE: 非工厂模式
注意事项  : 无。
***********************************************************************/
boolean chg_is_ftm_mode(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_ftm_mode=%d !\n",chg_current_ftm_mode);
    return chg_current_ftm_mode;
}


void chg_batt_error_handle(void)
{
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_batt_error_handle->chg_set_power_off\n");
    chg_set_power_off(DRV_SHUTDOWN_BATTERY_ERROR);
    return;
}

/**********************************************************************
函 数 名  : chg_get_batt_id_valid
功能描述  : 检测当前的电池是否在位，判断电池自身的ntc温度
输入参数  : 无。
输出参数  : 无。
返 回 值  : TRUE: 电池在位
            FALSE: 电池不在位
注意事项  : 无。
***********************************************************************/
boolean chg_get_batt_id_valid(void)
{
    int32_t batt_temp = 0;
    boolean chg_ftm_mode = FALSE;
    boolean chg_no_battery_powerup_mode = FALSE;

    /* 工厂模式下一直认为电池为合法电池**/
    chg_ftm_mode = chg_is_ftm_mode();
    chg_no_battery_powerup_mode = chg_is_no_battery_powerup_enable();

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_ftm_mode=%d ,no_battery_powerup_mode=%d!\n",
                            chg_ftm_mode,chg_no_battery_powerup_mode);

    if( (TRUE == chg_ftm_mode) || (TRUE == chg_no_battery_powerup_mode) )
    {
        return TRUE;
    }

    /* 使用电池温度判断目前电池是否为拔除**/
    batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN >= batt_temp)
    {
        batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
        if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN >=  batt_temp)
        {
            batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
            if (CHG_AUTO_CUR_CTRL_DIE_TEMP_MIN >= batt_temp)
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_temp below 30 degree bat not present!\n");
                return FALSE;
            }
        }
    }

    return TRUE;

}

/**********************************************************************
函 数 名  : chg_is_emergency_state
功能描述  : 查询当前的电池是否处于异常状态，异常包括电池高温(>=58度)，电池低电
输入参数  : 无。
输出参数  : 无。
返 回 值  : TRUE: 电池高温或者电池低电，进入快轮询
            FALSE: 电池条件正常
注意事项  : 无。
***********************************************************************/
boolean chg_is_emergency_state(void)
{
    if((chg_batt_high_temp_58_flag == TRUE) || (chg_batt_low_battery_flag == TRUE))
    {
        chg_batt_condition_error_flag = TRUE;
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:is_emergency_state!\n");
    }
    else
    {
        chg_batt_condition_error_flag = FALSE;
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:is not emergency state!\n");
    }

    return chg_batt_condition_error_flag;
}
/*****************************************************************************
 函 数 名  : chg_batt_error_detect_temp
 功能描述  : 电池不在位关机处理函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
 *****************************************************************************/
void chg_batt_error_detect_temp(void)
{
    if(FALSE == chg_get_batt_id_valid())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:batt invalid system shutdown!\n");
        mlog_print(MLOG_CHG, mlog_lv_fatal, "battery ts error detected.\n");

        chg_batt_error_handle();
    }

    return;
}
/*****************************************************************************
 函 数 名  : chg_batt_error_detect_volt
 功能描述  : 坏电池关机处理
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
void chg_batt_error_detect_volt(void)
{
    int32_t  batt_volt = 0;
    boolean  batt_err_flag = FALSE;
    boolean  chg_ftm_mode = FALSE;
    boolean  chg_no_battery_powerup_mode = FALSE;

    /* 工厂模式下一直认为电池为合法电池**/
    chg_ftm_mode = chg_is_ftm_mode();
    chg_no_battery_powerup_mode = chg_is_no_battery_powerup_enable();

    if( (TRUE == chg_ftm_mode) || (TRUE == chg_no_battery_powerup_mode) )
    {
        return;
    }

    /* 查询电池电压,如果电池电压低于规定值,判定电池损坏*/
    batt_volt = chg_get_batt_volt_value();
    if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();

        if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
        {
            batt_volt = chg_get_batt_volt_value();
            if (CHG_SHORT_CIRC_BATTERY_THRES > batt_volt)
            {
                batt_err_flag = TRUE;
            }
        }
    }

    if(TRUE == batt_err_flag)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt shorted system shutdown!\n");
        mlog_print(MLOG_CHG, mlog_lv_fatal, "battery short error detected.\n");
        chg_batt_error_handle();
    }

    return;
}


/*******************************************************************
Function:      chg_calc_average_temp_value
Description:   电池温度平滑处理函数
Data Accessed: 无
Data Updated:  无
Input:         新采集的温度值
Return:        平滑后的温度值
*******************************************************************/
static int32_t chg_calc_average_temp_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_temp_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static int32_t record_avg_num_fast = 0;
    static int32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = {0};

    /*查询当前轮询模式*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*由慢轮询切换到快轮询*/
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_temp;
            }
            record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_temp;
            }
            record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:new_poll_mode==old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)/*快轮询模式*/
    {
        /*数组中当前元素标号小于30*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于30个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else/*慢轮询模式*/
    {
        /*数组中当前元素标号小于5*/
        if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于5个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_SMOOTH_SAMPLE_SLOW;
            }
        }
    }

    return bat_temp_avg;

}

/*******************************************************************
Function:      chg_huawei_set_temp
Description:   电池温度补偿处理函数，需要根据具体的测试结果设计补偿函数，
               此处只提供接口，具体实现取决于产品实际测试结果。
Input:         平滑后的温度值
Return:        补偿后的温度值
*******************************************************************/
int32_t chg_huawei_set_temp(int32_t temp)
{
#if defined(BSP_CONFIG_BOARD_E5) || defined(BSP_CONFIG_BOARD_E5575S_210) \
    || defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5573S_156) \
    || defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5573S_856) \
    || defined(BSP_CONFIG_BOARD_E5573S_853) || defined(BSP_CONFIG_BOARD_E5573S_607) \
    ||defined(BSP_CONFIG_BOARD_E5577CS_603) || defined(BSP_CONFIG_BOARD_E5577S_603) \
    || defined(BSP_CONFIG_BOARD_E5575S_320) || defined(BSP_CONFIG_BOARD_E5573CS) \
    || defined(BSP_CONFIG_BOARD_E5573BS) || defined(BSP_CONFIG_BOARD_E5573CS_933)\
    || defined(BSP_CONFIG_BOARD_E5577S_324) || defined(BSP_CONFIG_BOARD_607HW)\
    || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
    uint32_t batt_id = chg_get_batt_id();

    if(is_chg_charger_removed())/*充电器不在位*/
    {
#if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
    || defined(BSP_CONFIG_BOARD_E5577S_324)
        if(ONLINE == g_exchg_online_flag)
        {
            switch(batt_id)
            {
                case CHG_BATT_ID_FEIMAOTUI_3000:
                case CHG_BATT_ID_LISHEN_3000:
                case CHG_BATT_ID_XINGWANGDA_3000:
                {
                    if((35 <= temp) && (temp <= 45))/*[34,45]*/
                    {
                        temp = temp - 2;/*补偿-2度*/
                    }
                    else if((46 <= temp) && (temp <= 63))/*[46,63]*/
                    {
                        temp = temp - 3;/*补偿-3度*/
                    }
                    break;
                }
                default:
                    break;
            }
            return temp;
        }
#endif
        switch(batt_id)
        {
            case CHG_BATT_ID_FEIMAOTUI:
            {
                if((35 <= temp) && (temp <= 63))/*[35,63]*/
                {
                    temp = temp - 1;/*补偿一度*/
                }
                break;             
            }
            case CHG_BATT_ID_XINGWANGDA:
            case CHG_BATT_ID_LISHEN:
            default:
                break;  
        }              
    }
    else/*充电器在位*/
    {
        switch(batt_id)
        {
            case CHG_BATT_ID_FEIMAOTUI:
            {
#if defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573S_156) \
    || defined(BSP_CONFIG_BOARD_E5573BS) || defined(BSP_CONFIG_BOARD_E5573CS_933)
                /* 由于在[56,63]补偿+2度会导致单板c核温度59度时a核温度到达61度直接关机
                无掉线动作，所以修改飞毛腿补偿方案与欣旺达保持一致，实测满足采集精度 */
                if((35 <= temp) && (temp <= 57))/*[35,57]*/
                {
                    temp = temp + 2;/*补偿两度*/
                }
#elif defined(BSP_CONFIG_BOARD_R218h)
                if((35 <= temp) && (temp <= 54))/*[35,54]*/
                {
                    temp = temp + 1;/*补偿一度*/
                }
#else
                if((35 <= temp) && (temp <= 55))/*[35,55]*/
                {
                    temp = temp + 1;/*补偿一度*/
                }
                else if((56 <= temp) && (temp <= 63))/*[56,63]*/
                {
                    temp = temp + 2;/*补偿两度*/
                }
                else if(63 < temp)/*[63,)*/
                {
                    temp = temp + 1;/*补偿一度*/
                }
#endif
                break;
            }
            case CHG_BATT_ID_XINGWANGDA:
            {
                if((35 <= temp) && (temp <= 57))/*[35,57]*/
                {
                    temp = temp + 2;/*补偿两度*/
                }
                break;  
            }
            case CHG_BATT_ID_LISHEN:
            {
                if((35 <= temp) && (temp <= 63))/*[35,63]*/
                {
                    temp = temp + 2;/*补偿两度*/
                }
                break;            
            }
            default:
                break;  
        }        
    }

#endif
    return temp;

}

/*******************************************************************
Function:       chg_temp_is_too_hot_or_too_cold_for_chg
Description:    电池温度判断，当其温度超过充电温保护门限时，充电进入了温保护状态，停止充电；
                当温度处于或者恢复到正常温度门限范围时，恢复充电。
Calls:         无
Data Accessed: 无
Data Updated:  全局充电温保护标志
Input:         无
Output:       无
Return:       无
Others:        无
*******************************************************************/
void chg_temp_is_too_hot_or_too_cold_for_chg ( void )
{
    static uint32_t up_over_temp_flag = FALSE;
    static uint32_t low_over_temp_flag = FALSE;

    /*充电温保护使能关闭*/
    if(0 == CHG_TEMP_PROTECT_ENABLE)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is disable!\n");
        chg_temp_protect_flag = FALSE;
        return;
    }
    else
    {
         chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is enable!\n");
    }

    /*高温检测:充电器不在位时不对充电温度做异常判断*/
    if (FALSE == chg_is_charger_present())
    {
        up_over_temp_flag = FALSE;
    }
    else if(FALSE == up_over_temp_flag)
    {   /*1、带高温充电功能产品通过NV50385将电池温度门限设置为55度
          2、不带高温充电功能产品通过NV50385将电池温度门限设置为45度*/
        #if defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
        ||defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW) \
        || defined(BSP_CONFIG_BOARD_R218h)
        if (CHG_BATT_ID_DEF == chg_get_batt_id())
        {
            if(CHG_DEF_BATT_TEMP_OVER_THRES <= chg_real_info.battery_temp)
            {
                up_over_temp_flag = TRUE;
            }
        }
        else if(CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp)
        {
            up_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't up_over_temp!\n");
        }
        #else
        if(CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp)
        {
            up_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't up_over_temp!\n");
        }
        #endif
    }
    else
    {
          /*1、带高温充电功能产品通过NV50385将电池温度门限设置为52度
          2、不带高温充电功能产品通过NV50385将电池温度门限设置为42度*/
        #if defined(BSP_CONFIG_BOARD_E5573CS) ||defined(BSP_CONFIG_BOARD_E5573BS) \
        || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW) \
        || defined(BSP_CONFIG_BOARD_R218h)
        if (CHG_BATT_ID_DEF == chg_get_batt_id())
        {
            if(CHG_DEF_BATT_TEMP_OVER_RUSUM_THRES > chg_real_info.battery_temp)
            {
                up_over_temp_flag = FALSE;
            }
        }
        else if(CHG_OVER_TEMP_RESUME_THRESHOLD > chg_real_info.battery_temp)
        {
            up_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery up temp don't resume nomal!\n");
        }
        #else
        if(CHG_OVER_TEMP_RESUME_THRESHOLD > chg_real_info.battery_temp)
        {
            up_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery up temp don't resume nomal!\n");
        }
        #endif
    }

    /*低温检测:充电器不在位时不对充电温度做异常判断*/
    if (FALSE == chg_is_charger_present())
    {
        low_over_temp_flag = FALSE;
    }
    else if(FALSE == low_over_temp_flag)
    {
        /*归一化规格电池温度小于0度低温停充*/
        if(CHG_LOW_TEMP_STOP_THRESHOLD > chg_real_info.battery_temp)
        {
            low_over_temp_flag = TRUE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery temp don't low_over_temp!\n");
        }
    }
    else
    {
        /*归一化规格电池温度大于等于3度恢复常温充电*/
        if(CHG_LOW_TEMP_RESUME_THRESHOLD <= chg_real_info.battery_temp)
        {
            low_over_temp_flag = FALSE;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:battery low temp don't resume nomal!\n");
        }
    }

    /*根据高低温检测结果判断充电温保护状态*/
    if((up_over_temp_flag == TRUE) || (low_over_temp_flag == TRUE))
    {
        chg_temp_protect_flag = TRUE;
    }
    else
    {
        chg_temp_protect_flag = FALSE;
    }
}
/*******************************************************************
Function:      chg_set_supply_current_by_temp
Description:   温保需求，根据温度配置前端限流
Data Accessed: none
Data Updated:  none
Input:         none
Return:        noee
*******************************************************************/
void chg_set_supply_current_by_temp(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();
    uint8_t need_resume_supply = 0;
    uint8_t curr_limit;
    /*支持可维可测*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_no_battery_powerup_enable() 
        || (0 == CHG_TEMP_PROTECT_ENABLE))
    {
        return;
    }

    if( TRUE == chg_is_powdown_charging())/*关机充电模式*/
    {
        return;
    }

#if( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    if( TRUE == chg_battery_protect_flag )
    {
        if(TRUE == chg_limit_supply_current_flag)
        {
            chg_limit_supply_current_flag = FALSE;
        }
        return;
    }
#endif

    if(TRUE == is_chg_charger_removed())
    {
        if(TRUE == chg_limit_supply_current_flag)
        {
            chg_limit_supply_current_flag = FALSE;
        }
        return;
    }
    
    if((chg_real_info.battery_temp >= LIMIT_SUPPLY_CURR_TEMP) 
        && (FALSE == chg_limit_supply_current_flag))
    {
        chg_limit_supply_current_flag = TRUE;
    }
    else if((chg_real_info.battery_temp <= LIMIT_SUPPLY_CURR_TEMP - LIMIT_SUPPLY_CURR__RESUME_LEN) 
        && (TRUE == chg_limit_supply_current_flag))
    {
        chg_limit_supply_current_flag = FALSE;
        need_resume_supply = 1;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current in high temp!\n");
    }

    if(TRUE == chg_limit_supply_current_flag)
    {
        curr_limit = chg_get_supply_limit();
        if(BQ24192_IINLIMIT_100 != curr_limit)
        {
            chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set supply current in high temp!\n");
        }

    }
    else
    {
        if(need_resume_supply)
        {
            switch(cur_chgr_type)
            {
                case CHG_WALL_CHGR:
                {
                    chg_set_supply_limit(chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current:%d\n",
                            chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                    break;
                }

                case CHG_USB_HOST_PC:
                {
                    if(CHG_CURRENT_SS == usb_speed_work_mode())
                    {
                        chg_set_supply_limit(chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current:%d\n",
                            chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                    }
                    else
                    {
                        chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current:%d\n",
                            chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                    }
                    break;
                }
                case CHG_500MA_WALL_CHGR:
                case CHG_NONSTD_CHGR:
                {
                    chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume supply current:%d\n",
                            chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                    break;
                }
                default:
                break;
            }
        }
    }
}


boolean chg_temp_protect_eixt_suspend_mode(void)
{
    boolean ret = TRUE;
    /*是否满足退出suspend条件*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    chg_print_level_message(CHG_MSG_DEBUG, \
        "CHG_STM:chg_temp_protect_eixt_suspend_mode chg_battery_protect_flag is %d!\r\n", chg_battery_protect_flag);
#endif

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_print_level_message(CHG_MSG_DEBUG, \
        "CHG_STM:chg_usb_temp_info.usb_temp_protect_cur_stat chg_battery_protect_flag is %d!\r\n",\
        chg_usb_temp_info.usb_temp_protect_cur_stat);
#endif

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT ) && (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    if ( (FALSE == chg_battery_protect_flag) && (FALSE == chg_usb_temp_info.usb_temp_protect_cur_stat) )
    {
        ret = chg_set_suspend_mode(FALSE);
    }
#else
    ret = chg_set_suspend_mode(FALSE);
#endif

    return ret;
}

#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )

static void chg_usb_over_temp_protect_enter_process(void)
{
    /*1.设置充电芯片进入suspend。2.更新POWER_SUPPLY节点。3.mlog。*/
    if ( TRUE != chg_set_suspend_mode(TRUE) )
    {
        chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC to suspend fail in usb high temp!\n");
#if ( FEATURE_ON == MBB_MLOG )
        mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Set charge IC to suspend fail in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif
        return;
    }
    chg_usb_temp_info.usb_temp_protect_pre_stat = chg_usb_temp_info.usb_temp_protect_cur_stat;
    chg_stm_state_info.usb_heath_type = POWER_SUPPLY_USB_TEMP_DEAD;
    chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)USB_TEMP_HIGH);
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC to suspend in usb high temp!\n");
#if ( FEATURE_ON == MBB_MLOG )
    printk(KERN_ERR "CHG_STM:USBPortOverTempCnt\n");
    mlog_set_statis_info("USBPortOverTempCnt",1);/*USB温保前段限流次数+1*/
    mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Entry usb temp protect in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif/*MBB_MLOG*/
    return;
}

static void chg_usb_over_temp_protect_exit_process(void)
{
    /*1.设置充电芯片退出suspend*/
    if ( TRUE != chg_temp_protect_eixt_suspend_mode() )
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC to suspend is fail in usb high temp!\n");
#if (FEATURE_ON == MBB_MLOG)
        mlog_print(MLOG_CHG, mlog_lv_error, "CHG_DCM: Set charge IC to suspend is fail in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif
        return;
    }

    /*1.根据当前状态恢复输入电流。2.更新POWER_SEPPLY节点。3.mlog。*/
    chg_set_supply_limit_by_stm_stat();
    chg_usb_temp_info.usb_temp_protect_pre_stat = chg_usb_temp_info.usb_temp_protect_cur_stat;
    chg_stm_state_info.usb_heath_type = POWER_SUPPLY_USB_TEMP_GOOD;
    chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)USB_TEMP_NORMAL);
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Resume charge IC from suspend in usb high temp!\n");
#if (FEATURE_ON == MBB_MLOG)
    mlog_print(MLOG_CHG, mlog_lv_error, "CHG_STM: Resume usb temp protect in usb temp %d.\n",
            chg_usb_temp_info.usb_cur_temp);
#endif
    return;
}

void chg_usb_temp_protect_proc(void)
{
    int32_t         usb_temp = TEMP_INITIAL_VALUE;
    int32_t         usb_temp_detect_count_t = USB_TEMP_DETECT_COUNT;

    /*获取当前温度值*/
    usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
    chg_usb_temp_info.usb_cur_temp = usb_temp;

    /*充电温保护使能关闭*/
    if (0 == CHG_TEMP_PROTECT_ENABLE)
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:CHG_TEMP_PROTECT is disable!\n");
        chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        return;
    }

    /*判断三次温度值是否符合触发和恢复门限，并做相应处理.*/
    if ( FALSE == chg_usb_temp_info.usb_temp_protect_cur_stat)
    {
        while ( --usb_temp_detect_count_t )
        {
            if ( CHG_USB_TEMP_LIMIT > usb_temp )
            {
                break;
            }
            usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
        }
        if ( 0 == usb_temp_detect_count_t )
        {
            chg_usb_temp_info.usb_temp_protect_cur_stat = TRUE;
        }
    }
    else
    {
        while ( --usb_temp_detect_count_t )
        {
            if ( CHG_USB_TEMP_RESUME < usb_temp )
            {
                break;
            }
            usb_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
        }
        if ( 0 == usb_temp_detect_count_t )
        {
            chg_usb_temp_info.usb_temp_protect_cur_stat = FALSE;
        }
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:usb_cur_temp=%d\n",chg_usb_temp_info.usb_cur_temp);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:usb_temp_protect_stat=%d\n",
            chg_usb_temp_info.usb_temp_protect_cur_stat);
    /*判断当前所处状态,并作相应处理*/
    if ( chg_usb_temp_info.usb_temp_protect_cur_stat != chg_usb_temp_info.usb_temp_protect_pre_stat )
    {
        if ( TRUE == chg_usb_temp_info.usb_temp_protect_cur_stat )
        {
            chg_usb_over_temp_protect_enter_process();
        }
        else
        {
            chg_usb_over_temp_protect_exit_process();
        }
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Set usb port stat by temp finish!\n");
}

int32_t chg_get_usb_health(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_health=%d\n",chg_stm_state_info.bat_heath_type);
    return chg_stm_state_info.usb_heath_type;
}

int32_t chg_get_usb_cur_temp(void)
{
    if ( TEMP_INITIAL_VALUE == chg_usb_temp_info.usb_cur_temp )
    {
        chg_usb_temp_info.usb_cur_temp = chg_get_temp_value(CHG_PARAMETER__USB_PORT_TEMP_DEGC);
    }
    return chg_usb_temp_info.usb_cur_temp;
}

boolean chg_get_usb_temp_protect_stat(void)
{
    return chg_usb_temp_info.usb_temp_protect_cur_stat;
}
#endif
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )

static boolean chg_battery_protect_enter_process(void)
{

    /*不支持suspend的使用前段限流100MA*/
    if (TRUE != chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Set supply current 100MA is fail in battery protect!\n");
        return FALSE;
    }

    return TRUE;
}


static void chg_battery_protect_exit_process(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    chg_stm_state_type curr_state = chg_stm_get_cur_state();

    /*curr_state要作为数组下标进行索引，所以这里需要判断一下合法性。*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

    cur_chgr_type = chg_stm_get_chgr_type();
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:battery protect resume!\n");
    switch(cur_chgr_type)
    {
        case CHG_WALL_CHGR:
        {
            chg_set_hardware_parameter(&chg_std_chgr_hw_paras[curr_state]);
            break;
        }
        case CHG_USB_HOST_PC:
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[curr_state]);
            }
            break;
        }
        case CHG_500MA_WALL_CHGR:
        case CHG_NONSTD_CHGR:
        case CHG_CHGR_UNKNOWN:
        {
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[curr_state]);
            break;
        }
        default:
        {
            break;
        }
    }

}


static void chg_poll_volt_temp_protect_state(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    cur_chgr_type = chg_stm_get_chgr_type();

    /*电池膨胀保护TBAT>=45°and VBAT >=4.1V 状态轮询*/
    if ((FALSE == g_chg_over_temp_volt_protect_flag)
        && (chg_get_sys_batt_temp () >= CHG_BATTERY_PROTECT_TEMP)
        && (chg_get_sys_batt_volt () >= CHG_BATTERY_PROTECT_VOLTAGE))
    {
        g_chg_over_temp_volt_protect_flag = TRUE;
        /*高温高压触发电池膨胀保护+1*/
        mlog_set_statis_info("BattExpandOverTempVoltCnt",1);
    }
    else if ((TRUE == g_chg_over_temp_volt_protect_flag)
            && ((chg_get_sys_batt_temp () < CHG_BATTERY_PROTECT_RESUME_TEMP)
            || (chg_get_sys_batt_volt () < CHG_BATTERY_PROTECT_RESUME_VOLTAGE)))
    {
        g_chg_over_temp_volt_protect_flag = FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:g_chg_over_temp_volt_protect_flag is %d!\n",
            g_chg_over_temp_volt_protect_flag);
}


uint32_t chg_stm_get_no_charging_charger_lasted_time(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:charger_lasted_without_charging_in_seconds=%u\n",\
                    chg_stm_state_info.charger_lasted_without_charging_in_seconds);
    /*1. Return the current time.*/
    return chg_stm_state_info.charger_lasted_without_charging_in_seconds;
}


void set_long_time_no_charge_protect_recharge_volt(void)
{
    /*长时间电源在位不充电进入保护后，修改常温复充门限为4.0v*/
    g_chgBattVoltProtect.battNormalTempChgRechargeThreshold = BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    g_chg_batt_expand_change_normal_rechg_flag = TRUE;
#endif
}


void resume_long_time_no_charge_protect_recharge_volt(void)
{
    /*电源拔出后，恢复常温复充门限为4.2v*/
    g_chgBattVoltProtect.battNormalTempChgRechargeThreshold = g_batt_normal_temp_recherge_threshold;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    g_chg_batt_expand_change_normal_rechg_flag = FALSE;
#endif
}


static void chg_poll_long_time_no_charge_state(void)
{
    uint32_t charger_lasted_time = 0;
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    cur_chgr_type = chg_stm_get_chgr_type();
    /*停充且充电器在位的时间*/
    charger_lasted_time = chg_stm_get_no_charging_charger_lasted_time();

    /*电池膨胀保护充电器在位持续>=16H且不充电状态轮询*/
    if ((FALSE == g_chg_longtime_nocharge_protect_flag)
        && (charger_lasted_time >= CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS)
        && (chg_get_sys_batt_volt () >= CHG_BATTERY_PROTECT_VOLTAGE))
    {
        g_chg_longtime_nocharge_protect_flag = TRUE;
        /*长时间电源在位不充电进入保护后，修改常温复充门限为4.0v*/
        (void)set_long_time_no_charge_protect_recharge_volt();
        /*USB长时间在位触发电池膨胀保护+1*/
        mlog_set_statis_info("BattExpandLongTimeCnt",1);
    }
    else if ((TRUE == g_chg_longtime_nocharge_protect_flag)
            && (chg_get_sys_batt_volt () < CHG_BATTERY_PROTECT_RESUME_VOLTAGE))
    {
        g_chg_longtime_nocharge_protect_flag = FALSE;
    }
    else
    {
        /*do nothing*/
    }
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:g_chg_longtime_nocharge_protect_flag is %d!\n",
            g_chg_longtime_nocharge_protect_flag);
}


static void chg_battery_protect_proc(void)
{
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
    chg_stm_state_type curr_state = CHG_STM_INIT_ST;
    cur_chgr_type = chg_stm_get_chgr_type();
    curr_state = chg_stm_get_cur_state();
    /*支持可维可测*/
    if ( (TRUE == chg_is_ftm_mode() ) \
        || (TRUE == chg_is_no_battery_powerup_enable() ) \
        || (0 == CHG_TEMP_PROTECT_ENABLE))
    {
        chg_battery_protect_flag = FALSE;
        g_chg_over_temp_volt_protect_flag = FALSE;
        g_chg_longtime_nocharge_protect_flag = FALSE;
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_chg_batt_expand_change_normal_rechg_flag = FALSE;
#endif
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:In FTM mode or TEMP PROTECT DISABLE do not enable battery protect!\n");
        return;
    }

    /*电源拔出后，退出电池膨胀保护*/
    if (TRUE == is_chg_charger_removed())
    {
        if (TRUE == chg_battery_protect_flag)
        {
            chg_battery_protect_flag = FALSE;
            g_chg_over_temp_volt_protect_flag = FALSE;
            g_chg_longtime_nocharge_protect_flag = FALSE;
            (void)chg_battery_protect_exit_process();
        }
        (void)resume_long_time_no_charge_protect_recharge_volt();

        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:charger remove disenable battery protect!\n");
        return;
    }

    if (FALSE == chg_battery_protect_flag)
    {
        /*满电停充后才进入Suspend/输入限流100mA*/
        if ( (curr_state != CHG_STM_MAINT_ST) || (BATT_CAPACITY_FULL != chg_real_info.bat_capacity) )
        {
                chg_print_level_message(CHG_MSG_INFO,
                    "CHG_STM:battery is not in full state, no need to protect!\n");
                return;
        }
    }
#if (MBB_BATT_EXPAND_HTEMP_DISCHG == FEATURE_ON)
    /*轮询保护状态*/
    (void)chg_poll_volt_temp_protect_state();
#endif
#if (MBB_BATT_EXPAND_LTIME_DISCHG == FEATURE_ON)
    (void)chg_poll_long_time_no_charge_state();
#endif

    /*两种保护有一个触发将进入保护*/
    if ( (TRUE == g_chg_over_temp_volt_protect_flag)
        || (TRUE == g_chg_longtime_nocharge_protect_flag) )
    {
        if (FALSE == chg_battery_protect_flag)
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:meet battery protect condition do battery protect!\n");
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:g_chg_over_temp_volt_protect_flag is %d!\n",
                g_chg_over_temp_volt_protect_flag);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:g_chg_longtime_nocharge_protect_flag is %d!\n",
                g_chg_longtime_nocharge_protect_flag);
            if( TRUE != chg_battery_protect_enter_process() )
            {
                return;
            }
            chg_battery_protect_flag = TRUE;
            /*进入电池膨胀保护+1*/
            mlog_set_statis_info("BattExpandProtectCnt",1);
            mlog_print(MLOG_CHG, mlog_lv_error, 
                    "CHG_STM: Entry battery expand protect by OverTempVolt:%d LongTime %d.\n",
                            g_chg_over_temp_volt_protect_flag,g_chg_longtime_nocharge_protect_flag);
        }
    }
    else
    {
        if (TRUE == chg_battery_protect_flag)
        {
            chg_battery_protect_flag = FALSE;
            chg_battery_protect_exit_process();
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:To resume from battery expand protect.\n");
            mlog_print(MLOG_CHG, mlog_lv_error,"CHG_STM: To resume from battery expand protect.\n");
        }
    }
}
#endif

/*******************************************************************
Function:       Chg_temp_is_too_hot_or_too_cold_for_shutoff
Description:   电池温度判断，当其温度超过关机门限时，调用关机函数接口，区分高温和低温。
Calls:         无；
Return:        无
*******************************************************************/
void chg_temp_is_too_hot_or_too_cold_for_shutoff(void)
{
    static uint32_t countNum = 0;
    static uint32_t high_temp_58_flag = FALSE;
    static uint32_t high_timer_flag = FALSE;
    static uint32_t low_timer_flag = FALSE;
#if !defined(BSP_CONFIG_BOARD_E5771S_852) && !defined(BSP_CONFIG_BOARD_E5771H_937)
    static uint32_t up_over_temp_shutoff_falg = FALSE;
    static uint32_t low_over_temp_shutoff_falg = FALSE;
#endif

    /*烧片版本不做处理*/
    /*可维可测:无电池开机使能*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_no_battery_powerup_enable() )
    {
        return;
    }

    /*当单板在开机状态下，温度高于等于(关机温度-2度)时上报给应用，告警提示*/
    if( FALSE == chg_is_powdown_charging())/*开机模式*/
    {
        if(chg_real_info.battery_temp >= (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN))
        {
            if(FALSE == high_temp_58_flag)
            {
                chg_batt_high_temp_58_flag = TRUE;
                /*切换轮询方式，高温进入快轮询模式*/
                chg_poll_timer_set(FAST_POLL_CYCLE);
                high_temp_58_flag = TRUE;
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT below 58 degree!\n");
            }

            /*上报APP，高温告警提示，上报5次，避免开机即高温，APP启动较慢收不到消息*/
            if(BATTERY_EVENT_REPORT_TIMES >= countNum)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send app msg with TEMP_BATT_HIGH \n ");

                chg_batt_temp_state = TEMP_BATT_HIGH;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery over-heated WARNING!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, " \
                    "[vBat_sys]%dmV, [tBat]%d'C, [tBat_sys]%d'C.\n", 
                    chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT_HIGH event report below 5 times!\n");
            }

            countNum++;
        }
        /*当单板在开机状态下，温度低于等于(关机温度-5度)时上报给应用，取消告警*/
        else if((chg_real_info.battery_temp <= (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_RESUME_LEN)))
        {
            if(TRUE == high_temp_58_flag)
            {
                chg_batt_high_temp_58_flag = FALSE;
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send app msg with TEMP_BATT_NORMAL \n ");
                chg_batt_temp_state = TEMP_BATT_NORMAL;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
                high_temp_58_flag = FALSE;
                countNum = 0;
            }
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:TEMP_BATT_HIGH!\n");
        }
    }
    else
    {
        /*关机状态不做处理*/
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System in power down chging do nothing!\n");
    }


    /*温度关机检测------高温检测*/
    if(SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD <= chg_real_info.battery_temp)
    {
        if(TRUE == SHUTOFF_OVER_TEMP_PROTECT_ENABLE)
        {
            if( FALSE == chg_is_powdown_charging() )/*开机模式*/
            {
                up_over_temp_shutoff_falg = TRUE;

                /*todo 上报APP执行高温关机操作，以按键关机形式上报*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for high temp power off  \n ");
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery Over-heated, system down!!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, " \
                    "[vBat_sys]%dmV, [tBat]%d'C, [tBat_sys]%d'C.\n",
                    chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);

                if(FALSE == high_timer_flag)
                {
                    high_timer_flag = TRUE;

                    /*启动定时器，回调函数为关机函数，定时时长为15秒；*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 15s timer for high temp power off \n ");
                    chg_bat_timer_set( OVER_TEMP_SHUTOFF_DURATION, \
                    chg_set_power_off, DRV_SHUTDOWN_TEMPERATURE_PROTECT);
                }
                else
                {
                    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Already start power off timer!\n");
                }
            }
            else //关机模式
            {
                /* 关机充电>=65度上报  */
                #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
                if (FALSE == is_chg_charger_removed())
                {
                    up_over_temp_shutoff_falg = TRUE;
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:pd charge send app high temp power off\n ");
                    chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
                    chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);
                }
                #else
                /*关机模式下的温度高于关机门限，不做处理*/
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:high temp but power down chging do nothing!\n");
                #endif
            }
        }
        else
        {
             chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Disable high temp protect!\n");
        }
    }
    #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    /* 关机充电 < 63 度上报  */
    if(chg_real_info.battery_temp < SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN )
    {
        if (FALSE == is_chg_charger_removed())
        {
            if( TRUE == chg_is_powdown_charging() && up_over_temp_shutoff_falg == TRUE)
            {   
                up_over_temp_shutoff_falg = FALSE;
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:pdcharge send app hightemp back off\n ");
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
            }
        }
    }
    #endif

    /*温度关机检测------低温检测*/
    if(SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD >= chg_real_info.battery_temp)
    {
        if(TRUE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE)
        {
            if( FALSE == chg_is_powdown_charging() )/*开机模式*/
            {
                low_over_temp_shutoff_falg = TRUE;
                /*上报APP执行低温关机操作*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for low temp power off  \n ");

                chg_batt_temp_state = TEMP_BATT_LOW;
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);

                mlog_print(MLOG_CHG, mlog_lv_warn, "Battery too Cold, system down!!!\n");
                mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, " \
                    "[vBat_sys]%dmV, [tBat]%d'C, [tBat_sys]%d'C.\n",
                    chg_real_info.battery_one_volt, chg_real_info.battery_volt,
                    chg_real_info.battery_one_temp, chg_real_info.battery_temp);

                if(FALSE == low_timer_flag )
                {
                    low_timer_flag = TRUE;
                    /*起定时器计时15秒，当15秒时间到后APP仍未关机，则底层自己执行关机操作；*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 15s timer for low temp power off \n ");
                    chg_bat_timer_set( OVER_TEMP_SHUTOFF_DURATION, \
                    chg_set_power_off,DRV_SHUTDOWN_LOW_TEMP_PROTECT);
                }
            }
            else
            {
                #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
                if (FALSE == is_chg_charger_removed())
                {
                    low_over_temp_shutoff_falg = TRUE;
                    /* 关机充电<=-20度上报  */
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:pdcharge send app lowtemp power off  \n ");
                    chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
                    chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);
                }
                #else
                /*关机模式下的温度低于关机门限，不做处理*/
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:low temp but power down chging do nothing!\n");
                #endif
            }
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Disable low temp protect!\n");
        }
    }
    #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if(chg_real_info.battery_temp > SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD + SHUTOFF_HIGH_TEMP_WARN_LEN )
    {    /* 关机充电> -18度上报  */
        if (FALSE == is_chg_charger_removed())
        {
            if( TRUE == chg_is_powdown_charging() && low_over_temp_shutoff_falg == TRUE)
            {
                low_over_temp_shutoff_falg = FALSE;
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:pdcharge send app lowtemp back off\n ");
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
            }
        }
    }
    #endif
}

/*******************************************************************
Function:      chg_poll_batt_temp
Description:   电池温度值加工处理的流程控制函数。
Calls:          chg_get_parameter_level-----------------采集温度值接口函数；
                chg_calc_average_temp_value-------------平滑算法处理函数；
                chg_huawei_set_temp----------------------温度补偿函数
                chg_temp_is_too_hot_or_too_cold_for_chg ----------充电温保护的温度检测函数
                chg_temp_is_too_hot_or_too_code_for_shutoff------温度关机条件检测函数
Data Accessed: 无
Data Updated:  全局变量的温度值
Input:          无
Output:         无
Return:         无
*******************************************************************/
void chg_poll_batt_temp(void)
{
    int32_t new_one_batt_temp = 0;
    int32_t new_batt_temp = 0;
    static uint32_t init_flag = FALSE;

    /*坏电池检测处理函数*/
    chg_batt_error_detect_temp();

    /*调用参数获取接口函数获取电压值*/
    new_one_batt_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);

    #if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    new_one_batt_temp = chg_batt_temp_revise(new_one_batt_temp);//电池温度补偿
    #endif

    /*保存单次采集的温度值到全局变量中*/
    chg_real_info.battery_one_temp = new_one_batt_temp;

    /*电池平滑温度初始化为首次采集值*/
    if(FALSE == init_flag)
    {
        chg_real_info.battery_temp = chg_real_info.battery_one_temp;
        init_flag = TRUE;
    }
    /*调用温度补偿函数对单次采集的温度进行补偿，不同平台对温度补偿要求不同
      有的要求补偿平滑后的温度，有的要求补偿平滑前的温度，9X25对外充电对单次
      电池温度进行补偿，因此删除下边的平滑温度补偿*/
    new_one_batt_temp = chg_huawei_set_temp(new_one_batt_temp);

    /*调用平滑算法对当前电池温度进行平滑运算*/
    new_batt_temp = chg_calc_average_temp_value(new_one_batt_temp);

    /*调用温度补偿函数进行对平滑后的温度进行补偿*/
    //new_batt_temp = chg_huawei_set_temp(new_batt_temp);

#if defined(BSP_CONFIG_BOARD_607HW)
    /*当电池温度发生变化时，及时通知应用查询电池温度*/
    if ( chg_real_info.battery_temp != new_batt_temp )
    {
        /*当电池温度发生变化时,首先保存最新获取的温度值到全局变量中,确保应用查询到的一定是最新的温度值*/
        chg_real_info.battery_temp = new_batt_temp;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM: curr_batt_temp=%d\n",new_batt_temp);
    }
#endif

    /*保存最终获取的温度值到全局变量中*/
    chg_real_info.battery_temp = new_batt_temp;

    /*调用充电温保护的温度检测函数执行*/
    chg_temp_is_too_hot_or_too_cold_for_chg ( );

    /* log打印接口，可维可测讨论内容*/
    chg_print_level_message(CHG_MSG_DEBUG, "**********CHG TEMP PARA: chg_poll_batt_temp  begin *********\n");

    /*打印充电温保护使能开关*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg temp protect enable flag = %d\n", CHG_TEMP_PROTECT_ENABLE);

    /*高、低温温保护充电停充门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_high_thr = %d,chg_low_thr = %d\n", \
        CHG_OVER_TEMP_STOP_THRESHOLD, CHG_LOW_TEMP_STOP_THRESHOLD);

    /*高、低温温保护充电复充门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:rechg_high_thr = %d,rechg_low_thr = %d\n", \
        CHG_OVER_TEMP_RESUME_THRESHOLD, CHG_LOW_TEMP_RESUME_THRESHOLD);

    /*打印高/低温保护关机使能开关；*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff over temp protect enable flag = %d\n",
                            SHUTOFF_OVER_TEMP_PROTECT_ENABLE);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff low  temp protect enable flag = %d\n",
                            SHUTOFF_LOW_TEMP_PROTECT_ENABLE);

    /*高、低温温保护关机门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff_high_thr = %d,shutoff_low_thr = %d\n", \
        SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD, SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD);

    /*打印单次电池温度和处理后的电池温度*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_one_temp = %d\n", chg_real_info.battery_one_temp);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_temp = %d\n", chg_real_info.battery_temp);

    /*打印当前的温保护状态chg_temp_protect_flag的值*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_temp_protect_flag = %d\n", chg_temp_protect_flag);

    chg_print_level_message(CHG_MSG_DEBUG, "**********CHG TEMP PARA: chg_poll_batt_temp  end  *********\n");

    /*调用温度关机条件检测执行*/
    chg_temp_is_too_hot_or_too_cold_for_shutoff( );
#if (FEATURE_ON == MBB_THERMAL_PROTECT)
#if !defined(BSP_CONFIG_BOARD_E5_E5770s) && !defined(BSP_CONFIG_BOARD_E5771S_852) && !defined(BSP_CONFIG_BOARD_E5771H_937) \
    && !defined(BSP_CONFIG_BOARD_E5575S_320) && !defined(BSP_CONFIG_BOARD_E5573CS) && !defined(BSP_CONFIG_BOARD_E5573BS)\
    && ! defined(BSP_CONFIG_BOARD_E5573CS_933) && !defined(BSP_CONFIG_BOARD_607HW) && !defined(BSP_CONFIG_BOARD_R218h)\
    && ! defined(BSP_CONFIG_BOARD_E5577BS_937) && ! defined(BSP_CONFIG_BOARD_E5577S_324)
    chg_set_supply_current_by_temp();
#endif
#endif
    /*电池膨胀保护处理*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    chg_battery_protect_proc();
#endif
    /*USB温保*/ 
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
    chg_usb_temp_protect_proc();
#endif
}

/*******************************************************************
Function:      chg_calc_average_volt_value
Description:   电池电压平滑处理函数
Data Accessed: 无
Data Updated:  无
Input:         新采集的温度值
Return:        平滑后的温度值
*******************************************************************/
int32_t chg_calc_average_volt_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_volt_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = {0};

    /*查询轮询模式*/
    new_poll_mode = chg_poll_timer_get();

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE == new_poll_mode)/*由慢轮询切换到当前的快轮询*/
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = chg_real_info.battery_volt;
            }
            record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = chg_real_info.battery_volt;
            }
            record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "new_poll_mode = old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE == new_poll_mode)//快轮询模式
    {
        /*数组中当前元素标号小于30*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于30个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_VOLT_SMOOTH_SAMPLE_FAST;
            }
        }
    }
    else//慢轮询模式
    {
        /*数组中当前元素标号小于5*/
        if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于5个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;

            /*如果元素个数标号是静态数组长度的两倍,重新置元素个数标号是静态数组长度即CHG_BAT_TEMP_SMOOTH_SAMPLE*/
            if(CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_VOLT_SMOOTH_SAMPLE_SLOW;
            }
        }

    }

    return bat_volt_avg;
}

/*******************************************************************
Function:      chg_volt_level_to_capacity
Description:   电池电量格数与百分比转换
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_volt_level_to_capacity(BATT_LEVEL_ENUM bat_volt_level)
{
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
    switch(bat_volt_level)
    {
        case BATT_LOW_POWER:
        {
            /*未充电场景才允许将电量设置为低电低电电量，防止上报低电电量后应用在充电场景进行低电提示*/
            if(FALSE == chg_get_charging_status())
            {
                chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
            }
            break;
        }
        case BATT_LEVEL_1:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL1);
            break;
        }
        case BATT_LEVEL_2:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL2);
            break;
        }
        case BATT_LEVEL_3:
        {
            chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL3);
            break;
        }
        case BATT_LEVEL_4:
        {
            /*满电停充设置电池电量为100*/
            if(CHG_STM_MAINT_ST == cur_stat)
            {
                chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_volt_level_to_capacity-->CHG_STM_MAINT_ST!\n ");
            }
            else
            {
                /*非满电停充但电池电压大于4.1V也设置电池电量为100*/
                if(chg_get_sys_batt_volt() >= BATT_CHG_TEMP_MAINT_THR)
                {
                    chg_set_sys_batt_capacity(BATT_CAPACITY_FULL);
                    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_volt_level_to_capacity VBAT>=4.1V!\n ");
                }
                else
                {
                    chg_set_sys_batt_capacity(BATT_CAPACITY_LEVEL4);
                }
            }

            break;
        }

        default:
            break;
    }

}


#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*******************************************************************
Function:      chg_soc2level
Description:   电池电量格数转换
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
BATT_LEVEL_ENUM chg_soc2level(int soc)
{
    BATT_LEVEL_ENUM volt_level;

    if ( (soc > BATT_CAPACITY_FULL) || (soc < 0) )
    {
        chg_print_level_message(CHG_MSG_ERR,\
            "chg_percent2level param %d error!! not in [0 ~ 100]\n ", soc);
    }

    if (soc < BATT_CAPACITY_LEVEL1) 
    {
        volt_level = BATT_LOW_POWER;
    }
    else if (soc < BATT_CAPACITY_LEVEL1)
    {
        volt_level = BATT_LEVEL_0;
    }
    else if (soc < BATT_CAPACITY_LEVEL2)
    {
        volt_level = BATT_LEVEL_1;
    }

    else if (soc < BATT_CAPACITY_LEVEL3)
    {
        volt_level = BATT_LEVEL_2;
    }
    else if (soc < BATT_CAPACITY_LEVEL4)
    {
        volt_level = BATT_LEVEL_3;
    }
    else 
    {
        volt_level = BATT_LEVEL_4;
    }

    return volt_level;
}
/*******************************************************************
Function:      chg_set_battery_level
Description:   电池电量格数设置
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_set_battery_level(void)
{
    int soc = 0;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    BATT_LEVEL_ENUM pre_bat_level = BATT_LEVEL_MAX;
    soc = chg_get_sys_batt_capacity();
    bat_volt_level = chg_soc2level(soc);
    pre_bat_level = chg_real_info.bat_volt_lvl;
    
   /*非充电状态，不允许电池电压反转*/
    if(FALSE == chg_get_charging_status())
    {
        if(bat_volt_level <= chg_real_info.bat_volt_lvl )
        {
            chg_real_info.bat_volt_lvl = bat_volt_level;
            chg_print_level_message(CHG_MSG_DEBUG,\
            "CHG_STM:NO chargin state volt_lvl decline to %d!\n ",bat_volt_level);
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG,\
            "CHG_STM:NO chargin state only allow volt_lvl decline!\n ");
        }
    }//充电状态
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,\
        "CHG_STM:chargin state volt_lvl increase to %d!\n ",bat_volt_level);
        chg_real_info.bat_volt_lvl = bat_volt_level;
    }

    if(BATT_LOW_POWER == chg_real_info.bat_volt_lvl) 
    {
        chg_batt_low_battery_flag = TRUE;
        if(FAST_POLL_CYCLE != chg_poll_timer_get())
        {
            chg_poll_timer_set(FAST_POLL_CYCLE);
        }
    }

    if(pre_bat_level != chg_real_info.bat_volt_lvl)
    {
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,\
                            (uint32_t)CHG_EVENT_NONEED_CARE);
    }

}
#endif
/*******************************************************************
Function:      chg_set_battery_volt_level
Description:   电池电量格数判断设置
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_set_battery_volt_level(void)
{
    static uint32_t count = 0;
    BATT_LEVEL_ENUM bat_volt_level = BATT_LEVEL_MAX;
    int32_t batt_volt = chg_real_info.battery_volt;

    if(batt_volt < BATT_VOLT_LEVELLOW_MAX)
    {

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        /*对外充电器在位且用户选择了仅对外充电或者对外充电加数据业务则不上报低电*/
        if(FALSE == is_5577cs_falg)
        {
            if(g_ui_choose_exchg_mode <= 0)
            {
                bat_volt_level = BATT_LOW_POWER;
            }
            else
            {
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:EXTCHG do not report Lowpower!\n");
            }
        }
        else
        {
            bat_volt_level = BATT_LOW_POWER;
        }
#else
        bat_volt_level = BATT_LOW_POWER;
#endif /*defined(MBB_CHG_EXTCHG)*/
    }

    else if(batt_volt < BATT_VOLT_LEVEL1_MAX)
    {
        bat_volt_level = BATT_LEVEL_1;
    }
    else if(batt_volt < BATT_VOLT_LEVEL2_MAX)
    {
        bat_volt_level = BATT_LEVEL_2;
    }
    else if(batt_volt < BATT_VOLT_LEVEL3_MAX)
    {
        bat_volt_level = BATT_LEVEL_3;
    }
    else
    {
        bat_volt_level = BATT_LEVEL_4;
    }

   /*非充电状态，不允许电池电压反转*/
    if(FALSE == chg_get_charging_status())
    {
        if(bat_volt_level <= chg_real_info.bat_volt_lvl )
        {
            chg_real_info.bat_volt_lvl = bat_volt_level;
            chg_volt_level_to_capacity(bat_volt_level);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NO chargin state volt_lvl decline to %d!\n ",bat_volt_level);
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:NO chargin state only allow volt_lvl decline!\n ");
        }
    }//充电状态
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chargin state volt_lvl increase to %d!\n ",bat_volt_level);
        chg_real_info.bat_volt_lvl = bat_volt_level;
        chg_volt_level_to_capacity(bat_volt_level);
    }


    /*低电非充电,且非关机充电状态下，上报APP低电事件*/
    if((BATT_LOW_POWER == chg_real_info.bat_volt_lvl) \
       && (FALSE == chg_is_powdown_charging()) \
       && (FALSE == chg_get_charging_status()))
    {
        if(BATTERY_EVENT_REPORT_TIMES > count) /* 上报5次 */
        {
            /*调用接口函数上报APP低电事件*/
            chg_batt_low_battery_flag = TRUE;
            chg_poll_timer_set(FAST_POLL_CYCLE);
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for show low power \n ");
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWER);
            count++;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:low power event report less than 5 times!\n");
        }
    }
    else
    {
        count = 0;
    }
}

/*******************************************************************
Function:      chg_detect_batt_volt_for_shutoff
Description:   电池电量格数判断设置
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_detect_batt_volt_for_shutoff(void)
{
    static uint32_t timer_flag = FALSE;

    /*烧片版本不做处理*/
    /*可维可测:低电关机/无电池开机使能*/
    if( TRUE == chg_is_ftm_mode() \
        || TRUE == chg_is_low_battery_poweroff_disable() \
        || TRUE == chg_is_no_battery_powerup_enable() )
    {
        return;
    }
    /*电池低电关机检测*/
    if(BATT_VOLT_POWER_OFF_THR > chg_real_info.battery_volt)
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:PowerOffThreshold=%d\n",BATT_VOLT_POWER_OFF_THR);
#if defined(BSP_CONFIG_BOARD_E5_E5770s)
        /*开机模式且充电器不在位*/
        if( FALSE == chg_is_powdown_charging() && (TRUE == is_chg_charger_removed()) )
#else
        if( FALSE == chg_is_powdown_charging() )/*开机模式*/
#endif
        {
            /*todo 上报APP执行低电关机操作*/
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:send MSG to app for low battery power off  \n ");
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
            /*对于支持库仑计的产品，需要根据电压门限来关机，同时需要coul的soc值平滑到2%*/
            if((FALSE == timer_flag) && (BATT_CAPACITY_SHUTOFF >= chg_get_sys_batt_capacity()))
            {
                timer_flag = TRUE;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWEROFF);
                /*启动定时器，回调函数为关机函数，定时时长为15秒；*/
                chg_print_level_message(CHG_MSG_ERR, \
                "CHG_STM:begin 15s timer for low battery power off \n ");
                chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION,\
                chg_set_power_off, DRV_SHUTDOWN_LOW_BATTERY);
            }
#else

            /*开机模式电池电压低于3.45V给应用上报电量为0，关机充电模式不上报防止反复开关机*/
            chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)BAT_LOW_POWEROFF);
#if ( FEATURE_ON == MBB_MLOG )
            mlog_print(MLOG_CHG, mlog_lv_warn, "Battery volt too low, report system down message!!!\n");
#endif
            if(FALSE == timer_flag)
            {
                timer_flag = TRUE;
                /*启动定时器，回调函数为关机函数，定时时长为15秒；*/
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:begin 15s timer for low battery power off \n ");
#if ( FEATURE_ON == MBB_MLOG )
                mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:begin 15s timer for low battery power off!!!\n");
#endif
                chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, chg_set_power_off, DRV_SHUTDOWN_LOW_BATTERY);
            }
#endif
        }
        else/*关机模式*/
        {
            /*关机模式下的电压低于关机门限，不做处理，本身为关机状态*/
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:power off voltage but in powdown_charging do nothing!\n");
        }
    }
}
/*******************************************************************
Function:      chg_detect_batt_chg_for_shutoff
Description:   关机充电情况下外电源移除，进行关机处理。
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_detect_batt_chg_for_shutoff(void)
{
    static int32_t charge_remove_check_count = 0;

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if(TRUE == chg_is_powdown_charging() && gpio_get_value(EXTCHG_OTG_DET_GPIO))
#else
    if(TRUE == chg_is_powdown_charging())
#endif
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        /*有无线充电功能时，关机充电如果连续两次检测到无线充电器和无线充电器均不在位则关机*/
        if((FALSE == chg_is_charger_present()) && (FALSE == chg_stm_get_wireless_online_st()))
#else
        /*无无线充电功能时，关机充电如果连续两次检测到无线充电和无线充电器均不在位则关机*/
        if(FALSE == chg_is_charger_present())
#endif/*MBB_CHG_WIRELESS*/
        {
            if(CHARGE_REMOVE_CHECK_MAX <= charge_remove_check_count)
            {
                chg_print_level_message(CHG_MSG_ERR,"CHG_STM:POWER OFF FOR CHARGER REMOVE !\n ");
                chg_set_power_off(DRV_SHUTDOWN_CHARGE_REMOVE);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO,"CHG_STM:charge_remove_check_count = %d\n ",charge_remove_check_count);
                charge_remove_check_count++;
            }
        }
        else
        {
            charge_remove_check_count = 0;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM: power on chging chgr remove do noting!\n");
    }
}


void chg_set_revise_value(int32_t value)
{
    chg_real_info.volt_revise_value = value;
}

int32_t chg_get_revise_value(void)
{
    return chg_real_info.volt_revise_value;
}

#if (MBB_CHG_EXTCHG == FEATURE_ON)


int32_t chg_get_extchg_revise(void)
{
    int32_t revise_val = 0;

    if(chg_get_revise_value() <= 0)
    {
        return revise_val;
    }
    /*放电电补偿上限*/
    if(chg_get_revise_value() < EXTCHG_BATT_VOLT_REVISE_LIMIT)
    {
        revise_val = chg_get_revise_value();
    }
    else
    {
        revise_val = EXTCHG_BATT_VOLT_REVISE_LIMIT;
    }

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg revise_val = %d\n", revise_val);
    return revise_val;
}

boolean is_need_extchg_revise(void)
{
    static boolean is_need_revise = FALSE;
    int32_t tmp_volt_revise_val = 0;
    int32_t batt_volt_average = chg_get_sys_batt_volt();
    int32_t batt_volt = chg_get_batt_volt_value();

    if(TRUE == is_5577cs_falg)
    {
        return FALSE;
    }

    /*电池端口电压低于3.35v则不再补偿*/
    if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_volt=%d below 3.35V no need revise!\n", batt_volt);
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
    }
    if((!is_need_revise) || (g_extchg_revise_count < CHG_BATT_VOLT_REVISE_COUNT))
    {
        if(FALSE == g_exchg_enable_flag)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stop extchg,no need revise!\n");
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
        if((batt_volt_average - batt_volt) > CHG_BATT_VOLT_REVISE_WINDOW)
        {
            batt_volt = chg_get_batt_volt_value();
            if((batt_volt_average - batt_volt) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,start ext revise!\n",
                    (batt_volt_average - batt_volt));
                tmp_volt_revise_val = batt_volt_average - batt_volt;
                chg_set_revise_value(tmp_volt_revise_val);
                is_need_revise = TRUE;
            }
        }
        g_extchg_revise_count++ ;
    }
    else
    {
        if(FALSE == g_exchg_enable_flag)
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:stop extchg,no need revise!\n");
            g_extchg_revise_count = 0;
            is_need_revise = FALSE;
        }
        else
        {
            /*对外充电线在位但是没有外接被充电设备虚低消除*/
            if((batt_volt_average - batt_volt) <= CHG_BATT_VOLT_REVISE_WINDOW)
            {
                batt_volt = chg_get_batt_volt_value();
                if((batt_volt_average - batt_volt) <= CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,stop ext revise!\n",
                        (batt_volt - batt_volt_average));
                    g_extchg_revise_count = 0;
                    is_need_revise = FALSE;
                }
            }
            /*虚低更新*/
            else
            {
                int32_t temp_revise_val = chg_get_revise_value();
                if(abs((batt_volt_average - batt_volt) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    batt_volt = chg_get_batt_volt_value();
                    if(abs((batt_volt_average - batt_volt) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                    {
                        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:extchg batt volt change=%d,dynamic ext revise!\n",
                            (batt_volt_average - batt_volt));
                        tmp_volt_revise_val = batt_volt_average - batt_volt;
                        chg_set_revise_value(tmp_volt_revise_val);

                    }
                }
            }
        }
    }
    return  is_need_revise;
}

#endif/*defined(MBB_CHG_EXTCHG*/

int32_t chg_get_chg_revise(void)
{
    int32_t revise_val = 0;

    if(chg_get_revise_value() <= 0)
    {
        return revise_val;
    }

    /*充电补偿上限*/
    if(chg_get_revise_value() < CHG_BATT_VOLT_REVISE_LIMIT)
    {
        revise_val = 0 - chg_get_revise_value();
    }
    else
    {
        revise_val = 0 - CHG_BATT_VOLT_REVISE_LIMIT;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg revise_val = %d\n", revise_val);
    return revise_val;
}


boolean is_need_chg_revise(void)
{
    static boolean is_need_revise = FALSE;
    int32_t tmp_volt_revise_val = 0;
    int32_t batt_volt_average = chg_get_sys_batt_volt();
    int32_t batt_volt = chg_get_batt_volt_value();

    /*电池端口电压高于4.15v则不再补偿*/
    if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_volt = %d\n", batt_volt);
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
    }
    if((!is_need_revise) || (g_chg_revise_count < CHG_BATT_VOLT_REVISE_COUNT))
    {
        if(FALSE == chg_get_charging_status())
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stop charging,no need revise!\n");
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
            return  is_need_revise;
        }
        if((batt_volt - batt_volt_average) > CHG_BATT_VOLT_REVISE_WINDOW)
        {
            batt_volt = chg_get_batt_volt_value();
            if((batt_volt - batt_volt_average) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,start revise!\n",
                    (batt_volt - batt_volt_average));
                tmp_volt_revise_val = batt_volt - batt_volt_average;
                chg_set_revise_value(tmp_volt_revise_val);
                is_need_revise = TRUE;
            }
        }
        g_chg_revise_count++ ;
    }
    else
    {
        if(FALSE == chg_get_charging_status())
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:stop charging ,stop revise!\n",
                (batt_volt - batt_volt_average));
            g_chg_revise_count = 0;
            is_need_revise = FALSE;
        }
        else
        {
            int32_t temp_revise_val = chg_get_revise_value ();
            if(abs((batt_volt - batt_volt_average) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
            {
                batt_volt = chg_get_batt_volt_value();
                if(abs((batt_volt - batt_volt_average) - temp_revise_val) > CHG_BATT_VOLT_REVISE_WINDOW)
                {
                    /*动态更新补偿值*/
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:batt volt change = %d,dynamic revise!\n",
                    (batt_volt - batt_volt_average));
                    tmp_volt_revise_val = batt_volt - batt_volt_average;
                    chg_set_revise_value(tmp_volt_revise_val);
                }
            }
        }
    }
    return  is_need_revise;
}


int32_t chg_batt_volt_revise(int32_t batt_volt)
{
    int32_t revised_batt_volt = 0;

    if(TRUE == is_need_chg_revise())
    {
        revised_batt_volt = batt_volt + chg_get_chg_revise();
        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:need chg revise batt_volt_revised=%d\n",revised_batt_volt);
    }

#if (MBB_CHG_EXTCHG == FEATURE_ON)
    else if(TRUE == is_need_extchg_revise())
    {
        revised_batt_volt = batt_volt + chg_get_extchg_revise();
        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:need extchg revise batt_volt_revised=%d\n",revised_batt_volt);
    }
#endif /*defined(MBB_CHG_EXTCHG)*/
    else
    {
        revised_batt_volt = batt_volt;
        chg_set_revise_value(0);

        chg_print_level_message(CHG_MSG_DEBUG, "VOLTAGE_COMPENCATE:no need revise batt_volt_revised=%d\n",revised_batt_volt);
    }
    chg_print_level_message(CHG_MSG_INFO, "VOLTAGE_COMPENCATE:batt_volt=%d,revised_value=%d,batt_volt_revised=%d\n",\
                           batt_volt,chg_get_revise_value(),revised_batt_volt);

    return revised_batt_volt;
}
#ifdef CONFIG_COUL
#define CURRENT_RECORD_NUM    (20)/*电流采集次数*/
#define CURRENT_LIMIT_UP_MAX   (2000)/*电流采集最大值边界值*/
#define CURRENT_LIMIT_UP_MIN    (-2000)/*电流采集最大值边界值*/
#define CURRENT_LIMIT_DOWN_MAX    (50)/*电流采集最小值边界值*/
#define CURRENT_LIMIT_DOWN_MIN    (-50)/*电流采集最小值边界值*/
#define CURRENT_UNIT_MA2UA    (1000)/*单位换算*/
#define R_BATT    (110)/*内阻大小需要根据电池容量和厂家来区分*/

int32_t chg_get_average_batt_current(void)
{
    int32_t i, used, current_ma, totalcur;

    used = 0;
    totalcur = 0;
    current_ma = 0;
    for (i = 0; i < CURRENT_RECORD_NUM; i++)
    {
        current_ma = bsp_coul_current_before(i);
        if((current_ma > CURRENT_LIMIT_UP_MAX) || (current_ma < CURRENT_LIMIT_UP_MIN))
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:invalid current = %d ma\n", current_ma);
            continue;
        }
        totalcur += current_ma;
        used++;
    }
    if(used > 0)
    {
        current_ma = totalcur / used;
    }
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:current_ma = %d\n", current_ma);
    return current_ma;
}

/*充电电流为正，有虚高；放电电流为负，有虚低*/
int32_t chg_batt_volt_coul_revise(int32_t batt_volt)
{
    int32_t current_ma = 0,revise_val = 0;
    int32_t revised_batt_volt = 0;

    revised_batt_volt = batt_volt;
    /*电池端口电压高于4.15v则不再补偿*/
    if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt > CHG_BATT_VOLT_REVISE_LIMIT_UP)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:wired chg revise_val = %d\n", revise_val);
            return revised_batt_volt;
        }
    }
    /*电池端口电压低于3.35v则不再补偿*/
    if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
    {
        batt_volt = chg_get_batt_volt_value();
        if(batt_volt < CHG_BATT_VOLT_REVISE_LIMIT_DOWN)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:wired chg revise_val = %d\n", revise_val);
            return revised_batt_volt;
        }
    }
    current_ma = chg_get_average_batt_current();
    if((CURRENT_LIMIT_DOWN_MIN < current_ma) && (CURRENT_LIMIT_DOWN_MAX > current_ma))
    {
        return revised_batt_volt;
    }
    revise_val = (current_ma * R_BATT) / CURRENT_UNIT_MA2UA;/*单位转换，结果转换为mV*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:revise_val = %d\n", revise_val);
    revised_batt_volt = batt_volt - revise_val;
    return revised_batt_volt;
}
#endif
/*******************************************************************
Function:      chg_huawei_se_volt
Description:   电池电压补偿处理函数，需要根据具体的测试结果设计补偿函数，
               此处只提供接口，具体实现取决于产品实际测试结果。
Input:         平滑后的电压值
Return:        补偿后的电压值
*******************************************************************/
int32_t chg_huawei_set_volt(int32_t bat_volt)
{
#if defined(BSP_CONFIG_BOARD_E5_E5578)
    uint32_t batt_id = chg_get_batt_id();
    switch(batt_id)
    {
        case CHG_BATT_ID_FEIMAOTUI:
        {
            bat_volt = bat_volt + CALI_VLOT_FEIMAOTUI ;
            break;             
        }
        case CHG_BATT_ID_XINGWANGDA:
        {
            bat_volt = bat_volt + CALI_VLOT_XINGWANGDA ;
            break;
        }
        case CHG_BATT_ID_LISHEN:
        default:
            break;
        }
#elif defined(BSP_CONFIG_BOARD_E5573S_856) || defined(BSP_CONFIG_BOARD_E5573S_853) \
    || defined(BSP_CONFIG_BOARD_E5573S_607) || defined(BSP_CONFIG_BOARD_E5573CS) \
    || defined(BSP_CONFIG_BOARD_E5573BS) || defined(BSP_CONFIG_BOARD_E5573CS_933)\
    || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
    uint32_t batt_id = chg_get_batt_id();
    switch(batt_id)
    {
        case CHG_BATT_ID_LISHEN:
        {
            bat_volt = bat_volt + CALI_VLOT_LISHEN ;
            break;
        }
         case CHG_BATT_ID_FEIMAOTUI:
        {
            bat_volt = bat_volt + CALI_VLOT_FEIMAOTUI ;
            break;
        }
        case CHG_BATT_ID_XINGWANGDA:
        {
            bat_volt = bat_volt + CALI_VLOT_XINGWANGDA ;
            break;
        }
        default:
            break;
        }
#endif
    return bat_volt;
}
/*******************************************************************
Function:      chg_poll_bat_level
Description:   电池电压值加工处理的流程控制函数。
Calls:         chg_get_parameter_level-------------采集电压值值接口函数；
               chg_calc_average_volt_value--------电压平滑算法处理函数；
               huawei_set_temp------------------------电压补偿函数
Data Accessed: 无
Data Updated:  全局变量的温度值
Input:         无
Output:        无
Return:        无
*******************************************************************/
void chg_poll_bat_level(void)
{
    int32_t new_one_batt_volt = 0;
    int32_t new_batt_volt = 0;
    static uint32_t init_flag = FALSE;


    /*坏电池检测处理函数*/
    chg_batt_error_detect_volt();

    /*调用函数chg_get_parameter_level，读取电池电压*/
    new_one_batt_volt = chg_get_batt_volt_value();
    /*根据电池电压ID差异对电池电压进行补偿*/
    new_one_batt_volt = chg_huawei_set_volt(new_one_batt_volt);
    /*充电或者对外充电时，需要对电池电压进行虚高或虚低补偿*/
#if CHG_BATT_REVISE
#if CHG_USE_COUL_REVISE
#ifdef CONFIG_COUL
    new_one_batt_volt = chg_batt_volt_coul_revise(new_one_batt_volt);
#endif
#else
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#else
    new_one_batt_volt = chg_batt_volt_revise(new_one_batt_volt);
#endif
#endif
#endif
    /*保存电池单次采集电压到全局变量*/
    chg_real_info.battery_one_volt = new_one_batt_volt;

    /*电池平滑电压初始化为首次采集值*/
    if(FALSE == init_flag)
    {
        chg_real_info.battery_volt = chg_real_info.battery_one_volt;
        chg_real_info.bat_volt_lvl = BATT_LEVEL_MAX;
        init_flag = TRUE;
    }

    /*对电池电压进行平滑算法处理*/
    new_batt_volt = chg_calc_average_volt_value(new_one_batt_volt);

    /*保存电池电压平滑值到全局变量*/
    chg_real_info.battery_volt = new_batt_volt;

    /*电池电量格数处理*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    chg_set_battery_level();
#else
    chg_set_battery_volt_level();
#endif

    /* log打印接口，可维可测讨论内容*/
    chg_print_level_message(CHG_MSG_DEBUG, "*********** CHG BAT VOLT PARA: chg_poll_bat_level  begin **********\n");

    /*打印充电温保护使能开关*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg temp protect enable flag = %d\n", CHG_TEMP_PROTECT_ENABLE);

    /*开机电压门限*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:shutoff_batt_volt_thr= %d\n", BATT_VOLT_POWER_ON_THR);

    /*打印电池单次采集电压*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_one_volt = %d\n", chg_real_info.battery_one_volt);
    /*打印电池电压平滑值*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.battery_volt = %d\n", chg_real_info.battery_volt);
    /*打印打印电池电量格数*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:chg_real_info.bat_volt_level = %d\n", chg_real_info.bat_volt_lvl);
    chg_print_level_message(CHG_MSG_DEBUG, "*********** CHG BAT VOLT PARA: chg_poll_bat_level  end **********\n");

    /*检测充电器移除关机*/
    chg_detect_batt_chg_for_shutoff();
    chg_detect_batt_volt_for_shutoff();
}
#if (MBB_CHG_COULOMETER == FEATURE_ON)
/*******************************************************************
Function:      chg_poll_batt_soc
Description:   update the battery soc,if changed,report to app
Calls:         chg main task
Input:         none
Output:        none
Return:        none
*******************************************************************/
void chg_poll_batt_soc(void)
{
    int32_t input_soc = 0;
    int32_t last_soc = chg_get_sys_batt_capacity();

    input_soc = smartstar_battery_capacity();
    if(input_soc - last_soc >= 1)
    {
        input_soc = last_soc + 1;
    }
    else if(last_soc - input_soc >= 1)
    {
        input_soc = last_soc - 1;
    }
    else
    {
    }
    
    chg_set_sys_batt_capacity(input_soc);
}
#endif

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*******************************************************************
Function:      chg_notify_app_charging_state
Description:   notify the battery charging state
Calls:         chg main task
Input:         batt_state
Output:        none
Return:        success:0 fail:-1
*******************************************************************/
int32_t chg_notify_app_charging_state(int32_t batt_state)
{
    if (NULL == g_chip)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG:NULL pointer,batt_state=%d\n ",batt_state);        
        return -1;
    }
    g_chip->bat_stat = batt_state;
    power_supply_changed(&g_chip->bat);
    return 0;
}
/*******************************************************************
Function:      chg_poll_batt_charging_state_for_coul
Description:   update the battery charging state,if changed,notify coul
Calls:         chg main task
Input:         none
Output:        none
Return:        none
*******************************************************************/
void chg_poll_batt_charging_state_for_coul(void)
{
    static CHG_COUL_EVENT_TYPE charge_status = VCHRG_STOP_CHARGING_EVENT;
    chg_stm_state_type cur_stm_state;
    chg_print_level_message(CHG_MSG_DEBUG,"CHG:charging old state=%d\n ",charge_status);
    switch (charge_status)
    {
        case VCHRG_NOT_CHARGING_EVENT:
            if (chg_is_IC_charging())
            {
                hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                charge_status = VCHRG_START_CHARGING_EVENT;
            }
            else
            {            
                if(!is_chg_charger_removed())
                {   /* 未充电但是充电器在位 */
                    #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
                    if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state()) 
                    || (CHG_STOP_COMPLETE == chg_get_stop_charging_reason()))
                    {
                        hisi_coul_charger_event_rcv(VCHRG_CHARGE_DONE_EVENT);
                        charge_status = VCHRG_CHARGE_DONE_EVENT;
                    }
                    #else
                     /*IC满电停充，但是soc不是100%,需要模拟一次充停过程*/
                    if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                    && (FALSE == chg_is_batt_full()))
                    {
                        chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                        hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                        charge_status = VCHRG_START_CHARGING_EVENT;
                    }
                    #endif
                }
            }
 
            break;
        case VCHRG_START_CHARGING_EVENT:
            if (!chg_is_IC_charging())
            {
                if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state()) 
                || (CHG_STOP_COMPLETE == chg_get_stop_charging_reason()))
                {
                    hisi_coul_charger_event_rcv(VCHRG_CHARGE_DONE_EVENT);
                    charge_status = VCHRG_CHARGE_DONE_EVENT;
                }
                else
                {
                    hisi_coul_charger_event_rcv(VCHRG_STOP_CHARGING_EVENT);
                    charge_status = VCHRG_STOP_CHARGING_EVENT;
                }
            }
            break;
        case VCHRG_STOP_CHARGING_EVENT:
            if (chg_is_IC_charging())
            {
                hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                charge_status = VCHRG_START_CHARGING_EVENT;
            }
            else
            {
                hisi_coul_charger_event_rcv(VCHRG_NOT_CHARGING_EVENT);
                charge_status = VCHRG_NOT_CHARGING_EVENT;
            }
            break;
        case VCHRG_CHARGE_DONE_EVENT:
            if(is_chg_charger_removed())
            {   /* 满充下拔掉充电器  */
                hisi_coul_charger_event_rcv(VCHRG_NOT_CHARGING_EVENT);
                charge_status = VCHRG_NOT_CHARGING_EVENT;                
            }
            else
            {
                if (chg_is_IC_charging())
                {   /* 满充插着充电器，有充电切到充电态，没有保持在满充态  */
                    hisi_coul_charger_event_rcv(VCHRG_START_CHARGING_EVENT);
                    charge_status = VCHRG_START_CHARGING_EVENT;
                }
                #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
                #else
                /*IC满电停充，但是soc不是100%,需要显示充电动画，平滑百分比*/
                else if ((CHG_STM_MAINT_ST == chg_stm_get_cur_state())
                    && (FALSE == chg_is_batt_full()))
                {
                    chg_notify_app_charging_state(POWER_SUPPLY_STATUS_CHARGING);
                }
                #endif
            }
            break;
        default:
            break;
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG:charging new state=%d\n ",charge_status);
}

#endif
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
/**********************************************************************
函 数 名  :  chg_tbat_status_get
功能描述  :  TBAT AT^TCHRENABLE?是否需要补电

输入参数  : 无
输出参数  : 无
返 回 值  : 1:需要补电
            0:不需要补电
注意事项  : 无
***********************************************************************/
int32_t chg_tbat_status_get(void)
{
    int32_t tbat_v = 0;
    int32_t ret = FALSE;
    (void)chg_set_charge_enable(FALSE);
    chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
    if(TRUE == chg_is_IC_charging())
    {
        (void)chg_set_charge_enable(FALSE);
        chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
    }

    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();  /* 该接口以及数据类型通过ADC头文件获取 */

    /*判断是否满足放电或补电，连续读3次*/
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if((tbat_v > tbatt_dischg_volt_threshold) 
        || (tbat_v < tbatt_supply_volt_threshold))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v > tbatt_dischg_volt_threshold) 
            || (tbat_v < tbatt_supply_volt_threshold))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v > tbatt_dischg_volt_threshold) 
                || (tbat_v < tbatt_supply_volt_threshold))
            {
                ret = TRUE;
            }
        }
    }
#else
    if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v > TBAT_DISCHG_VOLT) || (tbat_v < TBAT_SUPPLY_VOLT))
            {
                ret = TRUE;
            }
        }
    }
#endif
    if(TRUE == chg_get_charging_status())
    {
        (void)chg_set_charge_enable(TRUE);
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY GET VBAT VALUE:%d\n ", tbat_v);
    return ret;
}

/**********************************************************************
函 数 名  :  chg_is_sply_finish
功能描述  :  判断补电是否完成

输入参数  : 无
输出参数  : 无
返 回 值  : 1:完成补电
            0:补电未完成
注意事项  : 无
***********************************************************************/
boolean chg_is_sply_finish(void)
{
    uint32_t tbat_v = 0;

    /*读取电池电量*/
    tbat_v = chg_get_batt_volt_value();      /* 该接口以及数据类型通过ADC头文件获取 */

    /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY LEVEL:%d\n ",tbat_v);

    /*判断是否满足放电或补电，连续读3次*/
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if((tbat_v < tbatt_dischg_stop_threshold) 
        && (tbat_v > tbatt_supply_stop_threshold))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v < tbatt_dischg_stop_threshold) 
            && (tbat_v > tbatt_supply_stop_threshold))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v < tbatt_dischg_stop_threshold) 
                && (tbat_v > tbatt_supply_stop_threshold))
            {
                if( TRUE == fact_release_flag )
                {
                    chg_set_fact_release_mode(FALSE);
                    fact_release_flag = FALSE;
                }
                chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
                /*通知应用补电完成*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,
                    (uint32_t)CHG_EVENT_NONEED_CARE);

                /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
                chg_print_level_message(CHG_MSG_DEBUG,
                    "CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ", tbat_v);

                chg_sply_unlock();

                return TRUE;
            }
        }
    }
#else
    if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
    {
        tbat_v = chg_get_batt_volt_value();
        if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
        {
            tbat_v = chg_get_batt_volt_value();
            if((tbat_v < TBAT_DISCHG_STOP_VOLT) && (tbat_v > TBAT_SUPPLY_STOP_VOLT))
            {
                if( TRUE == fact_release_flag )
                {
                    chg_set_fact_release_mode(FALSE);
                    fact_release_flag = FALSE;
                }
                chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
                /*通知应用补电完成*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);

                /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY SPLY FINISH LEVEL:%d\n ", tbat_v);

                chg_sply_unlock();

                return TRUE;
            }
        }
    }
#endif

    return FALSE;
}

/**********************************************************************
函 数 名  :  chg_batt_suply_proc
功能描述  :  电池补电任务

输入参数  : 无
输出参数  : 无
返 回 值  : 1:不需要补电
            0:补电成功
注意事项  : 无
***********************************************************************/

int32_t chg_batt_supply_proc(void *task_data)
{
    int32_t tbat_v = 0;
    uint32_t wait_idx = 0;
    uint32_t  tc_on = TBAT_SUPLY_DELAY_COUNTER;
    uint32_t  tc_off = TBAT_STOP_DELAY_COUNTER;

    if ( NULL == task_data )
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:supply_proc task_data is NULL !\n");
    }

    /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:START DISCHARGE OR CHARGE\n ");

    /* 放电删除充电任务*/
    chg_task_delete();

    chg_sply_lock();
    chg_set_cur_chg_mode(CHG_SUPPLY_MODE);

    chg_stm_set_cur_state(CHG_STM_MAX_ST);

    chg_stm_state_info.bat_stat_type = POWER_SUPPLY_STATUS_NEED_SUPPLY;
    /*通知应用需要补电*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY,(uint32_t)CHG_EVENT_NONEED_CARE);

    do{
        tbat_v = chg_get_batt_volt_value();
        /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CURRENT LEVEL:%d\n ",tbat_v);

        /*如果电池电量大于放电阈值*/
        if(tbat_v > TBAT_DISCHG_STOP_VOLT)
        {
            /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY DISCHARGING BEGIN\n");

            (void)chg_set_charge_enable(FALSE);
            chg_set_suspend_mode(TRUE);
            chg_set_fact_release_mode(TRUE);
            fact_release_flag = TRUE;
            for (wait_idx = 0; wait_idx < tc_on; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            /*脉冲方式*/

            chg_set_suspend_mode(FALSE);

            for (wait_idx = 0; wait_idx < tc_off; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY DISCHARGING END\n");
        }

        else if(tbat_v < TBAT_SUPPLY_STOP_VOLT) /*充电*/
        {
            /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING BEGIN\n");

            //补电参数设置为fast充电参数
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
            (void)chg_set_charge_enable(TRUE);
            for (wait_idx = 0; wait_idx < tc_on; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }

            /*脉冲方式*/
            (void)chg_set_charge_enable(FALSE);
            for (wait_idx = 0; wait_idx < tc_off; wait_idx++)
            {
                chg_delay_ms(CHG_DELAY_COUNTER_SIZE);
            }
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING END\n");

        }
        else
        {
            /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_SUP:BATTERY CHARGING OR DISCHARGING EXIT:%d\n ",tbat_v);
        }
    }while(!chg_is_sply_finish());
    /* 打开使能 */
    (void)chg_set_charge_enable(FALSE);

#if (MBB_CHG_POWER_SUPPLY == FEATURE_OFF)
    /*LCD状态更新*/
    chg_display_interface( CHG_DISP_OK );
#endif/*MBB_CHG_POWER_SUPPLY*/
    tbat_v = chg_get_batt_volt_value();
    /* log打印接口，可维可测讨论内容，此处暂定为该接口名，后续接口确定后再做修改 */
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:BATTERY EXIT LEVEL:%d\n ",tbat_v);
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:BATTERY CHARGE or DISCHARGER FINISH\n ");

    return TBAT_SUPPLY_CURR_SUCCESS;
}

/**********************************************************************
函 数 名  :  chg_tbat_chg_sply
功能描述  :  TBAT AT^TCHRENABLE=4补电实现
                        补电成功LCD显示成功图标
输入参数  : 无
输出参数  : 无
返 回 值  : 1:TBAT_SUPPLY_CURR_SUCCESS 补电成功
            2:TBAT_NO_NEED_SUPPLY_CURR 不需要补电
注意事项  : 无
***********************************************************************/
int32_t chg_tbat_chg_sply(void)
{
    /* 调用归一化接口启动补电任务**/
    chg_sply_task_create();
/***************Note:平台相关代码，根据平台按需要添加，有的平台如V7R1需要
              移植人员根据需要，添加或者移除下边函数调用***************************/

#if (MBB_CHG_POWER_SUPPLY == FEATURE_OFF)
    /*补电开始显示补电未完成图标*/
    chg_display_interface( CHG_DISP_FAIL );
#endif/*MBB_CHG_POWER_SUPPLY*/
    return TBAT_SUPPLY_CURR_SUCCESS;
}
#endif /* MBB_CHG_COMPENSATE */

void chg_get_bat_temp(void)
{
    printk("bat_sys_temp is %d\r\n",chg_real_info.battery_temp);
}

void chg_get_bat_volt(void)
{
    printk("bat_sys_volt is %d\r\n",chg_real_info.battery_volt);
}
/**********************************************************************
函 数 名  :chg_get_cur_batt_volt
功能描述  : 获取当前电池实时电压的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池实时电压
注意事项  : 无。
***********************************************************************/
static int32_t chg_get_cur_batt_volt(void)
{
    if( 0xFFFF == chg_real_info.battery_one_volt )
    {
        chg_real_info.battery_one_volt = chg_get_batt_volt_value();
    }

    return chg_real_info.battery_one_volt;
}
/**********************************************************************
函 数 名  :chg_get_sys_batt_volt
功能描述  : 获取当前电池平滑电压的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池平滑电压
注意事项  : 无。
***********************************************************************/

int32_t chg_get_sys_batt_volt(void)
{
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if (FALSE == is_chg_charger_removed())
    {
        if( 0xFFFF == chg_real_info.battery_volt )
        {
            chg_real_info.battery_volt = chg_get_batt_volt_value();
        }
        return chg_real_info.battery_volt;
    }
    else
    {
        if(0xFFFF == battery_voltage)
        {
            battery_voltage = chg_get_batt_volt_value();
        }
        return battery_voltage;
    }
#else
    if( 0xFFFF == chg_real_info.battery_volt )
    {
        chg_real_info.battery_volt = chg_get_batt_volt_value();
    }
    return chg_real_info.battery_volt;
#endif
}
/**********************************************************************
函 数 名  :chg_get_cur_batt_temp
功能描述  : 获取当前电池实时温度的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池实时温度
注意事项  : 无。
***********************************************************************/
int32_t chg_get_cur_batt_temp(void)
{
    if( 0xFFFF == chg_real_info.battery_one_temp )
    {
        chg_real_info.battery_one_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    }

    return chg_real_info.battery_one_temp;
}
/**********************************************************************
函 数 名  :chg_get_sys_batt_temp
功能描述  : 获取当前电池实时温度的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池平滑温度
注意事项  : 无。
***********************************************************************/
int32_t chg_get_sys_batt_temp(void)
{
    if( 0xFFFF == chg_real_info.battery_temp )
    {
        chg_real_info.battery_temp = chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
    }

    return chg_real_info.battery_temp;
}

/**********************************************************************
函 数 名  :chg_set_sys_batt_capacity
功能描述  : 设置当前电池电量百分比的接口函数
输入参数  : capacity :电池电量百分比
输出参数  : 无。
返 回 值  : 电池电量百分比
注意事项  : 无。
***********************************************************************/
void chg_set_sys_batt_capacity(int32_t capacity)
{
    /*电量发生变化时才进行上报如果上次上报电量跟本次电量不同时才给应用上报*/
    if(chg_real_info.bat_capacity != capacity)
    {
        chg_real_info.bat_capacity = capacity;
        /*电池电量百分比发生变化通知应用查询*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_batt_capacity=%d\n",capacity);
    }
}

/**********************************************************************
函 数 名  :chg_get_sys_batt_capacity
功能描述  : 获取当前电池电量百分比的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量百分比
注意事项  : 无。
***********************************************************************/
int32_t chg_get_sys_batt_capacity(void)
{
    if(chg_is_no_battery_powerup_enable())
    {
        return BATT_CAPACITY_FULL;
    }
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    if(0 == chg_real_info.bat_capacity)
    {
        return smartstar_battery_capacity();
    }
#endif
    return chg_real_info.bat_capacity;
}
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/**********************************************************************
函 数 名  :chg_is_batt_in_state_of_emergency
功能描述  : 查询电池是否在紧急需要关机状态
输入参数  : 无。
输出参数  : 无。
返 回 值  : TRUE:yes，FALSE:no
注意事项  : 无。
***********************************************************************/
boolean chg_is_batt_in_state_of_emergency(void)
{
    //仅在开机状态下判断是否需要关机
    return (!chg_is_powdown_charging() && BATT_VOLT_POWER_OFF_THR > chg_get_sys_batt_volt());
}
#endif

/**********************************************************************
函 数 名  :chg_set_batt_time_to_full
功能描述  : 设置预测的剩余充电时间
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量距充满还有多长时间
注意事项  : 无。
***********************************************************************/
void chg_set_batt_time_to_full(int32_t time_to_full)
{
    chg_real_info.bat_time_to_full = time_to_full;
    /*通知应用剩余充电时间发生变化*/
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_batt_time_to_full=%d\n",time_to_full);
}

/**********************************************************************
函 数 名  :chg_get_batt_time_to_full
功能描述  : 获取电池电量距充满还有多长时间
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量距充满还有多长时间
注意事项  : 无。
***********************************************************************/
int32_t chg_get_batt_time_to_full(void)
{
    return chg_real_info.bat_time_to_full;
}

#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
/**********************************************************************
函 数 名  :chg_get_batt_current_average
功能描述  : 获取电池平均电流
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池的平均电流
注意事项  : 无。
***********************************************************************/
int32_t chg_get_batt_current_average(void)
{
    return chg_real_info.bat_current_avg;
}
#endif

/**********************************************************************
函 数 名  : chg_is_batt_full
功能描述  : 判断电池是否满电
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池满电

            0:电池非满电
注意事项  : 无。
***********************************************************************/
boolean chg_is_batt_full(void)
{
/*支持库仑计的产品，满电门限定义为95%*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return (BATT_CAPACITY_FULL == chg_get_sys_batt_capacity());
#else
    return (chg_get_sys_batt_volt() >= BATT_CHG_TEMP_MAINT_THR);
#endif
}

/**********************************************************************
函 数 名  : chg_is_batt_full_for_start
功能描述  : 判断电池电压是否满足开始充电条件
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池满电，不需要启动充电
            0:电池非满电，需要启动充电
注意事项  : 无。
***********************************************************************/
boolean chg_is_batt_full_for_start(void)
{
/*支持库仑计的产品，满电门限定义为95%*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    return (BATT_CAPACITY_FULL == chg_get_sys_batt_capacity());
#else
    return (chg_get_sys_batt_volt() >= BATT_CHG_FIRST_MAINT_THR);
#endif
}
/**********************************************************************
函 数 名  : chg_is_batt_full
功能描述  : 判断电池是否需要复充
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池需要复充
            0:电池不需要复充
注意事项  : 无。
***********************************************************************/
static boolean chg_is_batt_need_rechg(void)
{
/*充电器在位时，只要百分比低于100%，则启动充电*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*针对使用库仑计产品*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    /*电池高温状态，支持库仑计的使用电压作为复充门限判断*/
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_volt() < BATT_HIGH_TEMP_RECHARGE_THR);
    }
    if (TRUE == g_chg_batt_expand_change_normal_rechg_flag)
    {
        return (chg_get_sys_batt_volt() < BATT_NORMAL_TEMP_RECHARGE_THR);
    }
#endif/*FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT*/
    #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_capacity() <= BATT_CAPACITY_WARMRECHG);
    }
    else
    {
        return (chg_get_sys_batt_capacity() <= BATT_CAPACITY_RECHG);        
    }
    #else
    return (chg_get_sys_batt_capacity() <= BATT_CAPACITY_RECHG);
    #endif
#else
    if(TRUE == is_batttemp_in_warm_chg_area())
    {
        return (chg_get_sys_batt_volt() < BATT_HIGH_TEMP_RECHARGE_THR);
    }
    else
    {
        #if defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
        || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW)\
        || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
        if (CHG_BATT_ID_DEF == chg_get_batt_id())
        {
            return FALSE;
        }
        #endif
        return (chg_get_sys_batt_volt() < BATT_NORMAL_TEMP_RECHARGE_THR);        
    }
#endif
}

/**********************************************************************
函 数 名  : chg_is_batt_full
功能描述  : 判断电池是否过压
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池过压
            0:电池没有过压
注意事项  : 无。
***********************************************************************/
static boolean chg_is_batt_ovp(void)
{
    return (boolean)((chg_get_sys_batt_volt() >= BATT_CHG_OVER_VOLT_PROTECT_THR) 
            || (chg_get_cur_batt_volt() >= BATT_CHG_OVER_VOLT_PROTECT_ONE_THR));
}
/**********************************************************************
函 数 名  : chg_is_batt_full
功能描述  : 判断电池是否过温
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池没有过温
            0:电池过温
注意事项  : 无。
***********************************************************************/
static boolean chg_is_batt_temp_valid(void)
{
    return !chg_temp_protect_flag;
}

/**********************************************************************
函 数 名  : chg_get_batt_temp_state
功能描述  : 获取当前电池温度状态
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池温度状态
注意事项  : 无。
***********************************************************************/
TEMP_EVENT chg_get_batt_temp_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_temp_state=%d\n",chg_batt_temp_state);
    return chg_batt_temp_state;
}

/*****************************************************************************
 函 数 名  : chg_stm_set_chgr_type
 功能描述  : Update the current type of external charger.
 输入参数  : chgr_type  The charger type we gonna update to.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : chg_check_and_update_hw_param_per_chgr_type.
 CALL-WHOM : None.
 NOTICE    : Helper function.
*****************************************************************************/
void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type)
{
    /*1. Update the current external charger type.*/
    chg_stm_state_info.cur_chgr_type = chgr_type;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_chgr_type=%d\n",chgr_type);
}

/*****************************************************************************
 函 数 名  : chg_stm_get_chgr_type
 功能描述  : Get current external charger type:
             CHG_CHGR_UNKNOWN: Chgr type has not been got from USB module.
             CHG_WALL_CHGR   : Wall standard charger, which D+/D- was short.
             CHG_USB_HOST_PC : USB HOST PC or laptop or pad, etc.
             CHG_NONSTD_CHGR : D+/D- wasn't short and USB enumeration failed.
             CHG_CHGR_INVALID: External Charger invalid or absent.
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current external charger type.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
             The real chgr type checking would be done by USB module.
*****************************************************************************/
chg_chgr_type_t chg_stm_get_chgr_type(void)
{
    /*1. Return the current external charger type.*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_chgr_type=%d\n",chg_stm_state_info.cur_chgr_type);
    return chg_stm_state_info.cur_chgr_type;

}

boolean is_chg_charger_removed(void)
{
    boolean is_chgr_present      = chg_is_charger_present();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    boolean is_wireless_online   = chg_stm_get_wireless_online_st();
    if ((FALSE == is_chgr_present) && (FALSE == is_wireless_online))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed is_chgr_present=%d, is_wireless_online=%d.\n",
                    is_chgr_present,is_wireless_online);
        return TRUE;
    }
#else
    /*1. External charger removed, swith to battery only state.*/
    if (FALSE == is_chgr_present)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed is_chgr_present=%d !\n",is_chgr_present);
        return TRUE;
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:is_chg_charger_removed charger attached !\n");
        return FALSE;
    }
}

#if (MBB_CHG_WIRELESS == FEATURE_ON)
/*****************************************************************************
 函 数 名  : chg_stm_set_wireless_online_st
 功能描述  : get the wireless online status.
 输入参数  : ONLINE: 无线充电在位
             OFFLINE:无线充电不在位
 输出参数  : None
 返 回 值  : VOID.
*****************************************************************************/
void chg_stm_set_wireless_online_st(boolean online)
{
    chg_stm_state_info.wireless_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:set_wireless_online_st=%d\n",online);
}

/*****************************************************************************
 函 数 名  : chg_stm_get_wireless_online_st
 功能描述  : get the wireless online status.
 输入参数  :
 输出参数  : ONLINE: 无线充电在位
             OFFLINE:无线充电不在位
 返 回 值  : VOID.
*****************************************************************************/
boolean chg_stm_get_wireless_online_st(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:get_wireless_online_st=%d\n",\
                chg_stm_state_info.wireless_online_st);

    return chg_stm_state_info.wireless_online_st;
}
#endif/*MBB_CHG_WIRELESS*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
/*****************************************************************************
函 数 名  : chg_stm_set_extchg_online_st
功能描述  : get the extchg online status.
输入参数  : ONLINE: 无线充电在位
          OFFLINE:无线充电不在位
输出参数  : None
返 回 值  : VOID.
*****************************************************************************/
void chg_stm_set_extchg_online_st(boolean online)
{
    chg_stm_state_info.extchg_online_st = online;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_extchg_online_st=%d\n",online);
}

/*****************************************************************************
函 数 名  : chg_stm_get_extchg_online_st
功能描述  : get the extchg online status.
输入参数  : ONLINE: 无线充电在位
          OFFLINE:无线充电不在位
输出参数  : None
返 回 值  : VOID.
*****************************************************************************/
boolean chg_stm_get_extchg_online_st(void)
{
    if(TRUE == is_5577cs_falg)
    {
        return FALSE;
    }
     chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_extchg_online_st=%d\n",\
                            chg_stm_state_info.extchg_online_st);
    return chg_stm_state_info.extchg_online_st;
}
#endif/*MBB_CHG_EXTCHG*/

/*****************************************************************************
 函 数 名  : chg_stm_set_cur_state
 功能描述  : Update the current battery charge state to new state.
 输入参数  : new_state  The new state we gonna update to.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : chg_stm_init
             chg_stm_switch_state
 CALL-WHOM : None.
 NOTICE    : Helper function.
*****************************************************************************/
static void chg_stm_set_cur_state(chg_stm_state_type new_state)
{
    /*1. Update the current state machine state.*/
    chg_stm_state_info.cur_stm_state = new_state;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_cur_state=%d\n",new_state);
}

/*****************************************************************************
 函 数 名  : chg_stm_get_cur_state
 功能描述  : Get current battery charge state: Fast charge, Transitition state, etc.
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
chg_stm_state_type chg_stm_get_cur_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_cur_state=%d\n",\
                    chg_stm_state_info.cur_stm_state);
    /*1. Return the current state.*/
    return chg_stm_state_info.cur_stm_state;
}

/*****************************************************************************
 函 数 名  : chg_set_cur_chg_mode
 功能描述  : set the current  charge mode
 输入参数  : new_state  The new state we gonna update to.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  :
 CALL-WHOM : None.
 NOTICE    : Helper function.
*****************************************************************************/
void chg_set_cur_chg_mode(CHG_MODE_ENUM chg_mode)
{
    chg_stm_state_info.cur_chg_mode = chg_mode;
    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set_cur_chg_mode=%d\n",chg_mode);
}

/*****************************************************************************
 函 数 名  : chg_stm_get_cur_state
 功能描述  : Get current battery charge state: Fast charge, Transitition state, etc.
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
CHG_MODE_ENUM chg_get_cur_chg_mode(void)
{

    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_cur_chg_mode =%d\n",\
                  chg_stm_state_info.cur_chg_mode);

    /*1. Return the current state.*/
    return chg_stm_state_info.cur_chg_mode;
}

/*****************************************************************************
 函 数 名  : chg_get_charging_status
 功能描述  : 获取当前是否正在充电
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
boolean chg_get_charging_status(void)
{
    chg_stm_state_type chg_stm_cur_state = CHG_STM_INIT_ST;
    chg_stm_cur_state = chg_stm_get_cur_state();
    /*1. Return the current state.*/
    if((CHG_STM_FAST_CHARGE_ST == chg_stm_cur_state) \
       || (CHG_STM_WARMCHG_ST == chg_stm_cur_state))
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_charging_status is charging!\n");
        return TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_charging_status is not charging!\n");
        return FALSE;
    }
}


void chg_set_supply_limit_by_stm_stat(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();

    /*curr_state要作为数组下标进行索引，所以这里需要判断一下合法性。*/
    if (curr_state <= CHG_STM_INIT_ST || curr_state >= CHG_STM_MAX_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in %s.\n", 
            curr_state, __func__);
        return;
    }

    switch ( cur_chgr_type )
    {
        case CHG_WALL_CHGR:
        {
            chg_set_supply_limit(chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                    chg_std_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            break;
        }
        case CHG_USB_HOST_PC:
        {
            if ( CHG_CURRENT_SS == usb_speed_work_mode() )
            {
                chg_set_supply_limit(chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_usb3_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            else
            {
                chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                        chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            }
            break;
        }
        case CHG_500MA_WALL_CHGR:
        case CHG_NONSTD_CHGR:
        case CHG_CHGR_UNKNOWN:
        {
            chg_set_supply_limit(chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:resume supply current:%d\n",
                    chg_usb_chgr_hw_paras[curr_state].pwr_supply_current_limit_in_mA);
            break;
        }
        default:
            break;
    }
}
/*****************************************************************************
 函 数 名  : chg_set_hardware_parameter
 功能描述  : Call the charge IC driver interfaces to config/update the hardware
             parameter according to the given hw_param.
 输入参数  : Pointer of hw_param struct be set.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : chg_check_and_update_hw_param_per_chgr_type.
             Entry function of state machine state.
 CALL-WHOM : None.
 NOTICE    : Some feature would NOT be supported on certain IC, charge IC
             driver will handle this situation.
*****************************************************************************/
void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param)
{
    uint32_t ret_code = 0;

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if (TRUE == usb_err_flag)
    {
        return;
    }
#endif

    /* ASSERT(ptr_hw_param != NULL); */
    if (NULL == ptr_hw_param)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:NULL pointer in chg_set_hardware_parameter!!\n");
        return;
    }

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if(TRUE == chg_battery_protect_flag)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:In battery protect condition do not update parameter!!\n");
        return;
    }
#endif

    /*Dump all the parameters to set.*/
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[ILIMIT]: %dmA, [ICHG]: %dmA, [VBATREG]: %dmV.\n",
               ptr_hw_param->pwr_supply_current_limit_in_mA,
               ptr_hw_param->chg_current_limit_in_mA,
               ptr_hw_param->chg_CV_volt_setting_in_mV);

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:[ITERM]: %dmA, [CHARGE]: %s.\n",
               ptr_hw_param->chg_taper_current_in_mA,
               ptr_hw_param->chg_is_enabled ? "enabled" : "disabled");

    /*1. Set power supply front-end current limit.*/
    if ( FALSE == chg_set_supply_limit(ptr_hw_param->pwr_supply_current_limit_in_mA))
    {
        /*If error occured, set 1st bit of ret_code.*/
        ret_code |= (1 << 1);
    }
    #if defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS) \
    || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW)\
    || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
        if(CHG_BATT_ID_DEF != chg_get_batt_id())
        {
             /*2. Set charge current.*/
            if ( FALSE == chg_set_cur_level(ptr_hw_param->chg_current_limit_in_mA))
            {
                /*If error occured, set 2nd bit of ret_code.*/
                ret_code |= (1 << 2);
            }

            /*3. Set CV voltage, IC type dependent, may not work on some IC, e.g. max8903c.*/
            if ( FALSE == chg_set_vreg_level(ptr_hw_param->chg_CV_volt_setting_in_mV))
            {
                /*If error occured, set 3rd bit of ret_code.*/
                ret_code |= (1 << 3);
            }
        }
        else
        {
            #if (FEATURE_ON == MBB_CHG_BQ24196)
            /*2. Set charge current.*/
            if ( FALSE == chg_set_cur_level(CHG_DEF_BATT_CUR_LEVEL))
            {
                /*If error occured, set 2nd bit of ret_code.*/
                ret_code |= (1 << 2);
            }
            #else /*!MBB_CHG_BQ24196*/
            /*chg_set_current_level在其他充电IC驱动中未定义*/
            #error You need check HOWTO implement this on your charger IC!!
            #endif
            /*3. Set CV voltage, IC type dependent, may not work on some IC, e.g. max8903c.*/
            if ( FALSE == chg_set_vreg_level(CHG_DEF_BATT_VREG_LEVEL))
            {
                /*If error occured, set 3rd bit of ret_code.*/
                ret_code |= (1 << 3);
            }
        }
    #else
        /*2. Set charge current.*/
        if ( FALSE == chg_set_cur_level(ptr_hw_param->chg_current_limit_in_mA))
        {
            /*If error occured, set 2nd bit of ret_code.*/
            ret_code |= (1 << 2);
        }

        /*3. Set CV voltage, IC type dependent, may not work on some IC, e.g. max8903c.*/
        if ( FALSE == chg_set_vreg_level(ptr_hw_param->chg_CV_volt_setting_in_mV))
        {
            /*If error occured, set 3rd bit of ret_code.*/
            ret_code |= (1 << 3);
        }
    #endif
    /*4. Set taper(terminate) current. Also IC type dependent, may not work on
         some IC, e.g. max8903c.*/
    if ( FALSE == chg_set_term_current(ptr_hw_param->chg_taper_current_in_mA))
    {
        /*If error occured, set 4th bit of ret_code.*/
        ret_code |= (1 << 4);
    }

    /*5. Enable/Disable Charge.*/
    if ( FALSE == chg_set_charge_enable(ptr_hw_param->chg_is_enabled))
    {
        /*If error occured, set 5th bit of ret_code.*/
        ret_code |= (1 << 5);
    }

    if (ret_code)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Set charge IC hw parameter error 0x%x.\n", ret_code);
    }

}

/*****************************************************************************
 函 数 名  : chg_chgr_type_checking_timer_cb
 功能描述  : Callback function of charger type checking timer.
             This timer is not autoload, we may re-start it in
             chg_check_and_update_hw_param_per_chgr_type.
 输入参数  : VOID.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : OS timer interrupt or task.
 CALL-WHOM : None.
 NOTICE    : This function run in timer context.
*****************************************************************************/
static void chg_chgr_type_checking_timer_cb(chg_timer_para data)
{
    (void)data;

    /*1. Send CHG_CHGR_TYPE_CHECKING event to main task.*/
    (void)chg_send_msg_to_main(CHG_CHGR_TYPE_CHECKING_EVENT);
     chg_print_level_message(CHG_MSG_INFO, "CHG_STM: chg_send_msg_to_main CHG_CHGR_TYPE_CHECKING_EVENT!\n ");
}
/*****************************************************************************
 函 数 名  : chg_start_chgr_type_checking
 功能描述  : Callback function of charger type checking timer.
             This timer is not autoload, we may re-start it in
             chg_check_and_update_hw_param_per_chgr_type.
 输入参数  : VOID.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : OS timer interrupt or task.
 CALL-WHOM : None.
 NOTICE    : This function run in timer context.
*****************************************************************************/
void chg_start_chgr_type_checking(void)
{
    chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,chg_chgr_type_checking_timer_cb);
}

/*****************************************************************************
 函 数 名  : chg_check_and_update_hw_param_per_chgr_type
 功能描述  : When chgr_type_checking timer expired, this function would be called
             via charge main task.
             As its name suggests, it will check the chgr type from USB and if got,
             update the charge hardware parameter, or re-fire the timer utils max
             retry time reached.
 输入参数  : VOID.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : Charge Main Task.
 CALL-WHOM : None.
 NOTICE    : If max retry time reached, the chgr type would be set to CHG_NONSTD_CHGR.
             Need exported.
*****************************************************************************/
void chg_check_and_update_hw_param_per_chgr_type(void)
{
    static uint8_t chgr_type_checking_cnter = 0;
    chg_chgr_type_t chgr_type_from_usb = chg_stm_get_chgr_type();
    chg_stm_state_type curr_state = chg_stm_get_cur_state();

    /*1. If charger removal detected, give warning msg, return.*/
    if (FALSE == chg_is_charger_present())
    {
        chgr_type_checking_cnter = 0;
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr has been removed, type checking ignored.\n");
        return;
    }

    /*2. If chgr_type from USB is still CHG_CHGR_UNKNOWN:
       ->Y  a. IF chgr_type_checking_cnter >= CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES
                  chg_stm_set_chgr_type(CHG_NONSTD_CHGR); return;
               ELSE
                  chgr_type_checking_cnter++;
                  chgr_type_checking_timer;
       ->N  b. IF chgr_type != CHG_WALL_CHGR
                  chg_stm_set_chgr_type(chgr_type); return;
               ELSE
                  chg_stm_set_chgr_type(CHG_WALL_CHGR);
                  1) IF curr_state == FAST_CHG_STATE || INVALID_CHG_TEMP_STATE || MAINT_STATE
                        chg_set_hardware_parameter(&chg_std_chgr_hw_param[curr_state]);
                     ELSE
                        Give error msg.
     */

    if (CHG_CHGR_UNKNOWN == chgr_type_from_usb)
    {
        if (chgr_type_checking_cnter >= CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES)
        {
            chgr_type_checking_cnter = 0;
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type check timeout failed," \
                       " treat as non-standard.\n");
            mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Charger type check timeout failed," \
                       " treat as non-standard.\n");
            chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
        }
        else
        {
            chgr_type_checking_cnter++;
            chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,
                              chg_chgr_type_checking_timer_cb);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Raise %d time chgr type checking.\n",
                       chgr_type_checking_cnter);
        }

        return;
    }
    else if((CHG_WALL_CHGR == chgr_type_from_usb) || (CHG_USB_HOST_PC == chgr_type_from_usb)
             || (CHG_NONSTD_CHGR == chgr_type_from_usb))
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr type %d attached after %d times polling.\n",
            chgr_type_from_usb, chgr_type_checking_cnter + 1);
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG_STM:Charger type %d attached " \
                   "after %d times polling.\n",
                   chgr_type_from_usb, chgr_type_checking_cnter + 1);
        chgr_type_checking_cnter = 0;
        if (CHG_WALL_CHGR != chgr_type_from_usb)
        {
            if ((CHG_USB_HOST_PC == chgr_type_from_usb) && (CHG_CURRENT_SS == usb_speed_work_mode()))
            {
                chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_check_and_update_hw_param_per_chgr_type]set usb 3.0 charge parameter\n\r");
                chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[curr_state]);
            }
            else
            {
                 chg_print_level_message(CHG_MSG_INFO, "CHG_STM:USB/3rd chgr, no need update hw paras.\n");
            }
        }
        else
        {
            if (curr_state == CHG_STM_FAST_CHARGE_ST
                || curr_state == CHG_STM_WARMCHG_ST
                || curr_state == CHG_STM_MAINT_ST
                || curr_state == CHG_STM_INVALID_CHG_TEMP_ST)
            {
                chg_en_flag = 1;
                chg_en_timestamp = jiffies;
                chg_set_hardware_parameter(&chg_std_chgr_hw_paras[curr_state]);
            }
            else
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stm state %d "    \
                           "during chgr type checking.\n", curr_state);
            }
        }
    }
    else if(CHG_500MA_WALL_CHGR == chgr_type_from_usb)
    {
        chgr_type_checking_cnter = 0;
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[curr_state]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == chgr_type_from_usb)
    {
        chgr_type_checking_cnter = 0;
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[curr_state]);
        mlog_print(MLOG_CHG, mlog_lv_info, "Wireless charger detected in polling check.\n");
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chgr_type_checking_cnter = 0;
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid chgr_type!\n");
    }
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    if (CHG_CHGR_UNKNOWN != chgr_type_from_usb)
    {
        chg_hw_para_updated = 1;
    }
#endif
}

/*****************************************************************************
 函 数 名  : chg_stm_switch_state
 功能描述  : Charge State Machine Switch State function. State Machine or other
             Sub-Module call this function change the battery charge state.
 输入参数  : new_state  The new state we gonna switch to.
 输出参数  : None
 返 回 值  : VOID
 WHO-CALL  : All states' periodic function.
             Polling timer and charger insertion/removal event(BH)
             from charge task.
 CALL-WHOM : The exit function of current state.
             The entry function of new state.
 NOTICE    : If the new_state is Transition State, its periodic would be called.
*****************************************************************************/
void chg_stm_switch_state(chg_stm_state_type new_state)
{
    /*1. Get the system current state.*/

    /*2. Check the new state validity, or give error msg.*/

    /*3. new_state differ from curr_state
    ->Y  Switching state...
         a. If current state is not default initial state, do exit.
         b. Give info msg that system state switch from curr_state to new_state.
         c. Set the current state to new state.
         d. Do entry of new_state.
         e. If new_state is transition state, do periodic function.
    ->N  Give warning msg.
     */
    chg_stm_state_type cur_state = chg_stm_get_cur_state();
    if((cur_state >= CHG_STM_MAX_ST) || (cur_state <= CHG_STM_INIT_ST))
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_stm_switch_state: cur_state Invalid \n");
        return;
    }

    #if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    chg_stm_set_pre_state(cur_state);
    #endif /* MBB_CHG_WARM_CHARGE */

    if (new_state >= CHG_STM_MAX_ST || new_state <= CHG_STM_INIT_ST)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_stm_switch_state: Invalid state %d.\n", new_state);
        return;
    }

     chg_stm_set_cur_state(new_state);
    if (cur_state != new_state)
    {
        if (NULL != chg_stm_state_machine[cur_state].chg_stm_exit_func)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state exit %d state.\n", cur_state);
            chg_stm_state_machine[cur_state].chg_stm_exit_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Cur_state %d exit func doesN'T exist.\n", cur_state);
        }

        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge state machine switch from %d to %d state.\n",
            cur_state, new_state);
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge state machine switch from %s to %s state.\n",
            (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
            TO_STM_NAME(new_state));


        if (NULL != chg_stm_state_machine[new_state].chg_stm_entry_func)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state entry %d state.\n", new_state);
            chg_stm_state_machine[new_state].chg_stm_entry_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:New_state %d entry func doesN'T exist.\n", new_state);
        }

        /*added by ligang to shorten ui display time*/
        if ((CHG_STM_TRANSIT_ST == new_state)
            && (NULL != chg_stm_state_machine[new_state].chg_stm_period_func))
        {
            /*切换前进行一次电池电压轮询查询**/
            chg_poll_bat_level();
            /* 切换前进行一次电池温度轮询查询**/
            chg_poll_batt_temp();
            chg_stm_state_machine[new_state].chg_stm_period_func();
        }

        if (CHG_STM_INVALID_CHG_TEMP_ST == new_state)
        {
            mlog_print(MLOG_CHG, mlog_lv_warn, "CHG STM switch from %s to invalid chg temp state. "
                "tBat = %d'C, tBat_sys = %d'C.\n",
                (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
                chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        }

        mlog_print(MLOG_CHG, mlog_lv_info, "CHG STM switch from %s to %s state.\n",
            (CHG_STM_INIT_ST == cur_state) ? "Init" : TO_STM_NAME(cur_state),
            TO_STM_NAME(new_state));
        (void)chg_dump_ic_hwinfo();
        mlog_print(MLOG_CHG, mlog_lv_info, "Current Battery Info: [vBat]%dmV, [vBat_sys]%dmV, " \
            "[tBat]%d'C, [tBat_sys]%d'C.\n", chg_get_cur_batt_volt(), chg_get_sys_batt_volt(),
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
#if ( FEATURE_ON == MBB_MLOG )
        if(((CHG_STM_FAST_CHARGE_ST == new_state) && (CHG_STM_WARMCHG_ST != cur_state)) || 
            ((CHG_STM_WARMCHG_ST == new_state) && (CHG_STM_FAST_CHARGE_ST != cur_state)))
        {
            printk(KERN_ERR, "CHG_STM:charge_count\n");
            mlog_set_statis_info("charge_count",1);//充电总次数 加1
        }
        if(CHG_STM_WARMCHG_ST == new_state)
        {
            printk(KERN_ERR,"CHG_STM:overtemp_charge_count\n");
            mlog_set_statis_info("overtemp_charge_count",1);//高温充电次数加1
        }
        if(CHG_STM_INVALID_CHG_TEMP_ST == new_state) 
        {
            printk(KERN_ERR, "CHG_STM:overtemp_charge_stop_count\n");
            mlog_set_statis_info("overtemp_charge_stop_count",1); //高温停充次数加1
        }
#endif        
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_switch_state: charge has already be %s[%d] state, " \
                         "no need switch.\n", TO_STM_NAME(cur_state), cur_state);
    }
}

/*****************************************************************************
 函 数 名  : chg_stm_periodic_checking_func
 功能描述  : Periodic checking function of charge state machine. Charge main
             task would call it periodically once polling timer triggered.
 输出参数  : None
 返 回 值  : VOID
 WHO-CALL  : chg main task.
 CALL-WHOM : All states' period function.
 NOTICE    : The periodic function of transition state should NOT be called
             here.
             It's better NOT to call this function in timer/interrupt context.
             Need Exported.
*****************************************************************************/
void chg_stm_periodic_checking_func(void)
{
    chg_stm_state_type chg_current_st = CHG_STM_INIT_ST;
    chg_chgr_type_t    chg_cur_chgr   = CHG_CHGR_UNKNOWN;

    uint32_t chg_prt_timer_val = 0;
    int32_t  cur_poll_timer_period  = 0;
    /*0:-PwrOff Charging; 1:-Normal Charging.*/
    uint8_t  is_normal_chg_mode     = !chg_is_powdown_charging();
    /*0:-USB/NoStd Chgr;  1:-Wall/Standard Chgr.*/
    uint8_t  is_std_wall_chgr       = (CHG_WALL_CHGR == chg_stm_get_chgr_type());

    /* Get the current battery charging state */
    chg_current_st = chg_stm_get_cur_state();

    if(1 == chg_en_flag)
    {
        unsigned long time_now = jiffies;
        if((time_now - chg_en_timestamp) > CHG_EN_TIME_SLOT) 
        {
            chg_en_flag = 0;
        }
    }
    else
    {
        //for lint
    }

    /*Make sure that we are within the charger state machine
      configuration table*/
    if (chg_current_st > CHG_STM_INIT_ST && chg_current_st < CHG_STM_MAX_ST)
    {
        /* If we have a valid state function, call it. */
        if (NULL != chg_stm_state_machine[chg_current_st].chg_stm_period_func)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:stm_periodic_checking_func polling at %d state\n", chg_current_st);
            chg_stm_state_machine[chg_current_st].chg_stm_period_func();
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:State %d haveNOT period func.\n", chg_current_st);
        }

        if (CHG_STM_TRANSIT_ST == chg_current_st)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Transition state while period checking.\n");
        }
        /*Obtain current_st once again, since chg state may be updated in period_func*/
        chg_current_st = chg_stm_get_cur_state();
        chg_cur_chgr   = chg_stm_get_chgr_type();

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stay at %s[%d] state now.\n",
            TO_STM_NAME(chg_current_st), chg_current_st);
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Current chgr is %s[%d].\n",
            TO_CHGR_NAME(chg_cur_chgr), chg_cur_chgr);

        if(TRUE == chg_get_charging_status())
        {
            chg_prt_timer_val = chg_fastchg_timeout_value_in_sec[is_normal_chg_mode][is_std_wall_chgr];
            if (chg_stm_state_info.charging_lasted_in_sconds < chg_prt_timer_val)
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System keep staying at charging state.\n");

                cur_poll_timer_period = chg_poll_timer_get();
                if (CHG_ERROR == cur_poll_timer_period)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_poll_timer_get error.\n");

                }
                else
                {
                    /*充电未超时标志位置为false*/
                    chg_time_expired_flag = FALSE;
                    chg_stm_state_info.charging_lasted_in_sconds +=
                                   ((uint32_t)cur_poll_timer_period / MS_IN_SECOND);
                    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charging has lasted for %d Seconds.\n",
                                            chg_stm_state_info.charging_lasted_in_sconds);
                }
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:charging SW timer expired, " \
                       "battery voltage(sys) %dmV.\n", chg_get_sys_batt_volt());

                /*If battery is full already, or at pwroff chg mode, goto maint state.*/
                if (TRUE == chg_is_batt_full() || FALSE == is_normal_chg_mode)
                {
                   /*4. Reset fast charge counter.and switch to manit state,disable chg*/
                   /*关机状态充电超时，将标志位置为true，复充前判断此标志位*/
                   if (FALSE == is_normal_chg_mode)
                   {
                       chg_time_expired_flag = TRUE;
                   }
                   chg_stm_switch_state(CHG_STM_MAINT_ST);
                   chg_stm_state_info.charging_lasted_in_sconds = 0;
                }
                else
                {
                    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Reset charging counter, " \
                           "continue to charge.\n");
                 /*Reset fast charge counter and continue to charge */
                    chg_stm_state_info.charging_lasted_in_sconds = 0;
                }

                mlog_print(MLOG_CHG, mlog_lv_warn, "Charging SW timer expired, " \
                       "battery voltage(sys) %dmV.\n", chg_get_sys_batt_volt());
                mlog_print(MLOG_CHG, mlog_lv_warn, "current state: %s, current charger:%s.\n",
                    TO_STM_NAME(chg_current_st), TO_CHGR_NAME(chg_cur_chgr));
            }
        }
        else
        {
            chg_stm_state_info.charging_lasted_in_sconds = 0;
        }
        /*检测停充但充电器在位的时间*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
        if ((TRUE ==  chg_is_charger_present())
            && (FALSE == chg_get_charging_status())
            && (BATT_CAPACITY_FULL == chg_real_info.bat_capacity))
        {
            cur_poll_timer_period = chg_poll_timer_get();
            if (CHG_ERROR == cur_poll_timer_period)
            {
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_poll_timer_get error.\n");
            }
            else
            {
                chg_stm_state_info.charger_lasted_without_charging_in_seconds +=
                               ((uint32_t)cur_poll_timer_period / MS_IN_SECOND);
                chg_print_level_message(CHG_MSG_INFO,
                                        "CHG_STM:charger has existed without charging for %d Seconds.\n",
                                        chg_stm_state_info.charging_lasted_in_sconds);
            }
        }
        else
        {
            chg_stm_state_info.charger_lasted_without_charging_in_seconds = 0;
        }
#endif
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid state %d in period checking.\n", chg_current_st);
    }
}

/*****************************************************************************
 函 数 名  : chg_notify_UI_charging_state
 功能描述  : Notify the UI battery charging state changed.
             If the battery charging state didn't changed, this function doesn't
             take effect.
 输入参数  : bc_state: new battery charging state.
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: Entry function of every states.
 NOTICE    : This function is platform dependent.
*****************************************************************************/
static void chg_notify_UI_charging_state(uint8_t bc_state)
{
    /*Don't display battery charging as default.*/
    uint8_t   new_bc_state         = !!bc_state;
    uint8_t   is_pwr_off_charging  = chg_is_powdown_charging();
    BATTERY_EVENT  event_to_send   = BAT_EVENT_MAX;
    static uint8_t cur_ui_bc_state = 0;

    if (new_bc_state != cur_ui_bc_state)
    {
        if (new_bc_state)
        {
            if (is_pwr_off_charging)
            {
                event_to_send = BAT_CHARGING_OFF_START;
            }
            else
            {
                event_to_send = BAT_CHARGING_ON_START;
            }
        }
        else
        {
            if (is_pwr_off_charging)
            {
                event_to_send = BAT_CHARGING_DOWN_STOP;
            }
            else
            {
                event_to_send = BAT_CHARGING_UP_STOP;
            }
        }
        cur_ui_bc_state = new_bc_state;

        /*Send the event to UI module.*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, event_to_send);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Send battery event %d to APP.\n", event_to_send);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:UI bc state is already %d.\n", cur_ui_bc_state);
    }
}

/*****************************************************************************
 函 数 名  : chg_transit_state_entry_func
 功能描述  : Charge Transistion State entry function. Initialize all the
             statistic of current state: reset charger type, relevant timers,
             keep the UI battery charging status.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 PREVIOUS  : Initial State(Not REAL state)
             Battery Only State.
 NOTICE    : System can't stay at this state more than ONE polling timer
             period.
*****************************************************************************/
static void chg_transit_state_entry_func(void)
{
    boolean is_chgr_present      = chg_is_charger_present();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into transition state.\n");

    /*1. Reset charger type to INVALID will be done at:
         a. chg_stm_init() when system just powered up.
         b. Entry function of battery only state when normal running.
     */

    /*2. Don't change UI battery charging state, just KEEP is OK.*/

    /*3. Set poll timer mode, FAST or CHGR_INPUT, will be done:
         a. When system just powered up: chg_stm_init or other module init function.
         b. For CHGR_INPUT: period func.
     */
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:fastchg_state_period is_chgr_present=%d, cur_chgr_type=%d.\n",
                        is_chgr_present,cur_chgr_type);
    chg_poll_timer_set(FAST_POLL_CYCLE);
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:transit_state_entry_func poll_timer_set FAST_POLL_CYCLE.\n");

    /*4. IF chgr_is_present
       ->Y: 1. Read chgr type from USB, Set chgr_type
            IF chgr type still unknown
          ->Y: Raise chgr type checking timer.
            2. Set charging hardware parameter per charger type
     */
    if (TRUE == is_chgr_present)
    {
        if (CHG_CHGR_UNKNOWN == cur_chgr_type)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Chgr type unknown, raise checking timer.\n");
            /*chg_sta_timer_set(CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS,
                              chg_chgr_type_checking_timer_cb);*/
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Chgr_type = %d in transit entry.\n", cur_chgr_type);
        }

        /*Config charge hardware parameter according to chgr_type.*/
        if (CHG_WALL_CHGR == cur_chgr_type)
        {
            chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        //usb 3.0类型
        else if ((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
        {
            chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_transit_state_entry_func]set usb 3.0 charge parameter\n\r");
            chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
        {
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }
        //usb2.0类型
        else
        {
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_TRANSIT_ST]);
        }

        mlog_print(MLOG_CHG, mlog_lv_info, "charger type %d insertion detected.\n", cur_chgr_type);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else
    {
        if(CHG_WIRELESS_CHGR == cur_chgr_type)
        {
            /*一直打开无线充电芯片使能，无线充电过程中个状态的使能由BQ24196控制*/
            chg_set_wireless_chg_enable(TRUE);
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_transit_state_entry_func disable wireless chg!\n");
            chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_TRANSIT_ST]);

            mlog_print(MLOG_CHG, mlog_lv_info, "wireless charger detected.\n", cur_chgr_type);
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_transit_state_entry_func Is not wireless !\n");
        }
    }
#endif/*MBB_CHG_WIRELESS*/
}

/*****************************************************************************
 函 数 名  : chg_transit_state_period_func
 功能描述  : Charge Transistion State periodic polling function, which get called
             when polling timer event triggered.
             This function checks all the system condition, then choose the right
             state to swith.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
             chg_main_poll_cb(?)
 NOTICE    : Transition state is transient(unstable), as its name suggests.
             Thus system can't stay at this state more than ONE polling timer
             period.
*****************************************************************************/
static void chg_transit_state_period_func(void)
{
     /*1. IF charger_remove
             Switch to batttery only state.
       */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg not exist force transit_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);

    }

    /*2. IF vBat > batt_volt_full_threshold || batt OVP
            Switch to maintenance state.
     */
    else if (TRUE == chg_is_batt_full_for_start() || TRUE == chg_is_batt_ovp())
    
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Batt full/OVP, vBat = %dmV, vBat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Batt full/OVP, vBat = %dmV, vBat_sys = %dmV.\n",
                   chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }

    /*3. IF Battery is too hot or too cold
            Switch to invalid charge battery temperature state.
     */
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        mlog_print(MLOG_CHG, mlog_lv_warn, "CHG_STM:Invalid batt-temp, tBat = %d'C, " \
                   "tBat_sys = %d'C.\n", chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {
         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Switch transit state to warmchg state, tBat = %d'C, tBat_sys = %d'C.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);
    }
#endif /* MBB_CHG_WARM_CHARGE */

    /*4. ELSE Switch to fast chg state.*/
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:form transit_state to fast_stste!\n");
        chg_stm_switch_state(CHG_STM_FAST_CHARGE_ST);
    }
}

/*****************************************************************************
 函 数 名  : chg_transit_state_exit_func
 功能描述  : Charge Transistion State exit function.
             This function get called when system exit from Transition State,
             to do some clean/destroy stuff.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 NEXT      : All other states except transition state itself.
 NOTICE    : TO BE CHECKED: WHETHER NECESSARY.
             MAY BE REMOVED IN FUTURE.
*****************************************************************************/
static void chg_transit_state_exit_func(void)
{
   chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from transition state.\n");
}
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
/*******************************************************************
Function:      chg_set_high_volt_batt_current
Description:   设置0-10度充电电流
Data Accessed: 无
Data Updated:  无
Input:         充电器类型
Return:        无
*******************************************************************/
void chg_set_high_volt_batt_current(chg_chgr_type_t cur_chgr_type )
{
    switch(cur_chgr_type)
    {
        case CHG_WALL_CHGR:
        {
            #if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
            || defined(BSP_CONFIG_BOARD_E5577S_324) || defined(BSP_CONFIG_BOARD_E5577BS_937)
            if((FALSE == is_5577cs_falg)&&(TRUE == is_bat_3000))
            {
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_WALL_H);
            }
            else
            {
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_WALL);
            }
            #elif defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
            || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW) \
            || defined(BSP_CONFIG_BOARD_R218h)
            if (CHG_BATT_ID_DEF != chg_get_batt_id())
            {
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_WALL);
            }
            else
            {
                #if (FEATURE_ON == MBB_CHG_BQ24196)
                chg_set_cur_level(CHG_DEF_BATT_CUR_LEVEL);
                #else /*!MBB_CHG_BQ24196*/
                /*chg_set_current_level?????IC??????*/
                #error You need check HOWTO implement this on your charger IC!!
                #endif
            }
            #else
            chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_WALL);
            #endif

            break;  
        }
        case CHG_USB_HOST_PC:
        {
            if(CHG_CURRENT_SS == usb_speed_work_mode())
            {
                #if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
               || defined(BSP_CONFIG_BOARD_E5577S_324) || defined(BSP_CONFIG_BOARD_E5577BS_937)
                if((FALSE == is_5577cs_falg)&&(TRUE == is_bat_3000))
                {
                    chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB3_H); 
                }
                else
                {
                    chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB3); 
                }
                #elif defined(BSP_CONFIG_BOARD_E5573CS) ||defined(BSP_CONFIG_BOARD_E5573BS) \
                || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW) \
                || defined(BSP_CONFIG_BOARD_R218h)
                if (CHG_BATT_ID_DEF != chg_get_batt_id())
                {
                    chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB3); 
                }
                else
                {
                    #if (FEATURE_ON == MBB_CHG_BQ24196)
                    chg_set_cur_level(CHG_DEF_BATT_CUR_LEVEL);
                    #else /*!MBB_CHG_BQ24196*/
                    /*chg_set_current_level?????IC??????*/
                    #error You need check HOWTO implement this on your charger IC!!
                    #endif
                }
                #else
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB3); 
                #endif
                              

            }
            else
            {
                #if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
              || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
                if((FALSE == is_5577cs_falg)&&(TRUE == is_bat_3000))
                {
                    chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2_H); 
                }
                else
                {
                    chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2);
                }
                #elif defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
                || defined(BSP_CONFIG_BOARD_E5573CS_933)|| defined(BSP_CONFIG_BOARD_607HW) \
                || defined(BSP_CONFIG_BOARD_R218h)
                if (CHG_BATT_ID_DEF != chg_get_batt_id())
                {
                    chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2);
                }
                else
                {
                    #if (FEATURE_ON == MBB_CHG_BQ24196)
                    chg_set_cur_level(CHG_DEF_BATT_CUR_LEVEL);
                    #else /*!MBB_CHG_BQ24196*/
                    /*chg_set_current_level?????IC??????*/
                    #error You need check HOWTO implement this on your charger IC!!
                    #endif
                }
                #else
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2); 
                #endif

            }
            break;
        }
        case CHG_500MA_WALL_CHGR:
        case CHG_NONSTD_CHGR:
        {
            #if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
           || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
            if((FALSE == is_5577cs_falg)&&(TRUE == is_bat_3000))
            {
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2_H); 
            }
            else
            {
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2);
            }
            #elif defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
            || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW) \
            || defined(BSP_CONFIG_BOARD_R218h)
            if (CHG_BATT_ID_DEF != chg_get_batt_id())
            {
                chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2);
            }
            else
            {
                #if (FEATURE_ON == MBB_CHG_BQ24196)
                chg_set_cur_level(CHG_DEF_BATT_CUR_LEVEL);
                #else /*!MBB_CHG_BQ24196*/
                /*chg_set_current_level?????IC??????*/
                #error You need check HOWTO implement this on your charger IC!!
                #endif
            }
            #else
            chg_set_cur_level(CHG_SUB_LOW_TEMP_CURR_USB2); 
            #endif

            break;
        }
        default:
        break;
    }
}
/*******************************************************************
Function:      chg_high_volt_batt_entry_func
Description:   设置0-10度充电电流
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_high_volt_batt_entry_func(void)
{
    uint32_t battry_temp = chg_get_sys_batt_temp();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    
    chg_sub_low_temp_changed = -1;

    if(0 == chg_hw_para_updated)
    {
        /*充电参数还未更新*/
        return;
    }
#if( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    if( TRUE == chg_battery_protect_flag )
    {
        if( 0 == chg_sub_low_temp_changed )
        {
            chg_sub_low_temp_changed = -1;
        }
        return;
    }
#endif
    if (battry_temp <= CHG_SUB_LOW_TEMP_TOP)
    {
        chg_sub_low_temp_changed = 0;
        chg_set_high_volt_batt_current(cur_chgr_type);
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set high batt current in 0-10c!\n");
    }
}
/*******************************************************************
Function:      chg_high_volt_batt_period_func
Description:   设置0-10度充电电流
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_high_volt_batt_period_func(void)
{
    uint32_t battry_temp = chg_get_sys_batt_temp();
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    
    if(0 == chg_hw_para_updated)
    {
        /*充电参数还未更新*/
        return;
    }

#if( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
    if( TRUE == chg_battery_protect_flag )
    {
        if( 0 == chg_sub_low_temp_changed )
        {
            chg_sub_low_temp_changed = -1;
        }
        return;
    }
#endif

    /* 根据温度区间转换设置充电电流 */
    if (battry_temp <= CHG_SUB_LOW_TEMP_TOP && -1 == chg_sub_low_temp_changed)
    {
        chg_sub_low_temp_changed = 0;
        chg_set_high_volt_batt_current(cur_chgr_type);
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:set high batt current in 0-10c!\n");
    }
    else if (battry_temp >= (CHG_SUB_LOW_TEMP_TOP + CHG_TEMP_RESUM) && 0 == chg_sub_low_temp_changed)
    {
        chg_sub_low_temp_changed = -1;
        #if defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS)\
        || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW)\
        || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
        if (CHG_BATT_ID_DEF != chg_get_batt_id())
        {
            chg_set_cur_level(chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "Unknown batt type, use safer chg paras.\n");
            #if (FEATURE_ON == MBB_CHG_BQ24196)
            chg_set_cur_level(CHG_DEF_BATT_CUR_LEVEL);
            #else /*!MBB_CHG_BQ24196*/
            /*chg_set_current_level在其他充电IC驱动中未定义*/
            #error You need check HOWTO implement this on your charger IC!!
            #endif
        }
        #else
        chg_set_cur_level(chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST].chg_current_limit_in_mA);
        #endif
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:resume high batt current in 0-10c!\n");
    }
}
#endif

/*****************************************************************************
 函 数 名  : chg_fastchg_state_entry_func
 功能描述  : Charge Fast Charge State entry function. Initialize all the
             statistic of current state: reset fast charge timer, notify the
             UI that battery charging started.
             Set the corresponding charging parameter per charger type.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 PREVIOUS  : Transition State.
             Maintenance State.
             Invalid Charge Temperature State.
 WHO-CALLED: chg_switch_state
 NOTICE    : If charger type unknown, treat it as CHG_NONSTD_CHGR.
*****************************************************************************/
static void chg_fastchg_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into fast-charge state.\n");

    /*1. Notify UI that battery charging started.*/
    chg_notify_UI_charging_state(CHG_UI_START_CHARGING);

    /*2. Set charging hardware parameter per charger type.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        /*使能充电，设置标志*/  
        chg_en_flag = 1;
        chg_en_timestamp = jiffies;
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set std_chgr_hw_paras!\n");

    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_fastchg_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:fastchg_state_entry set usb_chgr_hw_paras!\n");
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_fastchg_state_entry_func enable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_fastchg_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:chg_fastchg_state_entry_funcset usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:fastchg_state_entry set usb_chgr_hw_paras!\n");
    }
#endif/*MBB_CHG_WIRELESS*/
    /*3. Enable IC Charge function.*/
    /*Already done in chg_set_hardware_parameter.*/
    
#if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
    chg_high_volt_batt_entry_func();
#endif

}

/*****************************************************************************
 函 数 名  : chg_fastchg_state_period_func
 功能描述  : Charge Fast Charge State periodic polling function, which get called
             when polling timer event triggered.
             This function checks the system condition, choose the right
             state to swith if system state changed, or keep staying at the fast
             charge state.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_stm_periodic_checking_func
 NOTICE    : Battery is charged during this state, which is also the ONLY state
             that battery get charged, otherwise charging is forbidden.
*****************************************************************************/
static void chg_fastchg_state_period_func(void)
{
    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;

    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force fastchg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    /*2. If charging stop received from IC, get the stop reason:
         a. Due to charging complete, switch to maintenance state.
         b. Due to charge IC protection timer expired, get the batt-volt(avg.):
            if (vBat > batt_volt_full_threshold)
               switch to maintenance state.
            else
               toggle CE(charge enable) pin, reset protection timer.
         c. Due to SW control(CE pin),
               do nothing, print warning msg.
     */
    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
                chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)  
            #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
            chg_stm_switch_state(CHG_STM_MAINT_ST);
            #else
            if(TRUE == chg_is_batt_full())
            {
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge Completed, but soc not 100, need adjust.\n");
            }
            #endif
#else
            chg_stm_switch_state(CHG_STM_MAINT_ST);
#endif
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                /*Toggle CEN, reset charge IC protection timer.*/
                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    /*If error occured, set 1st bit of ret_code.*/
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    /*If error occured, set 2nd bit of ret_code.*/
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_INVALID_TEMP == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stopped due to " \
                       "IC invalid temperature detected.\n");
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }

    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while fastchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }
    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while fastchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
    else if(TRUE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Switch fastchg state to warmchg state, tBat=%d'C,tBat_sys=%dC.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_WARMCHG_ST);

    }
#endif /* MBB_CHG_WARM_CHARGE */

    else
    {
        /* 非标准充电器充电电流快充阶段均一致 */
        #if (MBB_CHG_HIGH_VOLT_BATT == FEATURE_ON)
        chg_high_volt_batt_period_func();
        #endif
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
        if ((chg_get_sys_batt_volt() >= CHG_TERMI_VOLT_FOR_COUL) && \
                  (hisi_battery_current() <= CHG_TERMI_CURR_FOR_COUL))
        {
            chg_stm_switch_state(CHG_STM_MAINT_ST);
            chg_stm_state_info.charging_lasted_in_sconds = 0;
        }
#endif
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System keep staying at fast charging state.\n");
    }
}

/*****************************************************************************
 函 数 名  : chg_fastchg_state_exit_func
 功能描述  : Charge Fast Charge State exit function.
             This function get called when system exit from Fast Charge State,
             do some clean/destroy stuff.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 NEXT      : Charge Maintenance State
             Charge Invalid Charge Temperature State
             Battery Only State.
 NOTICE    : TO BE CHECKED: WHETHER NECESSARY.
             MAY BE REMOVED IN FUTURE.
*****************************************************************************/
static void chg_fastchg_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from fast-charge state.\n");
}


/*****************************************************************************
 函 数 名  : chg_maint_state_entry_func
 功能描述  : Charge Maintenance State entry function. Initialize all the
             statistic of current state: stop/disable battery charge via
             charge IC driver, notify UI that battery charging stopped.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 PREVIOUS  : Charge Transition State
             Charge Fast Charge State.
 NOTICE    : Set maximal power supply current limit as charger type, let the
             system get powered from external charger as much as possible.
             If charger type unknown, treat it as CHG_NONSTD_CHGR.
*****************************************************************************/
static void chg_maint_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into maintenance state.\n");

    /*1. Notify UI that battery charging stopped.*/
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if (TRUE == chg_is_batt_full())
    {
        chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);
    }
#else
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);
#endif

    /*2. Update charge parameter, set maximal PS. current limit.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_maint_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_maint_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_maint_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_maint_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_MAINT_ST]);
    }
#endif/*MBB_CHG_WIRELESS*/
}

/*****************************************************************************
 函 数 名  : chg_maint_state_period_func
 功能描述  : Charge Maintenance State periodic polling function, which get called
             when polling timer event triggered.
             This function checks all the system condition, then choose the right
             state to swith.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_main_poll_cb(?)
 NOTICE    : Battery is charged to full at Maintenance State. System would draw
             power from external charger preferentially, if battery is consumed
             low to recharge threshold, then re-charge raised.
*****************************************************************************/
static void chg_maint_state_period_func(void)
{
    /*1. IF charger_remove
            Switch to battery only state.
      */

    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force maint_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    /*2. Get the system battery volt:
         a. If re-charge condition meets?
            Notice: Re-charge is forbidden at pwroff charge, regardless of:
                    a). Battery is already full when system power-up.
                    b). Battery is charged to full.
                    c). Fast charge timeout at pwroff charge.
        ->Y 1) Get system battery temperature,
               a). Battery temperature valid, switch to fast charge state,
                   recharge started.
               b). Battery temperature invalid, switch to invlaid charge
                   temperature.
        ->N 2) Keep staying at maintenance state.
     */
    /*若充电超时标志位为true则直接退出不再判断是否需要复充*/
    else if (TRUE == chg_time_expired_flag)
    {
        return;
    }
    else if (TRUE == chg_is_batt_need_rechg())
    {
        if (FALSE == chg_is_batt_temp_valid())
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery temperature %d invlaid for charge.\n",  \
                       chg_get_sys_batt_temp());
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:batt_need_rechg force to TRANSIT_ST.\n");
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Keep staying at maint_state.\n");
    }
}

/*****************************************************************************
 函 数 名  : chg_maint_state_exit_func
 功能描述  : Charge Maintenance State exit function.
             This function get called when system exit from Maintenance State,
             to do some clean/destroy stuff.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 NEXT      : Charge Fast Charge State(Recharge).
             Battery Only State.
 NOTICE    : TO BE CHECKED: WHETHER NECESSARY.
             MAY BE REMOVED IN FUTURE.
*****************************************************************************/
static void chg_maint_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from maintenance state.\n");
}

/*****************************************************************************
 函 数 名  : chg_invalid_chg_temp_state_entry_func
 功能描述  : Invalid Charge Temperature State entry function. Initialize all the
             statistic of current state: stop/disable battery charge via
             charge IC driver, notify UI that battery charging stopped.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 PREVIOUS  : Charge Transition State
             Charge Fast Charge State.
 NOTICE    : Set maximal power supply current limit as charger type, let the
             system get powered from external charger as much as possible.
             If charger type unknown, treat it as CHG_NONSTD_CHGR.
*****************************************************************************/
static void chg_invalid_chg_temp_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into invalid chg temp state.\n");

    /*1. Notify UI that battery charging stopped.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    /*2. Update charge parameter, set maximal PS. current limit.*/
    /*3. Stop charging.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
#ifdef BSP_CONFIG_BOARD_E5_DCM
        if(CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp)
        {
            chg_set_hardware_parameter(&chg_std_chgr_over_temp_hw_paras);
        }
        else 
        {
            chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
        }
#else
        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
#endif
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:chg_invalid_chg_temp_state_entry_funcset usb 3.0 parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        /*温度异常状态使能无线充电芯片，再配置充电芯片限流及使能充电*/
        //chg_set_wireless_chg_enable(FALSE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_invalid_chg_temp_state_entry_func disable wireless chg!\n");
        chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_invalid_chg_temp_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_invalid_chg_temp_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
    }
    else
    {
#ifdef BSP_CONFIG_BOARD_E5_DCM
        if(CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp)
        {
            chg_set_hardware_parameter(&chg_usb_chgr_over_temp_hw_paras);
        }
        else 
        {
            chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
        }
#else
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_INVALID_CHG_TEMP_ST]);
#endif
    }
#endif/*MBB_CHG_WIRELESS*/
}

/*****************************************************************************
 函 数 名  : chg_invalid_chg_temp_state_period_func
 功能描述  : Invalid Charge Temperature State periodic polling function, which get called
             when polling timer event triggered.
             This function checks all the system condition, then choose the right
             state to swith.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_stm_periodic_checking_func
 NOTICE    : As its name suggests, during this state, charging is forbidden since
             battery is too hot/cold to charge.
*****************************************************************************/
static void chg_invalid_chg_temp_state_period_func(void)
{

    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force invalid_chg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }
    /*2. If the battery temperature resumed?
     ->Y a. If battery OVP detected, switch to maintenance state.
            else switch to fast charge state.
     ->N b. Keep staying at invalid charge temperature state.
     */
    else if (TRUE == chg_is_batt_temp_valid())
    {
        if (chg_is_batt_ovp())
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery OVP, batt-volt %dmV while temp resume.\n", \
                       chg_get_sys_batt_volt());
            chg_stm_switch_state(CHG_STM_MAINT_ST);
        }
        else
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Battery temp not resumed, staying at invalid temp.\n");
    }

}

/*****************************************************************************
 函 数 名  : chg_invalid_chg_temp_state_exit_func
 功能描述  : Invalid Charge Temperature State exit function.
             This function get called when system exit from Invalid Charge Temperature State,
             do some clean/destroy stuff.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 NEXT      : Fast Charge State
             Invalid Charge Temperature State
             Battery Only State
 NOTICE    : TO BE CHECKED: WHETHER NECESSARY.
             MAY BE REMOVED IN FUTURE.
*****************************************************************************/
static void chg_invalid_chg_temp_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from invalid chg temp state.\n");
}

/*****************************************************************************
 函 数 名  : chg_batt_only_state_entry_func
 功能描述  : Call when system is going to switch to Battery Only state.
             This phase we should: 1. Switch to fast polling(chgr absent) timer.
             2. Notify UI to stop battery charging display.
             3. Config the charge IC, disable charge, set minimal power supply
                front-end current limit, put IC to LPM(may auto).
             4. Set current chgr type to INVALID.
 输出参数  : None
 返 回 值  : VOID
 WHO-CALL  : chg_stm_switch_state.
 CALL-WHOM : chg_set_poll_timer_state.
 PREVIOUS  : All other states except Battery Only itself.
 NOTICE    : Leave charge IC config as minimal PS. current limit.
*****************************************************************************/
static void chg_batt_only_state_entry_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into batt-only state.\n");
    /*clear the temp flag*/
    chg_temp_is_too_hot_or_too_cold_for_chg();
    /*1. Notify UI to stop battery charging display.*/
    chg_notify_UI_charging_state(CHG_UI_STOP_CHARGING);

    chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
    /*2. Set current charger type to invalid, means charger absent.*/
    chg_stm_set_chgr_type(CHG_CHGR_INVALID);

    /*如果已经在切换到单电池状态了，检测到USB或者标准充电器还处于ONLINE，就降其置为OFFLINE*/
    if(TRUE == chg_get_usb_online_status())
    {
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
    else if(TRUE == chg_get_ac_online_status())
    {
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System enter batt-only state AC/USB already OFLINE!.\n");
    }

    /*3. Config charge hw parameter, disable charge, PS current limit min.*/
    //chg_set_hardware_parameter(&chg_batt_only_hw_paras);
    chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_BATTERY_ONLY]);
    /*4. Set poll timer state to fast mode(charger absent).*/
}

/*****************************************************************************
 函 数 名  : chg_batt_only_state_period_func
 功能描述  : Battery Only State periodic polling function, which get called
             when polling timer event triggered.
             Since charger attach event would handled via charge main task,
             this function just do nothing.
 输出参数  : None
 返 回 值  : VOID
 WHO-CALL  : chg_stm_periodic_checking_func.
 CALL-WHOM :
 NOTICE    : Battery only state will stay util charge main task switch it to
             new state once external charger insertion detected.
             MAY-TO-BE-REMOVED or SET TO NULL.
*****************************************************************************/
static void chg_batt_only_state_period_func(void)
{
    static uint32_t chg_poll_timer_switch_cnter = 0;
    int32_t         curr_polling_timer_mode     = chg_poll_timer_get();
    boolean         is_emergency_state          = chg_is_emergency_state();
    boolean is_chgr_present      = chg_is_charger_present();



    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:batt_only_state_period is_chgr_present=%d.\n",
                            is_chgr_present);

    /*Get poll timer period error, do nothing, return.*/
    if (CHG_ERROR == curr_polling_timer_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Get poll timer period error.\n");
        return;
    }

    /*1. If we stay at batt_only state more than CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC
         seconds, switch poll timer to slow mode.
         Notice: DON'T switch to slow timer mode if system in emergency state. e.g.,
                 low power, battery hot/cold, etc, supported by sampling sub-module.*/
    if (FAST_POLL_CYCLE == curr_polling_timer_mode && !is_emergency_state)
    {
        if (chg_poll_timer_switch_cnter >= CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System stay at batt-only state" \
                " more than %d seconds, switch to slow timer mode.\n",
                CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC);
            /*Clear cnter for next count.*/
            chg_poll_timer_switch_cnter = 0;
            chg_poll_timer_set(SLOW_POLL_CYCLE);
        }
        else
        {
            chg_poll_timer_switch_cnter += ((uint32_t)curr_polling_timer_mode / MS_IN_SECOND);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Poll switch cnter = %d.\n", chg_poll_timer_switch_cnter);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:%s polling mode, system %s at emergency State.\n",
            (curr_polling_timer_mode == FAST_POLL_CYCLE) ? "Fast" : "Slow",
            (is_emergency_state) ? "is" : "isn't");

        /*Clear cnter for next count.*/
        chg_poll_timer_switch_cnter = 0;
    }


#if (MBB_CHG_WIRELESS == FEATURE_ON)
    if(ONLINE == g_wireless_online_flag)
    {
        chg_stm_set_wireless_online_st(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_stm_set_chgr_type(CHG_WIRELESS_CHGR);
        chg_set_cur_chg_mode(CHG_WIRELESS_MODE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:wireless online change to CHG_WIRELESS_MODE!\n");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:System staying at batt-only state.\n");
    }
#endif/*MBB_CHG_WIRELESS*/
}

/*****************************************************************************
 函 数 名  : chg_batt_only_state_exit_func
 功能描述  : Charge Battery Only State exit function.
             This function get called when system exit from Battery Only State,
             to do some clean/destroy stuff.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : VOID
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
 NEXT      : Charge Transition State.
 NOTICE    : TO BE CHECKED: WHETHER NECESSARY.
             MAY BE REMOVED IN FUTURE.
*****************************************************************************/
static void chg_batt_only_state_exit_func(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from batt-only state.\n");
}

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)

void chg_stm_set_pre_state(chg_stm_state_type pre_state)
{
    chg_stm_state_info.pre_stm_state = pre_state;
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:set_pre_stm_state=%d\n",pre_state);
}


chg_stm_state_type chg_stm_get_pre_state(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:get_pre_stm_state=%d\n",\
                        chg_stm_state_info.pre_stm_state);
    return chg_stm_state_info.pre_stm_state;
}


boolean is_batttemp_in_warm_chg_area( void )
{
    boolean rtnValue = FALSE;
    static boolean last_rtnValue = FALSE;

    /*如果前一个状态是电池单独在位状态，说明发生了插拔外电源事件，
       将过温状态记录变量恢复到默认值不过温*/
    if (CHG_STM_BATTERY_ONLY == chg_stm_get_pre_state())
    {
        last_rtnValue = FALSE;
    }
#if defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS) \
    || defined(BSP_CONFIG_BOARD_E5573CS_933) || defined(BSP_CONFIG_BOARD_607HW)\
    || defined(BSP_CONFIG_BOARD_E5577BS_937) || defined(BSP_CONFIG_BOARD_R218h)
        if (CHG_BATT_ID_DEF == chg_get_batt_id())
        {
            return rtnValue;
        }
#endif

    if ( FALSE == last_rtnValue )
    {
        /*电池温度大于等于45度小于55度为高温充电区*/
        if ( ( CHG_WARM_CHARGE_ENTER_THRESHOLD <= chg_real_info.battery_temp ) \
             && ( CHG_OVER_TEMP_STOP_THRESHOLD > chg_real_info.battery_temp ) )
        {
            rtnValue = TRUE;
        }
        else
        {
            rtnValue = FALSE;
        }
    }
    else
    {
        /*小于42度或者大于55度单板不在高温充电区间 */
        if ( (CHG_WARM_CHARGE_EXIT_THRESHOLD > chg_real_info.battery_temp)\
             || (CHG_OVER_TEMP_STOP_THRESHOLD <= chg_real_info.battery_temp))
        {
            rtnValue = FALSE;
        }
        else
        {
            rtnValue = TRUE;
        }
    }

    last_rtnValue = rtnValue;
    return rtnValue;
}


static void chg_warmchg_state_entry_func(void)
{
    chg_chgr_type_t cur_chgr_type = chg_stm_get_chgr_type();

    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System enter into warm-charge state.\n");

    /*1.If the pre chg state is not CHG_STM_FAST_CHARGE_ST Notify UI
       that battery charging started.*/
    if ( CHG_STM_FAST_CHARGE_ST != chg_stm_get_pre_state())
    {
        chg_notify_UI_charging_state(CHG_UI_START_CHARGING);
    }

    /*2. Set charging hardware parameter per charger type.*/
    if (CHG_WALL_CHGR == cur_chgr_type)
    {
        /*使能充电，设置标志*/
        chg_en_flag = 1;
        chg_en_timestamp = jiffies;

        chg_set_hardware_parameter(&chg_std_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
    else if (CHG_500MA_WALL_CHGR == cur_chgr_type)
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_warmchg_state_entry_func]:set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else if((CHG_USB_HOST_PC == cur_chgr_type) || (CHG_NONSTD_CHGR == cur_chgr_type)
            || (CHG_CHGR_UNKNOWN == cur_chgr_type))
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }

    else if(CHG_WIRELESS_CHGR == cur_chgr_type)
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_warmchg_state_entry_func enable wireless chg!\n");
        /*开机无线充电设置正常的高温充电参数*/
        if(FALSE == chg_is_powdown_charging())
        {
            chg_set_hardware_parameter(&chg_wireless_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
        }
        /*关机无线充电设置关机的高温充电参数，前端限流900MA,充电限流1024MA,恒压电压4.1V
            截止电流256MA*/
        else
        {
            chg_set_hardware_parameter(&chg_wireless_poweroff_warmchg_paras);
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_warmchg_state_entry_func Is not wireless !\n");
    }
#else
    else if((CHG_USB_HOST_PC == cur_chgr_type) && CHG_CURRENT_SS == usb_speed_work_mode())
    {
        chg_print_level_message(CHG_MSG_DEBUG,"CHG_STM:[chg_warmchg_state_entry_func]set usb 3.0 charge parameter\n\r");
        chg_set_hardware_parameter(&chg_usb3_chgr_hw_paras[CHG_STM_FAST_CHARGE_ST]);
    }
    else
    {
        chg_set_hardware_parameter(&chg_usb_chgr_hw_paras[CHG_STM_WARMCHG_ST]);
    }

#endif/*MBB_CHG_WIRELESS*/

    /*3. Enable IC Charge function.*/
    /*Already done in chg_set_hardware_parameter.*/

}


static void chg_warmchg_state_period_func(void)
{
    chg_stop_reason stp_reas        = CHG_STOP_COMPLETE;
    uint32_t ret_code               = 0;
    /*1. IF charger_remove
            Switch to battery only state.
      */
    if(TRUE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg removed force warmchg_st to batt_only_st!\n");
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
    }

    /*2. If charging stop received from IC, get the stop reason:
         a. Due to charging complete, switch to maintenance state.
         b. Due to charge IC protection timer expired, get the batt-volt(avg.):
            if (vBat > batt_volt_full_threshold)
               switch to maintenance state.
            else
               toggle CE(charge enable) pin, reset protection timer.
         c. Due to SW control(CE pin),
               do nothing, print warning msg.
     */
    else if (FALSE == chg_is_IC_charging())
    {
        stp_reas = chg_get_stop_charging_reason();

        if (CHG_STOP_COMPLETE == stp_reas)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:Charge Completed, vBat = %dmV, vBat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)  
            #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
            chg_stm_switch_state(CHG_STM_MAINT_ST);
            #else
            if (TRUE == chg_is_batt_full())
            {
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            #endif
#else
            chg_stm_switch_state(CHG_STM_MAINT_ST);
#endif
        }
        else if (CHG_STOP_TIMEOUT == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge timeout from IC.\n");
            if (TRUE == chg_is_batt_full())
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Battery is already full, vBat_sys = %dmV.\n",
                    chg_get_sys_batt_volt());
                chg_stm_switch_state(CHG_STM_MAINT_ST);
            }
            else
            {
                chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Toggle CEN, continue to charge.\n");

                /*Toggle CEN, reset charge IC protection timer.*/
                if (FALSE == chg_set_charge_enable(FALSE))
                {
                    /*If error occured, set 1st bit of ret_code.*/
                    ret_code |= (1 << 1);
                }
                chg_delay_ms(CHG_TOGGLE_CEN_INTVAL_IN_MS);
                if (FALSE == chg_set_charge_enable(TRUE))
                {
                    /*If error occured, set 2nd bit of ret_code.*/
                    ret_code |= (1 << 2);
                }

                if (ret_code)
                {
                    chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Toggle CEN pin error 0x%x.\n", ret_code);
                }
            }
        }
        else if (CHG_STOP_INVALID_TEMP == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stopped due to " \
                       "IC invalid temperature detected.\n");
            chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
        }
        else if (CHG_STOP_BY_SW == stp_reas)
        {
            chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge stop due to SW control"   \
                       " in fast charge state.\n");
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Invalid stop reason %d.\n", stp_reas);
        }
    }
    /*3. battery OVP, switch to maint state.*/
    else if (TRUE == chg_is_batt_ovp())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Batt-OVP while warmchg, vBat = %dmV, vbat_sys = %dmV.\n",
            chg_get_cur_batt_volt(), chg_get_sys_batt_volt());
        chg_stm_switch_state(CHG_STM_MAINT_ST);
    }

    /*4. battery too cold/hot, switch to invalid charge temperature state.*/
    else if (FALSE == chg_is_batt_temp_valid())
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Invalid batt-temp while warmchg, tBat = %d'C, tBat_sys = %d'C.\n",
            chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
        chg_stm_switch_state(CHG_STM_INVALID_CHG_TEMP_ST);
    }
    /*5.battery temp resume switch to fastchg state*/
    else if(FALSE == is_batttemp_in_warm_chg_area())
    {

         chg_print_level_message(CHG_MSG_INFO, "CHG_STM:batt-temp resume form warmchg to fastchg, tBat = %d'C, tBat_sys = %d'C.\n",
         chg_get_cur_batt_temp(), chg_get_sys_batt_temp());
         chg_stm_switch_state(CHG_STM_FAST_CHARGE_ST);
    }
    else
    {
        #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
        if ((chg_get_sys_batt_volt() >= CHG_TERMI_WARMCHG_VOLT_FOR_COUL) && \
                  (hisi_battery_current() <= CHG_TERMI_CURR_FOR_COUL))
        {
            chg_stm_switch_state(CHG_STM_MAINT_ST);
            chg_stm_state_info.charging_lasted_in_sconds = 0;
        }
        #endif
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System keep staying at warmchg state.\n");
    }

}


static void chg_warmchg_state_exit_func(void)
{
     chg_print_level_message(CHG_MSG_INFO, "CHG_STM:System exit from warmchg state.\n");

}
 #endif /* MBB_CHG_WARM_CHARGE */



#if (MBB_CHG_EXTCHG == FEATURE_ON)

void extchg_stop_threshold_to_voltage(int32_t capacity)
{
    switch (capacity)
    {
        case EXTCHG_DEFAULT_STOP_THRESHOLD:
            g_extchg_voltage_threshold = EXTCHG_DEFAULT_STOP_VOLTAGE;
            break;
        case EXTCHG_STOP_CAPACITY_TEN:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_TEN;
            break;
        case EXTCHG_STOP_CAPACITY_TWENTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_TWENTY;
            break;
        case EXTCHG_STOP_CAPACITY_THIRTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_THIRTY;
            break;
        case EXTCHG_STOP_CAPACITY_FORTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_FORTY;
            break;
        case EXTCHG_STOP_CAPACITY_FIFTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_FIFTY;
            break;
        case EXTCHG_STOP_CAPACITY_SIXTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_SIXTY;
            break;
        case EXTCHG_STOP_CAPACITY_SEVENTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_SEVENTY;
            break;
        case EXTCHG_STOP_CAPACITY_EIGHTY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_EIGHTY;
            break;
        case EXTCHG_STOP_CAPACITY_NINETY:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_NINETY;
            break;
        case EXTCHG_STOP_CAPACITY_HUNDRED:
            g_extchg_voltage_threshold = EXTCHG_STOP_VOLTAGE_HUNDRED;
            break;
        default:
            break;
    }
}


void chg_extchg_config_data_init(void)
{
    int32_t fd    = 0;
    mm_segment_t fs;
    char extchg_threshold_temp = 0;
    char extchg_diable_st_temp = 0;
    int32_t extchg_capacity_threshold = 0;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fd = sys_open(EXTCHG_THRESHOLD_PATH, O_RDWR, 0);
    if(fd >= 0)
    {
        sys_read(fd, &extchg_threshold_temp, 1);
        extchg_capacity_threshold = extchg_threshold_temp;
        chg_print_level_message(CHG_MSG_INFO,"extchg_capacity_threshold=%d.\n", extchg_capacity_threshold);
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_extchg_stop_soc_threshold = extchg_capacity_threshold;
#else
        extchg_stop_threshold_to_voltage(extchg_capacity_threshold);
#endif
        sys_close(fd);
    }
    else
    {
        #if defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5577S_321) \
       || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
        extchg_capacity_threshold = EXTCHG_STOP_CAPACITY_TEN;
        #else
        extchg_capacity_threshold = EXTCHG_DEFAULT_STOP_THRESHOLD;
        #endif
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        g_extchg_stop_soc_threshold = extchg_capacity_threshold;
#else
        extchg_stop_threshold_to_voltage(extchg_capacity_threshold);
#endif
        chg_print_level_message(CHG_MSG_INFO,"UI set extchg_threshold read FAIL use default value 5!\n");
    }


    fd = sys_open(EXTCHG_DISABLE_PATH, O_RDWR, 0);
    if(fd >= 0)
    {
        sys_read(fd, &extchg_diable_st_temp, 1);
        g_extchg_diable_st = extchg_diable_st_temp;
        chg_print_level_message(CHG_MSG_INFO,"g_extchg_diable_st=%d.\n", g_extchg_diable_st);
        sys_close(fd);
    }
    else
    {
        g_extchg_diable_st = 0;
        chg_print_level_message(CHG_MSG_INFO,"UI set extchg_diable_st read FAIL use default value 0!\n");
    }
    set_fs(fs);
}




boolean chg_get_extchg_online_status(void)
{
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    return OFFLINE;
#else
    int32_t vbus_volt = 0;

    //TO DO:1:调用USB驱动接口短接D+ ,D-
    usb_notify_event(USB_OTG_CONNECT_DP_DM,NULL);

    /*使能无线充电*/
    chg_set_extchg_chg_enable(TRUE);

    /* 由于 E5577s-932 不使用 vbus 采集对外充电电压 */
#if defined(BSP_CONFIG_BOARD_E5577S_932)
    return ONLINE;
#endif

    /*延时500MS,防止使能充电后因为电路延迟导致采集电池电压异常*/
    chg_print_level_message(CHG_MSG_DEBUG,"CHG_PLT:chg_delay_ms 500MS! \n");
    chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
    vbus_volt = chg_get_volt_from_adc(CHG_PARAMETER__VBUS_VOLT);
    chg_print_level_message( CHG_MSG_INFO,"CHG_DRV:get_extchg_online vbus_volt = %d\r\n ",vbus_volt);
    if(vbus_volt > VBUS_JUDGEMENT_THRESHOLD)
    {
        return ONLINE;
    }
    else
    {
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: VBUS detect failure extchg is not online\r\n ");
        return OFFLINE;
    }
#endif
}

#endif /* MBB_CHG_EXTCHG */

/**********************************************************************
FUNCTION:    chg_get_batt_id_volt
DESCRIPTION: get batt id volt
INPUT:       None
OUTPUT:      None.
RETURN:      batt id volt
NOTE:        None
***********************************************************************/
int32_t chg_get_batt_id_volt(void)
{
    if(0xFFFF == chg_real_info.battery_id_volt)
    {
        chg_real_info.battery_id_volt = chg_get_volt_from_adc(CHG_PARAMETER__BATTERY_ID_VOLT);
    }
    return chg_real_info.battery_id_volt;
}


int32_t chg_get_bat_status(void)
{
    int32_t bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
    chg_stm_state_type stm_status = CHG_STM_MAX_ST;

    stm_status = chg_stm_get_cur_state();
    if((CHG_STM_INIT_ST == stm_status) || (CHG_STM_TRANSIT_ST == stm_status)
        || (CHG_STM_INVALID_CHG_TEMP_ST == stm_status)
        || (CHG_STM_BATTERY_ONLY == stm_status))
    {
        bat_stat_t = POWER_SUPPLY_STATUS_NOT_CHARGING;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=NOT_CHARGING\n");
    }
    else if ((CHG_STM_FAST_CHARGE_ST == stm_status) || (CHG_STM_WARMCHG_ST == stm_status))
    {
        bat_stat_t = POWER_SUPPLY_STATUS_CHARGING;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=CHARGING\n");
    }
    else if(CHG_STM_MAINT_ST == stm_status)
    {
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
        #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
        bat_stat_t = POWER_SUPPLY_STATUS_FULL;
        #else
        if(BATT_CAPACITY_FULL == chg_get_sys_batt_capacity())
        {
            bat_stat_t = POWER_SUPPLY_STATUS_FULL;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_FULL\n");
        }
        #endif
#else
        bat_stat_t = POWER_SUPPLY_STATUS_FULL;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_FULL\n");
#endif
    }
    else
    {
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
        if(POWER_SUPPLY_STATUS_NEED_SUPPLY == chg_stm_state_info.bat_stat_type)
        {
            bat_stat_t = POWER_SUPPLY_STATUS_NEED_SUPPLY;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=NEED_SUPPLY\n");
        }
        else if(POWER_SUPPLY_STATUS_SUPPLY_SUCCESS == chg_stm_state_info.bat_stat_type)
        {
            bat_stat_t = POWER_SUPPLY_STATUS_SUPPLY_SUCCESS;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=SUPPLY_SUCCESS\n");
        }
        else
        {
            bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_UNKNOWN\n");
        }
#else
        bat_stat_t = POWER_SUPPLY_STATUS_UNKNOWN;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_status=STATUS_UNKNOWN\n");
#endif
    }
    return bat_stat_t;
}


int32_t chg_get_bat_health(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_health=%d\n",chg_stm_state_info.bat_heath_type);
    return chg_stm_state_info.bat_heath_type;
}


int32_t chg_get_extchg_status(void)
{
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:extchg_status=%d\n",chg_stm_state_info.extchg_status);
    return chg_stm_state_info.extchg_status;
}
/*****************************************************************************
 函 数 名  : chg_stm_init
 功能描述  : Charge State Machine Sub-Module initialize procedure.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : CHG_STM_SUCCESS for initialize successful
             CHG_STM_FAILED  for initialize failed
 CALL-WHOM :
 WHO-CALLED: chg_switch_state
             chg_init(?)
 NOTICE    : Export Required.
*****************************************************************************/
int32_t chg_stm_init(void)
{
    static boolean chg_stm_inited = FALSE;
    chg_stm_state_type cur_chg_state = chg_stm_get_cur_state();
    chg_chgr_type_t cur_chg_type = chg_stm_get_chgr_type();
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
    chg_stm_state_type pre_chg_state = chg_stm_get_pre_state();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    boolean is_wireless_online   = chg_stm_get_wireless_online_st();
#endif/*MBB_CHG_WIRELESS*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    boolean is_extchg_online   = chg_stm_get_extchg_online_st();
#endif/*MBB_CHG_EXTCHG*/

    if (FALSE == chg_stm_inited)
    {
        /*Clear fast charge sw protection timer(cnter).*/

        chg_stm_state_info.charging_lasted_in_sconds = 0;
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
        chg_stm_state_info.charger_lasted_without_charging_in_seconds = 0;
#endif
        /*Initialize default chgr type and stm state. */
        if((cur_chg_mode <= CHG_MODE_INIT) || (cur_chg_mode >= CHG_MODE_UNKNOW))
        {
            chg_set_cur_chg_mode(CHG_MODE_UNKNOW);
        }
        if((cur_chg_type <= CHG_CHGR_UNKNOWN) || (cur_chg_type >= CHG_CHGR_INVALID))
        {
            chg_stm_set_chgr_type(CHG_CHGR_UNKNOWN);
        }
        if((cur_chg_state <= CHG_STM_INIT_ST) || (cur_chg_state >= CHG_STM_MAX_ST))
        {
            chg_stm_set_cur_state(CHG_STM_TRANSIT_ST);
        }

#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)
        if((pre_chg_state <= CHG_STM_INIT_ST) || (pre_chg_state >= CHG_STM_MAX_ST))
        {
            chg_stm_set_pre_state(CHG_STM_TRANSIT_ST);
        }
#endif /* MBB_CHG_WARM_CHARGE */

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        if(FALSE == is_5577cs_falg)
        {
            if(TRUE != is_extchg_online)
            {
                chg_stm_set_extchg_online_st(FALSE);
            }
        }
        else
        {
            chg_stm_set_extchg_online_st(FALSE);
        }
#endif /* MBB_CHG_EXTCHG */

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE != is_wireless_online)
        {
            chg_stm_set_wireless_online_st(FALSE);
        }
#endif /* MBB_CHG_WIRELESS */

        if(CHG_STM_INIT_ST == cur_chg_state)
        {
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
        mlog_print(MLOG_CHG, mlog_lv_info, "CHG STM initial state:\n");
        mlog_print(MLOG_CHG, mlog_lv_info, "current charge state: %d, pre-chargestate: %d.\n",
                   cur_chg_state, pre_chg_state);
        mlog_print(MLOG_CHG, mlog_lv_info, "charger type: %d, charge mode: %d.\n",
                   cur_chg_type, cur_chg_mode);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
        mlog_print(MLOG_CHG, mlog_lv_info, "extchg_online: %d.\n", is_extchg_online);
#endif /*MBB_CHG_EXTCHG*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        mlog_print(MLOG_CHG, mlog_lv_info, "wireless_online: %d.\n", is_wireless_online);
#endif /*MBB_CHG_WIRELESS*/

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge State Machine init successfully.\n");
        chg_stm_inited = TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:Charge STM init be called more than one time.\n");
    }

    return CHG_OK;
}


void chg_set_usb_online_status(boolean online)
{
    chg_stm_state_info.usb_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_state_info.usb_online_st=%d\n",online);
}


boolean chg_get_usb_online_status(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_get_usb_online_status=%d\n",\
                chg_stm_state_info.usb_online_st);
    return chg_stm_state_info.usb_online_st;
}


void chg_set_ac_online_status(boolean online)
{
    chg_stm_state_info.ac_online_st = online;
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_stm_state_info.ac_online_st=%d\n",online);
}

boolean chg_get_ac_online_status(void)
{
    chg_print_level_message(CHG_MSG_INFO, "CHG_STM:chg_get_ac_online_status=%d\n",\
                chg_stm_state_info.ac_online_st);
    return chg_stm_state_info.ac_online_st;
}


boolean chg_get_bat_present_status(void)
{
    if(TRUE == chg_get_batt_id_valid())
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_present_status=PRESENT\n");
        return PRESENT;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bat_present_status=UNPRESENT\n");
        return UNPRESENT;
    }
    return PRESENT;
}


void chg_update_power_suply_info(void)
{

   if (NULL == g_chip)
   {
       return;
   }
   g_chip->ac_online = chg_get_ac_online_status();
   g_chip->usb_online = chg_get_usb_online_status();
#if (MBB_CHG_WIRELESS == FEATURE_ON)
   g_chip->wireless_online = chg_stm_get_wireless_online_st();
#endif/*MBB_CHG_WIRELESS*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
   g_chip->extchg_online = chg_stm_get_extchg_online_st();
   g_chip->extchg_status = chg_get_extchg_status();
#endif/*MBB_CHG_EXTCHG*/
   g_chip->bat_present = chg_get_bat_present_status();
   g_chip->bat_stat = chg_get_bat_status();
   g_chip->bat_health = chg_get_bat_health();
   g_chip->bat_technology = POWER_SUPPLY_TECHNOLOGY_LION;
   g_chip->bat_avg_voltage = chg_get_sys_batt_volt();
   g_chip->bat_avg_temp = chg_get_sys_batt_temp();
   g_chip->bat_capacity = chg_get_sys_batt_capacity();
   g_chip->bat_time_to_full = chg_get_batt_time_to_full();
/*USB温保*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
   g_chip->usb_health = chg_get_usb_health();
#endif
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
   g_chip->battery_average_current = chg_get_batt_current_average();
#endif
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:update_power_suply_info successfully!\n");
}

void chg_print_info_on(void)
{
    chg_print_flag = TRUE;
}
void chg_print_info_off(void)
{
    chg_print_flag = FALSE;
}

void coul_print_info_on(void)
{
    coul_print_flag = TRUE;
}
void coul_print_info_off(void)
{
    coul_print_flag = FALSE;
}


void chg_print_test_view_info(void)
{
    static uint8_t count = 0;
    if(count >= 3) /*状态机轮训2次即6S打印一次充电信息防止打印信息太多*/
    {
        
        count = 0;
        if(FALSE == chg_print_flag)
        {
            return;
        }
        
        chg_print_level_message(CHG_MSG_ERR, "**************CHG Tester View Info Begin**********\n");
        /*打印当前所处的充电状态*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_stm_state=%d\n",
                             chg_stm_state_info.cur_stm_state);
        /*打印当前充电器类型*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_charger_type=%d\n",
                            chg_stm_state_info.cur_chgr_type);
        /*打印当前所处的充电模式，有线，无线还是对外充电*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_chg_mode=%d\n",
                            chg_stm_state_info.cur_chg_mode);
        /*打印当前充电持续时间*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:current_charging_lasted_time=%d\n",
                            chg_stm_state_info.charging_lasted_in_sconds);
        /*打印单次电池温度*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_one_temp = %d\n",
                            chg_real_info.battery_one_temp);
         /*打印平滑后的电池温度*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_temp = %d\n",
                            chg_real_info.battery_temp);

        /*打印电池单次采集电压*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_one_volt = %d\n",
                            chg_real_info.battery_one_volt);
        /*打印电池电压平滑值*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.battery_volt = %d\n",
                            chg_real_info.battery_volt);

        /*打印电池电量百分比*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_real_info.bat_capacity = %d\n",
                            chg_real_info.bat_capacity);
/*打印当前满电停冲后充电器在位时长*/
#if ( FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT )
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:charger_lasted_without_charging_in_seconds=%d\n",
                            chg_stm_state_info.charger_lasted_without_charging_in_seconds);
#endif
        /*打印USB当前温度*/ 
        #if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:chg_usb_cur_temp = %d'C.\n",
                                chg_usb_temp_info.usb_cur_temp);
        #endif
        chg_print_level_message(CHG_MSG_ERR, "**************CHG Tester View Info End**********\n");

    }
    else
    {
        count++;
    }
}

#ifdef CONFIG_MBB_FAST_ON_OFF
#if(MBB_REB == FEATURE_ON)
/*****************************************************************************
 函 数 名  : unsigned int chg_is_bat_only(void)
 功能描述  : 软件可靠性-查看是否是单电池在位
 输入参数  : none
 输出参数  : none
 返 回 值  : 1-单电池在位 0-USB/DC/wireless
*****************************************************************************/
unsigned int chg_is_bat_only(void)
{
    return ( CHG_BAT_ONLY_MODE == chg_stm_state_info.cur_chg_mode );
}

/*****************************************************************************
 函 数 名  : get_low_bat_level
 功能描述  : 获取产品电池低电电压值(该接口暂时打桩,后面要放到充放电模块中)
 输入参数  : none
 输出参数  : none
 返 回 值  : 电池电压值(mv)
*****************************************************************************/
int get_low_bat_level(void)
{
    /*要在充电模块初始化以后调用*/
    return g_chgBattVoltProtect.VbatLevelLow_MAX;
}
#endif/*CONFIG_MBB_REB*/


void chg_get_system_suspend_status(ulong64_t suspend_status)
{
#if (MBB_CHG_EXTCHG == FEATURE_ON)
    if(suspend_status)
    {
        mbb_fastonoff_chg_flag = true;
        /*进入假关机，模拟USB ID拔出*/
        if(TRUE == chg_stm_get_extchg_online_st())
        {
            #ifdef BSP_CONFIG_BOARD_E5771S_852
            #else
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Enter Fastboot extchg online stop extchg!\n");
            /*模拟USB ID拔出*/
            (void)battery_monitor_blocking_notifier_call_chain(0, CHG_EXGCHG_CHGR);
            #endif
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Enter Fastboot extchg offline do nothing!\n");
        }
    }
    else
    {
        mbb_fastonoff_chg_flag = false;
        /*退出假关机，由USB重新上报 ID中断，充电模块不做处理*/
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:Exit Fastboot extchg offline do nothing!\n");
    }
#endif
}
#endif/*CONFIG_MBB_FAST_ON_OFF*/

/**********************************************************************
函 数 名: chg_low_battery_event_handler
功能描述: 收到库仑计低电事件后的处理函数
输入参数: None
输出参数: None
返 回 值: 无。
注意事项: Linux系统中，调用该函数时须判断是否还有事件没有处理完成
***********************************************************************/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define LOW_BATT_DET_THRESHOLD    (3450)/*低电中断误报门限*/
void chg_low_battery_event_handler(void)
{
    int32_t batt_volt = chg_get_batt_volt_value();

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if(TRUE == chg_is_powdown_charging() && gpio_get_value(EXTCHG_OTG_DET_GPIO))
#else
    if(TRUE == chg_is_powdown_charging())
#endif
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:no need to care，set bypass!\n");
        return;        
    }
    if(LOW_BATT_DET_THRESHOLD < batt_volt)
    {
        batt_volt = chg_get_batt_volt_value();
        if(LOW_BATT_DET_THRESHOLD < batt_volt)
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:invalid event，set bypass!\n");
            return;
        }
    }
    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:battery is low power，system will shutdown!\n");
    wake_lock_timeout(&g_chip->alarm_wake_lock,(long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));  
    chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    chg_bat_timer_set( LOW_BATT_SHUTOFF_DURATION, chg_set_power_off, DRV_SHUTDOWN_LOW_BATTERY);  
}
#endif
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#define BOOST_DELAY_TIMER  50
boolean extchg_monitor_func_in_sleep_mode(int32_t bat_vol, int32_t bat_temp);
int32_t batt_revise_calc_average_volt_value(int32_t new_data);
int32_t batt_revise_calc_average_temp_value(int32_t new_data);
int32_t batt_revise_calc_average_curr_value(int32_t new_data);
#endif
/**********************************************************************
函 数 名: chg_sleep_batt_check_timer
功能描述: 充电任务处于休眠状态时被timer唤醒后检测电池电压温度等
输入参数: None
输出参数: 接收的事件
返 回 值: 无。
注意事项: Linux系统中，调用该函数时须判断是否还有事件没有处理完成
***********************************************************************/
void chg_sleep_batt_check_timer(struct work_struct *work)
{
    int32_t batt_volt = 0;
    int32_t batt_temp = 0;
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    static int32_t init_flag = FALSE;
    int32_t batt_volt_ave = 0;
    boolean low_bat_flag = FALSE;
#if defined(BSP_CONFIG_BOARD_E5771H_937)
    int ret = 0;
    int32_t volt_of_short = 0;
    int32_t batt_soc = 0;
#endif
#endif

    #ifdef CONFIG_COUL
    /*进行一次库仑计强制校准*/
    chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:bsp_coul_cail_on COUL_CAIL_ON");
    bsp_coul_cail_on();
    msleep(COUL_READY_DELAY_MS);
    #endif
    batt_volt = (int32_t)chg_get_batt_volt_value();
    batt_temp = (int32_t)chg_get_temp_value(CHG_PARAMETER__BATT_THERM_DEGC);
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    batt_temp = chg_batt_temp_revise(batt_temp);
    batt_volt_ave = chg_get_sys_batt_volt();
#endif

    chg_print_level_message(CHG_MSG_ERR,"CHG_STM:modem_notify_get_batt_info_callback, batt_volt %d,batt_temp %d!\n",
           batt_volt, batt_temp);

#if (MBB_CHG_EXTCHG == FEATURE_ON)
    /*有线/无线/对外充电器在位不进行低电或者过温处理*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if (FALSE == is_chg_charger_removed())
#else
    if ((FALSE == is_chg_charger_removed()) || (TRUE == chg_stm_get_extchg_online_st()))
#endif
    {
#if defined(BSP_CONFIG_BOARD_E5771H_937)
        /*E5771h-937关机对外充电低于2.94V，BOOST关闭，单板关机。保持负载依然在位，插入内充，达到soc 11%的恢复门限恢复不了对外充电的功能。
          插入内充后，在chg_extchg_monitor_func函数中检测到外充电路短路一定次数后会启动定时器，
          进入到chg_sleep_batt_check_timer函数，此时对内充电器在位，会进入到此分支，因此需要对其进行判断，11%的时候强制打开BOOST*/
        /*通过HKADC8读取电压*/
        ret = bsp_hkadc_convert(EXTCHG_SHORT_VOLT_CHAN, &volt_of_short);
        if(ret)
        {
            chg_print_level_message(CHG_MSG_ERR, "bsp_hkadc_convert is error!!!\n");
        }

        /*当对外充电的电压小于等于0时，进行boost检测，看是否需要重启对外充电监控定时器*/
        if(volt_of_short <= 0)
        {
            chg_print_level_message(CHG_MSG_ERR, "enter test branch!!!\n");

            /*判断电池容量大于10%*/
            batt_soc = chg_get_sys_batt_capacity();
            if(batt_soc > EXTCHG_STOP_CAPACITY_TEN)
            {
                /*强制打开boost*/
                gpio_direction_output(RE_ILIM_1A_GPIO, 1);
                extchg_set_charge_enable(TRUE);
                mdelay(BOOST_DELAY_TIMER);

                /*再次检测对外充电是否短路*/
                if(extchg_is_perph_circult_short())  
                {
                    /*对外充电短路，则关闭boost*/
                    extchg_set_charge_enable(FALSE);
                    chg_print_level_message(CHG_MSG_ERR, "extchg always short!!!\n");
                }
                else
                {                   
                    extchg_short_flag = FALSE;
                    /*此处对ext_chg_status_old变量初始化为默认值，防止恢复门限为1A的场景*/
                    ext_chg_status_old = RE_ILIM_NA;

                    /*重新启动对外充电的监控定时器*/
                    schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
                    chg_print_level_message(CHG_MSG_ERR, "extchg not short anymore, restart extchg monitor timer!!!\n");
                }
            }
            else
            {
                //do nothing
            }
        }
#endif
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:modem_notify_get_batt_info_callback charger plug in do nothing!*****\n");
        goto end;
    }
#else
    if (FALSE == is_chg_charger_removed())
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:modem_notify_get_batt_info_callback charger plug in do nothing!*****\n");
        goto end;
    }
#endif/*defined(MBB_CHG_EXTCHG)*/
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    /*轮询soc，以便应用低电提示*/
    chg_poll_batt_soc();
    if(batt_volt < BATT_VOLT_POWER_OFF_THR)
    {
        /*低电关机紧急状态，系统稍后关机*/
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,\
        "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    }
#elif (MBB_CHG_BQ27510 == FEATURE_ON)
    /*just monitor batt volt*/
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    #if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if(((batt_volt < BATT_VOLT_POWER_OFF_THR) || (batt_volt_ave < BATT_VOLTAGE_3350)) \
        && (FALSE == chg_is_powdown_charging()))
    {
            /*低电关机紧急状态，系统稍后关机*/
            wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
            low_bat_flag = TRUE;
            chg_print_level_message(CHG_MSG_ERR,\
                "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    } 
    #else  /* E5771s-852 有假关机所以不在关机门限附近低点告警不休眠。E5771h-937的情况待确认 */
       /*  do nothing */
    #endif
#else
    if(batt_volt < BATT_VOLT_POWER_OFF_THR)
    {
            /*低电关机紧急状态，系统稍后关机*/
            wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
            #if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
            low_bat_flag = TRUE;
            #endif
            chg_print_level_message(CHG_MSG_ERR,\
                "CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
    }  
#endif  
#else
    /* 如果电池电压大于低电门限则什么都不做，否则将相关事件上报应用 */
    if (BATT_VOLT_LEVELLOW_MAX < batt_volt)
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:batt_volt > low battery threshold do nothing!*****\n");
    }
    else if ((BATT_VOLT_POWER_OFF_THR < batt_volt) && (BATT_VOLT_LEVELLOW_MAX >= batt_volt))
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_volt is in 3450-3550mV report low battery*****\n");
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_set_sys_batt_capacity(BATT_CAPACITY_LEVELLOW);
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:send MSG to app for show low power! \n ");
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:batt_volt < poweroff voltage threshold report power off*****\n");
        chg_set_sys_batt_capacity(BATT_CAPACITY_SHUTOFF);
    }
#endif
    /* 如果温度保护不使能 或者 电池温度在正常范围则什么都不做，否则将相关事件上报应用 */
    #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    if((FALSE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE) || (FALSE == SHUTOFF_OVER_TEMP_PROTECT_ENABLE))
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Batt_temp protect is disable \n");
    }
    else if (((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) > batt_temp) \
        && (SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD < batt_temp))
    {
        if(TRUE == chg_is_powdown_charging())
        {    /* 关机充电是过温恢复上报  */
            if(batt_temp > SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD + SHUTOFF_HIGH_TEMP_WARN_LEN \
                && low_over_temp_shutoff_falg == TRUE)
            {
                low_over_temp_shutoff_falg = FALSE;
                wake_lock_timeout(&g_chip->alarm_wake_lock,
                    (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:pd charge send app low temp back off\n ");
            }
            else if(up_over_temp_shutoff_falg == TRUE)
            {
                up_over_temp_shutoff_falg = FALSE;
                wake_lock_timeout(&g_chip->alarm_wake_lock,
                    (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
                chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_GOOD;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_NORMAL);
                chg_print_level_message(CHG_MSG_ERR, "CHG_STM:pd charge send app high temp back off\n ");
            }
            else
            {
                /* do nothing */
            }
        }
    }
    #else
    if ((((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) > batt_temp)
        && (SHUTOFF_LOW_TEMP_SHUTOFF_THRESHOLD < batt_temp))
        || (FALSE == SHUTOFF_LOW_TEMP_PROTECT_ENABLE) 
            || (FALSE == SHUTOFF_OVER_TEMP_PROTECT_ENABLE))
    {
        chg_print_level_message(CHG_MSG_INFO,"CHG_STM:Batt_temp is in -20-58 deg do nothing*****\n");
    }
    #endif
    else if ((SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD - SHUTOFF_HIGH_TEMP_WARN_LEN) <= batt_temp)
    {
        if (SHUTOFF_OVER_TEMP_SHUTOFF_THRESHOLD <= batt_temp)
        {
            wake_lock_timeout(&g_chip->alarm_wake_lock,
                (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
            #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
            if(TRUE == chg_is_powdown_charging())
            {   /* 关机充电>=65度上报  */
                up_over_temp_shutoff_falg = TRUE;
            }
            #endif
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is more than 60 deg, power off*****\n");
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_DEAD;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_KEY, (uint32_t)GPIO_KEY_POWER_OFF);
            
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is in 58-60 deg, report overheat*****\n");
            wake_lock_timeout(&g_chip->alarm_wake_lock,
                (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
            chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_OVERHEAT;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_HIGH);
            chg_print_level_message(CHG_MSG_ERR,"CHG_STM:send MSG to app for show overheat! \n ");
        }
    }
    else
    {
        wake_lock_timeout(&g_chip->alarm_wake_lock,
            (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
        #if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
        if(TRUE == chg_is_powdown_charging())
        {   /* 关机充电<=-20度上报  */
            low_over_temp_shutoff_falg = TRUE;
        }
        #endif
        chg_print_level_message(CHG_MSG_ERR,"CHG_STM:Batt_temp is less than -20 deg, report cold*****\n");
        chg_stm_state_info.bat_heath_type = POWER_SUPPLY_HEALTH_COLD;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_TEMP, (uint32_t)TEMP_BATT_LOW);
    }
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if(FALSE == init_flag)
    {
        battery_voltage = batt_volt;
        battery_temperature = batt_temp;
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
        battery_current = hisi_battery_current();
#endif
        init_flag = TRUE;
    }

    battery_voltage = batt_revise_calc_average_volt_value(batt_volt);
    battery_temperature = batt_revise_calc_average_temp_value(batt_temp);
#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    battery_current = batt_revise_calc_average_curr_value(hisi_battery_current());

    if (battery_current_prev != battery_current)
    {
        battery_current_prev = battery_current;
        chg_real_info.bat_current_avg = battery_current;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, \
            (uint32_t)CHG_EVENT_NONEED_CARE);
    }
#endif
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if (FALSE == low_bat_flag)
    {
        if (extchg_monitor_func_in_sleep_mode(batt_volt, batt_temp))
        {
            bsp_softtimer_modify(&g_chg_sleep_timer, FAST_POLL_CYCLE_OF_EXTCHG);
            chg_print_level_message(CHG_MSG_ERR, \
                "CHG_STM:bsp_softtimer_modify to FAST_POLL_CYCLE!\n");
        }
        else
        {
            bsp_softtimer_modify(&g_chg_sleep_timer, SLOW_POLL_CYCLE_OF_EXTCHG);
            chg_print_level_message(CHG_MSG_ERR, \
                "CHG_STM:bsp_softtimer_modify to SLOW_POLL_CYCLE!\n");
        }
    }
    else
    {
        bsp_softtimer_modify(&g_chg_sleep_timer, FAST_POLL_CYCLE_OF_EXTCHG);
    }
#endif
    chg_print_level_message(CHG_MSG_INFO,"CHG_STM:wait next modem time expire!\n");

end:
#if (MBB_CHG_PLATFORM_V7R2 == FEATURE_ON)
    wake_unlock(&g_chip->chg_wake_lock);
#endif
}

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)

int32_t batt_revise_calc_average_curr_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_curr_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE_OF_EXTCHG;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_REVISE_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_REVISE_SAMPLE_SLOW] = {0};

    /*查询轮询模式*/
    new_poll_mode = g_chg_sleep_timer.timeout;

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE_OF_EXTCHG == new_poll_mode)/*由慢轮询切换到当前的快轮询*/
        {
            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = battery_current;
            }
            record_avg_num_fast = CHG_BAT_TEMP_REVISE_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = battery_current;
            }
            record_avg_num_slow = CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "new_poll_mode = old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE_OF_EXTCHG == new_poll_mode)//快轮询模式
    {
        /*数组中当前元素标号小于6*/
        if(CHG_BAT_TEMP_REVISE_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_curr_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于6个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_REVISE_SAMPLE_FAST] 
                = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_curr_avg = sum / CHG_BAT_TEMP_REVISE_SAMPLE_FAST;

            if(CHG_BAT_TEMP_REVISE_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_REVISE_SAMPLE_FAST;
            }
        }
    }
    else//慢轮询模式
    {
        /*数组中当前元素标号小于3*/
        if(CHG_BAT_TEMP_REVISE_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_curr_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于3个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_REVISE_SAMPLE_SLOW] 
                = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_curr_avg = sum / CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;

            if(CHG_BAT_TEMP_REVISE_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;
            }
        }

    }

    return bat_curr_avg;
}

int32_t chg_batt_temp_revise(int32_t batt_temp)
{
#if defined(BSP_CONFIG_BOARD_E5771S_852)
    /* E5771s-852不区分场景温补，温度补偿数据无直接规律，直接针对具体温度进行补偿 */
    if((batt_temp <= 40) || (batt_temp == 52) || (batt_temp == 57) || (batt_temp == 58))
    {
        batt_temp += 5;
    }
    else if(((batt_temp >= 41) && (batt_temp <= 46)) || (batt_temp == 51) || (batt_temp >= 59))
    {
        batt_temp += 4;
    }
    else if(batt_temp == 47)
    {
        batt_temp += 3;
    }
    else if((batt_temp >= 48) && (batt_temp <= 50))
    {
        batt_temp += 2;
    }
    else if(batt_temp == 54)
    {
        batt_temp += 7;
    }
    else if((batt_temp == 55) || (batt_temp == 56) || (batt_temp == 53))
    {
        batt_temp += 6;
    }
    else
    {
    /*  */
    }

    return batt_temp;
#elif defined(BSP_CONFIG_BOARD_E5771H_937)  /*937实际测试需要两张表补偿*/

    /*1.对内充电场景*/
    if(FALSE == is_chg_charger_removed())
    {
        if(batt_temp >= 30 && batt_temp <= 34)
        {
            batt_temp += 11;
        }
        else if(batt_temp >= 35 && batt_temp <= 44)
        {
            batt_temp += 10;
        }
        else if(45 == batt_temp)
        {
            batt_temp += 9;
        }
        else if(batt_temp >= 46 && batt_temp <= 54)
        {
            batt_temp += 8;
        }
        else if(batt_temp >= 55 && batt_temp <= 57)
        {
            batt_temp += 7;
        }
        else if(batt_temp >= 58 && batt_temp <= 62)
        {
            batt_temp = 64;
        }
        else if(batt_temp >= 63 && batt_temp <= 65)
        {
            batt_temp += 1;
        }
        else
        {
            //do nothing
        }
    }
    /*2.其他场景*/
    else
    {
        if(batt_temp >= 30 && batt_temp <= 48)
        {
            batt_temp += 7;
        }
        else if(batt_temp >= 49 && batt_temp <= 52)
        {
            batt_temp += 6;
        }
        else if(batt_temp >= 53 && batt_temp <= 55)
        {
            batt_temp += 7;
        }
        else if(batt_temp >= 56 && batt_temp <= 58)
        {
            batt_temp += 6;
        }
        else if(batt_temp >= 59 && batt_temp <= 62)
        {
            batt_temp = 64;
        }
        else if(batt_temp >= 63 && batt_temp <= 65)
        {
            batt_temp += 2;
        }
    }
    return batt_temp;
#else
    if (batt_temp <= CHG_BAT_TEMP_REVISE_TEM_OF_NOREVISE)
    {
        return batt_temp;
    }

    if ((FALSE == is_chg_charger_removed()) && chg_stm_get_extchg_online_st())
    {
        if (CHG_WALL_CHGR == chg_stm_get_chgr_type())
        {
            batt_temp += 4;
        }
        else
        {
            //do nothing
        }
    }
    else if ((FALSE == is_chg_charger_removed()) && (!chg_stm_get_extchg_online_st()))
    {
        batt_temp += 7;
    }
    else
    {
        if (battery_current <= BATT_CURRENT_I_3000)
        {
            batt_temp = (batt_temp * 2) / 3 + 5;
        }
        else if ((battery_current > BATT_CURRENT_I_3000) && \
            (battery_current <= BATT_CURRENT_I_2500))
        {
            batt_temp -= 8;
        }
        else if ((battery_current > BATT_CURRENT_I_2500) \
            && (battery_current <= BATT_CURRENT_I_1500))
        {
            batt_temp -= 3;
        }
        else//battery_current > -1500
        {
            batt_temp += 2;
        }
    }

    return batt_temp;
#endif
}
#endif
#if (MBB_CHG_EXTCHG == FEATURE_ON)

void chg_extchg_info_dump(void)
{
    if(FALSE == chg_print_flag)
    {
        return;
    }
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:extchg_diable_st=%d\n",g_extchg_diable_st);    
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:ui_mode=%d\n",g_ui_choose_exchg_mode);
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:otg_extchg=%d\n",is_otg_extchg);    
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:is_extchg_ovtemp=%d\n",is_extchg_ovtemp);        
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:extchg_status=%d\n",chg_stm_state_info.extchg_status);
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:extchg_online_st=%d\n",chg_stm_state_info.extchg_online_st);    
    chg_print_level_message(CHG_MSG_ERR, "EXTCHG:exchg_enable=%d\n",g_exchg_enable_flag);     
} 
#if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
    ||defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
boolean extchg_circult_short_detect(void)
{
    /*检查VBUS状态是否复位*/
    chg_status_type  chg_stat = {0};
    boolean op_result = TRUE;
    op_result = chg_get_IC_status(&chg_stat);
    if (FALSE == op_result)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_DRV: chg_get_IC_status error !\n");
    }
    else
    {
        if (CHG_STAT_UNKNOWN == chg_stat.chg_vbus_stat)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_DRV: chg_get_IC_status IC status VBUS UNKOWN!\n");
            return TRUE;
        }
    }
    return FALSE;
}
#endif
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
void ext_chg_info_dump(void)
{
    if (FALSE == coul_print_flag)
    {
        return;
    }

    chg_print_level_message(CHG_MSG_ERR, "**************BQ27510 Tester \
        Info Begin**********\n");

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: TEMPERATURE = %d\n", hisi_bq27510_battery_temperature());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: VOLTAGE = %d\n", hisi_bq27510_battery_voltage());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: CURRENT = %d\n", hisi_bq27510_battery_current());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: capacity = %d\n", hisi_bq27510_battery_capacity());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: fcc = %d\n", hisi_bq27510_battery_fcc());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: rm = %d\n", hisi_bq27510_battery_rm());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: Qmax = %d\n", hisi_bq27510_battery_qmax());

    chg_print_level_message(CHG_MSG_ERR, \
        "INFO FOR COUL: is_battery_full = %d\n", hisi_is_bq27510_battery_full());

    chg_print_level_message(CHG_MSG_ERR, "**************BQ27510 Tester \
        Info End**********\n");
}


int32_t batt_revise_calc_average_volt_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_volt_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE_OF_EXTCHG;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_REVISE_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_REVISE_SAMPLE_SLOW] = {0};

    /*查询轮询模式*/
    new_poll_mode = g_chg_sleep_timer.timeout;

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE_OF_EXTCHG == new_poll_mode)/*由慢轮询切换到当前的快轮询*/
        {
            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = battery_voltage;
            }
            record_avg_num_fast = CHG_BAT_TEMP_REVISE_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = battery_voltage;
            }
            record_avg_num_slow = CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "new_poll_mode = old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE_OF_EXTCHG == new_poll_mode)//快轮询模式
    {
        /*数组中当前元素标号小于6*/
        if(CHG_BAT_TEMP_REVISE_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于6个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_REVISE_SAMPLE_FAST] 
                = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_volt_avg = sum / CHG_BAT_TEMP_REVISE_SAMPLE_FAST;

            if(CHG_BAT_TEMP_REVISE_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_REVISE_SAMPLE_FAST;
            }
        }
    }
    else//慢轮询模式
    {
        /*数组中当前元素标号小于3*/
        if(CHG_BAT_TEMP_REVISE_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于3个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_REVISE_SAMPLE_SLOW] 
                = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_volt_avg = sum / CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;

            if(CHG_BAT_TEMP_REVISE_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;
            }
        }

    }

    return bat_volt_avg;
}


int32_t batt_revise_calc_average_temp_value(int32_t new_data)
{
    int32_t    index = 0;
    int32_t    sum = 0;
    int32_t    bat_temp_avg = 0;
    int32_t    new_poll_mode;
    static int32_t  old_poll_mode = FAST_POLL_CYCLE_OF_EXTCHG;
    static uint32_t record_avg_num_fast = 0;
    static uint32_t record_avg_num_slow = 0;
    static int32_t record_value_fast[CHG_BAT_TEMP_REVISE_SAMPLE_FAST] = {0};
    static int32_t record_value_slow[CHG_BAT_TEMP_REVISE_SAMPLE_SLOW] = {0};

    /*查询轮询模式*/
    new_poll_mode = g_chg_sleep_timer.timeout;

    if(new_poll_mode != old_poll_mode)
    {
        if(FAST_POLL_CYCLE_OF_EXTCHG == new_poll_mode)/*由慢轮询切换到当前的快轮询*/
        {
            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_FAST;index++)
            {
                record_value_fast[index] = battery_temperature;
            }
            record_avg_num_fast = CHG_BAT_TEMP_REVISE_SAMPLE_FAST;
        }
        else//由快轮询切换到慢轮询
        {
            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;index++)
            {
                record_value_slow[index] = battery_temperature;
            }
            record_avg_num_slow = CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;
        }

        old_poll_mode = new_poll_mode;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "new_poll_mode = old_poll_mode!\n");
    }

    if(FAST_POLL_CYCLE_OF_EXTCHG == new_poll_mode)//快轮询模式
    {
        /*数组中当前元素标号小于6*/
        if(CHG_BAT_TEMP_REVISE_SAMPLE_FAST > record_avg_num_fast)
        {
            record_value_fast[record_avg_num_fast] = new_data;
            record_avg_num_fast++;

            for(index = 0;index < record_avg_num_fast; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / record_avg_num_fast;
        }
        else/*元素个数标号大于等于6个*/
        {
            record_value_fast[record_avg_num_fast % CHG_BAT_TEMP_REVISE_SAMPLE_FAST] 
                = new_data;
            record_avg_num_fast++;

            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_FAST; index++)
            {
                sum += record_value_fast[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_REVISE_SAMPLE_FAST;

            if(CHG_BAT_TEMP_REVISE_SAMPLE_FAST * 2 == record_avg_num_fast)
            {
                record_avg_num_fast = CHG_BAT_TEMP_REVISE_SAMPLE_FAST;
            }
        }
    }
    else//慢轮询模式
    {
        /*数组中当前元素标号小于3*/
        if(CHG_BAT_TEMP_REVISE_SAMPLE_SLOW > record_avg_num_slow)
        {
            record_value_slow[record_avg_num_slow] = new_data;
            record_avg_num_slow++;

            for(index = 0;index < record_avg_num_slow; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / record_avg_num_slow;
        }
        else/*元素个数标号大于等于个*/
        {
            record_value_slow[record_avg_num_slow % CHG_BAT_TEMP_REVISE_SAMPLE_SLOW] 
                = new_data;
            record_avg_num_slow++;

            for(index = 0;index < CHG_BAT_TEMP_REVISE_SAMPLE_SLOW; index++)
            {
                sum += record_value_slow[index];
            }

            bat_temp_avg = sum / CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;

            if(CHG_BAT_TEMP_REVISE_SAMPLE_SLOW * 2 == record_avg_num_slow)
            {
                record_avg_num_slow = CHG_BAT_TEMP_REVISE_SAMPLE_SLOW;
            }
        }

    }

    return bat_temp_avg;
}


int extchg_set_charge_enable(boolean enable)
{
    int ret = 0;
#if defined(BSP_CONFIG_BOARD_E5771H_937)
/* E5771h-937产品硬件新版本修改控制BOOST输出GPIO_2_08拉低位为打开,通过BOOST分压电阻区分新老硬件*/
    int volt_of_boost = 0;
    bsp_hkadc_convert(EXTCHG_SHORT_VOLT_CHAN, &volt_of_boost);
    if(volt_of_boost > HW_VER_DISTINGUISH)
    {
        ret = gpio_direction_output(EXTCHG_CHG_ENABLE, enable);
    }
    else
    {
        if(FALSE == enable)
        {
            ret = gpio_direction_output(EXTCHG_CHG_ENABLE, 1);
        }
        else
        {
            ret = gpio_direction_output(EXTCHG_CHG_ENABLE, 0);
        }
    }
#else
    ret = gpio_direction_output(EXTCHG_CHG_ENABLE, enable);
#endif

    if(ret)
    {
        chg_print_level_message(CHG_MSG_ERR,"EXTCHG: %s EXTCHG_CHG_ENABLE failed!!\r\n ",\
                                             (enable?"Enable":"Disable"));
        return -1;
    }

    chg_print_level_message(CHG_MSG_DEBUG,"EXTCHG: %s EXTCHG_CHG_ENABLE success!!\r\n ",\
                                           (enable?"Enable":"Disable"));
    return 0;
}

#if defined(BSP_CONFIG_BOARD_E5771S_852)
void extchg_set_cur_level(EXTCHG_ILIM curr)
{
    if (RE_ILIM_500mA == curr)
    {   /*  E5771s-852产品上500mA与1A相同操作，实际1A */
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 0);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 500mA!!\r\n ");
    }
    else if(RE_ILIM_1A == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 0);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 1000mA!!\r\n ");
    }
    else if(RE_ILIM_2A == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 1);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 2000mA!!\r\n ");
    }
    else
    {
        //do nothing
    }
}
#elif defined(BSP_CONFIG_BOARD_E5_E5770s)  || defined(BSP_CONFIG_BOARD_E5771H_937)
/* E5771h-937的限流档和E5770一致，有500/1A/2A */
void extchg_set_cur_level(EXTCHG_ILIM curr)
{
    if (RE_ILIM_500mA == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 0);
        gpio_direction_output(RE_ILIM_2A_GPIO, 0);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 500mA!!\r\n ");
    }
    else if(RE_ILIM_1A == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 0);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 1000mA!!\r\n ");
    }
    else if(RE_ILIM_2A == curr)
    {
        gpio_direction_output(RE_ILIM_1A_GPIO, 1);
        gpio_direction_output(RE_ILIM_2A_GPIO, 1);
        chg_print_level_message( CHG_MSG_DEBUG, \
            "EXTCHG: extcharge current limit to 2000mA!!\r\n ");
    }
    else
    {
        //do nothing
    }
}
#else
void extchg_set_cur_level(EXTCHG_ILIM curr)
{
    return; /*  do nothing */
}
#endif

boolean extchg_is_perph_circult_short(void)
{
    int32_t volt_of_short = 0;
    int32_t volt_of_short_original = 0;
    int ret = 0;

    ret = bsp_hkadc_convert(EXTCHG_SHORT_VOLT_CHAN, &volt_of_short);
    if(ret)
    {
        chg_print_level_message( CHG_MSG_ERR,"EXTCHG: detect volt_of_short failed, \
                                              treat as short!!!\r\n ",0,0,0 );
        return TRUE;
    }
    chg_print_level_message( CHG_MSG_DEBUG, \
        "EXTCHG:volt_of_short = %d\n", volt_of_short,0,0);
#if defined(BSP_CONFIG_BOARD_E5771H_937)
    volt_of_short_original = (volt_of_short * RSIISTANCE_CENT_COUNT);//采集的电压为分压后的值，还原为原始值
#else
    volt_of_short_original = (volt_of_short * 151) / 51;//采集的电压为分压后的值，还原为原始值
#endif
    if (volt_of_short_original <= EXTCHG_SHORT_VOLT_THRESHOLD)
    {
        chg_print_level_message( CHG_MSG_ERR, \
                "EXTCHG: perph_circult is short!\
                        valtage = %d\n", volt_of_short_original,0,0);
        return TRUE;
    }
    else
    {
        chg_print_level_message( CHG_MSG_DEBUG,"EXTCHG: perph_circult is ok!\\n", 0,0,0);
        return FALSE;
    }
}

int usb_direction_flag_set(boolean flag)
{
    usb_set_direction_flag = flag;
    return flag;
}


void extchg_release_pwr_key(void)
{
    mdelay(500);
    gpio_direction_output(OTG_ON_CTRL_GPIO, 1);
    mdelay(500);
    gpio_direction_output(OTG_ON_CTRL_GPIO, 0);
}

static irqreturn_t extchg_plug_in_out_isr(int ext_dev_id)
{
    if(!gpio_int_state_get(EXTCHG_OTG_DET_GPIO))
    {
        return IRQ_NONE;
    }

    gpio_int_mask_set(EXTCHG_OTG_DET_GPIO);
    gpio_int_state_clear(EXTCHG_OTG_DET_GPIO);

    if(!gpio_get_value(EXTCHG_OTG_DET_GPIO))
    {
        schedule_work(&extchg_plug_in_work);
        gpio_int_trigger_set(EXTCHG_OTG_DET_GPIO, IRQ_TYPE_LEVEL_HIGH);
    }
    else
    {
        schedule_work(&extchg_plug_out_work);
        gpio_int_trigger_set(EXTCHG_OTG_DET_GPIO, IRQ_TYPE_LEVEL_LOW);
    }

    gpio_int_unmask_set(EXTCHG_OTG_DET_GPIO);
    return IRQ_HANDLED;
}

int extchg_gpio_isr_init(void)
{
    //对于有A型USB母口的设备，对外充电检测GPIO的初始化
    int ret = 0;
    nv_huawei_coul_firmware_update_info coul_firmware_update_info = {0x00};
    unsigned char battery_info[3] = {0x00};

#if defined(BSP_CONFIG_BOARD_E5_E5770s) || defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
    battery_current = hisi_battery_current();
    chg_real_info.bat_current_avg = battery_current;
#endif

    INIT_WORK(&extchg_plug_in_work, chg_extchg_insert_proc);
    INIT_WORK(&extchg_plug_out_work, chg_extchg_remove_proc);

    ret = gpio_direction_input(EXTCHG_OTG_DET_GPIO);
    if(ret)
    {
        chg_print_level_message( CHG_MSG_ERR, \
            "EXTCHG: EXTCHG_OTG_DET_GPIO gpio_direction_input error!!\r\n ");
        gpio_free(EXTCHG_OTG_DET_GPIO);
        return -1;
    }

    gpio_int_mask_set(EXTCHG_OTG_DET_GPIO);
    gpio_int_trigger_set(EXTCHG_OTG_DET_GPIO, IRQ_TYPE_LEVEL_LOW);
    gpio_set_function(EXTCHG_OTG_DET_GPIO, GPIO_INTERRUPT);

    ret = request_irq((unsigned int)gpio_to_irq(EXTCHG_OTG_DET_GPIO), \
            extchg_plug_in_out_isr,\
                      IRQF_NO_SUSPEND | IRQF_SHARED, "ext_a_chg_gpio_irq", &ext_dev_id);
    if (ret) {
        chg_print_level_message( CHG_MSG_ERR, \
            "EXTCHG: OTG_DET_GPIO request_irq error, errno = %d!!\r\n ", ret);
        free_irq((unsigned int)gpio_to_irq(EXTCHG_OTG_DET_GPIO), NULL);
        gpio_free(EXTCHG_OTG_DET_GPIO);
        return -1;
    }

    gpio_int_state_clear(EXTCHG_OTG_DET_GPIO);
    gpio_int_unmask_set(EXTCHG_OTG_DET_GPIO);
    enable_irq_wake(gpio_to_irq(EXTCHG_OTG_DET_GPIO));

    bsp_nvm_read(NV_HUAWEI_BATTERY_INFO_I,\
                    &coul_firmware_update_info,\
                        sizeof(nv_huawei_coul_firmware_update_info));
    strncpy(&battery_info,&coul_firmware_update_info.battery_sn[4],2);

    extchg_stop_threshold_powon = EXTCHG_STOP_THRESHOLD_POWON;
    extchg_stop_threshold_powdown = EXTCHG_STOP_THRESHOLD_POWDOWN;
    extchg_recharge_threshold = EXTCHG_RECHG_THRESHOLD;

#if defined(BSP_CONFIG_BOARD_E5_E5770s) 
    if(strstr(battery_info,"LC") != NULL)
    {
        extchg_limit_1a_capacity = EXTCHG_STOP_CAPACITY_THIRTYSIX;
        tbatt_supply_volt_threshold = LG_TBATT_SUPPLY_VOLT;
        tbatt_dischg_volt_threshold = LG_TBATT_DISCHG_VOLT;
        tbatt_supply_stop_threshold = LG_TBATT_SUPPLY_STOP_VOLT;
        tbatt_dischg_stop_threshold = LG_TBATT_DISCHG_STOP_VOLT;
    }
    else if(strstr(battery_info,"YI") != NULL)
    {
        extchg_limit_1a_capacity = EXTCHG_STOP_CAPACITY_EIGHTEEN;
        tbatt_supply_volt_threshold = SY_TBATT_SUPPLY_VOLT;
        tbatt_dischg_volt_threshold = SY_TBATT_DISCHG_VOLT;
        tbatt_supply_stop_threshold = SY_TBATT_SUPPLY_STOP_VOLT;
        tbatt_dischg_stop_threshold = SY_TBATT_DISCHG_STOP_VOLT;
    }
    else
    {
        extchg_limit_1a_capacity = EXTCHG_STOP_CAPACITY_THIRTYSIX;
        tbatt_supply_volt_threshold = SY_TBATT_SUPPLY_VOLT;
        tbatt_dischg_volt_threshold = SY_TBATT_DISCHG_VOLT;
        tbatt_supply_stop_threshold = SY_TBATT_SUPPLY_STOP_VOLT;
        tbatt_dischg_stop_threshold = SY_TBATT_DISCHG_STOP_VOLT;
    }
    extchg_limit_500ma_capacity = EXTCHG_STOP_CAPACITY_FIFTHTEEN;
#elif defined(BSP_CONFIG_BOARD_E5771S_852)  || defined(BSP_CONFIG_BOARD_E5771H_937)
/*  E5771 的电池补电数据，也要根据电池厂家区分 ，下面的数据待刷新 */
     extchg_limit_1a_capacity = EXTCHG_STOP_CAPACITY_THIRTYSIX;
     tbatt_supply_volt_threshold = SY_TBATT_SUPPLY_VOLT;
     tbatt_dischg_volt_threshold = SY_TBATT_DISCHG_VOLT;
     tbatt_supply_stop_threshold = SY_TBATT_SUPPLY_STOP_VOLT;
     tbatt_dischg_stop_threshold = SY_TBATT_DISCHG_STOP_VOLT;
     extchg_limit_500ma_capacity = EXTCHG_STOP_CAPACITY_TWENTYFIVE;
#else
     extchg_limit_1a_capacity = EXTCHG_STOP_CAPACITY_THIRTYSIX;
     tbatt_supply_volt_threshold = SY_TBATT_SUPPLY_VOLT;
     tbatt_dischg_volt_threshold = SY_TBATT_DISCHG_VOLT;
     tbatt_supply_stop_threshold = SY_TBATT_SUPPLY_STOP_VOLT;
     tbatt_dischg_stop_threshold = SY_TBATT_DISCHG_STOP_VOLT;
     extchg_limit_500ma_capacity = EXTCHG_STOP_CAPACITY_FIFTHTEEN;
#endif
#ifdef BSP_CONFIG_BOARD_E5_E5770s
    if (FALSE == chg_is_powdown_charging())
    {
        if (hisi_battery_capacity() <= extchg_stop_threshold_powon)
        {
            ext_chg_status_new = RE_ILIM_STOP;
        }
        else
        {
            //do nothing
        }
    }
    else
    {
        if (hisi_battery_capacity() <= extchg_stop_threshold_powdown)
        {
            ext_chg_status_new = RE_ILIM_STOP;
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_LOWPOWER_STOP_CHARGING;
        }
        else
        {
            //do nothing
        }
    }
#endif
    if(FALSE == chg_is_ftm_mode())
    {
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }

    return 0;
}


chg_chgr_type_t extchg_charger_type_detect(void)
{
    int ret = 0;

    gpio_direction_input(USB_GPIO_DM);
    gpio_direction_input(USB_GPIO_DP);
    mdelay(100);

    ret = gpio_get_value(USB_GPIO_DP);
    if (0 == ret)
    {
        chg_stm_set_chgr_type(CHG_USB_HOST_PC);
        return CHG_USB_HOST_PC;
    }
    else
    {
        gpio_direction_output(USB_GPIO_DM, 0);
        ret = gpio_get_value(USB_GPIO_DP);
        if (0 == ret)
        {
            chg_stm_set_chgr_type(CHG_WALL_CHGR);
            return CHG_WALL_CHGR;
        }
        else
        {
            chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
            return CHG_NONSTD_CHGR;
        }
    }
}
#if defined(BSP_CONFIG_BOARD_E5771S_852)  || defined(BSP_CONFIG_BOARD_E5771H_937)

EXTCHG_ILIM ext_chg_limit_current_low_batt(int32_t bat_vol, int32_t batt_soc)
{
    static EXTCHG_ILIM ext_chg_status = RE_ILIM_2A;
    static EXTCHG_ILIM ext_chg_status_prev = RE_ILIM_2A;

    if(batt_soc > EXTCHG_STOP_CAPACITY_FIFTY)
    {
        /*  电量大于50，限2A。内充在位限1A  */
        if(FALSE == is_chg_charger_removed())
        {
             ext_chg_status = RE_ILIM_1A;
        }
        else
        {
            ext_chg_status = RE_ILIM_2A;
        }   
    }
    else if((batt_soc <= EXTCHG_STOP_CAPACITY_FIFTY) && (batt_soc > EXTCHG_STOP_CAPACITY_TWENTYFIVE))
    {
        /*  电量在[50,25),限流1A  */
        ext_chg_status = RE_ILIM_1A;
    }
    else
    {  /*  电量在[25,0),限流500mA  */
        ext_chg_status = RE_ILIM_500mA;
    }

    /*  针对关机放电有停充和复充处理  */
    if (TRUE == chg_is_powdown_charging())
    {
        chg_print_level_message(CHG_MSG_INFO, "EXTCHG:is in pd charge!\n ");
    #if defined(BSP_CONFIG_BOARD_E5771H_937)  /*937关机对外停止充电的门限修改为2.94V，提高对外充电效率*/
        if(bat_vol < BATT_VOLTAGE_2940)
    #else
        if(bat_vol < BATT_VOLTAGE_3100)
    #endif
        {
            ext_chg_status = RE_ILIM_STOP;
        }
        else if((ext_chg_status_prev == RE_ILIM_STOP) && (batt_soc <= EXTCHG_STOP_CAPACITY_TEN))
        {
            ext_chg_status = RE_ILIM_STOP;
        }
        else if((ext_chg_status_prev == RE_ILIM_STOP) && (batt_soc > EXTCHG_STOP_CAPACITY_TEN))
        {
            ext_chg_status = RE_ILIM_500mA;
        } 
        else
        {
            /* nothing */
        }
    }

    ext_chg_status_prev = ext_chg_status;
    chg_print_level_message(CHG_MSG_INFO, "ext_limit_batt ->status = %d,soc=%d,volt=%d\n",\
        ext_chg_status,batt_soc,bat_vol);
    return ext_chg_status;
}


EXTCHG_ILIM ext_chg_limit_current_temp(int32_t bat_temp)
{
    static EXTCHG_TEMP_ILIM ext_chg_status = TEMP_ILIM_2A;
    static EXTCHG_TEMP_ILIM ext_chg_status_pre = TEMP_ILIM_2A;
    if (bat_temp < EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD)  
    { /*  < -20 */
        ext_chg_status = LOWTEMP_ILIM_STOP;
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD) \     
        && (bat_temp < EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD))
    {    /* [-20,-17)*/
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
            break;
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_STOP:
        case OVERTEMP_ILIM_1A:
            ext_chg_status = LOWTEMP_ILIM_1A;
            break;
        default:
            break;
        }
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD) \
        && (bat_temp < EXTCHG_LOW_TEMP_STOP_THRESHOLD))
    {  /* [-17,0)*/
        ext_chg_status = LOWTEMP_ILIM_1A;
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_STOP_THRESHOLD) \
        && (bat_temp < EXTCHG_LOW_TEMP_RESUME_THRESHOLD))
    {  /* [0,3)*/
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
            ext_chg_status = LOWTEMP_ILIM_1A;
            break;
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_STOP:
        case OVERTEMP_ILIM_1A:
            ext_chg_status = TEMP_ILIM_2A;
            break;
        default:
            break;
        }
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_RESUME_THRESHOLD) \
        && (bat_temp <= EXTCHG_WARM_CHARGE_RESUME_THRESHOLD))
    { /* [3,52] */
        ext_chg_status = TEMP_ILIM_2A;
    }
    else if ((bat_temp >  EXTCHG_WARM_CHARGE_RESUME_THRESHOLD) \
        && (bat_temp < EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD))
    {    /* (52,55)*/ 
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
        case TEMP_ILIM_2A:
            ext_chg_status = TEMP_ILIM_2A;
            break;
        case OVERTEMP_ILIM_1A:
            ext_chg_status = OVERTEMP_ILIM_1A;
            break;
        case OVERTEMP_ILIM_STOP:    
            ext_chg_status = OVERTEMP_ILIM_STOP;
            break;
        default:
            break;
        }
    }
    else if ((bat_temp >= EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD) \
        && (bat_temp < EXTCHG_OVER_TEMP_STOP_THRESHOLD))
    {   /* [55,61)*/ 
        switch(ext_chg_status_pre){
        case LOWTEMP_ILIM_STOP:
        case LOWTEMP_ILIM_1A:
        case TEMP_ILIM_2A:
        case OVERTEMP_ILIM_1A:
            ext_chg_status = OVERTEMP_ILIM_1A;
            break;   
        case OVERTEMP_ILIM_STOP:   
            ext_chg_status = OVERTEMP_ILIM_STOP;
            break;
        default:
            break;
        }    
    }
    else if (bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD)
    {  /* [61,....)*/ 
        ext_chg_status = OVERTEMP_ILIM_STOP;
    }
    ext_chg_status_pre = ext_chg_status;
    if((ext_chg_status == OVERTEMP_ILIM_STOP) || (ext_chg_status == LOWTEMP_ILIM_STOP))
    {
        chg_print_level_message(CHG_MSG_INFO, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_STOP,bat_temp);
        return RE_ILIM_STOP;
    }
    else if((ext_chg_status == OVERTEMP_ILIM_1A) || (ext_chg_status == LOWTEMP_ILIM_1A))
    {
        chg_print_level_message(CHG_MSG_INFO, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_1A,bat_temp);
        return  RE_ILIM_1A;
    }
    else
    {  
        chg_print_level_message(CHG_MSG_INFO, "ext_limit_temp ->status = %d,temp = %d\n",\
            RE_ILIM_2A,bat_temp);
        return  RE_ILIM_2A;
    }   

}
#elif defined(BSP_CONFIG_BOARD_E5_E5770s) 
EXTCHG_ILIM ext_chg_limit_current_low_batt(int32_t bat_vol, int32_t batt_soc)
{
    static EXTCHG_ILIM ext_chg_status = RE_ILIM_2A;
    static EXTCHG_ILIM ext_chg_status_prev = RE_ILIM_2A;
    static boolean is_charging_status = FALSE;
    static boolean limit_flag = FALSE;
    static int32_t num_of_limit = 0;
    ext_chg_status_prev = ext_chg_status;

    if (FALSE == is_chg_charger_removed())
    {
        is_charging_status = TRUE;
        if ((bat_vol < BATT_VOLTAGE_3500) && (bat_vol >= BATT_VOLTAGE_3350))
        {
            if (FALSE == chg_is_powdown_charging())
            {
                ext_chg_status = RE_ILIM_500mA;
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge turn to RE_ILIM_500mA in [3350 3400)!!\n");
            }
            else
            {
                ext_chg_status = RE_ILIM_1A;
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge turn to RE_ILIM_1A in [3350 3400) in pd!!\n");
            }
        }
        else if (bat_vol < BATT_VOLTAGE_3350)
        {
            ext_chg_status = RE_ILIM_500mA;
            chg_print_level_message(CHG_MSG_DEBUG, \
                "EXTCHG: extcharge turn to RE_ILIM_500mA!!\n");
        }

#ifdef BSP_CONFIG_BOARD_E5_E5770s
        if (FALSE == chg_is_powdown_charging())
        {
            if (batt_soc <= extchg_stop_threshold_powon)
            {
                ext_chg_status = RE_ILIM_STOP;
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge turn to RE_ILIM_STOP, SOC<=10!!\n");
            }
        }
        else
        {
            if (batt_soc <= extchg_stop_threshold_powdown)
            {
                ext_chg_status = RE_ILIM_STOP;
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge turn to RE_ILIM_STOP, SOC<=10!!\n");
            }
        }
#endif

        if (ext_chg_status > ext_chg_status_prev)
        {
            ext_chg_status = ext_chg_status_prev;
        }

        if (batt_soc >= extchg_recharge_threshold)
        {
            ext_chg_status = RE_ILIM_1A;
            chg_print_level_message(CHG_MSG_DEBUG, \
                "EXTCHG: extcharge turn to RE_ILIM_1A up to 50%!!\n");
        }
        else
        {
            if (RE_ILIM_2A == ext_chg_status_prev)
            {
                ext_chg_status = RE_ILIM_1A;
            }
            else
            {
                //do nothing
            }
        }
    }
    else
    {
        if (TRUE == chg_is_powdown_charging())
        {
            if (TRUE == is_charging_status)
            {
                ext_chg_status_prev = RE_ILIM_2A;
                is_charging_status = FALSE;
            }
            if (bat_vol >= BATT_VOLTAGE_3350)
            {
                ext_chg_status = RE_ILIM_2A;
            }
            else if ((bat_vol >= BATT_VOLTAGE_3300) && (bat_vol < BATT_VOLTAGE_3350))
            {
                ext_chg_status = RE_ILIM_1A;
            }
            else if (bat_vol < BATT_VOLTAGE_3300)
            {
                ext_chg_status = RE_ILIM_500mA;
            }
        }
        else
        {
            if (TRUE == is_charging_status)
            {
                ext_chg_status_prev = RE_ILIM_2A;
                is_charging_status = FALSE;
            }
            if (bat_vol >= BATT_VOLTAGE_3600)
            {
                limit_flag = FALSE;
                num_of_limit = 0;
                if (RE_ILIM_2A == ext_chg_status_prev)
                {
                    ext_chg_status = RE_ILIM_2A;
                    chg_print_level_message(CHG_MSG_DEBUG, \
                        "EXTCHG: extcharge turn to RE_ILIM_2A in [3600 +00)!!\n");
                }
            }
            else if ((bat_vol < BATT_VOLTAGE_3600) && (bat_vol >= BATT_VOLTAGE_3350))
            {
                limit_flag = FALSE;
                num_of_limit = 0;
                if (batt_soc <= extchg_limit_1a_capacity)
                {
                    if (bat_vol >= BATT_VOLTAGE_3500)
                    {
                        if (RE_ILIM_2A == ext_chg_status_prev)
                        {
                            ext_chg_status = RE_ILIM_1A;
                            chg_print_level_message(CHG_MSG_DEBUG, \
                                "EXTCHG: extcharge turn to RE_ILIM_1A in [3500 +00)!!\n");
                        }
                    }
                    else
                    {
                        if (batt_soc <= extchg_limit_500ma_capacity)
                        {
                            if (ext_chg_status_prev > RE_ILIM_500mA)
                            {
                                ext_chg_status = RE_ILIM_500mA;
                                chg_print_level_message(CHG_MSG_DEBUG, \
                                    "EXTCHG: extcharge turn to RE_ILIM_500mA!!\n");
                            }
                        }
                    }
                }
            }
            else if (bat_vol < BATT_VOLTAGE_3350)
            {
                if (limit_flag != TRUE)
                {
                    limit_flag = TRUE;
                    if (RE_ILIM_2A == ext_chg_status_prev)
                    {
                        ext_chg_status = RE_ILIM_1A;
                        chg_print_level_message(CHG_MSG_DEBUG, \
                            "EXTCHG: extcharge turn to RE_ILIM_1A from RE_ILIM_2A!!\n");
                    }
                    else if (RE_ILIM_1A == ext_chg_status_prev)
                    {
                        ext_chg_status = RE_ILIM_500mA;
                        chg_print_level_message(CHG_MSG_DEBUG, \
                            "EXTCHG: extcharge turn to RE_ILIM_500mA from RE_ILIM_1A!!\n");
                    }
                    else if (RE_ILIM_500mA == ext_chg_status_prev)
                    {
                        if (FALSE == chg_is_powdown_charging())
                        {
#ifdef BSP_CONFIG_BOARD_E5_E5770s
                            if (batt_soc <= extchg_stop_threshold_powon)
                            {
                                ext_chg_status = RE_ILIM_STOP;
                                chg_print_level_message(CHG_MSG_DEBUG, \
                                    "EXTCHG: extcharge turn to RE_ILIM_STOP SOC<=10!!\n");
                            }
#endif
                        }
                    }
                    else
                    {
                        //do nothing
                    }
                }
                else
                {
                    num_of_limit++;

                    if (num_of_limit > NUM_OF_LIMIT)
                    {
                        if (RE_ILIM_1A == ext_chg_status_prev)
                        {
                            ext_chg_status = RE_ILIM_500mA;
                            chg_print_level_message(CHG_MSG_DEBUG, \
                                "EXTCHG: extcharge turn to RE_ILIM_500mA cotinuously!!\n");
                        }
                        else if (RE_ILIM_500mA == ext_chg_status_prev)
                        {
                            if (FALSE == chg_is_powdown_charging())
                            {
#ifdef BSP_CONFIG_BOARD_E5_E5770s
                                if (batt_soc <= extchg_stop_threshold_powon)
                                {
                                    ext_chg_status = RE_ILIM_STOP;
                                    chg_print_level_message(CHG_MSG_DEBUG, \
                                        "EXTCHG: extcharge turn to STOP cotinuously!!\n");
                                }
#endif
                            }
                        }
                        else
                        {
                            //do nothing
                        }
                    }
                }
            }
        }
#ifdef BSP_CONFIG_BOARD_E5_E5770s
        if (FALSE == chg_is_powdown_charging())
        {
            if (batt_soc <= extchg_stop_threshold_powon)
            {
                ext_chg_status = RE_ILIM_STOP;
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge turn to RE_ILIM_STOP SOC<=10!!\n");
            }
        }
        else
        {
            if (batt_soc <= extchg_stop_threshold_powdown)
            {
                ext_chg_status = RE_ILIM_STOP;
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_LOWPOWER_STOP_CHARGING;
                chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge turn to RE_ILIM_STOP SOC<=2!!\n");
            }
        }
#endif

        if (ext_chg_status > ext_chg_status_prev)
        {
            ext_chg_status = ext_chg_status_prev;
        }
    }
#ifdef BSP_CONFIG_BOARD_E5_E5770s
    if ((RE_ILIM_STOP == ext_chg_status_prev) && (RE_ILIM_STOP != ext_chg_status))
    {
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
    }
#endif
    ext_chg_status_prev = ext_chg_status;
    return ext_chg_status;
}


EXTCHG_ILIM ext_chg_limit_current_temp(int32_t bat_temp)
{
    static EXTCHG_ILIM ext_chg_status = RE_ILIM_2A;
    static boolean over_temp_flag = FALSE;

    if (bat_temp <= EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD)
    {
        ext_chg_status = RE_ILIM_STOP;
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge turn to RE_ILIM_STOP, bat_temp=%d!!\n", bat_temp);
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_SHUTOFF_THRESHOLD) \
        && (bat_temp < EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD))
    {
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge remaining, bat_temp=%d!!\n", bat_temp);
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_SHUT_RESUM_THRSHOLD) \
		&& (bat_temp < EXTCHG_LOW_TEMP_STOP_THRESHOLD))
    {
        ext_chg_status = RE_ILIM_1A;
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge turn to RE_ILIM_1A, bat_temp=%d!!\n", bat_temp);
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_STOP_THRESHOLD) \
        && (bat_temp < EXTCHG_LOW_TEMP_RESUME_THRESHOLD))
    {
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge remaining, bat_temp=%d!!\n", bat_temp);
    }
    else if ((bat_temp >= EXTCHG_LOW_TEMP_RESUME_THRESHOLD) \
        && (bat_temp < EXTCHG_WARM_CHARGE_RESUME_THRESHOLD))
    {
        if (FALSE == is_chg_charger_removed())
        {
            ext_chg_status = RE_ILIM_1A;
            chg_print_level_message(CHG_MSG_DEBUG, \
                "EXTCHG: extcharge turn to RE_ILIM_1A, \
                    bat_temp=%d!!\n", bat_temp);
        }
        else
        {
            ext_chg_status = RE_ILIM_2A;
            chg_print_level_message(CHG_MSG_DEBUG, \
                "EXTCHG: extcharge turn to RE_ILIM_2A, \
                    bat_temp=%d!!\n", bat_temp);
        }
    }
    else if ((bat_temp >= EXTCHG_WARM_CHARGE_RESUME_THRESHOLD) \
        && (bat_temp < EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD))
    {
        if (TRUE == over_temp_flag)
        {
            ext_chg_status = RE_ILIM_500mA;
            over_temp_flag = FALSE;
            chg_print_level_message(CHG_MSG_DEBUG, \
                "EXTCHG: extcharge turn to EXTCHG_WARM_ST, \
                    bat_temp=%d!!\n", bat_temp);

        }
    }
    else if ((bat_temp >= EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD) \
        && (bat_temp < EXTCHG_OVER_TEMP_STOP_THRESHOLD))
    {
        if (TRUE == over_temp_flag)
        {
            ext_chg_status = RE_ILIM_STOP;
        }
        else
        {
            ext_chg_status = RE_ILIM_500mA;
        }
    }
    else if (bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD)
    {
        ext_chg_status = RE_ILIM_STOP;
        over_temp_flag = TRUE;
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge turn to EXTCHG_OVERTEMP_ST, \
                bat_temp=%d!!\n", bat_temp);
    }
    else
    {
        //do nothing
    }

    if ((hisi_battery_temperature() > EXTCHG_EXTREME_HIGH_TEMP) \
        && (RE_ILIM_STOP != ext_chg_status))
    {
        ext_chg_status = RE_ILIM_500mA;
        chg_print_level_message(CHG_MSG_ERR, \
            "EXTCHG: TEMP HIGHER THAN 77, ILIM TO 500MA, \
                bat_temp=%d!!\n", bat_temp);
    }

    return ext_chg_status;
}
#else
EXTCHG_ILIM ext_chg_limit_current_low_batt(int32_t bat_vol, int32_t batt_soc)
{
    static EXTCHG_ILIM ext_chg_status = RE_ILIM_2A;
    return ext_chg_status;
}
EXTCHG_ILIM ext_chg_limit_current_temp(int32_t bat_temp)
{
    static EXTCHG_ILIM ext_chg_status = RE_ILIM_2A;
    return ext_chg_status;
}
#endif

boolean extchg_monitor_func_in_sleep_mode(int32_t bat_vol, int32_t bat_temp)
{
    boolean need_to_change_timer = FALSE;
    int32_t batt_soc = chg_get_sys_batt_capacity();

    count_for_short_det = 0;
    poll_round_for_det = 0;

    if (TRUE == extchg_short_flag)
    {
        return FALSE;
    }
    if ((RE_ILIM_STOP != ext_chg_status_new) && (TRUE != usb_set_direction_flag))
    {
        if (extchg_is_perph_circult_short())
        {
            ext_chg_status_new = RE_ILIM_STOP;
            extchg_short_flag = TRUE;
            extchg_set_charge_enable(FALSE);
            chg_print_level_message(CHG_MSG_ERR, \
                "EXTCHG: extcharge circult is short in sleep mode, \
                    and extcharge is short protected!!\n");
            return FALSE;
        }
    }

    ext_chg_status_new = ext_chg_limit_current_low_batt(battery_voltage, batt_soc);
    ext_chg_status_temp = ext_chg_status_new;
    ext_chg_status_new = ext_chg_limit_current_temp(battery_temperature);
    ext_chg_status_new = (ext_chg_status_temp < ext_chg_status_new) \
        ? ext_chg_status_temp : ext_chg_status_new;

    if (ext_chg_status_old != ext_chg_status_new)
    {
        ext_chg_status_old = ext_chg_status_new;
        switch(ext_chg_status_new)
        {
            case RE_ILIM_STOP:
                extchg_set_charge_enable(FALSE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_charge_enable(FALSE)!!\n");
                break;
            case RE_ILIM_2A:
                extchg_set_cur_level(RE_ILIM_2A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_2A)!!\n");
                break;
            case RE_ILIM_1A:
                extchg_set_cur_level(RE_ILIM_1A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_1A)!!\n");
                break;
            case RE_ILIM_500mA:
#if defined(BSP_CONFIG_BOARD_E5771H_937)
                gpio_direction_output(RE_ILIM_1A_GPIO, 1);
                extchg_set_charge_enable(TRUE);
                extchg_set_cur_level(RE_ILIM_500mA);
#else
                extchg_set_cur_level(RE_ILIM_500mA);
                extchg_set_charge_enable(TRUE);
#endif
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_500mA)!!\n");
                break;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge status not change, do nothing!!\n");
    }
    if ((battery_temperature >= EXTCHG_WARM_CHARGE_LIMIT_THRESHOLD) \
        || (batt_soc < EXTCHG_STOP_CAPACITY_TWENTY))
    {
        need_to_change_timer = TRUE;
    }
    else
    {
        need_to_change_timer = FALSE;
    }

    return need_to_change_timer;
}
#endif


void chg_extchg_insert_proc(void)
{
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    wake_lock_timeout(&g_chip->alarm_wake_lock,
        (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
    mdelay(DELAY_TIME_OF_DEBOUNCE);
    if (1 == gpio_get_value(EXTCHG_OTG_DET_GPIO))
    {
        return;
    }

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN!\n ");

    if ((TRUE == extchg_short_flag) || \
        (chg_stm_state_info.extchg_status == POWER_SUPPLY_EXTCHGSTA_STOP_FAULT))
    {
        extchg_short_flag = FALSE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        extchg_set_charge_enable(TRUE);
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }
    g_exchg_online_flag = ONLINE;
    chg_stm_set_extchg_online_st(TRUE);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);
#else
    if(TRUE == is_5577cs_falg)
    {
        return;
    }
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG IN!\n ");
    if(OFFLINE == g_exchg_online_flag)
    {
        g_exchg_online_flag = ONLINE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    /*如果UI设置永久禁止对外充电此处屏蔽USB ID 插入中断*/
    if(1 == g_extchg_diable_st)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    /*无线充电场景不处理对外充电事件*/
    if(CHG_WIRELESS_MODE == cur_chg_mode)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:chg_mode in CHG_WIRELESS_MODE,do nothing!\n ");
        return;
    }

    chg_stm_set_extchg_online_st(TRUE);
    chg_set_cur_chg_mode(CHG_EXTCHG_MODE);
    //TO DO:1:调用USB驱动接口短接D+ ,D-
    usb_notify_event(USB_OTG_CONNECT_DP_DM,NULL);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG, (uint32_t)CHG_EVENT_NONEED_CARE);
#if ( FEATURE_ON == MBB_MLOG )
    printk(KERN_ERR, "CHG_STM:otg_charge_count\n");
    mlog_set_statis_info("otg_charge_count",1);//对外充电总次数 加1
#endif        
    /*启动对外充电监控work*/
    //is_vbus_unkonwn清零
    is_vbus_unkonwn = FALSE;
    if(NULL != g_chip)
    {
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }
#endif
}

void chg_extchg_remove_proc(void)
{
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    int count = 0;
#if defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937) 
    wake_lock_timeout(&g_chip->alarm_wake_lock,
        (long)msecs_to_jiffies(ALARM_REPORT_WAKELOCK_TIMEOUT));
#else
    wake_lock_timeout(&g_chip->alarm_wake_lock,
        (long)msecs_to_jiffies(SHUTDOWN_WAKELOCK_TIMEOUT));
#endif
    count_for_short_det = 0;
    poll_round_for_det = 0;
    mdelay(DELAY_TIME_OF_DEBOUNCE);
    if (0 == gpio_get_value(EXTCHG_OTG_DET_GPIO))
    {
        return;
    }

    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG OUT!\n ");

    if ((TRUE == extchg_short_flag) || \
        (chg_stm_state_info.extchg_status == POWER_SUPPLY_EXTCHGSTA_STOP_FAULT))
    {
        extchg_short_flag = FALSE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        extchg_set_charge_enable(TRUE);
        schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(0));
    }

    g_exchg_online_flag = OFFLINE;
    chg_stm_set_extchg_online_st(FALSE);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, (uint32_t)CHG_EVENT_NONEED_CARE);

    if (TRUE == chg_is_powdown_charging())
    {
        for(count = 0; count <= 1; count++)
        {
            chg_detect_batt_chg_for_shutoff();
        }
    }
#else
    if(ONLINE == g_exchg_online_flag)
    {
        g_exchg_online_flag = OFFLINE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:disable extchg,do nothing!\n");
        return;
    }
    chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:CHG_EXGCHG_CHGR PLUG OUT!\n ");
    /*终止运行的work*/
    cancel_delayed_work_sync(&g_chip->extchg_monitor_work);

    /*对外充电USB ID线拔出将温度补偿的标志位清除*/
    g_ui_choose_exchg_mode = 0;
    g_last_extchg_diable_st = 0;
    is_otg_extchg = FALSE;
    is_extchg_ovtemp = FALSE;
    chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
    chg_set_extchg_chg_enable(FALSE);
    chg_stm_set_extchg_online_st(FALSE);
    chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
#endif
}

void chg_extchg_monitor_func(void)
{
    int32_t bat_temp = chg_get_sys_batt_temp();
    int32_t bat_vol = chg_get_sys_batt_volt();
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    int32_t batt_soc = chg_get_sys_batt_capacity();
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    //短路保护
    if (TRUE == extchg_short_flag)
    {
        if (TIME_OF_SHORT_DET == count_for_short_det)
        {
            extchg_set_charge_enable(TRUE);
            mdelay(50);
            if (extchg_is_perph_circult_short())
            {
                extchg_set_charge_enable(FALSE);
                count_for_short_det = 0;
                count_for_short_det++;
                poll_round_for_det++;
            }
            else
            {
                ext_chg_status_new = RE_ILIM_2A;
                extchg_short_flag = FALSE;
                count_for_short_det = 0;
                poll_round_for_det = 0;
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extcharge is not short anymore!\n");
                bsp_softtimer_add(&g_chg_sleep_timer);
                wake_unlock(&g_chip->alarm_wake_lock);
                goto out;
            }
        }
        else
        {
            count_for_short_det++;
        }
        if (NUM_OF_SHORT_DET_POLL == poll_round_for_det)
        {
            poll_round_for_det = 0;
            count_for_short_det = 0;
            count_for_short_det++;
            #if !defined(BSP_CONFIG_BOARD_E5771S_852) && !defined(BSP_CONFIG_BOARD_E5771H_937)
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
            chg_send_stat_to_app((uint32_t)DEVICE_ID_BATTERY, \
                (uint32_t)CHG_EVENT_NONEED_CARE);
            mdelay(ALARM_REPORT_WAKELOCK_TIMEOUT);
            #endif
            chg_print_level_message(CHG_MSG_ERR, \
                "EXTCHG: extcharge is short protected!!!\n");
            bsp_softtimer_add(&g_chg_sleep_timer);
            wake_unlock(&g_chip->alarm_wake_lock);
            return;
        }
        goto out;
    }
    else
    {
        if ((RE_ILIM_STOP != ext_chg_status_new) && (TRUE != usb_set_direction_flag))
        {
            if (extchg_is_perph_circult_short())
            {
                wake_lock(&g_chip->alarm_wake_lock);
                bsp_softtimer_delete(&g_chg_sleep_timer);
                extchg_set_charge_enable(FALSE);
                ext_chg_status_new = RE_ILIM_STOP;
                extchg_short_flag = TRUE;
                count_for_short_det++;
                chg_print_level_message(CHG_MSG_ERR, \
                    "EXTCHG: extcharge circult is short!!\n");
                goto out;
            }
        }
    }

    ext_chg_status_new = ext_chg_limit_current_low_batt(bat_vol, batt_soc);
    ext_chg_status_temp = ext_chg_status_new;
    ext_chg_status_new = ext_chg_limit_current_temp(bat_temp);

    ext_chg_status_new = (ext_chg_status_temp < ext_chg_status_new) \
        ? ext_chg_status_temp : ext_chg_status_new;

    if (ext_chg_status_old != ext_chg_status_new)
    {
        ext_chg_status_old = ext_chg_status_new;
        switch(ext_chg_status_new)
        {
            case RE_ILIM_STOP:
                extchg_set_charge_enable(FALSE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_charge_enable(FALSE)!!\n");
                break;
            case RE_ILIM_2A:
                extchg_set_cur_level(RE_ILIM_2A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_2A)!!\n");
                break;
            case RE_ILIM_1A:
                extchg_set_cur_level(RE_ILIM_1A);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_1A)!!\n");
                break;
            case RE_ILIM_500mA:
                extchg_set_cur_level(RE_ILIM_500mA);
                extchg_set_charge_enable(TRUE);
                chg_print_level_message(CHG_MSG_DEBUG, \
                    "EXTCHG: extchg_set_cur_level(RE_ILIM_500mA)!!\n");
                break;
        }
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, \
            "EXTCHG: extcharge status not change, do nothing!!\n");
    }
out:
    //chg_extchg_info_dump();
    chg_print_test_view_info();
    ext_chg_info_dump();

    schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(2000));

    return;
#else
    if(OFFLINE == g_exchg_online_flag)
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_PLT:extchg device not exit,do nothing!\n");
        return;
    }
    /*更新UI配置*/
    chg_extchg_config_data_init();
    /*状态监控*/
#if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
    || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
    /*对外充电短路检测与处理*/
    if(TRUE == g_exchg_enable_flag)
    {
        if(TRUE == extchg_circult_short_detect())
        {
            if(TRUE == is_vbus_unkonwn)
            {
                chg_set_extchg_chg_enable(FALSE);
                chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
                is_vbus_unkonwn = FALSE;
                /*通知应用不满足对外充电条件停止对外充电*/
                chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
                chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:STOP EXTGHG!\n");
                return;
            }
            else
            {
            is_vbus_unkonwn = TRUE;
            }
        }
        else
        {
            is_vbus_unkonwn = FALSE;
        }
    }
#endif
    /*如果对外充电过流且为非OTG模式，则通过5V上下电reset usb模块*/
    if(TRUE == chg_extchg_ocp_detect())
    {
        chg_print_level_message(CHG_MSG_ERR, "CHG_STM:EXTCHG OCP happened is_otg_extchg=%d!\n",
            is_otg_extchg);

        if(FALSE == is_otg_extchg)
        {
           /*将充电芯片设置为 非OTG模式，5V 下电*/
           chg_set_charge_otg_enable(FALSE);
           /*延时500MS*/
           chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
           //TO DO:调用USB驱动接口直连基带，拉低HS_ID；
           usb_notify_event(USB_OTG_DISCONNECT_DP_DM,NULL);
           /*延时500MS*/
           chg_delay_ms(EXTCHG_DELAY_COUNTER_SIZE);
          /*将充电芯片设置为OTG模式5V上电*/
           chg_set_charge_otg_enable(TRUE);
           /*至此对外充电切换为OTG 500MA充电模式*/
           is_otg_extchg = TRUE;
           chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:1A EXTCHG OCP change to USB OTG 500MA!\n");
            
        }
        /*如果为OTG模式检测到过流直接停止对外充电*/
        else
        {
            chg_set_extchg_chg_enable(FALSE);
            chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
            /*通知应用不满足对外充电条件停止对外充电*/
            chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:USB OTG 500MA EXTCHG OCP STOP EXTGHG!\n");
        }
    }
    /*电池过温停止对外充电*/    
    #if defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5577S_321) \
   || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
     else if((bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD) || (bat_temp <= EXTCHG_LOW_TEMP_STOP_THRESHOLD))
    #else
     else if(bat_temp >= EXTCHG_OVER_TEMP_STOP_THRESHOLD)
    #endif
    {
        /*过温停止对外充电*/
        chg_set_extchg_chg_enable(FALSE);
        is_extchg_ovtemp = TRUE;

        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_OVERHEAT_STOP_CHARGING;
        /*通知应用温度异常停止对外充电*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);

        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG OVER TEMP STOP EXTGHG!\n");
    }
    /*电池温度恢复使能对外充电*/
    #if defined(BSP_CONFIG_BOARD_E5577S_932) || defined(BSP_CONFIG_BOARD_E5577S_321) \
    || defined(BSP_CONFIG_BOARD_E5577S_324) || defined(BSP_CONFIG_BOARD_E5577BS_937)
    else if(((bat_temp <= EXTCHG_OVER_TEMP_RESUME_THRESHOLD) && (bat_temp >= EXTCHG_LOW_TEMP_RESUME_THRESHOLD)) \
            && (TRUE == is_extchg_ovtemp))
    #else
    else if((bat_temp <= EXTCHG_OVER_TEMP_RESUME_THRESHOLD) && (TRUE == is_extchg_ovtemp))
    #endif
    {
        chg_set_extchg_chg_enable(TRUE);
        is_extchg_ovtemp = FALSE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_START_CHARGING;
        /*温度恢复正常通知应用开始对外充电*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG TEMP NORMAL RESUME EXTGHG!\n");
    }
   /*电池低电停止对外充电*/
#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
    else if(batt_soc <= g_extchg_stop_soc_threshold)
#else
    else if(bat_vol <= g_extchg_voltage_threshold)
#endif
    {
        chg_set_extchg_chg_enable(FALSE);
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_LOWPOWER_STOP_CHARGING;
        /*通知应用低电停止对外充电*/
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:EXTCHG LOW VOLTAGE STOP EXTGHG!\n");
    }
    /*正在对外充电过程中如果检测到用户通过TOUCH UI设置永久停止对外充电则直接停止对外充电*/
    else if((1 == g_extchg_diable_st) && (0 == g_last_extchg_diable_st))
    {
        g_last_extchg_diable_st = 1;
        chg_set_extchg_chg_enable(FALSE);
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:UI set disable extchg permanently!\n");
    }
    /*正在对外充电过程中如果检测到用户通过TOUCH UI设置永久停止对外充电则直接停止对外充电后
       如果用户再设置打开*/
    else if((0 == g_extchg_diable_st) && (1 == g_last_extchg_diable_st))
    {
        g_last_extchg_diable_st = 0;
        /*重新使能后将STA节点状态改为UNKNOWN,是选择框弹出来由用户选择后决定是否对外充电*/
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        chg_send_stat_to_app((uint32_t)DEVICE_ID_EXTCHG,(uint32_t)CHG_EVENT_NONEED_CARE);
        chg_print_level_message(CHG_MSG_INFO, "CHG_STM:UI set enable extchg permanently!\n");
    }
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:staying at extchg_state !\n");
    }
    chg_extchg_info_dump();

    schedule_delayed_work(&g_chip->extchg_monitor_work, msecs_to_jiffies(2000));
#endif
}


#endif

/**********************************************************************
FUNCTION:    chg_charger_insert_proc
DESCRIPTION: usb remove process
INPUT:       chg_chgr_type_t chg_type
OUTPUT:      None.
RETURN:      None.
NOTE:        None
***********************************************************************/
void chg_charger_insert_proc(chg_chgr_type_t chg_type)
{
    chg_stm_state_type cur_stat = chg_stm_get_cur_state();
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    chg_chgr_type_t charge_type = CHG_CHGR_UNKNOWN;
#endif
    /*USB插入*/
    if(CHG_USB_HOST_PC == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_USB_HOST_PC PLUG IN force to TRANSIT_ST!\n ");
        if(FALSE == chg_get_usb_online_status())
        {
            chg_set_ac_online_status(FALSE);
            chg_set_usb_online_status(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        chg_stm_set_chgr_type(CHG_USB_HOST_PC);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }

#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*USB插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "usb2.0 charger insert!\n ");
            /*usb2.0插入，自增一次*/
            mlog_set_statis_info("usb2_charge_insert_count",1);
    
#endif
        return ;
    }
    /*标充插入*/
    else if(CHG_WALL_CHGR == chg_type || CHG_USB_OTG_CRADLE == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WALL_CHGR PLUG IN force to TRANSIT_ST!\n ");
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#if (FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
        if (usb_get_switch_direction())
        {
#endif
            /*在喵王产品上，苹果充电器对单板充电，会被识别成自环，所以当外充在位时，才进入自环判断流程*/
            if(0 == gpio_get_value(EXTCHG_OTG_DET_GPIO))
            {
                gpio_direction_output(GPIO_USB_SELECT, USB_SWITCH_APORT_LEVEL);
                mdelay(DELAY_TIME_20MS);
                charge_type = extchg_charger_type_detect();
                if (charge_type == CHG_NONSTD_CHGR)
                {
                    if (TRUE == chg_is_charger_present())
                    {
                        if (g_exchg_online_flag == ONLINE)
                        {
                        
                            cancel_delayed_work_sync(&g_chip->extchg_monitor_work);
                            chg_stm_state_info.extchg_status = \
                                POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
                            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, \
                                (uint32_t)CHG_EVENT_NONEED_CARE);
                        #if defined(BSP_CONFIG_BOARD_E5771H_937)
                            usb_err_flag = TRUE;
                            chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA);
                       #else
                            extchg_set_charge_enable(FALSE);
                       #endif
                            chg_print_level_message(CHG_MSG_ERR, \
                                "CHG_PLT:CHARGING TO ITSELF, STOP CHARGING!\n ");
                        }
                        else
                        {
                            usb_err_flag = TRUE;
                            chg_stm_set_extchg_online_st(TRUE);
                            chg_set_supply_limit(CHG_IINPUT_LIMIT_100MA);
                            chg_stm_state_info.extchg_status = \
                                POWER_SUPPLY_EXTCHGSTA_STOP_FAULT;
                            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, \
                                (uint32_t)CHG_EVENT_NONEED_CARE);
                            chg_print_level_message(CHG_MSG_ERR, \
                                "CHG_PLT:CHARGING TO ITSELF, LIMIT CHARGING!\n ");
                        }
                    }
                }
                gpio_direction_output(GPIO_USB_SELECT,USB_SWITCH_MPORT_LEVEL);
            }
#if (FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
        }
#endif
#endif
        /*AF18 插入设置DPM电压值*/
        if(CHG_USB_OTG_CRADLE == chg_type)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:AF18 PLUG IN set Dpm.\n ");
            chg_set_dpm_val(CHG_AF18_DPM_VOLT);
        }
        /*如果检测到充电器类型是标充就将 USB 设置为OFFINE,将AC设置为ONLINE*/
        chg_set_usb_online_status(FALSE);

        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_stm_set_chgr_type(CHG_WALL_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);

        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*标充插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                 过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "std charger insert!\n ");
            mlog_set_statis_info("std_charge_insert_count",1);
#endif
        return ;
    }
    /*弱充插入*/
    else if(CHG_500MA_WALL_CHGR == chg_type)
    {
        chg_en_flag = 0;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_500MA_WALL_CHGR PLUG IN force to TRANSIT_ST!\n ");
        chg_stm_set_chgr_type(CHG_500MA_WALL_CHGR);
        return ;
    }
    /*第三方充电器插入*/
    else if(CHG_NONSTD_CHGR == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_NONSTD_CHGR PLUG IN force to TRANSIT_ST!\n ");
        //第三方充电器插入时，认为AC在位
        if(FALSE == chg_get_ac_online_status())
        {
            chg_set_ac_online_status(TRUE);
            chg_set_usb_online_status(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        chg_stm_set_chgr_type(CHG_NONSTD_CHGR);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);

        if((CHG_STM_INIT_ST >= cur_stat) || (CHG_STM_MAX_ST <= cur_stat)
            || (CHG_STM_BATTERY_ONLY == cur_stat) || (CHG_STM_TRANSIT_ST == cur_stat))
        {
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if(TRUE == chg_stm_get_wireless_online_st())
        {
            /*三方充电器插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
                过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
            chg_stm_set_wireless_online_st(FALSE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);

        }
#endif /*defined(MBB_CHG_WIRELESS)*/
#if ( FEATURE_ON == MBB_MLOG )
            chg_print_level_message(CHG_MSG_ERR, "no_std charger insert!\n ");
            mlog_set_statis_info("no_std_charge_insert_count",1);
#endif
        return ;
    }

    /*充电器类型未知，在此判断是无线充电还是有线充电设备插入*/
    else if(CHG_CHGR_UNKNOWN == chg_type)
    {
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        chg_stm_set_wireless_online_st(FALSE);
#endif/*MBB_CHG_WIRELESS*/
        /*如果是有线充电器插入先设置AC ONLINE保证应用可以迅速点屏
              如果检测到充电器类型是usb或第三方充电器就将AC 设置为OFFINE,将USB设置为ONLINE*/
        chg_set_ac_online_status(TRUE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        /*有线充电器插入后将无线充电器在位的ONLINE节点置为OFFLINE，防止无线充电
        过程中插入有线设备因ONLINE节点没有清除显示无线充电图标*/
        //chg_stm_set_chgr_type(CHG_CHGR_UNKNOWN);
        chg_set_cur_chg_mode(CHG_WIRED_MODE);
        //chg_start_chgr_type_checking();
        chg_print_level_message( CHG_MSG_ERR,"CHG_PLT: wired charger but chg type unknow force to TRANSIT_ST!\n ");
        chg_stm_switch_state(CHG_STM_TRANSIT_ST);
        return ;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    else if(CHG_WIRELESS_CHGR == chg_type)
    {
        /*通知USB驱动无线充电器插入,防止USB驱动未检测出充电器类型给充电模块
        上报第三方充电器，导致充电流程混乱*/
        g_wireless_online_flag = ONLINE;
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_STM:notify USB wireless charger!\n");
        usb_notify_event(USB_WIRELESS_CHGR_DET,NULL);
        if(0 == g_ui_choose_exchg_mode)
        {
            (void)chg_set_dpm_val(CHG_WIRELESS_DPM_VOLT);
            chg_stm_set_wireless_online_st(TRUE);
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WIRELESS_CHGR PLUG IN force to TRANSIT_ST!\n ");
            /*检测出来是无线充电器后就控制无线充电芯片GPIO使能无线充电，充电过程中的停复充有BQ24196控制*/
            chg_set_wireless_chg_enable(TRUE);
            chg_stm_set_chgr_type(CHG_WIRELESS_CHGR);
            chg_set_cur_chg_mode(CHG_WIRELESS_MODE);
            chg_stm_switch_state(CHG_STM_TRANSIT_ST);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
            return ;
        }
        else
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:WIRELESS_CHGR PLUG IN but current not CHG_BAT_ONLY_MODE!\n");
            return ;
        }
#if ( FEATURE_ON == MBB_MLOG )        
            chg_print_level_message(CHG_MSG_ERR, "wireless charger insert!\n ");
            mlog_set_statis_info("wireless_charge_insert_count",1);
#endif
    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:PLUG IN CHG TYPE UNKNOW!\n ");
    }
}
/**********************************************************************
FUNCTION:    chg_charger_remove_proc
DESCRIPTION: usb remove process
INPUT:       chg_chgr_type_t chg_type
OUTPUT:      None.
RETURN:      None.
NOTE:        None
***********************************************************************/
void chg_charger_remove_proc(chg_chgr_type_t chg_type)
{
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    CHG_MODE_ENUM cur_chg_mode = chg_get_cur_chg_mode();
#endif
/*电池膨胀防止快速插拔，状态不刷新*/
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    if (TRUE == chg_battery_protect_flag)
    {
        chg_battery_protect_flag = FALSE;
        g_chg_over_temp_volt_protect_flag = FALSE;
        g_chg_longtime_nocharge_protect_flag = FALSE;
        (void)chg_battery_protect_exit_process();
    }
    (void)resume_long_time_no_charge_protect_recharge_volt();
#endif

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
    if (TRUE == usb_err_flag)
    {
        usb_err_flag = FALSE;
        chg_stm_state_info.extchg_status = POWER_SUPPLY_EXTCHGSTA_UNKNOWN;
        chg_stm_set_extchg_online_st(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, \
            (uint32_t)CHG_EVENT_NONEED_CARE);
    }
#endif
    /*USB 拔出*/
    if(CHG_USB_HOST_PC == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_USB_HOST_PC PLUG OUT force to BATTERY_ONLY_st!\n ");
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*标充拔出*/
    else if(CHG_WALL_CHGR == chg_type || CHG_USB_OTG_CRADLE == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WALL_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        //AF18 拔出后恢复默认DPM电压值
        if(CHG_USB_OTG_CRADLE == chg_type)
        {
            chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:AF18 PLUG out set Dpm.\n ");
            chg_set_dpm_val(CHG_DEFAULT_DPM_VOLT);
        }
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*弱充拔出*/
    else if(CHG_500MA_WALL_CHGR == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_500MA_WALL_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        chg_set_ac_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        return ;
    }
    /*第三方充电器拔出*/
    else if(CHG_NONSTD_CHGR == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_NONSTD_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
        /*第三方充电器拔出按照USB驱动拔出给应用上报*/
        chg_set_usb_online_status(FALSE);
        chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);

        return ;
    }
    else if(CHG_CHGR_UNKNOWN == chg_type)
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_CHGR_UNKNOWN PLUG OUT force to BATTERY_ONLY_st!\n ");
        if (TRUE == chg_get_usb_online_status())
        {
            chg_set_usb_online_status(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_USB, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
        if(TRUE == chg_get_ac_online_status())
        {
            chg_set_ac_online_status(TRUE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_CHARGER, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
        if (TRUE == chg_stm_get_wireless_online_st())
        {
            chg_stm_set_wireless_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#endif/*defined(MBB_CHG_WIRELESS)*/
#if (MBB_CHG_EXTCHG == FEATURE_ON)
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#else
        if (TRUE == chg_stm_get_extchg_online_st())
        {
            chg_stm_set_extchg_online_st(FALSE);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
        }
#endif
#endif/*defined(MBB_CHG_EXTCHG)*/

        chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
        chg_stm_set_chgr_type(CHG_CHGR_INVALID);
        chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
        return ;
    }
#if (MBB_CHG_WIRELESS == FEATURE_ON)
    /*无线充电器拔出*/
    else if(CHG_WIRELESS_CHGR == chg_type)
    {
        /*如果当前是无线充电模式，无线充电器不在位后才进行状态切换否则只将无限充电器
            ONLINE节点设置为OFFLINE,不进行状态切换*/
        g_wireless_online_flag = OFFLINE;
        if((CHG_WIRELESS_MODE == cur_chg_mode) || (TRUE == chg_is_powdown_charging()))
        {
             chg_stm_set_wireless_online_st(FALSE);
             chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:CHG_WIRELESS_CHGR PLUG OUT force to BATTERY_ONLY_st!\n ");
            (void)chg_set_dpm_val(CHG_DEFAULT_DPM_VOLT);
            chg_set_cur_chg_mode(CHG_BAT_ONLY_MODE);
            chg_stm_switch_state(CHG_STM_BATTERY_ONLY);
            chg_stm_set_chgr_type(CHG_CHGR_INVALID);
            chg_send_stat_to_app((uint32_t)DEVICE_ID_WIRELESS, (uint32_t)CHG_EVENT_NONEED_CARE);
            return ;
        }

    }
#endif/*MBB_CHG_WIRELESS*/
    else
    {
        chg_print_level_message(CHG_MSG_DEBUG, "CHG_PLT:PLUG OUT CHG TYPE UNKNOW!\n ");
    }
}

#define TABLE_NUM    g_chg_hardware_data->adc2temp_map->size

void chg_hardware_paras_dump(void)
{
    int i;

    if(NULL == g_chg_hardware_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"no valid data!!\n");
    }
    chg_print_level_message(CHG_MSG_ERR,"id_voltage_min=%d\n",g_chg_hardware_data->id_voltage_min);  
    chg_print_level_message(CHG_MSG_ERR,"id_voltage_max=%d\n",g_chg_hardware_data->id_voltage_max);  
    chg_print_level_message(CHG_MSG_ERR,"batt_volt_hkadc_id=%d\n",g_chg_hardware_data->batt_volt_hkadc_id);  
    chg_print_level_message(CHG_MSG_ERR,"vbus_volt_hkadc_id=%d\n",g_chg_hardware_data->vbus_volt_hkadc_id);
    chg_print_level_message(CHG_MSG_ERR,"batt_temp_hkadc_id=%d\n",g_chg_hardware_data->batt_temp_hkadc_id);  
    chg_print_level_message(CHG_MSG_ERR,"batt_cali_volt_min=%d\n",g_chg_hardware_data->batt_cali_volt_min);
    chg_print_level_message(CHG_MSG_ERR,"batt_cali_volt_max=%d\n",g_chg_hardware_data->batt_cali_volt_max);  
    chg_print_level_message(CHG_MSG_ERR,"batt_bad_volt=%d\n",g_chg_hardware_data->batt_bad_volt);
    chg_print_level_message(CHG_MSG_ERR,"batt_valid_temp=%d\n",g_chg_hardware_data->batt_valid_temp);  
    chg_print_level_message(CHG_MSG_ERR,"extchg_stop_temp=%d\n",g_chg_hardware_data->extchg_stop_temp);
    chg_print_level_message(CHG_MSG_ERR,"extchg_resume_temp=%d\n",g_chg_hardware_data->extchg_resume_temp);  
    chg_print_level_message(CHG_MSG_ERR,"chg_ic_en_gpio=%d\n",g_chg_hardware_data->chg_ic_en_gpio);
    
    for(i=0;i<CHG_STM_MAX_ST;i++)
    {

        chg_print_level_message(CHG_MSG_ERR,"***********std paras,stm_status=%d*************\n",i); 
        chg_print_level_message(CHG_MSG_ERR,"supply_current_limit=%d\n",g_chg_hardware_data->chg_std_chgr_hw_paras->pwr_supply_current_limit_in_mA);     
        chg_print_level_message(CHG_MSG_ERR,"chg_current_limit=%d\n",g_chg_hardware_data->chg_std_chgr_hw_paras->chg_current_limit_in_mA); 
        chg_print_level_message(CHG_MSG_ERR,"CV_volt=%d\n",g_chg_hardware_data->chg_std_chgr_hw_paras->chg_CV_volt_setting_in_mV);     
        chg_print_level_message(CHG_MSG_ERR,"taper_current=%d\n",g_chg_hardware_data->chg_std_chgr_hw_paras->chg_taper_current_in_mA);
        chg_print_level_message(CHG_MSG_ERR,"chg_is_enabled=%d\n",g_chg_hardware_data->chg_std_chgr_hw_paras->chg_is_enabled);     
    }
    for(i=0;i<CHG_STM_MAX_ST;i++)
    {
        chg_print_level_message(CHG_MSG_ERR,"***********usb2 paras,stm_status=%d*************\n",i); 
        chg_print_level_message(CHG_MSG_ERR,"supply_current_limit=%d\n",g_chg_hardware_data->chg_usb_chgr_hw_paras->pwr_supply_current_limit_in_mA);     
        chg_print_level_message(CHG_MSG_ERR,"chg_current_limit=%d\n",g_chg_hardware_data->chg_usb_chgr_hw_paras->chg_current_limit_in_mA); 
        chg_print_level_message(CHG_MSG_ERR,"CV_volt=%d\n",g_chg_hardware_data->chg_usb_chgr_hw_paras->chg_CV_volt_setting_in_mV);     
        chg_print_level_message(CHG_MSG_ERR,"taper_current=%d\n",g_chg_hardware_data->chg_usb_chgr_hw_paras->chg_taper_current_in_mA);
        chg_print_level_message(CHG_MSG_ERR,"chg_is_enabled=%d\n",g_chg_hardware_data->chg_usb_chgr_hw_paras->chg_is_enabled);     
    }
    for(i=0;i<CHG_STM_MAX_ST;i++)
    {
        chg_print_level_message(CHG_MSG_ERR,"***********usb3 paras,stm_status=%d*************\n",i);
        chg_print_level_message(CHG_MSG_ERR,"supply_current_limit=%d\n",g_chg_hardware_data->chg_usb3_chgr_hw_paras->pwr_supply_current_limit_in_mA);     
        chg_print_level_message(CHG_MSG_ERR,"chg_current_limit=%d\n",g_chg_hardware_data->chg_usb3_chgr_hw_paras->chg_current_limit_in_mA); 
        chg_print_level_message(CHG_MSG_ERR,"CV_volt=%d\n",g_chg_hardware_data->chg_usb3_chgr_hw_paras->chg_CV_volt_setting_in_mV);     
        chg_print_level_message(CHG_MSG_ERR,"taper_current=%d\n",g_chg_hardware_data->chg_usb3_chgr_hw_paras->chg_taper_current_in_mA);
        chg_print_level_message(CHG_MSG_ERR,"chg_is_enabled=%d\n",g_chg_hardware_data->chg_usb3_chgr_hw_paras->chg_is_enabled);     
    }
    
    chg_print_level_message(CHG_MSG_ERR,"***********volt2temp,num=%d*************\n",TABLE_NUM);    
    for(i=0;i<TABLE_NUM;i++)
    {
        chg_print_level_message(CHG_MSG_ERR,"temp:%d  volt:%d\n",g_chg_hardware_data->adc2temp_map->adc_batt_therm_map[i].temperature,g_chg_hardware_data->adc2temp_map->adc_batt_therm_map[i].voltage);        
    }
}

int32_t chg_hardware_paras_init(void)
{
    int32_t hw_id = 0;
    hw_id = chg_get_hardware_data_id();
    g_chg_hardware_data = chg_get_hardware_data(hw_id);
    if(NULL == g_chg_hardware_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get hardware data fail!!\n");    
        return -1;    
    }
    return 0;
}


struct chg_batt_data *chg_get_batt_data(unsigned int id_voltage)
{
    int i;

    for (i = (BATTERY_DATA_ARRY_SIZE - 1); i > 0; i--){
        if ((id_voltage >= chg_batt_data_array[i]->id_voltage_min)
            && (id_voltage <= chg_batt_data_array[i]->id_voltage_max)){
            break;
        }
    }

    return chg_batt_data_array[i];
}


int32_t chg_batt_volt_paras_init(void)
{
    int32_t batt_id = 0;
    batt_id = chg_get_batt_id_volt();
    g_chg_batt_data = chg_get_batt_data(batt_id);
    if(NULL == g_chg_batt_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get batt data fail!!\n");    
        return -1;    
    }
    #if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
    || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
    if(FALSE == is_5577cs_falg)
    {
        if((g_chg_batt_data->batt_id == CHG_BATT_ID_XINGWANGDA_3000)
            ||(g_chg_batt_data->batt_id == CHG_BATT_ID_FEIMAOTUI_3000)
            ||(g_chg_batt_data->batt_id == CHG_BATT_ID_LISHEN_3000))
        {
            chg_high_bat_paras_init();//3000mA电池充电参数初始化
            is_bat_3000 = TRUE;

        }
        else
        {
            g_chg_batt_data = chg_batt_data_array[0];
            is_bat_3000 = FALSE;
        }
    }
    #endif
    //init batt para
    memcpy(&g_chgBattVoltProtect, (uint8_t*)&g_chg_batt_data->chg_batt_volt_paras, sizeof(CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE));
    
    return 0;
}
/*get batt brand*/
uint32_t chg_get_batt_id(void)
{
    if(NULL == g_chg_batt_data)
    {
        chg_print_level_message(CHG_MSG_ERR,"CHG_INIT:get batt data fail!!\n");    
        return CHG_BATT_ID_MAX;    
    }
    chg_print_level_message(CHG_MSG_DEBUG,"CHG: batt brand is %d\n",g_chg_batt_data->batt_id); 
    
    return g_chg_batt_data->batt_id;
}

void chg_batt_data_dump(void)
{
    chg_print_level_message(CHG_MSG_ERR,"battVoltPowerOnThreshold=%d\n",
        g_chgBattVoltProtect.battVoltPowerOnThreshold);  
    chg_print_level_message(CHG_MSG_ERR,"battVoltPowerOffThreshold=%d\n",
        g_chgBattVoltProtect.battVoltPowerOffThreshold); 
    chg_print_level_message(CHG_MSG_ERR,"battOverVoltProtectThreshold=%d\n",
        g_chgBattVoltProtect.battOverVoltProtectThreshold);  
    chg_print_level_message(CHG_MSG_ERR,"battOverVoltProtectOneThreshold=%d\n",
        g_chgBattVoltProtect.battOverVoltProtectOneThreshold); 
    chg_print_level_message(CHG_MSG_ERR,"battChgTempMaintThreshold=%d\n",
        g_chgBattVoltProtect.battChgTempMaintThreshold);  
    chg_print_level_message(CHG_MSG_ERR,"battChgRechargeThreshold=%d\n",
        g_chgBattVoltProtect.battChgRechargeThreshold); 
    chg_print_level_message(CHG_MSG_ERR,"VbatLevelLow_MAX=%d\n",g_chgBattVoltProtect.VbatLevelLow_MAX);  
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel0_MAX=%d\n",g_chgBattVoltProtect.VbatLevel0_MAX); 
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel1_MAX=%d\n",g_chgBattVoltProtect.VbatLevel1_MAX);  
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel2_MAX=%d\n",g_chgBattVoltProtect.VbatLevel2_MAX); 
    chg_print_level_message(CHG_MSG_ERR,"VbatLevel3_MAX=%d\n",g_chgBattVoltProtect.VbatLevel3_MAX); 
    chg_print_level_message(CHG_MSG_ERR,"battChgFirstMaintThreshold=%d\n",
        g_chgBattVoltProtect.battChgFirstMaintThreshold);  
    chg_print_level_message(CHG_MSG_ERR,"battNormalTempChgRechargeThreshold=%d\n",
        g_chgBattVoltProtect.battNormalTempChgRechargeThreshold); 
        
    chg_print_level_message(CHG_MSG_ERR,"batt brand is %d\n",g_chg_batt_data->batt_id);    
#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
    chg_print_level_message(CHG_MSG_ERR,"g_batt_normal_temp_recherge_threshold=%d\n",
        g_batt_normal_temp_recherge_threshold);
#endif
}
#if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
   || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
void chg_high_bat_paras_init()
{
    int i = 0;
    for(i=0;i<CHG_STM_MAX_ST;i++)
    {
        chg_std_chgr_hw_paras[i] = chg_std_chgr_hw_paras_h[i];
        chg_usb_chgr_hw_paras[i] = chg_usb_chgr_hw_paras_h[i];
        chg_usb3_chgr_hw_paras[i] = chg_usb3_chgr_hw_paras_h[i];
    }
}
#endif





#ifndef __HW_PNP_API_H__
#define __HW_PNP_API_H__
#include "usb_debug.h"
#include "android.h"
#include "hw_pnp.h"


USB_VOID pnp_register_usb_support_function(struct android_usb_function* usb_func);


USB_INT usb_pnp_system_type_get(USB_VOID);


USB_INT  usb_pnp_port_style_stat(USB_VOID);
/****************************************************************
 函 数 名  : pnp_if_gateway_mode
 功能描述  : 查询是否网关模式。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_INT  pnp_if_gateway_mode(USB_VOID);

USB_INT get_gateway_mode(USB_VOID);
/****************************************************************
 函 数 名  : debug_init
 功能描述  : debug初始化。
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_probe(USB_VOID);
/****************************************************************
 函 数 名  : pnp_remove
 功能描述  : pnp移除接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_remove(USB_VOID);

/****************************************************************
 函 数 名  : pnp_set_rewind_param
 功能描述  : 设置切换命令
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_set_rewind_param(USB_UINT8* cmnd);
/****************************************************************
 函 数 名  : pnp_is_service_switch
 功能描述  : 是否服务切换(RNDIS MASS GET MAX LUN切换)
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_INT pnp_is_service_switch(USB_VOID);
/****************************************************************
 函 数 名  : pnp_is_rewind_before_mode
 功能描述  : 是否切换前(是否满足切换条件)
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_INT pnp_is_rewind_before_mode(USB_VOID);
/****************************************************************
 函 数 名  : pnp_is_rewind_before_mode
 功能描述  : 是否服务multi lun条件
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_INT pnp_is_multi_lun_mode(USB_VOID);
/****************************************************************
 函 数 名  : pnp_switch_normal_work_mode
 功能描述  : 切换到正常模式(相当于第一次连接单板到PC)
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_switch_normal_work_mode(USB_VOID);
/****************************************************************
 函 数 名  : pnp_switch_mbim_debug_mode
 功能描述  : 切换到MBIM调试模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_switch_mbim_debug_mode(USB_VOID);
/****************************************************************
 函 数 名  : pnp_switch_rndis_debug_mode
 功能描述  : 切换到RNDIS调试模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_switch_rndis_debug_mode(USB_VOID);
/****************************************************************
 函 数 名  : pnp_switch_rndis_project_mode
 功能描述  : 切换到RNDIS工程模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_switch_rndis_project_mode(USB_VOID);
/****************************************************************
 函 数 名  : pnp_switch_rewind_after_mode
 功能描述  : 切换到工作模式
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_switch_rewind_after_mode(USB_VOID);
/****************************************************************
 函 数 名  : usb_notify_syswatch
 功能描述  : USB事件
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
void usb_notify_syswatch(int deviceid, int eventcode);
/****************************************************************
 函 数 名  : pnp_switch_autorun_port
 功能描述  : pnp重新运行接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_switch_autorun_port(USB_VOID);

/****************************************************************
 函 数 名  : huawei_set_usb_enum_state
 功能描述  : 设置枚举状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID huawei_set_usb_enum_state(usb_enum_state state);

/****************************************************************
 函 数 名  : huawei_get_usb_enum_state
 功能描述  : 查询枚举状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 枚举状态值
*****************************************************************/
usb_enum_state huawei_get_usb_enum_state( USB_VOID );

/****************************************************************
 函 数 名  : usb_power_off_chg_stat
 功能描述  : 查询关机状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_INT usb_power_off_chg_stat(USB_VOID);
/****************************************************************
 函 数 名  : pnp_set_ctl_app_flag
 功能描述  : 设置app是否启动标志
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************/
USB_VOID pnp_set_ctl_app_flag(USB_INT flag);
/*****************************************************************************
 函 数 名  : pnp_switch_charge_only_port
 功能描述  : 切换功能函数:切换到仅充电端口状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************************/
USB_VOID pnp_switch_charge_only_port(USB_VOID);
/*****************************************************************************
 函 数 名  : pnp_switch_charge_only_port
 功能描述  : 查询当前端口形态是否静态端口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************************/
USB_INT  pnp_if_static_port_mode(USB_VOID);
/*****************************************************************************
 函 数 名  : pnp_if_cdrom_can_open
 功能描述  : 查询当前cdrom是否可读
 输入参数  : 无
 输出参数  : 无
 返 回 值  : NA
*****************************************************************************/
USB_INT pnp_if_cdrom_can_open(USB_VOID);


USB_INT get_nv_backup_flag(USB_VOID);

#ifdef CONFIG_NCM_MBIM_SUPPORT
USB_VOID pnp_switch_mbim_mode(USB_INT mode);
#endif

USB_VOID huawei_set_adress_flag(USB_INT state);

USB_INT huawei_get_adress_flag(USB_VOID);
USB_INT pnp_get_dload_flag(USB_VOID);

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
USB_INT pnp_get_dload_flag_ften(USB_VOID);
#endif

int usb_port_enable(char *name);

#endif

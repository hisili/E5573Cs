
#ifndef __BSP_VERSION_H__
#define __BSP_VERSION_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <bsp_shared_ddr.h>
#include <drv_version.h>

#include <bsp_om.h>

#define CFG_PRODUCT_TAG_ID "193c25990102eeb27f5ed9243d9a01e144e3efc4"
#define DLOAD_VERSION               "2.0"                   /* 下载协议版本 */
#define VERSION_MAX_LEN 32
#define VERSION_ISO_MAX_LEN  128
#define VERSION_WEBUI_MAX_LEN  128

#define HW_VERSION_ADDR SHM_MEM_HW_VER_ADDR

#ifndef VER_ERROR
#define VER_ERROR (-1)
#endif

#ifndef VER_OK
#define VER_OK 0
#endif

#ifdef __VXWORKS__
#define StrParamType (unsigned int)
#define MemparamType (int)
#else
#define StrParamType (int)
#define MemparamType (unsigned int)
#endif

/*log*/
#define  ver_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_VERSION, "[version]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define  ver_print_info(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_INFO, BSP_MODU_VERSION, "[version]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif

struct dload_info_type{
	char software_ver[VERSION_MAX_LEN];
	char product_name[VERSION_MAX_LEN];
	char iso_ver[VERSION_MAX_LEN];
};

/*****************************************************************************
* 函 数 名  	: bsp_get_hardware_version
*
* 功能描述  	: 获取硬件版本号
*
* 输入参数  	: 无
* 输出参数  	: 硬件版本号字符串指针
*
* 返 回 值  	: 0获取成功
			-1获取失败
*
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_hardware(void);
#if (FEATURE_ON == MBB_COMMON)
/*****************************************************************************
* 函 数 名   : bsp_version_get_hardware_no_subver
*
* 功能描述   : 获取硬件版本号
*
* 输入参数   : 无
* 输出参数   : 硬件版本号字符串指针
*
* 返 回 值   : 0获取成功
             -1获取失败
*
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_hardware_no_subver(void);
#endif

/*****************************************************************************
* 函 数 名  : bsp_get_product_inner_name
*
* 功能描述  : 获取内部产品名称
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : 内部产品名称字符串指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_product_inner_name(void);

/*****************************************************************************
* 函 数 名  : bsp_get_product_out_name
*
* 功能描述  : 获取内部产品名称
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : 外部产品名称字符串指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_product_out_name(void);

/*****************************************************************************
* 函 数 名  : bsp_get_pcb_version
*
* 功能描述  : 获取PCB
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : PCB字符串指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_pcb(void);

/*****************************************************************************
* 函 数 名  : bsp_get_dloadid_version
*
* 功能描述  : 获取dloadid
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : dloadid字符串指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_dloadid(void);

/*****************************************************************************
* 函 数 名  : bsp_get_build_date_time
*
* 功能描述  : 获取编译日期和时间
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : 软件版本号字符串指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_build_date_time(void);

/*****************************************************************************
* 函 数 名  : bsp_get_chip_version
*
* 功能描述  : 获取芯片版本号
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  :芯片版本号字符串的指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_chip(void);

/*****************************************************************************
* 函 数 名  : bsp_get_firmware_version
*
* 功能描述  : 获取软件版本号
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : 软件版本号字符串指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_firmware(void);

/*****************************************************************************
* 函 数 名  : bsp_get_version_hash
*
* 功能描述  : 获取软件中心库的hash值
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : 软件中心库hash值字符串的指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_hash(void);

/*****************************************************************************
* 函 数 名  : bsp_get_iso_version
*
* 功能描述  : 获取iso_version
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : iso_version字符串的指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_iso(void);


void bsp_get_iso_version(char *str,int len);

#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* 函 数 名  : bsp_get_web_version
*
* 功能描述  : 获取webui_version
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : webui_version字符串的指针
* 修改记录  :
*
*****************************************************************************/
void bsp_get_web_version(char *str,int len);
#endif /* MBB_DLOAD */

/*****************************************************************************
* 函 数 名  : bsp_get_webui_version
*
* 功能描述  : 获取webui_version
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : webui_version字符串的指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_get_webui_version(void);

/*****************************************************************************
* 函 数 名  : bsp_get_release_version
*
* 功能描述  : 获取release_version
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : release_version字符串的指针
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_release(void);

int bsp_version_get_boardid(void);

/*****************************************************************************
* 函数	: bsp_get_board_chip_type
* 功能	: get board type and chip type
* 输入	: void
* 输出	: void
* 返回	: BOARD_TYPE_E
*****************************************************************************/
u32 bsp_version_get_board_type(void);

int bsp_dload_get_dload_info(struct dload_info_type* dload_info);

bool bsp_board_type_is_gutl(void);


int bsp_version_debug(void);


void bsp_version_init(void);


int bsp_productinfo_init(void);

/*****************************************************************************
* 函数	: bsp_get_board_actual_type
* 功能	: get board actual type 
* 输入	: void
* 输出	: void
* 返回	: BOARD_ACTUAL_TYPE_E       BBIT/SFT/ASIC
*
* 其它       : 无
*
*****************************************************************************/
BOARD_ACTUAL_TYPE_E bsp_get_board_actual_type(void);

#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* 函数	: huawei_set_update_info
* 功能	: set update info 
* 输入	: void
* 输出	: void
* 返回	: void
*
* 其它       : 无
*
*****************************************************************************/
void huawei_set_update_info(char* str);

/*****************************************************************************
* 函数	: huawei_get_update_info_times
* 功能	: get update info times
* 输入	: NA 
* 输出	: times: num of upgrade 
* 返回	: void 
*
* 其它       : 无
*
*****************************************************************************/
void huawei_get_update_info_times(s32* times);

/*****************************************************************************
* 函数	: huawei_get_spec_num_upinfo
* 功能	: get info of once update 
* 输入	: num : index of upgrade
* 输出	: void
* 返回	: 0获取成功/-1获取失败
*
* 其它       : 无
*
*****************************************************************************/
s32 huawei_get_spec_num_upinfo(char* str, s32 str_len, s32 num);
#endif
#ifdef __cplusplus
}
#endif

#endif


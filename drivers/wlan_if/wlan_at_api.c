

/* linux 系统头文件 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>


/* 平台头文件 */
#include <product_config.h>
#include <mbb_config.h>

#include "wlan_at.h"

/* 对应的wifi芯片操作对象 */
STATIC WLAN_CHIP_OPS* g_wlan_ops[WLAN_CHIP_MAX] = {NULL};

/* 默认wt使用第一个芯片校准，当外部条件触发后，比如cradle插入等，需要修改该值逻辑 */
STATIC int32 g_cur_chip = -1;

/* 判断索引是否有效 */
#define CHIP_IDX_IS_VALID(idx) ((WLAN_CHIP_MAX > idx) && (idx > chipstub) && (NULL != g_wlan_ops[idx]))
/* 索引有效的断言 */
#define ASSERT_CHIP_IDX_VALID(idx, ret)  do { \
    if ((WLAN_CHIP_MAX <= idx) || (idx <= chipstub))\
    {\
        PLAT_WLAN_ERR("chip idx valid = %d", idx); \
        return ret;\
    }\
    if (NULL == g_wlan_ops[idx])\
    {\
        PLAT_WLAN_ERR("chip idx %d valid = NULL", idx); \
        return ret;\
    }\  
}while(0)

/* 平台提供的接口，判断是否产测模式 */
extern int bsp_get_factory_mode(void);

//////////////////////////////////////////////////////////////////////////
/*(1)^WIENABLE 设置WiFi模块使能 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
 功能描述  : 用于wifi 进入测试模式，正常模式，关闭wifi
 输入参数  :  0  关闭
              1  打开正常模式
              2  打开测试模式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiEnable, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiEnable(onoff);
}
/*****************************************************************************
 函数名称  : WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable()
 功能描述  : 获取当前的WiFi模块使能状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0  关闭
             1  正常模式(信令模式)
             2  测试模式(非信令模式)
 其他说明  : 
*****************************************************************************/
WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable(void)
{ 
    static int is_mp_start = 0;
    if (!is_mp_start)
    {
        is_mp_start = 1;
        (void)wlan_set_log_flag(WLAN_LOG((WLAN_LOG_STDOUT | WLAN_LOG_FILE), WLAN_LOG_DRV_LOW));
    }

    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_WIENABLE_OFF);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiEnable, AT_WIENABLE_OFF);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiEnable();
}

//////////////////////////////////////////////////////////////////////////
/*(2)^WIMODE 设置WiFi模式参数 目前均为单模式测试*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 功能描述  : 设置WiFi AP支持的制式
 输入参数  : 0,  CW模式
             1,  802.11a制式
             2,  802.11b制式
             3,  802.11g制式
             4,  802.11n制式
             5,  802.11ac制式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiMode, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiMode(mode);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取当前WiFi支持的制式
             当前模式，以字符串形式返回eg: 2
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiMode, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiMode(strBuf);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取WiFi芯片支持的所有协议模式
             支持的所有模式，以字符串形式返回eg: 2,3,4
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiModeSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiModeSupport(strBuf);
}

//////////////////////////////////////////////////////////////////////////
/*(3)^WIBAND 设置WiFi带宽参数 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE width)
 功能描述  : 用于设置wifi带宽
 输入参数  : 0 20M
             1 40M
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 只有在n模式下才可以设置40M带宽
*****************************************************************************/
int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE band)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiBand, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiBand(band);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取当前带宽配置 
            当前带宽，以字符串形式返回eg: 0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiBand, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiBand(strBuf);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取WiFi支持的带宽配置 
            支持带宽，以字符串形式返回eg: 0,1
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiBandSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiBandSupport(strBuf);
}

//////////////////////////////////////////////////////////////////////////
/*(4)^WIFREQ 设置WiFi频点 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 设置WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiFreq, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiFreq(pFreq);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 获取WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiFreq, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiFreq(pFreq);
}

//////////////////////////////////////////////////////////////////////////
/*(5)^WIDATARATE 设置和查询当前WiFi速率集速率
  WiFi速率，单位为0.01Mb/s，取值范围为0～65535 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiDataRate(uint32 rate)
 功能描述  : 设置WiFi发射速率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiDataRate(uint32 rate)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiDataRate, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiDataRate(rate);
}
/*****************************************************************************
 函数名称  : uint32 WlanATGetWifiDataRate()
 功能描述  : 查询当前WiFi速率设置
 输入参数  : NA
 输出参数  : NA
 返 回 值  : wifi速率
 其他说明  : 
*****************************************************************************/
uint32 WlanATGetWifiDataRate(void)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, 0);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiDataRate, 0);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiDataRate();
}

//////////////////////////////////////////////////////////////////////////
/*(6)^WIPOW 来设置WiFi发射功率 
   WiFi发射功率，单位为0.01dBm，取值范围为 -32768～32767 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiPOW(int32 power_dBm_percent)
 功能描述  : 设置WiFi发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiPOW(int32 power_dBm_percent)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiPOW, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiPOW(power_dBm_percent);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiPOW()
 功能描述  : 获取WiFi当前发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiPOW(void)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, 0);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiPOW, 0);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiPOW();
}

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX 来设置WiFi发射机开关 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 打开或关闭wifi发射机
 输入参数  : 0 关闭
             1 打开
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiTX, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiTX(onoff);
}

/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiTX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  : 
*****************************************************************************/
WLAN_AT_FEATURE_TYPE WlanATGetWifiTX(void)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_FEATURE_DISABLE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiTX, AT_FEATURE_DISABLE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiTX();
}

//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
int32 WlanATSetWifiRX(WLAN_AT_WIRX_STRU *params)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiRX, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiRX(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
 功能描述  : 获取wifi接收机的状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiRX, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiRX(params);
}

//////////////////////////////////////////////////////////////////////////
/*(9)^WIRPCKG 查询WiFi接收机误包码，上报接收到的包的数量*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 清除Wifi接收统计包为零
 输入参数  : 0 清除wifi统计包
             非0 无效参数
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiRPCKG(int32 flag)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiRPCKG, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiRPCKG(flag);
}

/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 查询WiFi接收机误包码，上报接收到的包的数量
 输入参数  : WLAN_AT_WIRPCKG_STRU *params
 输出参数  : uint16 good_result; //单板接收到的好包数，取值范围为0~65535
             uint16 bad_result;  //单板接收到的坏包数，取值范围为0~65535
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiRPCKG(WLAN_AT_WIRPCKG_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiRPCKG, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiRPCKG(params);
}

/*===========================================================================
 (10)^WIINFO 查询WiFi的相关信息
===========================================================================*/
/* 说明: WlanATGetWifiInfo 已在平台通过读nv实现，wifi不再实现此接口 */
/*int32 WlanATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiInfo, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiInfo(params);
}*/

//////////////////////////////////////////////////////////////////////////
/*(11)^WIPLATFORM 查询WiFi方案平台供应商信息 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 功能描述  : 查询WiFi方案平台供应商信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiPlatform, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiPlatform();
}

//////////////////////////////////////////////////////////////////////////
/*(12)^TSELRF 查询设置单板的WiFi射频通路*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATSetTSELRF(uint32 group)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetTSELRF, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetTSELRF(group);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的天线索引序列，以字符串形式返回eg: 0,1,2,3
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetTSELRFSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetTSELRFSupport(strBuf);
}

//////////////////////////////////////////////////////////////////////////
/*(13)^WiPARANGE设置、读取WiFi PA的增益情况*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
 功能描述  : 设置WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiParange, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiParange(pa_type);
}

/*****************************************************************************
 函数名称  : WLAN_AT_WiPARANGE_TYPE WlanATGetWifiParange()
 功能描述  : 读取WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
WLAN_AT_WiPARANGE_TYPE WlanATGetWifiParange(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_WiPARANGE_BUTT);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiParange, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiParange();
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的pa模式序列，以字符串形式返回eg: l,h
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiParangeSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiParangeSupport(strBuf);
}


/*===========================================================================
 (14)^WICALTEMP设置、读取WiFi的温度补偿值
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
 功能描述  : 设置WiFi的温度补偿值
 输入参数  : NA
 输出参数  : params:温度补偿参数
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATGetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalTemp, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalTemp(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
 功能描述  : 设置WiFi的温度补偿值
 输入参数  : params:温度补偿参数
 输出参数  : NA
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalTemp, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalTemp(params);
}

/*===========================================================================
 (15)^WICALDATA设置、读取指定类型的WiFi补偿数据
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalData(WLAN_AT_WICALDATA_STRU * params)
 功能描述  : 指定类型的WiFi补偿数据
 输入参数  : NA
 输出参数  : params:补偿数据
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATGetWifiCalData(WLAN_AT_WICALDATA_STRU * params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip,AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalData,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalData(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATSetWifiCalData(WLAN_AT_WICALDATA_STRU *params)
 功能描述  : 指定类型的WiFi补偿数据
 输入参数  : params:补偿数据
 输出参数  : NA
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalData(WLAN_AT_WICALDATA_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalData, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalData(params);
}

/*===========================================================================
 (16)^WICAL设置、读取校准的启动状态，是否支持补偿
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiCal(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 设置校准的启动状态
 输入参数  : onoff:0,结束校准；1,启动校准
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifiCal(WLAN_AT_FEATURE_TYPE onoff)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCal, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCal(onoff);
}

/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiCal(void)
 功能描述  : 读取校准的启动状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
WLAN_AT_FEATURE_TYPE WlanATGetWifiCal(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_FEATURE_DISABLE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCal, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCal();
}

/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiCalSupport(void)
 功能描述  : 是否支持校准
 输入参数  : NA
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
WLAN_AT_FEATURE_TYPE WlanATGetWifiCalSupport(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_FEATURE_DISABLE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalSupport, AT_FEATURE_DISABLE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalSupport();
}

/*===========================================================================
 (17)^WICALFREQ 设置、查询频率补偿值
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置频率补偿
 输入参数  : params:补偿参数
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalFreq, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalFreq(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置频率补偿
 输入参数  : NA
 输出参数  : params:补偿参数
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATGetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalFreq,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalFreq(params);
}

/*===========================================================================
 (18)^WICALPOW 设置、查询功率补偿值
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置功率补偿
 输入参数  : NA
 输出参数  : params:补偿参数
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalPOW(WLAN_AT_WICALPOW_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalPOW,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalPOW(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalPOW(WLAN_AT_WICALPOW_STRU *params)
 功能描述  : 校准发射功率时，查询对应值
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int32
*****************************************************************************/
int32 WlanATGetWifiCalPOW(WLAN_AT_WICALPOW_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalPOW,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalPOW(params);
}

/***********************************************************************************
Function:          wlan_at_switch_chip
Description:    wt 模块初始化
Calls:
Input:           
Output:            NA
Return:            NULL or node
                 
************************************************************************************/
void wlan_at_switch_chip(int chip_type)
{
	PLAT_WLAN_INFO("switch chip from (%d)%s to (%d)%s \n"
        , g_cur_chip, wlan_chip_type2name(g_cur_chip), chip_type, wlan_chip_type2name(chip_type));
    g_cur_chip = (int32)chip_type;
}
EXPORT_SYMBOL(wlan_at_switch_chip);

#if (MBB_WIFI_CHIP2 == bcm43236)
/***********************************************************************************
Function:          wlan_at_cradle_insert
Description:    wt 模块初始化
Calls:
Input:           
Output:            NA
Return:            NULL or node
                 
************************************************************************************/
int wlan_at_cradle_insert(void)
{
    if (bcm43236 == g_cur_chip)
    {
        return AT_RETURN_SUCCESS;
    }
    else
    {
        return AT_RETURN_FAILURE;
    }
    PLAT_WLAN_INFO("wlan at switch chip to bcm43236 %d\n", g_cur_chip);
}
EXPORT_SYMBOL(wlan_at_cradle_insert);
#endif

/*****************************************************************************
函数名称  : wlan_at_reg_chip
功能描述  : 芯片注册函数
输入参数  : NA
输出参数  : NA
返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
WLAN_AT_RETURN_TYPE wlan_at_reg_chip(int32 chip_type, WLAN_CHIP_OPS *chip_ops)
{
    /* 0:产线模式；1:正常使用模式，默认正常模式, 读取nv36，判断是否产线模式，暂未使用 */
#ifdef MBB_WIFI_CHECK_FACTORY_MODE
    int wlan_mode = 1; 

    wlan_mode = bsp_get_factory_mode();
    PLAT_WLAN_INFO("factory_mode = %d",wlan_mode);
    if (1 == wlan_mode)
    {
        /* 正常模式，返回 */
        return AT_RETURN_FAILURE;
    }
#endif /* MBB_WIFI_CHECK_FACTORY_MODE */

    if (chip_type < 0 || chip_type >= ARRAY_SIZE(g_wlan_ops))
    {
        PLAT_WLAN_ERR("chip idx valid = %d", chip_type);
        return AT_RETURN_FAILURE;
    }

    PLAT_WLAN_INFO("chip %d = (%s) reg %p to %p"
        , chip_type, wlan_chip_type2name(chip_type), g_wlan_ops[chip_type], chip_ops);
    g_wlan_ops[chip_type] = chip_ops;
    if (!CHIP_IDX_IS_VALID(g_cur_chip))
    {
        g_cur_chip = chip_type;
    }
    PLAT_WLAN_INFO("Exit, g_cur_chip= %d\n",g_cur_chip);
    return AT_RETURN_SUCCESS;
}
EXPORT_SYMBOL(wlan_at_reg_chip);

/*****************************************************************************
函数名称  : wlan_at_unreg_chip
功能描述  : 芯片去注册函数
输入参数  : NA
输出参数  : NA
返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
WLAN_AT_RETURN_TYPE wlan_at_unreg_chip(int32 chip_type)
{
    ASSERT_CHIP_IDX_VALID(chip_type, AT_RETURN_SUCCESS);
    
    PLAT_WLAN_INFO("Exit, chip %d = (%s) unreg from %p", chip_type, wlan_chip_type2name(chip_type), g_wlan_ops[chip_type]);
    g_wlan_ops[chip_type] = NULL;
    
    return AT_RETURN_SUCCESS;
}
EXPORT_SYMBOL(wlan_at_unreg_chip);

/*****************************************************************************
 函数名称  : wlan_wt_init
 功能描述  : wlan at root 初始化函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
static int __init wlan_wt_init(void)
{
    int i = 0;
    PLAT_WLAN_INFO("enter curr_chip %d = (%s)", g_cur_chip, wlan_chip_type2name(g_cur_chip));
    
    for (i = 0; i < ARRAY_SIZE(g_wlan_ops); i++)
    {
        if (NULL != g_wlan_ops[i])
        {
            PLAT_WLAN_INFO("chip %d = (%s) reg to %p", i, wlan_chip_type2name(i), g_wlan_ops[i]);
        }
    }

    PLAT_WLAN_INFO("exit");
    return AT_RETURN_SUCCESS;
}

 /*****************************************************************************
 函数名称  : wlan_wt_exit
 功能描述  : wlan at root 去初始化函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
static void __exit wlan_wt_exit(void)
{
    PLAT_WLAN_INFO("enter, curr_chip = %d", g_cur_chip);
    memset(g_wlan_ops,0x0,sizeof(g_wlan_ops));
    g_cur_chip = -1;
    PLAT_WLAN_INFO("exit");
}

module_init(wlan_wt_init); /*lint !e529*/
module_exit(wlan_wt_exit); /*lint !e529*/

MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("Huawei WT");
MODULE_LICENSE("GPL");

//////////////////////////////////////////////////////////////////////////
EXPORT_SYMBOL(WlanATSetWifiEnable);
EXPORT_SYMBOL(WlanATGetWifiEnable);
EXPORT_SYMBOL(WlanATSetWifiMode);
EXPORT_SYMBOL(WlanATGetWifiMode);
EXPORT_SYMBOL(WlanATGetWifiModeSupport);
EXPORT_SYMBOL(WlanATSetWifiBand);
EXPORT_SYMBOL(WlanATGetWifiBand);
EXPORT_SYMBOL(WlanATGetWifiBandSupport);
EXPORT_SYMBOL(WlanATSetWifiFreq);
EXPORT_SYMBOL(WlanATGetWifiFreq);
EXPORT_SYMBOL(WlanATSetWifiDataRate);
EXPORT_SYMBOL(WlanATGetWifiDataRate);
EXPORT_SYMBOL(WlanATSetWifiPOW);
EXPORT_SYMBOL(WlanATGetWifiPOW);
EXPORT_SYMBOL(WlanATSetWifiTX);
EXPORT_SYMBOL(WlanATGetWifiTX);
EXPORT_SYMBOL(WlanATSetWifiRX);
EXPORT_SYMBOL(WlanATGetWifiRX);
EXPORT_SYMBOL(WlanATSetWifiRPCKG);
EXPORT_SYMBOL(WlanATGetWifiRPCKG);
EXPORT_SYMBOL(WlanATGetWifiPlatform);
EXPORT_SYMBOL(WlanATSetTSELRF);
EXPORT_SYMBOL(WlanATGetTSELRFSupport);
EXPORT_SYMBOL(WlanATSetWifiParange);
EXPORT_SYMBOL(WlanATGetWifiParange);
EXPORT_SYMBOL(WlanATGetWifiParangeSupport);
EXPORT_SYMBOL(WlanATSetWifiCalTemp);
EXPORT_SYMBOL(WlanATGetWifiCalTemp);
EXPORT_SYMBOL(WlanATSetWifiCalData);
EXPORT_SYMBOL(WlanATSetWifiCal);
EXPORT_SYMBOL(WlanATGetWifiCal);
EXPORT_SYMBOL(WlanATGetWifiCalSupport);
EXPORT_SYMBOL(WlanATSetWifiCalPOW);
EXPORT_SYMBOL(WlanATGetWifiCalFreq);

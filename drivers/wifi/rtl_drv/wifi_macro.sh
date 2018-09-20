#!/bin/sh

#******************************************************************#
#***************               配置说明                 ***********#
#******************************************************************#
#CONFIG_EXT_CLK_26M         //是否支持26m时钟晶振
#CONFIG_PHY_EAT_40MHZ       //是否支持40m时钟晶振
#CONFIG_RTL_88E_SUPPORT     //RTL8189芯片需要配置为y
#CONFIG_RTL_92E_SUPPORT     //RTL8192芯片需要配置为y
#WIFI_MODULE_NAME           //wifi 驱动的模块名称

#CONFIG_AUTOCH_TIMER            //支持动态自动信道选择特性
#CFG_RTL_SDIO30                 //支持SDIO3.0特性
#SDIO_AP_PS                     //支持低功耗模式(即，支持WiFi唤醒BB，带电池产品待机省电功能)
#INTEL_BEACON_POWER_NO_INC      //关闭针对intel网卡兼容性远场提升power的处理
#WLAN_PLATFORM_POWER_EXT_LNA    //支持外挂LNA

#WLAN_PLATFORM_BALONG_V3        //巴龙V3平台特性宏
#WLAN_PLATFORM_BALONG_V7        //巴龙V7平台特性宏
#WLAN_PLATFORM_QUALCOMM_9x15    //高通9x15平台特性宏
#WLAN_PLATFORM_HUAWEI_COMMON    //华为产品需要开启此特性
#WLAN_PLATFORM_HUAWEI_FACTORY   //烧片版本需要定义此特性

#代码中的宏定义,根据不同产品定义不同的宏 
#CFG_WIFI_EXTRA_NORMAL      //为升级版本需要定义的宏
#CFG_WIFI_EXTRA_FACTORY     //为烧片版本需要定义的宏
#CFG_WIFI_EXTRA_COMMON      //为升级和烧片版本均需要定义的宏

#******************************************************************#
#***************               配置实现                 ***********#
#******************************************************************#
WIFI_MODULE_NAME=rtl8192es
export CONFIG_RTL_92E_SUPPORT=y
export CONFIG_PHY_EAT_40MHZ=y

#默认宏定义
CFG_WIFI_EXTRA_NORMAL=""
CFG_WIFI_EXTRA_COMMON="-DCFG_RTL_SDIO30 -DWLAN_PLATFORM_BALONG_V7 -DINTEL_BEACON_POWER_NO_INC -DSOFTAP_PS_DURATION -DSDIO_AP_PS"
export CFG_WIFI_EXTRA_CFLAGS="${CFG_WIFI_EXTRA_COMMON}"
#产品宏定义



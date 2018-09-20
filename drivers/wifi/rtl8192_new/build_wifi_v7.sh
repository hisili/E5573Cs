#!/bin/sh

#******************************************************************#
#***************               配置实现                 ***********#
#******************************************************************#
#WIFI_HOME                  //驱动输出目录，包括打包文件(output目录)、编译生成中间文件(OUT_WIFI目录)等，默认配置为当前目录
#WIFI_OUTPUT_DIR            //打包文件目录，可定制。【注】：目录名称与rtl_drv目录下的ouput名称一样，则使用rtl_drv目录下的ouput
#WIFI_DRV_DIFF_DIR          //产品驱动与归一化驱动差异部分配置及代码目录，例如LNA特性特有的寄存器TXT档案
#WLAN_SET_DRV_CURRENT       //产品wifi 驱动电流宏,此宏用于修改wifi驱动电流寄存器0x74的值
#代码中的宏定义,根据不同产品定义不同的宏(参见rtl_drv/macro_readme.txt)，编译参数说明如下： 
#CFG_WIFI_EXTRA_NORMAL      //为升级版本需要定义的宏
#CFG_WIFI_EXTRA_FACTORY     //为烧片版本需要定义的宏
#CFG_WIFI_EXTRA_COMMON      //为升级和烧片版本均需要定义的宏

WIFI_HOME=$(cd "$(dirname "$0")"; pwd)

WIFI_MODULE_NAME=rtl8192es
export CONFIG_RTL_92E_SUPPORT=y
export CONFIG_PHY_EAT_40MHZ=y
WIFI_OUTPUT_DIR=output_balong_v7
WIFI_CHIP_DIR=wifi_rtl

#默认宏定义(wingle产品)
CFG_WIFI_EXTRA_NORMAL=""
CFG_WIFI_EXTRA_FACTORY="-DWLAN_PLATFORM_HUAWEI_FACTORY"
CFG_WIFI_EXTRA_COMMON="-DWLAN_PLATFORM_HUAWEI_COMMON -DSDIO_AP_PS  -DSOFTAP_PS_DURATION -DCFG_RTL_SDIO30 -DWLAN_PLATFORM_BALONG_V7 -DINTEL_BEACON_POWER_NO_INC"

#产品宏定义


if [ "${OBB_PRODUCT_NAME}" = "hi6921_v711_r218h" ] ;  then
    CFG_WIFI_EXTRA_NORMAL="${CFG_WIFI_EXTRA_NORMAL} -DSDIO_AP_PS -DSOFTAP_PS_DURATION"
    CFG_WIFI_EXTRA_COMMON="${CFG_WIFI_EXTRA_COMMON} "
fi

#产品差异部分的驱动目录定义(目录不存在则表示无差异, 不处理)
WIFI_DRV_DIFF_DIR=rtl_drv_diff/${OBB_PRODUCT_NAME}

#******************************************************************#
#***************               驱动编译                 ***********#
#******************************************************************#
cd ${WIFI_HOME}/../rtl_drv
if [ "${OBB_PRODUCT_NAME}" = "hi6921_v711_e5770s" ] ;  then
    WIFI_DRV_DIFF_DIR=rtl_drv_diff/${OBB_PRODUCT_NAME}/hi6921_v711_e5770s_923
    WIFI_OUTPUT_DIR=output_balong_v7_5770s
    . ./build_wifi_v7.sh
    
    cd ${WIFI_HOME}/../rtl_drv
    WIFI_DRV_DIFF_DIR=rtl_drv_diff/${OBB_PRODUCT_NAME}/hi6921_v711_e5770s_320
    WIFI_CHIP_DIR=wifi_rtl_320
 
fi

. ./build_wifi_v7.sh

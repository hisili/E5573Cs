#!/bin/sh

#******************************************************************#
#***************               配置实现                 ***********#
#******************************************************************#
#WIFI_HOME                  //驱动输出目录，包括打包文件(output目录)、编译生成中间文件(OUT_WIFI目录)等，默认配置为当前目录
#WIFI_OUTPUT_DIR            //打包文件目录，可定制。【注】：目录名称与rtl_drv目录下的ouput名称一样，则使用rtl_drv目录下的ouput
#WIFI_DRV_DIFF_DIR          //产品驱动与归一化驱动差异部分配置及代码目录，例如LNA特性特有的寄存器TXT档案
#代码中的宏定义,根据不同产品定义不同的宏(参见rtl_drv/macro_readme.txt)，编译参数说明如下： 
#CFG_WIFI_EXTRA_NORMAL      //为升级版本需要定义的宏
#CFG_WIFI_EXTRA_FACTORY     //为烧片版本需要定义的宏
#CFG_WIFI_EXTRA_COMMON      //为升级和烧片版本均需要定义的宏

WIFI_HOME=$(cd "$(dirname "$0")"; pwd)

WIFI_MODULE_NAME=rtl8192es
export CONFIG_RTL_92E_SUPPORT=y
export CONFIG_PHY_EAT_40MHZ=y
WIFI_OUTPUT_DIR=output_qualcomm_9x15

#默认宏定义
CFG_WIFI_EXTRA_NORMAL=""
CFG_WIFI_EXTRA_FACTORY="-DWLAN_PLATFORM_HUAWEI_FACTORY"
CFG_WIFI_EXTRA_COMMON="-DWLAN_PLATFORM_HUAWEI_COMMON -DWLAN_PLATFORM_QUALCOMM_9x15"

#产品宏定义
if [ -z "${CONFIG_FEATURE_PRODUCT_TYPE}" ];  then
    CONFIG_FEATURE_PRODUCT_TYPE="e5"
else
    CONFIG_FEATURE_PRODUCT_TYPE=`echo "${CONFIG_FEATURE_PRODUCT_TYPE}" | tr A-Z a-z`
fi
echo "[Wi-Fi]: wifi CONFIG_FEATURE_PRODUCT_TYPE = ${CONFIG_FEATURE_PRODUCT_TYPE}"

if [ "${CONFIG_FEATURE_PRODUCT_TYPE}" = "e5" ];  then
    CFG_WIFI_EXTRA_NORMAL="${CFG_WIFI_EXTRA_NORMAL} -DSDIO_AP_PS"
else
    CFG_WIFI_EXTRA_NORMAL="${CFG_WIFI_EXTRA_NORMAL} -DCONFIG_AUTOCH_TIMER"
fi

#******************************************************************#
#***************               驱动编译                 ***********#
#******************************************************************#
cd ${WIFI_HOME}/../rtl_drv
. ./build_wifi_9x15.sh
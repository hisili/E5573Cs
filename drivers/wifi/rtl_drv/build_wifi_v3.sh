#!/bin/sh

#******************************************************************#
#*******               project parameters                  ********#
#******************************************************************#
if [ -z "${WIFI_HOME}" ];  then
    WIFI_HOME=$(cd "$(dirname "$0")"; pwd)
fi
PROJ_HOME=$(cd "$(dirname "${WIFI_HOME}/../../../../../../../../.")"; pwd)
echo "[Wi-Fi]: PROJ_HOME = $PROJ_HOME"

#******************************************************************#
#*******               platform parameters                 ********#
#******************************************************************#
if [ -z "${OBB_JOBS}" ];  then
    OBB_JOBS="-j 40"
fi
if [ -z "${BALONG_TOPDIR}" ];  then
    export BALONG_TOPDIR=${PROJ_HOME}
fi
if [ -z "${OBB_PRODUCT_NAME}" ];  then
    export OBB_PRODUCT_NAME=hi6758_udp_e5
fi
if [ -z "${ANDROID_PLAT_PRODUCT_NAME}" ];  then
    export ANDROID_PLAT_PRODUCT_NAME=${OBB_PRODUCT_NAME}
fi
if [ -z "${OBB_PRODUCT_OBJ_DIR}" ];  then
    OBB_PRODUCT_OBJ_DIR=${PROJ_HOME}/build/delivery/${OBB_PRODUCT_NAME}/obj
fi
if [ "${OBB_PRODUCT_FACTORY_VER}" = "true" ];  then
    CONFIG_FEATURE_FACTORY="yes"
else
	CONFIG_FEATURE_FACTORY="no"
fi
echo "[Wi-Fi]: OBB_JOBS=${OBB_JOBS}, CONFIG_FEATURE_FACTORY=${CONFIG_FEATURE_FACTORY}, OBB_PRODUCT_FACTORY_VER = ${OBB_PRODUCT_FACTORY_VER}"

#******************************************************************#
#*******               compile env parameters              ********#
#******************************************************************#
#CONFIG_PLATFORM            //makefile采用的编译平台
export CONFIG_PLATFORM=ARM_PLATFORM_HUAWE

VERBOSE=1
export USE_CCACHE=y
export PRODUCT_CFG_BUILD_TYPE=ALLY
export CROSS_COMPILE=/opt/4.5.1/bin/arm-linux-
export CONFIG_PLATFORM_ARM_BALONG=y

export ANDROID_KERN_DIR=${PROJ_HOME}/modem/system/android-2.3.3_r1/hi6758/kernel
OUT_KERNEL=${OBB_PRODUCT_OBJ_DIR}/KERNEL_OBJ
WIFI_OUT_SYSTEM_BIN=${OBB_PRODUCT_OBJ_DIR}/android/target/product/${ANDROID_PLAT_PRODUCT_NAME}/system/bin

#增加wlan_if适配层目录引用
CFG_WIFI_EXTRA_COMMON="${CFG_WIFI_EXTRA_COMMON} -I${ANDROID_KERN_DIR}/drivers/wifi/wlan_if -include ${ANDROID_KERN_DIR}/drivers/wifi/wlan_if/wlan_exclude.h"

#******************************************************************#
#*********                build & output                 **********#
#******************************************************************#
. $(cd "$(dirname "$0")"; pwd)/build_wifi.sh
if [ -d ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin ];  then
    rsync -adF --exclude=.svn ${WIFI_OUTPUT_PATH}/ ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin
    echo "[Wi-Fi]: copy wifi to system = ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin"
fi
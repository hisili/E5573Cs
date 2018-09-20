#!/bin/sh

#******************************************************************#
#*******               project parameters                  ********#
#******************************************************************#
if [ -z "${WIFI_HOME}" ];  then
    WIFI_HOME=$(cd "$(dirname "$0")"; pwd)
fi
PROJ_HOME=$(cd "$(dirname "${WIFI_HOME}/../../../../../.")"; pwd)
echo "[Wi-Fi]: PROJ_HOME = $PROJ_HOME"

#******************************************************************#
#*******               platform parameters                 ********#
#******************************************************************#
if [ -z "${OBB_JOBS}" ];  then
    OBB_JOBS="-j 8"
fi

if [ -z "${CONFIG_FEATURE_FACTORY}" ];  then
    CONFIG_FEATURE_FACTORY="no"
fi

echo "[Wi-Fi]: OBB_JOBS=${OBB_JOBS}, CONFIG_FEATURE_FACTORY=${CONFIG_FEATURE_FACTORY}"

#******************************************************************#
#*******               compile env parameters              ********#
#******************************************************************#
#CONFIG_PLATFORM            	//makefile采用的编译平台
#CFG_MBB_FEATURE_MACRO_NAME		//驱动中引入的kernel编译宏的名称
#CFG_MBB_FEATURE_MACRO_FILE     //驱动中引入的kernel编译宏的定义文件
export CONFIG_PLATFORM=ARM_PLATFORM_HUAWE
export CFG_MBB_FEATURE_MACRO_NAME=FEATURE_HUAWEI_DFLAGS
export CFG_MBB_FEATURE_MACRO_FILE=${PROJ_HOME}/apps_proc/oe-core/meta/conf/huawei_flags.conf

VERBOSE=1
SYS_ROOT=${PROJ_HOME}/apps_proc/oe-core/build/tmp-eglibc/sysroots
export CROSS_COMPILE=${SYS_ROOT}/x86_64-linux/usr/bin/armv7a-vfp-neon-oe-linux-gnueabi/arm-oe-linux-gnueabi-
export ANDROID_KERN_DIR=${PROJ_HOME}/apps_proc/kernel
if [ -z "${MACHINE}" ];  then
    MACHINE="9615-cdp"
fi
OUT_KERNEL=${SYS_ROOT}/${MACHINE}/kernel
WIFI_OUT_SYSTEM_BIN=${PROJ_HOME}/apps_proc/eUAP/out/target/product/p915/system/bin
if [ ! -d ${WIFI_OUT_SYSTEM_BIN} ];  then
    echo "[Wi-Fi]: Error, [${WIFI_OUT_SYSTEM_BIN}] not exist, general ${OBB_PRODUCT_NAME} ${OBB_PRODUCT_PID} kernel frist!"
fi

#增加wlan_if适配层目录引用
CFG_WIFI_EXTRA_COMMON="${CFG_WIFI_EXTRA_COMMON} -I${ANDROID_KERN_DIR}/drivers/wifi/wlan_if"

#******************************************************************#
#*********                build & output                 **********#
#******************************************************************#
. ./build_wifi.sh
if [ -d ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin ];  then
    rsync -adF --exclude=.svn ${WIFI_OUTPUT_PATH}/ ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin
    echo "[Wi-Fi]: copy wifi to system = ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin"
fi
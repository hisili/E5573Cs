#!/bin/sh

#******************************************************************#
#*******               project parameters                  ********#
#******************************************************************#
if [ -z "${WIFI_HOME}" ];  then
    WIFI_HOME=$(cd "$(dirname "$0")"; pwd)
fi
if [ 0 -eq $(echo x${WIFI_HOME} | grep -c 'modem\/system\/external') ];  then
    PROJ_HOME=${WIFI_HOME}/../../../../../..
else
    PROJ_HOME=${WIFI_HOME}/../../..
fi
PROJ_HOME=$(cd "$(dirname "${PROJ_HOME}/.")"; pwd)
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
if [ -z "${CFG_PLATFORM}" ];  then
    export CFG_PLATFORM=hi6930_v7r2
fi
if [ -z "${OBB_PRODUCT_NAME}" ];  then
    export OBB_PRODUCT_NAME=hi6921_v711_wingle
fi
if [ -z "${CFG_OS_ANDROID_PRODUCT_NAME}" ];  then
    export CFG_OS_ANDROID_PRODUCT_NAME=balongv7r2
fi
if [ "${BUILD_MODE}" = "factory" ];  then
    CONFIG_FEATURE_FACTORY="yes"
else
	CONFIG_FEATURE_FACTORY="no"
fi
echo "[Wi-Fi]: OBB_JOBS=${OBB_JOBS}, CONFIG_FEATURE_FACTORY=${CONFIG_FEATURE_FACTORY}, build_mode = ${BUILD_MODE}"

#******************************************************************#
#*******               compile env parameters              ********#
#******************************************************************#
#CONFIG_PLATFORM            //makefile采用的编译平台
export CONFIG_PLATFORM=ARM_PLATFORM_HUAWE

export VERBOSE=1
export LINUXVER=3.4.5
#export CROSS_COMPILE=${PROJ_HOME}/modem/system/android/android_4.2_r1/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
export ANDROID_KERN_DIR=${PROJ_HOME}
export OUT_KERNEL=${ANDROID_KERN_DIR}/../out
if [ -d ${WIFI_DRIVER_PATH} ];  then
    OUT_KERNEL=$(cd "$(dirname "${OUT_KERNEL}/.")"; pwd)
fi
mkdir -p ${OUT_KERNEL}/system/bin
WIFI_OUT_SYSTEM_BIN=${OUT_KERNEL}/system/bin
if [ -d ${WIFI_DRIVER_PATH} ];  then
    WIFI_OUT_SYSTEM_BIN=$(cd "$(dirname "${WIFI_OUT_SYSTEM_BIN}/.")"; pwd)
fi

#增加wlan_if适配层目录引用
CFG_WIFI_EXTRA_COMMON="${CFG_WIFI_EXTRA_COMMON} -I${PROJ_HOME}/modem/drv/acore/kernel/drivers/wlan_if"

#******************************************************************#
#*********                build & output                 **********#
#******************************************************************#
. $(cd "$(dirname "$0")"; pwd)/build_wifi.sh
if [ -d ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin ];  then
    rsync -adF --exclude=.svn ${WIFI_OUTPUT_PATH}/ ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin
    echo "[Wi-Fi]: copy wifi to system = ${PROJ_HOME}/modem/eUAP/out/target/product/p711/system/bin"
fi
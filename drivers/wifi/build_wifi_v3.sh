#!/bin/bash

#get CFG_PLATFORM_CHIP_TYPE
if [ -z "${CFG_PLATFORM_CHIP_TYPE}" ];  then
    CFG_PLATFORM_CHIP_TYPE=hi6758
fi

if [ -z "${BALONG_TOPDIR}" ];  then
    BALONG_TOPDIR=$(cd "$(dirname "$0")"; pwd)/../../../../../../..
    export BALONG_TOPDIR=$(cd "$(dirname "${BALONG_TOPDIR}/.")"; pwd)
fi

#old wifi build clean
if [ -z "${OBB_PRODUCT_NAME}" ];  then
    export OBB_PRODUCT_NAME=hi6758_udp_e5
fi
if [ -z "${ANDROID_PLAT_PRODUCT_NAME}" ];  then
    export ANDROID_PLAT_PRODUCT_NAME=${OBB_PRODUCT_NAME}
fi
if [ -z "${OBB_PRODUCT_OBJ_DIR}" ];  then
    OBB_PRODUCT_OBJ_DIR=${BALONG_TOPDIR}/build/delivery/${OBB_PRODUCT_NAME}/obj
fi
WIFI_OUT_SYSTEM_BIN=${OBB_PRODUCT_OBJ_DIR}/android/target/product/${ANDROID_PLAT_PRODUCT_NAME}/system/bin
rm -f ${WIFI_OUT_SYSTEM_BIN}/wifi_init.sh
rm -rf ${WIFI_OUT_SYSTEM_BIN}/wifi_rtl
rm -rf ${WIFI_OUT_SYSTEM_BIN}/wifi_brcm

#get ANDROID_KERN_DIR
if [ -z "${ANDROID_KERN_DIR}" ];  then
    ANDROID_KERN_DIR=${BALONG_TOPDIR}/modem/system/android-2.3.3_r1/${CFG_PLATFORM_CHIP_TYPE}/kernel
fi

#get WIFI_DIR_NAME
WIFI_DIR_NAME=${CONFIG_FEATURE_WIFI_CHIP}

#product E5330BS
if [ "x${OBB_PRODUCT_PID}" = "xE5330BS" -o "x${OBB_PRODUCT_PID}" = "xE5336BS" ];  then
    WIFI_DIR_NAME=rtl8189
fi

#default
if [ -z "${WIFI_DIR_NAME}" ];  then
    WIFI_DIR_NAME=bcm43362
fi

#build wifi
echo "[Wi-Fi]: build wifi = ${ANDROID_KERN_DIR}/drivers/wifi/${WIFI_DIR_NAME}"
cd ${ANDROID_KERN_DIR}/drivers/wifi/${WIFI_DIR_NAME}
./build_wifi_v3.sh
if [ ! "$?" = "0" ]; then
    echo "[Wi-Fi]: Error, build wifi driver ${WIFI_DIR_NAME} failed"
    exit 255
fi
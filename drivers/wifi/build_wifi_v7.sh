#!/bin/bash
set -e
#export BALONG_TOPDIR=$1
#export CFG_OS_ANDROID_PRODUCT_NAME=$2
#export OBB_ANDROID_DIR=$3

#get CFG_PLATFORM_CHIP_TYPE
echo "[Wi-Fi]: BALONG_TOPDIR = ${BALONG_TOPDIR}"
echo "[Wi-Fi]: CFG_OS_ANDROID_PRODUCT_NAME = ${CFG_OS_ANDROID_PRODUCT_NAME}"
echo "[Wi-Fi]: OBB_ANDROID_DIR = ${OBB_ANDROID_DIR}"

#old wifi build clean
#WIFI_OUT_SYSTEM_BIN=${OBB_ANDROID_DIR}/out/target/product/${CFG_OS_ANDROID_PRODUCT_NAME}/system/bin
#rm -rf ${WIFI_OUT_SYSTEM_BIN}/wifi\_*

#get WIFI_DIR_NAME
WIFI_DIR_NAME=rtl8192
OBB_PRODUCT_NAME=hi6921_v711_r218h
MBB_WIFI_CHIP2=FEATURE_OFF
echo "[Wi-Fi]: chip1 = ${WIFI_DIR_NAME}"
#编译第一个芯片对应的驱动
if [ "${WIFI_DIR_NAME}" != "FEATURE_OFF" ]; then
	if [ "${WIFI_DIR_NAME}" = "rtl8192" ] \
       && [ "${OBB_PRODUCT_NAME}" != "hi6921_v711_wingle" ] \
       && [ "${OBB_PRODUCT_NAME}" != "hi6921_v711_wingle-153" ] \
       && [ "${OBB_PRODUCT_NAME}" != "hi6921_v711_wingletelstra" ] \
    ; then
		WIFI_DIR_NAME=rtl8192_new
	fi
    if [ "${OBB_PRODUCT_NAME}" == "hi6921_v711_e5573cs" ] \
       || [ "${OBB_PRODUCT_NAME}" == "hi6921_v711_e5573cs-323" ] \
       || [ "${OBB_PRODUCT_NAME}" == "hi6921_v711_e5573cs-933" ] \
    ; then
        WIFI_DIR_NAME=rtl8189_8192
    fi
    #build wifi
    cd ./rtl8192_new
	chmod 777 build_wifi_v7.sh
    ./build_wifi_v7.sh
    if [ ! "$?" = "0" ]; then
        echo "[Wi-Fi]: Error11: build wifi driver wifi ${WIFI_DIR_NAME} failed"
        exit 255
    fi
fi

#get WIFI_DIR_NAME
WIFI_DIR_NAME=${MBB_WIFI_CHIP2}
echo "[Wi-Fi]: wifi chip2 = ${WIFI_DIR_NAME}"
#编译第二个芯片对应的驱动
if [ "${WIFI_DIR_NAME}" != "FEATURE_OFF" ]; then
    #build wifi
    cd ${OBB_ANDROID_DIR}/kernel/drivers/wifi/${WIFI_DIR_NAME}
    chmod 777 build_wifi_v7.sh
    ./build_wifi_v7.sh
    if [ ! "$?" = "0" ]; then
        echo "[Wi-Fi]: Error22: build wifi driver wifi ${WIFI_DIR_NAME} failed"
        exit 255
    fi
fi

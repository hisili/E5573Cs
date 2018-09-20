#!/bin/bash

WIFI_HOME=$(cd "$(dirname "$0")"; pwd)
ANDROID_HOME=${WIFI_HOME}/../../..
ANDROID_HOME=$(cd "$(dirname "${ANDROID_HOME}/.")"; pwd)

WIFI_DIR_NAME=${CONFIG_FEATURE_WIFI_CHIP}
#default
if [ -z "${WIFI_DIR_NAME}" ];  then
    WIFI_DIR_NAME=rtl8192
fi

#wifi clean
export WIFI_OUT_SYSTEM_BIN=${ANDROID_HOME}/eUAP/out/target/product/p915/system/bin
rm -f ${WIFI_OUT_SYSTEM_BIN}/wifi_init.sh
rm -rf ${WIFI_OUT_SYSTEM_BIN}/wifi_rtl
rm -rf ${WIFI_OUT_SYSTEM_BIN}/wifi_brcm

#wifi build
echo "[Wi-Fi]: build wifi:${WIFI_HOME}/${WIFI_DIR_NAME}"
cd ${WIFI_HOME}/${WIFI_DIR_NAME}
./build_wifi_9x15.sh
if [ ! "$?" = "0" ]; then
    echo "[Wi-Fi]: Error: build wifi driver ${WIFI_DIR_NAME} failed"
    exit 255
fi



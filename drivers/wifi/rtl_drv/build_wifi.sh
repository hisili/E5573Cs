#!/bin/sh

#******************************************************************#
#*******               check compile env                   ********#
#******************************************************************#
echo "[Wi-Fi]: ****** check compile env ****** "
WIFI_DRV_HOME=$(cd "$(dirname "$0")"; pwd)
WIFI_MAKEFILE_DIR=${WIFI_DRV_HOME}/3rdComponent/drv-src
if [ -z "${WIFI_HOME}" ];  then
    WIFI_HOME=${WIFI_DRV_HOME}
else
    WIFI_HOME=$(cd "$(dirname "${WIFI_HOME}/.")"; pwd)
fi
WIFI_DRV_BAK=${WIFI_HOME}/rtl_drv_bak
echo "[Wi-Fi]: WIFI_HOME = $WIFI_HOME"
echo "[Wi-Fi]: WIFI_MAKEFILE_DIR = $WIFI_MAKEFILE_DIR"

OUT_KERNEL=${WIFI_DRV_HOME}/../../../../out
echo "[Wi-Fi]: OUT_KERNEL = $OUT_KERNEL"
if [ -z "${OUT_KERNEL}" ] || [ ! -d ${OUT_KERNEL} ];  then
    echo "[Wi-Fi]: Error: [OUT_KERNEL = ${OUT_KERNEL}] not exist, Please build kernel frist!"
    exit 255
fi

echo "[Wi-Fi]: WIFI_OUT_SYSTEM_BIN = ${WIFI_OUT_SYSTEM_BIN}"
if [ ! -z "${WIFI_OUT_SYSTEM_BIN}" ] && [ ! -d ${WIFI_OUT_SYSTEM_BIN} ];  then
    mkdir -p ${WIFI_OUT_SYSTEM_BIN}
fi

if [ -z "${CFG_WIFI_EXTRA_COMMON}" ] && [ -z "${CFG_WIFI_EXTRA_NORMAL}" ] && [ -z "${CFG_WIFI_EXTRA_FACTORY}" ];  then
    echo "[Wi-Fi]: "
    chmod 777 . ${WIFI_DRV_HOME}/wifi_macro.sh
    . ${WIFI_DRV_HOME}/wifi_macro.sh
fi
if [ -z "${WIFI_MODULE_NAME}" ];  then
    echo "[Wi-Fi]: Error: WIFI_MODULE_NAME not exist, Please set macro shell frist!"
    exit 255
fi
echo "[Wi-Fi]: build_macro_common = ${CFG_WIFI_EXTRA_COMMON}"
echo "[Wi-Fi]: build_macro_normal = ${CFG_WIFI_EXTRA_NORMAL}"
echo "[Wi-Fi]: build_macro_factory = ${CFG_WIFI_EXTRA_FACTORY}"

#******************************************************************#
#*******                replace wifi drv to diff            *******#
#******************************************************************#
if [ "${WIFI_DRV_DIFF_DIR}" ] && [ -d "${WIFI_HOME}/${WIFI_DRV_DIFF_DIR}" ]; then
    rm -rf "${WIFI_DRV_BAK}"
    rsync -adF --exclude=.svn --exclude=.git -b --backup-dir="${WIFI_DRV_BAK}" "${WIFI_HOME}/${WIFI_DRV_DIFF_DIR}/" "${WIFI_DRV_HOME}"
    echo "[Wi-Fi]: replace to diff drv ${WIFI_HOME}/${WIFI_DRV_DIFF_DIR}"
fi

#******************************************************************#
#*******                check wifi output                  ********#
#******************************************************************#
WIFI_OUTPUT_DIR=${WIFI_DRV_HOME}/out
echo "[Wi-Fi]: ****** check wifi output ****** "
WIFI_OUTPUT_PATH=${WIFI_HOME}/${WIFI_OUTPUT_DIR}
echo "[Wi-Fi]: WIFI_OUTPUT_PATH = $WIFI_OUTPUT_PATH"
if [ -z "${WIFI_OUTPUT_DIR}" ];  then
    echo "[Wi-Fi]: Error: WIFI_OUTPUT_DIR is null!"
    exit 255
fi
#output dir can be customizable
if [ "${WIFI_HOME}" != "${WIFI_DRV_HOME}" ] && [ -d ${WIFI_DRV_HOME}/${WIFI_OUTPUT_DIR} ];  then
    rm -rf ${WIFI_OUTPUT_PATH}
    rsync -adF --exclude=.svn --exclude=.git ${WIFI_DRV_HOME}/${WIFI_OUTPUT_DIR} ${WIFI_HOME}/
fi
if [ -z "${WIFI_OUTPUT_DIR}" ];  then
    echo "[Wi-Fi]: Error: WIFI_OUTPUT_DIR is not exist!"
    exit 255
fi

if [ -z "${WIFI_CHIP_DIR}" ];  then
    WIFI_CHIP_DIR=wifi_rtl
fi

#set wifi init flags
#WIFI_INIT_FLAGS_FILE=${WIFI_OUTPUT_PATH}/${WIFI_CHIP_DIR}/exe/wifi_flags.sh
#echo "CONFIG_WLAN_MODULE_NAME=${WIFI_MODULE_NAME}" > ${WIFI_INIT_FLAGS_FILE}
#echo "[Wi-Fi]: CONFIG_WLAN_MODULE_NAME = ${WIFI_MODULE_NAME}"
#echo "CONFIG_WLAN_FEATURE_FACTORY=${CONFIG_FEATURE_FACTORY}" >> ${WIFI_INIT_FLAGS_FILE}
#echo "[Wi-Fi]: CONFIG_WLAN_FEATURE_FACTORY = ${CONFIG_FEATURE_FACTORY}"

#******************************************************************#
#*******               build wifi driver                  ********#
#******************************************************************#
echo "[Wi-Fi]: ****** build wifi driver ****** "
OUT_WIFI=${WIFI_HOME}/OUT_WIFI_BUILD
WIFI_DRIVER_PATH=${WIFI_OUTPUT_PATH}/${WIFI_CHIP_DIR}/driver
mkdir -p ${WIFI_DRIVER_PATH}
rm -rf ${WIFI_DRIVER_PATH}/*.ko
rm -rf "${OUT_WIFI}"
mkdir -p "${OUT_WIFI}"

cd ${WIFI_MAKEFILE_DIR}
echo "[Wi-Fi]: begin to build wifi driver ${WIFI_MODULE_NAME}"

#general factory driver
make clean
CFG_WIFI_EXTRA_NORMAL=""
CFG_WIFI_EXTRA_COMMON="-DCFG_RTL_SDIO30 -DWLAN_PLATFORM_BALONG_V7 -DINTEL_BEACON_POWER_NO_INC -DSOFTAP_PS_DURATION -DSDIO_AP_PS"
export CFG_WIFI_EXTRA_CFLAGS="${CFG_WIFI_EXTRA_COMMON}"
make -s -C ${WIFI_MAKEFILE_DIR} ${OBB_JOBS} V=${VERBOSE} O=${OUT_KERNEL}
rsync -a --include=.*.cmd --include=*.o --include=*.ko --exclude=*.* "${WIFI_MAKEFILE_DIR}/" "${OUT_WIFI}/MFG"
find "${OUT_WIFI}/MFG" -type d -empty -delete
if [ ! -e ${WIFI_MODULE_NAME}.ko ];  then
    echo "[Wi-Fi]: Error, generate wifi driver ${WIFI_MODULE_NAME}_mfg.ko failed"
    exit 255
fi
cp -f ${WIFI_MODULE_NAME}.ko "${WIFI_DRIVER_PATH}/${WIFI_MODULE_NAME}_mfg.ko"

#generate nornal driver
make clean
export CFG_WIFI_EXTRA_CFLAGS="${CFG_WIFI_EXTRA_COMMON} ${CFG_WIFI_EXTRA_NORMAL}"
make -s -C ${WIFI_MAKEFILE_DIR} ${OBB_JOBS} V=${VERBOSE} O=${OUT_KERNEL}
rsync -a --include=.*.cmd --include=*.o --include=*.ko --exclude=*.* "${WIFI_MAKEFILE_DIR}/" "${OUT_WIFI}/Normal"
find "${OUT_WIFI}/Normal" -type d -empty -delete
if [ ! -e ${WIFI_MODULE_NAME}.ko ];  then
    echo "[Wi-Fi]: Error, generate wifi driver ${WIFI_MODULE_NAME}.ko failed"
    exit 255
fi
cp -f ${WIFI_MODULE_NAME}.ko "${WIFI_DRIVER_PATH}/${WIFI_MODULE_NAME}.ko"

#******************************************************************#
#*******               generate to system                  ********#
#******************************************************************#
echo "[Wi-Fi]: ****** generate to system ****** "
#copy to system bin
echo "[Wi-Fi]: copy wifi to system = ${WIFI_OUT_SYSTEM_BIN}"
chmod 777 -R ${WIFI_OUTPUT_PATH}
rsync -a --exclude=.svn --exclude=.git "${WIFI_OUTPUT_PATH}/" "${WIFI_OUT_SYSTEM_BIN}"
WIFI_OUT_SYSTEM_BIN = ${PROJ_HOME}/modem/atpv2/out/target/product/p711/system/bin/
WIFI_OUT_DIR_EUAP_PATH=${PROJ_HOME}/modem/atpv2/out/target/product/p711/system/bin

WIFI_HOME_DIR=$(cd "$(dirname "$0")"; pwd)
RTL_MAKEFILE_DIR=${WIFI_HOME_DIR}
if [ 0 -eq $(echo x${RTL_MAKEFILE_DIR} | grep -c 'modem\/system\/external') ];  then
    PROJ_HOME_DIR=${RTL_MAKEFILE_DIR}/../../../../../../../..
else
    PROJ_HOME_DIR=${RTL_MAKEFILE_DIR}/../../../../../../../..
fi
PROJ_HOME_DIR=$(cd "$(dirname "${PROJ_HOME_DIR}/.")"; pwd)
echo "[Wi-Fi]: ****** check compile env ******${WIFI_HOME}  ${WIFI_MAKEFILE_DIR}   ${PROJ_HOME}"
cp -rf  ${WIFI_OUTPUT_PATH}/*   ${WIFI_OUT_DIR_EUAP_PATH}

#******************************************************************#
#*******                recover wifi drv to normal         ********#
#******************************************************************#
if [ -d "${WIFI_DRV_BAK}" ]; then
    rsync -adF --exclude=.svn --exclude=.git "${WIFI_DRV_BAK}/" "${WIFI_DRV_HOME}"
    rm -rf "${WIFI_DRV_BAK}"
    echo "[Wi-Fi]: recover drv to normal from ${WIFI_DRV_BAK}"
fi
#!/system/bin/busybox sh

echo balong_sdio.1 > /sys/bus/platform/drivers/balong_sdio/unbind
WIFI_RESULT=$?
echo "[wi-fi]: sdio unbind ret = ${WIFI_RESULT}"

ecall balong_rtl_power_off
WIFI_RESULT=$?
echo "[wi-fi]: power off ret = ${WIFI_RESULT}"

exit ${WIFI_RESULT}

#!/system/bin/busybox sh

ecall balong_rtl_power_on
WIFI_RESULT=$?
echo "[wi-fi]: power on ret = ${WIFI_RESULT}"

WIFI_RESULT=0
if [ "$(ls /sys/devices/platform/balong_sdio.1/mmc_host | echo $?)" != "0" ]; then
	echo balong_sdio.1 > /sys/bus/platform/drivers/balong_sdio/bind
	WIFI_RESULT=$?
fi
echo "[wi-fi]: sdio bind ret = ${WIFI_RESULT}"

usleep 20000

exit ${WIFI_RESULT}

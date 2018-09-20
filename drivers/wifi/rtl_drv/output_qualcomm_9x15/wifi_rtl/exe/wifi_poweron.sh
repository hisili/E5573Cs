#!/system/bin/busybox sh

echo "0 poweron:1" > /sys/devices/platform/wifi_at_dev/wifi_at_dev
echo "[wi-fi]: wifi chip power on = $?"

echo "msm_sdcc.2" > /sys/bus/platform/drivers/msm_sdcc/bind
WIFI_RESULT=$?
echo "[wi-fi]: sdio bind ret = ${WIFI_RESULT}"

usleep 20000

exit ${WIFI_RESULT}

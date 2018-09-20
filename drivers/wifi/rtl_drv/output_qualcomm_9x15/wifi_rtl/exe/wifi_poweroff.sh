#!/system/bin/busybox sh

echo "msm_sdcc.2" > /sys/bus/platform/drivers/msm_sdcc/unbind
WIFI_RESULT=$?
echo "[wi-fi]: sdio unbind ret = ${WIFI_RESULT}"

echo "0 poweron:0" > /sys/devices/platform/wifi_at_dev/wifi_at_dev
echo "[wi-fi]: wifi chip power off = $?"

exit ${WIFI_RESULT}

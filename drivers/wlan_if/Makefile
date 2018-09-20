#
# Makefile for wlan_if
#

include $(BALONG_TOPDIR)/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

obj-y	+= wlan_utils.o
obj-y	+= wlan_security.o
obj-y	+= wlan_if.o

# mutil wifi chips support
$(info [wi-fi]: CHIP1 = $(MBB_WIFI_CHIP1); CHIP2 = $(MBB_WIFI_CHIP2))
ifneq ($(MBB_WIFI_CHIP1),FEATURE_OFF)
    WLAN_AT_API_CHIP := wlan_at_api_$(MBB_WIFI_CHIP1)
    EXTRA_CFLAGS += -DFEATURE_HUAWEI_MBB_$(shell echo '$(MBB_WIFI_CHIP1)' | tr '[a-z]' '[A-Z]')
    ifneq ($(filter rtl81%, $(MBB_WIFI_CHIP1)),)
        WLAN_AT_API_CHIP := wlan_at_api_rtl_def
    endif
    obj-y += $(WLAN_AT_API_CHIP).o
    $(info [wi-fi]: build $(WLAN_AT_API_CHIP).c)
endif

ifneq ($(MBB_WIFI_CHIP2),FEATURE_OFF)
    WLAN_AT_API_CHIP := wlan_at_api_$(MBB_WIFI_CHIP2)
    EXTRA_CFLAGS += -DFEATURE_HUAWEI_MBB_$(shell echo '$(MBB_WIFI_CHIP2)' | tr '[a-z]' '[A-Z]')
    ifneq ($(filter rtl81%, $(MBB_WIFI_CHIP2)),)
        WLAN_AT_API_CHIP := wlan_at_api_rtl_def
    endif
    obj-y += $(WLAN_AT_API_CHIP).o
    $(info [wi-fi]: build $(WLAN_AT_API_CHIP).c)
endif
$(info [wi-fi]: EXTRA_CFLAGS = $(EXTRA_CFLAGS))
# chip root reg 
obj-y	+= wlan_at_api.o



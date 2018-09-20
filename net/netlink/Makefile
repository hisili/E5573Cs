#
# Makefile for the netlink driver.
#

#/******************************************************************************************************
#     问题单号                       时间                   修改人                        说明
#*****************************************************************************************************/

obj-y := af_netlink.o genetlink.o hw_netlink.o atp_hw_netlink.o

ifeq ($(MBB_COMMON), $(FEATURE_ON))
obj-y += syswatch_ProMonitor.o
endif
obj-y += $(if $(CONFIG_ATP_CT_CLEAN), hw_ctclean.o)

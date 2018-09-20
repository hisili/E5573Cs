# Created 2013-07-11

# base Config
include $(BALONG_TOPDIR)/build/scripts/make_base.mk

# XML
NV_PRODUCT_XML              := $(BALONG_TOPDIR)/config/nvim/data/product/$(CFG_PRODUCT_HISILICON_VERSION)/*
TERMINAL_NV_DIFF_XML_PATH   := $(BALONG_TOPDIR)/../../vendor/huawei/manufacture/modem_NV/*

# .h or .txt
NV_PRODUCT_HEADFILES        := $(BALONG_TOPDIR)/include/nv/product/*

NV_PRODUCT_HEADFILE_FOLDER  := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/HeadFile
NV_PRODUCT_CUST_HEADFILE_FOLDER  := $(NV_PRODUCT_HEADFILE_FOLDER)/product
NV_PRODUCT_XMLDBFILE_FOLDER := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/XmlDbFiles

.PHONY: all do_product_nv_build
all: do_product_nv_build

do_product_nv_build: 
	$(Q)cp -rf $(NV_PRODUCT_HEADFILES) $(NV_PRODUCT_CUST_HEADFILE_FOLDER)
	$(Q)cp -rf $(NV_PRODUCT_XML) $(NV_PRODUCT_XMLDBFILE_FOLDER)
ifeq ($(OBB_PRODUCT_NAME),hi3630_udp)
	$(Q)cp -rf $(TERMINAL_NV_DIFF_XML_PATH) $(NV_PRODUCT_XMLDBFILE_FOLDER)
endif	

.PHONY: clean
clean:
#	$(Q)-rm -f $(GUNV_FILES)
#	$(Q)-rm -f $(GUNV_XML_FILES)	

.PHONY: FORCE
FORCE:;
# Created by MichaelYao/y00184236/1012-03-30

# base Config
include $(BALONG_TOPDIR)/build/scripts/make_base.mk

# XML $(CFG_PRODUCT_HISILICON_VERSION)/
NV_GU_COMMON_XML            := $(BALONG_TOPDIR)/config/nvim/data/gu/*.xml
NV_GU_DIFF_XML              := $(BALONG_TOPDIR)/config/nvim/data/gu/$(CFG_PRODUCT_HISILICON_VERSION)/*.xml
NV_GUTL_COMM_XML            := $(BALONG_TOPDIR)/config/nvim/data/comm/$(CFG_PRODUCT_HISILICON_VERSION)/*.xml
NV_GU_XML_FILES             := $(NV_GU_COMMON_XML) $(NV_GU_DIFF_XML) $(NV_GUTL_COMM_XML)

# NV path define
NV_PRODUCT_BUILD_FOLDER     := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build
NV_PRODUCT_HEADFILE_FOLDER  := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/HeadFile
NV_PRODUCT_XMLDBFILE_FOLDER := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/XmlDbFiles
NV_PRODUCT_NVCONFIG_FOLDER  := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/NV_CONFIG
NV_PRODUCT_NVLIB_FOLDER     := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/lib

# .h or .txt
NV_GU_ALL_HEADFILES     := $(BALONG_TOPDIR)/include/nv/gu $(BALONG_TOPDIR)/include/nv/comm

NV_COMM_HEADFILES           := $(BALONG_TOPDIR)/include/nv/comm/comm_nv_def.h
NV_GU_MAIN_HEADFILE         := $(BALONG_TOPDIR)/include/nv/gu/tool/nv_gu_struct_def.txt
NV_GU_DATATYPE_HEADFILE     := $(BALONG_TOPDIR)/include/nv/gu/tool/nv_gu_struct_datatype_def.txt
NV_MODEM_MAIN_HEADFILE      := $(BALONG_TOPDIR)/include/nv/gu/tool/nv_modem_struct_def.txt

NV_GU_BNT_HEADFILES         := $(NV_GU_MAIN_HEADFILE) $(NV_GU_DATATYPE_HEADFILE) $(NV_MODEM_MAIN_HEADFILE) $(NV_COMM_HEADFILES)

#gu dst folder
NV_GU_DST_HEADFILES       := $(NV_PRODUCT_HEADFILE_FOLDER)/nv 

.PHONY: all do_gu_nv_build
all: do_gu_nv_build

do_gu_nv_build: 
	$(Q)mkdir -p $(NV_GU_DST_HEADFILES)
	$(Q)cp -rf $(NV_GU_ALL_HEADFILES) $(NV_GU_DST_HEADFILES)
	$(Q)cp -f $(NV_GU_BNT_HEADFILES) $(NV_PRODUCT_HEADFILE_FOLDER)
	$(Q)cp -f $(NV_GU_XML_FILES) $(NV_PRODUCT_XMLDBFILE_FOLDER)

.PHONY: clean
clean:
	$(Q)-rm -f $(GUNV_FILES)
	$(Q)-rm -f $(GUNV_XML_FILES)	

.PHONY: FORCE
FORCE:;
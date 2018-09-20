# Created by MichaelYao/y00184236/1012-03-30

# base Config
include $(BALONG_TOPDIR)/build/scripts/make_base.mk

# NV product information
#include $(BALONG_TOPDIR)/config/nvim/data/tl/NV/LTE_NV/nv_config.cfg

# NV
DELIVERY_LIB_DIR := $(OBB_PRODUCT_DELIVERY_DIR)/lib
#LNV_HEADFILE_DIR := $(BALONG_TOPDIR)/config/nvim/data/tl/NV/LTE_NV/HeadFile
#LNV_DIR          := $(BALONG_TOPDIR)/config/nvim/data/tl/NV/LTE_NV/NV_CONFIG

# NV path define
NV_PRODUCT_HEADFILE_FOLDER  := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/HeadFile
NV_PRODUCT_XMLDBFILE_FOLDER := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/XmlDbFiles

NV_LPS_XML     := $(BALONG_TOPDIR)/config/nvim/data/tl/$(CFG_PRODUCT_HISILICON_VERSION)/lps/*.xml
NV_LPHY_XML    := $(BALONG_TOPDIR)/config/nvim/data/tl/$(CFG_PRODUCT_HISILICON_VERSION)/phy/*.xml
NV_LMSP_XML    := $(BALONG_TOPDIR)/config/nvim/data/tl/$(CFG_PRODUCT_HISILICON_VERSION)/oam/*.xml
NV_LEQUIP_XML  := $(BALONG_TOPDIR)/config/nvim/data/tl/$(CFG_PRODUCT_HISILICON_VERSION)/equip/*.xml
NV_LDRV_XML	   := $(BALONG_TOPDIR)/config/nvim/data/tl/$(CFG_PRODUCT_HISILICON_VERSION)/drv/*.xml

NV_DRV_HEADFILES     := $(BALONG_TOPDIR)/include/nv/tl/drv/*.h
NV_LPS_HEADFILES     := $(BALONG_TOPDIR)/include/nv/tl/lps/*.h
NV_LPHY_HEADFILES    := $(BALONG_TOPDIR)/include/nv/tl/phy/*.h
NV_LMSP_HEADFILES    := $(BALONG_TOPDIR)/include/nv/tl/oam/*.h

NV_LEQUIP_HEADFILES  := $(BALONG_TOPDIR)/include/nv/tl/tool/FactoryNvInterface.h
NV_LPS_APPNAS_HEADFILE := $(BALONG_TOPDIR)/include/ps/tlps/AppNasComm.h
NV_LPS_COMMON_HEADFILE := $(BALONG_TOPDIR)/include/ps/tlps/LPSCommon.h
NV_LPS_TYPEDEF_HEADFILE := $(BALONG_TOPDIR)/include/ps/tlps/PsTypeDef.h
NV_LPS_RRC_HEADFILE    := $(BALONG_TOPDIR)/include/ps/tlps/LRrcLPhyInterface.h

NV_TOOL_HEADFILES    := $(BALONG_TOPDIR)/include/nv/tl/tool/*.txt

NV_XML_FILE     := $(NV_LEQUIP_XML) $(NV_LPHY_XML) $(NV_LPS_XML) $(NV_LMSP_XML) $(NV_LDRV_XML)
NV_HEADFILES    := $(NV_DRV_HEADFILES) $(NV_LPHY_HEADFILES) $(NV_LPS_HEADFILES) $(NV_LMSP_HEADFILES) $(NV_LPS_APPNAS_HEADFILE) $(NV_LPS_COMMON_HEADFILE) $(NV_LPS_TYPEDEF_HEADFILE) $(NV_LPS_RRC_HEADFILE) $(NV_LEQUIP_HEADFILES)


.PHONY: all do_tl_nv_build 
all: do_tl_nv_build

do_tl_nv_build: 
	$(Q)cp -f $(OBB_PRODUCT_CONFIG_DIR)/product_config.h $(NV_PRODUCT_HEADFILE_FOLDER)
	$(Q)cp -f $(NV_HEADFILES) $(NV_PRODUCT_HEADFILE_FOLDER)/tl
	$(Q)cp -f $(NV_TOOL_HEADFILES) $(NV_PRODUCT_HEADFILE_FOLDER)
	$(Q)cp -f $(NV_XML_FILE) $(NV_PRODUCT_XMLDBFILE_FOLDER)


.PHONY: clean
clean:
	$(Q)rm -rf $(NV_PRODUCT_BUILD_FOLDER)

.PHONY: FORCE
FORCE:;
# Created by Jeffery.zhai/199916 2012.03.21

# What are targets ?
$(warning The targets are $(MAKECMDGOALS).)

# guess the Top dir
BALONG_TOPDIR ?= $(patsubst %/config/nvim/build,%,$(CURDIR))

# include the define at the top
include $(BALONG_TOPDIR)/build/scripts/make_base.mk

# Make sure that there are no spaces in the absolute path; the build system can't deal with them.
ifneq ($(words $(BALONG_TOPDIR)),1)
$(warning ************************************************************)
$(warning You are building in a directory whose absolute path contains a space character)
$(warning "$(BALONG_TOPDIR)")
$(warning Please move your source tree to a path that does not contain any spaces.)
$(warning ************************************************************)
$(error Directory names containing spaces not supported)
endif

# NV path define
NV_PRODUCT_BUILD_FOLDER     := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build
NV_PRODUCT_HEADFILE_FOLDER  := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/HeadFile
NV_PRODUCT_XMLDBFILE_FOLDER := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/XmlDbFiles
NV_PRODUCT_NVCONFIG_FOLDER  := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/NV_CONFIG
NV_PRODUCT_NVLIB_FOLDER     := $(OBB_PRODUCT_DELIVERY_DIR)/obj/Nv_Build/lib
NV_PRODUCT_TL_HEADFILE_FOLDER  := $(NV_PRODUCT_HEADFILE_FOLDER)/tl
NV_PRODUCT_CUST_HEADFILE_FOLDER  := $(NV_PRODUCT_HEADFILE_FOLDER)/product

# WINE
WINE :=
ifeq ($(OBB_OS_TYPE),Linux)
WINE := wine
endif


DELIVERY_LIB_DIR := $(OBB_PRODUCT_DELIVERY_DIR)/lib

GU_NV_SCRIPT_DIR := $(BALONG_TOPDIR)/config/nvim/build/gu
LT_NV_SCRIPT_DIR := $(BALONG_TOPDIR)/config/nvim/build/tl
PROD_NV_SCRIPT_DIR := $(BALONG_TOPDIR)/config/nvim/build/product

BNT_TOOL         :=$(BALONG_TOPDIR)/build/tools/utility/nvtools/bnt/hat_bnt.exe
BNT_TOOL_OPTOIN  := "/nvversion $(CFG_PRODUCT_VERSION_STR)" \
	"/ctrlversion $(CFG_PRODUCT_CONTROL_VERSION)" \
	"/nvfolder $(NV_PRODUCT_XMLDBFILE_FOLDER)" \
	"/headerpath $(NV_PRODUCT_HEADFILE_FOLDER)" \
	"/outpath $(NV_PRODUCT_NVLIB_FOLDER)" \
	"/pdtver "$(CFG_LNV_PRO_NAME)"" \
	"$(CFG_PRODUCT_NV_SPLITFILE)" \
	"$(CFG_PRODUCT_NV_FILE_MAX_SIZE)" \
	"$(CFG_PRODUCT_NV_BIN_MODEM_NUNBER)"\
	"$(CFG_PRODUCT_NV_XNV_MODEM_NUNBER)"\
	"$(CFG_PRODUCT_NV_XNV_SUPPORT_PRODUCAT_CAT)"
ifdef NV_BURN
BNT_TOOL_OPTOIN += "/burnver 1"
endif

#BEGIN DTS2016120501831  s00184745 Added 2016-12-05
ifeq ($(strip $(MBB_COMMON)),FEATURE_ON)
PRODUCT_CUST_XML_PATH           := $(BALONG_TOPDIR)/modem/mbb_custom
PRODUCT_CUST_XML_SH             := $(BALONG_TOPDIR)/modem/mbb_custom/build_cust_nv.sh
PRODUCT_CFG_NAME                := $(shell echo $(CFG_PRODUCT_NAME) | sed 's/"//g')
PRODUCT_CONFIGURE_MK            := $(BALONG_TOPDIR)/config/product/$(PRODUCT_CFG_NAME)/config/balong_product_config.mk
PRODUCT_DEST_CUST_XML_NAME      := $(NV_PRODUCT_NVLIB_FOLDER)/cust.xml
endif
#END DTS2016120501831  s00184745 Added 2016-12-05

.PHONY:all nv_build gu lt product
all: nv_build prebuild
prebuild:
	$(Q)mkdir -p $(NV_PRODUCT_BUILD_FOLDER)
	$(Q)mkdir -p $(NV_PRODUCT_HEADFILE_FOLDER)
	$(Q)mkdir -p $(NV_PRODUCT_XMLDBFILE_FOLDER)
	$(Q)mkdir -p $(NV_PRODUCT_NVCONFIG_FOLDER)
	$(Q)mkdir -p $(NV_PRODUCT_NVLIB_FOLDER)
	$(Q)mkdir -p $(NV_PRODUCT_TL_HEADFILE_FOLDER)
	$(Q)mkdir -p $(NV_PRODUCT_CUST_HEADFILE_FOLDER)
nv_build: gu lt product
#BEGIN DTS2016120501831  s00184745 Added 2016-12-05
ifeq ($(strip $(MBB_COMMON)),FEATURE_ON)
ifeq ($(strip $(LMT_COMPILE)),true)
ifeq ($(PRODUCT_CUST_XML_SH), $(wildcard $(PRODUCT_CUST_XML_SH)))
	$(Q)echo "welcome to mbb cust xml copy"
	$(Q)cd $(PRODUCT_CUST_XML_PATH) && ./build_cust_nv.sh $(PRODUCT_CONFIGURE_MK) $(PRODUCT_DEST_CUST_XML_NAME)
endif
endif
endif
#END DTS2016120501831  s00184745 Added 2016-12-05
	$(Q)wine $(BNT_TOOL) $(BNT_TOOL_OPTOIN)
	$(Q)cp -f $(NV_PRODUCT_NVLIB_FOLDER)/*.* $(OBB_PRODUCT_DELIVERY_DIR)/lib

gu : prebuild 
	echo "copy gu nv files"
	$(MAKE)  -C $(GU_NV_SCRIPT_DIR) -f makefile_nv.mk

lt : prebuild
	echo "copy tl nv files"
	$(MAKE)  -C $(LT_NV_SCRIPT_DIR) -f makefile_nv.mk

product : prebuild
	echo "copy product nv files"
	$(MAKE)  -C $(PROD_NV_SCRIPT_DIR) -f makefile_nv.mk
# clean
.PHONY: clean
clean:
	$(Q)rm -rf $(NV_PRODUCT_BUILD_FOLDER)
#	$(MAKE)  -C $(GU_NV_SCRIPT_DIR) -f makefile_nv.mk $@
#ifneq ($(CFG_RAT_MODE),RAT_GU)
#	$(MAKE)  -C $(LT_NV_SCRIPT_DIR) -f makefile_nv.mk $@
#endif
#	$(Q)echo "COMM_CODE clean complete."

include $(BALONG_TOPDIR)/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

EXTRA_CFLAGS += -fno-pic 
LINUXROOT =$(BALONG_TOPDIR)/modem/system/android/android_4.2_r1/out/target/product/balongv7r2/obj/KERNEL_OBJ/
obj-m += pss.o
pss-objs := balong_pss.o balong_pss_packet.o

.PHONY:modules
modules:
	$(MAKE) -C $(LINUXROOT) M=$(shell pwd) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
.PHONY:clean
clean:
	rm -rf   *.o *.ko .*cmd *.mod.c *.order .tmp_versions Module.symvers
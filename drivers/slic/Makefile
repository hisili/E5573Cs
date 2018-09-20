ifdef NV_BURN
obj-y += si32178.o
else
obj-m += si32178.o
endif 

si32178-objs := slic_ctl.o \
				proslic.o \
				BSP_SPI.o \
				drv_sio.o \
				drv_dma.o \
				pcm_voip_dma.o \
				pcm_voip_sio.o \
				pcm_voip_ctrl.o \
				med_ctrl_pcm.o \
				si_voice.o \
				si3217x_BKBT_constants.o \
				si3217x_intf.o \
				si3217x_patch_B_BB_2010DEC16.o \
				si3217x_patch_B_FB_2009APR09.o \
				vdaa.o \
				vdaa_constants.o

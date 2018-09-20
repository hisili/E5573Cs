/* Copyright (c) 1988-2012 by Huawei Technologies Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <asm/irq.h>
#include <linux/irq.h>
#include <mach/irqs.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/compat.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include "proslic.h"
#include "si3217x_constants.h"
#include "si3217x_registers.h"
#include "slic_ctl.h"
#include "BSP_DRV_SPI.h"
#include "slic_ctl.h"
#include "si_voice_datatypes.h"
#include "si3217x.h"

#include <linux/interrupt.h>      /* mdelay */
#include <linux/poll.h>         /* polling */
#include <linux/spinlock.h>  

#include <linux/semaphore.h>
#include <asm/fcntl.h>

#include <linux/syscalls.h>
#include <linux/file.h>

#include <linux/kernel.h>	/* dbg_print() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */

#include "pcm_voip_ctrl.h"
#include "DrvInterface.h"

#include "data_types.h"
#include "slic_log.h"
#include "drv_sio.h"

#include <mach/hardware.h> 
#define MNUM(inode)             (iminor(inode) & 0xf)
#define uint8 uInt8
#define uint16 uInt16
#define uint32 uInt32

static int __devinit  si32178_probe(struct platform_device *pdev);
static int __devexit si32178_remove(struct platform_device *pdev);

#ifdef CONFIG_SLIC_TEST
static void create_si3217x_proc_file(void);
#endif
/* Declaration of PCM channel state */
typedef struct slic_pcm slic_pcm_t;
slic_pcm_t slic_pcmdev;

/* SLIC device object - declared in slic_slic.c */

/* Function prototypes of PCM controller local functions */
static void pcm_init_state(int channel, struct slic_pcm *dev);

/* Exported functions to handle PCM functionality */
int slic_pcm_start(int channel);
int slic_pcm_read(int channel, char *buf, int size);
int slic_pcm_write(int channel, char *buf, int size);
void slic_read_registers(void);
void slic_loopback(void);

//int slic_pcm_spkout(char *dma_buf,int limit);
//int slic_pcm_micin(char *dma_buf,int size);
#define SLICCTL_NUMBER_OF_DEVICES	1
#define SLICCTL_CHAN_PER_DEVICE		2	/* SLIC board has one device with 2 channels */
#define SLICCTL_NUMBER_OF_CHAN		(SLICCTL_NUMBER_OF_DEVICES * SLICCTL_CHAN_PER_DEVICE)
#define PROSLIC_TG_TIMER_ENABLED    1

/* Variables related to event handling */
#define SLICCTL_ON_HOOK    (65)
#define SLICCTL_OFF_HOOK   (64)
#define SLICCTL_FLASH_HOOK (16)
#define SLICCTL_FLASH_HOOK_TIMEOUT  (250)
#define SLICCTL_FIRST_RING_COMPLETE (17)
#define SLICCTL_INVALID_EVENT (-1)

static int reset_count = 0;

/* function frame size larger than 1024, so make them as global vars */
static char readbuf[SLIC_MAX_BUFSIZE];
static char writebuf[SLIC_MAX_BUFSIZE];


/* PROSLIC instance - global variables */
int 				    spiGciObj; 	                            	/* spi interface object */
int 				    timerObj;                                   /* timer object */
controlInterfaceType    *ProHWIntf;                                 /* proslic hardware interface object */
ProslicDeviceType 		*ProSLICDevices[SLICCTL_NUMBER_OF_DEVICES]; /* proslic device object */
proslicChanType_ptr		ProslicChans[SLICCTL_NUMBER_OF_CHAN];       /* used for initialization only */
/* END - PROSLIC instance global variables */

/* Local Functions */
/* Wrappers */
int   ctrl_ResetWrapper (void *hSpiGci, int status);
uInt8    ctrl_ReadRegisterWrapper (void *hSpiGci, uInt8 channel, uInt8 addr);
int   ctrl_WriteRegisterWrapper (void *hSpiGci, uInt8 channel,  uInt8 addr, uInt8 data);
uInt32 ctrl_ReadRAMWrapper (void *hSpiGci, uInt8 channel, uInt16 addr);
int   ctrl_WriteRAMWrapper(void *hSpiGci, uInt8 channel, uInt16 addr, ramData data);

struct si32178_data   *slic_dev = NULL;
struct slic_ioc_data  *slic_ioc_data_pt = NULL;

/* other globals. */
int 				g_slicLock 			= 0;
Country_Type 		g_Country_Type 		= COUNTRY_CN;
int                 g_Equip_Test 		= 0;
slic_tone_enum_type g_Equip_Test_Tone 	= SLIC_FAIL_TONE;
int                 g_Equip_Test_Count 	= 1;
int                 g_Equip_Loopback    = 0;

#define HISI_CFGPCM_TXSLOT_CH0         32        /* Use timeslot 32 to 47 (16 bits) */
#define HISI_CFGPCM_RXSLOT_CH0         400       /* Use timeslot 400 to 415 (16 bits) */


uInt8 slic_fsk_flag = FALSE;	/* 需要来电显示时,设置为TRUE */
uInt8 g_slic_rkey   = RKEY_STATE_ONHOOK_VALID;	/*HOOKFLASH键标识,使用此初始值防止第一次摘机上报不了*/

slic_cmd_mark_enum_type slic_cmd_mark = SLIC_CMD_UNLOCK;/* ProSLIC uC interface lock mark */

int slic_fsk_seize_index 	= 0; /* FSK发送前导帧的索引 */
int slic_fsk_body_index 	= 0; /* FSK发送号码的索引 */
int callerid_body_lgth_conv = 0; /* FSK来显转换后的长度 */
int callerid_body_lgth 		= 0; /* PSTN中号码总长度 */

static byte slic_fsk_seize_num_reg[70]	=	{0};
static byte slic_num_reg[47]			=	{0};
static byte slic_num_reg_fsk_conv[60]	=	{0};
static byte dtmf_callerid_index 		= 	 0;
static byte dtmf_callerid_length 		= 	 0;
/* call waiting tone cycle counter, optus: 9 cycles */
static byte slic_cw_cycle_num = 0;

extern int HiInitIrq(void);
extern void  SlicSioInit(void);

uInt32 callerid_showtime = 0; /*从第一次振铃后到来电显示的时间间隔*/
#define CALLERID_SHOWTIME_FSK_RPAS  910 /*used for FSK RPAS mode parameter*/

byte slic_int_enable1_default_value = 0x00;

/* Tip tone type */
slic_tip_tone_enum_type slic_tip_tone_type = SLIC_NULL_TIP_TONE;

/* Four ring part indicator for ring part set*/
slic_four_ring_part_type slic_four_ring_part = SLIC_FIRST_PART_RING;

OSCILLATOR1_CREATE_SOUND_TYPE slic_oscillator_soundtype = TIP_TONE;

/* Four ring cadence for foreign country */
four_or_two_ring_type four_ring_cadence = TWO_RING;

slic_tone_enum_type slic_sound_stop_tone = SLIC_RING;

const unsigned short slic_keycodes[KEY_CODE_NUM] = {
    SLIC_0_K, SLIC_1_K, SLIC_2_K, SLIC_3_K,
    SLIC_4_K, SLIC_5_K, SLIC_6_K, SLIC_7_K,
    SLIC_8_K, SLIC_9_K,
    SLIC_STAR_K,
    SLIC_POUND_K,
    SLIC_HANGON_K,
    SLIC_HANGUP_K,
    SLIC_RKEY_K
};

slic_parm_type slic_parm[COUNTRY_COUNT] =
{
    /* BE */
    {
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },
    
    /* BR */
    {
        /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
        {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
        /* dial tone : 450Hz, 245mVrms on forever */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
        /* busy tone : 450Hz, 245mVrms, 250ms on, 250ms off */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x07d0, 0x07d0},
        /* alarm tone: 950Hz, 1.11Vrms  on forever */
        {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
        /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
        {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
        /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
        /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
        /* caller ID mode */
        SLIC_CALLERID_DTMF,
        /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        320,
        /* rkey lower limit */
        220,
        /* on hook lower limit */
        320
    },
    
    /* CL */
    {
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },
    
    /* CN */
    {
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
         DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* DK */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* EE */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* FI */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* FR */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* DE */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* HU */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750    
    },

    /* IT */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* JP */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* NL */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* US */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
         DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* ES */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* SE */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* CH */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* SA */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* GB */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* SG */
	{
	    /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 450Hz, 245mVrms, 350ms on, 350ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0af0, 0x0af0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },

    /* RU */
    {
	    /* ring : 22V offset, 425Hz, 42Vrms, 0.8s on, 3.2s off */
	    {0x0000000, 0x78F0000, 0x001ED4D2, 0x1900, 0x0000, 0x0000, 0x6400},
	    /* dial tone : 450Hz, 245mVrms on forever */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* busy tone : 425Hz, 245mVrms, 400ms on, 400ms off */
	    {0x78F0000, 0x284000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 950Hz/1400Hz(1800Hz), 245mVrms, 330ms on, 1000ms off */
	    {0x5E00000, 0x284000, 0x3A20000, 0x284000, 0x0A68, 0x1f40},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750
    },
    
    /* PL */
    {
        /* ring : 22V offset, 25Hz, 42Vrms, 1s on, 4s off */
        {0x0000000, 0x7E6C000, 0x001ED4D2, 0x1f40, 0x0000, 0x0000, 0x7d00},
        /* dial tone : 450Hz, 245mVrms on forever */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x0000, 0x0000},
        /* busy tone : 450Hz, 245mVrms, 250ms on, 250ms off */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x07d0, 0x07d0},
        /* alarm tone: 950Hz, 1.11Vrms  on forever */
        {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
        /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
        {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
        /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
        /* waiting tone: 450Hz, 245mVrms, xms on, xms off */
        {0x7816000, 0x284000, 0x0000, 0x0000, 0x0640, 0x0640},
        /* caller ID mode */
        SLIC_CALLERID_DTMF,
        /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_600_0_0_30_0,
        /* dc feed type */
        DCFEED_48V_20MA,
        /* ring preset type */
        RING_F20_45VRMS_0VDC_LPR,
        /* rkey upper limit */
        750,
        /* rkey lower limit */
        0,
        /* on hook lower limit */
        750     
    },
    /* AU */
    {
	    /* ring : 22V offset, 25Hz, 42Vrms, 0.4s on, 0.2s off, 0.4s on, 2s off*/
	    {0x0000000, 0x7E6C000, 0x001ED4D2, 0xC80, 0x640, 0xC80, 0x3E80},
	    /* dial tone : 425Hz+400Hz -15.00 dBm on forever */
	    {0x078F0000L, 0x00156000L, 0x079C0000L, 0x00140000L, 0x0000, 0x0000},
	    /* busy tone : 425Hz, -15.00 dBm, 380ms on, 380ms off */
	    {0x078F0000L, 0x00156000L, 0x0000, 0x0000, 0x0BE0, 0x0BE0},
	    /* alarm tone: 950Hz, 1.11Vrms  on forever */
	    {0x5DFE000, 0x190C000, 0x0000, 0x0000, 0x0000, 0x0000},
	    /* tip tone  : 950Hz, 245mVrms, 400ms on, 400ms off */
	    {0x5DFE000, 0x586000, 0x0000, 0x0000, 0x0c80, 0x0c80},
	    /* fault tone: 450Hz, 245mVrms, 700ms on, 700ms off */
	    {0x7816000, 0x284000, 0x0000, 0x0000, 0x15e0, 0x15e0},
	    /* waiting tone: 425Hz, -15.00 dBm, 200ms on, 200ms off */
	    {0x078F0000L, 0x00156000L, 0x0000, 0x0000, 0x0640, 0x0640},
	    /* caller ID mode */
	    SLIC_CALLERID_DTMF,
	    /* teletax mode */
	    SLIC_TELETAX_ALL,
	    /* zsynth type */
        ZSYN_220_820_115_30_0,
        /* dc feed type */
        DCFEED_48V_25MA,
        /* ring preset type */
        RING_F25_51VRMS_0VDC_LPR,
        /* rkey upper limit */
        150,
        /* rkey lower limit */
        70,
        /* on hook lower limit */
        750
    }
};

slic_customize_ex_parm_type slic_customize_ex_parm[COUNTRY_COUNT] =
{
    /* BE */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* BR */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* CL */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* CN */
    {      
        0x3d,   /* The first signal sent in DTMF caller ID is D */
        {0, 750},
        {
            {
                {0x578000, 0x82C000}, {0x3F7000, 0x742000}, {0x3F7000, 0x82C000},
                {0x3F7000, 0x941000}, {0x468000, 0x742000}, {0x468000, 0x82C000},
                {0x468000, 0x941000}, {0x4E9000, 0x742000}, {0x4E9000, 0x82C000},
                {0x4E9000, 0x941000}, {0x3F7000, 0xA8B000}, {0x468000, 0xA8B000},
                {0x4E9000, 0xA8B000}, {0x578000, 0xA8B000}
            }
        }
    },
    
    /* DK */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* EE */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* FI */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* FR */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* DE */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* HU */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* IT */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* JP */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
      
    /* NL */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* US */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* ES */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* SE */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* CH */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* SA */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* GB */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },
    
    /* SG */
    {     
        0x3a,   /* The first signal sent in DTMF caller ID is A */
        {220, 320},
        {
            {
                {0x004de000, 0x00745e14}, {0x00387599, 0x006759eb}, {0x00387599, 0x00745e14},
                {0x00387599, 0x0083c68f}, {0x003ebeb8, 0x006759eb}, {0x003ebeb8, 0x00745e14},
                {0x003ebeb8, 0x0083c68f}, {0x0045ebae, 0x006759eb}, {0x0045ebae, 0x00745e14},
                {0x0045ebae, 0x0045ebae}, {0x00387599, 0x009621c2}, {0x003ebeb8, 0x009621c2},
                {0x0045ebae, 0x009621c2}, {0x004de000, 0x009621c2}
            }
        }
    },

    /* RU */
    {      
        0x3d,   /* The first signal sent in DTMF caller ID is D */
        {0, 750},
        {
            {
                {0x578000, 0x82C000}, {0x3F7000, 0x742000}, {0x3F7000, 0x82C000},
                {0x3F7000, 0x941000}, {0x468000, 0x742000}, {0x468000, 0x82C000},
                {0x468000, 0x941000}, {0x4E9000, 0x742000}, {0x4E9000, 0x82C000},
                {0x4E9000, 0x941000}, {0x3F7000, 0xA8B000}, {0x468000, 0xA8B000},
                {0x4E9000, 0xA8B000}, {0x578000, 0xA8B000}
            }
        }
    },
    /* PL */
    {        
        0x3d,   /* The first signal sent in DTMF caller ID is D */
        {0, 750},
        {
            {
                {0x578000, 0x82C000}, {0x3F7000, 0x742000}, {0x3F7000, 0x82C000},
                {0x3F7000, 0x941000}, {0x468000, 0x742000}, {0x468000, 0x82C000},
                {0x468000, 0x941000}, {0x4E9000, 0x742000}, {0x4E9000, 0x82C000},
                {0x4E9000, 0x941000}, {0x3F7000, 0xA8B000}, {0x468000, 0xA8B000},
                {0x4E9000, 0xA8B000}, {0x578000, 0xA8B000}
            }
        }
    },
    /* AU*/
    {      
        0x3d,   /* The first signal sent in DTMF caller ID is D */
        {0, 750},
        {
            {
                {0x578000, 0x82C000}, {0x3F7000, 0x742000}, {0x3F7000, 0x82C000},
                {0x3F7000, 0x941000}, {0x468000, 0x742000}, {0x468000, 0x82C000},
                {0x468000, 0x941000}, {0x4E9000, 0x742000}, {0x4E9000, 0x82C000},
                {0x4E9000, 0x941000}, {0x3F7000, 0xA8B000}, {0x468000, 0xA8B000},
                {0x4E9000, 0xA8B000}, {0x578000, 0xA8B000}
            }
        }
    },
};

extern Si3217x_Ring_Cfg Si3217x_Ring_Presets[];

void slic_sound_stop ( slic_tone_enum_type tone );
int slic_direct_write (uInt8 channel,  uInt8 addr, uInt8 data);
extern int Start_Equ_Slic_Test(void);
void slic_sound_start (  slic_tone_enum_type tone );
uInt8 slic_direct_read (uInt8 channel, uInt8 addr);
extern signed int pca9555_i2c_write_word(uInt8 cmd, uInt16 value);
extern signed int pca9555_i2c_set_bit(uInt16 value, uInt8 bit);
static void slic_dtmf_callerid_show_stop(void);
void slic_callerid ( void );
static void slic_sound_start_delay ( void );

#ifdef FEATURE_HUAWEI_DTMF_FLUCTUATE_ELIMINATE
unsigned int g_dtmf_code = 0;
unsigned int g_dtmf_reg = 0;
#endif

/*------------------------------------------------------------------------
函数原型:slic_pcm_unmute_timer_cb
描述: CS语音剔除带内DTMF音，DTMF按键上报后需要MUTE掉PCM通道，而后起定时器恢复
输入: NA
输出:NA
返回值: NA
------------------------------------------------------------------------*/
#ifdef FEATURE_HUAWEI_CS_INBAND_DTMF_ELIMINATE
void slic_pcm_unmute_timer_cb(void)
{
    char dtmf_digit_valid = 0;
    dtmf_digit_valid = slic_direct_read(0, TONDTMF);
    dtmf_digit_valid = dtmf_digit_valid & 0x20;
    /* 按键已抬起 */
    if(0x0 == dtmf_digit_valid)
    {
        printk("unmute pcm channel.\n");
        ProSLIC_PCMStart(ProslicChans[0]);
        del_timer(&(slic_dev->slic_pcm_unmute_timer));
    }
    else
    {
        mod_timer(&(slic_dev->slic_pcm_unmute_timer), msecs_to_jiffies(10) + jiffies);
    }
}
#endif

#ifdef FEATURE_HUAWEI_DTMF_FLUCTUATE_ELIMINATE
/*------------------------------------------------------------------------
函数原型:slic_dtmf_valid_timer_cb
描述: DTMF去抖动超时定时器回调处理函数
输入: NA
输出:NA
返回值: NA
------------------------------------------------------------------------*/
void slic_dtmf_valid_timer_cb(void)
{
    char dtmf_reg = 0;
    dtmf_reg = slic_direct_read(0, TONDTMF);
    dtmf_reg = dtmf_reg & 0x3F;
        
    if(g_dtmf_reg == dtmf_reg)
    {


        
        input_report_key(slic_dev->input_dev, g_dtmf_code, 1);
        input_sync(slic_dev->input_dev);
        input_report_key(slic_dev->input_dev, g_dtmf_code, 0);
        input_sync(slic_dev->input_dev);
    }
    else
    {
        pr_info("slic_dtmf_valid_timer_cb, drop invalid DTMF!\n");
    }

    del_timer(&(slic_dev->slic_dtmf_valid_timer));
}
#endif

/*------------------------------------------------------------------------
函数原型:slic_dtmfkey_report
描述: 按键上报函数
输入:
输出:
返回值:
------------------------------------------------------------------------*/
static void slic_dtmfkey_report(unsigned int code)
{
#ifdef FEATURE_HUAWEI_CS_INBAND_DTMF_ELIMINATE
    printk("mute pcm channel.\n");
    ProSLIC_PCMStop(ProslicChans[0]);
    slic_dev->slic_pcm_unmute_timer.function = (void (*)(unsigned long))slic_pcm_unmute_timer_cb;
    mod_timer(&(slic_dev->slic_pcm_unmute_timer), msecs_to_jiffies(10) + jiffies);
#endif

#ifdef FEATURE_HUAWEI_DTMF_FLUCTUATE_ELIMINATE
    if(SLIC_POUND_K <= code && code <= SLIC_D_K)
    {
        slic_dev->slic_dtmf_valid_timer.function = (void (*)(unsigned long))slic_dtmf_valid_timer_cb;
        mod_timer(&(slic_dev->slic_dtmf_valid_timer), msecs_to_jiffies(43) + jiffies);

        g_dtmf_reg = slic_direct_read(0, TONDTMF);
        g_dtmf_reg &= 0x3F;
        g_dtmf_code = code;
        
        return;
    }
#endif

    input_report_key(slic_dev->input_dev, code, 1);
    input_sync(slic_dev->input_dev);
    input_report_key(slic_dev->input_dev, code, 0);
    input_sync(slic_dev->input_dev);
}

/*------------------------------------------------------------------------
函数原型:slic_rkey_callback
描述: rkey定时器回调函数，到750ms,则上报挂机键值
输入: NA
输出:NA
返回值: NA
------------------------------------------------------------------------*/
void slic_rkey_cb(void)
{

    if(RKEY_STATE_INVALID == g_slic_rkey)
    {
        g_slic_rkey = RKEY_STATE_VALID;

        /* 定时器接力 */
        slic_dev->slic_rkey_delay_timer.function = (void (*)(unsigned long))slic_rkey_cb;
        mod_timer(&(slic_dev->slic_rkey_delay_timer),
            msecs_to_jiffies(slic_parm[g_Country_Type].slic_rkey_upper_limit - slic_parm[g_Country_Type].slic_rkey_lower_limit) + jiffies);
    }
    else if(RKEY_STATE_VALID == g_slic_rkey)
    {
        g_slic_rkey = RKEY_STATE_OVERTIME;
        if(slic_parm[g_Country_Type].slic_rkey_upper_limit >= slic_parm[g_Country_Type].slic_onhook_lower_limit)
        {
            del_timer(&(slic_dev->slic_rkey_delay_timer));
            if (1 == g_Equip_Test)
            {
                slic_sound_stop(g_Equip_Test_Tone);
                g_Equip_Test = 0;
                g_Equip_Test_Count = 1;
            }
            else
            {
                slic_dtmfkey_report(SLIC_HANGUP_K);
                printk( "report- on hook.\n");
            }
        }
        else
        {
            /* 定时器再接力 */
            slic_dev->slic_rkey_delay_timer.function = (void (*)(unsigned long))slic_rkey_cb;
            mod_timer(&(slic_dev->slic_rkey_delay_timer),
                msecs_to_jiffies(slic_parm[g_Country_Type].slic_onhook_lower_limit - slic_parm[g_Country_Type].slic_rkey_upper_limit) + jiffies);
        }
    }
    else if(RKEY_STATE_OVERTIME == g_slic_rkey)
    {
        g_slic_rkey = RKEY_STATE_ONHOOK_VALID;
        del_timer(&(slic_dev->slic_rkey_delay_timer));
        if (1 == g_Equip_Test)
        {
            slic_sound_stop(g_Equip_Test_Tone);
            g_Equip_Test = 0;
            g_Equip_Test_Count = 1;
        }
        else
        {
            slic_dtmfkey_report(SLIC_HANGUP_K);
            printk( "report- on hook.\n");
        }
    }    
}

void Equip_SilcTestRel(char *pTestRel)
{
        struct file *filp; 
        struct inode *inode; 
        mm_segment_t old_fs; 
        char *buf = pTestRel;
        char * filename = "/data/equipdata/equipsilctestrel";
        printk("EquipSilcTestRel: start....\n");
        filp=filp_open(filename,O_RDWR|O_CREAT,0); 
        if (IS_ERR(filp))
        {
            printk("<error>Equip_SilcTestRel: Open file failed!\n");
            return ;
        }
        inode=filp->f_dentry->d_inode;  
        old_fs = get_fs();
        set_fs(KERNEL_DS);
        vfs_write(filp, buf, 19, &(filp->f_pos));
        filp_close(filp,NULL);   
        set_fs(old_fs);
        printk("Equip_SilcTestRel: end....\n");
}

/*------------------------------------------------------------------------
函数原型:slic_rkey_callback
描述: 装备测试函数.
输入: NA
输出:NA
返回值: NA
------------------------------------------------------------------------*/
void equip_slic_test(void)
{
    if (0 >= g_Equip_Test_Count)
    {
        return;
    }
    g_Equip_Test_Count--;
	
    /* 开启slic一层环回 */
    slic_direct_write(0, LOOPBACK, 1);  /* LOOPBACK: 43 */

	if (1 == Start_Equ_Slic_Test())
    {
        slic_direct_write(0, LOOPBACK,  0);
        g_Equip_Test_Tone = SLIC_BUSY_TONE;
        slic_sound_start(SLIC_BUSY_TONE);
        Equip_SilcTestRel("EquipSlicTestPass!");
    }
    else
    {
        slic_direct_write(0, LOOPBACK, 0);
        g_Equip_Test_Tone = SLIC_TIP_TONE;
        slic_sound_start(SLIC_TIP_TONE);
        Equip_SilcTestRel("EquipSlicTestFail!");
    }
}


/*------------------------------------------------------------------------
函数原型:static void slic_dtmfkey ( byte dtmf_int )
描述: SLIC DTMF处理上报键值函数

输入:
byte dtmf_int:
bit[7]      HOOK has changed
bit[6]      HOOK
bit[5]      Reserved
bit[4]      DTMF-OK
bit[3:0]    DTMF-KEY[3:0]
bit[3:0]:
0001       '1'
0010       '2'
0011       '3'
0100       '4'
0101       '5'
0110       '6'
0111       '7'
1000       '8'
1001       '9'
1010       '0'
1011       '*'
1100       '#'
输出: DTMF键值
HS_OFF_HOOK_K   off hook
HS_ON_HOOK_K    on hook
HS_POUND_K      '#'
HS_STAR_K       '*'
HS_0_K          '0'
HS_1_K          '1'
HS_2_K          '2'
HS_3_K          '3'
HS_4_K          '4'
HS_5_K          '5'
HS_6_K          '6'
HS_7_K          '7'
HS_8_K          '8'
HS_9_K          '9'
HS_RELEASE_K    release all key

返回值: 无
------------------------------------------------------------------------*/
static void slic_dtmfkey ( byte dtmf_int )
{
    static byte dtmf_last = 0x00;
    if ( (dtmf_int & 0x80) == 0x80 )    /* HOOK changed */
    {
        if (dtmf_last == dtmf_int)
        {
            /*有时候SLIC会错报连续两个摘机的键值，所以在中断上报部分需要
            *对于此问题加以考虑。修改键值上报部分，如果有此情况，不再重
            *新上报。*/
            pr_info( "report- same hang info\n");
            return;
        }

        if ( (dtmf_int & 0x40) == 0x00 ) /* off hook */
        {
            /* 在摘机状态下删除R键定时器 */
            del_timer(&(slic_dev->slic_rkey_delay_timer));
            pr_info( "del_timer slic_rkey_delay_timer %s().\n", __FUNCTION__ );        

            /* 摘机后先判断是不是在报R键状态，如果不是则报摘机键 */
            if(RKEY_STATE_ONHOOK_VALID == g_slic_rkey)
            {
                if (1 == g_Equip_Test)
                {
                    slic_sound_stop(SLIC_RING);
                    slic_sound_start(SLIC_DIAL_TONE);
                }
                else
                {
                    slic_dtmfkey_report(SLIC_HANGON_K);
                    printk( "report- off hook.\n");
                }
            }
            else if(RKEY_STATE_OVERTIME == g_slic_rkey)
            {
                if(slic_parm[g_Country_Type].slic_rkey_upper_limit >= slic_parm[g_Country_Type].slic_onhook_lower_limit)
                {
                    if (1 == g_Equip_Test)
                    {
                        slic_sound_stop(SLIC_RING);
                        slic_sound_start(SLIC_DIAL_TONE);
                    }
                    else
                    {
                        slic_dtmfkey_report(SLIC_HANGON_K);
                        printk( "report- off hook.\n");
                    }
                }
                else
                {
                    printk("rkey and onhook are all invalid, ignore.\n");
                }
            }
            else if(RKEY_STATE_VALID == g_slic_rkey)
            {
                slic_dtmfkey_report(SLIC_RKEY_K);
                printk( "report- rkey.\n");
            }
            else
            {
                printk("rkey < g_rkey_time_min ignore.\n");
            }                                    
        }
        else if ( (dtmf_int & 0x40) == 0x40 )   /* on hook */
        {
            g_slic_rkey = RKEY_STATE_INVALID;     

            /* 挂机后，取HOOKFLASH定时器 */
            if (0 == slic_parm[g_Country_Type].slic_rkey_lower_limit)
            {
                /* 如果RKEY最小时间为0，直接调用 */
                slic_rkey_cb();
            }
            else
            {                        
                slic_dev->slic_rkey_delay_timer.function = (void (*)(unsigned long))slic_rkey_cb;
                mod_timer(&(slic_dev->slic_rkey_delay_timer),
                    msecs_to_jiffies(slic_parm[g_Country_Type].slic_rkey_lower_limit) + jiffies);
            }               
        }
        else
        {
            pr_info( "Hook on off determine error!\n");
        }
        dtmf_last = dtmf_int;
    }
    else if ( (dtmf_int & 0x10) == 0x10 )    /* DTMF key */
    {
        switch ( dtmf_int & 0x0f )
        {
        case 0x1 : /* '1' */
            printk( "report- 1 ");
            slic_dtmfkey_report(SLIC_1_K);
            break;

        case 0x2 : /* '2' */
            printk ( "report- 2. ");
            slic_dtmfkey_report(SLIC_2_K);
            break;

        case 0x3 : /* '3' */
            printk ( "report- 3. ");
            slic_dtmfkey_report(SLIC_3_K);
            break;

        case 0x4 : /* '4' */
            printk ( "report- 4. ");
            slic_dtmfkey_report(SLIC_4_K);
            break;

        case 0x5 : /* '5' */
            printk ( "report- 5. ");
            slic_dtmfkey_report(SLIC_5_K);
            break;

        case 0x6 : /* '6' */
            printk ( "report- 6. ");
            slic_dtmfkey_report(SLIC_6_K);
            break;

        case 0x7 : /* '7' */
            printk ( "report- 7. ");
            slic_dtmfkey_report(SLIC_7_K);
            break;

        case 0x8 : /* '8' */
            printk ( "report- 8. ");
            slic_dtmfkey_report(SLIC_8_K);
            break;

        case 0x9 : /* '9' */
            printk ( "report- 9. ");
            slic_dtmfkey_report(SLIC_9_K);
            break;

        case 0x0a : /* '0' */
            printk ( "report- 0. ");
            slic_dtmfkey_report(SLIC_0_K);
            break;

        case 0x0b : /* '*' */
            printk ( "report- *. ");
            slic_dtmfkey_report(SLIC_STAR_K);
            break;

        case 0x0c : /* '#' */
            printk ( "report- #. ");
            
            if (1 == g_Equip_Test)
            {
                slic_sound_stop(SLIC_DIAL_TONE);
                //equip_slic_test();
                g_Equip_Loopback = 1;
                //slic_dtmfkey_report(SLIC_POUND_K);
                //syswatch_nl_send(30, 0, 0);
                /*
                init_timer(&g_stEquipTestTimer);
                g_stEquipTestTimer.function = equip_slic_test;
                g_stEquipTestTimer.expires = jiffies + msecs_to_jiffies(2000);
                add_timer(&g_stEquipTestTimer);
                */
                printk("#++++1+++++#");
            }
            else
            {
                slic_dtmfkey_report(SLIC_POUND_K);
            }
            break;

        case 0x0d :
            printk ( "report- A. ");
            slic_dtmfkey_report(SLIC_A_K);
            break;

        case 0x0e :
            printk ( "report- B. ");
            slic_dtmfkey_report(SLIC_B_K);
            break;

        case 0x0f :
            printk ( "report- C. ");
            slic_dtmfkey_report(SLIC_C_K);
            break;

        case 0x00 :
            printk ( "report- D. ");
            slic_dtmfkey_report(SLIC_D_K);
            break;

        default :   /* Error key */
            printk ( "Error key inside !\n");
            break;
        }
    }
    else    /* Error key */
    {
        printk ( "Error key inside !\n");
    }
}


/*------------------------------------------------------------------------
函数原型:static void slic_fsk_update_isr(void)
描述: fsk来电显示数据更新函数
输入: slic_fsk_seize_num_reg[]
输出:数据写入REG58_FSKDAT
返回值: 无
------------------------------------------------------------------------*/
static void slic_fsk_update_isr(void)
{
    int i = 0;
    uInt8 tmp = 0;
    tmp = slic_direct_read(0,IRQ1);
    if(0x40 != (0x40 & tmp))
    {
        return;
    }
    else /* FSK 8-byte FIFO buffer has emptied */
    {
        tmp = slic_direct_read(0,LCRRTP);
        if(0x02 == (0x02 &tmp))
        {
            /*在FSK来电显示过程当中摘机，则直接退出*/
            slic_direct_write   ( 0, OMODE,  0x00 );
            slic_direct_write   ( 0, OCON,   0x00 );
            slic_direct_write   ( 0, IRQ1,   0xff );
            slic_direct_write   ( 0, IRQ2,   0xff );
            if(TWO_RING == four_ring_cadence)
            {
                slic_direct_write   ( 0, IRQEN1,   0x00 );
            }
            else
            {
                slic_direct_write   ( 0, IRQEN1,   0x20 );
            }
            slic_direct_write   ( 0, IRQEN2, 0x13 );

            slic_fsk_flag = FALSE;
            slic_cmd_mark = SLIC_CMD_UNLOCK;
            slic_dtmfkey(0x80);
            return;
        }
        INTLOCK();
        if(slic_fsk_seize_index < FSK_MODEL_PRECURSOR_FRAME)
        {
            /* FSK write the FIFO seize signal, and add the FIFO full */
            for(i=0; i<2;i++)
            {
                slic_direct_write( 0, FSKDAT, slic_fsk_seize_num_reg[slic_fsk_seize_index]);
                slic_fsk_seize_index++;
            }
        }
        else
        {
            if(slic_fsk_body_index < (callerid_body_lgth_conv+7))
            {
                for(i=0; i<2;i++)
                {
                    slic_direct_write   ( 0, FSKDAT, slic_num_reg_fsk_conv[slic_fsk_body_index]);
                    slic_fsk_body_index++;
                }
            }
            else/*发送完毕的情况*/
            {
                slic_direct_write   ( 0, OMODE,   0x00 );
                slic_direct_write   ( 0, OCON,    0x00 );
                if(TWO_RING == four_ring_cadence)
                {
                    slic_direct_write   ( 0, IRQEN1,   0x00 );
                }
                else
                {
                    slic_direct_write   ( 0, IRQEN1,   0x20 );
                }
                slic_direct_write   ( 0, IRQEN2,  0x13 );
                slic_fsk_flag = FALSE;
                slic_cmd_mark = SLIC_CMD_UNLOCK;
                //slic_spi_to_sync();
            }
        }
        INTFREE();
    }
}

/*------------------------------------------------------------------------
函数原型:static irqreturn_t slic_irq_handler(int irq, void *dev_id)
描述: slic中断处理函数，完成FSK来电显示发送，按键键值上报
输入: 外部INT中断信号
输出: 按键键值
返回值: irqreturn_t
------------------------------------------------------------------------*/
extern unsigned int BSP_PWRCTRL_SleepVoteLock(PWC_CLIENT_ID_E  enClientId);

irqreturn_t slic_irq_handler(int irq, void *dev_id)
{

    byte int_status1 = 0;
    byte int_status2 = 0;
    byte int_status3 = 0;
    byte temp = 0;
    int temp1 =0;
    byte temp_key = 0;


    /* 确认是SLic中断. */
    if( 0!=SLIC_GPIO_REG_GETBIT( g_gpioBase, HI_GPIO_GPIO_INTSTATUS, 22 ) )
    {
         //pr_info( "detect GPIO_0_22 irq. \n" );
         //printk("DDR of bit 22 is %d\n", SLIC_GPIO_REG_GETBIT(g_gpioBase, HI_GPIO_SWPORT_DDR, 22));
    }
    else
    {
        pr_info( "not irq GPIO_0_22, ignored.\n" );
        return IRQ_NONE;
    }
    SLIC_GPIO_REG_SETBIT(g_gpioBase, HI_GPIO_PORT_EOI, 22);
    /* Slic中断投LCD唤醒票，待验证完成后，放到摘机中断处理的位置 */    
    #ifdef E5172_SLIC_WAKEUP_DEBUG
    BSP_PWRCTRL_SleepVoteLock(PWRCTRL_SLEEP_LCD);
    #endif
    /* End */

    if (TRUE == slic_fsk_flag)
    {
        /* FSK来显成功进入中断后，立即删除解锁回调定时器 */
        del_timer(&(slic_dev->slic_callerid_fsk_delay_timer));
        slic_fsk_update_isr();
        return IRQ_HANDLED;
    }


    /* Read interrupt registers and judge */
    if ( SLIC_CMD_UNLOCK == slic_cmd_mark )
    {
        /* Lock uC interface */
        slic_cmd_mark = SLIC_CMD_LOCK;
         if(1 == g_Equip_Test)
        {
            slic_cmd_mark = SLIC_CMD_UNLOCK;
        }

        /* If there is an error interrupt by noise, avoid this by following codes */
        temp1 = gpio_in(GPIO_SLIC_INT);
        if (GPIO_HIGH_VALUE == temp1)
        {
            slic_cmd_mark = SLIC_CMD_UNLOCK;
            return IRQ_HANDLED;
        }
        int_status1 = slic_direct_read (0,IRQ1);
        int_status2 = slic_direct_read (0,IRQ2);
        int_status3 = slic_direct_read (0,IRQ3);

        if (( int_status1 & 0x01 ) == 0x01)
        {
            uInt8 Tmp;
            if (TIP_TONE == slic_oscillator_soundtype)
            {
                switch (slic_tip_tone_type)
                {
                case SLIC_FAIL_TIP_TONE:
                    slic_tip_tone_type = SLIC_SUCCEED_TIP_TONE;
                    break;

                case SLIC_SUCCEED_TIP_TONE:
                    slic_tip_tone_type = SLIC_NULL_TIP_TONE;
                    break;

                default:
                    break;
                }

                if ( SLIC_NULL_TIP_TONE == slic_tip_tone_type)
                {
                    temp = slic_direct_read(0, IRQEN1);
                    temp = temp & 0xfe;
                    slic_direct_write(0, IRQEN1, temp);

                    /*close the two Oscillator*/
                    slic_direct_write (0, OCON,0x00);
                    slic_int_enable1_default_value &= 0xfe;
                }
            }

            else if (CALL_WAITING_TONE_ONTIME1 == slic_oscillator_soundtype)
            {
                Tmp = slic_direct_read(0, IRQEN1);
                Tmp = Tmp & (~0x01);
                slic_direct_write(0, IRQEN1, Tmp);

                slic_direct_write(0, O1TALO, 0x40 );
                slic_direct_write(0, O1TAHI, 0x06 );
                slic_direct_write(0, O1TILO, 0x40 );
                slic_direct_write(0, O1TIHI, 0x06 );

                slic_direct_write(0, IRQEN1, 0x02);
                slic_oscillator_soundtype = CALL_WAITING_TONE_OFFTIME1;
            }

            else if (CALL_WAITING_TONE_ONTIME2 == slic_oscillator_soundtype)
            {
                Tmp = slic_direct_read(0, IRQEN1);
                Tmp = Tmp & (~0x01);
                slic_direct_write(0, IRQEN1, Tmp);

                slic_direct_write   ( 0, O1TALO,  0x40 );
                slic_direct_write   ( 0, O1TAHI,  0x06 );
                slic_direct_write   ( 0, O1TILO,  0x80 );
                slic_direct_write   ( 0, O1TIHI,  0x89 );

                slic_direct_write(0, IRQEN1, 0x02);
                slic_oscillator_soundtype = CALL_WAITING_TONE_OFFTIME2;
                slic_cw_cycle_num++;

                /* 601 optus */
                if(COUNTRY_AU == g_Country_Type)
                {
                    if(9 == slic_cw_cycle_num)
                    {
                        slic_direct_write   (0, OCON,  0x00 );
                        slic_cw_cycle_num = 0;
                        Tmp = slic_direct_read(0, IRQEN1);
                        Tmp = Tmp & (~0x02);
                        slic_direct_write(0, IRQEN1, Tmp);
                        slic_oscillator_soundtype = CALL_WAITING_TONE_ONTIME1;
                    }
                }
            }
            else if (CONFIRMATION_TONE_ONTIME1 == slic_oscillator_soundtype)
            {
                Tmp = slic_direct_read(0, IRQEN1);
                Tmp = Tmp & (~0x01);
                slic_direct_write(0, IRQEN1, Tmp);

                slic_direct_write(0, O1TALO, 0x20 );
                slic_direct_write(0, O1TAHI, 0x03 );
                slic_direct_write(0, O1TILO, 0x20 );
                slic_direct_write(0, O1TIHI, 0x03 );

                slic_direct_write(0, IRQEN1, 0x02);
                slic_oscillator_soundtype = CONFIRMATION_TONE_OFFTIME1;
                
            }
            else if (CONFIRMATION_TONE_ONTIME2 == slic_oscillator_soundtype)
            {
                Tmp = slic_direct_read(0, IRQEN1);
                Tmp = Tmp & (~0x01);
                slic_direct_write(0, IRQEN1, Tmp);
                slic_direct_write( 0, OCON, 0x00 );
            }
        }

        /* Four ring cadence handling, used ringer inactive timer interrupt */
        else if (0x20 == ( int_status1 & 0x20))
        {
            slic_direct_write(0, RINGCON, 0x00);
            switch (slic_four_ring_part)
            {
            case SLIC_FIRST_PART_RING :
                slic_direct_write   ( 0, RINGTALO,        (byte)slic_parm[g_Country_Type].ring.ontime2 );
                slic_direct_write   ( 0, RINGTAHI,        (byte)(slic_parm[g_Country_Type].ring.ontime2>>8) );
                slic_direct_write   ( 0, RINGTILO,        (byte)slic_parm[g_Country_Type].ring.offtime2 );
                slic_direct_write   ( 0, RINGTIHI,        (byte)(slic_parm[g_Country_Type].ring.offtime2>>8) );
                slic_four_ring_part = SLIC_LAST_PART_RING;
                break;

            case SLIC_LAST_PART_RING :
                slic_direct_write   ( 0, RINGTALO,        (byte)slic_parm[g_Country_Type].ring.ontime1 );
                slic_direct_write   ( 0, RINGTAHI,        (byte)(slic_parm[g_Country_Type].ring.ontime1>>8) );
                slic_direct_write   ( 0, RINGTILO,        (byte)slic_parm[g_Country_Type].ring.offtime1 );
                slic_direct_write   ( 0, RINGTIHI,        (byte)(slic_parm[g_Country_Type].ring.offtime1>>8) );
                slic_four_ring_part = SLIC_FIRST_PART_RING;
                break;

            case SLIC_RPAS_RING :
                if( (byte)CLIP_FSK_RPAS == slic_ioc_data_pt->clipmode )
                {
                    /* burst   pause_ext   burst_ext  pause */
                    slic_direct_write   ( 0, RINGTALO,        0xc0 ); /*change for spain FSK standard RP-AS*/
                    slic_direct_write   ( 0, RINGTAHI,        0x08 );
                    slic_direct_write   ( 0, RINGTILO,        0x40 );
                    slic_direct_write   ( 0, RINGTIHI,        0x38 );
                    if( FOUR_RING == four_ring_cadence )
                    {
                        slic_four_ring_part = SLIC_LAST_PART_RING;
                    }
                    else
                    {
                        slic_four_ring_part = SLIC_RPAS_NORMAL_RING;
                    }
                    temp = slic_direct_read(0, IRQEN1);
                    temp = temp | (0x20);
                    slic_direct_write(0, IRQEN1, temp);
                }
                break;

            case SLIC_RPAS_NORMAL_RING:
                /* burst  pause */
                slic_direct_write   ( 0, RINGTALO,        (byte)slic_parm[g_Country_Type].ring.ontime1 );
                slic_direct_write   ( 0, RINGTAHI,        (byte)(slic_parm[g_Country_Type].ring.ontime1>>8) );
                slic_direct_write   ( 0, RINGTILO,        (byte)slic_parm[g_Country_Type].ring.offtime2 );
                slic_direct_write   ( 0, RINGTIHI,        (byte)(slic_parm[g_Country_Type].ring.offtime2>>8) );

                slic_four_ring_part = SLIC_RPAS_NORMAL_RING;
                temp = slic_direct_read(0, IRQEN1);
                temp = temp | (0x20);
                slic_direct_write(0, IRQEN1, temp);
                break;

            default :
                pr_info ( "Four ring interrupt part set error!  ");
                break;
            }
            slic_direct_write   ( 0, RINGCON,  0x58);
        }
        else if (0x02 == ( int_status1 & 0x02))
        {
            uInt8 Tmp;
            Tmp = slic_direct_read(0, IRQEN1);
            Tmp = Tmp & (~0x02);
            slic_direct_write(0, IRQEN1, Tmp);
            if(CALL_WAITING_TONE_OFFTIME1 == slic_oscillator_soundtype )
            {
                slic_direct_write   (0, OCON,  0x00 );

                slic_direct_write   (0, O1TALO,  0x40 );
                slic_direct_write   (0, O1TAHI,  0x06 );
                slic_direct_write   (0, O1TILO,  0x40 );
                slic_direct_write   (0, O1TIHI,  0x06 );

                slic_direct_write   (0, OCON,   0x07 );       //enable the oscillator1
                slic_direct_write   (0, OMODE,  0x02 );
                slic_direct_write   (0, IRQEN1, 0x01 );
                slic_oscillator_soundtype = CALL_WAITING_TONE_ONTIME2;
            }
            else if(CALL_WAITING_TONE_OFFTIME2 == slic_oscillator_soundtype )
            {
                slic_direct_write   (0, OCON,  0x00 );

                slic_direct_write   (0, O1TALO,       0x40 );
                slic_direct_write   (0, O1TAHI,       0x06 );
                slic_direct_write   (0, O1TILO,       0x80 );
                slic_direct_write   (0, O1TIHI,       0x89 );

                slic_direct_write   (0, OCON,  0x07 );       //enable the oscillator1
                slic_direct_write   (0, OMODE, 0x02 );
                slic_direct_write   (0, IRQEN1, 0x01);
                slic_oscillator_soundtype = CALL_WAITING_TONE_ONTIME1;
            }
            else if(CONFIRMATION_TONE_OFFTIME1 == slic_oscillator_soundtype )
            {
                slic_direct_write   (0, OCON,    0x00 );

                slic_direct_write   (0, O1TALO, 0x60);
                slic_direct_write   (0, O1TAHI, 0x09);
                slic_direct_write   (0, O1TILO, 0x20);
                slic_direct_write   (0, O1TIHI, 0x03);

                slic_direct_write   (0, OCON,  0x07);       //enable the oscillator1
                slic_direct_write   (0, OMODE, 0x02);
                slic_direct_write   (0, IRQEN1, 0x01);
                slic_oscillator_soundtype = CONFIRMATION_TONE_ONTIME2;
            }
        }
        /* DTMF tone detected interrupt */
        else if ( 0x10 == ( int_status2 & 0x10 ))
        {
            temp_key = slic_direct_read(0, TONDTMF);
            temp_key = temp_key | 0x10; /*用于标识是普通的按键*/
            slic_dtmfkey ( temp_key );
        }
        /* ring trip interrupt */
        else if ( 0x01 == ( int_status2 & 0x01) )
        {
            /* 振铃中摘机 Ring-trip Interrupt Active */
            slic_dtmfkey (0x80);
        }

        /* loop condition change interrupt */
        else if ( 0x02 == ( int_status2 & 0x02) )
        {
            temp_key = slic_direct_read (0, LCRRTP);
            /* check whether on-hook or off-hook!! */
            if (( temp_key & 0x02 ) == 0x00) /* on-hook */
            {
                slic_dtmfkey ( 0xc0 );
            }
            else
            {
                slic_dtmfkey (0x80);
            }
        }
        else /* Error interrupt entrance */
        {
            pr_info ( "Error interrupt type!  " );
            //slic_spi_to_sync();
        }
        /* Unlock uC interface */
        slic_cmd_mark = SLIC_CMD_UNLOCK;
    }
    else
    {
        pr_info ("Wait for reading interrupt registers.\n");
        slic_dev->slic_int_delay_timer.function = (void (*)(unsigned long))slic_irq_handler;

        /*check timer is working or not.*/
        if (!timer_pending(&(slic_dev->slic_int_delay_timer)))
            mod_timer(&(slic_dev->slic_int_delay_timer), msecs_to_jiffies(5) + jiffies);
    }

    return IRQ_HANDLED;
}

/*------------------------------------------------------------------------
函数原型:slic_gpio_config
描述: SPI 控制器
输入:   无
       
输出:   无
返回值: 无
------------------------------------------------------------------------*/
void slic_gpio_config(void)
{

    HiInitIrq();

    if( 0==g_gpioBase )
    {
        g_gpioBase = (unsigned long)ioremap(0x90006000,  0x1000);
    }   
    if(gpio_request(GPIO_SLIC_INT, "GPIO_SLIC_INT"))
    {
        printk("request slic int gpio failed.");
        return;
    }
    gpio_direction_input(GPIO_SLIC_INT);
    gpio_int_mask_set(GPIO_SLIC_INT);
    
    if(gpio_get_value(GPIO_SLIC_INT))
    {
        gpio_int_trigger_set(GPIO_SLIC_INT, IRQ_TYPE_LEVEL_LOW);
    }
    else
    {
        gpio_int_trigger_set(GPIO_SLIC_INT, IRQ_TYPE_LEVEL_HIGH);
    }
    gpio_set_function(GPIO_SLIC_INT, GPIO_INTERRUPT);
    gpio_int_state_clear(GPIO_SLIC_INT);
    gpio_int_unmask_set(GPIO_SLIC_INT);
    
    /* 复位管脚 */
    if(gpio_request(GPIO_SLIC_RST, "GPIO_SLIC_RST"))
    {
        printk("request slic reset gpio failed.");
        return;
    }
    gpio_direction_output(GPIO_SLIC_RST, 0);
    gpio_set_function(GPIO_SLIC_RST, GPIO_NORMAL);

    /* SPI管脚配置 */
    if(gpio_request(GPIO_SLIC_SPI_CLK, "GPIO_SLIC_SPI_CLK"))
    {
        printk("request slic spi clk gpio failed.");
        return;
    }
    gpio_direction_output(GPIO_SLIC_SPI_CLK, 0);
    gpio_set_function(GPIO_SLIC_SPI_CLK, GPIO_NORMAL);

    if(gpio_request(GPIO_SLIC_SPI_SDI, "GPIO_SLIC_SPI_SDI"))
    {
        printk("request slic spi data in gpio failed.");
        return;
    }
    gpio_direction_input(GPIO_SLIC_SPI_SDI);
    gpio_set_function(GPIO_SLIC_SPI_SDI, GPIO_NORMAL);

    if(gpio_request(GPIO_SLIC_SPI_SDO, "GPIO_SLIC_SPI_SDO"))
    {
        printk("request slic spi data out gpio failed.");
        return;
    }
    gpio_direction_output(GPIO_SLIC_SPI_SDO, 0);
    gpio_set_function(GPIO_SLIC_SPI_SDO, GPIO_NORMAL);

    /* PCM管脚配置 */
    if(gpio_request(GPIO_SLIC_PCM_CLK, "GPIO_SLIC_PCM_CLK"))
    {
        printk("request slic pcm clk gpio failed.");
        return;
    }
    gpio_direction_output(GPIO_SLIC_PCM_CLK, 0);
    gpio_set_function(GPIO_SLIC_PCM_CLK, GPIO_NORMAL);

    if(gpio_request(GPIO_SLIC_PCM_SYNC, "GPIO_SLIC_PCM_SYNC"))
    {
        printk("request slic pcm fsync clk gpio failed.");
        return;
    }
    gpio_direction_output(GPIO_SLIC_PCM_SYNC, 0);
    gpio_set_function(GPIO_SLIC_PCM_SYNC, GPIO_NORMAL);

    if(gpio_request(GPIO_SLIC_PCM_IN, "GPIO_SLIC_PCM_IN"))
    {
        printk("request slic pcm data in gpio failed.");
        return;
    }
    gpio_direction_input(GPIO_SLIC_PCM_IN);
    gpio_set_function(GPIO_SLIC_PCM_IN, GPIO_NORMAL);

    if(gpio_request(GPIO_SLIC_PCM_OUT, "GPIO_SLIC_PCM_OUT"))
    {
        printk("request slic pcm data out gpio failed.");
        return;
    }
    gpio_direction_output(GPIO_SLIC_PCM_OUT, 0);
    gpio_set_function(GPIO_SLIC_PCM_OUT, GPIO_NORMAL);   
    
    return;
}

/*------------------------------------------------------------------------
函数原型:get_countryType
描述: 从文件获取配置的国家
输入:  无
       
输出:国家枚举
返回值: 无
------------------------------------------------------------------------*/
void get_countryType(void) 
{ 
	struct file *filp; 
	struct inode *inode; 
	mm_segment_t old_fs; 
	off_t fsize; 
	char *buf = NULL; 
	int country = 0;
    int i = 0;
	
    char * filename = "/data/countryconfig";
    
	printk("<1>get_countryType: start....\n");

	filp=filp_open(filename,O_RDONLY,0); 
       if (IS_ERR(filp))
       {
            printk("<error>get_countryType: Open file failed!\n");
            g_Country_Type = COUNTRY_CN;
            return ;
       }

       /*获取文件大小*/
        inode=filp->f_dentry->d_inode;  
        fsize=inode->i_size; 

        buf = (char*)kmalloc(fsize+1,GFP_KERNEL);
        if (NULL == buf)
        {
            printk("<error>get_countryType: Malloc memory failed!\n");
            g_Country_Type = country;
            return ;
        }

        /*设置内存空间，进行用户态和内核态映射*/
        old_fs = get_fs();
        
        set_fs(KERNEL_DS);

        vfs_read(filp, buf, fsize, &(filp->f_pos));
        buf[fsize]='\0';
        
        /*计算国家枚举值*/

    	while (i < (fsize - 1))
    	{
    		country = country * 10 + ((int)buf[i] - 48);
    		i++;
    	}
    	if ((country >= 0) && (country < COUNTRY_COUNT))
    	{
    		g_Country_Type = country;
    	}
    	else
       {
            printk("<error>get_countryType: country error!country=%d\n",country); 
       }
       kfree(buf); 
       buf = NULL;
	filp_close(filp,NULL);   
       set_fs(old_fs);

       printk("<1>get_countryType: end....\n");
} 

/*------------------------------------------------------------------------
函数原型:slic_direct_read
描述: SLIC寄存器读函数
输入: addr--地址，buff--读得的数据存储位置
输出: 无
返回值: 0--成功
------------------------------------------------------------------------*/
uInt8 slic_direct_read (uInt8 channel, uInt8 addr)
{
    return SlicReadReg(addr);
}

/*------------------------------------------------------------------------
函数原型:slic_direct_write
描述: SLIC REG写
输入: addr--地址，data--待写的数据
输出: NA
返回值: 0-成功
------------------------------------------------------------------------*/
int slic_direct_write (uint8 channel,  uInt8 addr, uInt8 data)
{
    SlicWriteReg(addr, data);
    return 0;
}

/*------------------------------------------------------------------------
函数原型:slic_indirect_read
描述: 读SLIC RAM
输入: addr--地址
输出: 读得的RAM数据
返回值: RAM数据
------------------------------------------------------------------------*/
uInt32 slic_indirect_read (word addr)
{
    dword   data = 0;
    data = SlicReadRam(addr);
    return data;
}

/*------------------------------------------------------------------------
函数原型:slic_indirect_write
描述: SLIC RAM写
输入: addr-地址，data--数据
输出: NA
返回值: 无
------------------------------------------------------------------------*/
void slic_indirect_write (word addr, dword data)
{
    INTLOCK();

    SlicWriteRam(addr, data);

    INTFREE();
}


int ctrl_ResetWrapper (void *hSpiGci, int status)
{
    printk("will hard reset slic chip.\n");

    gpio_set_value(GPIO_SLIC_RST, 0);
    msleep(200);
    gpio_set_value(GPIO_SLIC_RST, 1);
    msleep(200);
    gpio_set_value(GPIO_SLIC_RST, 0);
    return 0;
}


uInt8 ctrl_ReadRegisterWrapper (void *hSpiGci, uInt8 channel, uInt8 addr)
{
      return (slic_direct_read (channel, addr));
}

int ctrl_WriteRegisterWrapper (void *hSpiGci, uint8 channel,  uInt8 addr, uInt8 data)
{
      slic_direct_write (channel, addr, data);
      return 0;
}

uInt32 ctrl_ReadRAMWrapper (void *hSpiGci, uint8 channel, uInt16 addr)
{
	  return (slic_indirect_read(addr));
}

int ctrl_WriteRAMWrapper (void *hSpiGci, uint8 channel, uInt16 addr, ramData data)
{
      slic_indirect_write(addr, data);
      return 1;
}

void TimerInit (int *pTimerObj)
{
    return;
}

int time_DelayWrapper (void *hTimer, int timeInMs)
{
    mdelay(timeInMs);
	return 1;
}

int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs)
{
	return 1;
}

int time_GetTimeWrapper (void *hTimer, void *time)
{
	return 1;
}

void Equ_Slic_Test(void)
{
    slic_loopback();
    slic_sound_start(SLIC_RING);
    g_Equip_Test = 1;
}



/* INIT ProSLIC hardware interface */
void slicctl_proslic_init(void)
{
      uInt8 channel=0;

      /*
      ** Step 1: (optional)
      ** Initialize user's control interface and timer objects. This
      ** may already be done, if not, do it here
      */

      /* Initialize SPI */
      //We have Initialized SPI bus before this func,
      SlicSioInit();
      mdelay(300);

      /* Initialize timer */
      TimerInit(&timerObj);

      pr_info("Initializing ProSLIC interface \n");

      /*
      ** Step 2: (required)
      ** Create ProSLIC Control Interface Object
      */

      /* Create controller interfece */
      ProSLIC_createControlInterface(&ProHWIntf);

      /*
      ** Step 3: (required)
      ** Create ProSLIC Device Objects
      */

      /* Create device */
      for (channel = 0; channel < SLICCTL_NUMBER_OF_DEVICES; channel++)
         ProSLIC_createDevice (&(ProSLICDevices[channel]));

      /*
      ** Step 4: (required)
      ** Create and initialize ProSLIC channel objects
      ** Also initialize array pointers to user's proslic channel object
      ** members to simplify initialization process.
      */

      /* Create channels */
      for (channel = 0; channel < SLICCTL_NUMBER_OF_CHAN; channel++) 
      {
         ProSLIC_createChannel(&ProslicChans[channel]);
         ProSLIC_SWInitChan (ProslicChans[channel],channel, SI3217X_TYPE/*SI3226_TYPE*/, ProSLICDevices[channel/SLICCTL_CHAN_PER_DEVICE], ProHWIntf);
         ProSLIC_setSWDebugMode(ProslicChans[channel], TRUE);   /* optional */
      }

      /*
      ** Step 5: (required)
      ** Establish linkage between host objects/functions and
      ** ProSLIC API
      */

      ProSLIC_setControlInterfaceCtrlObj (ProHWIntf, &spiGciObj);
      ProSLIC_setControlInterfaceReset (ProHWIntf, ctrl_ResetWrapper);
      ProSLIC_setControlInterfaceWriteRegister (ProHWIntf, ctrl_WriteRegisterWrapper);
      ProSLIC_setControlInterfaceReadRegister (ProHWIntf, ctrl_ReadRegisterWrapper);		
      ProSLIC_setControlInterfaceWriteRAM (ProHWIntf, ctrl_WriteRAMWrapper);
      ProSLIC_setControlInterfaceReadRAM (ProHWIntf, ctrl_ReadRAMWrapper);
      ProSLIC_setControlInterfaceTimerObj (ProHWIntf, &timerObj);
      ProSLIC_setControlInterfaceDelay (ProHWIntf, time_DelayWrapper);
      ProSLIC_setControlInterfaceTimeElapsed (ProHWIntf, time_TimeElapsedWrapper);
      ProSLIC_setControlInterfaceGetTime (ProHWIntf, time_GetTimeWrapper);
      ProSLIC_setControlInterfaceSemaphore (ProHWIntf, NULL);

RESETSLIC:
    
      /*
      ** Step 6: (system dependent)
      ** Soft Reset
      */	
      ctrl_WriteRegisterWrapper(ProHWIntf, 0,  RESET, 0x1);
     
      /*
      ** Assert hardware Reset
      */	
      //ProSLIC_Reset(*ProslicChans);	/* Reset the ProSLIC(s) before we begin */
      ctrl_ResetWrapper(ProHWIntf, 0);

      /*
      ** Step 7: (required)
      ** Initialize device (loading of general parameters, calibrations,
      ** dc-dc powerup, etc.)
      */
      /* if (ProSLIC_Init(&ProslicChans[0], SLICCTL_NUMBER_OF_CHAN)) */
      if (ProSLIC_Init(&ProslicChans[0], 1))
      {
         pr_info("ERROR: SLIC Initialization failed \n");

         pr_info("ERROR: Reset Proslic Again! \n");
         reset_count = reset_count + 1;
		 
         pr_info("reset_count = [%d], delay 250ms \n", reset_count);
         mdelay(250);

		 if(reset_count <= 5)
         {
            goto RESETSLIC;
         }
      }

	/*
	 ** Step 8: PCM TIME Slot setup.
	 */
	//ProSLIC_PCMTimeSlotSetup(ProslicChans[0], HISI_CFGPCM_TXSLOT_CH0, HISI_CFGPCM_RXSLOT_CH0);
	ctrl_WriteRegisterWrapper(ProHWIntf, 0, PCMTXLO, 0x1);  /* PCMTXLO */
    ctrl_WriteRegisterWrapper(ProHWIntf, 0, PCMTXHI, 0x0);  /* PCMTXHI SLOT 0 */
    ctrl_WriteRegisterWrapper(ProHWIntf, 0, PCMRXLO, 0x1);  /* PCMRXLO */
    ctrl_WriteRegisterWrapper(ProHWIntf, 0, PCMRXHI, 0x0);  /* PCMRXHI SLOT 0 */

	/*
      ** Step 9: read country code.
    */

        //get_countryType();
      /*
      ** Step 8: (design dependent)
      ** Execute longitudinal balance calibration
      ** or reload coefficients from factory LB cal
      */

      ProSLIC_LBCal(&ProslicChans[0], SLICCTL_NUMBER_OF_CHAN);
}


void slicctl_chinit(int channel)
{
      uint8 reg=0;

      pr_info("Initializing SLIC for Ch:%d \n", channel);
      ProSLIC_DCFeedSetup(ProslicChans[channel], slic_parm[g_Country_Type].slic_dcfeed_type);
      ProSLIC_RingSetup(ProslicChans[channel], slic_parm[g_Country_Type].slic_ring_preset_type);
      #if (HISI_PCM_MODE == 4)
	  	ProSLIC_PCMSetup(ProslicChans[channel], PCM_16LIN_WB);
      #elif (HISI_PCM_MODE == 3)
         ProSLIC_PCMSetup(ProslicChans[channel], PCM_16LIN);
      #elif (HISI_PCM_MODE == 2)
         ProSLIC_PCMSetup(ProslicChans[channel], PCM_8ALAW);
      #elif (HISI_PCM_MODE == 1)
         ProSLIC_PCMSetup(ProslicChans[channel], PCM_8ULAW);
      #endif
      ProSLIC_ZsynthSetup(ProslicChans[channel], slic_parm[g_Country_Type].slic_zsynth_type);
      /*ProSLIC_EnableInterrupts(ProslicChans[channel]);*/

      reg = ctrl_ReadRegisterWrapper(ProHWIntf, channel, 0x3);
      pr_info("ChInit: Reading SLIC STAT REG:0x%x \n", reg);

      /* Reset loopback register */
      ctrl_WriteRegisterWrapper (ProHWIntf, channel,  LOOPBACK, 0x0);     /* Reset loopback register */
      
      /* Disable Tx & Rx Modem tone detectors in SLIC */
      //ctrl_WriteRegisterWrapper (ProHWIntf, channel,  TONEN, 0x3);
      ProSLIC_SetLinefeedStatus(ProslicChans[channel], LF_FWD_ACTIVE);
      pr_info("delay 1500ms before Enable Interrupts... \n");
      mdelay(500);
      mdelay(500);
	  mdelay(500);
#if 0  /* 这里的初始化流程会影响DTMF正常上报.自测试发现在步步高HCD007(33)TSDL话机上DTMF高概率无法上报. */
      ProSLIC_dbgSetTXGain(ProslicChans[channel],0,ZSYN_600_0_0_30_0/*ZSYN_600_0_0_30_24_10*/,0);
      ProSLIC_dbgSetRXGain(ProslicChans[channel],0,ZSYN_600_0_0_30_0/*ZSYN_600_0_0_30_24_10*/,1);
      ProSLIC_TXAudioGainSetup(ProslicChans[channel],0);
      ProSLIC_RXAudioGainSetup(ProslicChans[channel],1);
      pr_info("ChInit: set TxGain and RxGain = 0, 0. \n");
#endif

      /* Enable Interrupts. */
      ProSLIC_EnableInterrupts(ProslicChans[channel]);

      /* Free Run Mode. 这个与低功耗相关,带电池的CPE需要. */
      //ProSLIC_PLLFreeRunStart(ProslicChans[channel]);

	  ProSLIC_PCMStart(ProslicChans[channel]);
      //ProSLIC_PrintDebugData(ProslicChans[channel]);

      //slicctl_events[channel].event_cnt = 0;
      //slicctl_onhook_validate[channel] = 0;  /* Reset ONHOOK validation flag */
      //slicctl_pre_hstate[channel] = 0;   /* Set previous event as ONHOOK */
      //slicctl_pre_fring[channel] = 0;    /* Reset First RING status */



    if((0x0 != slic_parm[g_Country_Type].ring.offtime1)
        && (0x0 != slic_parm[g_Country_Type].ring.ontime2))
    {
        four_ring_cadence = FOUR_RING;
        printk("FOUR RING\n");
    }
    else
    {
        four_ring_cadence = TWO_RING;
        printk("TWO RING\n");
    }
      

      if( (byte)CLIP_FSK_RPAS != slic_ioc_data_pt->clipmode )
      {
        /* Set Four ring cadence register */
        if ( FOUR_RING == four_ring_cadence )
        {
            /* burst   pause_ext   burst_ext  pause */
            slic_direct_write( 0, RINGTALO, (byte)slic_parm[g_Country_Type].ring.ontime1 );
            slic_direct_write( 0, RINGTAHI, (byte)(slic_parm[g_Country_Type].ring.ontime1>>8) );
            slic_direct_write( 0, RINGTILO, (byte)slic_parm[g_Country_Type].ring.offtime1 );
            slic_direct_write( 0, RINGTIHI, (byte)(slic_parm[g_Country_Type].ring.offtime1>>8) );
            slic_four_ring_part = SLIC_LAST_PART_RING;
        }
        else if ( TWO_RING == four_ring_cadence )
        {
            /* burst  pause */
            slic_direct_write( 0, RINGTALO, (byte)slic_parm[g_Country_Type].ring.ontime1 );
            slic_direct_write( 0, RINGTAHI, (byte)(slic_parm[g_Country_Type].ring.ontime1>>8) );
            slic_direct_write( 0, RINGTILO, (byte)slic_parm[g_Country_Type].ring.offtime2 );
            slic_direct_write( 0, RINGTIHI, (byte)(slic_parm[g_Country_Type].ring.offtime2>>8) );
            
        }
        else
        {      
            slic_direct_write( 0, RINGTALO, (byte)slic_parm[g_Country_Type].ring.ontime1 );
            slic_direct_write( 0, RINGTAHI, (byte)(slic_parm[g_Country_Type].ring.ontime1>>8) );
            slic_direct_write( 0, RINGTILO, (byte)slic_parm[g_Country_Type].ring.offtime2 );
            slic_direct_write( 0, RINGTIHI, (byte)(slic_parm[g_Country_Type].ring.offtime2>>8) );
        }
      }
      else
      {
        /* burst   pause_ext   burst_ext  pause */
        slic_direct_write( 0, RINGTALO, 0xc0 ); /*change for spain FSK standard RP-AS*/
        slic_direct_write( 0, RINGTAHI, 0x08 );
        slic_direct_write( 0, RINGTILO, 0x40 );
        slic_direct_write( 0, RINGTIHI, 0x38 );
        slic_four_ring_part = SLIC_RPAS_RING;
        pr_info("FSK_RPAS slic sound start");
      }
}



/*------------------------------------------------------------------------
函数原型:static void slic_sound_start (  slic_tone_enum_type tone )
描述: 各种声音处理函数
输入: tone-声音类型
输出: 声音寄存器配置
返回值: none
------------------------------------------------------------------------*/
void slic_sound_start (  slic_tone_enum_type tone )
{	
    if ( SLIC_CMD_UNLOCK == slic_cmd_mark ) /* uC interface is free */
    {
        DisSlicInt();
		
		/* Lock uC interface */
        slic_cmd_mark = SLIC_CMD_LOCK;
         if(1 == g_Equip_Test)
        {
            slic_cmd_mark = SLIC_CMD_UNLOCK;
        }
        
		switch ( (int)tone )
		{
		    case SLIC_RING:
			{
				
                /*ProSLIC_ToneGenStop(ProslicChans[0]);*/

				
                if(TWO_RING == four_ring_cadence)
                {
				    slic_direct_write (0, IRQEN1, 0x00);
                }
                else
                {
                    slic_direct_write (0, IRQEN1, 0x20);
                }
				

				

				/* B593s-22 add pcm start flow here. */
				//ProSLIC_PCMStart (ProslicChans[0]);

				/* Set linefeed status. */
				ProSLIC_RingStart(ProslicChans[0]);

				/* B539s-22 do not call ProSLIC_ToneGenStop here. it's very dangerous.  */
				//ProSLIC_ToneGenStop(ProslicChans[0]);

                /* Set tone type by Si3217x_Tone_Presets.  */
				//ProSLIC_ToneGenSetup(ProslicChans[0], DTMF_DIGIT_8);

				/* 从振铃后延时多少开始来显DTMF=1500 RPAS=900 */
				/* TODO: CS voice should set timer in kernel space, but PS voice can not. */
	            if ( (byte)CLIP_FSK_RPAS == slic_ioc_data_pt->clipmode )  //fsk
	            {
	                callerid_showtime = CALLERID_SHOWTIME_FSK_RPAS;
	            }
	            else 
	            {
                    /* ZJC:澳大利亚局方测试结果振铃478ms后就进入FSK来电显示流程，不符合大于500ms的规范，故增加50ms */
	                callerid_showtime = slic_parm[g_Country_Type].ring.ontime1 /8 + slic_parm[g_Country_Type].ring.offtime1 /8
	                    + slic_parm[g_Country_Type].ring.ontime2 /8 + 500 + 50;
	            }

				slic_dev->slic_callerid_delay_timer.function = (void (*)(unsigned long))slic_callerid;

            	/*check timer is working or not.*/
            	if (!timer_pending(&(slic_dev->slic_callerid_delay_timer)))
                	mod_timer(&(slic_dev->slic_callerid_delay_timer), msecs_to_jiffies(callerid_showtime) + jiffies);

		    }
			break;

			case SLIC_KEEP_RING:
		    {
		    }
			break;
            
			case SLIC_DIAL_TONE :
			{
                pr_info("**** dial tone[%d]. ****. \n", GEN_DIAL_TONE);
                
				ProSLIC_ToneGenStop(ProslicChans[0]);

				ProSLIC_ToneGenSetup(ProslicChans[0], GEN_DIAL_TONE); 

				/* Play dial tone. */
				ProSLIC_ToneGenStart(ProslicChans[0], 0x0);  //0x1 relate to OCON register.
		    }
			break;

			case SLIC_BUSY_TONE:
			{
                pr_info("**** busy tone[%d]. ****. \n", GEN_BUSY_TONE);
                
				ProSLIC_ToneGenStop(ProslicChans[0]);

				ProSLIC_ToneGenSetup(ProslicChans[0], GEN_BUSY_TONE); 

				/* Play Busy tone. */
				ProSLIC_ToneGenStart(ProslicChans[0], 0x1);
		    }
			break;

			case SLIC_ALARM_TONE:
			{
                pr_info("**** alarm tone[%d]. ****. \n", GEN_ALARM_TONE);
                
				ProSLIC_ToneGenStop(ProslicChans[0]);

				ProSLIC_ToneGenSetup(ProslicChans[0], GEN_ALARM_TONE); 

				/* Play Alarm tone. */
				ProSLIC_ToneGenStart(ProslicChans[0], 0x0);
		    }
			break;

			case SLIC_TIP_TONE:
			{
                pr_info("**** tip tone[%d]. ****. \n", GEN_TIP_TONE);
                
				ProSLIC_ToneGenStop(ProslicChans[0]);

				ProSLIC_ToneGenSetup(ProslicChans[0], GEN_TIP_TONE); 

				/* Play TIP tone. */
				ProSLIC_ToneGenStart(ProslicChans[0], 0x1);
		    }
			break;

			case SLIC_FAULT_TONE:
			{
                pr_info("**** fault tone[%d]. ****. \n", GEN_FAULT_TONE);
                
				ProSLIC_ToneGenStop(ProslicChans[0]);

				ProSLIC_ToneGenSetup(ProslicChans[0], GEN_FAULT_TONE); 

				/* Play Fault tone. */
				ProSLIC_ToneGenStart(ProslicChans[0], 0x1);
		    }
			break;

			case SLIC_CALL_WAITING_TONE:
			{
                pr_info("**** call waiting tone[%d]. ****. \n", GEN_CALL_WAITING_TONE);
                slic_direct_write( 0, OCON, 0x00 );

                /*control method of tone generator*/
                slic_indirect_write( OSC1FREQ, slic_parm[g_Country_Type].waiting_tone.frequency1 );
                slic_indirect_write( OSC1AMP,  slic_parm[g_Country_Type].waiting_tone.amplitude1 );
                slic_indirect_write( OSC1PHAS, 0x00000000 );

                /* Set Four ring cadence register */
                /* burst   pause_ext   burst_ext  pause */
                slic_direct_write( 0, O1TALO, (u8)slic_parm[g_Country_Type].waiting_tone.burst );
                slic_direct_write( 0, O1TAHI, (u8)(slic_parm[g_Country_Type].waiting_tone.burst >> 8) );
                slic_direct_write( 0, O1TILO, (u8)slic_parm[g_Country_Type].waiting_tone.pause );
                slic_direct_write( 0, O1TIHI, (u8)(slic_parm[g_Country_Type].waiting_tone.pause >> 8) );
                slic_oscillator_soundtype = CALL_WAITING_TONE_ONTIME1;

                slic_direct_write(0, IRQEN1, 0x01 );

                /* Control oscillators */
                slic_direct_write( 0, OCON, 0x07 );
                slic_direct_write( 0, OMODE, 0x02 );
		    }
			break;
            
            case SLIC_RINGBACK_TONE:
            {
                pr_info("**** ring back tone[%d]. ****. \n", GEN_RINGBACK_TONE);
                ProSLIC_ToneGenStop(ProslicChans[0]);
                ProSLIC_ToneGenSetup(ProslicChans[0], GEN_RINGBACK_TONE);
                ProSLIC_ToneGenStart(ProslicChans[0], 0x1);
		    }
            break;

            case SLIC_CONFIRMATION_TONE:
            {
                pr_info("**** confirmation tone[%d]. ****. \n", GEN_CONFIRMATION_TONE);
                
                slic_direct_write( 0, OCON, 0x00 );

                /* 425 Hz, -15 dBm */
                slic_indirect_write( OSC1FREQ, 0x078F0000L );
                slic_indirect_write( OSC1AMP,  0x00156000L );
                slic_indirect_write( OSC1PHAS, 0x0 );

                /* 100ms on, 100ms off */
                slic_direct_write( 0, O1TALO, 0x20 );
                slic_direct_write( 0, O1TAHI, 0x03 );
                slic_direct_write( 0, O1TILO, 0x20 );
                slic_direct_write( 0, O1TIHI, 0x03 );
                slic_oscillator_soundtype = CONFIRMATION_TONE_ONTIME1;

                slic_direct_write(0, IRQEN1, 0x01 );

                /* Control oscillators */
                slic_direct_write( 0, OCON, 0x07 );
                slic_direct_write( 0, OMODE, 0x02 );                
            }
            break;
            case SLIC_VM_STUTTER_TONE:
            {
                pr_info("**** voice mail stutter tone[%d]. ****. \n", GEN_VM_STUTTER_TONE);
                
                slic_direct_write( 0, OCON, 0x00 );

                /* 425 Hz, -15 dBm */
                slic_indirect_write( OSC1FREQ, 0x078F0000L );
                slic_indirect_write( OSC1AMP,  0x00156000L );
                slic_indirect_write( OSC1PHAS, 0x0 );
                /* 400 Hz, -15 dBm */
                slic_indirect_write( OSC2FREQ, 0x079C0000L );
                slic_indirect_write( OSC2AMP,  0x00140000L );
                slic_indirect_write( OSC2PHAS, 0x0 );

                /* 160ms on, 160ms off */
                slic_direct_write( 0, O1TALO, 0x00 );
                slic_direct_write( 0, O1TAHI, 0x05 );
                slic_direct_write( 0, O1TILO, 0x00 );
                slic_direct_write( 0, O1TIHI, 0x05 );
                /* 160ms on, 160ms off */
                slic_direct_write( 0, O2TALO, 0x00 );
                slic_direct_write( 0, O2TAHI, 0x05 );
                slic_direct_write( 0, O2TILO, 0x00 );
                slic_direct_write( 0, O2TIHI, 0x05 );

                slic_direct_write( 0, OMODE, 0x66 );
                /* Control oscillators */
                slic_direct_write( 0, OCON, 0x77 );
               
            }
            break;
            case SLIC_CALL_FWD_RMD_TONE:
            {
                pr_info("**** call forward reminder tone[%d]. ****. \n", GEN_CALL_FWD_RMD_TONE);
                
                slic_direct_write( 0, OCON, 0x00 );

                /* 425 Hz, -15 dBm */
                slic_indirect_write( OSC1FREQ, 0x078F0000L );
                slic_indirect_write( OSC1AMP,  0x00156000L );
                slic_indirect_write( OSC1PHAS, 0x0 );

                /* forever */
                slic_direct_write( 0, O1TALO, 0x0 );
                slic_direct_write( 0, O1TAHI, 0x0 );
                slic_direct_write( 0, O1TILO, 0x0 );
                slic_direct_write( 0, O1TIHI, 0x0 );

                /* Control oscillators */
                slic_direct_write( 0, OCON, 0x01 );
                slic_direct_write( 0, OMODE, 0x02 );                
            }
            break;

			default :   /* Error tone type */
			{
            	pr_info ("Error tone type!\n");
			}
            break;
		
		}

		/* Unlock uC interface */
        slic_cmd_mark = SLIC_CMD_UNLOCK;
		EnSlicInt();
    }
    else
    {
        pr_info("slic_sound_start uC interface is locked.\n");
        if(SLIC_TIP_TONE == tone)
        {
            slic_dev->slic_snd_start_delay_timer.function = (void (*)(unsigned long))slic_sound_start_delay;

            /*check timer is working or not.*/
            if (!timer_pending(&(slic_dev->slic_snd_start_delay_timer)))
                mod_timer(&(slic_dev->slic_snd_start_delay_timer), msecs_to_jiffies(50) + jiffies);
        }
    }
}


/*------------------------------------------------------------------------
函数原型:slic_sound_start_delay
描述: 当总线被锁后，延时50ms，再次调用发声
输入: na
输出: none
返回值: none
------------------------------------------------------------------------*/
static void slic_sound_start_delay ( void )
{
    slic_sound_start(SLIC_TIP_TONE);
}

/*------------------------------------------------------------------------
函数原型:slic_sound_stop_delay
描述: 当总线被锁后，延时50ms，再次调用停止发声
输入: NA
输出: NA
返回值: NA
------------------------------------------------------------------------*/
static void slic_sound_stop_delay (int4 timeunused)
{
    pr_info("come here! tone is %d", slic_sound_stop_tone);
    if ( SLIC_CMD_UNLOCK == slic_cmd_mark ) /* uC interface is free */
    {
        DisSlicInt();
		
		/* Lock uC interface */
        slic_cmd_mark = SLIC_CMD_LOCK;
         if(1 == g_Equip_Test)
        {
            slic_cmd_mark = SLIC_CMD_UNLOCK;
        }
		
        switch ( slic_sound_stop_tone )
        {
        	case SLIC_RING :
		    {
		    	ProSLIC_RingStop(ProslicChans[0]);
		    }
			break;

        	case SLIC_KEEP_RING :
		    {
		    }
			break;

        	case SLIC_CALL_WAITING_TONE :   /* call waiting tone */
			{
            	slic_oscillator_soundtype = TIP_TONE;

				ProSLIC_ToneGenStop(ProslicChans[0]);
        	}
            break;

	        case SLIC_TIP_TONE   :    /* Succeed/fail tone */
	        case SLIC_DIAL_TONE  :    /* Dial tone  */
	        case SLIC_BUSY_TONE  :    /* Busy tone  */
	        case SLIC_ALARM_TONE :    /* Alarm tone */
	        case SLIC_FAULT_TONE :    /* Fault tone */
			{
				ProSLIC_ToneGenStop(ProslicChans[0]);
	        }
            break;

        	default:
			{
            	pr_info ("Error tone type!\n");
        	}
            break;
        }

        /* Unlock uC interface */
        slic_cmd_mark = SLIC_CMD_UNLOCK;
		EnSlicInt();
    }
    else /* uC interface is locked */
    {
       
        /*延迟50毫秒后重新尝试停止该声音*/
        slic_dev->slic_snd_stop_delay_timer.function = (void (*)(unsigned long))slic_sound_stop_delay;

        /*check timer is working or not.*/
        if (!timer_pending(&(slic_dev->slic_snd_stop_delay_timer)))
            mod_timer(&(slic_dev->slic_snd_stop_delay_timer), msecs_to_jiffies(50) + jiffies);
    }
}

/*------------------------------------------------------------------------
函数原型:slic_sound_stop
描述: 声音停止函数
输入: tone--声音类型
输出: na
返回值: none
------------------------------------------------------------------------*/
void slic_sound_stop ( slic_tone_enum_type tone )
{
   if ( SLIC_CMD_UNLOCK == slic_cmd_mark ) /* uC interface is free */
    {
        /* 停止振铃时候同步取消定时器 */
        del_timer(&(slic_dev->slic_callerid_delay_timer));
        pr_info( "del_timer slic_callerid_delay_timer %s().\n", __FUNCTION__ );        
        
        DisSlicInt();
		
		/* Lock uC interface */
        slic_cmd_mark = SLIC_CMD_LOCK;
         if(1 == g_Equip_Test)
        {
            slic_cmd_mark = SLIC_CMD_UNLOCK;
        }
        
		switch(tone)
		{
		    case SLIC_RING :
		    {
		    	ProSLIC_RingStop(ProslicChans[0]);
		    }
			break;

			case SLIC_KEEP_RING :
		    {
		    }
			break;

			case SLIC_CALL_WAITING_TONE:
		    {
				slic_oscillator_soundtype = TIP_TONE;

				ProSLIC_ToneGenStop(ProslicChans[0]);
		    }
			break;

			case SLIC_TIP_TONE   :    /* Succeed/fail tone */
            case SLIC_DIAL_TONE  :    /* Dial tone  */
            case SLIC_BUSY_TONE  :    /* Busy tone  */
            case SLIC_ALARM_TONE :    /* Alarm tone */
            case SLIC_FAULT_TONE :    /* Fault tone */
            case SLIC_RINGBACK_TONE:  /* Ringback tone */
            case SLIC_CONFIRMATION_TONE: /* Confirmation tone */
            case SLIC_VM_STUTTER_TONE:   /* Voice Mail Stutter Tone */
            case SLIC_CALL_FWD_RMD_TONE: /* Call forward reminder tone */
			{
                ProSLIC_ToneGenStop(ProslicChans[0]);
            }
            break;

            default:
			{
            	pr_info ("slic_sound_stop Error tone type!\n");
            }
            break;
		}
        
        /* Unlock uC interface */
        slic_cmd_mark = SLIC_CMD_UNLOCK;
		
		EnSlicInt();
    }
    else /* uC interface is locked */
    {

	        /*记录当前声音，在slic_sound_stop_delay中使用*/
        slic_sound_stop_tone = tone;

        /*延迟50毫秒后重新尝试停止该声音*/
        slic_dev->slic_snd_stop_delay_timer.function = (void (*)(unsigned long))slic_sound_stop_delay;

        /*check timer is working or not.*/
        if (!timer_pending(&(slic_dev->slic_snd_stop_delay_timer)))
            mod_timer(&(slic_dev->slic_snd_stop_delay_timer), msecs_to_jiffies(50) + jiffies);
    }
}

/*------------------------------------------------------------------------
函数原型:
描述:  将FSK来电显示的数据进行转换，转换为符合标准格式的bit数据
输入:
src:  转换之前的数组
dest: 转换之后的数组
length: 转换之后的数组长度
输出: fsk_byte_index_new
返回值: none
------------------------------------------------------------------------*/
#define DTMF_SIGNAL_LEVEL (slic_customize_ex_parm[g_Country_Type].dtmf_signal_level)
#define osc1amp(x) DTMF_SIGNAL_LEVEL.ocsamp[x].osc1amp
#define osc2amp(x) DTMF_SIGNAL_LEVEL.ocsamp[x].osc2amp

static void slic_dtmf_callerid_show(void)
{
    byte tmp = 0;

    tmp = slic_direct_read(0, LCRRTP);
    if (0x02 == (0x02 & tmp))
    {
        /*单板已经摘机，则退出来电显示发送状态*/
        slic_direct_write   ( 0, OCON,       0x00 );  /*disable the two oscillator*/
        slic_dtmfkey(0x80);
        slic_direct_write   ( 0, IRQEN1,   slic_int_enable1_default_value );
        slic_direct_write   ( 0, IRQEN2,   0x13 );
        slic_cmd_mark = SLIC_CMD_UNLOCK;
        dtmf_callerid_index = 0;
        dtmf_callerid_length = 0;
        return;
    }

    switch (slic_num_reg[dtmf_callerid_index])
    {

        /*修改方案:测量DTMF信号强度幅度过大失真，所以将DTMF信号的幅度降低一倍*/
        /*==========================================
        Col1             Col2          Col3        Col4      Hz        Reg             Ampli
        Line1:    1               2              3            A        697   6d4A000	   3F7000(7EE000)
        Line2:    4               5              6            B        770   694A000	   468000(8D0000)
        Line3:    7               8              9            C        852   6466000	   4E9000(9D2000)
        Line4:    *               0              #           D         941   5E9A000	   578000(AF0000)
        Hz       1209          1336        1477        1633
        Reg     4A80000   3FC4000   331C000     2460000
        Ampli   742000     82C000     941000      A8B000
        (E84000)   (1058000) (1282000) (1516000)
        Every Tone are set with Osc1(Line) and Osc2(Column)
        Wrong number are set with the same tone of "0".
        ============================================*/
    case 0x31:/* 1 */
        /*MSG_HIGH ( "number 1 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
        //slic_indirect_write ( OSC1AMP,   0x3F7000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(1));        

        slic_indirect_write ( OSC2FREQ,  0x4A80000 );
        //slic_indirect_write ( OSC2AMP,   0x742000  );
        slic_indirect_write ( OSC2AMP,   osc2amp(1));
        
        break;

    case 0x32:/* 2 */
        /*MSG_HIGH ( "number 2 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
        //slic_indirect_write ( OSC1AMP,   0x3F7000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(2)); 

        slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
        //slic_indirect_write ( OSC2AMP,   0x82C000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(2)); 

        break;

    case 0x33:/* 3 */
        /* MSG_HIGH ( "number 3 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
        //slic_indirect_write ( OSC1AMP,   0x3F7000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(3)); 

        slic_indirect_write ( OSC2FREQ,  0x331C000 );
        //slic_indirect_write ( OSC2AMP,   0x941000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(3)); 

        break;

    case 0x34:/* 4 */
        /* MSG_HIGH ( "number 4 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x694A000 );
        //slic_indirect_write ( OSC1AMP,   0x468000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(4)); 

        slic_indirect_write ( OSC2FREQ,  0x4A80000 );
        //slic_indirect_write ( OSC2AMP,   0x742000  );
        slic_indirect_write ( OSC2AMP,   osc2amp(4)); 

        break;

    case 0x35:/* 5 */
        /*MSG_HIGH ( "number 5 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x694A000 );
        //slic_indirect_write ( OSC1AMP,   0x468000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(5)); 

        slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
        //slic_indirect_write ( OSC2AMP,   0x82C000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(5)); 

        break;

    case 0x36:/* 6 */
        /*MSG_HIGH ( "number 6 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x694A000 );
        //slic_indirect_write ( OSC1AMP,   0x468000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(6)); 

        slic_indirect_write ( OSC2FREQ,  0x331C000 );
        //slic_indirect_write ( OSC2AMP,   0x941000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(6));

        break;

    case 0x37:/* 7 */
        /*MSG_HIGH ( "number 7 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6466000 );
        //slic_indirect_write ( OSC1AMP,   0x4E9000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(7));

        slic_indirect_write ( OSC2FREQ,  0x4A80000 );
        //slic_indirect_write ( OSC2AMP,   0x742000  );
        slic_indirect_write ( OSC2AMP,   osc2amp(7));

        break;

    case 0x38:/* 8 */
        /*MSG_HIGH ( "number 8 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6466000 );
        //slic_indirect_write ( OSC1AMP,   0x4E9000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(8));

        slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
        //slic_indirect_write ( OSC2AMP,   0x82C000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(8));

        break;

    case 0x39:/* 9 */
        /*MSG_HIGH ( "number 9 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6466000 );
        //slic_indirect_write ( OSC1AMP,   0x4E9000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(9));

        slic_indirect_write ( OSC2FREQ,  0x331C000 );
        //slic_indirect_write ( OSC2AMP,   0x941000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(9));

        break;

    case 0x3a : /* a */
        /*MSG_HIGH ( "number a ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
        //slic_indirect_write ( OSC1AMP,   0x3F7000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(0xa));

        slic_indirect_write ( OSC2FREQ,  0x2460000 );
        //slic_indirect_write ( OSC2AMP,   0xA8B000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(0xa));

        break;

    case 0x3b: /* b */
        /*MSG_HIGH ( "number b ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x694A000 );
        //slic_indirect_write ( OSC1AMP,   0x468000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(0xb));

        slic_indirect_write ( OSC2FREQ,  0x2460000 );
        //slic_indirect_write ( OSC2AMP,   0xA8B000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(0xb));

        break;

    case 0x3c: /* c */
        /*MSG_HIGH ( "number c ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x6466000 );
        //slic_indirect_write ( OSC1AMP,   0x4E9000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(0xc));

        slic_indirect_write ( OSC2FREQ,  0x2460000 );
        //slic_indirect_write ( OSC2AMP,   0xA8B000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(0xc));

        break;

    case 0x3d: /* d */
        /*MSG_HIGH ( "number d ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x5E9A000 );
        //slic_indirect_write ( OSC1AMP,   0x578000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(0xd));

        slic_indirect_write ( OSC2FREQ,  0x2460000 );
        //slic_indirect_write ( OSC2AMP,   0xA8B000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(0xd));

        break;

    case 0x30:/* 0 */
        /*MSG_HIGH ( "number 0 ", 0, 0, 0 );*/
        slic_indirect_write ( OSC1FREQ,  0x5E9A000 );
        //slic_indirect_write ( OSC1AMP,   0x578000  );
        slic_indirect_write ( OSC1AMP,   osc1amp(0));

        slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
        //slic_indirect_write ( OSC2AMP,   0x82C000 );
        slic_indirect_write ( OSC2AMP,   osc2amp(0));

        break;

    default: /* error number, no DTMF output */
        break;
    }
    slic_direct_write   ( 0, OMODE,     0x22);   /*enable the two oscillator transmit to RX*/
    slic_direct_write   ( 0, OCON,      0x11);   /*enable the two oscillator*/

    slic_dev->slic_callerid_delay_timer.function = (void (*)(unsigned long))slic_dtmf_callerid_show_stop;

    /*check timer is working or not.*/
    if (!timer_pending(&(slic_dev->slic_callerid_delay_timer)))
        mod_timer(&(slic_dev->slic_callerid_delay_timer), msecs_to_jiffies(CALLERID_INTERVAL_TIME_MS) + jiffies);
}


/*------------------------------------------------------------------------
函数原型:slic_dtmf_callerid_show_stop
描述:  停止将FSK来电显示的数据进行转换，转换为符合标准格式的bit数据
输入:
src:  转换之前的数组
dest: 转换之后的数组
length: 转换之后的数组长度
输出: fsk_byte_index_new
返回值: none
------------------------------------------------------------------------*/

static void slic_dtmf_callerid_show_stop(void)
{
    byte tmp = 0;

    slic_direct_write   ( 0, OCON,      0x00);   /*disable the two oscillator*/
    slic_direct_write   ( 0, OMODE,     0x00);   /*enable the two oscillator transmit to RX*/

    tmp = slic_direct_read(0, LCRRTP);
    if (0x02 == (0x02 & tmp))
    {
        /*单板已经摘机，则退出来电显示发送状态*/
        slic_direct_write   ( 0, OCON,       0x00 );  /*disable the two oscillator*/
        slic_dtmfkey(0x80);
        slic_direct_write   ( 0, IRQEN1,   slic_int_enable1_default_value );
        slic_direct_write   ( 0, IRQEN2,   0x13 );
        slic_cmd_mark = SLIC_CMD_UNLOCK;
        dtmf_callerid_index = 0;
        dtmf_callerid_length = 0;
        return;
    }
    dtmf_callerid_index++;
    /*if callerid show over, break and clear the variable*/
    if(dtmf_callerid_index < dtmf_callerid_length)
    {
        slic_dev->slic_callerid_delay_timer.function = (void (*)(unsigned long))slic_dtmf_callerid_show;

        /*check timer is working or not.*/
        if (!timer_pending(&(slic_dev->slic_callerid_delay_timer)))
            mod_timer(&(slic_dev->slic_callerid_delay_timer), msecs_to_jiffies(CALLERID_INTERVAL_TIME_MS) + jiffies);
    }
    else
    {
        pr_info( "callerid_show_stop ^^^^^^^" );
        slic_direct_write   ( 0, IRQEN1,   slic_int_enable1_default_value );
        slic_direct_write   ( 0, IRQEN2,   0x13 );
        slic_cmd_mark = SLIC_CMD_UNLOCK;
        dtmf_callerid_index = 0;
        dtmf_callerid_length = 0;
    }
}


/*------------------------------------------------------------------------
函数原型:slic_insert_fsk_bits
描述:  将FSK来电显示的数据进行转换，转换为符合标准格式的bit数据
输入:
src:  转换之前的数组
dest: 转换之后的数组
length: 转换之后的数组长度
输出: fsk_byte_index_new
返回值: none
------------------------------------------------------------------------*/
static int slic_insert_fsk_bits(byte* src,byte* dest,int length)
{
    unsigned char temp = 0;
    int fsk_byte_index_old = 0;
    int fsk_byte_index_new = 0;
    int fsk_bit_index_old = 0;
    int fsk_bit_index_new = 0;

    for(fsk_byte_index_old=0;fsk_byte_index_old<length;fsk_byte_index_old++)
    {
        for(fsk_bit_index_old=0;fsk_bit_index_old<=7;fsk_bit_index_old++)
        {
            /*在一个字节的开头，需要添加bit 0*/
            if(0 == fsk_bit_index_old)
            {
                fsk_bit_index_new++;
                if((0 == fsk_bit_index_new%8)&&(0 != fsk_bit_index_new))
                {
                    fsk_bit_index_new = (fsk_bit_index_new%8);
                    fsk_byte_index_new++;
                }
            }

            temp = src[fsk_byte_index_old]%2;

            if(1 == temp)
            {
                dest[fsk_byte_index_new] = (uInt8)(dest[fsk_byte_index_new]|(temp<<fsk_bit_index_new));
            }
            src[fsk_byte_index_old] = src[fsk_byte_index_old]>>1;

            fsk_bit_index_new++;

            if((0 == fsk_bit_index_new%8)&&(0 != fsk_bit_index_new))
            {
                fsk_bit_index_new = (fsk_bit_index_new%8);
                fsk_byte_index_new++;
            }

            /*在一个字节的结尾，需要添加bit 1*/
            if(7 == fsk_bit_index_old)
            {
                dest[fsk_byte_index_new]= (uInt8)(dest[fsk_byte_index_new]|(1<<fsk_bit_index_new));
                fsk_bit_index_new++;
                if((0 == fsk_bit_index_new%8)&&(0 != fsk_bit_index_new))
                {
                    fsk_bit_index_new = (fsk_bit_index_new%8);
                    fsk_byte_index_new++;
                }
                /*字符串填充已完成，如果目前字符串不能被8整除，则填充bit 1*/
                if(fsk_byte_index_old == length-1)
                {
                    if((0 != fsk_bit_index_new%8))
                    {
                        while(fsk_bit_index_new<8)
                        {
                            dest[fsk_byte_index_new]= (uInt8)(dest[fsk_byte_index_new]|(1<<fsk_bit_index_new));
                            fsk_bit_index_new++;
                        }
                        fsk_byte_index_new++;
                    }
                }
            }
        }
    }
    return fsk_byte_index_new;
}


/*------------------------------------------------------------------------
函数原型:slic_fsk_callerid_show_unlock
描述:  在FSK来显异常情况下，解锁SPI总线,防止出现总线死锁
输入:  NA
输出:  NA
返回值: NA
------------------------------------------------------------------------*/
static void slic_fsk_callerid_show_unlock(void)
{
    slic_cmd_mark = SLIC_CMD_UNLOCK;
    pr_info ( "slic_fsk_callerid_show_unlock!\n" );
}


/*------------------------------------------------------------------------
函数原型:slic_callerid
描述:  来电显示函数
输入:  NA
输出:  NA
返回值: NA
------------------------------------------------------------------------*/
void slic_callerid ( void )
{
    int i = 0;
    int j = 0;
    int k = 0;
    byte callerid_lgth = 0;
    byte date_time_lgth = 0;
    byte month = 0;

    byte tmp = 0;

    slic_fsk_seize_index = 0;
    slic_fsk_body_index = 0;
    callerid_body_lgth = 0;
    callerid_body_lgth_conv = 0;

    memset(slic_num_reg, 0, sizeof(slic_num_reg));
    memset(slic_num_reg_fsk_conv, 0, sizeof(slic_num_reg_fsk_conv));

    /* Information type indicator */
    slic_num_reg[0] = 0x80;

    /* App do pass date and time data to driver: 1 <= month <= 12*/
    month = 10 * (slic_ioc_data_pt->date_time[0] - '0')
            + slic_ioc_data_pt->date_time[1] - '0';
    if((1 <= month) && (12 >= month))
    {
        /* date time lgth and arg type and arg lgth */
        date_time_lgth = 8 + 2;
    }

    while('\0' != slic_ioc_data_pt->slic_num_data[i])
    {
        i++;
    };
    slic_ioc_data_pt->pos = i;

    if ( 0x00 == slic_ioc_data_pt->pos )
    {
        printk("Caller ID Blocked\n");
        /*说明APP侧未收到主叫号码，话机应该显示private,FSK模式下字串为04 01 50*/
        if ((byte)CLIP_DTMF == slic_ioc_data_pt->clipmode)
        {
            /* DTMF模式下暂时直接返回，不显示号码 */
            return;
        }
    }
    else if ( slic_ioc_data_pt->pos <= 32 )
    {
        /* Save caller ID length */
        if(slic_ioc_data_pt->slic_num_data[0] == '+')
        {
            callerid_lgth = slic_ioc_data_pt->pos + 1;
        }
        else
        {
            callerid_lgth = slic_ioc_data_pt->pos;
        }
    }
    else    /* caller ID is too long */
    {
        callerid_lgth = 32;
        pr_info ( "Caller ID is too long!\n" );
    }

    slic_num_reg[1] = (byte)(2 + callerid_lgth + date_time_lgth);

    /* has date and time */
    if( 0 != date_time_lgth)
    {
        slic_num_reg[2] = 0x01;
        slic_num_reg[3] = 0x08;
        for(i = 0; i < 8; i++)
        {
            slic_num_reg[4 + i ] = slic_ioc_data_pt->date_time[i];
        }
    }
    
    slic_num_reg[2 + date_time_lgth] = 0x02;

    if((byte)CLIP_DTMF != slic_ioc_data_pt->clipmode)
    {
        if( TRUE == slic_ioc_data_pt->secret || ( FALSE == slic_ioc_data_pt->secret && 'O' == slic_ioc_data_pt->slic_num_data[0]))
        {
            slic_num_reg[2 + date_time_lgth] = 0x04;
        }
    }

    slic_num_reg[3 + date_time_lgth] = callerid_lgth;

    /* Get Callerid number data */
    if(slic_ioc_data_pt->slic_num_data[0] == '+')
    {
        slic_num_reg[4 + date_time_lgth] = '0';
        slic_num_reg[5 + date_time_lgth] = '0';
        for ( i=1; i<callerid_lgth; i++)
        {
            slic_num_reg[5+i + date_time_lgth] = slic_ioc_data_pt->slic_num_data[i];
        }
        callerid_body_lgth = 5 + callerid_lgth + date_time_lgth;
    }
    else
    {
        for ( i=0; i<callerid_lgth; i++)
        {
            /* Translate ASCII code to Proslic code */
            slic_num_reg[4+i + date_time_lgth] = slic_ioc_data_pt->slic_num_data[i];
        }
        callerid_body_lgth = 4 + callerid_lgth + date_time_lgth;
    }
    /* Checksum */
    slic_num_reg[4+callerid_lgth + date_time_lgth] = 0x00;
    for ( i=0; i<(4+callerid_lgth + date_time_lgth); i++ )
    {
        slic_num_reg[4+callerid_lgth + date_time_lgth]
            = slic_num_reg[4+callerid_lgth + date_time_lgth] + slic_num_reg[i];
    }

    slic_num_reg[4+callerid_lgth + date_time_lgth] = (byte)(-slic_num_reg[4+callerid_lgth + date_time_lgth]);
    callerid_body_lgth += 1;

    tmp = slic_direct_read(0, LCRRTP);
    if (0x02 == (0x02 & tmp))
    {
        /*在来电显示之前摘机则不显示来电号码*/
        return;
    }

    if ((byte)CLIP_DTMF == slic_ioc_data_pt->clipmode)
    {
        if ( SLIC_CMD_UNLOCK == slic_cmd_mark ) /* uC interface is free */
        {
            slic_cmd_mark = SLIC_CMD_LOCK;
             if(1 == g_Equip_Test)
            {
                slic_cmd_mark = SLIC_CMD_UNLOCK;
            }
            if(TWO_RING == four_ring_cadence)
            {
                slic_direct_write   ( 0, IRQEN1,   0x00 );
            }
            else
            {
                slic_direct_write   ( 0, IRQEN1,   0x20 );
            }
            slic_direct_write   ( 0, IRQEN2,   0x00 );
            slic_direct_write   ( 0, IRQEN3,   0x00 );
            /*修改来电显示方法:在原有的发送序列前发送D、结束发送C的DTMF音*/
            if(FALSE == slic_ioc_data_pt->secret)
            {
                //slic_num_reg[3] = 0x3d;
                slic_num_reg[3 + date_time_lgth] = slic_customize_ex_parm[g_Country_Type].callid_first_signal;
            }
            else
            {
                slic_num_reg[3 + date_time_lgth] = 0x3b;
            }
            slic_num_reg[4+callerid_lgth + date_time_lgth] = 0x3c;

            dtmf_callerid_index = 3 + date_time_lgth;
            dtmf_callerid_length = 4+callerid_lgth+1+date_time_lgth; //callerid length

            switch (slic_num_reg[dtmf_callerid_index])
            {
                /*================================================================================================
                Col1             Col2          Col3        Col4      Hz        Reg             Ampli
                Line1:    1               2              3            A        697   6d4A000   3F7000(7EE000)
                Line2:    4               5              6            B        770   694A000   468000(8D0000)
                Line3:    7               8              9            C        852   6466000   4E9000(9D2000)
                Line4:    *               0              #           D         941   5E9A000   578000(AF0000)
                Hz       1209          1336        1477        1633
                Reg     4A80000   3FC4000   331C000     2460000
                Ampli   742000     82C000     941000      A8B000
                (E84000)   (1058000) (1282000) (1516000)
                Every Tone are set with Osc1(Line) and Osc2(Column)
                Wrong number are set with the same tone of "0".
                ===================================================================================================*/
            case 0x31:/* 1 */
                /*MSG_HIGH ( "number 1 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
                //slic_indirect_write ( OSC1AMP,   0x3F7000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(1));        

                slic_indirect_write ( OSC2FREQ,  0x4A80000 );
                //slic_indirect_write ( OSC2AMP,   0x742000  );
                slic_indirect_write ( OSC2AMP,   osc2amp(1));

                break;

            case 0x32:/* 2 */
                /*MSG_HIGH ( "number 2 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
                //slic_indirect_write ( OSC1AMP,   0x3F7000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(2)); 

                slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
                //slic_indirect_write ( OSC2AMP,   0x82C000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(2)); 

                break;

            case 0x33:/* 3 */
                /* MSG_HIGH ( "number 3 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
                //slic_indirect_write ( OSC1AMP,   0x3F7000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(3)); 

                slic_indirect_write ( OSC2FREQ,  0x331C000 );
                //slic_indirect_write ( OSC2AMP,   0x941000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(3)); 

                break;

            case 0x34:/* 4 */
                /* MSG_HIGH ( "number 4 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x694A000 );
                //slic_indirect_write ( OSC1AMP,   0x468000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(4)); 

                slic_indirect_write ( OSC2FREQ,  0x4A80000 );
                //slic_indirect_write ( OSC2AMP,   0x742000  );
                slic_indirect_write ( OSC2AMP,   osc2amp(4)); 

                break;

            case 0x35:/* 5 */
                /*MSG_HIGH ( "number 5 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x694A000 );
                //slic_indirect_write ( OSC1AMP,   0x468000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(5)); 

                slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
                //slic_indirect_write ( OSC2AMP,   0x82C000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(5)); 

                break;

            case 0x36:/* 6 */
                /*MSG_HIGH ( "number 6 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x694A000 );
                //slic_indirect_write ( OSC1AMP,   0x468000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(6)); 

                slic_indirect_write ( OSC2FREQ,  0x331C000 );
                //slic_indirect_write ( OSC2AMP,   0x941000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(6));

                break;

            case 0x37:/* 7 */
                /*MSG_HIGH ( "number 7 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6466000 );
                //slic_indirect_write ( OSC1AMP,   0x4E9000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(7));

                slic_indirect_write ( OSC2FREQ,  0x4A80000 );
                //slic_indirect_write ( OSC2AMP,   0x742000  );
                slic_indirect_write ( OSC2AMP,   osc2amp(7));

                break;

            case 0x38:/* 8 */
                /*MSG_HIGH ( "number 8 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6466000 );
                //slic_indirect_write ( OSC1AMP,   0x4E9000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(8));

                slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
                //slic_indirect_write ( OSC2AMP,   0x82C000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(8));

                break;

            case 0x39:/* 9 */
                /*MSG_HIGH ( "number 9 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6466000 );
                //slic_indirect_write ( OSC1AMP,   0x4E9000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(9));

                slic_indirect_write ( OSC2FREQ,  0x331C000 );
                //slic_indirect_write ( OSC2AMP,   0x941000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(9));

                break;

            case 0x3a : /* a */
                /*MSG_HIGH ( "number a ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6D4A000 );
                //slic_indirect_write ( OSC1AMP,   0x3F7000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(0xa));

                slic_indirect_write ( OSC2FREQ,  0x2460000 );
                //slic_indirect_write ( OSC2AMP,   0xA8B000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(0xa));

                break;

            case 0x3b: /* b */
                /*MSG_HIGH ( "number b ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x694A000 );
                //slic_indirect_write ( OSC1AMP,   0x468000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(0xb));

                slic_indirect_write ( OSC2FREQ,  0x2460000 );
                //slic_indirect_write ( OSC2AMP,   0xA8B000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(0xb));

                break;

            case 0x3c: /* c */
                /*MSG_HIGH ( "number c ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x6466000 );
                //slic_indirect_write ( OSC1AMP,   0x4E9000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(0xc));

                slic_indirect_write ( OSC2FREQ,  0x2460000 );
                //slic_indirect_write ( OSC2AMP,   0xA8B000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(0xc));

                break;

            case 0x3d: /* d */
                /*MSG_HIGH ( "number d ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x5E9A000 );
                //slic_indirect_write ( OSC1AMP,   0x578000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(0xd));

                slic_indirect_write ( OSC2FREQ,  0x2460000 );
                //slic_indirect_write ( OSC2AMP,   0xA8B000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(0xd));

                break;

            case 0x30:/* 0 */
                /*MSG_HIGH ( "number 0 ", 0, 0, 0 );*/
                slic_indirect_write ( OSC1FREQ,  0x5E9A000 );
                //slic_indirect_write ( OSC1AMP,   0x578000  );
                slic_indirect_write ( OSC1AMP,   osc1amp(0));

                slic_indirect_write ( OSC2FREQ,  0x3FC4000 );
                //slic_indirect_write ( OSC2AMP,   0x82C000 );
                slic_indirect_write ( OSC2AMP,   osc2amp(0));

                break;

                default: /* error number, no DTMF output */
                    pr_info ("DTMF callerid show failed,error number, no DTMF output!\n");
                    break;
            }

            slic_direct_write   ( 0, OMODE,     0x22);   /*enable the two oscillator transmit to RX*/
            slic_direct_write   ( 0, OCON,      0x11);  /*enable the two oscillator*/

            slic_dev->slic_callerid_delay_timer.function = (void (*)(unsigned long))slic_dtmf_callerid_show_stop;

            /*check timer is working or not.*/
            if (!timer_pending(&(slic_dev->slic_callerid_delay_timer)))
                mod_timer(&(slic_dev->slic_callerid_delay_timer), msecs_to_jiffies(CALLERID_INTERVAL_TIME_MS) + jiffies);
        }
        else    /* uC interface is locked */
        {
            pr_info ("DTMF callerid show failed,interface is locked!\n");
        }
    }
    else
    {
        INTLOCK();
        /* Write caller ID to Proslic */
        if ( SLIC_CMD_UNLOCK == slic_cmd_mark ) /* uC interface is free */
        {
            /* unlock uC interface for interrupt operation */
            slic_cmd_mark = SLIC_CMD_LOCK;
             if(1 == g_Equip_Test)
            {
                slic_cmd_mark = SLIC_CMD_UNLOCK;
            }

            /* Fsk initialize set */
            slic_direct_write   ( 0, O1TALO,       0x13 );
            slic_direct_write   ( 0, O1TAHI,       0x00 );
            slic_direct_write   ( 0, O1TILO,       0x00 );
            slic_direct_write   ( 0, O1TIHI,       0x00 );

            if ((byte)CLIP_FSK_BELL == slic_ioc_data_pt->clipmode)//BELL_CORE 0: 2200Hz; 1: 1200Hz amplitude: 0.22v
            {
                slic_indirect_write ( FSKAMP0,   0x003C0000 );
                slic_indirect_write ( FSKAMP1 ,  0x00202000 );
                slic_indirect_write ( FSKFREQ0,  0x06B5A000 );
                slic_indirect_write ( FSKFREQ1,  0x079BC000 );
                slic_indirect_write ( FSK01,     0x02238000 );
                slic_indirect_write ( FSK10,     0x077AE000 );
            }
            else//ITU  0: 2100Hz; 1: 1300Hz amplitude: 0.22v
            {
                slic_indirect_write ( FSKAMP0,   0x00392000 );
                slic_indirect_write ( FSKAMP1 ,  0x0022E000 );
                slic_indirect_write ( FSKFREQ0,  0x06D24000 );
                slic_indirect_write ( FSKFREQ1,  0x078A8000 );
                slic_indirect_write ( FSK01,     0x02700000 );
                slic_indirect_write ( FSK10,     0x06904000 );
            }

            /*<kyj add the seize signal 300 "01010101..............01" and mark signal 180 "11111.....111" begin*/
            for(k=0; k<60; k++)
            {
                if(k < 37)
                {
                    slic_fsk_seize_num_reg[k] = 0xaa;
                }
                else if(37 == k)
                {
                    slic_fsk_seize_num_reg[k] = 0xfa;
                }
                else
                {
                    slic_fsk_seize_num_reg[k] = 0xff;
                }
            }
            /* add the seize signal 300 "01010101..............01" and mark signal 180 "11111.....111" */
            if ( 0x00 == slic_ioc_data_pt->pos )
            {
                /* Caller ID Blocked */
                memset(slic_num_reg, 0, sizeof(slic_num_reg));
                slic_num_reg[0] = 0x80; /* Msg Type */
                slic_num_reg[1] = 0x03; /* Msg Length */
                slic_num_reg[2] = 0x04; /* Parameter Type */
                slic_num_reg[3] = 0x01; /* Parameter Length */
                slic_num_reg[4] = 0x50; /* Parameter Content: Private */
                slic_num_reg[5] = 0x28; /* Checksum */
                callerid_body_lgth = 6;
            }
            callerid_body_lgth_conv = slic_insert_fsk_bits(slic_num_reg,slic_num_reg_fsk_conv, callerid_body_lgth);
            slic_num_reg_fsk_conv[callerid_body_lgth_conv] = 0xFF;//in the end continue send "111111111"

            //slic_spi_to_sync();
            slic_direct_write   ( 0, FSKDEPTH, 0x05 ); /* 每发6bytes FIFO空，产生一个中断 */

            for(j=0; j<6; j++)
            {
                slic_direct_write   ( 0, FSKDAT, slic_fsk_seize_num_reg[slic_fsk_seize_index]);
                slic_fsk_seize_index++;
            }

            slic_fsk_flag = TRUE;

            if(TWO_RING == four_ring_cadence)
            {
                slic_direct_write   ( 0, IRQEN1,   0x40 );
            }
            else
            {
                slic_direct_write   ( 0, IRQEN1,   0x60 );
            }
            slic_direct_write   ( 0, IRQEN2, 0x00 );
            slic_direct_write   ( 0, OMODE, 0x8A );
            slic_direct_write   ( 0, OCON,   0x05 );
            //slic_spi_to_sync();
            slic_dev->slic_callerid_fsk_delay_timer.function = (void (*)(unsigned long))slic_fsk_callerid_show_unlock;
            /*check timer is working or not，从此处到第一次进中断的时间是15ms，此处取100ms回调.*/
            if (!timer_pending(&(slic_dev->slic_callerid_fsk_delay_timer)))
                mod_timer(&(slic_dev->slic_callerid_fsk_delay_timer), msecs_to_jiffies(CALLERID_FSK_DELAY_MS) + jiffies);
        }
        INTFREE();
    }
}

/*------------------------------------------------------------------------
函数原型:si32178_read
描述: 应用层读接口
输入: 应用层读命令
输出: 返回读得得值给应用层
返回值: 0--成功
------------------------------------------------------------------------*/
static ssize_t si32178_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    int ret = 0;
    int channel=0;
    int req_size = 0;
    struct si32178_data   *slicdev  = NULL;
    struct slic_pcm *pcmdev = NULL;
    channel = MNUM(filp->f_path.dentry->d_inode);
    /* slicdev = file_p->private_data; */
    slicdev  = slic_dev;

    if(NULL == slicdev)
    {
        pr_err("WARING:slicdev is not probed.\r\n");
        return -EAGAIN;
    }
    pcmdev = slic_dev->slic_pcmdev;
    if(NULL == pcmdev)
    {
        printk("the pcmdev is NULL\n");
        return 0;
    }
    if(0 != channel)
    {
        channel =0;
    }
    init_waitqueue_head(&slicdev->read_q);

    if (slicdev->inread)
    {
        return -EALREADY;
    }

    slicdev->inread = 1;

    set_current_state(TASK_INTERRUPTIBLE);
    mb();
 
    /* Start PCM channel if not started already - if device is opened in READ ONLY mode */
    if (((filp->f_flags & 0x3) == O_RDONLY))
    {
        if(0 == pcmdev->pcm_status[channel])
        {
            pr_err("WARING:slicdev is not probed.\r\n");
            slic_pcm_start(0);
        }
    }       
   
    pcmdev->read_data[channel].xfer_start = 1;
    req_size =  min(count, (size_t)(slicdev->read_buffer_size));

    pcmdev->read_data[channel].req_size = req_size;

    /* Don't wait for event if non-block flag is set */
    if (!(filp->f_flags & O_NONBLOCK))
    {
        slicdev->read_blocking = 1;   /* Set blocking flag when waiting for event */
   
        if(pcmdev->pcm_status[channel] == 1)
        {
            /* dbg_print("Ch%d READ WAIT, data_ready:%d \n", channel, pcmdev->read_data[channel].data_ready); */
            wait_event_interruptible(slicdev->read_q, (pcmdev->read_data[channel].data_ready == 1));
            pcmdev->read_data[channel].data_ready = 0;
        }
        else
        {
            slicdev->inread = 0;
            set_current_state(TASK_RUNNING);
            return -EAGAIN;
        }
    }
    else
    {
        slicdev->read_blocking = 0;   /* Reset blocking flag when not waiting for event */

        /* dbg_print("Device READ opened in O_NONBLOCK mode \n"); */
        if(pcmdev->read_data[channel].data_ready != 1)
        {
            slicdev->inread = 0;
            set_current_state(TASK_RUNNING);
            return -EAGAIN;
        }
    }
   
    /* Check for pending SIGNAL */
    if (signal_pending(current)) 
    {
        set_current_state(TASK_RUNNING);
        slicdev->inread = 0;
        return -EINTR;
    }

    set_current_state(TASK_RUNNING);
   
    ret = slic_pcm_read(channel, readbuf, req_size);

    /* Don't ever copy more than what the user asks */
    ret = copy_to_user(buf, readbuf, req_size);
    if (ret)
    {
        slicdev->inread = 0;
        return -EFAULT;
    } 
    else
    {
        slicdev->inread = 0;
        return req_size;
    }

    return 0;
}
/*------------------------------------------------------------------------
函数原型: si32178_write
描述: 应用层写接口
输入: 来自应用层的写接口和数据
输出: 将应用层数据写到内核
返回值: 0--成功
------------------------------------------------------------------------*/
static ssize_t si32178_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    int ret = 0;
    int channel=0;
    int req_size = 0;
    struct si32178_data *slicdev = NULL; 
    struct slic_pcm *pcmdev = NULL;

    channel = MNUM(filp->f_path.dentry->d_inode);
    /* slicdev = file_p->private_data; */
    slicdev  = slic_dev;
    pcmdev = slicdev->slic_pcmdev;

    if(NULL == slicdev)
    {
        pr_err("WARING:slicdev is not probed.\r\n");
        return -EAGAIN;
    }

    if(0 != channel)
    {
        //pr_info("WARING:other channel are not supported.\r\n");
        channel =0;
    }
    /* dbg_print("\nslic_slic:: write() for ch:%d size:%d \n", channel, count); */
    init_waitqueue_head(&slicdev->write_q);

    if (slicdev->inwrite)
    {
        return -EALREADY;
    }

    slicdev->inwrite = 1;

    set_current_state(TASK_INTERRUPTIBLE);
    mb();

    /* Start PCM channel if not started already - if device is opened in WRITE ONLY mode */
    if (((filp->f_flags & 0x3) == O_WRONLY))
    {
        if(0 == pcmdev->pcm_status[channel])
        {
            pr_err("WARING:slicdev is not probed.\r\n");
            slic_pcm_start(0);
        }
    }      
   
    pcmdev->write_data[channel].xfer_start = 1;
    req_size =  min(count, (size_t)(slicdev->write_buffer_size));
    pcmdev->write_data[channel].req_size = req_size;

    /* Don't wait for event if non-block flag is set */
    if (!(filp->f_flags & O_NONBLOCK))
    {
        slicdev->write_blocking = 1;   /* Set blocking flag when waiting for event */

        if(pcmdev->pcm_status[channel] == 1)
        {
            /* dbg_print("Ch%d WRITE WAIT, data_ready:%d \n", channel, pcmdev->write_data[channel].data_ready); */
            wait_event_interruptible(slicdev->write_q, (pcmdev->write_data[channel].data_ready == 1));
            pcmdev->write_data[channel].data_ready = 0;
        }
        else
        {
            slicdev->inwrite = 0;
            set_current_state(TASK_RUNNING);
            return -EAGAIN;
        }
    }
    else
    {
        slicdev->write_blocking = 0;   /* Reset blocking flag when not waiting for event */

        /* dbg_print("Device WRITE opened in O_NONBLOCK mode \n");  */
        if(pcmdev->write_data[channel].data_ready != 1)
        {
            slicdev->inwrite = 0;
            set_current_state(TASK_RUNNING);
            return -EAGAIN;
        }
    }
    
    /* Check for pending SIGNAL */
    if (signal_pending(current)) 
    {
        set_current_state(TASK_RUNNING);
        slicdev->inwrite = 0;
        return -EINTR;
    }
    set_current_state(TASK_RUNNING);

    ret = copy_from_user(writebuf, buf, req_size);
    if (ret) 
    {
        printk("copy_from_user failed\n");
        slicdev->inwrite = 0;
        return -EFAULT;
    }
    ret = slic_pcm_write(channel, writebuf, req_size);
    slicdev->inwrite = 0;
    return ret;
}
/*------------------------------------------------------------------------
函数原型:si32178_open
描述: linux系统调用，打开设备
输入: 应用层.open命令
输出: 无
返回值: 0--成功
------------------------------------------------------------------------*/
static int si32178_open(struct inode *inode, struct file *file)
{
    return nonseekable_open(inode, file);
}

/*------------------------------------------------------------------------
函数原型:si32178_release
描述: linux系统调用，释放设备
输入: 系统.release调用
输出: 无
返回值: 0--成功
------------------------------------------------------------------------*/
static int si32178_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/*------------------------------------------------------------------------
函数原型:si32178_ioctl
描述: 应用ioctl调用接口
输入: linux系统参数
输出:  ioctl操作
返回值:0-成功
------------------------------------------------------------------------*/
static long si32178_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *) arg;
    byte tmp[2] = {0};
    //int lHookStatus = 0;
    byte HookStatus = 0;

    //EnableSlicIrq( 0 ); //关闭SLIC中断

    switch (cmd)
    {
    case SLIC_IOC_SOUND_START :
#ifdef CONFIG_SLIC_TEST
        pr_info("---------------SLIC_IOC_SOUND_START--------------!\n");
#endif

        if (copy_from_user(slic_ioc_data_pt, argp, sizeof(struct slic_ioc_data)))
        {
            pr_info ( "si32178_ioctl copy_from_user error1!\n");
            goto err_here;
        }
        slic_sound_start((slic_tone_enum_type)(slic_ioc_data_pt->tone));
        break;

    case SLIC_IOC_SOUND_STOP :
#ifdef CONFIG_SLIC_TEST
        pr_info("---------------SLIC_IOC_SOUND_STOP--------------!\n");
#endif

        if (copy_from_user(slic_ioc_data_pt, argp, sizeof(struct slic_ioc_data)))
        {
            pr_info ( "si32178_ioctl copy_from_user error2!\n");
            goto err_here;
        }
        slic_sound_stop((slic_tone_enum_type)(slic_ioc_data_pt->tone));
        break;
    case SLIC_IOC_PCM_MODE:
        if (copy_from_user(slic_ioc_data_pt, argp, sizeof(struct slic_ioc_data)))
        {
            pr_info ( "si32178_ioctl copy_from_user error1!\n");
            goto err_here;
        }
        break;
    case SLIC_IOC_HOOK_STATUS:
		{
			ProSLIC_ReadHookStatus(ProslicChans[0], &HookStatus);
        	if(copy_to_user( (void*)argp, &HookStatus, sizeof(HookStatus) ))
            {
                goto err_here;
            }
    	}
        break;
    case SLIC_IOC_TEST_WRITE :
        if (copy_from_user(tmp, argp, sizeof(tmp)))
        {
            pr_info ( "si32178_ioctl copy_from_user error3!\n");
            goto err_here;
        }
        slic_direct_write(0, tmp[0], tmp[1]);
        break;

    case SLIC_IOC_TEST_READ :
        if (copy_from_user(tmp, argp, sizeof(tmp)))
        {
            pr_info ( "si32178_ioctl copy_from_user error4!\n");
            goto err_here;
        }

        tmp[1] = slic_direct_read(0, tmp[0]);

        if (copy_to_user(argp, tmp, sizeof(tmp)))
        {
            goto err_here;
        }
        break;
        case SLIC_IOC_MEDIA_START:
            printk("start voip dma......\n");
            MED_PCM_Ctrl_Start();
            break;
            
        case SLIC_IOC_MEDIA_STOP:
            printk("stop voip dma......\n");
            MED_PCM_Ctrl_Stop();
            break;
    default:
        pr_info("si32178_ioctl error cmd!\n");
        break;
    }
    return 0;
err_here:
    //EnableSlicIrq( 1 ); //SLIC中断使能
    return -DEFAULT;
}

/* 文件系统结构体 */
static const struct file_operations si32178_fops = {
    .owner = THIS_MODULE,
    .open = si32178_open,
    .release = si32178_release,
    .read = si32178_read,
    .write = si32178_write,
    .unlocked_ioctl = si32178_ioctl,
};

#if 0
static const struct spi_device_id si32178_id[] = {
    {"si32178", 0},
    {}
};

/* 驱动结构体 */
static struct spi_driver si32178_driver = {
    .driver = {
        .name = "si32178",
        .bus  = &spi_bus_type,
        .owner = THIS_MODULE,
    },
    .probe = si32178_probe,
    .remove = __devexit_p(si32178_remove),
    .id_table = si32178_id,
};
#endif

static struct platform_device si32178_platform_device = {
    .name = "si32178",
    .id = -1,
};

static struct platform_driver si32178_driver = {
    .probe = si32178_probe,
    .remove = __devexit_p(si32178_remove),
    .driver = {
        .name = "si32178",
        .owner = THIS_MODULE,
    },
};

#define MAX_SI32178_NUM 1
static struct platform_device *si32178_platform_devices[MAX_SI32178_NUM] = {
    &si32178_platform_device,
};


/* 杂项设备结构体 */
static struct miscdevice si32178_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "si32178_dev",
    .fops = &si32178_fops,
};
void slic_test_timer_cb(void)
{
    uInt8 Reg0;
    
    Reg0 = slic_direct_read (0, 0);
        slic_debug("enter slic_test_timer_cb, reg0 = 0x%X", Reg0);
    mod_timer(&(slic_dev->slic_test_timer), msecs_to_jiffies(1000) + jiffies);
}
/*------------------------------------------------------------------------
函数原型:si32178_probe
描述: SLIC驱动探测
输入: 设备指针
输出: 成功注册设备和驱动
返回值: 0-成功
------------------------------------------------------------------------*/
static int __devinit  si32178_probe(struct platform_device *pdev)
{
    int err = 0;
    int i = 0;
    struct slic_pcm *pcmdev = (struct slic_pcm *)&slic_pcmdev;
    struct input_dev *input_dev = NULL;

#ifdef CONFIG_SLIC_TEST
    byte num_test_buf[] = {'1','3','8','8','2','1','2','3','5','9','1','\0'};
    byte dt_test_buf[] = {'0','4','0','9','1','4','3','2', '\0'};
#endif

    pr_info("si32178_probe start!\n");

    get_countryType();
    printk("g_Country_Type = %d\n", g_Country_Type);

    if(NULL == pdev) {
        pr_info("slic platform_device is null!\n");
        err = -EINVAL;
        goto  exit_platform_device_null;
    }
    /*管脚配置*/
    slic_gpio_config();

    if(OK != SlicSpiInit())
    {
        pr_info("slic spi init fail\n");
        err = -ENODEV;
        goto exit_slic_spi_init_failed;
    }

    slic_dev = kzalloc(sizeof(struct si32178_data), GFP_KERNEL);
    if (!slic_dev) {
        pr_info("failed to allocate memory for module exit_alloc_slic_dev_failed\n");
        err = -ENOMEM;
        goto exit_alloc_slic_dev_failed;
    }

    slic_ioc_data_pt = kzalloc(sizeof(struct slic_ioc_data), GFP_KERNEL);
    if (!slic_ioc_data_pt) {
        pr_info("failed to allocate memory for slic_ioc_data_pt!\n");
        err = -ENOMEM;
        goto exit_alloc_slic_ioc_data_pt_failed;
    }

#ifdef CONFIG_SLIC_TEST
    /* 临时测试功能用,实际参数，应用层通过IOCTL传入 */
    slic_ioc_data_pt->clipmode = CLIP_FSK_BELL;
    slic_ioc_data_pt->pos = 11;
    strcpy(slic_ioc_data_pt->slic_num_data, num_test_buf);
    strcpy(slic_ioc_data_pt->date_time, dt_test_buf);
    slic_ioc_data_pt->secret = FALSE;
    slic_ioc_data_pt->tone = SLIC_RING;
#endif

    input_dev = input_allocate_device();
    if (!input_dev)
    {
        pr_info("input device allocate failed!\n");
        err = -ENOMEM;
        goto err_alloc_input_device;
    }

    input_dev->name = pdev->name;
    input_dev->id.bustype = BUS_HOST;
    input_dev->dev.parent = &pdev->dev;

    /* Add the keycode */
    input_dev->keycode = slic_dev->keycodes;
    input_dev->keycodesize = sizeof(slic_dev->keycodes[0]);
    input_dev->keycodemax = ARRAY_SIZE(slic_keycodes);

    set_bit(EV_KEY, input_dev->evbit);

    for (i = 0; i < ARRAY_SIZE(slic_keycodes); i++)
    {
        slic_dev->keycodes[i] = slic_keycodes[i];
        set_bit(slic_keycodes[i], input_dev->keybit);
    }

    slic_dev->input_dev = input_dev;

    /* get slic irq */
    slic_dev->slic_irq = HI6920_GPIO0_IRQ;
    if (slic_dev->slic_irq < 0)
    {
        dev_err(&pdev->dev, "Failed to get slic irq!\n");
        err = slic_dev->slic_irq;
        goto err_get_slic_irq;
    }

    /* Initialize spin lock */
    spin_lock_init(&slic_dev->lock);

    slic_dev->slic_pcmdev = pcmdev;

    slic_dev->read_blocking = 0;
    slic_dev->write_blocking = 0;
    slic_dev->read_buffer_size = SLIC_READ_BUFSIZE;
    slic_dev->write_buffer_size = SLIC_WRITE_BUFSIZE;

    init_waitqueue_head(&slic_dev->read_q);
    init_waitqueue_head(&slic_dev->write_q);


    /* Initialize PCM state variables */
    slic_pcm_start(0);

    
    err = input_register_device(slic_dev->input_dev);
    if (err) {
        pr_info("Failed to register input device!\n");
        goto err_register_input_device;
    }

    err = misc_register(&si32178_device);
    if (err) {
        pr_info("si32178_probe: si32178_device register failed\n");
        goto exit_misc_device_register_failed;
    }

    /*initial timer before we use it.*/
    init_timer(&(slic_dev->slic_int_delay_timer));
    init_timer(&(slic_dev->slic_callerid_delay_timer));
    init_timer(&(slic_dev->slic_snd_start_delay_timer));
    init_timer(&(slic_dev->slic_snd_stop_delay_timer));
    init_timer(&(slic_dev->slic_callerid_fsk_delay_timer));
    init_timer(&(slic_dev->slic_rkey_delay_timer));    
    init_timer(&(slic_dev->slic_pcm_mode_timer));
#ifdef FEATURE_HUAWEI_CS_INBAND_DTMF_ELIMINATE
    init_timer(&(slic_dev->slic_pcm_unmute_timer));
#endif
#ifdef FEATURE_HUAWEI_DTMF_FLUCTUATE_ELIMINATE
    init_timer(&(slic_dev->slic_dtmf_valid_timer));
#endif
    init_timer(&(slic_dev->slic_test_timer));

    /*slic芯片初始化*/
	slicctl_proslic_init();
	
    slicctl_chinit(0);

    /* clear all interrupts during chip initialization */
    SLIC_GPIO_REG_SETBIT(g_gpioBase, HI_GPIO_PORT_EOI, 22);
    slic_direct_read (0,IRQ1);
    slic_direct_read (0,IRQ2);
    slic_direct_read (0,IRQ3);

    /* install interrupt handler after chip init finished */
#ifdef E5172_SLIC_WAKEUP_DEBUG
    err = request_irq(slic_dev->slic_irq, slic_irq_handler, IRQF_SHARED, "si32178 interrupt", &si32178_platform_device);
#else
    err = request_irq(slic_dev->slic_irq, slic_irq_handler, IRQF_NO_SUSPEND | IRQF_SHARED, "si32178 interrupt", &si32178_platform_device);
#endif    
    if (err) {
        pr_info("Failed to request interupt handler!\n");
        goto err_request_irq;
    }
    pr_info( "after request_irq()\n" );

#ifdef CONFIG_SLIC_TEST
    pr_info("now create_si3217x_proc_file here\n");
    create_si3217x_proc_file();
#endif

    pr_info("slic driver probe ok!\n");

    //msleep(1000);

    //slic_direct_write (0, 30, 4);

    //slic_read_registers();

    //slic_dev->slic_test_timer.function = (void (*)(unsigned long))slic_test_timer_cb;
    //mod_timer(&(slic_dev->slic_test_timer), msecs_to_jiffies(1000) + jiffies);

    return 0;

exit_misc_device_register_failed:
    input_unregister_device(slic_dev->input_dev);
    slic_dev->input_dev = NULL;
err_register_input_device:
    free_irq(slic_dev->slic_irq,slic_dev);
err_request_irq:
err_get_slic_irq:
    input_free_device(slic_dev->input_dev);
    slic_dev->input_dev = NULL;
err_alloc_input_device:
    kfree(slic_ioc_data_pt);
    slic_ioc_data_pt = NULL;
exit_alloc_slic_ioc_data_pt_failed:
    kfree(slic_dev);
    slic_dev = NULL;
exit_alloc_slic_dev_failed:
exit_platform_device_null:
exit_slic_spi_init_failed:
    return err;
}

#ifdef CONFIG_SLIC_TEST
void slic_set_clip_mode(int mode)
{
    if(0 == mode)
    {
        slic_ioc_data_pt->clipmode = CLIP_FSK_BELL;
    }
    else
    {
        slic_ioc_data_pt->clipmode = CLIP_DTMF;
    }
}
#endif

/*------------------------------------------------------------------------
函数原型:si32178_remove
描述: SLIC模块移除
输入: 设备指针
输出: 移除设备分配的内存和支持
返回值: 0-成功
------------------------------------------------------------------------*/
static int __devexit si32178_remove(struct platform_device *pdev)
{
    pr_info("si32178_remove\n");
    free_irq(slic_dev->slic_irq,slic_dev);
    input_unregister_device(slic_dev->input_dev);
    misc_deregister(&si32178_device);
    kfree(slic_dev);
    kfree(slic_ioc_data_pt);
    return 0;
}

/*=======================================test code======================================*/
#ifdef CONFIG_SLIC_TEST
#define SI3217X_PROC_FILE "driver/si32178"
static struct proc_dir_entry *g_si3217x_proc_file = NULL;

static void slic_change_clipmode(void)
{
    if(slic_ioc_data_pt->clipmode == CLIP_DTMF)
    {
        slic_ioc_data_pt->clipmode = CLIP_FSK_ITU;
        pr_info("clipmode = CLIP_FSK_ITU\n");
    }
    else if(slic_ioc_data_pt->clipmode == CLIP_FSK_ITU)
    {
        slic_ioc_data_pt->clipmode = CLIP_FSK_BELL;
        pr_info("clipmode = CLIP_FSK_BELL\n");
    }
    else if(slic_ioc_data_pt->clipmode == CLIP_FSK_BELL)
    {
        slic_ioc_data_pt->clipmode = CLIP_FSK_RPAS;
        pr_info("clipmode = CLIP_FSK_RPAS\n");
    }
    else if(slic_ioc_data_pt->clipmode == CLIP_FSK_RPAS)
    {
        slic_ioc_data_pt->clipmode = CLIP_DTMF;
        pr_info("clipmode = CLIP_DTMF\n");
    }
    else
    {
        /* do nothing */
    }
}

static void slic_secret_set(void)
{
    if(slic_ioc_data_pt->secret == TRUE)
    {
        slic_ioc_data_pt->secret = FALSE;
        pr_info("secret = FALSE\n");
    }
    else
    {
        slic_ioc_data_pt->secret = TRUE;
        pr_info("secret = TRUE\n");
    }
}

static void slic_change_tonetype(void)
{
    switch(slic_ioc_data_pt->tone)
    {
    case SLIC_NONE_TONE:
        slic_ioc_data_pt->tone = SLIC_RING;
        pr_info("tone = SLIC_RING\n");
        break;

    case SLIC_RING:
        slic_ioc_data_pt->tone = SLIC_KEEP_RING;
        pr_info("tone = SLIC_KEEP_RING\n");
        break;

    case SLIC_KEEP_RING:
        slic_ioc_data_pt->tone = SLIC_DIAL_TONE;
        pr_info("tone = SLIC_DIAL_TONE\n");
        break;

    case SLIC_DIAL_TONE:
        slic_ioc_data_pt->tone = SLIC_BUSY_TONE;
        pr_info("tone = SLIC_BUSY_TONE\n");
        break;

    case SLIC_BUSY_TONE:
        slic_ioc_data_pt->tone = SLIC_ALARM_TONE;
        pr_info("tone = SLIC_ALARM_TONE\n");
        break;

    case SLIC_ALARM_TONE:
        slic_ioc_data_pt->tone = SLIC_TIP_TONE;
        pr_info("tone = SLIC_TIP_TONE\n");
        break;

    case SLIC_TIP_TONE:
        slic_ioc_data_pt->tone = SLIC_FAULT_TONE;
        pr_info("tone = SLIC_FAULT_TONE\n");
        break;

    case SLIC_FAULT_TONE:
        slic_ioc_data_pt->tone = SLIC_CALL_WAITING_TONE;
        pr_info("tone = SLIC_CALL_WAITING_TONE\n");
        break;

    case SLIC_CALL_WAITING_TONE:
        slic_ioc_data_pt->tone = SLIC_NONE_TONE;
        pr_info("tone = SLIC_NONE_TONE\n");
        break;

    default:
        pr_info("error tone type!\n");
        break;
    }
}


static ssize_t si3217x_proc_read(struct file *filp,char __user *buffer, size_t length, loff_t *offset)
{
    uInt8 tmp0= 0;
    uInt8 tmp1= 0;
    uInt8 tmp2= 0;
    uInt8 tmp3= 0;
    uInt8 tmp4= 0;


    tmp0 = slic_direct_read(0, 0x00);
    tmp1 = slic_direct_read(0, PCMMODE);
    tmp2 = slic_direct_read(0, CALR1);

    tmp3 = slic_direct_read(0, RA);

    tmp4 = slic_direct_read(0, ID);


    if (copy_to_user(buffer, &tmp0, sizeof(tmp0)))
        return -EFAULT;
    if (copy_to_user(buffer, &tmp1, sizeof(tmp1)))
        return -EFAULT;
    if (copy_to_user(buffer, &tmp2, sizeof(tmp2)))
        return -EFAULT;
    if (copy_to_user(buffer, &tmp2, sizeof(tmp3)))
        return -EFAULT;

    if (copy_to_user(buffer, &tmp2, sizeof(tmp4)))
        return -EFAULT;

    pr_info("++++++++slic_read+++++++0x00 = %d\n", tmp0);
    pr_info("++++++++slic_read+++++++REG11_PCMMODE = %d\n", tmp1);
    pr_info("++++++++slic_read+++++++REG27_CALR1 = %d\n", tmp2);
    pr_info("++++++++slic_read+++++++REG45_RA = %d\n", tmp3);
    pr_info("++++++++slic_read+++++++REG0_ID = %d\n", tmp4);
    return 0;
}

static ssize_t si3217x_proc_write(struct file *filp,const char __user *buff, size_t len, loff_t *off)
{
    char step = 0;
    uInt8 testdata= 0;

    if (copy_from_user(&step, buff, sizeof(step)))
    {
        pr_info ( "proc_write copy_from_user error!\n");
        return -EFAULT;
    }

    switch (step)
    {
    case '1' :
        //slic_init();
        break;

    case '2' :
        slic_sound_start((slic_tone_enum_type)(slic_ioc_data_pt->tone));
        break;

    case '3' :
        slic_sound_stop((slic_tone_enum_type)(slic_ioc_data_pt->tone));
        break;

    case '4' :
        slic_change_clipmode();
        break;

    case '5' :
        slic_secret_set();
        break;

    case '6' :
        slic_change_tonetype();
        break;

    case '7' :
        slic_direct_write(0, CALR1,119);
        pr_info("write 119 successful\n");
        break;

    case '8' :
        testdata = slic_direct_read(0, CALR1);
        pr_info("REG27_CALR1 = %d\n",testdata);
        break;

    case '9' :
      pr_info("slic_callerid\n");
      slic_callerid();
      break;

    case 'n' : //窄带格式
        pr_info("slic_pcm_mode(SLIC_PCM_16LIN)\n");
        break;        

    case 'w' : //宽带格式
        pr_info("slic_pcm_mode(SLIC_PCM_16LIN_WB)\n");
        break;        

    default:
        pr_info ( "Error proc_write *buff!\n");
        break;
    }

    return len;
}

static struct file_operations si3217x_proc_ops = {
    .read  = si3217x_proc_read,
    .write = si3217x_proc_write,
};

static void create_si3217x_proc_file(void)
{
    g_si3217x_proc_file = create_proc_entry(SI3217X_PROC_FILE, 0644, NULL);
    if (g_si3217x_proc_file)
    {
        g_si3217x_proc_file->proc_fops = &si3217x_proc_ops;
    }
    else
    {
        pr_warning("%s: create proc entry for si3217x failed\n", __FUNCTION__);
    }
}
#endif
/*=======================================test code======================================*/
void pcm_init_state(int channel, struct slic_pcm *dev)
{
    /* Initialize PCM data structure */
    dev->read_data[channel].rd_ptr = (char *)&dev->read_data[channel].data_buf[0];
    dev->read_data[channel].wr_ptr = (char *)&dev->read_data[channel].data_buf[0];
    dev->read_data[channel].data_buf_end  = (char *)&dev->read_data[channel].data_buf[SLIC_CFGPCM_DATA_SIZE];
    dev->write_data[channel].rd_ptr = (char *)&dev->write_data[channel].data_buf[0];
    dev->write_data[channel].wr_ptr = (char *)&dev->write_data[channel].data_buf[0];
    dev->write_data[channel].data_buf_end  = (char *)&dev->write_data[channel].data_buf[SLIC_CFGPCM_DATA_SIZE];
    dev->read_data[channel].xfer_start = 0;
    dev->write_data[channel].xfer_start = 0;
    dev->read_data[channel].data_count = 0;
    dev->write_data[channel].data_count = 0;
}
EXPORT_SYMBOL_GPL(pcm_init_state);
/* Start PCM */
int slic_pcm_start(int channel)
{
    int status = -1;
    struct slic_pcm *dev;
    dev = (struct slic_pcm  *)&slic_pcmdev;
    spin_lock_irq(&dev->pcmdev_lock);

    if (1 == dev->pcm_status[channel])
    {
        spin_unlock_irq(&dev->pcmdev_lock);
        return 0;
    }

    /* Initialize PCM state variables */
    pcm_init_state(channel, dev);

    dev->read_data[channel].data_ready = 0;
    dev->write_data[channel].data_ready = 0;

    /* Set transfer start here, else select won't unblock */
    dev->read_data[channel].xfer_start = 1;
    dev->write_data[channel].xfer_start = 1;


    /* Clear & pre-fill playout data */
    memset(dev->write_data[channel].data_buf, 0, SLIC_CFGPCM_DATA_SIZE); 

    dev->write_data[channel].data_count = (SLIC_CFGPCM_DATA_SIZE);

    /* Set PCM status to indicate PCM controller has started */
    dev->pcm_status[channel] = 1; 

    spin_unlock_irq(&dev->pcmdev_lock);
    MED_PCM_Ctrl_Init();	   

    return (status = 0);

}
EXPORT_SYMBOL_GPL(slic_pcm_start);

int slic_pcm_spkout(char *dma_buf,int limit)
{
    struct slic_pcm *dev = NULL;

    int indx = 0;
    int size=0;
    int channel=0;

    unsigned long flags;
    u32 *rd_ptr = NULL;
    u32 *wr_ptr = NULL;
    dev = (struct slic_pcm  *)&slic_pcmdev;

    if(NULL == dma_buf)
    {
        pr_err("SLIC_PCM:speakout dma buf is null.\r\n");
        return 0;
    }
    
    if(320 < limit)
    {
        pr_info("SLIC_PCM:speakout size is not set.\r\n");
        limit = 320;
    }

    if(NULL == dev)
    {
        pr_err("dev is null.\r\n");
        return 0;
    }
    
    size = min(limit,dev->write_data[channel].data_count);

    spin_lock_irqsave(&dev->pcmdev_lock, flags);
  
    rd_ptr = (u32 *)dev->write_data[channel].rd_ptr;
    wr_ptr = (u32 *)dma_buf;
    for(indx = 0; indx < (size>>2); indx++) 
    {
        /* Read from PCM RX buffer */
        *wr_ptr++ = (u32)(*rd_ptr++);
        if (rd_ptr >= (u32 *)dev->write_data[channel].data_buf_end) 
        {
            rd_ptr = (u32 *)&dev->write_data[channel].data_buf[0];
        }
    }

    if (rd_ptr >= (u32 *)dev->write_data[channel].data_buf_end) 
    {
        rd_ptr = (u32 *)&dev->write_data[channel].data_buf[0];
    }

    dev->write_data[channel].data_count -=size;
    dev->write_data[channel].rd_ptr = (char *)rd_ptr;

    /* Wake up write_q if requested size of space is available to write */
    if(dev->write_data[channel].data_count <= (SLIC_CFGPCM_DATA_SIZE - dev->write_data[channel].req_size))
    {
        if (dev->write_data[channel].xfer_start == 1)
        {
            dev->write_data[channel].data_ready = 1;
        }
    }
    if(NULL == slic_dev)
    {
        pr_err("slic_dev is null.\r\n");
        return 0;
    }
    spin_unlock_irqrestore(&dev->pcmdev_lock, flags);
    
    /* Wake up writeq only if data is ready and if WRITE call is blocking on event */
    if((dev->write_data[channel].data_ready == 1) && (slic_dev->write_blocking == 1))
    {
        wake_up_interruptible(&slic_dev->write_q);
    }

    return size;
}

EXPORT_SYMBOL_GPL(slic_pcm_spkout);


int slic_pcm_micin(char *dma_buf,int limit)
{
    struct slic_pcm *dev = NULL;

    int indx = 0;
    int size = 0;
    int channel = 0;

    unsigned long flags = 0;

    u32 *rd_ptr = NULL;
    u32 *wr_ptr = NULL;
    dev = (struct slic_pcm  *)&slic_pcmdev;
    
    if(NULL == dma_buf)
    {
        pr_err("SLIC_PCM:mic in dma buf is null.\r\n");
        return 0;
    }
    
    if(0 == limit)
    {
        pr_err("SLIC_PCM: size is 0, not need to transer.\r\n");
        return 0;
    }
    if(NULL == dev)
    {
        pr_err("dev is null.\r\n");
        return 0;
    }
    spin_lock_irqsave(&dev->pcmdev_lock, flags);

    size  = min(limit,(SLIC_CFGPCM_DATA_SIZE-dev->read_data[channel].data_count));

    wr_ptr= (u32 *)dev->read_data[channel].wr_ptr;
    rd_ptr = (u32 *)dma_buf;
    for(indx = 0; indx < (size>>2); indx++) 
    {
        /* Read from PCM RX buffer */
        *wr_ptr++ = (u32)(*rd_ptr++);
        if(wr_ptr >= (u32 *)dev->read_data[channel].data_buf_end) 
        {
            wr_ptr = (u32 *)&dev->read_data[channel].data_buf[0];
        }
    }

    if (wr_ptr >= (u32 *)dev->read_data[channel].data_buf_end) 
    {
        wr_ptr = (u32 *)&dev->read_data[channel].data_buf[0];
    }
    dev->read_data[channel].data_count +=size;
    dev->read_data[channel].wr_ptr = (char *)wr_ptr;

    /* Wake up read_q if requested size of data is read */
    if(dev->read_data[channel].data_count >= dev->read_data[channel].req_size)
    {
        if (dev->read_data[channel].xfer_start == 1)
        {
            dev->read_data[channel].data_ready = 1;
        }
    }
    spin_unlock_irqrestore(&dev->pcmdev_lock, flags);

    if(NULL == slic_dev)
    {
        pr_err("slic_dev is null.\r\n");
        return 0;
    }
    /* Wake up readq only if data is ready and if READ call is blocking on event */
    if((dev->read_data[channel].data_ready == 1) && (slic_dev->read_blocking == 1))
    {
        wake_up_interruptible(&slic_dev->read_q);   /* Wake up blocked read */
    }
    
    return size;
}

EXPORT_SYMBOL_GPL(slic_pcm_micin);
/* Do PCM read - Copy from PCM state RX buffer to user buffer */
int slic_pcm_read(int channel, char *buf, int size)
{
    struct slic_pcm *dev = NULL;
    u32 *rd_ptr = NULL;
    u32 *wr_ptr = NULL;
    int indx = 0;

    dev = (struct slic_pcm  *)&slic_pcmdev;
    if(NULL == dev)
    {
        printk("the dev = 0; slic_pcm_read\n");
        return 0;
    }
    spin_lock(&dev->pcmdev_lock);

    /* rd_ptr -> RX FIFO, wr_ptr -> user buffer */
    rd_ptr = (u32 *)dev->read_data[channel].rd_ptr;
    wr_ptr = (u32 *)buf;

    /* If data count is less than requested size, return */
    if(dev->read_data[channel].data_count < size)
    {
        spin_unlock(&dev->pcmdev_lock);
        pr_err("SLIC_PCM:the read buffer data is shorter than the request size,please retry\r\n");
        return 0;
    }

    for(indx = 0; indx < (size>>2); indx++) 
    {
        /* Read from PCM RX buffer */
        *wr_ptr++ = (u32)(*rd_ptr++);
        
        if (rd_ptr >= (u32 *)dev->read_data[channel].data_buf_end) 
        {
            rd_ptr = (u32 *)&dev->read_data[channel].data_buf[0];
        }
    }
    dev->read_data[channel].data_count -= size;
    dev->read_data[channel].rd_ptr = (char *)rd_ptr;

    if(dev->read_data[channel].data_count < size)
    {
        dev->read_data[channel].data_ready = 0;
    }

    spin_unlock(&dev->pcmdev_lock);
    return (size);
}
EXPORT_SYMBOL_GPL(slic_pcm_read);

/* Do PCM write - Copy from user buffer to PCM state TX buffer */
int slic_pcm_write(int channel, char *buf, int size)
{
    struct slic_pcm *dev = NULL;
    u32 *wr_ptr = NULL;
    u32 *rd_ptr = NULL;
    int indx = 0;

    dev = (struct slic_pcm  *)&slic_pcmdev;
    if(NULL == dev)
    {
        printk("the dev = 0; slic_pcm_write\n");
        return 0;
    }
    spin_lock(&dev->pcmdev_lock);

    /* wr_ptr -> TX FIFO, rd_ptr -> user buffer */
    wr_ptr = (u32 *)dev->write_data[channel].wr_ptr;
    rd_ptr = (u32 *)buf;

    /* Don't write anything is the buffer is already FULL */
    if(dev->write_data[channel].data_count >= SLIC_CFGPCM_DATA_SIZE)
    {
        spin_unlock(&dev->pcmdev_lock);
        pr_err("SLIC_PCM:data_count is almost full,please retry.\r\n");
        return 0;   
    }

    for(indx = 0; indx < (size>>2); indx++) 
    {
        /* Send known pattern while writing - if 'pcm_log_state' is set to 0x20 */
        *wr_ptr++ = (u32)(*rd_ptr++);

        /* Check wrap around of user buffer */
        if (wr_ptr >= (u32 *)dev->write_data[channel].data_buf_end) 
        {
            wr_ptr = (u32 *)&dev->write_data[channel].data_buf[0];
        }
    }

    dev->write_data[channel].data_count += size;
    dev->write_data[channel].wr_ptr = (char *)wr_ptr;

    /* Reset 'data_ready' flag if atleast 'req_size' space is not available in the buffer */
    if(dev->write_data[channel].data_count > (SLIC_CFGPCM_DATA_SIZE - size))
    {
        dev->write_data[channel].data_ready = 0;
    }
    spin_unlock(&dev->pcmdev_lock);
    return (size);
}
EXPORT_SYMBOL_GPL(slic_pcm_write);

extern void (*slic_test_hook)(void);
/*------------------------------------------------------------------------
函数原型:si32178_init
描述:  驱动模块加载
输入:  NA
输出:  NA
返回值:  0-成功
------------------------------------------------------------------------*/
static int __init si32178_init(void)
{
    int ret = 0;

    pr_info("si32178 driver init, build number is: 201211271715\n");

    ret = platform_add_devices(si32178_platform_devices, MAX_SI32178_NUM);
    if (ret != 0)
    {
        pr_info("register platform device fail\n");
        return ret;
    }

    ret = platform_driver_register(&si32178_driver);
    if(ret != 0)
    {
        pr_info("register platform driver fail\n");
    }

    slic_test_hook = Equ_Slic_Test;		

    return ret;
}

void slic_read_register(int i)
{
	LOGPRINT("Si3217x Register %d = %X\n", i,
			ProslicChans[0]->deviceId->ctrlInterface->ReadRegister_fptr(ProslicChans[0]->deviceId->ctrlInterface->hCtrl,
				ProslicChans[0]->channel,i));
		    return;
}

void slic_write_register(int i, int value)
{
	slic_direct_write (0, i, value);
}

void slic_read_registers(void)
{
    ProSLIC_PrintDebugReg(ProslicChans[0]);
}
void slic_read_ram(int addr)
{
    printk("RAM %d = %X\n", addr, (unsigned int)slic_indirect_read(addr));
}

void slic_write_ram(int addr, int data)
{
    slic_indirect_write(addr, data);
}

void slic_read_rams(void)
{
    ProSLIC_PrintDebugRAM(ProslicChans[0]);
}

void slic_reset_chip (void)
{
    printk("will hard reset slic chip.\n");

    gpio_set_value(GPIO_SLIC_RST, 0);
    msleep(200);
    gpio_set_value(GPIO_SLIC_RST, 1);
    msleep(200);
    gpio_set_value(GPIO_SLIC_RST, 0);
}
int slic_loopback_thread(void *unused)
{
    char pcm_buf[160] = {0};
    struct slic_pcm *dev = NULL;

    dev = (struct slic_pcm  *)&slic_pcmdev;
    
    while(1)
    {
        if(1 == g_Equip_Loopback)
        {
            printk("start pcm for loopback.\n");
            MED_PCM_Ctrl_Start();
            while(1)
            {
                if(1 == dev->read_data[0].data_ready)
                {
                    slic_pcm_read(0, pcm_buf, sizeof(pcm_buf));
                    slic_pcm_write(0, pcm_buf, sizeof(pcm_buf));
                } 
            }
        }
        msleep(100);
    }
    return 0;
}
void slic_loopback(void)
{
    kthread_run(slic_loopback_thread, NULL, "SLIC_LOOPBACK"); 
}
int slic_driver_version(void)
{
    printk("2012.12.31 10:55\n");
    printk("add ring back tone\n");
    printk("add pcm read and write interface\n");
    return 0;
}

/*------------------------------------------------------------------------
函数原型:si32178_exit
描述: 驱动模块卸载
输入: NA
输出: NA
返回值:  none
------------------------------------------------------------------------*/
static void __exit si32178_exit(void)
{
    pr_info("si32178 driver exit\n");
    platform_driver_unregister(&si32178_driver);
    platform_device_unregister(&si32178_platform_device);
}

//module_init(si32178_init);
late_initcall(si32178_init);
module_exit(si32178_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd");
MODULE_DESCRIPTION("SLIC driver");
MODULE_LICENSE("GPL");

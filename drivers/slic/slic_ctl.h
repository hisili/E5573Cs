
#ifndef SLIC_CTL_H
#define SLIC_CTL_H


#include <linux/ioctl.h>

#define SPI_IOC_MAGIC 'k'

#define SLIC_IOC_SOUND_START    _IOW(SPI_IOC_MAGIC, 1, __u8)
#define SLIC_IOC_SOUND_STOP     _IOW(SPI_IOC_MAGIC, 2, __u8)
#define SLIC_IOC_PCM_MODE       _IOW(SPI_IOC_MAGIC, 3, __u8)
#define SLIC_IOC_HOOK_STATUS   _IOW(SPI_IOC_MAGIC, 4, __u8)
#define SLIC_IOC_TEST_WRITE     _IOW(SPI_IOC_MAGIC, 6, __u8)
#define SLIC_IOC_MEDIA_START    _IOW(SPI_IOC_MAGIC, 7, __u8)
#define SLIC_IOC_MEDIA_STOP    _IOW(SPI_IOC_MAGIC, 8, __u8)
#define SLIC_IOC_TEST_READ      _IOR(SPI_IOC_MAGIC, 1, __u8)

#define CONFIG_SLIC_TEST /* only for test */

typedef unsigned char 		byte;
typedef unsigned short  	word;
typedef unsigned long int 	dword;

typedef long int          	int4;

#define INTLOCK()
#define INTFREE()

#define SLIC_INT_GPIO 85
#define SLIC_RST_GPIO 22

#define SLIC_COVERT_OE 51

#define KEY_CODE_NUM 15 /* 需要上报按键的个数 */

#define SLIC_WR_CMD 0x20  /* SLIC写命令字 */
#define SLIC_RD_CMD 0x60  /* SLIC读命令字 */

#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)

#define FSK_MODEL_PRECURSOR_FRAME  60 /* FSK 模式前导帧必须先发送480bit即60byte */
#define CALLERID_INTERVAL_TIME_MS  70   /*used for DTMF callerid show */

#define CALLERID_FSK_DELAY_MS 100 /* 在FSK来显异常情况下,回调解锁SPI总线 */

#define HISI_PCM_MODE      3   /* 1 -> U-Law, 2 -> A-Law, 3 -> 16bit Linear */

#define GPIO_SLIC_RST     (GPIO_1_12)
#define GPIO_SLIC_SPI_CS  (GPIO_2_18)
#define GPIO_SLIC_SPI_CLK (GPIO_2_19)
#define GPIO_SLIC_SPI_SDI (GPIO_2_20)
#define GPIO_SLIC_SPI_SDO (GPIO_2_21)

#define GPIO_SLIC_INT     (GPIO_0_22)

#define GPIO_SLIC_PCM_CLK (GPIO_2_24)
#define GPIO_SLIC_PCM_SYNC (GPIO_2_25)
#define GPIO_SLIC_PCM_OUT (GPIO_2_26)
#define GPIO_SLIC_PCM_IN (GPIO_2_27)

#define GAIN_DELTA_MAX 6
#define GAIN_DELTA_MIN -12
#define GAIN_MAX 6
#define GAIN_MIN -30
#define MAX_AC_GAIN 3
#define MIN_AC_GAIN -16
#define CALLERID_NUM_MAX 32
#define SLIC_HOOKFLASH_TIME  (750L)  /* HOOKFLASH键的超时时间 750ms */

extern const unsigned short slic_keycodes[KEY_CODE_NUM];
extern int __gpio_get_value(unsigned gpio);
extern void __gpio_set_value(unsigned gpio, int value);

struct slic_ioc_data
{
    byte  slic_num_data[CALLERID_NUM_MAX]; /* 来电号码 */
    byte  date_time[12];     /* 来电时间 */
    byte  clipmode;          /* 来电显示模式 */
    byte  tone;              /* 声音类型 */
    byte  secret;            /* 是否号码隐藏 */
    byte  pos;               /* 来电号码个数 */
    byte  pcm_mode;         /* PCM格式 */
};


typedef enum
{
    
	COUNTRY_BE = 0,      /*比利时(BELGIUM)*/
	COUNTRY_BR = 1,      /*巴西(BRAZIL)*/
	COUNTRY_CL = 2,      /*智利(CHILE) */
	COUNTRY_CN = 3,      /*中国(CHINA)*/
	COUNTRY_DK = 4,      /*丹麦(DENMARK)*/   
	COUNTRY_EE = 5,      /*爱沙尼亚(ESTONIA) */          
	COUNTRY_FI = 6,      /*芬兰(FINLAND)*/
	COUNTRY_FR = 7,      /*法国(FRANCE) */
	COUNTRY_DE = 8,      /*德国(GERMANY)*/
	COUNTRY_HU = 9,      /*匈牙利(HUNGARY)*/
    COUNTRY_IT = 10,      /*意大利(ITALY)*/
    COUNTRY_JP = 11,      /*日本(JAPAN)*/
    COUNTRY_NL = 12,      /*荷兰(NETHERLANDS)*/
    COUNTRY_US = 13,      /*美国(UNITED STATES)*/
    COUNTRY_ES = 14,      /*西班牙(SPAIN)*/
    COUNTRY_SE = 15,      /*瑞典(SWEDEN)*/
    COUNTRY_CH = 16,      /*瑞士(SWITZERLAND)*/
    COUNTRY_SA = 17,      /*沙特阿拉伯(SAUDI ARABIA)*/
    COUNTRY_GB = 18,      /*英国(UNITED KINGDOM) */
    COUNTRY_SG = 19,	  /*新加坡(SINGAPORE)*/   
    COUNTRY_RU = 20,       /*俄罗斯(RUSSIAN FEDERATION)*/
    COUNTRY_PL = 21,      /*波兰(POLAND)*/
    COUNTRY_AU = 22,      /*澳大利亚(AUSTRALIA)*/
	COUNTRY_COUNT
} Country_Type;

typedef enum
{
    GPIO_LOW_VALUE  = 0,
    GPIO_HIGH_VALUE = 1
} GPIO_ValueType;

/* Caller ID mode enum */
typedef enum 
{
    SLIC_CALLERID_DTMF = 0,                 /* DTMF */
    SLIC_CALLERID_FSK_BELL202,              /* FSK Bell 202 */
    SLIC_CALLERID_FSK_ITUTV23,              /* FSK ITU-T V.23 */
    SLIC_CALLERID_FSK_RPAS                   /* FSK RPAS */
} slic_callerid_enum_type;

/* Teletax mode enum */
typedef enum {
    SLIC_TELETAX_REVERSAL = 0,              /* Polarity reversal */
    SLIC_TELETAX_12K,                       /* 12KHz burst */
    SLIC_TELETAX_16K,                       /* 16KHz burst */
    SLIC_TELETAX_ALL                        /* ALL sort of teletax mode */
} slic_teletax_enum_type;

/* Ringing structure */
typedef struct {
    dword    offset;                        /* Ringing DC offset */
    dword    frequency;                     /* Ringing frequency */
    dword    amplitude;                     /* Ringing amplitude */
    word    ontime1;                        /* Ringing 1st on time  */
    word    offtime1;                       /* Ringing 1st off time */
    word    ontime2;                        /* Ringing 2nd on time  */
    word    offtime2;                       /* Ringing 2nd off time */
} slic_ringing_type;

/* Tone structure */
typedef struct {
    dword    frequency1;                    /* Tone 1 frequency */
    dword    amplitude1;                    /* Tone 1 amplitude */
    dword    frequency2;                    /* Tone 2 frequency */
    dword    amplitude2;                    /* Tone 2 amplitude */
    word    burst;                          /* Tone on time     */
    word    pause;                          /* Tone off time    */
} slic_tone_type;

/* ProSLIC parameters strcture */
typedef struct {
    slic_ringing_type       ring;           /* Ringing generator */
    slic_tone_type          dial_tone;      /* Dial tone generator */
    slic_tone_type          busy_tone;      /* Busy tone generator */
    slic_tone_type          alarm_tone;     /* Alarm tone generator */
    slic_tone_type          tip_tone;       /* Succeed/fail tone generator */
    slic_tone_type          fault_tone;     /* Fault tone generator */
    slic_tone_type          waiting_tone;     /* waiting tone generator */
    slic_callerid_enum_type callerid_mode;  /* Sending CallerID mode */
    slic_teletax_enum_type  teletax_mode;   /* Teletax mode */
    byte                    slic_zsynth_type; /* zsynth type eg. ZSYN_600_0_0_30_0*/
    byte                    slic_dcfeed_type; /* dc feed type eg. DCFEED_48V_20MA */
    byte                    slic_ring_preset_type; /* ring preset type eg. RING_F20_45VRMS_0VDC_LPR */
    unsigned int            slic_rkey_upper_limit; /* flash hook time upper limit eg. 150ms */
    unsigned int            slic_rkey_lower_limit; /* flash hook time lower limit eg. 70ms */
    unsigned int            slic_onhook_lower_limit; /* on hook lower limit time eg. 750ms */
} slic_parm_type;


#define MAX_OSCAMP_NUM 14

typedef struct{
    dword osc1amp;
    dword osc2amp;  
}slic_ocsamp;

typedef struct{
    slic_ocsamp ocsamp[MAX_OSCAMP_NUM];
}slic_dtmf_signal_level;

typedef struct{
    dword rkey_time_min;
    dword rkey_time_max;
}slic_rkey_time;

/* 语音定制扩展参数 */
typedef struct{
    byte callid_first_signal;   
    slic_rkey_time rkey_time;
    slic_dtmf_signal_level dtmf_signal_level;
}slic_customize_ex_parm_type;

/* ProSLIC uC interface lock status mark enum */
typedef enum {
    SLIC_CMD_UNLOCK = 0,                    /* Unlocked */
    SLIC_CMD_LOCK                           /* Locked   */
} slic_cmd_mark_enum_type;

typedef enum{
    TIP_TONE =0 ,

    CALL_WAITING_TONE_ONTIME1,
    CALL_WAITING_TONE_OFFTIME1,
    CALL_WAITING_TONE_ONTIME2,
    CALL_WAITING_TONE_OFFTIME2,

    CONFIRMATION_TONE_ONTIME1,
    CONFIRMATION_TONE_OFFTIME1,
    CONFIRMATION_TONE_ONTIME2,

}OSCILLATOR1_CREATE_SOUND_TYPE;

/* Tip tone enum type */
typedef enum
{
    SLIC_NULL_TIP_TONE = 0,
    SLIC_SUCCEED_TIP_TONE = 1,
    SLIC_FAIL_TIP_TONE = 2
} slic_tip_tone_enum_type;

/* Four ring part are set to 2 parts, first and last */
typedef enum{
    SLIC_FIRST_PART_RING,
    SLIC_LAST_PART_RING,
    SLIC_RPAS_RING,
    SLIC_RPAS_NORMAL_RING
} slic_four_ring_part_type;

typedef enum{
    CLIP_NONE,
    CLIP_DTMF,
    CLIP_FSK_ITU,
    CLIP_FSK_BELL,
    CLIP_FSK_RPAS,
    CLIP_MAX
} slic_clip_mode;

/* Tone enum type 驱动与应用层保持一致 */
typedef enum{
    SLIC_NONE_TONE = 0,                     /* none */
    SLIC_RING,                              /* Ring */
    SLIC_RING_FOR_MONITOR,                  /* Ring for houtai Monitor */
    SLIC_KEEP_RING,                         /* Keep Ring */
    SLIC_DIAL_TONE,                         /* Dial tone */
    SLIC_BUSY_TONE,                         /* Busy tone */
    SLIC_ALARM_TONE,                        /* Alarm tone */
    SLIC_TIP_TONE,                          /* Succeed/fail tone */
    SLIC_FAULT_TONE,                        /* Fault tone */
    SLIC_CALL_WAITING_TONE,                 /* call waiting tone */
    SLIC_RINGBACK_TONE,                     /* Ring Back tone */
    SLIC_CONFIRMATION_TONE,                 /* Confirmation tone */
    SLIC_VM_STUTTER_TONE,                /* Voice Mail Stutter Tone */
    SLIC_CALL_FWD_RMD_TONE,             /* Call forward reminder tone */
    SLIC_SUCCEED_TONE,
    SLIC_FAIL_TONE
} slic_tone_enum_type;

/* Four/Two ring indicator type */
typedef enum{
    TWO_RING,
    FOUR_RING
}four_or_two_ring_type;

/* A5驱动层，DTMF键值定义，Q6应用层也应与此保持一致 */
typedef enum {
    SLIC_NONE_K = 0x00,
    /**< -- Deprecated. */
    SLIC_POUND_K = 0x23,
    /**< -- # key, ASCII #. */
    SLIC_STAR_K = 0x2A,
    /**< -- * key, ASCII *. */
    SLIC_0_K = 0x30,
    /**< -- 0 key, ASCII 0. */
    SLIC_1_K = 0x31,
    /**< -- 1 key, ASCII 1. */
    SLIC_2_K = 0x32,
    /**< -- 2 key, ASCII 2. */
    SLIC_3_K = 0x33,
    /**< -- 3 key, ASCII 3. */
    SLIC_4_K = 0x34,
    /**< -- 4 key, ASCII 4. */
    SLIC_5_K = 0x35,
    /**< -- 5 key, ASCII 5. */
    SLIC_6_K = 0x36,
    /**< -- 6 key, ASCII 6. */
    SLIC_7_K = 0x37,
    /**< -- 7 key, ASCII 7. */
    SLIC_8_K = 0x38,
    /**< -- 8 key, ASCII 8. */
    SLIC_9_K = 0x39,
    /**< -- 9 key, ASCII 9. */
    SLIC_A_K = 0x41,
    /**< -- A key, ASCII A. */
    SLIC_B_K = 0x42,
    /**< -- B key, ASCII B. */
    SLIC_C_K = 0x43,
    /**< -- C key, ASCII C. */
    SLIC_D_K = 0x44,
    /**< -- D key, ASCII D. */
    SLIC_SEND_K = 0x50,
    /**< -- Send key or Call key, usually signified by the green key on a
    handset that is used to start a call. */
    SLIC_END_K = 0x51,
    /**< -- End key, usually signified by the red key on a handset that is
    used to end a call. */
    SLIC_RKEY_K = 0xF7, /*R键*/
    SLIC_HANGON_K = 0xF8, /*the phone is hung on，摘机键*/
    SLIC_HANGUP_K = 0xF9, /*the phone is hung up，挂机键*/
    SLIC_PRESS_KEY_IN_CALLING_K = 0xFB, /*the key be pressed in calling，通话中按键*/
    SLIC_RELEASE_K = 0xFF,
} slic_key_code;
#define SLIC_CFGPCM_DATA_SIZE          160*2  /* FIFO_SIZE in bytes */ 

#define SLIC_MAX_BUFSIZE    960*2

#define SLIC_READ_BUFSIZE   640*2   /* Don't write more than 640 bytes */
#define SLIC_WRITE_BUFSIZE  640*2   /* Don't write more than 640 bytes */


#define SLIC_CFGPCM_NB_CHANNEL 1

struct pcm_data
{
      int          xfer_start;
      int          data_ready;
      int          data_count;
      int          req_size;
      char         data_buf[SLIC_CFGPCM_DATA_SIZE];
      char         *rd_ptr;
      char         *wr_ptr;
      char         *data_buf_end;
};

struct slic_pcm {
      spinlock_t             pcmdev_lock;

      struct pcm_data        read_data[SLIC_CFGPCM_NB_CHANNEL];
      struct pcm_data        write_data[SLIC_CFGPCM_NB_CHANNEL];

      int                    pcm_status[SLIC_CFGPCM_NB_CHANNEL];
      int                    active_channel;
};
struct si32178_data
{
    struct input_dev *input_dev;
    //struct spi_device *spi_dev;
    struct  timer_list slic_int_delay_timer;       /* 定义中断延时处理定时器 */
    struct  timer_list slic_callerid_delay_timer;  /* 定义来显延时处理定时器 */
    struct  timer_list slic_snd_start_delay_timer; /* 定义发声延时处理定时器 */
    struct  timer_list slic_snd_stop_delay_timer;  /* 定义声音停止延时处理定时器 */
    struct  timer_list slic_callerid_fsk_delay_timer;  /* 定义FSK来显异常延时处理定时器 */
    struct  timer_list slic_rkey_delay_timer; /* 定义R键定时器 */
    struct  timer_list slic_pcm_mode_timer;  /* 定义PCM mode设置定时器*/
#ifdef FEATURE_HUAWEI_CS_INBAND_DTMF_ELIMINATE
    struct  timer_list slic_pcm_unmute_timer;  /* MUTE后重新打开PCM码流定时器 */
#endif
#ifdef FEATURE_HUAWEI_DTMF_FLUCTUATE_ELIMINATE
    struct  timer_list slic_dtmf_valid_timer;       /* DTMF防抖动定时器 */
#endif
    struct timer_list slic_test_timer; /* 测试用定时器 */
    int     slic_irq;
    unsigned long lock_flags;
    unsigned short keycodes[ARRAY_SIZE(slic_keycodes)];
   /* Instance of PCM controller state */
   struct slic_pcm *slic_pcmdev;

   /* lock for SLIC driver */
   spinlock_t  lock;

   /* Store MAX buffer sizes allowed in SLIC driver */
   unsigned long int read_buffer_size;
   unsigned long int write_buffer_size;

   /* Flag to set if READ/WRITE controls are using blocking or non-blocking calls 
   ** This is important as PCM controller will wake up SLIC driver, 
   ** only if this flag is set 
   */
   int read_blocking; 
   int write_blocking; 
   
   int dbg_rd_mode;
   int dbg_wr_mode;
   /* Wait Qs for user read/write controls  */
   /*sel_queue_t *sel_q;   */
   wait_queue_head_t read_q;
   wait_queue_head_t write_q;
   unsigned int inwrite:1;      /* 1 -> Currently writing, 0 -> free */
   unsigned int inread:1;       /* 1 -> Currently reading  0 -> free */

};

typedef enum
{
    RKEY_STATE_INVALID,
    RKEY_STATE_VALID, 
    RKEY_STATE_OVERTIME,
    RKEY_STATE_ONHOOK_VALID
}SLIC_RKEY_STATE_TYPE;

extern struct si32178_data *slic_dev;
extern struct slic_ioc_data  *slic_ioc_data_pt;
extern slic_parm_type slic_parm[COUNTRY_COUNT];
extern slic_customize_ex_parm_type slic_customize_ex_parm[COUNTRY_COUNT];
extern Country_Type g_Country_Type;
void slicctl_proslic_init(void);
void slicctl_chinit(int channel);

static inline void gpio_out (unsigned gpio, int value)
{
    __gpio_set_value(gpio, value);
}

static inline int gpio_in (unsigned gpio)
{
    return __gpio_get_value(gpio);
}	

#endif

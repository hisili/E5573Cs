

#ifndef __PRODUCT_NV_DEF_H__
#define __PRODUCT_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WEB_STR_LEN 36

#define MMA_HUAWEI_PLMN_MODIFY_MAX_SIZE                   (128)
#define TAF_NVIM_MAX_APN_LOCK_LEN                        (99)
#define TAF_NVIM_MAX_APN_LOCK_STR_LEN                    (TAF_NVIM_MAX_APN_LOCK_LEN + 1)
#define TAF_NVIM_HWLOCK_LEN                              (32)
/*请参考结构体样例*/
typedef struct
{
    BSP_S32 reserved;  /*note */
}PRODUCT_MODULE_STRU;
/*NV50336结构定义*/
typedef struct
{
    UINT8 SKU_TYPE;    /*合法取值 0、1、2 0：Voddafone Mobile WiFi;1:Vodafone Pocket WiFi;2:Enterprise None P&P*/
    UINT8 RESERVED;    /*预留*/
}NV_ITEM_SKU_STRU;

/*NV50337结构定义*/
typedef struct
{
    UINT8 ACCOUNT_TYPE;   /*合法取值 0、1；0:Prepay first,contract second;1:Contract first,Prepay second*/
    UINT8 RESERVED;       /*预留*/
}NV_ITEM_ACCOUNT_STRU;
/*NV50364电池参数数据结构**/
typedef struct
{
    BSP_U8 no_battery_powerup_enable;        /*非工厂模式下电池部在位开机使能标志*/
    BSP_U8 exception_poweroff_poweron_enable;/*异常关机后，下次插入电源进入开机模式使能标志*/
    BSP_U8 low_battery_poweroff_disable;     /*低电关机禁止标志*/
    BSP_U8 reserved;                         /*保留*/
}POWERUP_MODE_TYPE;

/*充电过程中温度参数NV 50385结构定义*/
typedef struct
{
    BSP_U32    ulChargeIsEnable;                    //充电温保护使能    
    BSP_S32     overTempchgStopThreshold;           //充电高温保护门限
    BSP_S32     subTempChgLimitCurrentThreshold;    //高温充电进入门限
    BSP_S32     lowTempChgStopThreshold;            //充电低温保护门限
    BSP_S32     overTempChgResumeThreshold;         //充电高温恢复温度门限
    BSP_S32     lowTempChgResumeThreshold;          //充电低温恢复温度门限
    BSP_U32     chgTempProtectCheckTimes;           //充电停充轮询次数
    BSP_U32     chgTempResumeCheckTimes;            //充电复充轮询次数
    BSP_S32     exitWarmChgToNormalChgThreshold;    //由高温充电恢复到常温充电温度门限
    BSP_S32     reserved2;                          //预留    
}CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE;

/*充电过程中温度参数NV50386结构定义*/
typedef struct
{
    BSP_S32         battVoltPowerOnThreshold;           //开机电压门限
    BSP_S32         battVoltPowerOffThreshold;          //关机电压门限
    BSP_S32         battOverVoltProtectThreshold;       //平滑充电过压保护门限(平滑值)
    BSP_S32         battOverVoltProtectOneThreshold;    //单次充电过压保护门限(单次值)
    BSP_S32         battChgTempMaintThreshold;          //区分高温停充和正常停充的判断门限
    BSP_S32         battChgRechargeThreshold;           //充电二次复充门限
    BSP_S32         VbatLevelLow_MAX;                   //低电上限门限
    BSP_S32         VbatLevel0_MAX;                     //0格电压上限门限
    BSP_S32         VbatLevel1_MAX;                     //1格电压上限门限
    BSP_S32         VbatLevel2_MAX;                     //2格电压上限门限
    BSP_S32         VbatLevel3_MAX;                     //3格电压上限门限 
    BSP_S32         battChgFirstMaintThreshold;         //首次判断是否满电
    BSP_S32         battNormalTempChgRechargeThreshold; //常温区间复充门限
}CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE;


typedef struct 
{       
    BSP_U8     FileSysActiveProtectEnable;   /* 文件系统主动保护使能 */     
    BSP_U8     BadImgResumFromOnlineEnable;   /* 镜像损坏后从Online分区恢复使能*/     
    BSP_U8     BootNotOperaFileSysEnable;      /*系统启动过程不对文件系统操作使能*/     
    BSP_U8     FastOffFailTrueShutdownEnable;    /*假关机失败进入真关机功能使能*/      
    BSP_U8     SoftPowOffReliableEnable;          /*软件关机可靠性功能使能*/      
    BSP_U8     ZoneWriteProtectEnable;          /*分区写越界保护使能*/       
    BSP_U8     BadZoneReWriteEnable;            /* Flash病危块回写功能使能*/       
    BSP_U8     BootCanFromkernelBEnable;  /*主镜像破坏后可以从备份镜像启动使能*/
    BSP_U8     OnlineChangeNotUpgradEnable; /*Online分区变化不进行在线升级使能*/
    /*预留*/
    BSP_U8     BadZoneScanEnable;   /* Flash病危块扫描功能使能*/
    BSP_U8     reserved2;
    BSP_U8     reserved3;
    BSP_U8     reserved4;
    BSP_U8     reserved5;
    BSP_U8     reserved6;
    BSP_U8     MmcReliabilityEnable;            /*协议MMC可靠性保护使能开关*/
    BSP_U32    MmcReliabilityBitmap;     /* 通信协议协议MMC可靠性保护功能掩码 */
    BSP_U32    DangerTime;                   /*频繁上下电危险期经验值 ，单位为秒*/       
    BSP_U32    WaitTime;           /*应用通知到底软假关机后，定时器值单位为秒*/
    /*预留*/
    BSP_S32    reserved7; 
    BSP_S32    reserved8; 
    BSP_S32    reserved9; 
    BSP_S32    reserved10; 
    BSP_S32    reserved11;       
}SOFT_RELIABLE_CFG_STRU;


typedef struct
{
    VOS_UINT8 webSite[WEB_STR_LEN];
}WEB_SITE_STRU;

typedef struct
{
    VOS_UINT8 wpsPin[WEB_STR_LEN];
}WPS_PIN_STRU;

typedef struct
{
    VOS_UINT8 userName[WEB_STR_LEN];
}WEB_USER_NAME_STRU;

#define CUSTOM_MCC_LIST_NUM_MAX                 (30)
typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;
    VOS_UINT8                           ucCustomMccNum;
    VOS_UINT8                           aucReserve[2];    
    VOS_UINT32                          aulCustommMccList[CUSTOM_MCC_LIST_NUM_MAX];  /* 允许漫游的国家码列表 */    
}NAS_MMC_NVIM_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_STRU_EX;


#define DISABLE_RPLMN_ACT_ON       (0)
#define DISABLE_RPLMN_ACT_OFF      (1)
typedef struct
{
    VOS_UINT8                       ucEnableFlag;/*0: Disable RPLMN ACT 特性关闭,1: Disable RPLMN特性打开*/
    VOS_UINT8                       aucReserve[3];
}NAS_MCC_NVIM_RPLMN_DISABLE_ACT_CFG_STRU;

/*****************************************************************************
 结构名    : GID1_TYPE_STRU
 结构说明  : 用于定制GID1锁卡功能
 结构 ID   : 50083
*****************************************************************************/
typedef struct
{
    VOS_UINT8   bGID1Enable;                 /*GID1锁卡使能位*/
    VOS_UINT8   ucLeagalValue;               /*合法GID1值*/
}GID1_TYPE_STRU;

/*****************************************************************************
 结构名    : NAS_NV_Vodafone_CPBS
 结构说明  : 德国 Vodafone CPBS定制
 结构 ID   : 50429
*****************************************************************************/
typedef struct
{
   VOS_UINT16 Vodafone_CPBS;                 /*定制使能位*/
   VOS_UINT16 Reserved;
}NAS_NV_Vodafone_CPBS;

/*****************************************************************************
 结构名    : TAF_NV_CSIM_CUSTOMIZED
 结构说明  : AT+CSIM定制
 结构 ID   : 50189
*****************************************************************************/
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
     VOS_UINT8  ucReserved[3];         /* 保留字段*/
} TAF_NV_CSIM_CUSTOMIZED;


typedef struct
{
   VOS_UINT8    ucNvActiveFlag;
   VOS_UINT8    ucReserved[3];            /*保留字段*/
   VOS_UINT32   ulBandGroup1Low32Bit;
   VOS_UINT32   ulBandGroup1High32Bit;
   VOS_UINT32   ulBandGroup2Low32Bit;
   VOS_UINT32   ulBandGroup2High32Bit;
   VOS_UINT32   ulReserved[4];            /*保留字段，用于后续扩展*/
}NAS_NV_CUSTOMIZED_BAND_GROUP;


typedef struct
{
    VOS_UINT8 ucNvimActiveFlg;
    VOS_UINT8 aucReserved[3];                        /*保留字段*/
}NV_ITEM_START_MANUAL_TO_AUTO_ST;


#define MMA_SPECIAL_HPLMN_ITEM_SIZE     (4)      /* 单个PLMN空间大小*/
#define MMA_SPECIAL_HPLMN_LIST_SIZE     (30)     /* 最多可以存储30个PLMN*/

typedef struct
{
     VOS_UINT8            aucHPlmnId[MMA_SPECIAL_HPLMN_ITEM_SIZE];    /*4字节数组表示一个HPLMN*/
}MMA_SPEC_HPLMN_TYPE;

typedef struct
{
     VOS_UINT32           ulHPlmnCount;                               /*HPLMN个数*/
     MMA_SPEC_HPLMN_TYPE  astHPlmnGroup[MMA_SPECIAL_HPLMN_LIST_SIZE]; /*HPLMN列表*/
}TAF_MMA_SPEC_PLMN_NOT_ROAM_ST;


#define MMA_HUAWEI_PLMN_MODIFY_MAX_SIZE                   (128)

typedef struct
{
    VOS_UINT8 aucRcvPlmnData[MMA_HUAWEI_PLMN_MODIFY_MAX_SIZE];
}NAS_MMA_HUAWEI_PLMN_MODIFY_STRU;


typedef struct
{
    VOS_UINT8 ucCollected;
    VOS_UINT8 aucReserved;
}SALES_AGENT_RECORD_STRU;

typedef struct
{
     VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
     VOS_UINT8  ucReserved[3];         /* 保留字段*/
}HPLMN_Within_EPLMN_NotRoam;

/*****************************************************************************
 结构名    : SI_PIH_IMSI_TIMER_CONFIG
 结构说明  : MMA保存IMSI读取周期的NV结构
 结构 ID    : 50426
*****************************************************************************/
typedef struct
{
    VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
    VOS_UINT8  ucReserved;         /* 保留字段*/
    VOS_UINT16  usTimerLen;        /* 读取IMSI周期(单位为秒)*/
}SI_PIH_IMSI_TIMER_CONFIG;
/*****************************************************************************
 结构名    : TIM_CPIN_STRU
 结构说明  : AT+CPIN?定制
 结构 ID   : 50450
*****************************************************************************/
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;     /* NV激活标志字段*/
     VOS_UINT8  ucReserved[3];         /* 保留字段*/
} TIM_CPIN_STRU;

typedef struct
{
    VOS_UINT8   led_enable;        /* LED灯使能开关 */
    VOS_UINT8   led_dr;            /* LED灯所使用的DR */
    VOS_UINT8   led_mode;          /* LED灯模式标志 */
    VOS_UINT8   led_reserve;       /* 保留字段 */
    VOS_UINT32  full_on;           /* LED灯呼吸模式稳定亮的持续时间 */
    VOS_UINT32  full_off;          /* LED灯呼吸模式稳定暗的持续时间 */
    VOS_UINT32  fade_on;           /* LED灯呼吸模式从暗到亮的持续时间 */
    VOS_UINT32  fade_off;          /* LED灯呼吸模式从亮到暗的持续时间 */
    VOS_UINT32  delay_on;          /* LED灯闪烁模式亮的时间 */
    VOS_UINT32  delay_period;     /* LED灯闪烁模式的闪烁周期时间*/
    VOS_UINT32  full_long_on;     /* LED灯长亮的持续时间 */
    VOS_UINT32  full_long_off;    /* LED灯长暗的持续时间 */
    VOS_UINT32  brightness;       /* LED灯亮度电流值 */
}NV_LED_PARA_STRU;

/*****************************************************************************
 结构名    : SMS_AUTO_REG_STRU
 结构说明  : 短信自注册定制
 结构 ID   : 50449
*****************************************************************************/
#define MAX_IMSI_LEN 15
typedef struct 
{
     VOS_UINT8  ucNvActiveFlag;                               /*NV激活标志字段*/
     VOS_UINT8  ucSmsRegFlag;                                 /*注册标志位*/
     VOS_UINT8  ucSmsRegImsi[MAX_IMSI_LEN];               /*已注册IMSI号*/
     VOS_UINT8  ucReserved[3];                                /*保留字段*/
} SMS_AUTO_REG_STRU;

/*****************************************************************************
 结构名    : TATA_DEVICELOCK_STRU
 结构说明  : 设备锁NV的结构
 结构 ID   : 50432
*****************************************************************************/
#define MAX_DLCK_ENCODE_LEN_MBB 32
typedef struct
{
    VOS_UINT32 ulStatus;                           /*设备锁激活状态*/
    VOS_UINT8   aucLockCode[MAX_DLCK_ENCODE_LEN_MBB];    /*设备锁的密码*/
}TATA_DEVICELOCK_STRU;

/*****************************************************************************
 结构名    : NAS_NV_HPLMN_FIRST_UMTS_TO_LTE
 结构说明  : YOTA定制，从3G到LTE时优先搜索EHPLMN
 结构 ID   : 50432
*****************************************************************************/
typedef struct
{
    VOS_UINT8 ucNvActiveFlag; /* 是否激活 YOTA 定制功能 */
    VOS_UINT8 aucReserved[3]; /* 保留 */
}NAS_NV_HPLMN_FIRST_UMTS_TO_LTE;


typedef struct
{
    VOS_UINT8    ucNvActiveFlag;   /* 是否激活 EE 运营商显示定制功能 */
    VOS_UINT8    aucReserved[3];   /* 保留 */
}NAS_NV_EE_OPERATOR_NAME_DISPLAY;


#define MODE_LIST_MAX_LEN    (6)
#define MODE_LIST_MAX_NUM    (10)

typedef struct
{
    VOS_UINT8    ucListItemNum;
    VOS_UINT8    ucRestrict;
    VOS_UINT8    aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
}NAS_NV_SYSCFGEX_MODE_LIST;


/* 锁APN设置 */
typedef struct
{
    VOS_UINT8   ucStatus;       /* 1: NV有效标志位，0：无效 */
    VOS_UINT8   ucApnLength;
    VOS_UINT8   aucApn[TAF_NVIM_MAX_APN_LOCK_STR_LEN];       /* APN从该数组的第一个字节开始写入，并且以'\0'作为结尾 */
    VOS_UINT8   aucRsv[2];
}APS_APN_LOCK_STRU;

/*****************************************************************************
 结构名    : NV_WINBLUE_PROFILE_STRU
 结构说明  : Windows8.1 特性控制NV
 结构 ID   : 50424
*****************************************************************************/
typedef struct
{
    VOS_UINT8  InterfaceName[32];
    VOS_UINT8  MBIMEnable;
    VOS_UINT8  CdRom;
    VOS_UINT8  TCard;
    VOS_UINT8  MaxPDPSession;
    VOS_UINT16 IPV4MTU;
    VOS_UINT16 IPV6MTU;
    VOS_UINT32 Reserved1;
    VOS_UINT32 Reserved2;
}NV_WINBLUE_PROFILE_STRU;

/*50577*/
typedef struct
{
VOS_UINT8 nv_status;    /*该nv是否激活*/
VOS_UINT8 diag_enable;  /*diag端口是否使能*/
VOS_UINT8 shell_enable; /*shell端口是否使能*/
VOS_UINT8 adb_enable;   /*adb端口是否使能*/
VOS_UINT8 om_enable;   /*om端口是否使能*/
VOS_UINT8 ucReserved_0; /*保留字段*/
VOS_UINT8 ucReserved_1; /*保留字段*/
VOS_UINT8 ucReserved_2; /*保留字段*/
}HUAWEI_NV_USB_SECURITY_FLAG;

/*****************************************************************************
 结构名    : NV_USB_CDC_NET_SPEED_STRU
 结构说明  : 获取网络速度默认值的NV
 结构 ID   : 50456
*****************************************************************************/
typedef struct
{
    VOS_UINT32  nv_status;
    VOS_UINT32  net_speed;  
    VOS_UINT32  reserve1;
    VOS_UINT32  reserve2;
    VOS_UINT32  reserve3;
}NV_USB_CDC_NET_SPEED_STRU;

/*****************************************************************************
 结构名    : ss_coul_nv_info
 结构说明  : 库仑计NV
 结构 ID   : 50462
*****************************************************************************/
/*add by zhouyunfei 20130916*/
#define COUL_MAX_TEMP_LEN 10
#define COUL_RESERVED_LEN 4
typedef struct
{
    BSP_S32 charge_cycles;
    BSP_S32 r_pcb; // uohm
    BSP_S32 v_offset_a;
    BSP_S32 v_offset_b;
    BSP_S32 c_offset_a;
    BSP_S32 c_offset_b;
    BSP_S16 temp[COUL_MAX_TEMP_LEN];
    BSP_S16 real_fcc[COUL_MAX_TEMP_LEN];
    BSP_S16 calc_ocv_reg_v;
    BSP_S16 calc_ocv_reg_c;
    BSP_S16 hkadc_batt_temp;
    BSP_S16 hkadc_batt_id_voltage;
    BSP_S32 start_cc;
    BSP_S32 ocv_temp;
    BSP_S32 limit_fcc;
    BSP_S32 reserved[COUL_RESERVED_LEN];
}ss_coul_nv_info;
#define BATTERY_SN_LEN  (32)
typedef struct
{
    BSP_U8  battery_sn[BATTERY_SN_LEN];
    BSP_U32  coul_firmware_update_status;
}nv_huawei_coul_firmware_update_info;

/*****************************************************************************
 结构名    : NV_FTEN_USB_SWITCH_SEL_INFO
 结构说明  : 设置丰田BOX 双USB口选择
 结构 ID   : 50607
*****************************************************************************/
typedef struct
{
    VOS_UINT8 nv_status;
    VOS_UINT8 usb_sel_flag;
    VOS_UINT8 reserve1;
    VOS_UINT8 reserve2;
} NV_FTEN_USB_SWITCH_SEL_INFO;
/*****************************************************************************
 结构名    : NV_PLATFORM_CATEGORY_SET_STRU
 结构说明  : 设置单板CAT等级
 结构 ID   : 50458
*****************************************************************************/
typedef struct
{
    VOS_UINT32   nv_status;
    VOS_UINT32   cat_value;
    VOS_UINT32   reserve1;
    VOS_UINT32   reserve2;
    VOS_UINT32   reserve3;
}NV_PLATFORM_CATEGORY_SET_STRU;

/*****************************************************************************
 函数名称  : WLAN_AT_WIINFO_CHANNELS_NVIM_STRU
 功能描述  : 查询WiFi的信道和功率结构体 
 结构 ID   :      50468，50469
*****************************************************************************/
#define MAX_CHANNEL24G_SIZE     (32)                    /* 数组存储24个信道和之间逗号的ASCII码值 */
#define MAX_CHANNEL5G_SIZE      (96)        
#define MAX_PWR_SIZE            (8)

typedef struct 
{
    VOS_UINT8   Channels24G[MAX_CHANNEL24G_SIZE];            /* 2.4G下工作的信道 */
    VOS_UINT8   Channels5G[MAX_CHANNEL5G_SIZE];             /* 5G下工作的信道 */
}WLAN_AT_WIINFO_CHANNELS_NVIM_STRU;
typedef struct 
{
    VOS_UINT8   bMode_24G_pwr[MAX_PWR_SIZE];                         /* 2.4G b模式下的目标功率 */
    VOS_UINT8   gMode_24G_pwr[MAX_PWR_SIZE];                         /* 2.4G g模式下的目标功率 */
    VOS_UINT8   nMode_24G_pwr[MAX_PWR_SIZE];                         /* 2.4G n模式下的目标功率 */
    VOS_UINT8   aMode_5G_pwr[MAX_PWR_SIZE];                           /* 5G a模式下的目标功率 */
    VOS_UINT8   nMode_5G_pwr[MAX_PWR_SIZE];                           /* 5G n模式下的目标功率 */
    VOS_UINT8   acMode_5G_pwr[MAX_PWR_SIZE];                         /* 5G ac模式下的目标功率 */
    VOS_UINT8   reserve1[MAX_PWR_SIZE];                                      /*预留*/
    VOS_UINT8   reserve2[MAX_PWR_SIZE];                                      /*预留*/
}WLAN_WIINFO_POWER_NVIM_STRU;


typedef struct
{
    VOS_UINT32      ulDSLastLinkTime;                       /*DS最近一次连接时间*/
    VOS_UINT32      ulDSTotalSendFluxLow;                   /*DS累计发送流量低四个字节*/
    VOS_UINT32      ulDSTotalSendFluxHigh;                  /*DS累计发送流量高四个字节*/
    VOS_UINT32      ulDSTotalLinkTime;                      /*DS累计连接时间*/
    VOS_UINT32      ulDSTotalReceiveFluxLow;                /*DS累计接收流量低四个字节*/
    VOS_UINT32      ulDSTotalReceiveFluxHigh;               /*DS累计接收流量高四个字节*/

} TAF_APS_DSFLOW_NV_STRU_EX;

typedef struct
{
    TAF_APS_DSFLOW_NV_STRU_EX    DsflowNvWan[5]; /*根据目前的使用情况，及NV结构，支持最多保存5个WAN流量*/
} TAF_APS_DSFLOW_NV_STRU_EXT;

/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_NAME
 结构说明  : 用于运营商定制的在PC上显示CDROM和SD 卡的字符串
 结构 ID   : 50108
*****************************************************************************/
#define DYNAMIC_CD_NAME_CHAR_NUM (28)
#define reserved_num_32  (32)
typedef struct
{
    VOS_UINT32 nv_status;
    VOS_UINT8 huawei_cdrom_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM];
    VOS_UINT8 huawei_sd_dynamic_name[DYNAMIC_CD_NAME_CHAR_NUM];
    VOS_UINT8 reserved[reserved_num_32];
}HUAWEI_DYNAMIC_NAME_STRU;


/*****************************************************************************
 结构名    : NV_HUAWEI_DYNAMIC_INFO_NAME
 结构说明  : 此NV项用于运营商定制的在PC上显示product manufacturer和configuration的字符串。
 结构 ID   : 50109
*****************************************************************************/
#define DYNAMIC_INFO_NAME_CHAR_NUM (40)
#define reserved_num_8  (8)
typedef struct
{
    VOS_UINT32 nv_status;
    VOS_UINT8 huawei_product_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    VOS_UINT8 huawei_manufacturer_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    VOS_UINT8 huawei_configuration_dynamic_name[DYNAMIC_INFO_NAME_CHAR_NUM];
    VOS_UINT8 reserved[reserved_num_8];
}HUAWEI_DYNAMIC_INFO_NAME_STRU;

/*****************************************************************************
 结构名    : CPE_TELNET_SWITCH_NVIM_STRU
 结构说明  : 此NV项用于控制CPE产品telnet功能是否打开。
 结构 ID   : 50501
*****************************************************************************/
typedef struct
{
    VOS_UINT8 nv_telnet_switch;
    VOS_UINT8 reserved;
}CPE_TELNET_SWITCH_NVIM_STRU;
typedef struct
{
    VOS_UINT8 InterfaceName[32];     /* 接口名称 */
    VOS_UINT8  MBIMEnable;            /* 是否启用MBIM,取值范围 [0-1] */
    VOS_UINT8  CdRom;                 /* 启用MBIM后，是否同时上报光盘,取值范围 [0-1] */
    VOS_UINT8  TCard;                 /* 启用MBIM后，是否同时上报T卡,取值范围 [0-1] */
    VOS_UINT8  MaxPDPSession;         /* 多PDP支持的最大PDP数量,取值范围 [1-8] */
    VOS_UINT16 IPV4MTU;               /* IPV4 MTU大小,取值范围 [296-1500] */
    VOS_UINT16 IPV6MTU;               /* IPV6 MTU大小,取值范围 [1280-65535] */
    VOS_UINT32 Reserved1;             /* 保留 */
    VOS_UINT32 Reserved2;             /* 保留 */
}NAS_WINBLUE_PROFILE_TYPE_STRU;


typedef  struct
{
    BSP_U8 ucNvActiveFlag;    /*是否激活BIP功能*/
    BSP_U8 ucRoamingEnable;  /*漫游时是否允许进行BIP业务*/
    BSP_U8 ucReserved[2];    /*对齐保留*/
}NV_BIP_FEATURE_STRU;

/*add by wanghaijie for simlock 3.0*/
typedef struct
{
    VOS_UINT8    nv_lock30[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock21[TAF_NVIM_HWLOCK_LEN];/*simlock 2.1 未使用*/
    VOS_UINT8    nv_lock20[TAF_NVIM_HWLOCK_LEN];/*simlock 2.0 未使用*/
    VOS_UINT8    reserved[TAF_NVIM_HWLOCK_LEN];
}NV_AUHT_OEMLOCK_STWICH_SRTU;

typedef struct
{
    VOS_UINT8    nv_lock30[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock21[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    nv_lock20[TAF_NVIM_HWLOCK_LEN];
    VOS_UINT8    reserved[TAF_NVIM_HWLOCK_LEN];
}NV_AUHT_SIMLOCK_STWICH_STRU;

typedef  struct
{
    BSP_U8 ucSimRefreshIndFlag;    /*SIM卡RESET上报使能*/
    BSP_U8 ucReserv;             /* C31之后的基线此位不用*/
    BSP_U8 ucReserv1;             /*对齐保留*/
    BSP_U8 ucReserv2;            /*对齐保留*/
}NV_SIM_CUST_FEATURE_STRU;


#define MMA_ROMA_WHITELIST_NV_SIZE                   (127)

typedef struct
{
    VOS_UINT8 ucNvActiveFlag;           /*是否激活漫游白名单，1表示激活，0不激活*/
    VOS_UINT8 aucRoamWhiteListData[MMA_ROMA_WHITELIST_NV_SIZE]; /* NV数据 */
} NV_ROAM_WHITELIST_DATA;

/*****************************************************************************
 结构名    : nv_mac_num
 结构说明  :定义每一种类型对应的设备MAC地址个数
 结构 ID   : 50517
*****************************************************************************/
typedef struct
{
    VOS_UINT8 lanmac_num;        /* LAN_MAC/Cradle MAC个数*/
    VOS_UINT8 wifimac_num;       /* WIFI_MAC 个数*/
    VOS_UINT8 btmac_num;        /* BLUETOOTH_MAC个数*/
    VOS_UINT8 usbmac_num;       /* USB_MAC个数*/
    VOS_UINT8 reserve[4];          /*预留字段*/
} NV_MAC_NUM_STRU;

/*****************************************************************************
 结构名    : SPY_TEMP_PROTECT_SAR_NV_STRU
 结构说明  : 此NV项用于控制温度保护降SAR方案
 结构 ID   : 50520
*****************************************************************************/
typedef struct
{
    VOS_UINT32  ulIsEnable;
    VOS_INT32   lReduceSARThreshold;
    VOS_INT32   lResumeSARThreshold;
    VOS_UINT32  ulTempOverCount;
    VOS_UINT32  ulTempResumeCount;
    VOS_INT32   lReserved_1;
} SPY_TEMP_PROTECT_SAR_NV_STRU;


#define  MMC_MAX_LEN  5
typedef struct
{
    VOS_UINT32                   aulRoamMCCList[MMC_MAX_LEN];/* 存储非漫游的国家码 */
}NAS_MMC_NVIM_ROAM_MCC_CMP_STRU;

/********************************************************************************
  结构名    : NV_LTE_TDD_PRIOR_STRU
  结构说明  : NV_LTE_TDD_PRIOR_STRU 结构
*****************************************************************************/
typedef  struct
{
    BSP_U8 ucActiveFlag;        /*LTE_TDD优先搜网功能使能*/
    BSP_U8 ucActNum;            /*配置不保存FDD频段的个数*/
    BSP_U8 ucReserved[2];       /*对齐保留*/
    BSP_U16 usBand[32];         /*配置不保存FDD的频段的NVID*/
}NV_LTE_TDD_PRIOR_STRU;

/********************************************************************************
  结构名    : NAS_PLMN_ID_STRU
  结构说明  : NAS_PLMN_ID_STRU 结构
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usMcc;   /* MCC,3 bytes */
    VOS_UINT16                          usMnc;   /* MNC,2 or 3 bytes */
}NAS_PLMN_ID_STRU;

/********************************************************************************
  结构名    : USER_SET_BAND_STRU
  结构说明  : USER_SET_BAND_STRU 结构
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulBandLow; /*低32位*/
    VOS_UINT32 ulBandHigh;/*高32位*/
} USER_SET_BAND_STRU;

/********************************************************************************
  结构名    : USER_SET_PLMN_BAND_STRU
  结构说明  : USER_SET_PLMN_BAND_STRU 结构
*****************************************************************************/
typedef struct
{
    NAS_PLMN_ID_STRU stPlmnId; /* PLMN*/
    USER_SET_BAND_STRU stLBand;/* 频段*/
} USER_SET_PLMN_BAND_STRU;

/********************************************************************************
  结构名    : NAS_NV_LTE_BANDLOCK_STRU
  结构说明  : NAS_NV_LTE_BANDLOCK_STRU 结构
*****************************************************************************/
typedef  struct
{
    VOS_UINT8    ucNvActiveFlag;               /* 是否激活开机锁频*/
    VOS_UINT8    ucPlmnNum;                    /*配置的PLMN的个数*/
    VOS_UINT8    ucReserved[2];                /*对齐保留*/
    USER_SET_PLMN_BAND_STRU  stPlmnBandId[10]; /*配置的PLMN ID及band*/
}NAS_NV_LTE_BANDLOCK_STRU;



typedef struct
{
    VOS_UINT8                           enUeMode;       /*卡类型，1---软卡，2---硬卡*/
    VOS_UINT8                           enMasterdata;   /*产线预置主卡数据，enMasterdata置为1*/
    VOS_UINT8                           enHashflg;      /*防篡改校验flg TRUE时需要校验*/
    VOS_UINT8                           aucRfu;         /*预留*/
}VSIM_UE_MODE_NV_STRU;

#define NV_VSIM_KEYLEN_MAX                   (256)         /*NV 中填充数据的最大长度*/
#define NV_VSIM_KEYHASH_MAX                  (32)         /*HASH 256 最长32 byte*/

typedef struct
{
    VOS_UINT32                          ulHashLen;
    VOS_UINT8                           aucHash[NV_VSIM_KEYHASH_MAX];
}NV_VSIM_HASHDATA_STRU;

typedef struct
{
    VOS_UINT32                          ulKeyLen;
    VOS_UINT8                            aucKey[NV_VSIM_KEYLEN_MAX];
}NV_VSIM_KEYDATA_STRU;

typedef struct
{
    VOS_UINT32                             enAlgorithm;
    VOS_UINT32                             ulDHLen;         /* DH算法生成因子的长度 */
    NV_VSIM_KEYDATA_STRU                   stCPrivateKey;   /* 单板测私钥 */
    NV_VSIM_KEYDATA_STRU                   stCPublicKey;    /* 单板测公钥 */
    NV_VSIM_KEYDATA_STRU                   stSPublicKey;    /* 服务器公钥 */
    NV_VSIM_HASHDATA_STRU                  stHashData;      /*存放明文密钥拼接后的HASH*/
}NV_VSIM_HVSDH_NV_STRU;


#define NV_VSIM_DIEID_MAXLEN                   (64)/*入参为32位的随机数，HUK加密后不会超过64*/
#define NV_VSIM_DIEID_HASHLEN                  (32)
typedef struct
{
    VOS_UINT32                           ulNumLen;
    VOS_UINT8                            aucNum[NV_VSIM_DIEID_MAXLEN];
    VOS_UINT32                           ulHashLen;
    VOS_UINT8                            aucHash[NV_VSIM_DIEID_HASHLEN];
}NV_VSIM_DIEID_STRU;


typedef struct
{
    VOS_UINT8    wifi_enable; 
    VOS_UINT8    ucReserved1;
    VOS_UINT8    ucReserved2;
    VOS_UINT8    ucReserved3;
}NV_SMS_CTRL_WIFI_STRU;

/* NV50578~NV50583 */
#define MAX_LTE_APN_IMSI_PREFIX_SUPPORT          10
#define MIN_LTE_APN_IMSI_PREFIX_SUPPORT          5  /* AT设置时，最少必须有mcc,mnc */
#define MAX_LTE_APN_IMSI_PREFIX_BCD_SUPPORT      (MAX_LTE_APN_IMSI_PREFIX_SUPPORT / 2)
#define MAX_LTE_ATTACH_APN_NAME_LEN              32
#define MAX_LTE_ATTACH_APN_USERNAME_LEN          32
#define MAX_LTE_ATTACH_APN_USERPWD_LEN           32
#define MAX_LTE_ATTACH_PROFILE_NAME_LEN          19

typedef struct
{
    VOS_UINT8 ucActiveFlag;
    VOS_UINT8 ucPdpType;
    VOS_UINT8 ucAuthType;
    VOS_UINT8 ucImsiPrefixLen;
    VOS_UINT8 aucImsiPrefixBcd[MAX_LTE_APN_IMSI_PREFIX_BCD_SUPPORT]; /*匹配的IMSI前缀的BCD码*/
    VOS_UINT8 ucApnLen;
    VOS_UINT8 aucApn[MAX_LTE_ATTACH_APN_NAME_LEN]; /*匹配的APN*/
    VOS_UINT8 ucUserNameLen;
    VOS_UINT8 aucUserName[MAX_LTE_ATTACH_APN_USERNAME_LEN]; /*匹配的APN用户名*/
    VOS_UINT8 ucPwdLen;
    VOS_UINT8 aucPwd[MAX_LTE_ATTACH_APN_USERPWD_LEN]; /*匹配的APN密码*/
    VOS_UINT8 ucProfileNameLen;
    VOS_UINT8 aucProfileName[MAX_LTE_ATTACH_PROFILE_NAME_LEN]; /* Profile的名称 */
}NV_LTE_ATTACH_PROFILE_STRU;

typedef struct{
    VOS_UINT8 ucActiveFlag;
    VOS_UINT8 ucPrefer0;            /* 定制APN与NV8451(如果激活)优先级，0:NV8451优先， 1:定制优先 */
    VOS_UINT8 aucReservd[6];        /* 预留，便于将来扩展 */
}NV_LTE_ATTACH_PROFILE_CTRL;
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif



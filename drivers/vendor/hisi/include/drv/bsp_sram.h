
 
#ifndef __BSP_MEMORY_H__
#define __BSP_MEMORY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_memmap.h"
#include "soc_onchiprom.h"
#include <product_config.h>

/*sram虚拟地址和物理地址转换的宏*/
#define SRAM_V2P(addr) (addr - HI_SRAM_MEM_ADDR_VIRT + HI_SRAM_MEM_ADDR)
#define SRAM_P2V(addr) (addr - HI_SRAM_MEM_ADDR + HI_SRAM_MEM_ADDR_VIRT)

#ifndef __ASSEMBLY__

struct uart_infor
{
	unsigned int flag;          /*flag which denotes wether this uart will be used*/
	unsigned int base_addr;     /*the register base address*/
	unsigned int interrupt_num; /*interrupt number*/
};
#if (FEATURE_ON == MBB_HWTEST)  
/*hardware test*/
typedef  struct
{
    unsigned int smem_hw_mode;             /* 自检模式*/
    unsigned int smem_hw_option;           /* 测试项选择*/
    unsigned int smem_reserved;            /* 保留*/
}hwtest_smem_info;
#define SRAM_HWTEST_SIZE    12
#define HW_TEST_MAGIC_NUM    (0x33557799)
#endif
#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define MAX_TEMPS 10
#define RESERVED_LEN 4
struct ss_coul_nv_info{
    int charge_cycles;
    int r_pcb; // uohm
    int v_offset_a;
    int v_offset_b;
    int c_offset_a;
    int c_offset_b;
    short temp[MAX_TEMPS];
    short real_fcc[MAX_TEMPS];
    short calc_ocv_reg_v;
    short calc_ocv_reg_c;
    short hkadc_batt_temp;
    short hkadc_batt_id_voltage;
    int start_cc;
    int ocv_temp;
    int limit_fcc;
    int reserved[RESERVED_LEN];
};

#define SRAM_COUL_SIZE    sizeof(struct ss_coul_nv_info)
#endif
#endif
#if ((FEATURE_ON == MBB_SIMLOCK_THREE) || (FEATURE_ON == MBB_SIMLOCK_FOUR))
#define CONFIDENTIAL_NV_OPR_SIZE    (12)
typedef  struct
{
    unsigned int smem_confidential_nv_opr_flag;   /* datalock,simlock NV操作允许 标志*/
    unsigned int smem_datalock_state;                  /* datalock 状态*/
    unsigned int smem_reserve;
}confidential_nv_opr_info;
#endif
/*占用空间较小的部分*/
#define SRAM_TEMP_PROTECT_SIZE 32
#define SRAM_DLOAD_SIZE 64
#define SRAM_DSP_MNTN_SIZE 32
struct dfs_ddr_cfg
{
    unsigned int ddr_freq;
    unsigned int ddr_reg_83c;
    unsigned int ddr_reg_abc;
    unsigned int ddr_reg_b3c;
    unsigned int ddr_reg_bbc;
    unsigned int ddr_reg_c3c;
};
typedef struct tag_SRAM_SMALL_SECTIONS
{
	unsigned int        SRAM_USB_ASHELL;
	struct uart_infor   UART_INFORMATION[3];                            /*three uarts:0/1/2*/
	unsigned int        SRAM_ONOFF[8];
	unsigned int        SRAM_DICC[8];				                    /*GU使用的DICC*/
	unsigned int        SRAM_DSP_DRV;
	unsigned int        SRAM_PCIE_INFO[64];			                    /*DSP镜像加载时使用*/
	unsigned int        SRAM_SEC_ROOTCA[65];
	unsigned int        SRAM_WDT_AM_FLAG;	                            /*连仿真器停狗标志*/
	unsigned int        SRAM_WDT_CM_FLAG;
	unsigned int        SRAM_BUCK3_ACORE_ONOFF_FLAG;                    /*BUCK3上下电标志*/
	unsigned int        SRAM_BUCK3_CCORE_ONOFF_FLAG;
	unsigned int        SRAM_CUR_CPUFREQ_PROFILE;                       /* current profile */
	unsigned int        SRAM_MAX_CPUFREQ_PROFILE;                       /* max profile */
	unsigned int        SRAM_MIN_CPUFREQ_PROFILE;                       /* min profile */
	unsigned int        SRAM_CPUFREQ_DOWN_FLAG[2];
	unsigned int        SRAM_CPUFREQ_DOWN_PROFILE[2];
#if (MBB_COMMON == FEATURE_ON)
    unsigned int        SRAM_MBB_COMMON;          /* for MBB COMMON driver */
#endif
#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    unsigned int        SRAM_COUL[SRAM_COUL_SIZE];/*库仑计NV信息存储*/
#endif
#endif
	/* 下面是不可修改区，其它项只能在前面加 */
	unsigned int        SRAM_REBOOT_INFO[8];		                    /* E5 开机信息区 不可以被修改*/
	unsigned int        SRAM_TEMP_PROTECT[SRAM_TEMP_PROTECT_SIZE];		/*温保使用的该地址不能被修改*/
	unsigned char       SRAM_DLOAD[SRAM_DLOAD_SIZE];			        /*升级模块使用，不可以被修改*/
	struct tagOcrShareData  SRAM_SEC_SHARE;			                    /*onchiprom启动时存放信息的标志位，放在SRAM的高地址处,不可修改*/

	unsigned char       SRAM_DSP_MNTN_INFO[SRAM_DSP_MNTN_SIZE];		    /* DSP邮箱异常时的可维可测信息 */
	struct dfs_ddr_cfg SRAM_DFS_DDRC_CFG[2];
	unsigned int 	    SRAM_DUMP_POWER_OFF_FLAG;
#ifdef FEATURE_E5_ONOFF
    unsigned int        SRAM_MBB_PD_CHARGE;          /* for MBB PDCHARGE */
#endif
#if (FEATURE_ON == MBB_HWTEST)  
    unsigned char   SRAM_HWTEST[SRAM_HWTEST_SIZE];   /*硬件自检*/
#endif
#if (FEATURE_ON == MBB_MLOG)  
    unsigned int   SRAM_ABNORAL_REBOOT;   /*MLOG异常重启*/
#endif
#if ((FEATURE_ON == MBB_SIMLOCK_THREE) || (FEATURE_ON == MBB_SIMLOCK_FOUR))
unsigned char SRAM_CONFIDENTIAL_NV_OPR[CONFIDENTIAL_NV_OPR_SIZE];
#endif

#if (FEATURE_ON == MBB_FEATURE_GATEWAY)
    unsigned int  SRAM_APP_START_FLAG;  /*app 是否启动标记*/
    unsigned int  SRAM_GATEWAY_WORK_MODE; /*网关工作模式*/
#endif
}SRAM_SMALL_SECTIONS;

#define SRAM_USB_CSHELL_ADDR             (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_USB_CSHELL))
#define SRAM_USB_ASHELL_ADDR             (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_USB_ASHELL))
#define SRAM_UART_INFO_ADDR              (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->UART_INFORMATION)
#define SRAM_AT_UART_FLAG_ADDR           (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_AT_UART_FLAG))
#define SRAM_ONOFF_ADDR                  (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_ONOFF)
#define MEMORY_AXI_DICC_ADDR             (unsigned int)((((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DICC))
#define SRAM_DSP_DRV_ADDR                (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DSP_DRV))
#define SRAM_PCIE_INFO_ADDR              (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_PCIE_INFO)
#define SRAM_SEC_ROOTCA_ADDR             (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_SEC_ROOTCA)
#define SRAM_WDT_AM_FLAG_ADDR            (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_WDT_AM_FLAG))
#define SRAM_WDT_CM_FLAG_ADDR            (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_WDT_CM_FLAG))
#define SRAM_BUCK3_ACORE_ONOFF_FLAG_ADDR (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_BUCK3_ACORE_ONOFF_FLAG))
#define SRAM_BUCK3_CCORE_ONOFF_FLAG_ADDR (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_BUCK3_CCORE_ONOFF_FLAG))
#define SRAM_CUR_CPUFREQ_PROFILE_ADDR    (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_CUR_CPUFREQ_PROFILE))
#define SRAM_MAX_CPUFREQ_PROFILE_ADDR    (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_MAX_CPUFREQ_PROFILE))
#define SRAM_MIN_CPUFREQ_PROFILE_ADDR    (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_MIN_CPUFREQ_PROFILE))
#define SRAM_DFS_DDRC_CFG_ADDR           (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DFS_DDRC_CFG)
#define SRAM_REBOOT_ADDR                 (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_REBOOT_INFO)
#define SRAM_TEMP_PROTECT_ADDR           (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_TEMP_PROTECT)
#define SRAM_DLOAD_ADDR                  (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DLOAD)
#define SRAM_SEC_SHARE                   (unsigned int)(&(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_SEC_SHARE))
#define SRAM_DSP_MNTN_INFO_ADDR          (unsigned int)((((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DSP_MNTN_INFO))
#define DSP_MAILBOX_PROTECT_FLAG_ADDR    (unsigned int)(&((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DSP_DRV)
#define DSP_MAILBOX_PROTECT_FLAG_SIZE    (sizeof(unsigned int))

#define SRAM_SUPPORT_ASHELL_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_USB_ASHELL)
#define SRAM_DUMP_POWER_OFF_FLAG_ADDR    (unsigned int)(&((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_DUMP_POWER_OFF_FLAG)
#if (MBB_COMMON == FEATURE_ON)
#define SRAM_MBB_COMMON_NUM              (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_MBB_COMMON)
#endif
#if (MBB_CHARGE == FEATURE_ON)

#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define SRAM_COUL_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_COUL)/*库仑计共享内存地址*/
#endif
#endif
#ifdef FEATURE_E5_ONOFF
#define SRAM_MBB_PD_CHARGE_ON     0x77778888
#define SRAM_MBB_PD_CHARGE_OFF    0x00000000
#define SRAM_MBB_PD_CHARGE_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_MBB_PD_CHARGE)
#endif
#if (FEATURE_ON == MBB_HWTEST)  
#define SRAM_HWTEST_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_HWTEST)/*硬件自检共享内存地址*/
#endif
#if (FEATURE_ON == MBB_MLOG)
#define SRAM_ABNORAL_REBOOT_NUM     0x6F545550
#define SRAM_ABNORAL_REBOOT_CLEAR   0x00000000
#define SRAM_ABNORMAL_REBOOT_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_ABNORAL_REBOOT)/*异常重启共享内存地址*/
#endif
#if ((FEATURE_ON == MBB_SIMLOCK_THREE) || (FEATURE_ON == MBB_SIMLOCK_FOUR))
#define SMEM_DATALOCK_STATE_NUM             (0X8D79F60A)  /*DATALOCK解锁状态魔术字 */
#define SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM   (0X9C7C021D)  /*允许读取DATALOCK\SIMLOCK NV魔术字 */
#define SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR (0x00000000)
#define SRAM_CONFIDENTIAL_NV_OPR_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_CONFIDENTIAL_NV_OPR)
#endif

#if (FEATURE_ON == MBB_FEATURE_GATEWAY)
#define MEMORY_AXI_APP_START_MAGIC           0xA6A62233
#define MEMORY_AXI_APP_DISABLE_MAGIC         0x6A6A3322

#define SRAM_APP_START_FLAG_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_APP_START_FLAG)
/*****************************************************************************
 函 数 名  : SetAppStartFlag()
 功能描述  : 设置app是否启动标志
 输入参数  : flag: MEMORY_AXI_APP_START_MAGIC
                  MEMORY_AXI_APP_DISABLE_MAGIC
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
#define SetAppStartFlag(mode) do{*(unsigned int *)SRAM_APP_START_FLAG_ADDR = (mode);}while(0)
/*****************************************************************************
 函 数 名  : GetAppStartFlag()
 功能描述  : 获取app是否启动标志
 输入参数  : 无
 输出参数  : 无
 返 回 值  : MEMORY_AXI_APP_DISABLE_MAGIC: 禁止启动app
             其它值: 启动app
*****************************************************************************/
#define GetAppStartFlag() (*(unsigned int *)SRAM_APP_START_FLAG_ADDR)

#define CONFIG_GATEWAY_NONE     (0x00)
#define CONFIG_GATEWAY_MODEM    (0x01)
#define CONFIG_GATEWAY_NDIS     (0x02)
#define SRAM_GATEWAY_WORK_MODE_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)SRAM_SMALL_SECTIONS_ADDR)->SRAM_GATEWAY_WORK_MODE)

/*
 *功能描述  : 设置适配网关的工作模式
 *输入参数  : mode: CONFIG_GATEWAY_NONE    非网关模式
                   CONFIG_GATEWAY_MODEM   网关模式modem形态
                   CONFIG_GATEWAY_NDIS    网关模式ndis形态
 */
#define SetGatewayWorkMode(Mode) do{*(unsigned int *)SRAM_GATEWAY_WORK_MODE_ADDR = (Mode);}while(0)

/*
 *功能描述  : 获取适配网关的工作模式
 *返回      : mode: CONFIG_GATEWAY_NONE    非网关模式
                   CONFIG_GATEWAY_MODEM   网关模式modem形态
                   CONFIG_GATEWAY_NDIS    网关模式ndis形态
 */
#define GetGatewayWorkMode() (*(unsigned int *)SRAM_GATEWAY_WORK_MODE_ADDR)
#endif

/*RTT 低功耗部分使用的AXI,存放标志等信息,共32字节*/
typedef struct tag_SRAM_RTT_SLEEP_FLAG
{
    unsigned int        SRAM_DSP_FLAG;          /*DSP使用，存放DSP是进入深睡或者是浅睡的标志*/
}SRAM_RTT_SLEEP_FLAG;

#define SRAM_DSP_SLEEP_FLAG_ADDR (unsigned int)(&(((SRAM_RTT_SLEEP_FLAG*)SRAM_RTT_SLEEP_FLAG_ADDR)->SRAM_DSP_FLAG))
#if(FEATURE_ON == MBB_DLOAD)
#define SMEM_DLOAD_FLAG_NUM          0X444C464E  /*DLFN*/
#define SMEM_SDUP_FLAG_NUM           0X53445550  /*SDUP*/
#define SMEM_DLOAD_FLAG_NUM          0X444C464E  /*DLFN*/
#define SMEM_SUOK_FLAG_NUM           0X53554F4B  /*SUOK  Scard Upgrade OK*/
#define SMEM_SUFL_FLAG_NUM           0X5355464C  /*SUFL   Scard Upgrade Fail*/
#define SMEM_SDNR_FLAG_NUM           0X53444E52  /*SDNR SD卡升级 NV自动恢复阶段  魔术字*/
#define SMEM_ONNR_FLAG_NUM           0X4F4E4E52  /*ONNR  在线 升级 NV自动恢复阶段  魔术字*/
#define SMEM_RECA_FLAG_NUM           0X5245434A  /*RECA*/
#define SMEM_RECB_FLAG_NUM           0X5245434B  /*RECB*/
#define SMEM_SWITCH_PUCI_FLAG_NUM    0X444E5350  /*NV自动恢复阶段PCUI端口形态*/
#define SMEM_FORCELOAD_FLAG_NUM      0X46434C44  /*FCLD Modem逃生标志 魔术字*/
#define SMEM_FORCELOAD_SUCCESS_NUM  0X4643534E /*FCSN  逃生成功魔术字，    
                                                       在线升级自动恢复也替换为该魔术字,   
                                                       后续下面两个自动恢复相关的魔术字不再使用 */
#define SMEM_ONLINE_AUTO_UPDATE_ENTER_NUM   0x4F415545 /*OAUE 在线升级自动恢复进入魔术字*/
#define SMEM_ONLINE_AUTO_UPDATE_SUCCESS_NUM 0x4F415553 /*OAUS 在线升级自动恢复成功魔术字*/
#define SMEM_BURN_UPDATE_FLAG_NUM           0x4255464E /*BUFN 烧片版本升级魔术字*/
#define SMEM_MULTIUPG_FLAG_NUM                  0x4D545550 /* MTUP 组播升级魔术字*/
#define SMEM_ONUP_FLAG_NUM           0x53555246    /* 在线升级魔术字 */


#define SMEM_DATALOCK_STATUS_FLAG_NUM   0X444C554C  /* DATALOCK 解锁状态魔术字 */

typedef  struct
{
    unsigned int smem_dload_flag;                  /* 标记是否进入下载模式 */
    unsigned int smem_new_recovery_flag;           /* 标识启动最新需要加载的recovery flag*/
    unsigned int smem_sd_upgrade;                  /* 记录SD卡升级升级标识位，升级时，从SD卡读取升级文件 */
    unsigned int smem_switch_pcui_flag;            /* 标识NV自动恢复阶段，启动时USB端口形态设置*/
    unsigned int smem_online_upgrade_flag;         /* 记录在线升级/SD卡升级NV 自动恢复阶段，APP不启动 */
    unsigned int smem_forceload_flag;              /* 标记是否进入逃生模式 */
    unsigned int smem_online_auto_updata_flag;     /* 标识在线升级自动恢复开始，成功*/ 
    unsigned int smem_burn_update_flag;            /* 烧片版本升级标识*/        
    unsigned int smem_multiupg_flag;               /* 组播升级标识*/ 
    unsigned int smem_update_times;                /* mlog升级次数统计标识*/
    unsigned int smem_datalock_status;             /* datalock解锁状态 */
    unsigned int smem_ften_usb_flag;               /* 标识下次启动时枚举丰田usb或micro usb */
    unsigned int smem_ften_reboot_flag;            /* 标识丰田usb在切换过程中单板是否已经重启过 */
    unsigned int smem_reserve8;                    /* 8 -- 10 暂时保留 */
    unsigned int smem_reserve9;
    unsigned int smem_reserve10;
}huawei_smem_info;
#endif  /*MBB_DLOAD*/

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
#define SMEM_FTEN_USB_FLAG           0x55335353
#define SMEM_MICRO_USB_FLAG           0x55225252
#define SMEM_FTEN_USB_SWITCH       0x55115151
#endif


#endif/*__ASSEMBLY__*/

#define SRAM_MEM_TOP_ADDR				(HI_SRAM_MEM_ADDR_VIRT + HI_SRAM_MEM_SIZE )

/*以下定义为GU使用的，底软应按照最新的使用方式*/
#define GLOBAL_AXI_MEM_BASE_ADDR		(HI_SRAM_MEM_ADDR_VIRT)
#define GLOBAL_AXI_MEM_BASE_ADDR_PHY	(HI_SRAM_MEM_ADDR)

#define SRAM_TOP_RESERVE_ADDR           (HI_SRAM_MEM_ADDR_VIRT)
#define SRAM_TOP_RESERVE_SIZE           (2*1024)
#define SRAM_SMALL_SECTIONS_ADDR      	(SRAM_TOP_RESERVE_ADDR)
#define SRAM_SMALL_SECTIONS_SIZE		(sizeof(SRAM_SMALL_SECTIONS))

/* MCU PM 1K */
#define SRAM_MCU_RESERVE_ADDR           (HI_SRAM_MEM_ADDR_VIRT+SRAM_TOP_RESERVE_SIZE)
#define SRAM_MCU_RESERVE_SIZE           (1*1024)

/* ICC通道(mcore与acore; mcore与ccore), 各2KB, 共4KB */
#define SRAM_ICC_ADDR                   (SRAM_MCU_RESERVE_ADDR + SRAM_MCU_RESERVE_SIZE)
#define SRAM_ICC_SIZE                   (4*1024)

/*RTT 低功耗部分用于存放标志信息等的区域*/
#define SRAM_RTT_SLEEP_FLAG_ADDR        (SRAM_ICC_ADDR + SRAM_ICC_SIZE)
#define SRAM_RTT_SLEEP_FLAG_SIZE        (32)

#define SRAM_GU_MAC_HEADER_ADDR         (SRAM_RTT_SLEEP_FLAG_ADDR + SRAM_RTT_SLEEP_FLAG_SIZE)
#define SRAM_GU_MAC_HEADER_SIZE         (56*1024)

/*K3V3 fpga GBBP DRX寄存器地址访问data abort屏蔽*/
#define SRAM_ADP_GDRX_ADDR              (SRAM_GU_MAC_HEADER_ADDR+SRAM_GU_MAC_HEADER_SIZE)
#if (defined BSP_CONFIG_HI3630) && (defined BSP_CONFIG_BOARD_SFT)
#define SRAM_ADP_GDRX_SIZE              (8*1024)
#else
#define SRAM_ADP_GDRX_SIZE              (0)
#endif

#define SRAM_TLPHY_ADDR                 (SRAM_ADP_GDRX_ADDR + SRAM_ADP_GDRX_SIZE)
#define SRAM_TLPHY_SIZE                 (12*1024)

#define SRAM_BUSSTRESS_ADDR				(SRAM_TLPHY_ADDR+SRAM_TLPHY_SIZE)
#ifdef   CONFIG_MODULE_BUSSTRESS
#define SRAM_BUSSTRESS_SIZE				(16*1024)
#else
#define SRAM_BUSSTRESS_SIZE				(0)
#endif

/*SRAM动态区*/
#define SRAM_DYNAMIC_SEC_ADDR           (SRAM_BUSSTRESS_ADDR + SRAM_BUSSTRESS_SIZE)
#define SRAM_DYNAMIC_SEC_SIZE           (SRAM_MEM_TOP_ADDR - SRAM_DYNAMIC_SEC_ADDR)

#ifdef __cplusplus
}
#endif

#endif



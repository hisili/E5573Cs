

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#include "AtParse.h"

#include "ATCmdProc.h"
/* Added by f62575 for SMALL IMAGE, 2012-1-3, begin */
#include "AtDeviceCmd.h"
#include "AtCheckFunc.h"
#include "DrvInterface.h"
/* Added by f62575 for SMALL IMAGE, 2012-1-3, end   */
#include "AtCmdMsgProc.h"

#include "phyoaminterface.h"

#include "AtInputProc.h"

#if(FEATURE_ON == FEATURE_LTE)
#include "LNvCommon.h"
#include "RrcNvInterface.h"
#include "msp_nvim.h"
#endif

#include "NasNvInterface.h"
#include "TafNvInterface.h"

/* ADD by c64416 for V9R1/V7R1 AT, 2013/09/18 begin */
#if (FEATURE_ON==FEATURE_LTE)
#include "at_lte_common.h"
#endif
/* ADD by c64416 for V9R1/V7R1 AT, 2013/09/18 end */

#if (FEATURE_ON == MBB_COMMON)
#include "RfNvId.h"
#include "LNvCommon.h"
#include "SysNvId.h"
#include "LPsNvInterface.h"
#include "GasNvInterface.h"
#include "wlan_if.h"
#endif
#if (FEATURE_ON == MBB_WPG_COMMON)
#include "MbbPsCsCommon.h"
#endif
/*add by wanghaijie for mbb_leds*/
#if (FEATURE_ON == MBB_LED)
#include "mbb_leds.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e767 修改人:罗建 107747;检视人:孙少华65952;原因:Log打印*/
#define    THIS_FILE_ID        PS_FILE_ID_AT_DEVICECMD_C
/*lint +e767 修改人:罗建 107747;检视人:sunshaohua*/

#define BAND_WIDTH_NUMS 6

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (FEATURE_ON == MBB_COMMON)
VOS_UINT32              g_SetTseLrfParValue = 0;
#endif

/* 命令受限标志宏定义说明
#define CMD_TBL_E5_IS_LOCKED        (0x00000001)     不受E5锁定控制的命令
#define CMD_TBL_PIN_IS_LOCKED       (0x00000002)     不受PIN码锁定控制的命令
#define CMD_TBL_IS_E5_DOCK          (0x00000004)     E5 DOCK命令
#define CMD_TBL_CLAC_IS_INVISIBLE   (0x00000008)     +CLAC命令中不输出显示的命令
*/

/* ^PSTANDBY命令解MAIN锁接口 */
#define LOCK_BUF_LEN       32

typedef enum
{
    APP_MAIN_LOCK,
    APP_MAIN_UNLOCK,
} app_main_lock_e;

static const char *off_state = "mem";
static const char *on_state = "on";
static const char *screen_state_path = "/sys/power/state";

VOS_UINT32                 g_ulNVRD = 0;
VOS_UINT32                 g_ulNVWR = 0;

#if (FEATURE_ON == MBB_LED)
 /* Begin Christopher Add 2015.11.06*/
#if defined(BSP_CONFIG_BOARD_E8378WS_210)
#define   MODE_LED_RED              "mode_led:red"
#define   MODE_LED_GREEN            "mode_led:green"
#define   MODE_LED_BLUE             "mode_led:blue"
#define   LTE_LED_RED               "lte_led:red"
#endif
 /* End Christopher Add 2015.11.06*/
#define   LTE_FLOW_LED    "lte_led:green"
/*呼吸灯枚举*/
#define   BRETH_LED_GREEN              "breath_led:green"
#define   NETWORK_LED_BLUE                 "lte_led:blue"
#define   LTE_FLOW_LED    "lte_led:green"
#define   FLOW_LED_GREEN  "flow_led:green"
#define   BAT2_LED_WHITE  "bat2_led:white"
#define   BAT3_LED_WHITE  "bat3_led:white"
#define   BAT4_LED_WHITE  "bat4_led:white"
#define   ROAMING_LED_GREEN  "roaming_led:green"  /*E5771h-937新增漫游灯，led name待和应用确定统一*/
#define   ROAMING_LED_BLUE   "roaming_led:blue"   /*E5771h-937新增漫游灯，led name待和应用确定统一*/
#define   LED_OFF         0
#ifdef BSP_CONFIG_BOARD_E5573CS
#define   LED_LTE_RED         "lte_led:red"
#define   LED_LTE_GREEN       "lte_led:green"
#define   LED_BATT_RED        "bat_led:red" 
#define   LED_BATT_GREEN      "bat_led:green" 
#define   LED_MSG_GREEN       "msg_led:green"
#endif
#ifdef BSP_CONFIG_BOARD_E5573BS
#define   LED_LTE_RED         "lte_led:red"
#define   LED_LTE_GREEN       "lte_led:green"
#define   LED_BATT_RED        "bat_led:red" 
#define   LED_BATT_GREEN      "bat_led:green" 
#define   LED_MSG_GREEN       "msg_led:green"
#endif
#endif
AT_PAR_CMD_ELEMENT_STRU g_astAtDeviceCmdTbl[] = {
    {AT_CMD_GTIMER,
    AT_SetGTimerPara,    AT_NOT_SET_TIME,    AT_QryGTimerPara,      AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^GTIMER",   (VOS_UINT8*)"(0-429496728)"},
#if(FEATURE_ON == MBB_COMMON)
    {AT_CMD_RSIM,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryRsimPara,        AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^RSIM",     VOS_NULL_PTR},
#if(FEATURE_ON == MBB_FEATURE_ESIM_SWITCH)
    {AT_CMD_TSIMSWITCH,
    AT_SetTSimSwitchPara, AT_NOT_SET_TIME, At_QryTSimSwitchPara, AT_NOT_SET_TIME, At_TestTSimSwitchPara,AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^TSIMSWITCH", (VOS_UINT8*)"(2,3)"},
#endif
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    {AT_CMD_USIMSTATUS,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryUsimstatusPara,        AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"*USIMSTATUS",     VOS_NULL_PTR},
#endif
#else
    {AT_CMD_RSIM,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryRsimPara,        AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^RSIM",     VOS_NULL_PTR},
#endif
    {AT_CMD_PHYNUM,
    AT_SetPhyNumPara,    AT_NOT_SET_TIME,    AT_QryPhyNumPara,      AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PHYNUM",   VOS_NULL_PTR},

    {AT_CMD_CSVER,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QryCsVer,           AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^CSVER",    VOS_NULL_PTR},

    {AT_CMD_QOS,
    At_SetQosPara,       AT_NOT_SET_TIME,    At_QryQosPara,         AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
   AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^QOS",      VOS_NULL_PTR},

    {AT_CMD_SDOMAIN,
    At_SetSDomainPara,   AT_NOT_SET_TIME,    At_QrySDomainPara,     AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SDOMAIN",  VOS_NULL_PTR},

    {AT_CMD_DPACAT,
    At_SetDpaCat,        AT_NOT_SET_TIME,    At_QryDpaCat,          AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^DPACAT",   VOS_NULL_PTR},

    {AT_CMD_HSSPT,
    AT_SetHsspt  ,       AT_NOT_SET_TIME,    AT_QryHsspt,         AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HSSPT",    (VOS_UINT8*)"(0,1,2,6)"},

    {AT_CMD_PLATFORM,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QryPlatForm,        AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PLATFORM", VOS_NULL_PTR},

    {AT_CMD_BSN,
    At_SetBsn,           AT_NOT_SET_TIME,    At_QryBsn,             AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^BSN",      VOS_NULL_PTR},
#if(FEATURE_ON == MBB_CHG_BQ27510)

    {AT_CMD_BATSN,
    At_SetBatSn,     AT_NOT_SET_TIME,    At_QryBatSn,      AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^BATSN",  VOS_NULL_PTR},
    {AT_CMD_FWUP,
    VOS_NULL_PTR,     AT_NOT_SET_TIME,    At_QryFwUp,      AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FWUP",  VOS_NULL_PTR},

#endif

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
    {
        AT_CMD_USB_SELECT,
        At_SetUsbSel,     AT_NOT_SET_TIME,    At_QryUsbSel,      AT_QRY_PARA_TIME,  At_TestUsbSel ,    AT_NOT_SET_TIME,
        VOS_NULL_PTR,        AT_NOT_SET_TIME,
        AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
        (VOS_UINT8*)"^USBSEL",  (VOS_UINT8*)"(0,1)"
    },
#endif


#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_SFM,
    At_SetSfm,          AT_SET_PARA_TIME,   At_QrySfm,            AT_NOT_SET_TIME,    At_TestSfm , AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SFM",     (VOS_UINT8*)"(0,1,2)"},
#else
#if ( FEATURE_ON == FEATURE_LTE )
    {AT_CMD_SFM,
    At_SetSfm,          AT_SET_PARA_TIME,   At_QrySfm,            AT_NOT_SET_TIME,    VOS_NULL_PTR , AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SFM",     (VOS_UINT8*)"(0,1)"},
#endif
#endif

    /* Added by f62575 for SMALL IMAGE, 2012-1-3, begin */
    {AT_CMD_TMODE,
    At_SetTModePara,     AT_SET_PARA_TIME,   At_QryTModePara,       AT_QRY_PARA_TIME ,  At_TestTmodePara , AT_TEST_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TMODE",    (VOS_UINT8*)"(0,1,2,3,4,11,12,13,14,15,16,17,18,19)"},
    /* Added by f62575 for SMALL IMAGE, 2012-1-3, end   */

    {AT_CMD_FCHAN,
    At_SetFChanPara,     AT_SET_PARA_TIME,   At_QryFChanPara,       AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_FCHAN_OTHER_ERR  ,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FCHAN",    (VOS_UINT8*)"(0-7),(0-255),(0-65535)"},

    {AT_CMD_FTXON,
    At_SetFTxonPara,     AT_SET_PARA_TIME,   At_QryFTxonPara,       AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_FTXON_OTHER_ERR  ,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FTXON",    (VOS_UINT8*)"(0,1,2)"},

    {AT_CMD_FDAC,
    AT_SetFDac,          AT_SET_PARA_TIME,   AT_QryFDac,            AT_NOT_SET_TIME,    At_TestFdacPara , AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FDAC",     (VOS_UINT8*)"(0-65536)"},


    {AT_CMD_FRXON,
    At_SetFRxonPara,     AT_SET_PARA_TIME,   At_QryFRxonPara,       AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_FRXON_OTHER_ERR  ,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FRXON",    (VOS_UINT8*)"(0-1)"},

    {AT_CMD_FPA,
    At_SetFpaPara,       AT_SET_PARA_TIME,   At_QryFpaPara,         AT_NOT_SET_TIME,    VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_FPA_OTHER_ERR  ,      CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPA",      (VOS_UINT8*)"(0-255)"},

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_FLNA,
    At_SetFlnaPara,      AT_SET_PARA_TIME,   At_QryFlnaPara,        AT_QRY_PARA_TIME,   At_TestFlnaPara ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_FLNA_OTHER_ERR  ,     CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FLNA",     (VOS_UINT8*)"(0-255)"},
#else
    {AT_CMD_FLNA,
    At_SetFlnaPara,      AT_SET_PARA_TIME,   At_QryFlnaPara,        AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_FLNA_OTHER_ERR  ,     CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FLNA",     (VOS_UINT8*)"(0-255)"},
#endif

    {AT_CMD_FRSSI,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QryFrssiPara,       AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FRSSI",    VOS_NULL_PTR},

    {AT_CMD_MDATE,
    AT_SetMDatePara,     AT_NOT_SET_TIME,    AT_QryMDatePara,       AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MDATE",    (VOS_UINT8*)"(@time)"},

    {AT_CMD_FACINFO,
    AT_SetFacInfoPara,   AT_NOT_SET_TIME,    AT_QryFacInfoPara,     AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FACINFO",  (VOS_UINT8*)"(0,1),(@valueInfo)"},


    {AT_CMD_SD,
    At_SetSD,            AT_NOT_SET_TIME,    At_QrySD,              AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_SD_CARD_OTHER_ERR ,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SD",       (VOS_UINT8*)"(0-4),(0-429496728),(0-3)"},

    {AT_CMD_GPIOPL,
    At_SetGPIOPL,        AT_SET_PARA_TIME,   At_QryGPIOPL,          AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^GPIOPL",   (VOS_UINT8*)"(@GPIOPL)"},

#if ( FEATURE_ON == FEATURE_LTE )
    {AT_CMD_GETEXBANDINFO,
    AT_SetExbandInfoPara,      AT_NOT_SET_TIME,  VOS_NULL_PTR,            AT_NOT_SET_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,       CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^GETEXBANDINFO",     (VOS_UINT8*)"(101-116)"},

    {AT_CMD_GETEXBANDTESTINFO,
    AT_SetExbandTestInfoPara,      AT_NOT_SET_TIME, VOS_NULL_PTR ,     AT_NOT_SET_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,       CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^GETEXBANDTESTINFO",      (VOS_UINT8*)"(101-116),(14,50,100,150,200)"},
    /* 生产NV恢复 */
    {AT_CMD_INFORRS,
    At_SetInfoRRS,       AT_SET_PARA_TIME,    VOS_NULL_PTR,          AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^INFORRS",  VOS_NULL_PTR},
#endif

#ifdef FEATURE_UPGRADE_TL
    {AT_CMD_INFORBU,
    atSetNVFactoryBack,  AT_SET_PARA_TIME,    VOS_NULL_PTR,          AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^INFORBU",  VOS_NULL_PTR},
#else
    {AT_CMD_INFORBU,
    At_SetInfoRBU,       AT_SET_PARA_TIME,    VOS_NULL_PTR,          AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^INFORBU",  VOS_NULL_PTR},
#endif

    {AT_CMD_DATALOCK,
    At_SetDataLock,      AT_SET_PARA_TIME,   At_QryDataLock,        AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^DATALOCK", (VOS_UINT8*)"(@nlockCode)"},

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_VERSION,
    At_SetVersionPara,        AT_NOT_SET_TIME,    At_QryVersion,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^VERSION",  VOS_NULL_PTR},
#else
    {AT_CMD_VERSION,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QryVersion,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^VERSION",  VOS_NULL_PTR},
#endif

    {AT_CMD_SIMLOCK,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QrySimLockPlmnInfo, AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SIMLOCK",  VOS_NULL_PTR},

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_MAXLCK_TIMES,
    At_SetMaxLockTimes,  AT_SET_PARA_TIME,   At_QryMaxLockTimes,    AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MAXLCKTMS", (VOS_UINT8*)"(0-255)"},
#else
    {AT_CMD_MAXLCK_TIMES,
    At_SetMaxLockTimes,  AT_SET_PARA_TIME,   At_QryMaxLockTimes,    AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MAXLCKTMS",(VOS_UINT8*)"(0-429496728)"},
#endif

    {AT_CMD_CALLSRV,
    At_SetCallSrvPara,   AT_NOT_SET_TIME,    At_QryCallSrvPara,     AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^CALLSRV",  (VOS_UINT8*)"(0,1)"},

    {AT_CMD_CSDFLT,
    At_SetCsdfltPara,    AT_NOT_SET_TIME,    At_QryCsdfltPara,      AT_QRY_PARA_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR,                       CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^CSDFLT",   (VOS_UINT8*)"(0,1)"},

    {AT_CMD_SECUBOOTFEATURE,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,    At_QrySecuBootFeaturePara, AT_NOT_SET_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SECUBOOTFEATURE",  (VOS_UINT8*)"(0,1)"},

    /* Modified by f62575 for B050 Project, 2012-2-3, begin   */
    {AT_CMD_SECUBOOT,
    At_SetSecuBootPara,  AT_SET_PARA_TIME,    At_QrySecuBootPara,    AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SECUBOOT", (VOS_UINT8*)"(0,1)"},
    /* Modified by f62575 for B050 Project, 2012-2-3, end */

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_TMMI,
    AT_SetTmmiPara,      AT_NOT_SET_TIME,    AT_QryTmmiPara,        AT_NOT_SET_TIME,   At_TestTmmiPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TMMI",  (VOS_UINT8*)"(0,1)"},
#else
    {AT_CMD_TMMI,
    AT_SetTmmiPara,      AT_NOT_SET_TIME,    AT_QryTmmiPara,        AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TMMI",  (VOS_UINT8*)"(0,1)"},
#endif

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_TCHRENABLE,
    AT_SetChrgEnablePara,AT_NOT_SET_TIME,    AT_QryChrgEnablePara,  AT_NOT_SET_TIME,   AT_TestChrgEnablePara, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TCHRENABLE",(VOS_UINT8*)"(0,1,2,3,4),(0,1,2)"},
#else
    /* V7R1要求使用"=?"查询充电使能状态 */
    {AT_CMD_TCHRENABLE,
    AT_SetChrgEnablePara,AT_NOT_SET_TIME,    AT_QryChrgEnablePara,  AT_NOT_SET_TIME,   AT_TestChrgEnablePara, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TCHRENABLE",(VOS_UINT8*)"(0,1,4)"},
#endif

    {AT_CMD_TCHRINFO,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryChrgInfoPara,    AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TCHRINFO",VOS_NULL_PTR},

    {AT_CMD_TSCREEN,
    AT_SetTestScreenPara,AT_NOT_SET_TIME,    VOS_NULL_PTR,          AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TSCREEN",  (VOS_UINT8*)"(0-255),(0-255)"},

    {AT_CMD_BATVOL,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryBatVolPara,      AT_QRY_PARA_TIME,  At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TBATVOLT",  VOS_NULL_PTR},

    {AT_CMD_WUPWD,
    AT_SetWebPwdPara,    AT_NOT_SET_TIME,    VOS_NULL_PTR,          AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUPWD",  (VOS_UINT8*)"(0,1),(@WUPWD)"},

    {AT_CMD_PRODTYPE,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryProdTypePara,    AT_QRY_PARA_TIME,  At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PRODTYPE",  VOS_NULL_PTR},

    {AT_CMD_FEATURE,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryFeaturePara,     AT_QRY_PARA_TIME,  At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SFEATURE",  VOS_NULL_PTR},

    {AT_CMD_PRODNAME,
    AT_SetProdNamePara,  AT_NOT_SET_TIME,    AT_QryProdNamePara,    AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PRODNAME",  (VOS_UINT8*)"(@ProductName)"},

    {AT_CMD_FWAVE,
    AT_SetFwavePara,     AT_SET_PARA_TIME,   VOS_NULL_PTR,          AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FWAVE",  (VOS_UINT8*)"(0-7),(0-65535)"},

    {AT_CMD_EQVER,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    AT_QryEqverPara,       AT_NOT_SET_TIME,   VOS_NULL_PTR,     AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^EQVER",  VOS_NULL_PTR},

    {AT_CMD_WIENABLE,
    AT_SetWiFiEnablePara, AT_NOT_SET_TIME, AT_QryWiFiEnablePara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIENABLE",(VOS_UINT8*)"(0,1,2)"},

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_WIMODE,
    AT_SetWiFiModePara,   AT_NOT_SET_TIME, AT_QryWiFiModePara,   AT_NOT_SET_TIME, At_TestWiFiModePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIMODE",  (VOS_UINT8*)"(0,1,2,3,4,5)"},
#else
    {AT_CMD_WIMODE,
    AT_SetWiFiModePara,   AT_NOT_SET_TIME, AT_QryWiFiModePara,   AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIMODE",  (VOS_UINT8*)"(0,1,2,3,4)"},
#endif
    
#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_WIBAND,
    AT_SetWiFiBandPara,   AT_NOT_SET_TIME, AT_QryWiFiBandPara,   AT_NOT_SET_TIME, At_TestWiFiBandPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIBAND",  (VOS_UINT8*)"(0,1,2,3)"},
#else
    {AT_CMD_WIBAND,
    AT_SetWiFiBandPara,   AT_NOT_SET_TIME, AT_QryWiFiBandPara,   AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIBAND",  (VOS_UINT8*)"(0,1)"},
#endif

    {AT_CMD_WIFREQ,
    AT_SetWiFiFreqPara,   AT_NOT_SET_TIME, AT_QryWiFiFreqPara,   AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIFREQ",  (VOS_UINT8*)"(0-65535),(@offset)"},

    {AT_CMD_WIRATE,
    AT_SetWiFiRatePara,   AT_NOT_SET_TIME, AT_QryWiFiRatePara,   AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIDATARATE",  (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_WIPOW,
    AT_SetWiFiPowerPara,  AT_NOT_SET_TIME, AT_QryWiFiPowerPara,  AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPOW",    (VOS_UINT8*)"(0-65535)"},

    {AT_CMD_WITX,
    AT_SetWiFiTxPara,     AT_NOT_SET_TIME, AT_QryWiFiTxPara,     AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WITX",     (VOS_UINT8*)"(0,1)"},

    {AT_CMD_WIRX,
    AT_SetWiFiRxPara,     AT_NOT_SET_TIME, AT_QryWiFiRxPara,     AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIRX",     (VOS_UINT8*)"(0,1),(@smac),(@dmac)"},

    {AT_CMD_WIRPCKG,
    AT_SetWiFiPacketPara, AT_NOT_SET_TIME, AT_QryWiFiPacketPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIRPCKG",  (VOS_UINT8*)"(0)"},

/* Add by z60575 for multi_ssid, 2012-9-5 begin */
    {AT_CMD_SSID,
    AT_SetWiFiSsidPara,   AT_NOT_SET_TIME, AT_QryWiFiSsidPara,   AT_NOT_SET_TIME, AT_TestSsidPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SSID",  (VOS_UINT8*)"(0-3),(@SSID)"},
/* Add by z60575 for multi_ssid, 2012-9-5 end */

    {AT_CMD_WIKEY,
    AT_SetWiFiKeyPara,    AT_NOT_SET_TIME, AT_QryWiFiKeyPara,    AT_NOT_SET_TIME, AT_TestWikeyPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIKEY",  (VOS_UINT8*)"(0-3),(@WIKEY)"},

    {AT_CMD_WILOG,
    AT_SetWiFiLogPara,    AT_NOT_SET_TIME, AT_QryWiFiLogPara,    AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WILOG",  (VOS_UINT8*)"(0,1),(@content)"},
    
#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_WIINFO,
    AT_SetWifiInfoPara,   AT_NOT_SET_TIME, VOS_NULL_PTR,         AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIINFO",  (VOS_UINT8*)"(0,1,2),(0-255)"},
#else
    {AT_CMD_WIINFO,
    AT_SetWifiInfoPara,   AT_NOT_SET_TIME, VOS_NULL_PTR,         AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIINFO",  (VOS_UINT8*)"(0,1),(0,1)"},
#endif

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_WICAL,
    AT_SetWifiCalPara, AT_NOT_SET_TIME, AT_QryWifiCalPara, AT_NOT_SET_TIME, AT_TestWifiCalPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICAL",  (VOS_UINT8*)"(0,1)"},

    {AT_CMD_WICALDATA,
    AT_SetWifiCalDataPara, AT_NOT_SET_TIME, AT_QryWifiCalDataPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALDATA",  (VOS_UINT8*)"(0-255),(0-32767),(0-32767),(0-32767),(0-32767),(0-32767),(@data)"},

    {AT_CMD_WICALTEMP,
    AT_SetWifiCalTempPara, AT_NOT_SET_TIME, AT_QryWifiCalTempPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALTEMP",  (VOS_UINT8*)"(0-255),(0-32767)"},

    {AT_CMD_WICALFREQ,
    AT_SetWifiCalFreqPara, AT_NOT_SET_TIME, AT_QryWifiCalFreqPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALFREQ",  (VOS_UINT8*)"(0-1),(0-32767)"},

    {AT_CMD_WICALPOW,
    AT_SetWifiCalPowPara, AT_NOT_SET_TIME, AT_QryWifiCalPowPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WICALPOW",  (VOS_UINT8*)"(0-1),(0-32767)"},

    {AT_CMD_NAVTYPE,
    AT_SetNavTypePara, AT_NOT_SET_TIME, AT_QryNavTypePara, AT_NOT_SET_TIME, At_TestNavTypePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVTYPE",  (VOS_UINT8*)"(0-255)"},

    {AT_CMD_NAVENABLE,
    AT_SetNavEnablePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVENABLE",  (VOS_UINT8*)"(0-1)"},

    {AT_CMD_NAVFREQ,
    AT_SetNavFreqPara, AT_NOT_SET_TIME, AT_QryNavFreqPara, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVFREQ",  (VOS_UINT8*)"(0-65535),(0-999),(0-999)"},

    {AT_CMD_NAVRX,
    VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNavRxPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NAVRX",  VOS_NULL_PTR},

    {AT_CMD_AUDIO,
    AT_SetAudioPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^AUDIO",  (VOS_UINT8*)"(0-255),(0-1),(0-1)"},

#endif/*END FOR MBB_COMMON*/
    {AT_CMD_WIPARANGE,
    AT_SetWifiPaRangePara, AT_NOT_SET_TIME, AT_QryWifiPaRangePara, AT_NOT_SET_TIME, AT_TestWifiPaRangePara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPARANGE",  (VOS_UINT8*)"(@gainmode)"},

    {AT_CMD_NVRD,
    AT_SetNVReadPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVRD",(VOS_UINT8*)"(0-65535)"},

    {AT_CMD_NVWR,
    AT_SetNVWritePara,    AT_SET_PARA_TIME,  VOS_NULL_PTR,        AT_NOT_SET_TIME,  VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NVWR",(VOS_UINT8*)"(0-65535),(0-2048),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data),(@data)"},
    /* Added by 傅映君/f62575 for AT Project, SIM卡保护标志确认, 2011/11/15, begin */
    {AT_CMD_CURC,
    At_SetCurcPara,      AT_NOT_SET_TIME,     At_QryCurcPara,     AT_QRY_PARA_TIME, VOS_NULL_PTR,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^CURC", (VOS_UINT8*)"(0-2)"},
    /* Added by 傅映君/f62575 for AT Project, SIM卡保护标志确认, 2011/11/15, end */


    {AT_CMD_SN,
    At_SetSnPara,        AT_NOT_SET_TIME,    VOS_NULL_PTR,           AT_NOT_SET_TIME,    At_CmdTestProcOK, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_DEVICE_OTHER_ERROR, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^SN",       VOS_NULL_PTR},

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_TBAT,
    AT_SetTbatPara,     AT_SET_PARA_TIME,   AT_QryTbatPara,  AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^TBAT",    (VOS_UINT8 *)"(0,1,2),(0,1),(0-65535)"},
#else
    /* Added by f62575 for SMALL IMAGE, 2012-1-3, begin */
    {AT_CMD_TBAT,
    AT_SetTbatPara,     AT_SET_PARA_TIME,   AT_QryTbatPara,  AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^TBAT",    (VOS_UINT8 *)"(0,1),(0,1),(0-65535),(0-65535)"},
#endif
    {AT_CMD_PSTANDBY,
    AT_SetPstandbyPara,     AT_SET_PARA_TIME,   VOS_NULL_PTR,  AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^PSTANDBY",    (VOS_UINT8 *)"(0-65535),(0-65535)"},

/* Add by z60575 for multi_ssid, 2012-9-5 begin */
    {AT_CMD_WIWEP,
    AT_SetWiwepPara,        AT_SET_PARA_TIME,   AT_QryWiwepPara,  AT_QRY_PARA_TIME,  AT_TestWiwepPara, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^WIWEP",    (VOS_UINT8 *)"(0-3),(@wifikey),(0-3)"},
/* Add by z60575 for multi_ssid, 2012-9-5 end */

    {AT_CMD_CMDLEN,
    AT_SetCmdlenPara,        AT_SET_PARA_TIME,   AT_QryCmdlenPara,  AT_QRY_PARA_TIME,  At_CmdTestProcOK, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^CMDLEN",    (VOS_UINT8 *)"(0-65535),(0-65535)"},

    {AT_CMD_TSELRF,
    AT_SetTseLrfPara,        AT_SET_PARA_TIME,   AT_QryTseLrfPara,  AT_QRY_PARA_TIME,  VOS_NULL_PTR, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"^TSELRF",    (VOS_UINT8 *)"(0-255),(0-255)"},
    /* Added by f62575 for SMALL IMAGE, 2012-1-3, end   */

    {AT_CMD_HUK,
    AT_SetHukPara,              AT_SET_PARA_TIME,   VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HUK",         (VOS_UINT8 *)"(@huk)"},

    {AT_CMD_FACAUTHPUBKEY,
    VOS_NULL_PTR,               AT_NOT_SET_TIME,    VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FACAUTHPUBKEY",       (VOS_UINT8 *)"(@Pubkey)"},

    {AT_CMD_IDENTIFYSTART,
    AT_SetIdentifyStartPara,    AT_SET_PARA_TIME,   VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^IDENTIFYSTART",       (VOS_UINT8 *)"(@Rsa)"},

    {AT_CMD_IDENTIFYEND,
    AT_SetIdentifyEndPara,      AT_SET_PARA_TIME,   VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^IDENTIFYEND",       VOS_NULL_PTR},

    {AT_CMD_SIMLOCKDATAWRITE,
    VOS_NULL_PTR,               AT_NOT_SET_TIME,    VOS_NULL_PTR,   AT_NOT_SET_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SIMLOCKDATAWRITE",    (VOS_UINT8 *)"(@SimlockData)"},

    {AT_CMD_PHONESIMLOCKINFO,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,    AT_QryPhoneSimlockInfoPara, AT_QRY_PARA_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PHONESIMLOCKINFO",    VOS_NULL_PTR},

    {AT_CMD_SIMLOCKDATAREAD,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,    AT_QrySimlockDataReadPara,  AT_QRY_PARA_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SIMLOCKDATAREAD",     VOS_NULL_PTR},

    {AT_CMD_PHONEPHYNUM,
    AT_SetPhonePhynumPara,  AT_SET_PARA_TIME,    AT_QryPhonePhynumPara,  AT_QRY_PARA_TIME,   AT_TestHsicCmdPara,   AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PHONEPHYNUM",         (VOS_UINT8 *)"(@type),(@Phynum)"},

    {AT_CMD_PORTCTRLTMP,
    AT_SetPortCtrlTmpPara,          AT_SET_PARA_TIME,   AT_QryPortCtrlTmpPara,  AT_NOT_SET_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PORTCTRLTMP",    (VOS_UINT8 *)"(@password)"},

    {AT_CMD_PORTATTRIBSET,
    AT_SetPortAttribSetPara,          AT_SET_PARA_TIME,   AT_QryPortAttribSetPara,  AT_QRY_PARA_TIME,  AT_TestHsicCmdPara,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PORTATTRIBSET",    (VOS_UINT8 *)"(0,1),(@password)"},

    {AT_CMD_SIMLOCKUNLOCK,
    AT_SetSimlockUnlockPara, AT_SET_PARA_TIME,  VOS_NULL_PTR,   AT_NOT_SET_TIME,    AT_TestSimlockUnlockPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SIMLOCKUNLOCK",    (VOS_UINT8 *)"(\"NET\",\"NETSUB\",\"SP\"),(pwd)"},

#if (FEATURE_ON == MBB_COMMON)
    {AT_CMD_TNETPORT,
    AT_SetTnetPortPara,    AT_NOT_SET_TIME,    AT_QryTnetPortPara,      AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TNETPORT",   VOS_NULL_PTR},

    {AT_CMD_EXTCHARGE,
    VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryExtChargePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^EXTCHARGE",   VOS_NULL_PTR},
    
    {AT_CMD_WUSITE,
    AT_SetWebSitePara,   AT_NOT_SET_TIME,    AT_QryWebSitePara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUSITE", (VOS_UINT8*)"(@WUSITE)"},

    {AT_CMD_WIPIN,
    AT_SetWiFiPinPara,   AT_NOT_SET_TIME,    AT_QryWiFiPinPara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPIN", (VOS_UINT8*)"(@WIPIN)"},

    {AT_CMD_WUUSER,
    AT_SetWebUserPara,   AT_NOT_SET_TIME,    AT_QryWebUserPara,     AT_NOT_SET_TIME,   At_CmdTestProcERROR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WUUSER", (VOS_UINT8*)"(@WUUSER)"},

    {AT_CMD_PORTLOCK,
    At_SetPortLockPara, AT_SET_PARA_TIME, At_QryPortLockPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^PORTLOCK",   VOS_NULL_PTR},

    {AT_CMD_TBATDATA,
    AT_SetTbatDataPara, AT_NOT_SET_TIME, At_QryTbatDataPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TBATDATA",   (VOS_UINT8 *)"(0,1),(0-255),(0-65535)"},

    {AT_CMD_WIPLATFORM,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryWiFiPlatformPara,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^WIPLATFORM",  VOS_NULL_PTR},

    {AT_CMD_ANTENNA,
    AT_SetAntennaPara,     AT_NOT_SET_TIME, AT_QryAntennaPara,     AT_NOT_SET_TIME, At_TestAntenna, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^ANTENNA",  (VOS_UINT8 *)"(0,1,2)"},
#endif

    {AT_CMD_FPLLSTATUS,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,    AT_QryFPllStatusPara,  AT_QRY_PARA_TIME,  VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPLLSTATUS",     (VOS_UINT8 *)"(0,1),(0,1)"},

    {AT_CMD_FPOWDET,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,    AT_QryFpowdetTPara,  AT_QRY_PARA_TIME,  VOS_NULL_PTR,  AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FPOWDET",     VOS_NULL_PTR},
#if (FEATURE_ON == MBB_SIMLOCK_THREE)
    {AT_CMD_SETHWLOCK,
    At_SetHWLock,        AT_SET_PARA_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SETHWLOCK",  VOS_NULL_PTR},
    
    {AT_CMD_TESTHWLOCK,
    At_TestHWlock,        AT_SET_PARA_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TESTHWLOCK",  VOS_NULL_PTR},
#endif

#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
    {AT_CMD_SETHWLOCK,
    At_SetHWLock,        AT_NOT_SET_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SETHWLOCK",  VOS_NULL_PTR},

    {AT_CMD_TESTHWLOCK,
    At_TestHWlock,        AT_NOT_SET_TIME,    VOS_NULL_PTR,         AT_QRY_PARA_TIME,  VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TESTHWLOCK",  VOS_NULL_PTR},
#endif /*MBB_SIMLOCK_FOUR*/

#if (MBB_COMMON == FEATURE_ON)
    {AT_CMD_TEMPINFO,
    VOS_NULL_PTR,     AT_NOT_SET_TIME, AT_QryTempInfo,     AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^TEMPINFO",  VOS_NULL_PTR},
    {AT_CMD_FTYRESET,
    AT_SetFtyResetPara, AT_NOT_SET_TIME, AT_QryFtyResetPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^FTYRESET",  (VOS_UINT8 *)"(0,1,2)"},
#endif
#if(FEATURE_ON == MBB_OPERATOR_VDF)
    {AT_CMD_SKU,
    At_SetSkuType, AT_SET_PARA_TIME,    At_QrySkuType,  AT_SET_PARA_TIME,   VOS_NULL_PTR, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SKU",(VOS_UINT8*)"(0-4)"},

    /*获取WiFi方案平台信息命令*/
    {AT_CMD_ACCOUNT,
    At_SetAccountType, AT_SET_PARA_TIME,    At_QryAccountType,  AT_SET_PARA_TIME,   VOS_NULL_PTR, AT_SET_PARA_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^ACCOUNT",(VOS_UINT8*)"(0-2)"},
#endif
#if(FEATURE_ON == MBB_CHG_BQ27510)
    {AT_CMD_COULCURRENT,
    VOS_NULL_PTR, AT_NOT_SET_TIME,    At_QryCoulcurrentType,  AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^COULCURRENT",(VOS_UINT8*)"(-65535-65535)"},
#endif

#if (FEATURE_ON == MBB_FEATURE_VSIM_HUK)
    {AT_CMD_SETDIEID,
    AT_SetDieId,           AT_SET_PARA_TIME,    AT_QryDieId,  AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SETKEY",      (VOS_UINT8*)"(1-20)"},
    {AT_CMD_GETDIEIDINFO,
    AT_GetDieIdInfo,        AT_SET_PARA_TIME,    VOS_NULL_PTR,  AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^GETKEYINFO",      (VOS_UINT8*)"(1-20)"},
#endif /*FEATURE_ON == MBB_FEATURE_VSIM_HUK*/
};


/* 示例: ^CMDX 命令是不受E5密码保护命令，且在+CLAC列举所有命令时不显示，第一个参数是不带双引号的字符串,
        第二个参数是带双引号的字符串，第三个参数是整数型参数

   !!!!!!!!!!!注意: param1和param2是示例，实际定义命令时应尽量定义的简短(可提高解析效率)!!!!!!!!!!!!!

    {AT_CMD_CMDX,
    At_SetCmdxPara, AT_SET_PARA_TIME, At_QryCmdxPara, AT_QRY_PARA_TIME, At_TestCmdxPara, AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_E5_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
    (VOS_UINT8*)"^CMDX", (VOS_UINT8*)"(@param1),(param2),(0-255)"},
*/


/*****************************************************************************
  3 函数实现
*****************************************************************************/
#if (FEATURE_ON == MBB_COMMON)

VOS_UINT32 At_CheckWiwepHex()
{
    int wepIndex = 0;
    VOS_UINT8 *PaucPara = gastAtParaList[1].aucPara;
    for(wepIndex = 0; wepIndex < gastAtParaList[1].usParaLen ; wepIndex++)
    {
        if(((PaucPara[wepIndex] >= '0') && (PaucPara[wepIndex] <= '9')) 
           || ((PaucPara[wepIndex] >= 'a') && (PaucPara[wepIndex] <= 'f')) 
           || ((PaucPara[wepIndex] >= 'A') && (PaucPara[wepIndex] <= 'F'))) 
        {
            //do nothing;
        }
        else
        {
            return AT_ERROR;
        }
    }

    if(wepIndex == gastAtParaList[1].usParaLen)
    {
        return AT_OK;
    }
    
    return AT_ERROR;
}

#endif


VOS_UINT32 At_TestTmodePara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                  "%s: %d",
                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                  g_stAtDevCmdCtrl.ucCurrentTMode);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 At_TestFdacPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength  = 0;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: (0-2047)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: (0-1023)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 At_RegisterDeviceCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astAtDeviceCmdTbl, sizeof(g_astAtDeviceCmdTbl)/sizeof(g_astAtDeviceCmdTbl[0]));
}

/* Added by f62575 for AT Project, 2011-10-28, begin */


VOS_UINT32 AT_TestSsidPara(VOS_UINT8 ucIndex)
{
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    /* Modified by z60575 for multi_ssid, 2012-9-5 begin */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    AT_WIFI_MAX_SSID_NUM);
    /* Modified by z60575 for multi_ssid, 2012-9-5 end */
    return AT_OK;
}


VOS_UINT32 AT_TestWikeyPara(VOS_UINT8 ucIndex)
{
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    AT_WIFI_KEY_NUM);
    return AT_OK;
}

/* Added by f62575 for AT Project, 2011-10-28, end */

/* Added by f62575 for SMALL IMAGE, 2012-1-3, begin   */

VOS_UINT32 AT_SetTbatPara(VOS_UINT8 ucIndex)
{
#if (FEATURE_ON == FEATURE_LTE)
    return atSetTBATPara(ucIndex);

#else
    AT_TBAT_OPERATION_DIRECTION_ENUM_UINT32 enTbatOperdirection;
    AT_TBAT_BATTERY_ADC_INFO_STRU           stAdcInfo;
    
#if (FEATURE_ON == MBB_COMMON)
    VOS_UINT32                              ulTbatType;
    VOS_INT32                               batInfo;
#else
    VOS_UINT32                              ulRet;
#endif

    /* 参数有效性检查 */
    /* ^TBAT命令设置命令至少带2个参数: 操作类型和方向 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

#if (FEATURE_ON == MBB_COMMON)
    if(2 == gucAtParaIndex)
    {
        if ((0 == gastAtParaList[0].usParaLen)
         || (0 == gastAtParaList[1].usParaLen))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (AT_TBAT_GET_FROM_UUT != gastAtParaList[1].ulParaValue)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        return AT_ERROR;
    }

    /*功能判断:若产品不支持电池，则直接返回ERROR*/
    if (BSP_MODULE_UNSUPPORT == DRV_GET_BATTERY_SUPPORT())
    {
        return AT_ERROR;
    }

    ulTbatType            = gastAtParaList[0].ulParaValue;
    enTbatOperdirection   = gastAtParaList[1].ulParaValue;

    if(AT_TBAT_GET_FROM_UUT == enTbatOperdirection)
    {
        /* 读取 */
        switch(ulTbatType)
        {
            case AT_TBAT_BATTERY_ANALOG_VOLTAGE:
                {
                    if (CHG_OK != chg_tbat_read(CHG_AT_BATTERY_LEVEL, &batInfo))          
                    {              
                        return AT_ERROR;         
                    }
                }
                break;
            case AT_TBAT_BATTERY_DIGITAL_VOLTAGE:
                {
                    if (CHG_OK != chg_tbat_read(CHG_AT_BATTERY_ADC, &batInfo))          
                    {              
                        return AT_ERROR;         
                    }
                }
                break;
            case AT_TBAT_BATTERY_VOLUME:
                {
                    if (CHG_OK != chg_tbat_read(CHG_AT_BATTERY_CAPACITY, &batInfo))          
                    {              
                        return AT_ERROR;         
                    }
                }
                break;
            default:
                return AT_ERROR; 
        }
        
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s:%d,%d",
                                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                        ulTbatType,
                                                        batInfo);
            
        return AT_OK;
    }
    else
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#else
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ^TBAT命令设置命令参数检查:
       数字电池电压设置操作必须有3.7V对应的HKADC值和4.2V对应的HKADC值
       数字电池电压查询操作无<value1>和<value2>参数
      */

    /* V3R2-不支持电池模拟电压 */
    if (AT_TBAT_BATTERY_ANALOG_VOLTAGE == gastAtParaList[0].ulParaValue)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    enTbatOperdirection = gastAtParaList[1].ulParaValue;
    if (AT_TBAT_READ_FROM_UUT == enTbatOperdirection)
    {
        if (2 != gucAtParaIndex)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        if ((4 != gucAtParaIndex)
         || (0 == gastAtParaList[2].usParaLen)
         || (0 == gastAtParaList[3].usParaLen))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    if (AT_TBAT_READ_FROM_UUT == enTbatOperdirection)
    {
        /* 调用底软接口获取电池数字电压 */
        if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                                 gastAtClientTab[ucIndex].opId,
                                                 DRV_AGENT_HKADC_GET_REQ,
                                                 VOS_NULL_PTR,
                                                 0,
                                                 I0_WUEPS_PID_DRV_AGENT))
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TBAT_SET;     /*设置当前操作模式 */
            return AT_WAIT_ASYNC_RETURN;                                                       /* 等待异步事件返回 */
        }
        else
        {
            return AT_ERROR;
        }
    }
    else
    {
        /* 写参数到电池校准用的NV项90 en_NV_Item_BATTERY_ADC */
        stAdcInfo.usMinAdc = (VOS_UINT16)gastAtParaList[2].ulParaValue;
        stAdcInfo.usMaxAdc = (VOS_UINT16)gastAtParaList[3].ulParaValue;
        ulRet = NV_WriteEx(MODEM_ID_0, en_NV_Item_BATTERY_ADC, &stAdcInfo, sizeof(stAdcInfo));
        if (NV_OK != ulRet)
        {
             AT_WARN_LOG("AT_SetTbatPara: Fail to write NV en_NV_Item_BATTERY_ADC.");
             return AT_ERROR;
        }

        return AT_OK;
    }
#endif
#endif
    /*
    根据参数不同进行下述操作:
    1.  支持用户输入AT^TBAT=1,0获取电池电压数字值；
    调用底软/OM接口获取电池数字值
    2.  支持用户输入AT^TBAT=1,1,<value1>,<value2>设置电池电压数字值，用于电池校准；
    写校准电压到NVID 90(en_NV_Item_BATTERY_ADC)，此处有疑问待确认，
    en_NV_Item_BATTERY_ADC中记录的是下述两个值，而AT命令仅一个参数，如何映射
    3.4V 电压对应的ADC值
    4.2V 电压对应的ADC值
    */
}


VOS_UINT32 AT_QryTbatPara(VOS_UINT8 ucIndex)
{
    /*
    调用底软接口获取电池安装方式:
    <mount type> 电池安装方式
    0 无电池
    1 可更换电池
    2 内置一体化电池
    */
#if (FEATURE_ON == MBB_COMMON)
    VOS_INT8       batInstallWay = -1;
#endif

    /*命令状态类型检查*/
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_BATTERY_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

#if (FEATURE_ON == MBB_COMMON)
    /* 查询电池安装方式接口调用 */
    batInstallWay = chg_MNTN_get_batt_state();
    
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    batInstallWay);
    return AT_OK;
#else
    if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                               gastAtClientTab[ucIndex].opId,
                                               DRV_AGENT_TBAT_QRY_REQ,
                                               VOS_NULL_PTR,
                                               0,
                                               I0_WUEPS_PID_DRV_AGENT))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TBAT_QRY;             /*设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                                              /* 等待异步事件返回 */
    }
    else
    {
        return AT_ERROR;
    }
#endif
}


static int set_screen_state(app_main_lock_e on)
{
    struct file *file_handle = NULL;
    char lock_buf[LOCK_BUF_LEN];
    int len = -1;
    mm_segment_t old_fs = get_fs();
    
    (void)memset(lock_buf, 0x0, sizeof(lock_buf));/*lint !e506*/
    
    if(APP_MAIN_LOCK == on)
    {
        len = snprintf(lock_buf, sizeof(lock_buf), "%s", on_state);
    }
    else if(APP_MAIN_UNLOCK == on)
    {
        len = snprintf(lock_buf, sizeof(lock_buf), "%s", off_state);
    }
    else
    {
        return AT_ERROR;
    }

    set_fs(KERNEL_DS);

    file_handle = filp_open(screen_state_path, O_RDWR, S_IRWXU);
    if(IS_ERR(file_handle))
    {
        printk("%s: fatal error opening \"%s\".\n", __func__,screen_state_path);
        set_fs(old_fs);
        return AT_ERROR;
    }
    
    len = file_handle->f_op->write(file_handle, lock_buf, len, &file_handle->f_pos);
    if(len < 0) 
    {
        printk("%s: fatal error writing \"%s\".\n", __func__,screen_state_path);
        (void)filp_close(file_handle, NULL);
        set_fs(old_fs);
        return AT_ERROR;
    }
    
    (void)filp_close(file_handle, NULL);
    set_fs(old_fs);

    return AT_OK;
}



VOS_UINT32 AT_SetPstandbyPara(VOS_UINT8 ucIndex)
{
    int ret = 0;
    DRV_AGENT_PSTANDBY_REQ_STRU         stPstandbyInfo;

    /* Added by c64416 for ^PSTANDBY low power proc, 2013-9-13, Begin */
#ifdef FEATURE_UPGRADE_TL

    TAF_MMA_PHONE_MODE_PARA_STRU        stPhoneModePara;

#endif
    /* Added by c64416 for ^PSTANDBY low power proc, 2013-9-13, End */

    /* ^PSTANDBY设置命令有且仅有2个参数: 进入待机状态的时间长度和单板进入待机状态的切换时间 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen)
     || (2 != gucAtParaIndex))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    At_FormatResultData(ucIndex, AT_OK); /*需要先回复OK*/

    /*保证返回数据发送完成*/
    VOS_TaskDelay(10);

    /*
    调用底软和OM接口使单板进入待机状态:
    ACPU上完成的操作：
    1、下电WIFI
    2、LED下电
    3、USB PowerOff

    发消息到C核，指示CCPU上完成的操作：
    1、通信模块下电
    2、关定时器
    3、关中断
    4、调用底软接口进入深睡
    */
    /* Modify by f62575 for V7代码同步, 2012-04-07, Begin   */
    stPstandbyInfo.ulStandbyTime = gastAtParaList[0].ulParaValue;
    stPstandbyInfo.ulSwitchTime   = gastAtParaList[1].ulParaValue;
    
#if (FEATURE_ON == MBB_COMMON)
#if (FEATURE_ON == MBB_WIFI) && (FEATURE_ON == CONFIG_WIFI_SDIO)
    extern int wlan_run_shell(const char *pshell);
    extern void mshci_wifi_power_off(void);
    wlan_run_shell("echo \"hi_mci.0\" > /sys/bus/platform/drivers/hi_mci/unbind");
    mshci_wifi_power_off();
#endif
#if defined(CONFIG_USB_GADGET_SNPS_DWC_OTG)
#if (FEATURE_ON == MBB_WIFI)
    extern void dwc_otg_wifi_pwctrl_down(void);
    dwc_otg_wifi_pwctrl_down();
#endif
#endif

#else

#if defined(FEATURE_WIFI_ON) && defined(CONFIG_USB_GADGET_SNPS_DWC_OTG)
    extern void dwc_otg_wifi_pwctrl_down(void);
    dwc_otg_wifi_pwctrl_down();
#endif  /*defined(FEATURE_WIFI_ON) && defined(CONFIG_USB_GADGET_SNPS_DWC_OTG)*/

#endif

    /* 待机命令执行之前，先解除应用mian锁 */
    ret = set_screen_state(APP_MAIN_UNLOCK);
    if(AT_OK != ret)
    {
        return AT_ERROR;
    }
/*E5573 关闭  LED*/
#if (FEATURE_ON == MBB_LED)
#if defined(BSP_CONFIG_BOARD_E5) || defined(BSP_CONFIG_BOARD_E5575S_210) || defined(BSP_CONFIG_BOARD_E5573S_156) \
    || defined(BSP_CONFIG_BOARD_E5573S_856) || defined(BSP_CONFIG_BOARD_E5573S_853) \
    || defined(BSP_CONFIG_BOARD_E5573S_607) || defined(BSP_CONFIG_BOARD_E5575S_320)
    ret = led_kernel_status_set(LED_NODE_4,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LTE_FLOW_LED,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_5,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
#endif

/*E5575s-210 关闭所有灯*/
#if defined(BSP_CONFIG_BOARD_E5575S_210) || defined(BSP_CONFIG_BOARD_E5575S_320)
    /*关闭呼吸灯蓝灯*/
    ret = led_kernel_status_set(LED_NODE_2,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    /*关闭电池蓝灯*/
    ret = led_kernel_status_set(LED_NODE_5,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    /*关闭蓝色网络灯*/
    ret = led_kernel_status_set(NETWORK_LED_BLUE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
#endif
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    ret = led_kernel_status_set(LTE_FLOW_LED,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(FLOW_LED_GREEN,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_4,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_5,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
#endif
#if defined(BSP_CONFIG_BOARD_E5771S_852)
    ret = led_kernel_status_set(LED_NODE_1,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_2,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_3,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(NETWORK_LED_BLUE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(BAT2_LED_WHITE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(BAT3_LED_WHITE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(BAT4_LED_WHITE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
#endif
#if defined(BSP_CONFIG_BOARD_E5771H_937)
    ret = led_kernel_status_set(LED_NODE_1,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_2,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_3,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(NETWORK_LED_BLUE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(BAT2_LED_WHITE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(BAT3_LED_WHITE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(BAT4_LED_WHITE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(ROAMING_LED_GREEN,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(ROAMING_LED_BLUE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
#endif
#if defined(BSP_CONFIG_BOARD_E8378WS_210)

    ret = led_kernel_status_set(LED_NODE_1,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    
    ret = led_kernel_status_set(LED_NODE_2,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_NODE_3,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    
    ret = led_kernel_status_set(LED_NODE_4,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
   
    ret = led_kernel_status_set(MODE_LED_RED,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(MODE_LED_GREEN,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }

    ret = led_kernel_status_set(MODE_LED_BLUE,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }	
		
    ret = led_kernel_status_set(LTE_LED_RED,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
#endif

/*E5573Cs-609 关闭所有灯*/
#if (defined(BSP_CONFIG_BOARD_E5573CS) || defined(BSP_CONFIG_BOARD_E5573BS))
    ret = led_kernel_status_set(LED_LTE_RED,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }    
    ret = led_kernel_status_set(LED_LTE_GREEN,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_BATT_RED,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_BATT_GREEN,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }
    ret = led_kernel_status_set(LED_MSG_GREEN,LED_OFF);
    if(VOS_OK != ret)
    {
        ;/*for lint*/
    }

#endif
#endif
    
    DRV_PWRCTRL_STANDBYSTATEACPU(stPstandbyInfo.ulStandbyTime, stPstandbyInfo.ulSwitchTime);

    /*发送消息到c核*/
    if (TAF_SUCCESS != AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                           gastAtClientTab[ucIndex].opId,
                           DRV_AGENT_PSTANDBY_SET_REQ,
                           &stPstandbyInfo,
                           sizeof(stPstandbyInfo),
                           I0_WUEPS_PID_DRV_AGENT))
    {
        AT_ERR_LOG("AT_SetPstandbyPara: AT_FillAndSndAppReqMsg fail.");
    }

    /* Added by c64416 for ^PSTANDBY low power proc, 2013-9-13, Begin */
    /* V7R2采用关机进入低功耗流程流程 */
#ifdef FEATURE_UPGRADE_TL

    stPhoneModePara.PhMode = TAF_PH_MODE_MINI;

    if (VOS_TRUE == TAF_MMA_PhoneModeSetReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stPhoneModePara))
    {
        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PSTANDBY_SET;

        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }
#endif
    /* Added by c64416 for ^PSTANDBY low power proc, 2013-9-13, End */

    return AT_SUCCESS;
}

#if(FEATURE_ON == FEATURE_LTE)
VOS_UINT32 AT_SetExbandInfoPara(VOS_UINT8 ucIndex)
{
    LTE_COMM_NON_STANDARD_BAND_COMM_STRU stLpsNonstandBand = {0};
    VOS_UINT32 ulRet  = AT_OK;
    VOS_UINT32 ulNvId = 0;

    /* 1、AT命令类型是否正确 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2、参数个数是否符合要求 */
    if(1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulNvId = gastAtParaList[0].ulParaValue -LTE_COMM_NONSTANDARD_BAND_BEGIN  + EN_NV_ID_BANDNon1_BAND_INFO  ;
    if((ulNvId < EN_NV_ID_BANDNon1_BAND_INFO )||(ulNvId > EN_NV_ID_BANDNon16_BAND_INFO))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = NVM_Read(ulNvId, (VOS_VOID*) &stLpsNonstandBand, sizeof(LTE_COMM_NON_STANDARD_BAND_COMM_STRU));
     if(  NV_OK != ulRet)
     {
        vos_printf("read non stand band nv fail,ulNvId = %d,ulRet = %d!\n",ulNvId,ulRet);
        return AT_ERROR;
     }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:BANDNO:%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     stLpsNonstandBand.ucBandID);


    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                     "%s:DUPLEX:%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     stLpsNonstandBand.enBandMode);

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                   "%s:FREQ:%d,%d,%d,%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    stLpsNonstandBand.stUlFreqInfo.usFLow,
                                                    stLpsNonstandBand.stUlFreqInfo.usFHigh,
                                                    stLpsNonstandBand.stDlFreqInfo.usFLow,
                                                    stLpsNonstandBand.stDlFreqInfo.usFHigh
                                                 );

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                   "%s:CHANNUM:%d,%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    stLpsNonstandBand.stUlFreqInfo.usRangOfNLow,
                                                    stLpsNonstandBand.stDlFreqInfo.usRangOfNLow);

     return AT_OK;
}

VOS_UINT32 AT_SetExbandTestInfoPara(VOS_UINT8 ucIndex)
{
    LTE_COMM_NON_STANDARD_BAND_COMM_STRU stLpsNonstandBand = {0};
    VOS_UINT32 ulRet  = AT_OK;
    VOS_UINT32 ulNvId = 0,ulArrayID=0 ,i=0;

    VOS_UINT32 BandWidthArray[BAND_WIDTH_NUMS]= {14,30,50,100,150,200};

    /* 1、AT命令类型是否正确 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 2、参数个数是否符合要求 */
    if(2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulNvId = gastAtParaList[0].ulParaValue -LTE_COMM_NONSTANDARD_BAND_BEGIN  + EN_NV_ID_BANDNon1_BAND_INFO  ;

    if((ulNvId  < EN_NV_ID_BANDNon1_BAND_INFO ) ||(ulNvId > EN_NV_ID_BANDNon16_BAND_INFO))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = NVM_Read(ulNvId, (VOS_VOID*) &stLpsNonstandBand, sizeof(LTE_COMM_NON_STANDARD_BAND_COMM_STRU));

    if(  NV_OK != ulRet)
    {
        vos_printf("read non stand band nv fail,ulRet = %d!\n",ulRet);
        return AT_ERROR;
    }

    ulArrayID = BandWidthArray[BAND_WIDTH_NUMS -1];

    for(i = 0 ; i < BAND_WIDTH_NUMS; i++)
    {
        if( BandWidthArray[i] == gastAtParaList[1].ulParaValue)
        {
            ulArrayID = i;
            break;
        }
    }

    gstAtSendData.usBufLen = 0 ;

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                     "%s:FREQS:%d,%d,%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                     stLpsNonstandBand.stTestInfo[ulArrayID].usTestFreqs[0],
                                                     stLpsNonstandBand.stTestInfo[ulArrayID].usTestFreqs[1],
                                                     stLpsNonstandBand.stTestInfo[ulArrayID].usTestFreqs[2]);

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                   "%s:MAXPOWTOLERANCE:%d,%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    stLpsNonstandBand.stUePowerClassInfo.sPowerToleranceHigh,
                                                    stLpsNonstandBand.stUePowerClassInfo.sPowerToleranceLow
                                                 );

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                   "%s:REFSENSPOW:%d\r\n",
                                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                   stLpsNonstandBand.stTestInfo[ulArrayID].sRxRefSensPower);
      return AT_OK;
}
#endif

/* Modify by z60575 for multi_ssid, 2012-9-5 begin */

VOS_UINT32 AT_WriteWiWep(
    VOS_UINT32                          ulIndex,
    VOS_UINT8                           aucWiWep[],
    VOS_UINT16                          usWiWepLen,
    TAF_AT_MULTI_WIFI_SEC_STRU         *pstWifiSecInfo,
    VOS_UINT8                           ucGroup
)
{
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulLoop;
    VOS_UINT8                          *paucWifiWepKey;

    /* 因为仅修改en_NV_Item_WIFI_KEY中的WIFI KEY子项，所以需要先获取全部NV值，然后更新修改部分 */
    ulRet = NV_ReadEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY, pstWifiSecInfo, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU));
    if (NV_OK != ulRet)
    {
        AT_WARN_LOG("AT_WriteWiWep: Fail to read en_NV_Item_WIFI_KEY.");
        return AT_ERROR;
    }

    /* 根据index获取NV中保存的密码 */
    paucWifiWepKey = ((0 == ulIndex) ? pstWifiSecInfo->aucWifiWepKey1[ucGroup] :
                      ((1 == ulIndex) ? pstWifiSecInfo->aucWifiWepKey2[ucGroup] :
                      ((2 == ulIndex) ? pstWifiSecInfo->aucWifiWepKey3[ucGroup] : pstWifiSecInfo->aucWifiWepKey4[ucGroup])));

    /* 判断新的WIFI KEY与NV中记录的是否一致 */
    for (ulLoop = 0; ulLoop < AT_NV_WLKEY_LEN; ulLoop++)
    {
        if (paucWifiWepKey[ulLoop] != aucWiWep[ulLoop])
        {
            break;
        }
    }

    /* 判断新的WIFI KEY与NV中记录的一致则直接返回操作完成*/
    if (AT_NV_WLKEY_LEN == ulLoop)
    {
        return AT_OK;
    }

    /* 更新数据到NV项en_NV_Item_WIFI_KEY */
    PS_MEM_SET(paucWifiWepKey, 0, AT_NV_WLKEY_LEN);

    PS_MEM_CPY(paucWifiWepKey, aucWiWep, usWiWepLen);

    ulRet = NV_WriteEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY, pstWifiSecInfo, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU));
    if (NV_OK != ulRet)
    {
         AT_WARN_LOG("AT_WriteWiWep: Fail to write NV en_NV_Item_WIFI_KEY.");
         return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_SetWiwepPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulIndex;
    VOS_UINT8                           ucGroup;
    VOS_UINT32                          ulRet;
    TAF_AT_MULTI_WIFI_SEC_STRU         *pstWifiSecInfo;

    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    /* 输入参数检查: 有且仅有< index >和< content >两个参数 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

#if (FEATURE_ON == MBB_COMMON)
    if(3 != gucAtParaIndex)
    {
        return AT_ERROR;
    }
#endif

    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen)
     || (0 == gastAtParaList[2].usParaLen))
    {
        return AT_ERROR;
    }

    /* WIFI key 操作受DATALOCK保护 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }

    /* < index >必须在0-3范围内，< content >长度小于NV_WLKEY_LEN */
    ulIndex = gastAtParaList[0].ulParaValue;
#if (FEATURE_ON == MBB_COMMON)
    if (ulIndex >= AT_WIWEP_CARD_WIFI_KEY_TOTAL)
    {
        return AT_ERROR;
    }
#else
    if (ulIndex > AT_WIWEP_CARD_WIFI_KEY_TOTAL)
    {
        return AT_ERROR;
    }
#endif

    if (gastAtParaList[1].usParaLen > AT_NV_WLKEY_LEN)
    {
        return AT_ERROR;
    }

#if (FEATURE_ON == MBB_COMMON)
    /*若WIWEP长度为5或13位ASCII码 、10位或者26位16进制，则参数正确，否则返回错误*/
    if ((gastAtParaList[1].usParaLen == AT_NV_WLKEY_LEN3) 
        || (gastAtParaList[1].usParaLen == AT_NV_WLKEY_LEN4))
    {
        ulRet = At_CheckWiwepHex();
        if (ulRet == AT_ERROR)
        {
            return AT_ERROR;
        }
    }
    else if((gastAtParaList[1].usParaLen == AT_NV_WLKEY_LEN1) 
        || (gastAtParaList[1].usParaLen == AT_NV_WLKEY_LEN2))
    {
        // do nothing;
    }
    else
    {
        return AT_ERROR;
    }
#endif

    ucGroup = (VOS_UINT8)gastAtParaList[2].ulParaValue;

    if (ucGroup >= AT_WIFI_MAX_SSID_NUM)
    {
        return AT_ERROR;
    }

    /* 组装WIFI KEY到NV结构并更新数据到NV项en_NV_Item_WIFI_KEY */
    pstWifiSecInfo = (TAF_AT_MULTI_WIFI_SEC_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                                                  sizeof(TAF_AT_MULTI_WIFI_SEC_STRU));
    if (VOS_NULL_PTR == pstWifiSecInfo)
    {
        return AT_ERROR;
    }

    ulRet = AT_WriteWiWep(ulIndex,
                          gastAtParaList[1].aucPara,
                          gastAtParaList[1].usParaLen,
                          pstWifiSecInfo,
                          ucGroup);

    PS_MEM_FREE(WUEPS_PID_AT, pstWifiSecInfo);

    return ulRet;
}


VOS_UINT32 AT_QryWiwepPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulLoop;
    VOS_UINT16                          usLength;
    TAF_AT_MULTI_WIFI_SEC_STRU         *pstWifiSecInfo;
    VOS_UINT8                           aucWifiWepKey[AT_NV_WLKEY_LEN + 1];
    VOS_UINT8                           aucWepKeyLen1[AT_WIFI_MAX_SSID_NUM];
    VOS_UINT8                           aucWepKeyLen2[AT_WIFI_MAX_SSID_NUM];
    VOS_UINT8                           aucWepKeyLen3[AT_WIFI_MAX_SSID_NUM];
    VOS_UINT8                           aucWepKeyLen4[AT_WIFI_MAX_SSID_NUM];
    VOS_UINT8                           ucWepKeyNum;

    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    /* 为读取WIFI KEY申请内存，读NV项en_NV_Item_WIFI_KEY获取WIFI KEY信息 */
    pstWifiSecInfo = (TAF_AT_MULTI_WIFI_SEC_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT,
                                                      sizeof(TAF_AT_MULTI_WIFI_SEC_STRU));
    if (VOS_NULL_PTR == pstWifiSecInfo)
    {
        return AT_ERROR;
    }

    ulRet = NV_ReadEx(MODEM_ID_0, en_NV_Item_MULTI_WIFI_KEY, pstWifiSecInfo, sizeof(TAF_AT_MULTI_WIFI_SEC_STRU));
    if (NV_OK != ulRet)
    {
        AT_WARN_LOG("AT_QryWiwepPara: Fail to read en_NV_Item_WIFI_KEY.");
        PS_MEM_FREE(WUEPS_PID_AT, pstWifiSecInfo);
        return AT_ERROR;
    }

    ucWepKeyNum = 0;
    PS_MEM_SET(aucWepKeyLen1, 0, sizeof(aucWepKeyLen1));
    PS_MEM_SET(aucWepKeyLen2, 0, sizeof(aucWepKeyLen2));
    PS_MEM_SET(aucWepKeyLen3, 0, sizeof(aucWepKeyLen3));
    PS_MEM_SET(aucWepKeyLen4, 0, sizeof(aucWepKeyLen4));

    for (ulLoop = 0; ulLoop < AT_WIFI_MAX_SSID_NUM; ulLoop++)
    {
        /* KEY1对应的NV不空表示KEY1有效 */
        aucWepKeyLen1[ulLoop] = (VOS_UINT8)VOS_StrLen((VOS_CHAR*)pstWifiSecInfo->aucWifiWepKey1[ulLoop]);
        if (0 != aucWepKeyLen1[ulLoop])
        {
            ucWepKeyNum++;
        }

        aucWepKeyLen2[ulLoop] = (VOS_UINT8)VOS_StrLen((VOS_CHAR*)pstWifiSecInfo->aucWifiWepKey2[ulLoop]);
        if (0 != aucWepKeyLen2[ulLoop])
        {
            ucWepKeyNum++;
        }

        aucWepKeyLen3[ulLoop] = (VOS_UINT8)VOS_StrLen((VOS_CHAR*)pstWifiSecInfo->aucWifiWepKey3[ulLoop]);
        if (0 != aucWepKeyLen3[ulLoop])
        {
            ucWepKeyNum++;
        }

        aucWepKeyLen4[ulLoop] = (VOS_UINT8)VOS_StrLen((VOS_CHAR*)pstWifiSecInfo->aucWifiWepKey4[ulLoop]);
        if (0 != aucWepKeyLen4[ulLoop])
        {
            ucWepKeyNum++;
        }
    }

    /* 未解锁时，需要返回已定制0组 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        ucWepKeyNum = 0;
        PS_MEM_SET(aucWepKeyLen1, 0, sizeof(aucWepKeyLen1));
        PS_MEM_SET(aucWepKeyLen2, 0, sizeof(aucWepKeyLen2));
        PS_MEM_SET(aucWepKeyLen3, 0, sizeof(aucWepKeyLen3));
        PS_MEM_SET(aucWepKeyLen4, 0, sizeof(aucWepKeyLen4));
    }

    /*
    满足工位对接要求，需要输出工位支持的全部数20个WiFi WEP（WIFI的KEY）
    打印四组单板支持的WIFI KEY信息
    填充16行^WIWEP: <index>,与工位要求的20个的WiFi WEP（WIFI的KEY）一致，
    */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s:%d%s",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                     ucWepKeyNum,
                                     gaucAtCrLf);

    for (ulLoop = 0; ulLoop < AT_WIFI_MAX_SSID_NUM; ulLoop++)
    {
        if (0 != aucWepKeyLen1[ulLoop])
        {
            /* wifikey1 */
            PS_MEM_SET(aucWifiWepKey, 0, AT_NV_WLKEY_LEN + 1);

            PS_MEM_CPY(aucWifiWepKey,
                       pstWifiSecInfo->aucWifiWepKey1[ulLoop],
                       AT_NV_WLKEY_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            #if (FEATURE_ON == MBB_COMMON)
                                                "%s:%d,%s%s%s,%d%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                AT_WIFI_WEP_INDEX0,
                                                gaucAtQuotation,aucWifiWepKey,gaucAtQuotation,
                                            #else
                                                "%s:%d,%s,%d%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                0,
                                                aucWifiWepKey,
                                            #endif
                                                ulLoop,
                                                gaucAtCrLf);
        }

        if (0 != aucWepKeyLen2[ulLoop])
        {
            /* wifikey1 */
            PS_MEM_SET(aucWifiWepKey, 0, AT_NV_WLKEY_LEN + 1);

            PS_MEM_CPY(aucWifiWepKey,
                       pstWifiSecInfo->aucWifiWepKey2[ulLoop],
                       AT_NV_WLKEY_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            #if (FEATURE_ON == MBB_COMMON)
                                                "%s:%d,%s%s%s,%d%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                AT_WIFI_WEP_INDEX1,
                                                gaucAtQuotation,aucWifiWepKey,gaucAtQuotation,
                                            #else
                                                "%s:%d,%s,%d%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                1,
                                                aucWifiWepKey,
                                            #endif
                                                ulLoop,
                                                gaucAtCrLf);
        }

        if (0 != aucWepKeyLen3[ulLoop])
        {
            /* wifikey1 */
            PS_MEM_SET(aucWifiWepKey, 0, AT_NV_WLKEY_LEN + 1);

            PS_MEM_CPY(aucWifiWepKey,
                       pstWifiSecInfo->aucWifiWepKey3[ulLoop],
                       AT_NV_WLKEY_LEN);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        #if (FEATURE_ON == MBB_COMMON)
                                              "%s:%d,%s%s%s,%d%s",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              AT_WIFI_WEP_INDEX2,
                                              gaucAtQuotation,aucWifiWepKey,gaucAtQuotation,
                                        #else
                                              "%s:%d,%s,%d%s",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              2,
                                              aucWifiWepKey,
                                        #endif
                                              ulLoop,
                                              gaucAtCrLf);
        }

        if (0 != aucWepKeyLen4[ulLoop])
        {
            /* wifikey1 */
            PS_MEM_SET(aucWifiWepKey, 0, AT_NV_WLKEY_LEN + 1);

            PS_MEM_CPY(aucWifiWepKey,
                       pstWifiSecInfo->aucWifiWepKey4[ulLoop],
                       AT_NV_WLKEY_LEN);

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        #if (FEATURE_ON == MBB_COMMON)
                                              "%s:%d,%s%s%s,%d%s",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              AT_WIFI_WEP_INDEX3,
                                              gaucAtQuotation,aucWifiWepKey,gaucAtQuotation,
                                        #else
                                              "%s:%d,%s,%d%s",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              3,
                                              aucWifiWepKey,
                                        #endif
                                              ulLoop,
                                              gaucAtCrLf);
        }
    }

    gstAtSendData.usBufLen = usLength - (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);

    PS_MEM_FREE(WUEPS_PID_AT, pstWifiSecInfo);

    return AT_OK;
}
/* Modify by z60575 for multi_ssid, 2012-9-5 end */


VOS_UINT32 AT_TestWiwepPara(VOS_UINT8 ucIndex)
{
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   "%s:%d",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                   AT_WIWEP_TOOLS_WIFI_KEY_TOTAL);

    return AT_OK;
}


VOS_UINT32 AT_TestWifiPaRangePara (VOS_UINT8 ucIndex)
{
#if (FEATURE_ON == MBB_COMMON)
#if (FEATURE_ON == MBB_WIFI)

    WLAN_AT_BUFFER_STRU   ucWifiPaType = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }

    /* 获取支持的WIFI PA 模式 */
    if (AT_RETURN_SUCCESS != WlanATGetWifiParangeSupport(&ucWifiPaType))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (TAF_CHAR *)ucWifiPaType.buf);
#endif
    return AT_OK;
#else
    AT_WIFI_MODE_ENUM_UINT8             ucWifiMode;

    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    /* 初始化 */
    ucWifiMode                          = (VOS_UINT8)WIFI_GET_PA_MODE();

    /* 查询单板支持的模式：如果只支持PA模式，没有NO PA模式，则只返回h即可，如果两种模式都支持，则返回h,l。*/
    if (AT_WIFI_MODE_ONLY_PA == ucWifiMode)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                "%s:%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                "h");
    }
    else if (AT_WIFI_MODE_ONLY_NOPA == ucWifiMode)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                "%s:%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                "l");
    }
    else if (AT_WIFI_MODE_PA_NOPA == ucWifiMode)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                "%s:%s",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                "h,l");
    }
    else
    {
        return AT_ERROR;
    }

    return AT_OK;
#endif
}

VOS_VOID AT_GetTseLrfLoadDspInfo(
    AT_TSELRF_PATH_ENUM_UINT32          enPath,
    VOS_BOOL                           *pbLoadDsp,
    DRV_AGENT_TSELRF_SET_REQ_STRU      *pstTseLrf
)
{
    /* ^TSELRF命令设置的射频通路编号为GSM且当前已经LOAD了该通路，无需LOAD */
    if (AT_TSELRF_PATH_GSM == enPath)
    {
        if ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
         && (VOS_TRUE == g_stAtDevCmdCtrl.bDspLoadFlag))
        {
            *pbLoadDsp = VOS_FALSE;
        }
        else
        {
            pstTseLrf->ucLoadDspMode     = VOS_RATMODE_GSM;
            pstTseLrf->ucDeviceRatMode   = AT_RAT_MODE_GSM;
            *pbLoadDsp                   = VOS_TRUE;
        }
        return;
    }

    /* ^TSELRF命令设置的射频通路编号为WCDMA主集且当前已经LOAD了该通路，无需LOAD */
    if (AT_TSELRF_PATH_WCDMA_PRI == enPath)
    {
        if (((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
          || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
         && (VOS_TRUE == g_stAtDevCmdCtrl.bDspLoadFlag))
        {
            *pbLoadDsp = VOS_FALSE;
        }
        else
        {
            pstTseLrf->ucLoadDspMode     = VOS_RATMODE_WCDMA;
            pstTseLrf->ucDeviceRatMode   = AT_RAT_MODE_WCDMA;
            *pbLoadDsp                   = VOS_TRUE;
        }
        return;
    }

    *pbLoadDsp = VOS_FALSE;

    AT_WARN_LOG("AT_GetTseLrfLoadDspInfo: enPath only support GSM or WCDMA primary.");

    return;
}


VOS_UINT32 AT_SetTseLrfPara(VOS_UINT8 ucIndex)
{
    DRV_AGENT_TSELRF_SET_REQ_STRU       stTseLrf;
    VOS_BOOL                            bLoadDsp;

#if (FEATURE_ON == MBB_COMMON)
    g_SetTseLrfParValue = gastAtParaList[0].ulParaValue;
#endif

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

#if (FEATURE_ON == FEATURE_LTE)
    if ((AT_TSELRF_PATH_WCDMA_PRI!=gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_WCDMA_DIV!=gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_GSM !=gastAtParaList[0].ulParaValue))
    {
        return atSetTselrfPara(ucIndex);
    }
#endif

#if(FEATURE_UE_MODE_TDS == FEATURE_ON)
    if(AT_TSELRF_PATH_TD == gastAtParaList[0].ulParaValue)
    {
        return atSetTselrfPara(ucIndex);
    }

#endif
    /* 参数不符合要求 */
    if ((1 != gucAtParaIndex)
     || (0 == gastAtParaList[0].usParaLen))
    {
        return AT_ERROR;
    }

#if (FEATURE_ON == MBB_COMMON)
#if (FEATURE_ON == MBB_WIFI)

    if (AT_TSELRF_PATH_WIFI == gastAtParaList[0].ulParaValue)
    {
        if ( BSP_MODULE_SUPPORT == DRV_GET_WIFI_SUPPORT())
        {
            if(2 == gucAtParaIndex)
            {
                /* WIFI通路设置 */
                if(AT_RETURN_SUCCESS != WlanATSetTSELRF(gastAtParaList[1].ulParaValue))
                {
                    return AT_ERROR;
                }
            }
            else
            {
                /* WIFI通路设置 */
                if(AT_RETURN_SUCCESS != WlanATSetTSELRF(0))
                {
                    return AT_ERROR;
                }
            }
            
            g_stAtDevCmdCtrl.ucDeviceRatMode = AT_RAT_MODE_WIFI;

            return AT_OK;
        }
        else
        {
            return AT_ERROR;
        }
    }
#endif
#else
    if (AT_TSELRF_PATH_WIFI == gastAtParaList[0].ulParaValue)
    {
        /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
        if ( BSP_MODULE_SUPPORT == DRV_GET_WIFI_SUPPORT() )
        {
            /*WIFI未Enable直接返回失败*/
            if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
            {
                return AT_ERROR;
            }

            g_stAtDevCmdCtrl.ucDeviceRatMode = AT_RAT_MODE_WIFI;

            return AT_OK;
        }
        else
        {
            return AT_ERROR;
        }
        /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, end */
    }
#endif

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* 打开分集必须在FRXON之后，参考RXDIV实现 */
    if (AT_TSELRF_PATH_WCDMA_DIV == gastAtParaList[0].ulParaValue)
    {
        if (DRV_AGENT_DSP_RF_SWITCH_ON != g_stAtDevCmdCtrl.ucRxOnOff)
        {
            g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_DIV_ON;
            return AT_OK;
        }

        if (AT_FAILURE == At_SendRfCfgAntSelToHPA(AT_RX_DIV_ON))
        {
            return AT_ERROR;
        }

        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;
        g_stAtDevCmdCtrl.ucIndex               = ucIndex;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    /* Modify by f62575 for V7代码同步, 2012-04-07, Begin   */
    if ((AT_TSELRF_PATH_GSM != gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_WCDMA_PRI != gastAtParaList[0].ulParaValue))
    {
        return AT_ERROR;
    }
    /* Modify by f62575 for V7代码同步, 2012-04-07, End   */

    /* 此处判断是否需要重新加载DSP: 需要则发请求到C核加载DSP，否则，直接返回OK */
    AT_GetTseLrfLoadDspInfo(gastAtParaList[0].ulParaValue, &bLoadDsp, &stTseLrf);
    if (VOS_TRUE == bLoadDsp)
    {
        if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                                   gastAtClientTab[ucIndex].opId,
                                                   DRV_AGENT_TSELRF_SET_REQ,
                                                   &stTseLrf,
                                                   sizeof(stTseLrf),
                                                   I0_WUEPS_PID_DRV_AGENT))
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;             /*设置当前操作模式 */
            return AT_WAIT_ASYNC_RETURN;                                           /* 等待异步事件返回 */
        }
        else
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}


VOS_UINT32 AT_QryTseLrfPara(VOS_UINT8 ucIndex)
{
#if (FEATURE_ON == MBB_COMMON)
    VOS_UINT16                               usBandNum = 0;
    VOS_UINT16                               supportPathNum = 0;
    /* W分集 */
    VOS_UINT16                               aulSptWDIVBand;
    /* W和G主集 */
    AT_NV_WG_RF_MAIN_BAND_STRU               aulSptGUBand;
    /* L主集与分集 */
    LRRC_NV_UE_EUTRA_CAP_STRU                aulSptLTEBand;
    /* TDS主集 */
    NVIM_UE_TDS_SUPPORT_FREQ_BAND_LIST_STRU  aulSptTDSBand;
#if (FEATURE_ON == MBB_WIFI)
    /* WIFI */
    WLAN_AT_BUFFER_STRU                      wifiBuf;
#endif
    /* 各通路支持情况标志数组 */
    VOS_UINT8                                pathSupported[AT_TSELRF_PATH_TYPE_NUM];
    
    (VOS_VOID)PS_MEM_SET((VOS_VOID*)&aulSptGUBand, 0x00, sizeof(AT_NV_WG_RF_MAIN_BAND_STRU));/*lint !e506 !e160*/
    /* W和G主集能力NV读取 */
    if (NV_OK != NV_Read(en_NV_Item_WG_RF_MAIN_BAND, &aulSptGUBand, sizeof(AT_NV_WG_RF_MAIN_BAND_STRU)))
    {
        AT_WARN_LOG("AT_QryTseLrfPara():Read en_NV_Item_WG_RF_MAIN_BAND Failed!");
        return AT_ERROR;
    }

    (VOS_VOID)PS_MEM_SET((VOS_VOID*)&aulSptWDIVBand, 0x00, sizeof(aulSptWDIVBand));/*lint !e506 !e160*/
    /* W分集能力NV读取 */
    if (NV_OK != NV_Read(en_NV_Item_W_RF_DIV_BAND, &aulSptWDIVBand, sizeof(aulSptWDIVBand)))
    {
        AT_WARN_LOG("AT_QryTseLrfPara():Read en_NV_Item_W_RF_DIV_BAND Failed!");
        return AT_ERROR;
    }
    
    (VOS_VOID)PS_MEM_SET((VOS_VOID*)&aulSptLTEBand, 0x00, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));/*lint !e506 !e160*/
    /* LTE主集能力NV读取 */
    if( NV_OK != NVM_Read(EN_NV_ID_UE_CAPABILITY, &aulSptLTEBand, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU)))
    {
        AT_WARN_LOG("AT_QryTseLrfPara():Read EN_NV_ID_UE_CAPABILITY Failed!");
        return AT_ERROR;
    }

    (VOS_VOID)PS_MEM_SET((VOS_VOID*)&aulSptTDSBand, 0x00, sizeof(NVIM_UE_TDS_SUPPORT_FREQ_BAND_LIST_STRU));/*lint !e506 !e160*/
    /* TDS主集能力NV读取 */
    if( NV_OK != NV_Read(en_NV_Item_UTRAN_TDD_FREQ_BAND, &aulSptTDSBand, sizeof(NVIM_UE_TDS_SUPPORT_FREQ_BAND_LIST_STRU)))
    {
        AT_WARN_LOG("AT_QryTseLrfPara():Read en_NV_Item_UTRAN_TDD_FREQ_BAND Failed!");
        return AT_ERROR;
    }

    (VOS_VOID)PS_MEM_SET((VOS_VOID*)pathSupported, 0x00, sizeof(pathSupported));/*lint !e506 !e160*/
    /* W 主集支持情况统计 */
    if(0 != aulSptGUBand.unWcdmaBand.ulBand)
    {
        supportPathNum++;
        pathSupported[AT_TSELRF_WCDMA_MAIN] = AT_TSELRF_EXIST;
    }

    /* W 分集支持情况统计 */
    if(0 != aulSptWDIVBand)
    {
        supportPathNum++;
        pathSupported[AT_TSELRF_WCDMA_DIV] = AT_TSELRF_EXIST;
    }

    /* G 主集支持情况统计 */
    if(0 != aulSptGUBand.unGsmBand.ulBand)
    {
        supportPathNum++;
        pathSupported[AT_TSELRF_GSM] = AT_TSELRF_EXIST;
    }

    /* LTE FDD 主集和分集支持情况统计 */
    for(usBandNum = 0; usBandNum < aulSptLTEBand.stRfPara.usCnt; ++usBandNum)
    {
        if(AT_TSELRF_FDD_BAND_MIN <= aulSptLTEBand.stRfPara.astSuppEutraBandList[usBandNum].ucEutraBand 
           && AT_TSELRF_FDD_BAND_MAX >= aulSptLTEBand.stRfPara.astSuppEutraBandList[usBandNum].ucEutraBand)
        {
            supportPathNum++;
            pathSupported[AT_TSELRF_FDD_MAIN] = AT_TSELRF_EXIST;
            supportPathNum++;
            pathSupported[AT_TSELRF_FDD_DIV] = AT_TSELRF_EXIST;
            break;
        }
    }

    /* LTE TDD 主集和分集支持情况统计 */
    for(usBandNum = 0; usBandNum < aulSptLTEBand.stRfPara.usCnt; ++usBandNum)
    {
        if(AT_TSELRF_TDD_BAND_MIN <= aulSptLTEBand.stRfPara.astSuppEutraBandList[usBandNum].ucEutraBand 
           && AT_TSELRF_TDD_BAND_MAX >= aulSptLTEBand.stRfPara.astSuppEutraBandList[usBandNum].ucEutraBand)
        {
            supportPathNum++;
            pathSupported[AT_TSELRF_TDD_MAIN] = AT_TSELRF_EXIST;
            supportPathNum++;
            pathSupported[AT_TSELRF_TDD_DIV] = AT_TSELRF_EXIST;
            break;
        }
    }

    /* TDS 主集支持情况统计 */
    if(0 != aulSptTDSBand.ucBandCnt)
    {
        supportPathNum++;
        pathSupported[AT_TSELRF_TDS] = AT_TSELRF_EXIST;
    }

    /* WIFI 支持情况统计 */
    if (BSP_MODULE_SUPPORT == DRV_GET_WIFI_SUPPORT())
    {
        supportPathNum++;
        pathSupported[AT_TSELRF_WIFI] = AT_TSELRF_EXIST;
    }

    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      supportPathNum,
                                      gaucAtCrLf);

    if(0 != supportPathNum)
    {
        for(usBandNum = 0; usBandNum < AT_TSELRF_PATH_TYPE_NUM; ++usBandNum)
        {
            if(AT_TSELRF_EXIST == pathSupported[AT_TSELRF_ALL + usBandNum])
            {
                if(AT_TSELRF_WIFI == (AT_TSELRF_ALL + usBandNum))
                {
                #if (FEATURE_ON == MBB_WIFI)
                    (void)PS_MEM_SET((void *)&wifiBuf, 0, sizeof(wifiBuf));
                    if(AT_RETURN_SUCCESS == WlanATGetTSELRFSupport(&wifiBuf))
                    {
                        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                  "%s:%d,%s%s",
                                                  g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                  AT_TSELRF_WIFI,
                                                  wifiBuf.buf,
                                                  gaucAtCrLf);
                    }
                #endif
                }
                else
                {
                    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                          "%s:%d%s",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          (AT_TSELRF_ALL + usBandNum),
                                          gaucAtCrLf);
                }
            }
        }
    }
    
    if(0 != gstAtSendData.usBufLen)
    {
        gstAtSendData.usBufLen -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
    }
    
    return AT_OK;
#else
#if(FEATURE_LTE == FEATURE_ON)
    return atQryTselrfPara(ucIndex);
#else
    VOS_UINT16                          usLength;

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_TOTAL,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_GSM,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WCDMA_PRI,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d%s",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WCDMA_DIV,
                                      gaucAtCrLf);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "%s:%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      AT_TSELRF_PATH_WIFI);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
#endif
#endif
}


VOS_UINT32 AT_SetCmdlenPara(VOS_UINT8 ucIndex)
{
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen)
     || (2 != gucAtParaIndex))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gastAtParaList[0].ulParaValue > (AT_COM_BUFF_LEN - sizeof("AT")))
    {
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_QryCmdlenPara(VOS_UINT8 ucIndex)
{
    /*
      输出单板最大可以直接接收AT命令字符个数(AT_CMD_MAX_LEN - sizeof("AT"))，
      该字符个数均不包含AT这两个字符，以及单板通过AT一次最大可以响应的字符个数，
      BALONG产品响应字符串长度可以分
      多次上报，没有最大值概念，工位对该值无处理，此处输出一次上报的最大值AT_CMD_MAX_LEN。
    */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (AT_COM_BUFF_LEN - VOS_StrLen("AT")),
                                                    AT_CMD_MAX_LEN);

    return AT_OK;
}


VOS_UINT32 AT_UpdateMacPara(
    VOS_UINT8                           aucMac[],
    VOS_UINT16                          usMacLength
)
{
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulWifiGlobalMacOffset;
    VOS_UINT32                          ulPhyNumMacOffset;
    VOS_UINT8                           aucWifiGlobalMac[AT_MAC_ADDR_LEN];

    /* MAC地址长度检查: 必须12位 */
    if (AT_PHYNUM_MAC_LEN != usMacLength)
    {
        return AT_PHYNUM_LENGTH_ERR;
    }

    /* MAC地址格式匹配: 7AFEE22111E4=>7A:FE:E2:21:11:E4*/
    ulWifiGlobalMacOffset = 0;
    ulPhyNumMacOffset     = 0;
    for (ulLoop = 0; ulLoop < (AT_PHYNUM_MAC_COLON_NUM + 1); ulLoop++)
    {
        PS_MEM_CPY(&aucWifiGlobalMac[ulWifiGlobalMacOffset],
                   &aucMac[ulPhyNumMacOffset],
                   AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS);
        ulWifiGlobalMacOffset += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        ulPhyNumMacOffset     += AT_WIFIGLOBAL_MAC_LEN_BETWEEN_COLONS;
        aucWifiGlobalMac[ulWifiGlobalMacOffset] = ':';
        ulWifiGlobalMacOffset++;
    }

    aucWifiGlobalMac[AT_PHYNUM_MAC_LEN + AT_PHYNUM_MAC_COLON_NUM] = '\0';

    /* 更新MAC地址到NV */
    ulRet = NV_WriteEx(MODEM_ID_0, en_NV_Item_WIFI_MAC_ADDR, aucWifiGlobalMac, AT_MAC_ADDR_LEN);
    if (NV_OK != ulRet)
    {
         AT_WARN_LOG("AT_UpdateMacPara: Fail to write NV.");
         return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 AT_SetTmodeAutoPowerOff(VOS_UINT8 ucIndex)
{
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */

    TAF_MMA_PHONE_MODE_PARA_STRU        stPhModeSet;

    VOS_UINT8                             *pucSystemAppConfig;


    PS_MEM_SET(&stPhModeSet, 0x00, sizeof(TAF_PH_OP_MODE_STRU));

    pucSystemAppConfig                    = AT_GetSystemAppConfigAddr();

    if ( SYSTEM_APP_WEBUI == *pucSystemAppConfig)
    {
        /* 发消息给C核通知关机并下电 */
        /* 先通知用户AT命令已执行，用户通过检测端口是否消失来确认任务是否正确执行
           此命令执行完成后已下电，无需等待异步返回
        */

        stPhModeSet.PhMode  = TAF_PH_MODE_POWEROFF;

        if (VOS_TRUE == TAF_MMA_PhoneModeSetReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stPhModeSet))
        {
            return AT_OK;
        }
        else
        {
            return AT_ERROR;
        }

    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, end */

    return AT_ERROR;

}
/* Added by f62575 for SMALL IMAGE, 2012-1-3, end   */



VOS_UINT32 AT_SDParamErrCode(VOS_VOID)
{
    if(0 == g_stATParseCmd.ucParaCheckIndex)
    {
        return AT_SD_CARD_OPRT_NOT_ALLOWED;
    }
    else if(1 == g_stATParseCmd.ucParaCheckIndex)
    {
        return AT_SD_CARD_ADDR_ERR;
    }
    else
    {
        return AT_SD_CARD_OTHER_ERR;
    }
}


/* Added by f62575 for B050 Project, 2012-2-3, Begin   */

VOS_VOID AT_GetSpecificPort(
    VOS_UINT8                           ucPortType,
    VOS_UINT8                           aucRewindPortStyle[],
    VOS_UINT32                         *pulPortPos,
    VOS_UINT32                         *pulPortNum
)
{
    VOS_UINT32                          ulLoop;

    *pulPortPos = AT_DEV_NONE;
    for (ulLoop = 0; ulLoop < AT_SETPORT_PARA_MAX_LEN; ulLoop++)
    {
        if (0 == aucRewindPortStyle[ulLoop])
        {
            break;
        }

        /* 端口值为ucPortType，表示找到指定端口，记录ulLoop为端口在NV项中的偏移 */
        if (ucPortType == aucRewindPortStyle[ulLoop])
        {
            *pulPortPos = ulLoop;
        }
    }

    /* 端口值为0或到达循环上限，ulLoop即为单板的端口总数 */
    *pulPortNum = ulLoop;

    return;
}


VOS_UINT32 AT_ExistSpecificPort(VOS_UINT8 ucPortType)
{
    AT_DYNAMIC_PID_TYPE_STRU            stDynamicPidType;
    VOS_UINT32                          ulPortPos;
    VOS_UINT32                          ulPortNum;


    PS_MEM_SET(&stDynamicPidType, 0x00, sizeof(stDynamicPidType));

    ulPortPos = 0;
    ulPortNum = 0;


    /* 读NV项en_NV_Item_Huawei_Dynamic_PID_Type获取当前的端口状态 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Huawei_Dynamic_PID_Type,
                        &stDynamicPidType,
                        sizeof(AT_DYNAMIC_PID_TYPE_STRU)))
    {
        AT_WARN_LOG("AT_ExistSpecificPort: Read NV fail!");
        return VOS_FALSE;
    }

    /* 判断DIAG端口是否已经打开: 已经打开则直接返回AT_OK */
    if (VOS_TRUE == stDynamicPidType.ulNvStatus)
    {
        /* 查询NV项en_NV_Item_Huawei_Dynamic_PID_Type中是否已经存在DIAG口 */
        AT_GetSpecificPort(ucPortType,
                           stDynamicPidType.aucRewindPortStyle,
                           &ulPortPos,
                           &ulPortNum);

        if (AT_DEV_NONE != ulPortPos)
        {
            return VOS_TRUE;
        }

    }

    return VOS_FALSE;
}



VOS_UINT32 AT_OpenDiagPort(VOS_VOID)
{
    AT_DYNAMIC_PID_TYPE_STRU            stDynamicPidType;
    VOS_UINT32                          ulPortPos;
    VOS_UINT32                          ulPortNum;


    PS_MEM_SET(&stDynamicPidType, 0x00, sizeof(stDynamicPidType));

    ulPortPos = 0;
    ulPortNum = 0;


    /* 读NV项en_NV_Item_Huawei_Dynamic_PID_Type获取当前的端口状态 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Huawei_Dynamic_PID_Type,
                        &stDynamicPidType,
                        sizeof(AT_DYNAMIC_PID_TYPE_STRU)))
    {
        AT_WARN_LOG("AT_OpenDiagPort: Read NV fail!");
        return AT_ERROR;
    }

    /* 判断DIAG端口是否已经打开: 已经打开则直接返回AT_OK */
    if (VOS_TRUE == stDynamicPidType.ulNvStatus)
    {
        /* 查询NV项en_NV_Item_Huawei_Dynamic_PID_Type中是否已经存在DIAG口 */
        AT_GetSpecificPort(AT_DEV_DIAG,
                           stDynamicPidType.aucRewindPortStyle,
                           &ulPortPos,
                           &ulPortNum);

        if (AT_DEV_NONE != ulPortPos)
        {
            return AT_OK;
        }

    }
    else
    {
        AT_WARN_LOG("AT_OpenDiagPort: en_NV_Item_Huawei_Dynamic_PID_Type is inactive!");
        return AT_OK;
    }

    /* DIAG端口操作权限未获取: 直接返回AT_OK */
    if (VOS_FALSE == g_stSpWordCtx.ucShellPwdCheckFlag)
    {
        return AT_OK;
    }

    /* 追加DIAG端口到切换后端口集 */
    if (AT_SETPORT_PARA_MAX_LEN == ulPortNum)
    {
        return AT_OK;
    }

    stDynamicPidType.aucRewindPortStyle[ulPortNum] = AT_DEV_DIAG;

    /* 更新端口集合数据到NV项en_NV_Item_Huawei_Dynamic_PID_Type */
    if (NV_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_Huawei_Dynamic_PID_Type,
                        &stDynamicPidType,
                        sizeof(AT_DYNAMIC_PID_TYPE_STRU)))
    {
        AT_ERR_LOG("AT_OpenDiagPort: Write NV fail");
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


VOS_UINT32 AT_CloseDiagPort(VOS_VOID)
{
    AT_DYNAMIC_PID_TYPE_STRU            stDynamicPidType;
    VOS_UINT32                          ulPortPos;
    VOS_UINT32                          ulPortNum;
    VOS_UINT32                          ulLoop;


    PS_MEM_SET(&stDynamicPidType, 0x00, sizeof(stDynamicPidType));

    ulPortPos = 0;
    ulPortNum = 0;


    /* 读NV项en_NV_Item_Huawei_Dynamic_PID_Type获取当前的端口状态 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_Huawei_Dynamic_PID_Type,
                          &stDynamicPidType,
                          sizeof(AT_DYNAMIC_PID_TYPE_STRU)))
    {
        AT_ERR_LOG("AT_CloseDiagPort: Read NV fail!");
        return AT_ERROR;
    }

    /* 判断DIAG端口是否已经打开: 已经打开则直接返回AT_OK */
    if (VOS_TRUE == stDynamicPidType.ulNvStatus)
    {
        /* 查询NV项en_NV_Item_Huawei_Dynamic_PID_Type中是否已经存在DIAG口 */
        AT_GetSpecificPort(AT_DEV_DIAG,
                           stDynamicPidType.aucRewindPortStyle,
                           &ulPortPos,
                           &ulPortNum);

        if (AT_DEV_NONE == ulPortPos)
        {
            return AT_OK;
        }
    }
    else
    {
        return AT_OK;
    }
	
    
    /* 删除NV项中的DIAG端口 */
    stDynamicPidType.aucRewindPortStyle[ulPortPos] = 0;
    ulPortNum--;
    for (ulLoop = ulPortPos; ulLoop < ulPortNum; ulLoop++)
    {
        stDynamicPidType.aucRewindPortStyle[ulLoop] = stDynamicPidType.aucRewindPortStyle[ulLoop + 1];
    }
    stDynamicPidType.aucRewindPortStyle[ulPortNum] = 0;

    /* 端口异常数据保护: 切换后的设备形态中，第一个设备不能为MASS设备(0xa1,0xa2) */
    if (0 != ulPortNum)
    {
        if ((AT_DEV_CDROM == stDynamicPidType.aucRewindPortStyle[0])
         || (AT_DEV_SD == stDynamicPidType.aucRewindPortStyle[0]))
        {
            return AT_OK;
        }
    }

    /* 更新端口集合数据到NV项en_NV_Item_Huawei_Dynamic_PID_Type */
    if (NV_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_Huawei_Dynamic_PID_Type,
                          &stDynamicPidType,
                          sizeof(AT_DYNAMIC_PID_TYPE_STRU)))
    {
        AT_ERR_LOG("AT_CloseDiagPort: Write NV fail");
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


VOS_UINT32 AT_ExistSpecificPortChange(
    VOS_UINT8                           ucPortType,
    VOS_UINT8                           aucOldRewindPortStyle[],
    VOS_UINT8                           aucNewRewindPortStyle[]
)
{
    VOS_UINT32                          ulOldPortPos;
    VOS_UINT32                          ulNewPortPos;
    VOS_UINT32                          ulPortNum;


    ulOldPortPos = 0;
    ulNewPortPos = 0;
    ulPortNum    = 0;

    AT_GetSpecificPort(ucPortType, aucOldRewindPortStyle, &ulOldPortPos, &ulPortNum);
    AT_GetSpecificPort(ucPortType, aucNewRewindPortStyle, &ulNewPortPos, &ulPortNum);

    /* 新增一个指定端口 */
    if ((AT_DEV_NONE == ulOldPortPos)
     && (AT_DEV_NONE != ulNewPortPos))
    {
        return VOS_TRUE;
    }

    /* 删除一个指定端口 */
    if ((AT_DEV_NONE != ulOldPortPos)
     && (AT_DEV_NONE == ulNewPortPos))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;

}


VOS_UINT32 AT_CheckSetPortRight(
    VOS_UINT8                           aucOldRewindPortStyle[],
    VOS_UINT8                           aucNewRewindPortStyle[]
)
{
    VOS_UINT32                          diag3GPort          = VOS_FALSE;
    VOS_UINT32                          gps4GPort           = VOS_FALSE;
    VOS_UINT32                          diag4GPort          = VOS_FALSE;
    VOS_UINT32                          needPasswordPort    = VOS_FALSE;

    diag3GPort = AT_ExistSpecificPortChange(AT_DEV_DIAG,
                                       aucOldRewindPortStyle,
                                       aucNewRewindPortStyle);

    gps4GPort = AT_ExistSpecificPortChange(AT_DEV_4G_GPS,
                                       aucOldRewindPortStyle,
                                       aucNewRewindPortStyle);

    diag4GPort = AT_ExistSpecificPortChange(AT_DEV_4G_DIAG,
                                       aucOldRewindPortStyle,
                                       aucNewRewindPortStyle);
    /*如果需要解密的任意一个端口存在，并且没有被解密，端口变更无效*/
    needPasswordPort = diag3GPort || gps4GPort || diag4GPort;

    if ((VOS_TRUE == needPasswordPort)
        && (VOS_FALSE == g_stSpWordCtx.ucShellPwdCheckFlag))
    {
        return AT_ERROR;
    }

    return AT_OK;
}

/* Added by f62575 for B050 Project, 2012-2-3, end   */


VOS_UINT32 AT_SetHsspt(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucRRCVer;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DPAUPA_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_DPAUPA_ERROR;
    }

    /* 检查是否解除数据保护,未解除时返回出错信息:ErrCode:0 */
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return  AT_DATA_UNLOCK_ERROR;
    }

    ucRRCVer = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 调用写NV接口函数: AT_WriteRrcVerToNV,返回操作结果 */
    if (VOS_OK == AT_WriteRrcVerToNV(ucRRCVer))
    {
        return  AT_OK;
    }
    else
    {
        AT_WARN_LOG("At_SetHsspt:WARNING:Write NV failed!");
        return AT_DPAUPA_ERROR;
    }
}


VOS_UINT32 AT_QryHsspt(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucRRCVer;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usLength;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DPAUPA_ERROR;
    }

    ulResult = AT_ReadRrcVerFromNV(&ucRRCVer);

    if (VOS_OK == ulResult)
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr, "%s:",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength, "%d",
                                           (VOS_UINT32)ucRRCVer);
        gstAtSendData.usBufLen = usLength;
        return  AT_OK;
    }
    else
    {
        AT_WARN_LOG("AT_QryHspaSpt:WARNING:WAS_MNTN_QueryHspaSpt failed!");
        return AT_DPAUPA_ERROR;
    }
}


VOS_UINT32 AT_TestHsicCmdPara(VOS_UINT8 ucIndex)
{
    /* 通道检查 */
    /* Modified by L60609 for MUX，2012-08-13,  Begin */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    /* Modified by L60609 for MUX，2012-08-13,  End */
    {
        return AT_ERROR;
    }

    return AT_OK;
}

#if(FEATURE_ON == FEATURE_UE_MODE_TDS)

VOS_UINT32 At_TestTdsScalibPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                           usLength;

    usLength = 0;
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                        "%s:%s", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                        AT_TDS_SCALIB_TEST_STR);
    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}
#endif


VOS_UINT32 AT_TestSimlockUnlockPara( VOS_UINT8 ucIndex )
{
    /* 通道检查 */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: (\"NET\",\"NETSUB\",\"SP\")",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_String2Hex( VOS_UINT8 *nptr,VOS_UINT16 usLen, VOS_UINT32 *pRtn)
{
    VOS_UINT32                          c     = 0;         /* current Char */
    VOS_UINT32                          total = 0;         /* current total */
    VOS_UINT8                           Length = 0;        /* current Length */

    c = (VOS_UINT32)*nptr++;

    while(Length++ < usLen)
    {
        if( (c  >= '0') && (c  <= '9') )
        {
            c  = c  - '0';
        }
        else if( (c  >= 'a') && (c  <= 'f') )
        {
            c  = c  - 'a' + 10;
        }
        else if( (c  >= 'A') && (c  <= 'F') )
        {
            c  = c  - 'A' + 10;
        }
        else
        {
            return VOS_ERR;
        }

        if(total > 0x0FFFFFFF)              /* 发生反转 */
        {
            return VOS_ERR;
        }
        else
        {
            total = (total << 4) + c;       /* accumulate digit */
            c = (VOS_UINT32)(*(nptr++));    /* get next Char */
        }
    }
     /* return result, negated if necessary */
    *pRtn = total;
    return AT_SUCCESS;
}


VOS_UINT32 AT_NVWRGetItemValue( VOS_UINT8 *pucPara,  VOS_UINT8 *pucValue,  VOS_UINT8 **pucParaDst)
{
    VOS_UINT32                          ulTmp = 0;
    VOS_UINT8                          *pucStart = pucPara;
    VOS_UINT16                          usLen = 0;
    VOS_UINT8                          *pucEnd = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    /* 调用的地方保证pucPara,pucValue,pucParaDst不为NULL */

    /* 直到第一个不是' '的字符 */
    while(' ' == *pucStart)
    {
        pucStart++;
    }

    if(' ' == *(pucStart+1))
    {
        usLen = 1;
        pucEnd = pucStart+2;
    }
    else if(' ' == *(pucStart+2))
    {
        usLen = 2;
        pucEnd = pucStart+3;
    }
    else
    {
        return VOS_ERR;
    }

    ulRet = AT_String2Hex(pucStart, usLen, &ulTmp);
    if((VOS_OK != ulRet) || (ulTmp > 0xff))
    {
        return VOS_ERR;
    }

    *pucValue = (VOS_UINT8)ulTmp;
    *pucParaDst = pucEnd;

    return VOS_OK;
}


VOS_UINT32 AT_NVWRGetParaInfo( AT_PARSE_PARA_TYPE_STRU * pstPara, VOS_UINT8 * pu8Data, VOS_UINT32 * pulLen)
{
    VOS_UINT32                          ulNum = 0;
    VOS_UINT8                          *pu8Start   = VOS_NULL_PTR;
    VOS_UINT8                          *pu8ParaTmp = VOS_NULL_PTR;
    VOS_UINT16                          usLen = 0;
    VOS_UINT32                          ulRet;
    VOS_UINT16                          i = 0;

    /* 调用的地方保证pstPara,pu8Data不为NULL */

    pu8Start = pstPara->aucPara;
    usLen= pstPara->usParaLen;

    if(usLen < AT_PARA_MAX_LEN)
    {
        pstPara->aucPara[usLen] = ' ';
    }
    else
    {
        return VOS_ERR;
    }

    ulNum = 0;
    for(i = 0; i < usLen; )
    {
        ulRet = AT_NVWRGetItemValue(pu8Start, (pu8Data+ulNum), &pu8ParaTmp);
        if(ERR_MSP_SUCCESS != ulRet)
        {
            return ulRet;
        }

        ulNum++;

        /* 最多128个 */
        /*MAX_NV_NUM_PER_PARA */
        if(ulNum == 128)
        {
            break;
        }

        if(pu8ParaTmp >= (pu8Start+usLen))
        {
            break;
        }

        i += (VOS_UINT16)(pu8ParaTmp - pu8Start);
        pu8Start = pu8ParaTmp;
    }

    *pulLen = ulNum;
    return VOS_OK;
}


VOS_UINT32 AT_SetNVReadPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId  = 0;
    VOS_UINT32                          ulNvLen = 0;
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    VOS_UINT32                          i       = 0;
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          ulRet;

    /* 权限检查 */
#if (FEATURE_ON == MBB_COMMON)
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
#endif

    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVRD = 1;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if(gucAtParaIndex > 1)
    {
        g_ulNVRD = 2;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        g_ulNVRD = 3;
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        usNvId = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    }

    ulRet = NV_GetLength(usNvId, &ulNvLen);

    if(VOS_OK != ulRet)
    {
        g_ulNVRD = 4;
        return AT_ERROR;
    }

    /* 长度大于128，只取前128个字节*/
    pucData = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, ulNvLen);
    if(VOS_NULL_PTR == pucData)
    {
        g_ulNVRD = 5;
        return AT_ERROR;
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVReadPara:Get modem id fail");

        PS_MEM_FREE(WUEPS_PID_AT, pucData);

        g_ulNVWR =8;
        return AT_ERROR;
    }

    ulRet = NV_ReadEx(enModemId, usNvId, (VOS_VOID*)pucData, ulNvLen);

    if(VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);

        g_ulNVRD = 6;
        return AT_ERROR;
    }
    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
        "^NVRD: %d,", ulNvLen);

    for(i = 0; i < ulNvLen; i++)
    {
        if(0 == i)
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,"%02X", pucData[i]);
        }
        else
        {
            gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen," %02X", pucData[i]);
        }
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVRD = 7;
    return AT_OK;
}


VOS_UINT32 AT_SetNVWritePara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usNvId = 0;
    VOS_UINT16                          usNvTotleLen = 0;
    VOS_UINT32                          ulNvLen = 0;
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    VOS_UINT32                          ulNvNum = 0;
    VOS_UINT8                           au8Data[128] = {0};/* MAX_NV_NUM_PER_PARA */
    MODEM_ID_ENUM_UINT16                enModemId = MODEM_ID_0;
    VOS_UINT32                          i = 0;
    VOS_UINT32                          ulRet;

    gstAtSendData.usBufLen = 0;

    /* 权限检查 */
#if (FEATURE_ON == MBB_COMMON)
    if (VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }
#endif

    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        g_ulNVWR =1;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if((0 == gastAtParaList[0].usParaLen)
        || (0 == gastAtParaList[1].usParaLen)
        || (0 == gastAtParaList[2].usParaLen))
    {
        g_ulNVWR =2;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usNvId = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    vos_printf("\n atSetNVWRPara usNvId = %d\n",usNvId);

    usNvTotleLen = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    vos_printf("\n atSetNVWRPara usNvTotleLen = %d\n",usNvTotleLen);

    pucData = PS_MEM_ALLOC(WUEPS_PID_AT, usNvTotleLen);
    if(VOS_NULL_PTR == pucData)
    {
        g_ulNVWR =3;
        return AT_ERROR;
    }

    i = 0;
    while(0 != gastAtParaList[2 + i].usParaLen)
    {
        ulRet = AT_NVWRGetParaInfo((AT_PARSE_PARA_TYPE_STRU*)(&(gastAtParaList[2 + i])), au8Data, &ulNvNum);

        if(VOS_OK != ulRet)
        {
            PS_MEM_FREE(WUEPS_PID_AT, pucData);
            g_ulNVWR =4;
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 如果参数的长度大于128，则返回失败 */
        /*MAX_NV_NUM_PER_PARA */
        if(ulNvNum > 128)
        {
            PS_MEM_FREE(WUEPS_PID_AT, pucData);
            g_ulNVWR =5;
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 如果累加的参数个数大于总长度 */
        if((ulNvLen+ulNvNum) > usNvTotleLen)
        {
            PS_MEM_FREE(WUEPS_PID_AT, pucData);
            g_ulNVWR =6;
            return AT_CME_INCORRECT_PARAMETERS;
        }

        PS_MEM_CPY((pucData + ulNvLen), au8Data, ulNvNum);

        ulNvLen += ulNvNum;
        i++;

        if(i >= (AT_MAX_PARA_NUMBER-2))
        {
            break;
        }
    }

    /* 如果累加的参数个数与总长度不相等 */
    if(ulNvLen != usNvTotleLen)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =7;
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_SetNVWritePara:Get modem id fail");
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =8;
        return AT_ERROR;
    }

    ulRet = NV_WriteEx(enModemId, usNvId, (VOS_VOID*)pucData, usNvTotleLen);

    if(VOS_OK != ulRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucData);
        g_ulNVWR =9;

        return AT_ERROR;
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucData);
    g_ulNVWR =10;

    return AT_OK;
}


VOS_VOID AT_GetNvRdDebug(VOS_VOID)
{
    vos_printf("\n g_ulNVRD=0x%x \n",g_ulNVRD);
    vos_printf("\n g_ulNVWR=0x%x \n",g_ulNVWR);
}


VOS_UINT32 AT_QryFPllStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulReceiverPid;
    AT_PHY_RF_PLL_STATUS_REQ_STRU      *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMsgId;

    /*判断当前接入模式，只支持G/W*/
    if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        ulReceiverPid = DSP_PID_WPHY;
        usMsgId       = ID_AT_WPHY_RF_PLL_STATUS_REQ;
    }
    else if ( (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
            ||(AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode) )
    {
        ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
        usMsgId       = ID_AT_GPHY_RF_PLL_STATUS_REQ;
    }
    else if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
            ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFPllStatusPara(ucIndex);
    }
    else
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 申请AT_PHY_RF_PLL_STATUS_REQ_STRU消息 */
    ulLength = sizeof(AT_PHY_RF_PLL_STATUS_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_PHY_RF_PLL_STATUS_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_QryFPllStatusPara: Alloc msg fail!");
        return AT_ERROR;
    }

    /* 填充消息 */
    pstMsg->ulReceiverPid = ulReceiverPid;
    pstMsg->usMsgID       = usMsgId;
    pstMsg->usRsv1        = 0;
    pstMsg->usDspBand     = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->usRsv2        = 0;

    /* 向对应PHY发送消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_QryFPllStatusPara: Send msg fail!");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPLLSTATUS_QRY;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_ON == MBB_COMMON)

VOS_UINT32 At_TestFlnaPara(VOS_UINT8 ucIndex)
{
    //待实现
    return AT_ERROR;
}


VOS_UINT32 At_TestWiFiModePara(VOS_UINT8 ucIndex)
{
#if (FEATURE_ON == MBB_WIFI)

    WLAN_AT_BUFFER_STRU     strBuf = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));
    
    /* 读取WIFI模式 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiModeSupport(&strBuf))
    {
        return AT_ERROR;
    }
    
    /* WIFI模块模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
#endif
    return AT_OK;
}


VOS_UINT32 At_TestWiFiBandPara(VOS_UINT8 ucIndex)
{
#if (FEATURE_ON == MBB_WIFI)

    WLAN_AT_BUFFER_STRU     strBuf = {0,{0}};

    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* WIFI是否支持 */
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }

    /* 初始化 */
    (void)VOS_MemSet((void *)&strBuf, 0, sizeof(strBuf));

    /* 读取WIFI支持带宽 */
    if(AT_RETURN_SUCCESS != WlanATGetWifiBandSupport(&strBuf))
    {
        return AT_ERROR;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    strBuf.buf);
#endif
    return AT_OK;
}


VOS_UINT32 At_TestTmmiPara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_DEVICE_OTHER_ERROR;
    }

    /* TMMI支持的测试模式 */
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s=%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    AT_MMI_TEST_SUPPORT_MANUAL);
    return AT_OK;
}

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)

VOS_UINT32 At_TestUsbSel(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                             (VOS_CHAR*)pgucAtSndCodeAddr,
                             (VOS_CHAR*)pgucAtSndCodeAddr,
                             "%s: %s",
                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                             "(0,1)");
    return AT_OK;
}
#endif



VOS_UINT32 At_TestSfm(VOS_UINT8 ucIndex)
{
    TAF_CHAR  *supportedMode = "0,1,2";
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s", 
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 
                                                 supportedMode);

    return AT_OK;
}


VOS_UINT32 At_TestAntenna(VOS_UINT8 ucIndex)
{
#if (FEATURE_ON == MBB_ANTENNA)
    VOS_UINT8 SupportSoftSwich = 0;
    VOS_UINT8 SupportSwich = 0;
    VOS_UINT16    usLength = 0;
    int i = 0;
    
    for(i = 0; i < SUPPORT_RF_BAND_FOR_ANTE; i++)
    {
        if(g_SetTseLrfParValue == ante_switch_array[i].rf_band_for_ante)
        {      
            SupportSoftSwich = ante_switch_array[i].support_soft_switch;
            SupportSwich = ante_switch_array[i].support_switch_type;
                
            usLength = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
            (TAF_CHAR *)pgucAtSndCodeAddr,
            (TAF_CHAR *)pgucAtSndCodeAddr,
            "%s:%d,%d\n",
            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
            SupportSoftSwich,
            SupportSwich);
            gstAtSendData.usBufLen = usLength;
            
            return AT_OK;
        }
    }

     return AT_ERROR;
#else
    TAF_CHAR  *supportedMode = "0,0";
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 "%s:%s", 
                                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName, 
                                                 supportedMode);

    return AT_OK;
#endif

}
#endif


VOS_VOID At_RfPllStatusCnfProc(PHY_AT_RF_PLL_STATUS_CNF_STRU *pstMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* 获取本地保存的用户索引 */
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (AT_CMD_FPLLSTATUS_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("At_RfPllStatusCnfProc: CmdCurrentOpt is not AT_CMD_FPLLSTATUS_QRY!");
        return;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: %d,%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       pstMsg->usTxStatus,
                                       pstMsg->usRxStatus);

    gstAtSendData.usBufLen = usLength;

    At_FormatResultData(ucIndex, AT_OK);

    return;
}


VOS_UINT32 AT_QryFpowdetTPara(VOS_UINT8 ucIndex)
{
    AT_PHY_POWER_DET_REQ_STRU          *pstMsg;
    VOS_UINT32                          ulLength;

    /*判断当前接入模式，只支持W*/
    if (AT_RAT_MODE_WCDMA != g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 申请AT_PHY_POWER_DET_REQ_STRU消息 */
    ulLength = sizeof(AT_PHY_POWER_DET_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_PHY_POWER_DET_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_QryFpowdetTPara: Alloc msg fail!");
        return AT_ERROR;
    }

    /* 填充消息 */
    pstMsg->ulReceiverPid = DSP_PID_WPHY;
    pstMsg->usMsgID       = ID_AT_WPHY_POWER_DET_REQ;
    pstMsg->usRsv         = 0;

    /* 向对应PHY发送消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_QryFpowdetTPara: Send msg fail!");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPOWDET_QRY;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_VOID At_RfFpowdetTCnfProc(PHY_AT_POWER_DET_CNF_STRU *pstMsg)
{
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;

    /* 获取本地保存的用户索引 */
    ucIndex = g_stAtDevCmdCtrl.ucIndex;

    if (AT_CMD_FPOWDET_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("At_RfFPOWDETCnfProc: CmdCurrentOpt is not AT_CMD_FPOWDET_QRY!");
        return;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 应物理层要求，如果返回值为0x7FFF则为无效值，项查询者返回ERROR */
    if(0x7FFF == pstMsg->sPowerDet)
    {
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           "%s: %d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstMsg->sPowerDet);

        gstAtSendData.usBufLen = usLength;

        At_FormatResultData(ucIndex, AT_OK);
    }

    return;
}
#if (FEATURE_ON == MBB_COMMON)

VOS_UINT32 AT_TestWifiCalPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ucWifiCalSupport = 0;
    if (BSP_MODULE_UNSUPPORT == DRV_GET_WIFI_SUPPORT() )
    {
        return AT_ERROR;
    }

    /*获取wical支持参数*/
    ucWifiCalSupport = WlanATGetWifiCalSupport();

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    ucWifiCalSupport);
    return AT_OK;
}


VOS_UINT32 At_TestNavTypePara(VOS_UINT8 ucIndex)
{
    return AT_ERROR;
}
#if(FEATURE_ON == MBB_FEATURE_ESIM_SWITCH)

VOS_UINT32 At_TestTSimSwitchPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s:2\r\n",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s:2,\"3\"\r\n",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s:3,\"2\"",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}
#endif
#endif/*END FOR MBB_COMMON*/
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

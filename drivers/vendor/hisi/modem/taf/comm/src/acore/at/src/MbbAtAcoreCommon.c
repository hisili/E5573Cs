

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtParse.h"
#include "ATCmdProc.h"
#include "AtDataProc.h"
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "MbbPsCsCommon.h"
#include "MbbAtGuComm.h"
#include "linux/mlog_lib.h"
#include "AtMntn.h"
#include "MbbMpdpComm.h"
#include "AtCheckFunc.h"
#include "LPsNvInterface.h"
#include "UsimmApi.h"

#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
#include "MbbTafApsApi.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_WPG_COMMON)

#if (FEATURE_ON == MBB_FEATURE_BIP)
#include "drv_bip.h"
#endif

/*****************************************************************************
  1 宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_EXTENDPRIVATECMD_C

#define MBB_MEM_CPY(pDst, pSrc, len)       (VOS_VOID)PS_MEM_CPY(((VOS_VOID*)(pDst)), ((VOS_VOID*)(pSrc)),((unsigned int)(len)))
#define MBB_MEM_FREE(ulPid, pAddr)         (VOS_VOID)PS_MEM_FREE(ulPid,pAddr)
#define MBB_MEM_SET(pAddr, ucData, len)  (VOS_VOID)PS_MEM_SET(((VOS_VOID*)(pAddr)), ucData, ((unsigned int)(len)))
#define MBB_MEM_ALLOC(ulPid, ulSize)        PS_MEM_ALLOC(ulPid, ulSize)

#define MMA_PLMN_ID_LEN_5   (5)
#define MMA_PLMN_ID_LEN_6   (6)
#define AT_RSRP_MAX          (-44)
#define AT_RSRQ_MAX          (-3)
#define AT_RSRP_MIN          (-140)
#define AT_RSRQ_MIN          (-20)
#define DEST_STRING_MAX_LEN      (150)
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
#define AT_CMER_PARA_COUNT_MAX              (5)       /*AT+CMER设置函数参数个数*/
#define AT_CMER_RPT_MODE_SWITCH_OFF         (0)       /* +CMER命令标示，mode开关为关*/
#define AT_CMER_RPT_MODE_SWITCH_ON          (3)       /* +CMER命令标示，mode开关为开*/
#define AT_CMER_RPT_SWITCH_OFF              (0)       /* +CMER命令标示开关为关*/
#define AT_CMER_RPT_SWITCH_ON               (1)       /* +CMER命令标示开关为开*/
#define AT_CIND_RPT_SWITCH_OFF              (0)       /* +CIND命令标示开关为关*/
#define AT_CIND_RPT_SWITCH_ON               (1)       /* +CIND命令标示开关为开*/
#define TAF_CIND_SRVSTA_NORMAL_SERVICE      (1)       /* +CIND命令返回值表示正常服务 */
#define TAF_CIND_SRVSTA_NO_SERVICE          (0)       /* +CIND命令返回值表示无服务*/
#define AT_CIND_PARA_COUNT_MAX              (8)       /*AT+CIND设置函数参数最大个数*/
#define AT_CIND_PARA_COUNT_MIN              (1)       /*AT+CIND设置函数参数最小个数*/
#define CIEV_SIGNAL_FLAG                    (2)       /*+CIEV命令天线水平上报标示*/
/* 定义高温无服务状态AT*ANT的返回结果 */
#define    AT_ANT_TEMPPRT_LEVEL       (99)
#endif
#define SYSCFGEX_MODE_AUTO           (0x00)
#define SYSCFGEX_MODE_RAT_MAX        (0x06)
#define SYSCFGEX_MODE_NO_CHANGE      (0x99)
#define SYSCFGEX_MODE_INVALID        (0xFF)

#define AT_SIMPOWER_ON          (1)
#define AT_SIMPOWER_OFF         (0)

VOS_UINT32 g_ulMbbAtAcoreDebug = VOS_FALSE;
#define MBB_AT_ACORE_COMMON_DEBUG(str, var) \
    do\
    {\
        if (VOS_TRUE == g_ulMbbAtAcoreDebug)\
        {\
            (VOS_VOID)vos_printf("%s, %d %s=%d\n", __func__, __LINE__, str, var);\
        }\
    }while(0)

#define MBB_AT_ACORE_COMMON_DEBUG_STR(str) \
    do\
    {\
        if (VOS_TRUE == g_ulMbbAtAcoreDebug)\
        {\
            (VOS_VOID)vos_printf("%s, %d %s\n", __func__, __LINE__, str);\
        }\
    }while(0)

#if (FEATURE_ON == MBB_OPERATOR_INDUSTRYCARD)
#define UPDATE_MODE_DATA_LEN                    (2)
#define UPDATE_VERSION_STAT_MAX                 (8)
#define APP_INIT_UPDATE_MODE_OK                 (1)
#endif

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
NAS_NV_SYSCFGEX_MODE_LIST            g_stSyscfgexModeList = {0};

/*德国 Vodafone CPBS定制*/
VOS_UINT16                              gucVodafoneCpbs;

/*AT+CSIM定制*/
VOS_UINT8                               gucCsimActiveFlag;

#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
extern  VOS_UINT8                       g_TempprtEvent_flag;
#endif
#ifdef BSP_CONFIG_BOARD_K5160
extern AT_USER_TYPE                      g_stAtNdisUserType;
extern VOS_UINT8                         g_stAtNdisCid;
#endif/*BSP_CONFIG_BOARD_K5160*/

VOS_UINT32                             g_ulApConnDebugFlag = VOS_FALSE;

#if(FEATURE_ON == MBB_FEATURE_MPDP)
/* 多PDP的句柄，g_ulAtUdiNdisMpdpHdl[0] 等于 g_ulAtUdiNdisHdl，是主PDP */
UDI_HANDLE       g_ulAtUdiNdisMpdpHdl[MAX_NDIS_NET] = {UDI_INVALID_HANDLE};

/*NV50242中设置的MPDP个数*/
VOS_UINT8          g_MpdpNum = 0;
                            
int                        gMbimFeatureFlag = FALSE;

static VOS_UINT8          g_ActUsbNetNum = 0;

#define IS_MBIM_OS()   (gMbimFeatureFlag)

AT_CHDATA_NDIS_RMNET_ID_STRU            g_astAtChdataNdisRmNetIdTab[] =
{
    {AT_CH_DATA_CHANNEL_ID_1, NDIS_RM_NET_ID_0},
    {AT_CH_DATA_CHANNEL_ID_2, NDIS_RM_NET_ID_1},
    {AT_CH_DATA_CHANNEL_ID_3, NDIS_RM_NET_ID_2},
    {AT_CH_DATA_CHANNEL_ID_4, NDIS_RM_NET_ID_3},
    {AT_CH_DATA_CHANNEL_ID_5, NDIS_RM_NET_ID_4},
    {AT_CH_DATA_CHANNEL_ID_6, NDIS_RM_NET_ID_5},
    {AT_CH_DATA_CHANNEL_ID_7, NDIS_RM_NET_ID_6},
    {AT_CH_DATA_CHANNEL_ID_8, NDIS_RM_NET_ID_7}
};
#endif/*FEATURE_ON == MBB_FEATURE_MPDP*/
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    AT_CMD_CMER_PARA      g_ucCmerpara = {0};
    AT_CMD_CIND_PARA      g_ucCindpara = {0};
    static AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     g_oldGsmAntLevel = AT_CMD_ANTENNA_LEVEL_BUTT;
    static AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     g_oldWcdmaAntLevel = AT_CMD_ANTENNA_LEVEL_BUTT;
    static AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     g_oldTDscdmaAntLevel = AT_CMD_ANTENNA_LEVEL_BUTT;
#endif

VOS_UINT32 g_TimeInfoDebug = VOS_FALSE;
MCC_ZONE_INFO_STRU g_mcc_zone_infoTbl[] =
{
    {0x202, 30  * 4 / 10, 0},  /* Greece */
    {0x204, 10  * 4 / 10, 0},  /* Greece */
    {0x206, 10  * 4 / 10, 0},  /* Holland */
    {0x208, 10  * 4 / 10, 0},  /* France */
    {0x212, 10  * 4 / 10, 0},  /* Monaco */
    {0x213, 10  * 4 / 10, 0},  /* Andorra */
    {0x214, 10  * 4 / 10, 0},  /* Spain */
    {0x216, 10  * 4 / 10, 0},  /* Hungary */
    {0x218, 10  * 4 / 10, 0},  /* Bosnia and Herzegovina, the Republic of */
    {0x219, 10  * 4 / 10, 0},  /* Croatia */
    {0x220, 10  * 4 / 10, 0},  /* Montenegro / Monte Nige Lu / Serbia*/
    {0x222, 10  * 4 / 10, 0},  /* Italy */
    {0x226, 20  * 4 / 10, 0},  /* Romania */
    {0x228, 10  * 4 / 10, 0},  /* Switzerland */
    {0x230, 10  * 4 / 10, 0},  /* Czech */
    {0x231, 10  * 4 / 10, 0},  /* Slovakia */
    {0x232, 10  * 4 / 10, 0},  /* Austria */
    {0x234, 10  * 4 / 10, 0},  /* Guernsey */
    {0x238, 10  * 4 / 10, 0},  /* Denmark */
    {0x240, 10  * 4 / 10, 0},  /* Sweden */
    {0x242, 10  * 4 / 10, 0},  /* Norway */
    {0x244, 20  * 4 / 10, 0},  /* Finland */
    {0x246, 20  * 4 / 10, 0},  /* Lithuania */
    {0x247, 20  * 4 / 10, 0},  /* Latvia */
    {0x248, 20  * 4 / 10, 0},  /* Estonia */
    {0x250, 80  * 4 / 10, 0},  /* Russia GMT+3 ~ GMT+12 */
    {0x255, 20  * 4 / 10, 0},  /* Ukraine */
    {0x257, 20  * 4 / 10, 0},  /* Belarus */
    {0x259, 20  * 4 / 10, 0},  /* Moldova */
    {0x260, 10  * 4 / 10, 0},  /* Poland */
    {0x262, 10  * 4 / 10, 0},  /* Germany */
    {0x266, 10  * 4 / 10, 0},  /* Gibraltar */
    {0x268, 0  * 4 / 10, 0},  /* Portugal */
    {0x270, 10  * 4 / 10, 0},  /* Luxembourg */
    {0x272, 0  * 4 / 10, 0},  /* Ireland  GMT+0 ~ GMT+1*/
    {0x274, 0  * 4 / 10, 0},  /*Iceland  */
    {0x276, 10  * 4 / 10, 0},  /* Albania */
    {0x278, 10  * 4 / 10, 0},  /* Malta */
    {0x280, 20  * 4 / 10, 0},  /* Cyprus */
    {0x282, 40  * 4 / 10, 0},  /* Georgia */
    {0x283, 40  * 4 / 10, 0},  /* Armenia */
    {0x284, 20  * 4 / 10, 0},  /* Bulgaria */
    {0x286, 20  * 4 / 10, 0},  /* Turkey */
    {0x288, 0  * 4 / 10, 0},  /* Faroe Islands */
    {0x290, - 20  * 4 / 10, 0},  /* Greenland GMT+0 ~ GMT-4 */
    {0x292, 10  * 4 / 10, 0},  /* San Marino */
    {0x293, 10  * 4 / 10, 0},  /* Slovenia */
    {0x294, 10  * 4 / 10, 0},  /* Macedonia */
    {0x295, 10  * 4 / 10, 0},  /* Liechtenstein */
    {0x302, - 40  * 4 / 10, 0},  /* Canada */
    {0x308, - 30  * 4 / 10, 0},  /* Saint-Pierre and Miquelon */
    {0x310, - 80 * 4 / 10, 0},  /* America */
    {0x311, - 80  * 4 / 10, 0},  /* America */
    {0x330, - 40 * 4 / 10, 0},  /* Puerto Rico */
    {0x334, - 70 * 4 / 10, 0},  /* Mexico */
    {0x338, - 50 * 4 / 10, 0},  /* Jamaica */
    {0x340, - 40 * 4 / 10, 0},  /* Martinique */
    {0x342, - 40 * 4 / 10, 0},  /* Barbados */
    {0x344, - 40 * 4 / 10, 0},  /* Antigua and Barbuda */
    {0x346, - 50 * 4 / 10, 0},  /* Cayman Islands */
    {0x348, - 40 * 4 / 10, 0},  /* The British Virgin Islands */
    {0x350, - 30 * 4 / 10, 0},  /* Bermuda */
    {0x352, - 40 * 4 / 10, 0},  /* Grenada */
    {0x354, - 40 * 4 / 10, 0},  /* Montserrat */
    {0x356, - 40 * 4 / 10, 0},  /* Saint Kitts and Nevis */
    {0x358, - 40 * 4 / 10, 0},  /* St. Lucia */
    {0x360, - 40 * 4 / 10, 0},  /* Saint Vincent and the Grenadines */
    {0x362, - 40 * 4 / 10, 0},  /* Netherlands Antilles */
    {0x363, - 40 * 4 / 10, 0},  /* Aruba */
    {0x364, - 50 * 4 / 10, 0},  /* Bahamas */
    {0x365, - 40 * 4 / 10, 0},  /* Anguilla */
    {0x366, - 50 * 4 / 10, 0},  /* Dominican */
    {0x368, - 50 * 4 / 10, 0},  /* Cuba */
    {0x370, - 50 * 4 / 10, 0},  /* Dominican Republic */
    {0x372, - 50 * 4 / 10, 0},  /* Haiti */
    {0x374, - 40 * 4 / 10, 0},  /* The Republic of Trinidad and Tobago */
    {0x376, - 50 * 4 / 10, 0},  /* Turks and Caicos Islands */
    {0x400, 40  * 4 / 10, 0},  /* Republic of Azerbaijan */
    {0x401, 40  * 4 / 10, 0},  /* Kazakhstan */
    {0x402, 60  * 4 / 10, 0},  /* Bhutan */
    {0x404, 55  * 4 / 10, 0},  /* India */
    {0x405, 55  * 4 / 10, 0},    /*India*/
    {0x410, + 50 * 4 / 10, 0},   /*Pakistan*/
    {0x412, + 45 * 4 / 10, 0},  /*Afghanistan*/
    {0x413, + 55 * 4 / 10, 0},  /*Sri Lanka*/
    {0x414, + 65 * 4 / 10, 0},  /* Myanmar */
    {0x415, + 30 * 4 / 10, 0},  /* Lebanon */
    {0x416, + 20 * 4 / 10, 0},  /* Jordan */
    {0x417, + 20 * 4 / 10, 0},  /* Syria */
    {0x418, + 30 * 4 / 10, 0},  /* Iraq */
    {0x419, + 30 * 4 / 10, 0},  /* Kuwait */
    {0x420, + 30 * 4 / 10, 0},  /* Saudi Arabia */
    {0x421, + 30 * 4 / 10, 0},  /* Yemen */
    {0x422, + 40 * 4 / 10, 0},  /* Oman */
    {0x424, + 40 * 4 / 10, 0},  /* United Arab Emirates */
    {0x425, + 20 * 4 / 10, 0},  /* Israel */
    {0x426, + 30 * 4 / 10, 0},  /* Bahrain  */
    {0x427, + 30 * 4 / 10, 0},  /* Qatar */
    {0x428, + 80 * 4 / 10, 0},  /* Mongolia */
    {0x429, + 58 * 4 / 10, 0},  /* Nepal */
    {0x432, + 35 * 4 / 10, 0},  /* Iran */
    {0x434, + 50 * 4 / 10, 0},  /* Uzbekistan */
    {0x436, + 50 * 4 / 10, 0},  /* Tajikistan */
    {0x437, + 60 * 4 / 10, 0},  /* Kyrgyzstan */
    {0x438, + 50 * 4 / 10, 0},  /* Turkmenistan */
    {0x440, + 90 * 4 / 10, 0},  /* Japan */
    {0x450, + 90 * 4 / 10, 0},  /* South Korea */
    {0x452, + 70 * 4 / 10, 0},  /* Vietnam */
    {0x454, + 80 * 4 / 10, 0},  /* Hong Kong */
    {0x455, + 80 * 4 / 10, 0},  /* Macau */
    {0x456, + 70 * 4 / 10, 0},  /* Cambodia */
    {0x457, + 70 * 4 / 10, 0},  /* Laos */
    {0x460, + 80 * 4 / 10, 0},  /*China*/
    {0x466, + 80 * 4 / 10, 0},  /* Taiwan */
    {0x467, + 90 * 4 / 10, 0},  /* North Korea */
    {0x470, + 60 * 4 / 10, 0},  /* Bangladesh */
    {0x472, + 50 * 4 / 10, 0},  /* Maldives */
    {0x502, + 80 * 4 / 10, 0},  /*  */
    {0x505, + 90 * 4 / 10, 0},  /*  */
    {0x510, + 80 * 4 / 10, 0},  /*  */
    {0x514, + 90 * 4 / 10, 0},  /*  */
    {0x515, + 80 * 4 / 10, 0},  /*  */
    {0x520, + 70 * 4 / 10, 0},  /*  */
    {0x525, + 80 * 4 / 10, 0},  /*  */
    {0x528, + 80 * 4 / 10, 0},  /*  */
    {0x530, + 120 * 4 / 10, 0},  /*  */
    {0x537, + 100 * 4 / 10, 0},  /*  */
    {0x539, + 130 * 4 / 10, 0},  /*  */
    {0x540, 110 * 4 / 10, 0},  /*  */
    {0x541, 110 * 4 / 10, 0},  /*  */
    {0x542, 120 * 4 / 10, 0},  /*  */
    {0x544, - 110 * 4 / 10, 0},  /* American Samoa*/
    {0x545, 130 * 4 / 10, 0},  /*GMT +13 - +15*/
    {0x546, 110 * 4 / 10, 0},  /*  */
    {0x547, - 100 * 4 / 10, 0},  /*  */
    {0x548, - 100 * 4 / 10, 0},  /*  */
    {0x549, - 110 * 4 / 10, 0},  /*  */
    {0x550, + 100 * 4 / 10, 0},  /*  */
    {0x552, + 90 * 4 / 10, 0},  /*  */
    {0x602, + 20 * 4 / 10, 0},  /*  */
    {0x603, + 10 * 4 / 10, 0},  /*  */
    {0x604, 0 * 4 / 10, 0},  /*  */
    {0x605, + 10 * 4 / 10, 0},  /*  */
    {0x606, + 20 * 4 / 10, 0},  /*  */
    {0x607, 0 * 4 / 10, 0},  /*  */
    {0x608, 0 * 4 / 10, 0},  /*  */
    {0x609, 0 * 4 / 10, 0},  /*  */
    {0x610, 0 * 4 / 10, 0},  /*  */
    {0x611, 0 * 4 / 10, 0},  /*  */
    {0x612, 0 * 4 / 10, 0},  /*  */
    {0x613, 0 * 4 / 10, 0},  /*  */
    {0x614, + 10 * 4 / 10, 0},  /*  */
    {0x615, 0 * 4 / 10, 0},  /*  */
    {0x616, + 10 * 4 / 10, 0},  /*  */
    {0x617, + 40 * 4 / 10, 0},  /*  */
    {0x618,   0 * 4 / 10, 0},  /*  */
    {0x619,   0 * 4 / 10, 0},  /*  */
    {0x620,   0 * 4 / 10, 0},  /*  */
    {0x621,  + 10 * 4 / 10, 0},  /*  */
    {0x622,  + 10 * 4 / 10, 0},  /*  */
    {0x623,  + 10 * 4 / 10, 0},  /*  */
    {0x624,  + 10 * 4 / 10, 0},  /*  */
    {0x625,  + 10 * 4 / 10, 0},  /*  */
    {0x626,   0 * 4 / 10, 0},  /*  */
    {0x627,   + 10 * 4 / 10, 0},  /*  */
    {0x628,   + 10 * 4 / 10, 0},  /*  */
    {0x629,   + 10 * 4 / 10, 0},  /*  */
    {0x630,   + 20 * 4 / 10, 0},  /*  */
    {0x631,   + 10 * 4 / 10, 0},  /*  */
    {0x632,   0 * 4 / 10, 0},  /*  */
    {0x633,   + 40 * 4 / 10, 0},  /*  */
    {0x634,   + 30 * 4 / 10, 0},  /*  */
    {0x635,   + 20 * 4 / 10, 0},  /*  */
    {0x636,   + 30 * 4 / 10, 0},    /* Ethiopia */
    {0x637,   + 30 * 4 / 10, 0},    /* Somalia */
    {0x638,   + 30 * 4 / 10, 0},    /* Djibouti */
    {0x639,   + 30 * 4 / 10, 0},    /* Kenya */
    {0x640,   + 30 * 4 / 10, 0},    /* Tanzania */
    {0x641,   + 30 * 4 / 10, 0},    /* Uganda */
    {0x642,   + 20 * 4 / 10, 0},    /* Burundi */
    {0x643,   + 20 * 4 / 10, 0},    /* Mozambique */
    {0x645,   + 20 * 4 / 10, 0},    /* Zambia */
    {0x646,   + 30 * 4 / 10, 0},    /* Madagascar */
    {0x647,   0 * 4 / 10, 0},    /* not sure */
    {0x648,   + 20 * 4 / 10, 0},    /* Zimbabwe */
    {0x649,   + 10 * 4 / 10, 0},    /* Namibia */
    {0x650,   + 20 * 4 / 10, 0},    /* Malawi */
    {0x651,   + 20 * 4 / 10, 0},    /* Lesotho */
    {0x652,   + 20 * 4 / 10, 0},    /* Botswana */
    {0x653,   + 20 * 4 / 10, 0},    /* Swaziland */
    {0x654,   + 20 * 4 / 10, 0},    /* Comoros */
    {0x655,   + 20 * 4 / 10, 0},    /* South Africa */
    {0x659,   + 30 * 4 / 10, 0},    /* sudan */
    {0x702,   - 60 * 4 / 10, 0},    /* Belize */
    {0x704,   - 60 * 4 / 10, 0},    /* Guatemala */
    {0x706,   - 60 * 4 / 10, 0},    /* Salvador */
    {0x708,   - 60 * 4 / 10, 0},    /* Honduras */
    {0x710,   - 60 * 4 / 10, 0},    /* Nicaragua */
    {0x712,   - 60 * 4 / 10, 0},    /* Costa Rica */
    {0x714,   - 50 * 4 / 10, 0},    /* Panama */
    {0x716,   - 50 * 4 / 10, 0},    /* Peru */
    {0x722,   - 30 * 4 / 10, 0},    /* Argentina */
    {0x724,   - 30 * 4 / 10, 0},    /* Brazil */
    {0x730,   - 50 * 4 / 10, 0},    /* Chile */
    {0x732,   - 50 * 4 / 10, 0},    /* Colombia */
    {0x734,   - 45 * 4 / 10, 0},    /* Venezuela */
    {0x736,   - 40 * 4 / 10, 0},    /* Bolivia */
    {0x738,   - 30 * 4 / 10, 0},    /* Guyana */
    {0x740,   - 50 * 4 / 10, 0},    /* Ecuador */
    {0x744,   - 40 * 4 / 10, 0},    /* Paraguay */
    {0x746,   - 30 * 4 / 10, 0},    /* Suriname */
    {0x748,   - 30 * 4 / 10, 0},    /* Uruguay */
};

static AT_QUERY_TYPE_FUNC_STRU g_aAtQryTypeProcFuncTblMbb[] =
{
    {(VOS_UINT32)TAF_MM_PLMN_TIMEZONE_QRY_PARA, At_QryMmPlmnIdRspProc},
    {(VOS_UINT32)TAF_PH_HCSQ_PARA,     AT_QryParaRspHcsqProc},
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    {(VOS_UINT32)TAF_PH_CIND_VALUE_PARA,    AT_QryParaRspCindProc},
    {(VOS_UINT32)TAF_PH_ANT_VALUE_PARA,     AT_QryParaAntProc},
#endif
#if(FEATURE_ON == MBB_FEATURE_CELLROAM)
    {TAF_PH_CELLROAM_PARA,             At_QryParaRspCellRoamProc},
#endif/*MBB_FEATURE_CELLROAM*/
    {TAF_TELE_PARA_BUTT,               TAF_NULL_PTR}
};

AT_HFREQINFO_REPORT_TYPE      g_AtHFreqinforeport = AT_HFREQINFO_NO_REPORT;

VOS_CHAR g_szBufForDebug[BUFFER_LENGTH_256 + 1] = {0};

AT_DEBUG_INFO_STRU g_stAtDebugInfo = {
    .ucPcuiPsCallFlg = VOS_TRUE,
#if ( FEATURE_ON == MBB_FEATURE_GATEWAY )
    .usUserClientId  = AT_CLIENT_ID_NDIS,
#else /*FEATURE_ON == MBB_FEATURE_GATEWAY*/
    .usUserClientId  = AT_CLIENT_ID_APP,
#endif /*FEATURE_ON == MBB_FEATURE_GATEWAY*/
};

VOS_UINT8 g_ucIPv6VerFlag = AT_IPV6_SECOND_VERSION;

extern AT_SEND_DATA_BUFFER_STRU                gstAtSendData;   /* 单个命令的返回信息存储区 */
NAS_MM_INFO_IND_STRU    gstCCLKInfo = {
    .ucIeFlg = NAS_MM_INFO_IE_UTLTZ | NAS_MM_INFO_IE_LTZ,
    .cLocalTimeZone = AT_INVALID_TZ_VALUE,
    .ucDST = 0,
    .ulTimeSeconds = 0,  /*no do second time*/
    .stUniversalTimeandLocalTimeZone = {
            .ucYear          = 0,  /*CCLK year = ucYear+2000, 2000 year*/
            .ucMonth        = 1,    /*TIME 2000/01/06 08:00+00,00*/
            .ucDay           = 6,    /*TIME 2000/01/06 08:00+00,00*/
            .ucHour          = 8,    /*TIME 2000/01/06 08:00+00,00*/
            .ucMinute       = 0,
            .ucSecond      = 0,
            .cTimeZone    = AT_INVALID_TZ_VALUE,
            .Reserved     = 0,
    },
};

MBB_RAT_SUPPORT_STRU g_MbbIsRatSupport = {0};
TAF_PH_BAND_NAME_STRU    gastSyscfgexLteLowBandStr[] =
{
    {MN_MMA_LTE_EUTRAN_BAND1,        "LTE BC1" },
    {MN_MMA_LTE_EUTRAN_BAND2,        "LTE BC2" },
    {MN_MMA_LTE_EUTRAN_BAND3,        "LTE BC3" },
    {MN_MMA_LTE_EUTRAN_BAND4,        "LTE BC4" },
    {MN_MMA_LTE_EUTRAN_BAND5,        "LTE BC5" },
    {MN_MMA_LTE_EUTRAN_BAND6,        "LTE BC6" },
    {MN_MMA_LTE_EUTRAN_BAND7,        "LTE BC7" },
    {MN_MMA_LTE_EUTRAN_BAND8,        "LTE BC8" },
    {MN_MMA_LTE_EUTRAN_BAND9,        "LTE BC9" },
    {MN_MMA_LTE_EUTRAN_BAND10,       "LTE BC10"},
    {MN_MMA_LTE_EUTRAN_BAND11,       "LTE BC11"},
    {MN_MMA_LTE_EUTRAN_BAND12,       "LTE BC12"},
    {MN_MMA_LTE_EUTRAN_BAND13,       "LTE BC13"},
    {MN_MMA_LTE_EUTRAN_BAND14,       "LTE BC14"},
    {MN_MMA_LTE_EUTRAN_BAND15,       "LTE BC15"},
    {MN_MMA_LTE_EUTRAN_BAND16,       "LTE BC16"},
    {MN_MMA_LTE_EUTRAN_BAND17,       "LTE BC17"},
    {MN_MMA_LTE_EUTRAN_BAND18,       "LTE BC18"},
    {MN_MMA_LTE_EUTRAN_BAND19,       "LTE BC19"},
    {MN_MMA_LTE_EUTRAN_BAND20,       "LTE BC20"},
    {MN_MMA_LTE_EUTRAN_BAND21,       "LTE BC21"},
    {MN_MMA_LTE_EUTRAN_BAND22,       "LTE BC22"},
    {MN_MMA_LTE_EUTRAN_BAND23,       "LTE BC23"},
    {MN_MMA_LTE_EUTRAN_BAND24,       "LTE BC24"},
    {MN_MMA_LTE_EUTRAN_BAND25,       "LTE BC25"},
    {MN_MMA_LTE_EUTRAN_BAND26,       "LTE BC26"},
    {MN_MMA_LTE_EUTRAN_BAND27,       "LTE BC27"},
    {MN_MMA_LTE_EUTRAN_BAND28,       "LTE BC28"},
    {MN_MMA_LTE_EUTRAN_BAND29,       "LTE BC29"},
    {MN_MMA_LTE_EUTRAN_BAND30,       "LTE BC30"},
    {MN_MMA_LTE_EUTRAN_BAND31,       "LTE BC31"},
    {MN_MMA_LTE_EUTRAN_BAND32,       "LTE BC32"}
};

TAF_PH_BAND_NAME_STRU    gastSyscfgexLteHighBandStr[] =
{
    {MN_MMA_LTE_EUTRAN_BAND33,       "LTE BC33"},
    {MN_MMA_LTE_EUTRAN_BAND34,       "LTE BC34"},
    {MN_MMA_LTE_EUTRAN_BAND35,       "LTE BC35"},
    {MN_MMA_LTE_EUTRAN_BAND36,       "LTE BC36"},
    {MN_MMA_LTE_EUTRAN_BAND37,       "LTE BC37"},
    {MN_MMA_LTE_EUTRAN_BAND38,       "LTE BC38"},
    {MN_MMA_LTE_EUTRAN_BAND39,       "LTE BC39"},
    {MN_MMA_LTE_EUTRAN_BAND40,       "LTE BC40"},
    {MN_MMA_LTE_EUTRAN_BAND41,       "LTE BC41"},
    {MN_MMA_LTE_EUTRAN_BAND42,       "LTE BC42"},
    {MN_MMA_LTE_EUTRAN_BAND43,       "LTE BC43"},
    {MN_MMA_LTE_EUTRAN_BAND44,       "LTE BC44"},
    {MN_MMA_LTE_EUTRAN_BAND45,       "LTE BC45"},
    {MN_MMA_LTE_EUTRAN_BAND46,       "LTE BC46"},
    {MN_MMA_LTE_EUTRAN_BAND47,       "LTE BC47"},
    {MN_MMA_LTE_EUTRAN_BAND48,       "LTE BC48"},
    {MN_MMA_LTE_EUTRAN_BAND49,       "LTE BC49"},
    {MN_MMA_LTE_EUTRAN_BAND50,       "LTE BC50"},
    {MN_MMA_LTE_EUTRAN_BAND51,       "LTE BC51"},
    {MN_MMA_LTE_EUTRAN_BAND52,       "LTE BC52"},
    {MN_MMA_LTE_EUTRAN_BAND53,       "LTE BC53"},
    {MN_MMA_LTE_EUTRAN_BAND54,       "LTE BC54"},
    {MN_MMA_LTE_EUTRAN_BAND55,       "LTE BC55"},
    {MN_MMA_LTE_EUTRAN_BAND56,       "LTE BC56"},
    {MN_MMA_LTE_EUTRAN_BAND57,       "LTE BC57"},
    {MN_MMA_LTE_EUTRAN_BAND58,       "LTE BC58"},
    {MN_MMA_LTE_EUTRAN_BAND59,       "LTE BC59"},
    {MN_MMA_LTE_EUTRAN_BAND60,       "LTE BC60"},
    {MN_MMA_LTE_EUTRAN_BAND61,       "LTE BC61"},
    {MN_MMA_LTE_EUTRAN_BAND62,       "LTE BC62"},
    {MN_MMA_LTE_EUTRAN_BAND63,       "LTE BC63"},
    {MN_MMA_LTE_EUTRAN_BAND64,       "LTE BC64"}
};

#ifdef BSP_CONFIG_BOARD_WINGLE_MS2172S_818
#if(FEATURE_OFF == MBB_FEATURE_BOX_FTEN)
AT_LOCK_CMD_TAB_STRU gastAtLockLimitCmdTab[] = /*通用版本AT黑名单限制*/
{
    {AT_CMD_PORT, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SETPORT", VOS_NULL, VOS_NULL},
};
const TAF_UINT16 gusAtLockLimitCmdNum = sizeof(gastAtLockLimitCmdTab) / sizeof(AT_LOCK_CMD_TAB_STRU);
#endif
#endif

#if (FEATURE_ON == MBB_OPERATOR_INDUSTRYCARD)
VOS_INT8 g_UpdateModeIsNewFlag = 0;
VOS_INT8 g_UpdateModeValue[UPDATE_MODE_DATA_LEN + 1] = {0};
VOS_INT8 g_UpdateCheckValue = 0;
#endif

#if (FEATURE_ON == MBB_FEATURE_AT_CMD_FILTER)
AT_LOCK_CMD_TAB_STRU gastAtLockCmdTab[] = /*lint !e129 !e10*/
{
    /*general commands*/
#ifdef BSP_CONFIG_BOARD_K5160
    {AT_CMD_SYSINFOEX, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SYSINFOEX", VOS_NULL, VOS_NULL},
    {AT_CMD_NDISDUP, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"^NDISDUP", VOS_NULL, VOS_NULL},
    {AT_CMD_DHCP, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^DHCP", VOS_NULL, VOS_NULL},
    {AT_CMD_HVER, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^HVER", VOS_NULL, VOS_NULL},
    {AT_CMD_CGDCONT, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CGDCONT", VOS_NULL, VOS_NULL},
    {AT_CMD_SYSINFO, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SYSINFO", VOS_NULL, VOS_NULL},
    {AT_CMD_SYSCFG, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SYSCFG", VOS_NULL, VOS_NULL},
    {AT_CMD_COPS, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+COPS", VOS_NULL, VOS_NULL},
    {AT_CMD_CPWD, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CPWD", VOS_NULL, VOS_NULL},
    {AT_CMD_CLCK, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CLCK", VOS_NULL, VOS_NULL},
    {AT_CMD_CPIN, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CPIN", VOS_NULL, VOS_NULL},
    {AT_CMD_CPIN_2, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CPIN", VOS_NULL, VOS_NULL},
    {AT_CMD_CFUN, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CFUN", VOS_NULL, VOS_NULL},
    {AT_CMD_CARDMODE, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CARDMODE", VOS_NULL, VOS_NULL},
    {AT_CMD_CSQ, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CSQ", VOS_NULL, VOS_NULL},
    {AT_CMD_STSF, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"^STSF", VOS_NULL, VOS_NULL},
    {AT_CMD_A, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"A", VOS_NULL, VOS_NULL},
    {AT_CMD_D, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"D", VOS_NULL, VOS_NULL},
    {AT_CMD_CHUP, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CHUP", VOS_NULL, VOS_NULL},
    {AT_CMD_CLIP, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CLIP", VOS_NULL, VOS_NULL},
    {AT_CMD_DTMF, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^DTMF", VOS_NULL, VOS_NULL},
    {AT_CMD_DDSETEX, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE, (VOS_UINT8*)"^DDSETEX", VOS_NULL, VOS_NULL},
    {AT_CMD_SCID, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SCID", VOS_NULL, VOS_NULL},
    {AT_CMD_I, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"I", VOS_NULL, VOS_NULL},
    {AT_CMD_CCFC, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CCFC", VOS_NULL, VOS_NULL},
    {AT_CMD_CCWA, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CCWA", VOS_NULL, VOS_NULL},
    {AT_CMD_CHLD, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CHLD", VOS_NULL, VOS_NULL},
    {AT_CMD_CSSN, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CSSN", VOS_NULL, VOS_NULL},
    {AT_CMD_CREG, CMD_TBL_NO_LIMITED, (VOS_UINT8*)"+CREG", VOS_NULL, VOS_NULL},
    {AT_CMD_DSFLOWQRY, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^DSFLOWQRY", VOS_NULL, VOS_NULL},
    {AT_CMD_DSFLOWCLR, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^DSFLOWCLR", VOS_NULL, VOS_NULL},
    {AT_CMD_DSFLOWRPT, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^DSFLOWRPT", VOS_NULL, VOS_NULL},
    {AT_CMD_DIALMODE, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_STICK, (VOS_UINT8*)"^DIALMODE", VOS_NULL, VOS_NULL},
    {AT_CMD_SYSCFGEX, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SYSCFGEX", VOS_NULL, VOS_NULL},
    {AT_CMD_H, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"H", VOS_NULL, VOS_NULL},
    {AT_CMD_CGEQREQ, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CGEQREQ", VOS_NULL, VOS_NULL},
    {AT_CMD_CSNR, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CSNR", VOS_NULL, VOS_NULL},
    {AT_CMD_CGATT, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CGATT", VOS_NULL, VOS_NULL},
    {AT_CMD_CLVL, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CLVL",VOS_NULL, VOS_NULL},
    {AT_CMD_CGREG, CMD_TBL_NO_LIMITED, (VOS_UINT8*)"+CGREG",VOS_NULL, VOS_NULL},
    {AT_CMD_DHCPV6,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"^DHCPV6",VOS_NULL, VOS_NULL},
    {AT_CMD_NDISSTATQRY,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"^NDISSTATQRY",VOS_NULL, VOS_NULL},
    {AT_CMD_CGEQNEG,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CGEQNEG",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGF,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"+CMGF",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGL,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGL",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGS,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGS",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGW,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGW",VOS_NULL, VOS_NULL},
    {AT_CMD_CNMI,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"+CNMI",VOS_NULL, VOS_NULL},
    {AT_CMD_CNUM,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CNUM",VOS_NULL, VOS_NULL},
    {AT_CMD_CPMS,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CPMS",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGD,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGD",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGR,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGR",VOS_NULL, VOS_NULL},
    {AT_CMD_CMSS,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMSS",VOS_NULL, VOS_NULL},
    {AT_CMD_CUSD,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CUSD",VOS_NULL, VOS_NULL},
    {AT_CMD_VTS,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+VTS",VOS_NULL, VOS_NULL},
    {AT_CMD_D,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"D",VOS_NULL, VOS_NULL},
#endif
    {AT_CMD_E,   CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"E", VOS_NULL, VOS_NULL},
    {AT_CMD_S0, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"S0", VOS_NULL, VOS_NULL},
    {AT_CMD_S3, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"S3", VOS_NULL, VOS_NULL},
    {AT_CMD_S4, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"S4", VOS_NULL, VOS_NULL},
    {AT_CMD_S5, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"S5", VOS_NULL, VOS_NULL},
    {AT_CMD_V,   CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"V",   VOS_NULL, VOS_NULL},
    {AT_CMD_I,    CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"I", VOS_NULL, VOS_NULL},
    {AT_CMD_GCAP, CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+GCAP", VOS_NULL, VOS_NULL},
    {AT_CMD_GMI, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+GMI", VOS_NULL, VOS_NULL},
    {AT_CMD_CGMI, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CGMI",VOS_NULL, VOS_NULL},
    {AT_CMD_CGMM, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CGMM",VOS_NULL, VOS_NULL},
    {AT_CMD_GMM, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+GMM",VOS_NULL, VOS_NULL},
    {AT_CMD_GMR, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+GMR", VOS_NULL, VOS_NULL},
    {AT_CMD_CGMR, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CGMR",VOS_NULL, VOS_NULL},
    {AT_CMD_CGSN,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CGSN",VOS_NULL, VOS_NULL},
    {AT_CMD_GSN, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+GSN", VOS_NULL, VOS_NULL},
    {AT_CMD_CSCS, CMD_TBL_LIMITED_NULL,  (TAF_UINT8*)"+CSCS", VOS_NULL, VOS_NULL},   
    {AT_CMD_Z,   CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"Z", VOS_NULL, VOS_NULL},
    {AT_CMD_AMP_F, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"&F", VOS_NULL, VOS_NULL},
    /*A/ &w &v \s命令巴龙平台本台不支持 因此不在此处添加*/
    {AT_CMD_Q, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5, (VOS_UINT8*)"Q", VOS_NULL, VOS_NULL},
    {AT_CMD_CCLK, CMD_TBL_NO_LIMITED, (VOS_UINT8*)"+CCLK", VOS_NULL, VOS_NULL},
    {AT_CMD_X, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5, (VOS_UINT8*)"X", VOS_NULL, VOS_NULL},
    {AT_CMD_CRC, CMD_TBL_LIMITED_NULL,  (VOS_UINT8*)"+CRC", VOS_NULL, VOS_NULL},
    /*Serial Interface Control Commands */
    {AT_CMD_AMP_C, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5, (VOS_UINT8*)"&C", VOS_NULL, VOS_NULL},
    {AT_CMD_AMP_D, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5, (VOS_UINT8*)"&D", VOS_NULL, VOS_NULL},
#if (FEATURE_ON == FEATURE_AT_HSUART)
    {AT_CMD_AMP_S, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5, (VOS_UINT8*)"&S", VOS_NULL, VOS_NULL},
    {AT_CMD_IPR, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8 *)"+IPR", VOS_NULL, VOS_NULL},
    {AT_CMD_ICF, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8 *)"+ICF",  VOS_NULL, VOS_NULL},
    {AT_CMD_IFC, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8 *)"+IFC",  VOS_NULL, VOS_NULL},
#endif
    /*7 Mobile Termination errors*/
    {AT_CMD_CMEE, CMD_TBL_PIN_IS_LOCKED, (TAF_UINT8*)"+CMEE",  VOS_NULL, VOS_NULL},
    {AT_CMD_CEER, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE, (TAF_UINT8*)"+CEER", VOS_NULL, VOS_NULL},
     /*doad command AUTHVERYFY DATAMODE命令巴龙平台不支持*/
    {AT_CMD_DLOADVER, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^DLOADVER",VOS_NULL, VOS_NULL},
    {AT_CMD_DLOADINFO, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^DLOADINFO", VOS_NULL, VOS_NULL},
    {AT_CMD_NVBACKUP, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^NVBACKUP", VOS_NULL, VOS_NULL},
    {AT_CMD_NVRESTORE, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^NVRESTORE",VOS_NULL, VOS_NULL},
    {AT_CMD_AUTHORITYVER, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^AUTHORITYVER", VOS_NULL, VOS_NULL},
    {AT_CMD_AUTHORITYID, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^AUTHORITYID", VOS_NULL, VOS_NULL},
    {AT_CMD_GODLOAD,  CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^GODLOAD",VOS_NULL, VOS_NULL},
    {AT_CMD_RESET, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^RESET",VOS_NULL, VOS_NULL},
    {AT_CMD_NVRSTSTTS,  CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^NVRSTSTTS",VOS_NULL, VOS_NULL},
    /*factory command */
    {AT_CMD_TMODE, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TMODE", VOS_NULL, VOS_NULL},    
    {AT_CMD_SN, CMD_TBL_PIN_IS_LOCKED, (TAF_UINT8*)"^SN", VOS_NULL, VOS_NULL},
    {AT_CMD_SD, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SD", VOS_NULL, VOS_NULL},
    {AT_CMD_BSN, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^BSN", VOS_NULL, VOS_NULL},
    {AT_CMD_GPIOPL, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^GPIOPL", VOS_NULL, VOS_NULL},
    {AT_CMD_PHYNUM, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^PHYNUM", VOS_NULL, VOS_NULL},
    {AT_CMD_RSIM, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^RSIM", VOS_NULL, VOS_NULL},
    {AT_CMD_INFORBU, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^INFORBU",  VOS_NULL, VOS_NULL},
    {AT_CMD_DATALOCK, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^DATALOCK", VOS_NULL, VOS_NULL},
    {AT_CMD_CSVER, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CSVER", VOS_NULL, VOS_NULL},
    {AT_CMD_PORTLOCK, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^PORTLOCK", VOS_NULL, VOS_NULL},
    {AT_CMD_SIMLOCK,   CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SIMLOCK",  VOS_NULL, VOS_NULL},
    {AT_CMD_CSDFLT, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CSDFLT", VOS_NULL, VOS_NULL},
    {AT_CMD_FCHAN, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^FCHAN", VOS_NULL, VOS_NULL},
    {AT_CMD_FTXON, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^FTXON",  VOS_NULL, VOS_NULL},
    {AT_CMD_FRXON,CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^FRXON",   VOS_NULL, VOS_NULL},
    {AT_CMD_FLNA, CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"^FLNA",     VOS_NULL, VOS_NULL},
    {AT_CMD_FRSSI, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^FRSSI",    VOS_NULL, VOS_NULL},
    {AT_CMD_PLATFORM, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^PLATFORM", VOS_NULL, VOS_NULL},
    {AT_CMD_AUDIO, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^AUDIO", VOS_NULL, VOS_NULL},
    {AT_CMD_FACINFO,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^FACINFO",  VOS_NULL, VOS_NULL},
    {AT_CMD_VERSION,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^VERSION",  VOS_NULL, VOS_NULL},
    {AT_CMD_MAXLCK_TIMES,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^MAXLCKTMS",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIENABLE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIENABLE",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIMODE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIMODE",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIBAND,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIBAND",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIFREQ,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIFREQ",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIRATE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIDATARATE",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIPOW,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIPOW",  VOS_NULL, VOS_NULL},
    {AT_CMD_WITX,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WITX",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIRX,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIRX",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIRPCKG,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIRPCKG",  VOS_NULL, VOS_NULL},
    {AT_CMD_TMMI,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TMMI",  VOS_NULL, VOS_NULL},
    {AT_CMD_PRODTYPE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^PRODTYPE",  VOS_NULL, VOS_NULL},
    {AT_CMD_TSELRF,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TSELRF",  VOS_NULL, VOS_NULL},
    {AT_CMD_ANTENNA,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^ANTENNA",  VOS_NULL, VOS_NULL},
    {AT_CMD_TCHRENABLE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TCHRENABLE",  VOS_NULL, VOS_NULL},
    {AT_CMD_SSID,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SSID",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIKEY,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIKEY",  VOS_NULL, VOS_NULL},
    {AT_CMD_WUPWD,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WUPWD",  VOS_NULL, VOS_NULL},
    {AT_CMD_FEATURE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SFEATURE",  VOS_NULL, VOS_NULL},
    {AT_CMD_FWAVE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^FWAVE",  VOS_NULL, VOS_NULL},
    {AT_CMD_EQVER,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^EQVER",  VOS_NULL, VOS_NULL},
    {AT_CMD_TBAT,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TBAT",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIINFO,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIINFO",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIPARANGE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIPARANGE",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIWEP,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIWEP",  VOS_NULL, VOS_NULL},
    {AT_CMD_PSTANDBY,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^PSTANDBY",  VOS_NULL, VOS_NULL},
    {AT_CMD_SFM,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SFM",  VOS_NULL, VOS_NULL},
    {AT_CMD_TBATDATA,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TBATDATA",  VOS_NULL, VOS_NULL},
    {AT_CMD_WITX,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIPLATFORM",  VOS_NULL, VOS_NULL},
    {AT_CMD_WUSITE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WUSITE",  VOS_NULL, VOS_NULL},
    {AT_CMD_WIPIN,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WIPIN",  VOS_NULL, VOS_NULL},
    {AT_CMD_TNETPORT,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^TNETPORT",  VOS_NULL, VOS_NULL},
    {AT_CMD_EXTCHARGE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^EXTCHARGE",  VOS_NULL, VOS_NULL},
    /*CSTCONFIG命令不支持*/
    {AT_CMD_WUUSER,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^WUUSER",  VOS_NULL, VOS_NULL},
    /*GPIOLOOP[17]命令不支持*/
    {AT_CMD_CIMI,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CIMI",  VOS_NULL, VOS_NULL},
    {AT_CMD_CARDLOCK,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CARDLOCK",  VOS_NULL, VOS_NULL},
    {AT_CMD_RSFR,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^RSFR",  VOS_NULL, VOS_NULL},
    {AT_CMD_RSFW,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^RSFW",  VOS_NULL, VOS_NULL},
    {AT_CMD_CMDLEN,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CMDLEN",  VOS_NULL, VOS_NULL},
    {AT_CMD_CURC,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^CURC",  VOS_NULL, VOS_NULL},
    {AT_CMD_BODYSARON,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^BODYSARON",  VOS_NULL, VOS_NULL},
    {AT_CMD_BODYSARWCDMA,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^BODYSARWCDMA",  VOS_NULL, VOS_NULL},
    {AT_CMD_BODYSARGSM,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^BODYSARGSM",  VOS_NULL, VOS_NULL},
    /*^BODYSARCDMA BODYSARLTE命令不支持*/
    {AT_CMD_CICCID,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^ICCID",  VOS_NULL, VOS_NULL},
    {AT_CMD_SECUBOOT,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SECUBOOT",  VOS_NULL, VOS_NULL},
    {AT_CMD_SECUBOOTFEATURE,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^SECUBOOTFEATURE",  VOS_NULL, VOS_NULL},
    {AT_CMD_AUTHVER,  CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"^AUTHVER",  VOS_NULL, VOS_NULL},
    /*^ACCOUNT *SKU命令不支持*/
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    {AT_CMD_USIMSTATUS, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"*USIMSTATUS", VOS_NULL, VOS_NULL},
    {AT_CMD_RESET, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"*RESET",VOS_NULL, VOS_NULL},
    {AT_CMD_CICCID, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"*ICCID",  VOS_NULL, VOS_NULL},
    {AT_CMD_CGDCONT, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"+CGDCONT", VOS_NULL, VOS_NULL},
    {AT_CMD_AMP_F, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"&F", VOS_NULL, VOS_NULL},
    {AT_CMD_ERROR, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"*ERROR", VOS_NULL, VOS_NULL},
    {AT_CMD_ANT, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"*ANT", VOS_NULL, VOS_NULL},
    {AT_CMD_QOSIG, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"*QOSIG", VOS_NULL, VOS_NULL},
    {AT_CMD_CGATT, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CGATT", VOS_NULL, VOS_NULL},
    {AT_CMD_CMER, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CMER", VOS_NULL, VOS_NULL},
    {AT_CMD_CIND, CMD_TBL_LIMITED_NULL, (VOS_UINT8*)"+CIND", VOS_NULL, VOS_NULL},
    {AT_CMD_CNUM,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CNUM",VOS_NULL, VOS_NULL},
    {AT_CMD_GMM,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"+GMM",VOS_NULL, VOS_NULL},
    {AT_CMD_GMR,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"+GMR",VOS_NULL, VOS_NULL},
    {AT_CMD_D,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"D",VOS_NULL, VOS_NULL},
    {AT_CMD_S7,CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,(VOS_UINT8*)"S7",VOS_NULL, VOS_NULL},
    {AT_CMD_H, CMD_TBL_PIN_IS_LOCKED, (VOS_UINT8*)"H", VOS_NULL, VOS_NULL},
    {AT_CMD_CMGF,CMD_TBL_PIN_IS_LOCKED,(VOS_UINT8*)"+CMGF",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGL,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGL",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGD,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGD",VOS_NULL, VOS_NULL},
    {AT_CMD_CMGR,CMD_TBL_LIMITED_NULL,(VOS_UINT8*)"+CMGR",VOS_NULL, VOS_NULL},
#endif
#if(FEATURE_ON == MBB_FEATURE_ESIM_SWITCH)
    {AT_CMD_TSIMSWITCH, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED, (VOS_UINT8*)"^TSIMSWITCH",VOS_NULL, VOS_NULL},
#endif
};
const TAF_UINT16 gusAtLockCmdNum = sizeof(gastAtLockCmdTab) / sizeof(AT_LOCK_CMD_TAB_STRU);


PRIVATE VOS_UINT32 At_MatchLockCmdName(VOS_UINT8 ucIndex, VOS_CHAR *pszCmdName)
{
    VOS_UINT32  i = 0;
    for (i = 0; i < gusAtLockCmdNum; i++)
    {
        if (VOS_NULL_PTR != (TAF_CHAR*)gastAtLockCmdTab[i].pszCmdName) /*lint !e10*/
        {
            if (ERR_MSP_SUCCESS == AT_STRCMP(pszCmdName,(TAF_CHAR*)gastAtLockCmdTab[i].pszCmdName))/*lint !e10*/
            {
                return AT_SUCCESS;
            }
        }
    }

    return AT_FAILURE;
}

VOS_UINT32 At_IsForbiddenAtCmd(VOS_CHAR* pszCmdName, VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRet = VOS_TRUE;
    if((AT_USBCOM_USER == gastAtClientTab[ucClientId].UserType)
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    || (AT_MODEM_USER == gastAtClientTab[ucClientId].UserType)
#endif
      )
    {
        if(AT_SUCCESS == At_MatchLockCmdName(ucClientId, pszCmdName))
        {
            ulRet = VOS_TRUE;/*AT在白名单中*/
        }
        else
        {
            ulRet = VOS_FALSE;/*AT从PCUI口下发且不是白名单中的数据*/
        }
    }
    return ulRet;
}
#endif
#ifdef BSP_CONFIG_BOARD_WINGLE_MS2172S_818
#if(FEATURE_OFF == MBB_FEATURE_BOX_FTEN)


PRIVATE VOS_UINT32 At_MatchLockGlobalCmdName(VOS_UINT8 ucIndex, VOS_CHAR *pszCmdName)
{
    VOS_UINT32  i = 0;
    for (i = 0; i < gusAtLockLimitCmdNum; i++)
    {
        if (VOS_NULL_PTR != (TAF_CHAR*)gastAtLockLimitCmdTab[i].pszCmdName) /*lint !e10*/
        {
            if (ERR_MSP_SUCCESS == AT_STRCMP(pszCmdName,(TAF_CHAR*)gastAtLockLimitCmdTab[i].pszCmdName))/*lint !e10*/
            {
                return AT_SUCCESS;
            }
        }
    }
    return AT_FAILURE;
}


VOS_UINT32 At_IsGlobalForbiddenAtCmd(VOS_CHAR* pszCmdName, VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRet = VOS_FALSE;
    if((AT_USBCOM_USER == gastAtClientTab[ucClientId].UserType) || (AT_MODEM_USER == gastAtClientTab[ucClientId].UserType))
    {
        if(AT_SUCCESS == At_MatchLockGlobalCmdName(ucClientId, pszCmdName))
        {
            ulRet = VOS_TRUE;
        }
        else
        {
            ulRet = VOS_FALSE;
        }
    }
    return ulRet;
}
#endif
#endif
#if (FEATURE_ON == MBB_WPG_HFEATURESTAT)
static  TAF_UINT8  gucIsSinglePdn = AT_HFEATURE_NOT_OPEN;
/*********************************************************************************************
 函 数 名  : AT_ReadTafPdpParaNV
 功能描述  : at 初始化时通过读取单板频段NV判断RAT支持情况
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
 修改历史  :
*********************************************************************************************/
VOS_VOID  AT_ReadTafPdpParaNV(VOS_VOID)
{
    TAF_NVIM_PDP_PARA_STRU              *pstNvPdpTable = VOS_NULL_PTR;

    pstNvPdpTable = (TAF_NVIM_PDP_PARA_STRU *)MBB_MEM_ALLOC(WUEPS_PID_TAF, sizeof(TAF_NVIM_PDP_PARA_STRU));
    if (VOS_NULL_PTR == pstNvPdpTable)
    {
        return;
    }
    /*lint -e516*/
    MBB_MEM_SET(pstNvPdpTable, 0x00, sizeof(TAF_NVIM_PDP_PARA_STRU));
    /*lint +e516*/
    if (NV_OK != NV_Read(en_NV_Item_Taf_PdpPara_0, pstNvPdpTable, sizeof(TAF_NVIM_PDP_PARA_STRU)))
    {
       /*lint -e516*/
        MBB_MEM_FREE(WUEPS_PID_AT, pstNvPdpTable);
       /*lint +e516*/
        return;
    }

    /*该PDP没有定义或者该PDP被定义成二次PDP*/
    if ((TAF_FREE == pstNvPdpTable->aucPdpPara[0]) || (TAF_USED == pstNvPdpTable->aucPdpPara[2]))
    {
        gucIsSinglePdn = AT_HFEATURE_OPEN;
    }
    /*lint -e516*/
    MBB_MEM_FREE(WUEPS_PID_AT, pstNvPdpTable);
    /*lint +e516*/
    pstNvPdpTable = VOS_NULL_PTR;
}/*lint !e438*/



static TAF_UINT32 At_SetHFeaturestat( TAF_UINT8 ucIndex )
{
   VOS_UINT8 ucFeatureId = 0;

   /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /*参数长度不为1*/
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }

    ucFeatureId = (TAF_UINT8)(gastAtParaList[0].ulParaValue);

    if(AT_HFEATURE_SINGLE_PDN != ucFeatureId)
    {
        return AT_ERROR;/*目前仅支持single_pdn特性的查询设置*/
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
               (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
               "%s: %d,%d",
               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
               ucFeatureId,
               gucIsSinglePdn);
    
    return AT_OK;
}

static TAF_UINT32 At_QryHFeaturestat( TAF_UINT8 ucIndex )
{
    VOS_UINT8 ucFeatureId = AT_HFEATURE_SINGLE_PDN;/*目前仅支持SINGLE PDN特性*/
   
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
                "%s: %d,%d",
                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                ucFeatureId,
                gucIsSinglePdn);
    
    return AT_OK;
}
#endif/*FEATURE_ON == MBB_WPG_HFEATURESTAT*/
#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)

VOS_UINT32 At_SetLteProfileParaCheckParameter(VOS_UINT8 ucIndex)
{
    /* "(0,1),(0-65535),(\"IP\",\"IPV6\",\"IPV4V6\"),(IMSIPREFIX),(APN),(UserName),(UserPwd),(0-3)"  */

    /* APP端口可以正常读写 */
    if ((VOS_TRUE == g_bAtDataLocked) && (AT_CLIENT_ID_APP != ucIndex) && (AT_CLIENT_ID_APP1 != ucIndex))
    {
        return AT_ERROR;
    }
    
    /*check R/W, 不能为空 */
    if (0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*check IPtype, 只能是IP/IPV6/IPV4V6，不能为空 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 读取只要有两个参数就够了 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        if ( 2 < gucAtParaIndex)  /* 不能多于两个  */
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        return AT_SUCCESS;
    }

    if (2 == gucAtParaIndex) /* 写入时只有两个参数，表示删除 */
    {
        return AT_SUCCESS;
    }
    
    /* check ImsiPrefix */
    if ((MAX_LTE_APN_IMSI_PREFIX_SUPPORT < gastAtParaList[3].usParaLen)     /*imsiprefix length must in [5-10]*/
        || (MIN_LTE_APN_IMSI_PREFIX_SUPPORT > gastAtParaList[3].usParaLen)) /*imsiprefix length must in [5-10]*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* check ImsiPrefix format */
    if (AT_SUCCESS != At_CheckNumString(gastAtParaList[3].aucPara, gastAtParaList[3].usParaLen)) /*imsiprefix is digit*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*check apn length*/
    if ((MAX_LTE_ATTACH_APN_NAME_LEN < gastAtParaList[4].usParaLen) || (0 == gastAtParaList[4].usParaLen)) /*apn len in [0,32]*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* check apn format */
    if( VOS_OK != AT_CheckApnFormat(gastAtParaList[4].aucPara, gastAtParaList[4].usParaLen)) /*check apn format*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* check name and p-word length */
    if ((MAX_LTE_ATTACH_APN_USERNAME_LEN < gastAtParaList[5].usParaLen) /*userName len in[0,32]*/
        || (MAX_LTE_ATTACH_APN_USERPWD_LEN < gastAtParaList[6].usParaLen)) /*userPwd len in[0,32]*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*检查authType不为空 */
    if ((0 == gastAtParaList[7].usParaLen) || (TAF_PDP_AUTH_TYPE_BUTT <= gastAtParaList[7].usParaLen))/*authType check*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*检查profileName不为空 */
    if (0 == gastAtParaList[8].usParaLen) /* profileName check*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_SUCCESS != At_AsciiNum2HexString(gastAtParaList[8].aucPara, &gastAtParaList[8].usParaLen)) /*profileName covert*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    return AT_SUCCESS;
}


VOS_UINT32 At_QryLteProfilePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32    ulResult = VOS_ERR;
    TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU stGetCustomAttachApn;

    PS_MEM_SET(&stGetCustomAttachApn, 0, sizeof(TAF_PS_CUSTOM_ATTACH_APN_INFO_EXT_STRU));
    stGetCustomAttachApn.usIndex = gastAtParaList[1].ulParaValue;
    
    ulResult = TAF_PS_GetCustomAttachApnReq(WUEPS_PID_AT,
                                            AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                            0,
                                            &stGetCustomAttachApn);
    if (VOS_OK == ulResult)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTEPROFILE_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 At_SetLteProfilePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32    ulResult = VOS_ERR;
    TAF_CUSTOM_ATTACH_APN_INFO stSetCustomAttachApn;
    NV_LTE_ATTACH_PROFILE_STRU  *pstNvApnInfo = VOS_NULL_PTR;
    
    /* "(0,1),(0-65535),(\"IP\",\"IPV6\",\"IPV4V6\"),(IMSIPREFIX),(APN),(UserName),(UserPwd),(0-3)"  */
    ulResult = At_SetLteProfileParaCheckParameter(ucIndex);
    if (AT_SUCCESS != ulResult)
    {
        return ulResult;
    }

    /* 0表示读取 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        return At_QryLteProfilePara(ucIndex);
    }

    pstNvApnInfo = &stSetCustomAttachApn.stNvAttachApnInfo;
    PS_MEM_SET(&stSetCustomAttachApn, 0, sizeof(TAF_CUSTOM_ATTACH_APN_INFO));

    stSetCustomAttachApn.stExInfo.usIndex = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stSetCustomAttachApn.stExInfo.usFilter = 0;

    /* 只允许设置序号为0的定制APN,debug版方便调试允许设置 */
#if (FEATURE_OFF == MBB_BUILD_DEBUG)
    if (0 < stSetCustomAttachApn.stExInfo.usIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    if ( 2 == gucAtParaIndex) /* 写入时只有两个参数，表示删除 */
    {
        pstNvApnInfo->ucActiveFlag = VOS_FALSE;
    }
    else
    {
        pstNvApnInfo->ucActiveFlag = VOS_TRUE;
        /*设置 IpType  */
        /* IP:1, IPV6:2, IPV4V6:3, */
        pstNvApnInfo->ucPdpType = (VOS_UINT8)(gastAtParaList[2].ulParaValue + 1); /* param2 IpType */

        if ( (TAF_PDP_IPV6   == pstNvApnInfo->ucPdpType)
          || (TAF_PDP_IPV4V6 == pstNvApnInfo->ucPdpType) )
        {
#if (FEATURE_ON == FEATURE_IPV6)
            if (AT_IPV6_CAPABILITY_IPV4_ONLY == AT_GetIpv6Capability())
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }
#else
            return AT_CME_INCORRECT_PARAMETERS;
#endif
        }
        
        /*设置 imsiPrefix  */
        pstNvApnInfo->ucImsiPrefixLen = (VOS_UINT8)gastAtParaList[3].usParaLen; /* param3 imsiprefix */
        (VOS_VOID)At_AsciiString2HexSimple(pstNvApnInfo->aucImsiPrefixBcd,
                    gastAtParaList[3].aucPara, pstNvApnInfo->ucImsiPrefixLen);/* param3 imsiprefix hex to bcd */

        /*设置 apn  */
        pstNvApnInfo->ucApnLen = (VOS_UINT8)gastAtParaList[4].usParaLen;/* param4 apn len */
        PS_MEM_CPY(pstNvApnInfo->aucApn, gastAtParaList[4].aucPara, pstNvApnInfo->ucApnLen);/* param4 apn value save */

        /*设置 auth info */
        pstNvApnInfo->ucUserNameLen = (VOS_UINT8)gastAtParaList[5].usParaLen; /* param5 user len */
        if (pstNvApnInfo->ucUserNameLen > 0)
        {
            PS_MEM_CPY(pstNvApnInfo->aucUserName, gastAtParaList[5].aucPara, pstNvApnInfo->ucUserNameLen);/* param5 user save */
        }

        pstNvApnInfo->ucPwdLen = (VOS_UINT8)gastAtParaList[6].usParaLen; /* param6 pwd len */
        if (pstNvApnInfo->ucPwdLen > 0)
        {
            PS_MEM_CPY(pstNvApnInfo->aucPwd, gastAtParaList[6].aucPara, pstNvApnInfo->ucPwdLen);/* param6 pwd save */
        }

        pstNvApnInfo->ucAuthType = gastAtParaList[7].ulParaValue; /* param7 authType */

        if (MAX_LTE_ATTACH_PROFILE_NAME_LEN <= gastAtParaList[8].usParaLen)/*profileName check */
        {
            gastAtParaList[8].usParaLen = MAX_LTE_ATTACH_PROFILE_NAME_LEN - 1; /*AT手册一致， profileName 18字节自动截断  */
        }
        pstNvApnInfo->ucProfileNameLen = (VOS_UINT8)gastAtParaList[8].usParaLen;/*profileName*/
        if (pstNvApnInfo->ucProfileNameLen > 0)
        {
            PS_MEM_CPY(pstNvApnInfo->aucProfileName, gastAtParaList[8].aucPara, pstNvApnInfo->ucProfileNameLen);/* para8 save */
        }
    }
    
    ulResult = TAF_PS_SetCustomAttachApnReq(WUEPS_PID_AT,
                                            AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                            0,
                                            &stSetCustomAttachApn);
    if (VOS_OK == ulResult)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTEPROFILE_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 At_TestLteProfilePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32    ulResult = VOS_ERR;

    ulResult = TAF_PS_TestCustomAttachApnReq(WUEPS_PID_AT,
                            AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                            0);
    if (VOS_OK == ulResult)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTEPROFILE_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_QryApnAttachSwitch(VOS_UINT8 ucIndex)
{

    NV_LTE_ATTACH_PROFILE_CTRL stLteApnAttachCtrl = {0};

    VOS_UINT32 ulResult = AT_ERROR;

    ulResult = NV_Read(NV_ID_LTE_ATTACH_PROFILE_CONTROL,
                        &stLteApnAttachCtrl, sizeof(NV_LTE_ATTACH_PROFILE_CTRL));
    if (NV_OK != ulResult)
    {
        return AT_ERROR;
    }

    /*APN 轮询开关关闭*/
    if (VOS_TRUE != stLteApnAttachCtrl.ucActiveFlag)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
                                            "%s: %d,0", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                             stLteApnAttachCtrl.ucActiveFlag);
        return AT_OK;
    }

    /*APN 轮询开关开启*/
    ulResult = TAF_PS_GetApnAttachStatusReq(WUEPS_PID_AT,
                    AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId), 0);

    if (VOS_OK == ulResult)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTEAPNATTACH_SWITCH;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;

}

VOS_UINT32 AT_SetLteApnAttachSwitch(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulResult = AT_ERROR;
    AT_MTA_ATTACH_SWITCH_INFO_STRU stSwtichInfo = {0};

    if (1 < gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }


    /* 参数只能为0和1 */
    if (1 < gastAtParaList[0].ulParaValue)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }


    stSwtichInfo.ucFlag = gastAtParaList[0].ulParaValue;

    ulResult = TAF_PS_SetLteApnAttachReq(WUEPS_PID_AT,
                                        AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                        0,
                                        &stSwtichInfo);

    if (VOS_OK == ulResult)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTEAPNATTACH_SWITCH;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

#endif

/*****************************************************************************
  3 函数申明
*****************************************************************************/
extern TAF_UINT32 At_CheckNumString( TAF_UINT8 *pData,TAF_UINT16 usLen );
extern VOS_UINT32 TAF_AGENT_GetSysMode(VOS_UINT16 usClientId,  TAF_AGENT_SYS_MODE_STRU*pstSysMode);
extern VOS_UINT32 atSendDataMsg(VOS_UINT32 TaskId, VOS_UINT32 MsgId, VOS_VOID* pData, VOS_UINT32 uLen);

extern VOS_UINT32 OM_GetSeconds(VOS_VOID);
extern VOS_UINT8 AT_GetSyscfgexModeListItemNum(VOS_VOID);
extern VOS_UINT8* AT_GetSyscfgexModeList(VOS_VOID);
extern VOS_VOID At_AdjustLocalDate(TIME_ZONE_TIME_STRU *pstUinversalTime,
     VOS_INT8 cAdjustValue, TIME_ZONE_TIME_STRU *pstLocalTime);
extern VOS_UINT32  AT_GetSysModeName(MN_PH_SYS_MODE_EX_ENUM_U8 enSysMode, VOS_CHAR *pucSysModeName);
extern VOS_UINT32 AT_PS_ReportDhcp(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportDhcpv6(TAF_UINT8 ucIndex);
extern VOS_UINT32 AT_PS_ReportApraInfo(TAF_UINT8 ucIndex);
extern AT_APP_CONN_STATE_ENUM_U32 AT_AppConvertPdpStateToConnStatus(AT_PDP_STATE_ENUM_U8 enPdpState);
extern VOS_UINT32 TAF_AGENT_FindCidForDial(MN_CLIENT_ID_T usClientId, VOS_UINT8 *pCid);
extern VOS_VOID AT_PS_GenCallDialParam(    AT_DIAL_PARAM_STRU *pstCallDialParam,
    AT_DIAL_PARAM_STRU *pstUsrDialParam,
    VOS_UINT8 ucCid,
    TAF_PDP_TYPE_ENUM_UINT8 enPdpType);
extern AT_PDP_STATE_ENUM_U8 AT_PS_GetCallStateByType(VOS_UINT16 usClientId, VOS_UINT8 ucCallId,
    TAF_PDP_TYPE_ENUM_UINT8 enPdpType);
extern VOS_UINT32  AT_PS_IsUsrDialTypeDualStack(VOS_UINT16 usClientId, VOS_UINT8 ucCallId);
extern AT_PS_USER_INFO_STRU* AT_PS_GetUserInfo(VOS_UINT16 usClientId, VOS_UINT8 ucCallId);
extern VOS_UINT32 TAF_AGENT_GetPdpCidPara(TAF_PDP_PRIM_CONTEXT_STRU *pstPdpPriPara,
    MN_CLIENT_ID_T usClientId, VOS_UINT8 ucCid);
extern AT_PDP_STATUS_ENUM_UINT32 AT_NdisGetConnStatus(AT_PDP_STATE_ENUM_U8 enPdpState);

#if (FEATURE_ON == MBB_FEATURE_MPDP)
extern VOS_UINT8* AT_PutNetworkAddr32(VOS_UINT8 *pucAddr, VOS_UINT32 ulAddr);
extern VOS_UINT32 AT_ActiveUsbNet(VOS_VOID);
VOS_VOID AT_ReadWinblueProfileType(VOS_VOID);
#endif
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
VOS_VOID AT_CalculateModeAntennaLevel(
            VOS_UINT8 Ant_Rssi,
            VOS_UINT8 ucCurRaForSysInfo,
            MN_PH_MODE_ANTLEVEL_ENUM_UINT8 *Cind_Ant);
#endif

#if (FEATURE_ON == MBB_OPERATOR_INDUSTRYCARD)
VOS_UINT32 AT_SetUpdateModePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryUpdateModePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_UpdateCheck( VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryUpdateCheck(VOS_UINT8 ucIndex);
VOS_UINT32 AT_UpdateReq(VOS_UINT8 ucIndex);
#endif

/*****************************************************************************
  4 函数定义
*****************************************************************************/


VOS_UINT16 AT_IsCSIMCustommed(VOS_VOID)
{
    return (gucCsimActiveFlag == VOS_TRUE);
}

VOS_UINT16 AT_IsVodafoneCustommed(VOS_VOID)
{
    return (gucVodafoneCpbs == VOS_TRUE);
}

VOS_VOID AT_SetVodafoneCustomFlag(VOS_UINT16 flag)
{
    gucVodafoneCpbs = flag;
}


NAS_NV_SYSCFGEX_MODE_LIST* AT_GetSyscfgexModeListPtr(VOS_VOID)
{
    return &g_stSyscfgexModeList;
}


VOS_UINT8 AT_GetSyscfgexModeListItemNum(VOS_VOID)
{
    return g_stSyscfgexModeList.ucListItemNum;
}


VOS_UINT8 AT_GetSyscfgexModeRestrictFlag(VOS_VOID)
{
    return g_stSyscfgexModeList.ucRestrict;
}


VOS_UINT8* AT_GetSyscfgexModeList(VOS_VOID)
{
    return &(g_stSyscfgexModeList.aucModeList[0][0]);
}


VOS_UINT32 AT_CheckModeListParaValid( NAS_NV_SYSCFGEX_MODE_LIST *pstSyscfgexModeList )
{
    VOS_UINT8                i;
    VOS_UINT8                ucIndex;
    VOS_UINT8                aucModeListTmp[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
    VOS_UINT8               *paucTemp = VOS_NULL_PTR;
    VOS_UINT32               ulSpecialSetting = VOS_FALSE;

    MBB_MEM_SET(&(aucModeListTmp[0][0]), 0x00, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN);

    /*参数检查*/
    if ( VOS_NULL_PTR == pstSyscfgexModeList )
    {
        return VOS_FALSE;
    }

    /*列表数为0，认为NV未设置*/
    if ( 0 == pstSyscfgexModeList->ucListItemNum )
    {
        return VOS_FALSE;
    }
    MBB_MEM_CPY( aucModeListTmp, pstSyscfgexModeList->aucModeList, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN );

    for ( i = 0; i < pstSyscfgexModeList->ucListItemNum; i++ )
    {
        /*读取一个组合*/
        paucTemp = aucModeListTmp[i];
        if ( SYSCFGEX_MODE_INVALID == *paucTemp )
        {
            return VOS_FALSE;
        }

        ucIndex = 0;
        ulSpecialSetting = VOS_FALSE;
        
        while ( (MODE_LIST_MAX_LEN != ucIndex) && (SYSCFGEX_MODE_INVALID != *paucTemp) )
        {
            /*之前出现过AUTO或者NOCHANGE，直接返回失败*/
            if ( VOS_TRUE == ulSpecialSetting )
            {
                return VOS_FALSE;
            }

            /*存在非法值*/
            if ( (*paucTemp > SYSCFGEX_MODE_RAT_MAX) 
              && (*paucTemp != SYSCFGEX_MODE_NO_CHANGE) )
            {
                return VOS_FALSE;
            }

            /*特殊标志*/
            if ( (SYSCFGEX_MODE_AUTO == *paucTemp) || (SYSCFGEX_MODE_NO_CHANGE == *paucTemp) )
            {
                /*非第一个接入技术，返回失败*/
                ulSpecialSetting = VOS_TRUE;
                if ( 0 != ucIndex )
                {
                    return VOS_FALSE;
                }
            }

            paucTemp++;
            ucIndex++;
        }
    }

    return VOS_TRUE;
}


VOS_VOID AT_ReadSyscfgexBandListFromNV( VOS_VOID )
{
    VOS_UINT32                            ulRet;
    NAS_NV_SYSCFGEX_MODE_LIST             stSyscfgexModeList = {0};

    ulRet = NV_Read( NV_ID_HUAWEI_SYSCFGEX_MODE_LIST, &stSyscfgexModeList,
                     sizeof(NAS_NV_SYSCFGEX_MODE_LIST) );
    if ( NV_OK == ulRet )
    {
        if ( VOS_TRUE == AT_CheckModeListParaValid(&stSyscfgexModeList) )
        {
            MBB_MEM_CPY( AT_GetSyscfgexModeListPtr(), &stSyscfgexModeList,
                        sizeof(NAS_NV_SYSCFGEX_MODE_LIST) );
        }
        else
        {
            /*NV非法，不处理*/
        }
    }
    else
    {
        /*读取NV失败, 什么都不做*/
    }
}

VOS_VOID    AT_ReadCsimCustomizationNV(VOS_VOID)
{
    VOS_UINT32                          ulResult;
    TAF_NV_CSIM_CUSTOMIZED              stCsimCustomized;

    stCsimCustomized.ucNvActiveFlag = 0;

    ulResult = NV_Read(NV_ID_CSIM_CUSTOMIZATION,&stCsimCustomized,sizeof(TAF_NV_CSIM_CUSTOMIZED));

    if((NV_OK == ulResult)
        && (VOS_TRUE == stCsimCustomized.ucNvActiveFlag))
    {
       gucCsimActiveFlag  = VOS_FALSE;
    }
    else
    {
       gucCsimActiveFlag = VOS_TRUE;
    }

    return;
}

/*德国 Vodafone CPBS定制*/

VOS_VOID AT_ReadVodafoneCpbsNV( VOS_VOID )
{
    VOS_UINT32                          ulResult;
    NAS_NV_Vodafone_CPBS                stgucVodafoneCpbs;

    stgucVodafoneCpbs.Vodafone_CPBS = 0;

    ulResult = NV_Read(NV_ID_VODAFONE_CPBS,&stgucVodafoneCpbs,sizeof(NAS_NV_Vodafone_CPBS));

    if((NV_OK == ulResult)
        && (VOS_TRUE == stgucVodafoneCpbs.Vodafone_CPBS))
    {
       AT_SetVodafoneCustomFlag(VOS_TRUE);
    }
    else
    {
       AT_SetVodafoneCustomFlag(VOS_FALSE);
    }

    return;
}

/*********************************************************************************************
 函 数 名  : AT_ReadRfCapabilityNV
 功能描述  : at 初始化时通过读取单板频段NV判断RAT支持情况
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :
 修改历史  :
*********************************************************************************************/
VOS_VOID AT_ReadRfCapabilityNV(VOS_VOID)
{
    AT_NV_WG_RF_MAIN_BAND_STRU       stRfMainBand = {0};
    VOS_UINT8                        *pucAcqOrder = NULL;
    VOS_UINT32                        ulRst = NV_OK;
#if(FEATURE_ON == FEATURE_LTE)
    LRRC_NV_UE_EUTRA_CAP_STRU        *pstEutraCap = NULL;
    pstEutraCap = (LRRC_NV_UE_EUTRA_CAP_STRU*)PS_MEM_ALLOC(WUEPS_PID_AT,
                            sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));

    if (VOS_NULL == pstEutraCap)
    {
        /*NV读取失败需要对全局变量赋值为GUL全部支持*/
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        MBB_MEM_CPY((VOS_CHAR *) g_MbbIsRatSupport.ucGsmSupport, "030201", SYSCFGEX_MAX_RAT_STRNUM - 1);
        MBB_AT_ACORE_COMMON_DEBUG_STR("AT_ReadRfCapabilityNV():malloc for 0xd22c Failed!");
        return;
    }
  

    MBB_MEM_SET(pstEutraCap, 0x00, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));
#endif

    ulRst = NV_Read(en_NV_Item_WG_RF_MAIN_BAND, &stRfMainBand, sizeof(stRfMainBand));

    if (NV_OK != ulRst)
    {
        /*NV读取失败需要对全局变量赋值为GUL全部支持*/
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        MBB_MEM_CPY((VOS_CHAR *) g_MbbIsRatSupport.ucGsmSupport, "030201", SYSCFGEX_MAX_RAT_STRNUM - 1);
        MBB_AT_ACORE_COMMON_DEBUG_STR("Read en_NV_Item_WG_RF_MAIN_BAND Failed!");
        MBB_MEM_FREE(WUEPS_PID_AT, pstEutraCap);
        return;
    }

    ulRst = NVM_Read(EN_NV_ID_UE_CAPABILITY, pstEutraCap, sizeof(LRRC_NV_UE_EUTRA_CAP_STRU));

    if (NV_OK != ulRst)
    {
        /*NV读取失败需要对全局变量赋值为GUL全部支持*/
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        MBB_MEM_CPY((VOS_CHAR *) g_MbbIsRatSupport.ucGsmSupport, "030201", SYSCFGEX_MAX_RAT_STRNUM - 1);
        MBB_AT_ACORE_COMMON_DEBUG_STR("Read EN_NV_ID_UE_CAPABILITY Failed");
        MBB_MEM_FREE(WUEPS_PID_AT, pstEutraCap);
        return;
    }

    pucAcqOrder = g_MbbIsRatSupport.aucAutoAcqorder;

    /*LTE 频段不为空*/
    if(0 != pstEutraCap->stRfPara.usCnt)
    {
        g_MbbIsRatSupport.ucLteSupport = VOS_TRUE;
        MBB_MEM_CPY((VOS_CHAR *) pucAcqOrder, "03", AT_SYSCFGEX_RAT_MODE_STR_LEN);
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
    }

    if(0 != stRfMainBand.unWcdmaBand.ulBand)
    {
        g_MbbIsRatSupport.ucWcdmaSupport = VOS_TRUE;
        MBB_MEM_CPY((VOS_CHAR *) pucAcqOrder, "02", AT_SYSCFGEX_RAT_MODE_STR_LEN);
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
    }

    if(0 != stRfMainBand.unGsmBand.ulBand)
    {
        g_MbbIsRatSupport.ucGsmSupport = VOS_TRUE;
        MBB_MEM_CPY((VOS_CHAR *) pucAcqOrder, "01", AT_SYSCFGEX_RAT_MODE_STR_LEN);
        pucAcqOrder += AT_SYSCFGEX_RAT_MODE_STR_LEN;
    }

    *pucAcqOrder = '\0';

    MBB_MEM_FREE(WUEPS_PID_AT, pstEutraCap);
    return;
}


VOS_VOID AT_ReadNvMbbCustorm(VOS_VOID)
{
#if(FEATURE_ON == MBB_FEATURE_MPDP)
   AT_ReadWinblueProfileType();
#endif/*FEATURE_ON == MBB_FEATURE_MPDP*/
    /*德国 Vodafone CPBS定制*/
    AT_ReadVodafoneCpbsNV();
    /*AT+CSIM定制*/
    AT_ReadCsimCustomizationNV();
    AT_ReadSyscfgexBandListFromNV();
   AT_ReadRfCapabilityNV();
#if (FEATURE_ON == MBB_WPG_HFEATURESTAT)
    AT_ReadTafPdpParaNV();
#endif/*FEATURE_ON == MBB_WPG_HFEATURESTAT*/
}


VOS_VOID At_RunQryParaRspProcCus(TAF_UINT8 ucIndex,TAF_UINT8 OpId, TAF_VOID *pPara, TAF_PARA_TYPE QueryType)
{
    VOS_UINT32 i = 0;
    for (i = 0; i != TAF_TELE_PARA_BUTT; i++ )
    {
        if (QueryType == g_aAtQryTypeProcFuncTblMbb[i].QueryType)
        {
            g_aAtQryTypeProcFuncTblMbb[i].AtQryParaProcFunc(ucIndex,OpId,pPara);
            return;
        }
    }
}


VOS_UINT32 AT_GenerateModeListFromNV( VOS_CHAR *pachDest )
{
    VOS_UINT8                i;
    VOS_UINT8                ucIndex;
    VOS_UINT8                aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN];
    VOS_UINT8               *paucTemp = VOS_NULL_PTR;           
    VOS_UINT32               ulLength = 0;
    VOS_UINT8                ucListItemNum = AT_GetSyscfgexModeListItemNum();
    VOS_UINT8               *paucModeList = AT_GetSyscfgexModeList();
    MBB_MEM_SET(aucModeList, 0x00, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN);
    MBB_MEM_CPY( aucModeList, paucModeList, MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN);

    /*当前NV无效，走默认分支*/
    if ( 0 == ucListItemNum )
    {
        return VOS_FALSE;
    }

    /*添加括号*/
    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest, "%s", "(" );

    for ( i = 0; i < ucListItemNum; i++ )
    {
        paucTemp = aucModeList[i];
        if ( SYSCFGEX_MODE_INVALID == *paucTemp )
        {
            continue;
        }

        ucIndex = 0;

        /*每个组合前添加引号*/
        ulLength += (VOS_UINT32)At_sprintf( DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "%s", "\"" );
        while ( (MODE_LIST_MAX_LEN != ucIndex) && (SYSCFGEX_MODE_INVALID != *paucTemp) )
        {
            switch ( *paucTemp )
            {   
                /*针对每个接入技术，增加对应的字符串*/
                case SYSCFGEX_MODE_AUTO:
                    ulLength += (VOS_UINT32)At_sprintf( DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "%s", "00" );
                    break;
                case SYSCFGEX_MODE_GSM:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "%s", "01" );
                    break;
                case SYSCFGEX_MODE_WCDMA:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "02" );
                    break;
                case SYSCFGEX_MODE_LTE:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "03" );
                    break;
                case SYSCFGEX_MODE_CDMA:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "04" );
                    break;
                case SYSCFGEX_MODE_TDSCDMA:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "05" );
                    break;
                case SYSCFGEX_MODE_WIMAX:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "06" );
                    break;
                case SYSCFGEX_MODE_NOT_CHANGE:
                    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "99" );
                    break;
                default:
                    return VOS_FALSE;       /*存在非法值，直接返回错误*/
            }

            paucTemp++;
            ucIndex++;
        }
        /*组合之后添加引号*/
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,  pachDest + ulLength, "%s", "\"" );

        if ( i != (ucListItemNum - 1) )
        {
            /*每个组合结束后，添加逗号*/
            ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest,   pachDest + ulLength, "%s", "," );
        }
    }
    /*添加右括号*/
    (VOS_VOID)At_sprintf(DEST_STRING_MAX_LEN, pachDest,   pachDest + ulLength, "%s", ")" );

    return VOS_TRUE;
}
VOS_VOID At_Pb_VodafoneCPBSCus(TAF_UINT16* usLength, TAF_UINT8 ucIndex)
{
    /*德国 Vodafone CPBS定制*/
    if(AT_IsVodafoneCustommed())
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                                          "%s: (\"SM\",\"EN\",\"ON\")",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    else
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                                          "%s: (\"SM\",\"EN\",\"ON\",\"FD\")",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
}


VOS_VOID AT_RssiConvert(VOS_INT32      lRssiValue, VOS_UINT8    *pucRssiLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= lRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_RSSI_VALUE_MAX <= lRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_RSSI_LEVEL_MAX;
    }
    else if (AT_HCSQ_RSSI_VALUE_MIN > lRssiValue)
    {
        *pucRssiLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucRssiLevel = (VOS_UINT8)((lRssiValue - AT_HCSQ_RSSI_VALUE_MIN) + 1);
    }

    return;
}


VOS_VOID AT_RscpConvert(VOS_INT16      lRscpValue, VOS_UINT8    *pucRscpLevel )
{
    if(AT_HCSQ_VALUE_INVALID <= lRscpValue)
    {
        *pucRscpLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_RSCP_VALUE_MAX <= lRscpValue)
    {
        *pucRscpLevel = AT_HCSQ_RSCP_LEVEL_MAX;
    }
    else if (AT_HCSQ_RSCP_VALUE_MIN > lRscpValue)
    {
        *pucRscpLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucRscpLevel = (VOS_UINT8)((lRscpValue - AT_HCSQ_RSCP_VALUE_MIN) + 1);
    }

    return;
}


VOS_VOID AT_EcioConvert(VOS_INT16      lEcioValue, VOS_UINT8    *pucEcioLevel )
{
    if (AT_HCSQ_VALUE_INVALID <= lEcioValue)
    {
        *pucEcioLevel = AT_HCSQ_VALUE_INVALID;
    }
    else if (AT_HCSQ_ECIO_VALUE_MAX <= lEcioValue)
    {
        *pucEcioLevel = AT_HCSQ_ECIO_LEVEL_MAX;
    }
    else if (AT_HCSQ_ECIO_VALUE_MIN > lEcioValue)
    {
        *pucEcioLevel = AT_HCSQ_LEVEL_MIN;
    }
    else
    {
        *pucEcioLevel = (VOS_UINT8)(((lEcioValue - AT_HCSQ_ECIO_VALUE_MIN) * 2) + 1);/*calu ecio level*/
    }

    return;
}

VOS_VOID AT_QryParaRspHcsqProc( 
    VOS_UINT8         ucIndex, 
    VOS_UINT8         ucOpId, 
    VOS_VOID          *pPara)
{
    VOS_UINT32                          ulResult = (VOS_UINT32)AT_OK;
    VOS_UINT16                          usLength = 0;
    TAF_PH_HCSQ_STRU                    strHcsq = {0};
    VOS_CHAR                            aucSysModeName[AT_HCSQ_RAT_NAME_MAX];
    VOS_UINT8                           rssi = 0;
    VOS_UINT8                           rscp = 0;
    VOS_UINT8                           ecio = 0;
    /*判断输入指针合法性*/
    if(NULL == pPara)
    {
        ulResult = (VOS_UINT32)AT_ERROR;
        At_FormatResultData(ucIndex, ulResult);/*lint !e64*/
        return;
    }
    MBB_MEM_CPY(&strHcsq, pPara, sizeof(TAF_PH_HCSQ_STRU));
    MBB_MEM_SET(aucSysModeName, 0x00, sizeof(aucSysModeName));
    /*根据系统模式获取当前模式的名字*/
    (VOS_VOID)AT_GetSysModeName(strHcsq.ucSysMode, aucSysModeName);
    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s:",
                                          (VOS_INT8*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    if(MN_PH_SYS_MODE_EX_NONE_RAT == strHcsq.ucSysMode)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "\"NOSERVICE\"");   
    }
    else
    {    
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "\"%s\"",aucSysModeName);
    }
    
    AT_RssiConvert(strHcsq.ucRssiValue, &rssi);
    switch(strHcsq.ucSysMode)
    {            
        case MN_PH_SYS_MODE_EX_GSM_RAT:
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  ",%d", rssi);
            break;
        }
        case MN_PH_SYS_MODE_EX_WCDMA_RAT:
        {
            /*调用转换函数对RSCP按规则进行转换*/
            AT_RscpConvert(strHcsq.ucRSCP, &rscp);
            /*调用转换函数对ECIO按规则进行转换*/
            AT_EcioConvert(strHcsq.ucEcio, &ecio);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  ",%d,%d,%d",
                                                  rssi, rscp, ecio);
            break;
        }
        case MN_PH_SYS_MODE_EX_TDCDMA_RAT:
        {
            /*调用转换函数对RSCP按规则进行转换*/
            AT_RscpConvert(strHcsq.ucRSCP, &rscp);
            /*调用转换函数对ECIO按规则进行转换*/
            AT_EcioConvert(strHcsq.ucEcio, &ecio);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                  ",%d,%d,%d",
                                                  rscp, rscp, 255);/*invalid value 255*/
            break;
        }
        default:
            break;
    }

    gstAtSendData.usBufLen = usLength;
    /* 回复用户命令结果 */
    At_FormatResultData(ucIndex,ulResult);/*lint !e64*/
    return;
}
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)

VOS_VOID AT_CalculateModeAntennaLevel(
    VOS_UINT8 Ant_Rssi,
    VOS_UINT8 ucCurRaForSysInfo,
    MN_PH_MODE_ANTLEVEL_ENUM_UINT8 *Cind_Ant
    )
{
    VOS_UINT32 ulAbsRssiValue;
    VOS_UINT32 aulGsmRssi[MN_PH_MODE_ANTLEVEL_BUTT] = {18, 21, 23, 26, 96}; /*2G天线计算门限*/
    VOS_UINT32 aulWcdmaRscp[MN_PH_MODE_ANTLEVEL_BUTT] = {9, 11, 13, 19, 96}; /*3G天线计算门限*/
    VOS_UINT32 *pulRssiConvertRule;
    ulAbsRssiValue = Ant_Rssi;
    if (MN_PH_SYS_MODE_EX_GSM_RAT == ucCurRaForSysInfo)
    {
        pulRssiConvertRule = aulGsmRssi;
    }
    else
    {
        pulRssiConvertRule = aulWcdmaRscp;
    }
    /* 根据获取的rssi对应信号水平*/
    if (ulAbsRssiValue <= pulRssiConvertRule[MN_PH_MODE_ANTLEVEL_0])
    {
        *Cind_Ant = AT_CMD_ANTENNA_LEVEL_0;
    }
    else if (ulAbsRssiValue <= pulRssiConvertRule[MN_PH_MODE_ANTLEVEL_1])
    {
        *Cind_Ant = AT_CMD_ANTENNA_LEVEL_1;
    }
    else if (ulAbsRssiValue <= pulRssiConvertRule[MN_PH_MODE_ANTLEVEL_2])
    {
        *Cind_Ant = AT_CMD_ANTENNA_LEVEL_2;
    }
    else if (ulAbsRssiValue <= pulRssiConvertRule[MN_PH_MODE_ANTLEVEL_3])
    {
        *Cind_Ant = AT_CMD_ANTENNA_LEVEL_3;
    }
    else if (ulAbsRssiValue < pulRssiConvertRule[MN_PH_MODE_ANTLEVEL_4])
    {
        *Cind_Ant = AT_CMD_ANTENNA_LEVEL_4;
    }
    else
    {
        *Cind_Ant = AT_CMD_ANTENNA_LEVEL_0;
    }
    return;
}

VOS_VOID  AT_QryParaRspCindProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucOpId,
    VOS_VOID                            *pPara
)
{
    VOS_UINT32                          ulResult = (VOS_UINT32)AT_OK;
    VOS_UINT16                          usLength = 0;
    TAF_PH_CIND_STRU                    strCind = {0};
    VOS_CHAR                            aucSysModeName[AT_HCSQ_RAT_NAME_MAX];
    VOS_UINT8                           rssi = 0;
    VOS_UINT8                           rscp = 0;
    VOS_UINT8                           ecio = 0;
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     ant_level = AT_CMD_ANTENNA_LEVEL_0;
    /*判断输入指针合法性*/
    if(NULL == pPara)
    {
        ulResult = (VOS_UINT32)AT_ERROR;
        At_FormatResultData(ucIndex, ulResult);
        return;
    }
    MBB_MEM_CPY(&strCind, pPara, sizeof(TAF_PH_CIND_STRU));
    MBB_MEM_SET(aucSysModeName, 0x00, sizeof(aucSysModeName));
    if (TAF_SDC_REPORT_SRVSTA_NORMAL_SERVICE == strCind.ucSrvStatus)
    {
        strCind.ucSrvStatus = TAF_CIND_SRVSTA_NORMAL_SERVICE;
    }
    else
    {
        strCind.ucSrvStatus = TAF_CIND_SRVSTA_NO_SERVICE;
    }
    (VOS_VOID)AT_GetSysModeName(strCind.ucSysMode,aucSysModeName);
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d,",
                                          (VOS_INT8*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          CIND_RESERVED);
    
    switch(strCind.ucSysMode)
    {
        case MN_PH_SYS_MODE_EX_GSM_RAT:
        {
            AT_RssiConvert(strCind.ucRssiValue, &rssi);
            AT_CalculateModeAntennaLevel(rssi,strCind.ucSysMode,&ant_level);
            break;
        }
        case MN_PH_SYS_MODE_EX_WCDMA_RAT:
        case MN_PH_SYS_MODE_EX_TDCDMA_RAT:    
        {
            /*调用转换函数对RSCP按规则进行转换*/
            AT_RscpConvert(strCind.ucRSCP,&rscp);
            AT_CalculateModeAntennaLevel(rscp,strCind.ucSysMode,&ant_level);
            break;
        }
        default:
        {
            strCind.ucSrvStatus = TAF_CIND_SRVSTA_NO_SERVICE;
            break;
        }
    }
    strCind.ucAntLevel = ant_level;
    if (TAF_CIND_SRVSTA_NO_SERVICE == strCind.ucSrvStatus)
    {
        strCind.ucAntLevel = AT_CMD_ANTENNA_LEVEL_0;
    }
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                "%d,%d,%d,%d,%d,%d,%d",
                                strCind.ucAntLevel,
                                strCind.ucSrvStatus,
                                CIND_RESERVED,
                                CIND_RESERVED,
                                CIND_RESERVED,
                                CIND_RESERVED,
                                CIND_RESERVED);

    gstAtSendData.usBufLen = usLength;
    /* 回复用户命令结果 */
    At_FormatResultData(ucIndex,ulResult);
    return;
}

VOS_VOID  AT_QryParaAntProc(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           ucOpId,
    VOS_VOID                            *pPara
)
{
    VOS_UINT32                          ulResult = (VOS_UINT32)AT_OK;
    VOS_UINT16                          usLength = 0;
    TAF_PH_ANT_STRU                     strAnt = {0};
    VOS_CHAR                            aucSysModeName[AT_HCSQ_RAT_NAME_MAX];
    VOS_UINT8                           rssi = 0;
    VOS_UINT8                           rscp = 0;
    VOS_UINT8                           ecio = 0;
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8  ant_level = AT_CMD_ANTENNA_LEVEL_0;
    /*判断输入指针合法性*/
    if(NULL == pPara)
    {
        ulResult = (VOS_UINT32)AT_ERROR;
        At_FormatResultData(ucIndex, ulResult);/*lint !e64*/
        return;
    }
    MBB_MEM_CPY(&strAnt, pPara, sizeof(TAF_PH_ANT_STRU));
    MBB_MEM_SET(aucSysModeName, 0x00, sizeof(aucSysModeName));
    /*根据系统模式获取当前模式的名字*/
    (VOS_VOID)AT_GetSysModeName(strAnt.ucSysMode, aucSysModeName);
    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s:",
                                       (VOS_INT8 *)g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    if((MN_PH_SYS_MODE_EX_NONE_RAT == strAnt.ucSysMode)||(TAF_SDC_REPORT_SRVSTA_NORMAL_SERVICE != strAnt.ucSrvStatus))
    {
        if(AT_ANT_TEMPPRTEVENT_H == g_TempprtEvent_flag)/*高温无服务 ant_leve=99*/
        {
            ant_level = AT_ANT_TEMPPRT_LEVEL;
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%d",
                                          ant_level);
        gstAtSendData.usBufLen = usLength;
    /* 回复用户命令结果 */
        At_FormatResultData(ucIndex,ulResult);/*lint !e64*/
        return;
    }
    else
    {
        switch(strAnt.ucSysMode)
        {            
            case MN_PH_SYS_MODE_EX_GSM_RAT:
            {
                AT_RssiConvert(strAnt.ucRssiValue, &rssi);
                AT_CalculateModeAntennaLevel(rssi,strAnt.ucSysMode,&ant_level);   
                break;
            }
            case MN_PH_SYS_MODE_EX_WCDMA_RAT:
            case MN_PH_SYS_MODE_EX_TDCDMA_RAT:
            {
                /*调用转换函数对RSCP按规则进行转换*/
                AT_RscpConvert(strAnt.ucRSCP, &rscp);
                AT_CalculateModeAntennaLevel(rscp,strAnt.ucSysMode,&ant_level);
                break;
            }
            default:
                break;
        }
    }
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%d", 
                                       ant_level);
    gstAtSendData.usBufLen = usLength;
    /* 回复用户命令结果 */
    At_FormatResultData(ucIndex,ulResult);/*lint !e64*/
    return;
}
#endif

TAF_UINT16 At_TafCallBackMakeData( TAF_UINT8 ucIndex ,TAF_PH_PLMN_NAME_STRU *PlmnListInfo ,VOS_UINT16 usLength)
{
    /*将AT+COPN组装进入传输字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s:", g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /*将MCC组装进入传输字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%x%x%x",
                                            (0x0f00 & PlmnListInfo->PlmnId.Mcc) >> 8,
                                            (0x00f0 & PlmnListInfo->PlmnId.Mcc) >> 4,
                                            (0x000f & PlmnListInfo->PlmnId.Mcc));

    /*判断PLMN ID 个数，确认是否是6位MNC，
    对于第六位取出组装入字符串*/
    if(MMA_PLMN_ID_LEN == PlmnListInfo->PlmnLen)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%x",
                                                (0x0f00 & PlmnListInfo->PlmnId.Mnc) >> 8);
    }

    /*将MNC剩余几位取出组装入字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%x%x",
                                            (0x00f0 & PlmnListInfo->PlmnId.Mnc) >> 4,
                                            (0x000f & PlmnListInfo->PlmnId.Mnc));

     /*将网络长名组装入字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                        (TAF_CHAR *)pgucAtSndCodeAddr,
                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%s",
                                        PlmnListInfo->aucOperatorNameLong);

    /*将回车换行组装入字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s",gaucAtCrLf);
    
    return usLength;
}

TAF_UINT16 At_TafCallBackMakeEons(TAF_UINT8 ucIndex ,TAF_PH_EONS_INFO_STRU *pstEonsInfo)
{
    VOS_UINT16 usLength = 0;
    /*添加接收TYPE值，接收C核传入值进行打印显示*/
    VOS_UINT16 usType = 0;

    usType = pstEonsInfo->stPlmnInfo.ucEonsType;
    
    /*将^EONS:组装进入传输字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s:%d", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,usType);

    if(TAF_REPORT_SRVSTA_NORMAL_SERVICE == pstEonsInfo->stPlmnInfo.ucServiceStatus || 0 != pstEonsInfo->stPlmnInfo.PlmnLen)
    {
        /*将MCC组装进入传输字符串*/
        pstEonsInfo->stPlmnInfo.PlmnId.Mcc &= 0x0FFF;/*MCC取三位*/
        if(AT_PLMN_ID_MAX_LEN == pstEonsInfo->stPlmnInfo.PlmnLen)
        {
            pstEonsInfo->stPlmnInfo.PlmnId.Mnc &= 0x0FFF;/*MNC取三位*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                                (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%03X%03X",
                                                pstEonsInfo->stPlmnInfo.PlmnId.Mcc,
                                                pstEonsInfo->stPlmnInfo.PlmnId.Mnc);
        }
        else
        {
            pstEonsInfo->stPlmnInfo.PlmnId.Mnc &= 0x00FF;/*MNC取两位位*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                                 (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%03X%02X",
                                                 pstEonsInfo->stPlmnInfo.PlmnId.Mcc,
                                                 pstEonsInfo->stPlmnInfo.PlmnId.Mnc);
        }
        
        /* 长名输出 */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"");
        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                      (TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_UINT8 *)(pgucAtSndCodeAddr + usLength),
                                      pstEonsInfo->stNWName.stLNameInfo.aucName, 
                                      pstEonsInfo->stNWName.stLNameInfo.ucNameLen);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                      (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");

        /* 短名输出 */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                     (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"");
                                     
        usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                     (TAF_CHAR *)pgucAtSndCodeAddr,
                                                     (TAF_UINT8 *)(pgucAtSndCodeAddr + usLength),
                                                     pstEonsInfo->stNWName.stSNameInfo.aucName, 
                                                     pstEonsInfo->stNWName.stSNameInfo.ucNameLen);
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                                     (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");

        if( 0 < pstEonsInfo->stSpnInfo.ucSpnLen)          
        {
            /*将SPN 显示条件标志位组装入字符串*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                                  (TAF_CHAR *)pgucAtSndCodeAddr,
                                                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",
                                                                  pstEonsInfo->stSpnInfo.ucDispMode);

            /*将SPN 信息组装入字符串*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength,",\"");
            usLength += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                                                         (TAF_CHAR *)pgucAtSndCodeAddr,
                                                         (TAF_UINT8 *)(pgucAtSndCodeAddr + usLength),
                                                         pstEonsInfo->stSpnInfo.aucSpn,
                                                         pstEonsInfo->stSpnInfo.ucSpnLen);
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                                        (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"\"");
        }
    }


    /*将回车换行组装入字符串*/
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s",gaucAtCrLf);
    
    return usLength;
}


TAF_VOID At_Ascii2Unicode( TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen,  TAF_UINT8 *pusdest, TAF_UINT16 *pusDstLen)
{
    TAF_UINT16 usSrcOffSet = 0;
    TAF_UINT16 usDstOffSet = 0;

    if( (VOS_NULL_PTR == pucSrc) || (VOS_NULL_PTR == pusdest) || (VOS_NULL_PTR == pusDstLen) || ( 1 > usSrcLen ))
    {
        MBB_AT_ACORE_COMMON_DEBUG_STR("paramater error!");
        return;
    }

    while( ('\0' != *pucSrc) && ( usSrcLen > usSrcOffSet ))
    {
        *(pusdest + usDstOffSet) = 0x00;

        usDstOffSet++;
        *(pusdest + usDstOffSet) = (*(pucSrc + usSrcOffSet));
         
        usDstOffSet++;
        usSrcOffSet++;
    }

    *pusDstLen = usDstOffSet;
}


VOS_UINT32  AT_UnPack7Bit(
    const VOS_UINT8                     *pucOrgChar,
    VOS_UINT32                          ulLen,
    VOS_UINT8                           ucFillBit,
    VOS_UINT8                           *pucUnPackedChar
)
{
    /*存放字节地址*/
    VOS_UINT32                          ulPos = 0;
     /*存放位偏移*/
    VOS_UINT32                          ulOffset;
    VOS_UINT32                          ulLoop;

    if ((TAF_NULL_PTR == pucOrgChar)
     || (TAF_NULL_PTR == pucUnPackedChar))
    {
        AT_WARN_LOG("AT_UnPack7Bit ERROR: bad parameter ");
        return AT_FAILURE;
    }

    /*根据协议23040 9.2.3.24 UDHL和UDH后面是Fill Bits和SM，去掉Fill Bits后就是SM(Unit: Septet),可以获得SM中包含字符个数*/
    ulOffset = ucFillBit % 8;

    /*第一步，移出当前无效的偏移位ulOffset，得到字符的低(8 - ulOffset)位，
      第二步，若(8 - ulOffset)小于7位，需要从下一个OCTET中获取高(7 - (8 - ulOffset))位
      第三步，获取下一个数据源的下标(ulPos)和需要去除的数据位(偏移位ulOffset)*/
    for (ulLoop = 0; ulLoop < ulLen; ulLoop++)
    {
        pucUnPackedChar[ulLoop] = (VOS_UINT8)(pucOrgChar[ulPos] >> ulOffset);
        if (ulOffset > 1)
        {
            pucUnPackedChar[ulLoop] |= (VOS_UINT8)((pucOrgChar[ulPos + 1] << (8 - ulOffset)) & AT_MSG_7BIT_MASK);
        }
        else
        {
            pucUnPackedChar[ulLoop] &= AT_MSG_7BIT_MASK;
        }

        ulPos   += (ulOffset + 7) / 8;
        ulOffset = (ulOffset + 7) % 8;
    }

    return AT_SUCCESS;
}



TAF_UINT16 AT_EONSName2Unicode(TAF_UINT8 *pucSrc, TAF_UINT16 usSrcLen, TAF_UINT8 ucCode, TAF_UINT8 ucSrcMaxLen)
{
   TAF_UINT8     aucTmpSrc[TAF_PH_OPER_PNN_USC2_CODE_LEN + 1] = {0};
   TAF_UINT16    usTmplen = 0;

    /* 传入参数检查 */
    if( (VOS_NULL_PTR == pucSrc) || ( 1 > usSrcLen))
    {
        AT_WARN_LOG("At_TafEONSInfoFormat: Parameter is error.");
        return 0;
    }

    /* 把不同的编码转换为unicode编码 */
    switch(ucCode)
    {
        case TAF_PH_EONS_GSM_7BIT_DEFAULT:
            /* 转码后不能超过最大长度 */
            if ((usSrcLen * 4) > ucSrcMaxLen)
            {
                usSrcLen = ucSrcMaxLen / 4;
            }

            At_PbGsmToUnicode(pucSrc,usSrcLen,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_UCS2_80:
            At_PbUnicode80FormatPrint(pucSrc,usSrcLen,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_UCS2_81:
            At_PbUnicode81FormatPrint(pucSrc,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_UCS2_82:
            At_PbUnicode82FormatPrint(pucSrc,&aucTmpSrc[0],&usTmplen);
            break;

        case TAF_PH_EONS_ASCII:
            /* 转码后不能超过最大长度 */
            if ((usSrcLen * 4) > ucSrcMaxLen)
            {
                usSrcLen = ucSrcMaxLen / 4;
            }

            At_Ascii2Unicode(pucSrc, usSrcLen, &aucTmpSrc[0], &usTmplen);
            break;

        default:
            break;
    }

    /* 转码后的数据保存 */
    if( 0 < usTmplen )
    {
        MBB_MEM_SET(pucSrc, 0, ucSrcMaxLen);
        MBB_MEM_CPY((pucSrc), aucTmpSrc, usTmplen);
        usSrcLen = usTmplen;
    }

    return usSrcLen;
}



TAF_VOID At_TafEONSInfoFormat(TAF_PH_EONS_INFO_STRU *pstEonsInfo)
{
    TAF_UINT8 ucStrLen = 0;
    TAF_UINT8 ausUnPack[TAF_PH_OPER_PNN_USC2_CODE_LEN] = {0};

    /* 参数检查 */
    if( VOS_NULL_PTR == pstEonsInfo )
    {
        AT_WARN_LOG("At_TafEONSInfoFormat: Parameter is null.");
        return;
    }

    /* 如果长名的编码方式为7bit，则先转换为8bit存储，然后在转换为uincode */
    if( TAF_PH_EONS_GSM_7BIT_PACK == pstEonsInfo->stNWName.stLNameInfo.ucNamecode)
    {
        /* 7bit编码方式的字符转换为8bit字符 */
        ucStrLen = (VOS_UINT8)((pstEonsInfo->stNWName.stLNameInfo.ucNameLen * 8) / 7);
         (VOS_VOID)AT_UnPack7Bit( pstEonsInfo->stNWName.stLNameInfo.aucName, ucStrLen, 0, &ausUnPack[0]);

        /* 对于长度超过32个字节的只输出前32个字节 */
        if( TAF_PH_OPER_NETWORTNAME_LEN < ucStrLen )
        {
            ucStrLen = TAF_PH_OPER_NETWORTNAME_LEN;
        }

        /* 保存转换后的字符和信息 */
        MBB_MEM_SET(&pstEonsInfo->stNWName.stLNameInfo, 0, sizeof(TAF_PH_NWNAME_INFO_STRU));
            pstEonsInfo->stNWName.stLNameInfo.ucNamecode = TAF_PH_EONS_GSM_7BIT_DEFAULT;
        MBB_MEM_CPY(pstEonsInfo->stNWName.stLNameInfo.aucName, ausUnPack, ucStrLen );
            pstEonsInfo->stNWName.stLNameInfo.ucNameLen = ucStrLen;
    }

    /* 转换长名为uincode */
    ucStrLen = (VOS_UINT8)AT_EONSName2Unicode(pstEonsInfo->stNWName.stLNameInfo.aucName,
                                                    pstEonsInfo->stNWName.stLNameInfo.ucNameLen,
                                                    pstEonsInfo->stNWName.stLNameInfo.ucNamecode,
                                                    TAF_PH_OPER_PNN_USC2_CODE_LEN);
    pstEonsInfo->stNWName.stLNameInfo.ucNameLen = ucStrLen;

    /* 如果短名的编码方式为7bit，则先转换为8bit存储，然后在转换为uincode */
    if( TAF_PH_EONS_GSM_7BIT_PACK == pstEonsInfo->stNWName.stSNameInfo.ucNamecode)
    {
        /* 7bit编码方式的字符转换为8bit字符 */
        ucStrLen = (VOS_UINT8)((pstEonsInfo->stNWName.stSNameInfo.ucNameLen * 8) / 7);
         (VOS_VOID)AT_UnPack7Bit( pstEonsInfo->stNWName.stSNameInfo.aucName, ucStrLen, 0, &ausUnPack[0]);

        /* 对于长度超过32个字节的只输出前32个字节 */
        if( TAF_PH_OPER_NETWORTNAME_LEN < ucStrLen )
        {
            ucStrLen = TAF_PH_OPER_NETWORTNAME_LEN;
        }

        /* 保存转换后的字符和信息 */
        MBB_MEM_SET(&pstEonsInfo->stNWName.stSNameInfo, 0, sizeof(TAF_PH_NWNAME_INFO_STRU));
            pstEonsInfo->stNWName.stSNameInfo.ucNamecode = TAF_PH_EONS_GSM_7BIT_DEFAULT;
        MBB_MEM_CPY(pstEonsInfo->stNWName.stSNameInfo.aucName, ausUnPack, ucStrLen );
            pstEonsInfo->stNWName.stSNameInfo.ucNameLen = (VOS_UINT8)ucStrLen;
    }

    /* 转换短名为uincode */
    ucStrLen = (VOS_UINT8)AT_EONSName2Unicode(pstEonsInfo->stNWName.stSNameInfo.aucName,
                                                    pstEonsInfo->stNWName.stSNameInfo.ucNameLen,
                                                    pstEonsInfo->stNWName.stSNameInfo.ucNamecode,
                                                    TAF_PH_OPER_PNN_USC2_CODE_LEN);
    pstEonsInfo->stNWName.stSNameInfo.ucNameLen = ucStrLen;

    /* 转换SPN为uincode */
    ucStrLen = (VOS_UINT8)AT_EONSName2Unicode(pstEonsInfo->stSpnInfo.aucSpn,
                                                    pstEonsInfo->stSpnInfo.ucSpnLen,
                                                    pstEonsInfo->stSpnInfo.ucSPNCode,
                                                    TAF_PH_OPER_SPN_USC2_CODE_LEN);
    pstEonsInfo->stSpnInfo.ucSpnLen = ucStrLen;
}


TAF_UINT16     At_TafCallBackNWNameProc(TAF_UINT8* pData)
{
    TAF_PH_EONS_INFO_RSP_STRU*        pstEvent = (TAF_PH_EONS_INFO_RSP_STRU*)pData;
    VOS_UINT8                         ucIndex = 0;
    VOS_UINT16                        usLength = 0;

    /*获取用户索引*/
    if(AT_FAILURE == At_ClientIdToUserId(pstEvent->ClientId,&ucIndex))
    {
        AT_WARN_LOG("At_TafCallBackNWNameProc: ClientId error.");
        return usLength;
    }

    /*检查是否是广播状态*/   
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("At_TafCallBackNWNameProc : AT_BROADCAST_INDEX.");
        return usLength;
    }

    /* 对获取到得数据进行格式化 */
    At_TafEONSInfoFormat(&pstEvent->stEonsInfo);

    /* 返回EONS信息 */
    usLength = At_TafCallBackMakeEons(ucIndex, &(pstEvent->stEonsInfo));

    return usLength;
}


VOS_UINT32 AT_GetTimeInfoDebugFlag(VOS_VOID)
{
    return g_TimeInfoDebug;
}

VOS_VOID AT_SetTimeInfoDebugFlag(VOS_UINT32 flag)
{
    g_TimeInfoDebug = flag;
}

TAF_UINT32 At_GetTimeZonebyPlmn(VOS_UINT16 PlmnMcc, VOS_INT8 *MccZoneInfo)
{
    TAF_UINT32  usTotalMccNum;
    TAF_UINT32  i;
    TAF_UINT32    Ret = VOS_FALSE;

    usTotalMccNum = sizeof(g_mcc_zone_infoTbl) / sizeof(g_mcc_zone_infoTbl[0]);

    for (i = 0; i < usTotalMccNum; i++)
    {
        if (PlmnMcc == g_mcc_zone_infoTbl[i].MCC)
        {
            *MccZoneInfo = g_mcc_zone_infoTbl[i].Zone;
            
            Ret = VOS_TRUE;
            break;
        }
    }
    
    return Ret;
}


TAF_VOID At_QryMmPlmnIdRspProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT32 ulResult = AT_FAILURE;
    TAF_UINT16 usLength = 0;
    TAF_UINT16 Mcc = 0;
    TAF_PLMN_ID_STRU     *pstPlmnId;
    VOS_INT8 tz = 0;
    NAS_MM_INFO_IND_STRU    stInvalidAtTimeInfo = {
        .ucIeFlg = NAS_MM_INFO_IE_UTLTZ,
        .cLocalTimeZone = 0,
        .ucDST = 0,
        .ulTimeSeconds = 0,  /*no do slice time*/
        .stUniversalTimeandLocalTimeZone = {
                .ucYear          = 90,  /*1990 year*/
                .ucMonth        = 1,    /*TIME 90/01/06 08:00+00,00*/
                .ucDay           = 6,    /*TIME 90/01/06 08:00+00,00*/
                .ucHour          = 8,    /*TIME 90/01/06 08:00+00,00*/
                .ucMinute       = 0,
                .ucSecond      = 0,
                .cTimeZone    = 0,
                .Reserved     = 0,
        },
    };
    pstPlmnId = (TAF_PLMN_ID_STRU *)pPara;
    Mcc = ((pstPlmnId->Mcc & 0x0f) << 8 ) | 
                ((pstPlmnId->Mcc & 0x0f00) >> 4 ) |
                ((pstPlmnId->Mcc & 0x0f0000) >> 16 );/*Mnc*/

    MBB_AT_ACORE_COMMON_DEBUG("pstPlmnId->Mcc", pstPlmnId->Mcc);
    MBB_AT_ACORE_COMMON_DEBUG("old Mcc", Mcc);

    /*  ^NWTIME:<time><tz>,<DST>     */
    if ( (TAF_UINT32)VOS_TRUE == At_GetTimeZonebyPlmn( Mcc, &tz ) )
    {
        stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone = tz;
        stInvalidAtTimeInfo.cLocalTimeZone = stInvalidAtTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone;
        usLength = At_PrintNwTimeInfo( &stInvalidAtTimeInfo, usLength, 
                        (VOS_CHAR*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName, ":", NULL, TIME_FORMAT_QRY_NWTIME);

        ulResult = AT_OK; 
    }
    else
    {
        ulResult = AT_ERROR;
    }
    
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}


VOS_UINT32 At_GetLiveTime( NAS_MM_INFO_IND_STRU *pstATtime, 
            NAS_MM_INFO_IND_STRU *pstNewTime )
{
    TIME_ZONE_TIME_STRU stNewTime;
    TAF_UINT32 SecondNow = 0;
    TAF_UINT32 Seconds = 0;
    TAF_UINT32 adjustDate = 0;
    TAF_UINT32 i = 0;


    if ( ( NULL == pstATtime ) || ( NULL == pstNewTime) )
    {
        return 0;
    }

    MBB_MEM_SET(&stNewTime, 0x00, sizeof(stNewTime));

    if (NAS_MM_INFO_IE_UTLTZ == (pstATtime->ucIeFlg & NAS_MM_INFO_IE_UTLTZ))/*全局变量中保存有时间信息 */
    {
        SecondNow = OM_GetSeconds();/*lint !e128 !e58 slice to sec*/
        AT_EVENT_REPORT_LOG_1("SecondNow", SecondNow);
        AT_EVENT_REPORT_LOG_1("pstATtime->ulTimeSeconds", pstATtime->ulTimeSeconds);
        if (VOS_TRUE == AT_GetTimeInfoDebugFlag())
        {
            SecondNow *= TIME_INFO_DEBUG_VAR;
            AT_EVENT_REPORT_LOG_1("SecondNow", SecondNow);
        }
        /*如果ulTimeSeconds=0表示没有收到网络侧下发的mm信息所以不进行计数*/
        if (0 == pstATtime->ulTimeSeconds || SecondNow < pstATtime->ulTimeSeconds)
        {
            Seconds = 0;
        }
        else
        {
            Seconds = SecondNow - pstATtime->ulTimeSeconds;
        }

        MBB_MEM_CPY(pstNewTime, pstATtime, sizeof(NAS_MM_INFO_IND_STRU) );
        /*pstATtime->ulTimeSeconds == 0 标明当前未获取到网络侧的时间下发信息*/
        if ( 0 != pstATtime->ulTimeSeconds )
        {
            pstNewTime->ulTimeSeconds = SecondNow; /*update slice time*/
            MBB_MEM_CPY(&stNewTime, 
                                      &pstATtime->stUniversalTimeandLocalTimeZone, sizeof(stNewTime) );

            stNewTime.ucSecond += (VOS_UINT8)(Seconds % 60);  /* slice secods*/
            stNewTime.ucMinute  += (VOS_UINT8)((Seconds / 60) % 60);   /* slice mitutes */
            stNewTime.ucHour    += (VOS_UINT8)((Seconds / 3600) % 24);   /*slice hours*/
            adjustDate = (Seconds / (3600 * 24));   /*slice days*/

            if ( stNewTime.ucSecond >= 60 ) /* check second*/
            {
                stNewTime.ucMinute += stNewTime.ucSecond / 60;  /* second to minute*/
                stNewTime.ucSecond = stNewTime.ucSecond % 60;  /* left second */
            }

            if ( stNewTime.ucMinute >= 60 ) /* check minute*/
            {
                stNewTime.ucHour += stNewTime.ucMinute / 60;  /* minute to hour*/
                stNewTime.ucMinute = stNewTime.ucMinute % 60;  /* left minute*/
            }

            if ( (adjustDate > 0) || (stNewTime.ucHour >= 24) ) /* check hour*/
            {
                adjustDate += (stNewTime.ucHour / 24);  /* hour to day*/
                stNewTime.ucHour = stNewTime.ucHour % 24;  /* left hour*/
                MBB_MEM_CPY(&pstNewTime->stUniversalTimeandLocalTimeZone,
                                        &stNewTime, sizeof(stNewTime));

                for ( i = 0; i < adjustDate; i++ )
                {
                    At_AdjustLocalDate( &stNewTime, 1, &pstNewTime->stUniversalTimeandLocalTimeZone );
                    MBB_MEM_CPY(&stNewTime, 
                                            &pstNewTime->stUniversalTimeandLocalTimeZone, sizeof(stNewTime));
                }
            }
            else
            {
                MBB_MEM_CPY(&pstNewTime->stUniversalTimeandLocalTimeZone,
                                        &stNewTime, sizeof(stNewTime) );
            }
        }

        AT_INFO_LOG("\r\nAt_GetLocalTime: Have Date and Time Info\n");
    }

    return (VOS_UINT32)pstATtime->ucIeFlg;
}


NAS_MM_INFO_IND_STRU* At_GetTimeInfo(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                          ulRslt = VOS_OK;
    AT_MODEM_NET_CTX_STRU               *pstNetCtx = VOS_NULL_PTR;
    MODEM_ID_ENUM_UINT16             enModemId;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("At_PrintMmTimeInfo: Get modem id fail.");
        return NULL;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromModemId(enModemId);
    
    return &(pstNetCtx->stTimeInfo);
}


TAF_UINT16 At_PrintNwTimeInfo(
    NAS_MM_INFO_IND_STRU    *pstMmInfo,
    TAF_UINT16              usLength,
    CONST_T VOS_CHAR       *cmd,
    CONST_T VOS_CHAR       *cmd_sep,
    CONST_T VOS_CHAR       *ending_str,
    TIME_FORMAT_ENUM_UINT32 eTimeFormat
)
{
    TAF_INT8            cTimeZone;
    TAF_UINT8          ucDST = 0;
    TAF_UINT16        old_usLength = usLength;
    NAS_MM_INFO_IND_STRU    stLocalAtTimeInfo;

    /*时间显示格式: ^TIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    if ( NAS_MM_INFO_IE_UTLTZ == (pstMmInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) )
    {
        MBB_MEM_SET(&stLocalAtTimeInfo, 0x00, sizeof(stLocalAtTimeInfo) );
        
        (VOS_VOID)At_GetLiveTime( pstMmInfo, &stLocalAtTimeInfo );
        /* "^NWTIME: */
        /* 主动上报前面有分隔符号，非主动上报不需要: */
        if ((TIME_FORMAT_RPT_TIME == eTimeFormat) || (TIME_FORMAT_RPT_NWTIME == eTimeFormat) )
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%s",
                                (VOS_CHAR *)gaucAtCrLf);
        }
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%s%s",
                                (VOS_CHAR *)cmd, cmd_sep);

        if (TIME_FORMAT_QRY_CCLK == eTimeFormat)
        {
            
            /* YYYY */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%4d/",
                           stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear + 2000); /*year*/            
        }
        else
        {
            /* YY */
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%d%d/",
                           stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear / 10, /*year high*/
                           stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucYear % 10); /*year low*/
        }
        /* MM */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d/",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth / 10,/*month high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMonth % 10);/*month low*/
        /* dd */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d,",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucDay / 10,/*day high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucDay % 10);/*day high*/

        /* hh */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d:",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucHour / 10,/*hour high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucHour % 10);/*hour high*/

        /* mm */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d:",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute / 10,/*minutes high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucMinute % 10);/*minutes high*/

        /* ss */
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%d%d",
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond / 10,/*sec high*/
                               stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.ucSecond % 10);/*sec high*/

        /* GMT±tz, Summer(Winter) Time" */

        /* 获得时区 */
        if (NAS_MM_INFO_IE_LTZ == (stLocalAtTimeInfo.ucIeFlg & NAS_MM_INFO_IE_LTZ))
        {
            cTimeZone = stLocalAtTimeInfo.cLocalTimeZone;
            AT_EVENT_REPORT_LOG_1("cTimeZone", cTimeZone);
        }
        else
        {
             cTimeZone = stLocalAtTimeInfo.stUniversalTimeandLocalTimeZone.cTimeZone;
             AT_EVENT_REPORT_LOG_1("cTimeZone", cTimeZone);
        }
        if (AT_INVALID_TZ_VALUE != cTimeZone)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                               (VOS_CHAR *)pgucAtSndCodeAddr,
                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                               "%s%02d",
                               (cTimeZone < 0)? "-": "+",
                               (cTimeZone < 0)? ( - cTimeZone): cTimeZone);
        }


        /* 显示夏时制或冬时制信息 */
        if ( (NAS_MM_INFO_IE_DST == (stLocalAtTimeInfo.ucIeFlg & NAS_MM_INFO_IE_DST))
          && (stLocalAtTimeInfo.ucDST > 0))
        {
            ucDST = stLocalAtTimeInfo.ucDST;
        }
        else
        {
            ucDST = 0;
        }
    if (TIME_FORMAT_QRY_CCLK != eTimeFormat)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           ",%d%d",
                           ucDST / 10, ucDST % 10); /*dst high/low*/
    }
    
    if (NULL != ending_str)
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%s",
                           ending_str );/*DST,endString*/
    }
    
    /*主动上报AT后面需要添加换行结束符号*/
    if ((TIME_FORMAT_RPT_TIME == eTimeFormat) || (TIME_FORMAT_RPT_NWTIME == eTimeFormat) )
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR *)pgucAtSndCodeAddr,
                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                           "%s",
                            (VOS_CHAR *)gaucAtCrLf);
    }

    }

    return usLength - old_usLength;
}
/*****************************************************************************
 函 数 名  : At_FormatDlckCnf
 功能描述  : 格式化dlck的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 At_FormatDlckCnf(TAF_UINT16* usLength, TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU  *pEvent)
{
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (TAF_CHAR *)pgucAtSndCodeAddr,
                (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                "%s: ",
                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                (TAF_CHAR *)pgucAtSndCodeAddr,
                (TAF_CHAR *)pgucAtSndCodeAddr + *usLength,
                "%d,%d",
                pEvent->MePersonalisation.unReportContent.TataLockInfo.ulDeviceLockStatusFlg,
                pEvent->MePersonalisation.unReportContent.TataLockInfo.ulDeviceUnlockFlg);

    return AT_OK;
}
/*****************************************************************************
 函 数 名  : At_FormatEons0
 功能描述  : 格式化dlck的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatAndSndEons0(TAF_UINT8 ucIndex, TAF_PHONE_EVENT_INFO_STRU *pEvent)
{
    TAF_UINT16 usLength = 0;
    if((VOS_TRUE == pEvent->stMmInfo.blRcvNwNameFlag) && (VOS_TRUE == pEvent->OP_MmInfo))
    {
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                      "\r\n^EONS:0\r\n");
        At_SendResultData(ucIndex,pgucAtSndCodeAddr, usLength);
    }
}


VOS_UINT32 TAF_SyscfgexLteLowBand2Str(TAF_PH_PREF_BAND band,TAF_CHAR *strBand)
{
    VOS_UINT32 i,ulLen;
    VOS_UINT32 ulBandStrLen = 0;
    TAF_CHAR *strSep = "/";

    ulLen = sizeof(gastSyscfgexLteLowBandStr) / sizeof(gastSyscfgexLteLowBandStr[0]);

    for (i = 0; i < ulLen; i++)
    {
        if(0 != (band & gastSyscfgexLteLowBandStr[i].BandMask))
        {
            /*lint -e534*/
            strncpy(strBand + ulBandStrLen, gastSyscfgexLteLowBandStr[i].BandStr,
                            VOS_StrLen(gastSyscfgexLteLowBandStr[i].BandStr));

            ulBandStrLen += VOS_StrLen(gastSyscfgexLteLowBandStr[i].BandStr);

            strncpy(strBand + ulBandStrLen, strSep,VOS_StrLen(strSep));
            /*lint +e534*/
            ulBandStrLen += VOS_StrLen(strSep);

            if((TAF_PH_SYSCFG_GROUP_BAND_LEN / 2) <= ulBandStrLen)
            {
                break;
            }
        }
    }

    if (ulBandStrLen > 0)
    {
        strBand[ulBandStrLen - 1] = '\0';
    }

    return TAF_SUCCESS;
}


VOS_UINT32 TAF_SyscfgexLteHighBand2Str(TAF_PH_PREF_BAND band,TAF_CHAR *strBand)
{
    VOS_UINT32 i,ulLen;
    VOS_UINT32 ulBandStrLen = 0;
    TAF_CHAR *strSep = "/";

    ulLen = sizeof(gastSyscfgexLteHighBandStr) / sizeof(gastSyscfgexLteHighBandStr[0]);

    for (i = 0; i < ulLen; i++)
    {
        if(0 != (band & gastSyscfgexLteHighBandStr[i].BandMask))
        {
            /*lint -e534*/
            strncpy(strBand + ulBandStrLen, gastSyscfgexLteHighBandStr[i].BandStr,
                            VOS_StrLen(gastSyscfgexLteHighBandStr[i].BandStr));

            ulBandStrLen += VOS_StrLen(gastSyscfgexLteHighBandStr[i].BandStr);

            strncpy(strBand + ulBandStrLen, strSep,VOS_StrLen(strSep));
            /*lint +e534*/
            ulBandStrLen += VOS_StrLen(strSep);
            /*字符串过长*/
            if((TAF_PH_SYSCFG_GROUP_BAND_LEN / 2) <= ulBandStrLen)
            {
                break;
            }
        }
    }

    if (ulBandStrLen > 0)
    {
        strBand[ulBandStrLen - 1] = '\0';
    }

    return TAF_SUCCESS;

}


VOS_VOID    AT_GetLTEBandStrMbb(TAF_PH_SYSCFG_BAND_STR *pstSysCfgBandStr, TAF_PHONE_EVENT_INFO_STRU *pEvent)
{ 
    TAF_PH_PREF_BAND  ltelowbandnone = MN_MMA_LTE_LOW_BAND_NONE;
    TAF_CHAR *strTempLteBandGroup = pstSysCfgBandStr->strSysCfgBandGroup;
    TAF_CHAR *LteBandStr1 = VOS_NULL_PTR;
    TAF_CHAR *LteBandStr2 = VOS_NULL_PTR;

    if ((0 == pEvent->ulSysCfgExLTEBandGroup1) && (0 == pEvent->ulSysCfgExLTEBandGroup2))
    {
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((7fffffffffffffff,\"All bands\"))");
        return;
    }

    /*高低位频段字符串的长度分别为总长度的一半*/
    LteBandStr1 = (TAF_CHAR*)MBB_MEM_ALLOC(WUEPS_PID_AT, (TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));

    if (VOS_NULL_PTR == LteBandStr1)
    {
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((7fffffffffffffff,\"All bands\"))");
        return;
    }
    /*高低位频段字符串的长度分别为总长度的一半*/
    LteBandStr2 = (TAF_CHAR*)MBB_MEM_ALLOC(WUEPS_PID_AT, (TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));

    if (VOS_NULL_PTR == LteBandStr2)
    {
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((7fffffffffffffff,\"All bands\"))");
        /*lint -e516*/
        MBB_MEM_FREE(WUEPS_PID_AT, LteBandStr1);
        /*lint +e516*/
        return;
    }
    /*高低位频段字符串的长度分别为总长度的一半*/
    MBB_MEM_SET(LteBandStr1, 0,(TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));
    /*高低位频段字符串的长度分别为总长度的一半*/
    MBB_MEM_SET(LteBandStr2, 0,(TAF_PH_SYSCFG_GROUP_BAND_LEN / 2));

    if ((0 != pEvent->ulSysCfgExLTEBandGroup1) && (0 != pEvent->ulSysCfgExLTEBandGroup2))
    {
        (VOS_VOID)TAF_SyscfgexLteHighBand2Str(pEvent->ulSysCfgExLTEBandGroup1, LteBandStr1);
        (VOS_VOID)TAF_SyscfgexLteLowBand2Str(pEvent->ulSysCfgExLTEBandGroup2, LteBandStr2);

        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN,strTempLteBandGroup,strTempLteBandGroup, 
                                           "((%x%08x,\"%s/%s\"),(7fffffffffffffff,\"All bands\"))",
                                           pEvent->ulSysCfgExLTEBandGroup1, 
                                           pEvent->ulSysCfgExLTEBandGroup2,
                                           LteBandStr2,
                                           LteBandStr1);
    }
    else if  (0 != pEvent->ulSysCfgExLTEBandGroup1)
    {
        (VOS_VOID)TAF_SyscfgexLteHighBand2Str(pEvent->ulSysCfgExLTEBandGroup1, LteBandStr1);

        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN, strTempLteBandGroup,
                                strTempLteBandGroup, "((%x%08x,\"%s\"),(7fffffffffffffff,\"All bands\"))",
                                 pEvent->ulSysCfgExLTEBandGroup1, ltelowbandnone, LteBandStr1);
    }
    else
    {
        (VOS_VOID)TAF_SyscfgexLteLowBand2Str(pEvent->ulSysCfgExLTEBandGroup2, LteBandStr2);
        (VOS_VOID)At_sprintf(TAF_PH_SYSCFG_GROUP_BAND_LEN,strTempLteBandGroup,
                                strTempLteBandGroup, "((%x,\"%s\"),(7fffffffffffffff,\"All bands\"))",
                                pEvent->ulSysCfgExLTEBandGroup2, LteBandStr2);
    }
    /*lint -e516*/
    MBB_MEM_FREE(WUEPS_PID_AT, LteBandStr1);
    MBB_MEM_FREE(WUEPS_PID_AT, LteBandStr2);
    /*lint +e516*/
    return;
}


VOS_VOID    AT_ProcSimRefreshInd(
    VOS_UINT8                           ucIndex,
    const TAF_PHONE_EVENT_INFO_STRU          *pstEvent
)
{
    VOS_UINT16                          usLength = 0;
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT32                          ulRslt;

    usLength  = 0;
    enModemId = MODEM_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRslt)
    {
        AT_ERR_LOG("AT_ProcSimRefreshInd: Get modem id fail."); /*lint !e516*/
        return;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,
        (VOS_CHAR *)pgucAtSndCodeAddr + usLength, "%s^SIMREFRESH:%d%s",
        gaucAtCrLf, pstEvent->RefreshType, gaucAtCrLf);
        
    At_SendResultData(ucIndex,pgucAtSndCodeAddr,usLength);
    
    return;
}


/*****************************************************************************
 函 数 名  : AT_MBBConverAutoMode
 功能描述  : 转化SYSCFGEX命令设置的ATUO模式为单板实际支持模式的全集
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID AT_MBBConverAutoMode(TAF_PH_RAT_ORDER_STRU    *pstSysCfgRatOrder)
{
    VOS_UINT8                           ucUserRatNum = 0;

#if(FEATURE_ON == FEATURE_LTE)
        if (VOS_TRUE == g_MbbIsRatSupport.ucLteSupport)
        {
             pstSysCfgRatOrder->aenRatOrder[ucUserRatNum] = TAF_PH_RAT_LTE;
             ucUserRatNum++;
        }
#endif

    /* 平台支持WCDMA */
    if (VOS_TRUE == g_MbbIsRatSupport.ucWcdmaSupport)
    {
         pstSysCfgRatOrder->aenRatOrder[ucUserRatNum] = TAF_PH_RAT_WCDMA;
         ucUserRatNum++;
    }

#if (FEATURE_OFF == MBB_TELSTRA_CUSTOMSIZE)
    /* 平台支持GSM */
    if (VOS_TRUE == g_MbbIsRatSupport.ucGsmSupport)
    {
         pstSysCfgRatOrder->aenRatOrder[ucUserRatNum] = TAF_PH_RAT_GSM;
         ucUserRatNum++;
    }
#endif /*FEATURE_OFF == MBB_TELSTRA_CUSTOMSIZE*/

    pstSysCfgRatOrder->ucRatOrderNum = ucUserRatNum;
    return;
}

/*****************************************************************************
 函 数 名  : At_GetSupportRatOrderMbb
 功能描述  : 格式化syscfg的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID   At_GetSupportRatOrderMbb(VOS_CHAR *pachDest)
{
    VOS_UINT32                          ulLength = 0;

    /*添加括号*/
    ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest,  "(\"00\"," );

    if(VOS_TRUE == g_MbbIsRatSupport.ucGsmSupport)
    {
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength,  "\"01\",");      
    }

    if(VOS_TRUE == g_MbbIsRatSupport.ucWcdmaSupport)
    {
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength,  "\"02\"," );
    }

    /*LTE 频段不为空*/
    if(VOS_TRUE == g_MbbIsRatSupport.ucLteSupport)
    {
        ulLength += (VOS_UINT32)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength,  "\"03\"," );
    }

    (VOS_VOID)At_sprintf(DEST_STRING_MAX_LEN, pachDest, pachDest + ulLength, "\"99\")" );

    return;
}

/*****************************************************************************
 函 数 名  : At_FormatSyscfgMbb
 功能描述  : 格式化syscfg的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatSyscfgMbb(AT_MODEM_NET_CTX_STRU *pstNetCtx, TAF_PHONE_EVENT_INFO_STRU* pEvent, VOS_UINT8 ucIndex)
{
    VOS_CHAR                            achModeStr[DEST_STRING_MAX_LEN] = { 0 };
    VOS_CHAR                            strSysCfgLTEBandGroup[TAF_PH_SYSCFG_GROUP_BAND_LEN]= {"\0"};

    AT_GetLTEBandStrMbb((TAF_PH_SYSCFG_BAND_STR*)strSysCfgLTEBandGroup, pEvent);
    if ( VOS_TRUE == AT_GenerateModeListFromNV( achModeStr ) )
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            "%s: "
                                                            "%s,"
                                                            "%s,"
                                                            "(0-2),"
                                                            "(0-4),"
                                                            "%s",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            achModeStr,
                                                            pEvent->strSysCfgBandGroup,
                                                            strSysCfgLTEBandGroup);
    }
    else
    {
        At_GetSupportRatOrderMbb(achModeStr);
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                                            "%s:"
                                                            "%s,"
                                                            "%s,"
                                                            "(0-2),"
                                                            "(0-4),"
                                                            "%s",
                                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                            achModeStr,
                                                            pEvent->strSysCfgBandGroup,
                                                            strSysCfgLTEBandGroup);
    }
}
/*****************************************************************************
 函 数 名  : At_FormatEons0
 功能描述  : 格式化eons=0的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatSysinfoExMbb(VOS_UINT16* usLength, TAF_PH_SYSINFO_STRU* stSysInfo)
{
    if (TAF_PH_INFO_NO_SERV == stSysInfo->ucSrvStatus)
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *usLength,",%d",TAF_PH_INFO_NO_DOMAIN);
    }
    else 
    {
        *usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + *usLength,",%d",stSysInfo->ucSrvDomain);
    }
}

/*****************************************************************************
 函 数 名  : At_FormatRssiInfo
 功能描述  : 格式化rssi的at返回
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_VOID At_FormatRssiInfo(TAF_UINT8 ucIndex,TAF_PHONE_EVENT_INFO_STRU *pEvent)
{
    VOS_UINT16                         usLength = 0;
    VOS_CHAR                           aucSysModeName[AT_HCSQ_RAT_NAME_MAX] = {0};
    VOS_UINT8                           rssi = 0;
    VOS_UINT8                           rscp = 0;
    VOS_UINT8                           ecio = 0;
    VOS_UINT8                           aucSysModeEx = MN_PH_SYS_MODE_EX_BUTT_RAT;
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8  ant_level = AT_CMD_ANTENNA_LEVEL_0;
    VOS_UINT16 Ciev_usLength = 0;
#endif
    if(1 == pEvent->OP_Rssi)
    {
        MBB_MEM_SET(aucSysModeName, 0x00, sizeof(aucSysModeName));
        switch(pEvent->RssiValue.enRatType)
        {
            case TAF_PH_RAT_GSM:
                aucSysModeEx = MN_PH_SYS_MODE_EX_GSM_RAT;
                break;

            case TAF_PH_RAT_WCDMA:
                aucSysModeEx = AT_JUDGE_FDD_TDD(pEvent->RssiValue.ucCurrentUtranMode);
                break;

            default: 
                    aucSysModeEx = MN_PH_SYS_MODE_EX_NONE_RAT;
                    break;
        }
        
        (VOS_VOID)AT_GetSysModeName(aucSysModeEx, aucSysModeName);
        
        if(aucSysModeEx != MN_PH_SYS_MODE_EX_NONE_RAT)
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (TAF_CHAR *)pgucAtSndCodeAddr,
                                          (TAF_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s%s\"%s\"",
                                          gaucAtCrLf,
                                          gastAtStringTab[AT_STRING_HCSQ].pucText,
                                          aucSysModeName);
        }
        
        switch(aucSysModeEx)
        {
            case MN_PH_SYS_MODE_EX_GSM_RAT:
                {
                    /*调用转换函数对rssi按规则进行转换*/
                    AT_RssiConvert(pEvent->RssiValue.aRssi[0].u.stGCellSignInfo.sRssiValue, &rssi);
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d%s", 
                                                      rssi,
                                                      gaucAtCrLf);
                    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);
#if (FEATURE_ON == MBB_MLOG)
                    /*TS 43.022 4.4.3 A PLMN shall be understood to be received with high quality signal if the signal level is above  -85 dBm*/
                    g_stSignalInfo.sRssiValue = pEvent->RssiValue.aRssi[0].u.stGCellSignInfo.sRssiValue;
#endif
                }
                break;
                
            case MN_PH_SYS_MODE_EX_WCDMA_RAT:
                {
                    /*调用转换函数对rssi按规则进行转换*/
                    AT_RssiConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue 
                                    - pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sEcioValue,
                                     &rssi);
                    /*调用转换函数对RSCP按规则进行转换*/
                    AT_RscpConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue, &rscp);
                    /*调用转换函数对ECIO按规则进行转换*/
                    AT_EcioConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sEcioValue, &ecio);         
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d,%d,%d%s", 
                                                      rssi,
                                                      rscp,
                                                      ecio,
                                                      gaucAtCrLf);
                    At_SendResultData(ucIndex, pgucAtSndCodeAddr,usLength); 
                    /* TS25304  5.1.2.2 UMTS PLMN for an FDD cell, the measured primary CPICH RSCP value shall be greater than or equal to -95 dBm.*/
#if (FEATURE_ON == MBB_MLOG)
                    g_stSignalInfo.sRscpValue = pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue;
                    g_stSignalInfo.sEcioValue = pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sEcioValue;
#endif
                }
                break;
                
            case MN_PH_SYS_MODE_EX_TDCDMA_RAT:
                {
                    /*调用转换函数对rssi按规则进行转换*/
                    AT_RssiConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue 
                                    - pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sEcioValue, &rssi);
                    /*调用转换函数对RSCP按规则进行转换*/
                    AT_RssiConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue, &rscp);
                    /* 非fdd 3g 小区，ecio值为无效值255 ，rssi报上来的是0*/               
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                                      (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                      ",%d,%d,%d%s", 
                                                      rscp,
                                                      rscp,
                                                      255,/*invalid value 255*/
                                                      gaucAtCrLf);
                    /*TS25304  5.1.2.2, For a TDD cell, the measured P-CCPCH RSCP shall be greater than or equal to -84 dBm*/
#if (FEATURE_ON == MBB_MLOG)
                    g_stSignalInfo.sRssiValue = pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue;
                    if(AT_HIGH_QULITY_RSCP_TDD_MIN > pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue)
                    {
                        mlog_print("at",mlog_lv_error,"rscp is %d.\n",pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue);
                    }
#endif
                    At_SendResultData(ucIndex, pgucAtSndCodeAddr,(VOS_UINT16)usLength); 
                }
                break;
                
            
            default:
                break;
        }
    }
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    if(((AT_CMER_RPT_MODE_SWITCH_ON == g_ucCmerpara.mode)
        && (AT_CMER_RPT_SWITCH_ON == g_ucCmerpara.ind))
        && (AT_CIND_RPT_SWITCH_ON == g_ucCindpara.signal))
    {
        switch(aucSysModeEx)
        {
            case MN_PH_SYS_MODE_EX_GSM_RAT:
            {
                /*调用转换函数对rssi按规则进行转换*/
                AT_RssiConvert(pEvent->RssiValue.aRssi[0].u.stGCellSignInfo.sRssiValue, &rssi);
                AT_CalculateModeAntennaLevel(rssi, MN_PH_SYS_MODE_EX_GSM_RAT, &ant_level);
                if(g_oldGsmAntLevel != ant_level)
                {
                    Ciev_usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + Ciev_usLength,
                                                  "%s%s%d,%d%s",
                                                  gaucAtCrLf,
                                                  gastAtStringTab[AT_STRING_CIEV].pucText,
                                                  CIEV_SIGNAL_FLAG,
                                                  ant_level,
                                                  gaucAtCrLf);
                At_SendResultData(ucIndex, pgucAtSndCodeAddr, Ciev_usLength);
                g_oldGsmAntLevel = ant_level;
                }
                break;
            }
            case MN_PH_SYS_MODE_EX_WCDMA_RAT:
            {
                /*调用转换函数对RSCP按规则进行转换*/
                AT_RscpConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue, &rscp);
                AT_CalculateModeAntennaLevel(rscp, MN_PH_SYS_MODE_EX_WCDMA_RAT, &ant_level);
                if(g_oldWcdmaAntLevel != ant_level)
                {
                    Ciev_usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + Ciev_usLength,
                                                  "%s%s%d,%d%s",
                                                  gaucAtCrLf,
                                                  gastAtStringTab[AT_STRING_CIEV].pucText,
                                                  CIEV_SIGNAL_FLAG,
                                                  ant_level,
                                                  gaucAtCrLf);
                    At_SendResultData(ucIndex, pgucAtSndCodeAddr, Ciev_usLength);
                    g_oldWcdmaAntLevel = ant_level;
                }
                break;
            }
            case MN_PH_SYS_MODE_EX_TDCDMA_RAT:
            {
                /*调用转换函数对RSCP按规则进行转换*/
                AT_RssiConvert(pEvent->RssiValue.aRssi[0].u.stWCellSignInfo.sRscpValue, &rscp);
                /* 非fdd 3g 小区，ecio值为无效值255 ，rssi报上来的是0*/
                AT_CalculateModeAntennaLevel(rscp,MN_PH_SYS_MODE_EX_TDCDMA_RAT,&ant_level);
                if(g_oldTDscdmaAntLevel != ant_level)
                {
                    Ciev_usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr,
                                                  (VOS_CHAR *)pgucAtSndCodeAddr + Ciev_usLength,
                                                  "%s%s%d,%d%s",
                                                  gaucAtCrLf,
                                                  gastAtStringTab[AT_STRING_CIEV].pucText,
                                                  CIEV_SIGNAL_FLAG,
                                                  ant_level,
                                                  gaucAtCrLf);
                    At_SendResultData(ucIndex, pgucAtSndCodeAddr, Ciev_usLength);
                    g_oldTDscdmaAntLevel = ant_level;
                }
                break;
            }
            default:
                break;
        }
    }
#endif
}
/*****************************************************************************
 函 数 名  : AT_PS_ReportDefaultDhcp
 功能描述  : 默认返回第一个wan的信息
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 AT_PS_ReportDefaultDhcp(VOS_UINT8 ucIndex)
{
    /*多WAN与原单WAN兼容处理，可以处理使用查询命令查询到第一个WAN的地址信息*/
    gucAtParaIndex = 1;

#ifdef BSP_CONFIG_BOARD_K5160
    gastAtParaList[0].ulParaValue = g_stAtNdisCid;
#else
    gastAtParaList[0].ulParaValue = AT_PS_USER_CID_1;
#endif/*BSP_CONFIG_BOARD_K5160*/

    /* 查询指定CID的实体PDP上下文 */
    return AT_PS_ReportDhcp(ucIndex);
}
/*****************************************************************************
 函 数 名  : AT_PS_ReportDefaultDhcpV6
 功能描述  : 默认返回第一个wan的信息
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 AT_PS_ReportDefaultDhcpV6(VOS_UINT8 ucIndex)
{
    /*多WAN与原单WAN兼容处理，可以处理使用查询命令查询到第一个WAN的地址信息*/
    gucAtParaIndex = 1;

#ifdef BSP_CONFIG_BOARD_K5160
    gastAtParaList[0].ulParaValue = g_stAtNdisCid;
#else
    gastAtParaList[0].ulParaValue = AT_PS_USER_CID_1;
#endif/*BSP_CONFIG_BOARD_K5160*/

    /* 查询指定CID的实体PDP上下文 */
    return AT_PS_ReportDhcpv6(ucIndex);
}
/*****************************************************************************
 函 数 名  : AT_PS_ReportDefaultApraInfo
 功能描述  : 默认返回第一个wan的信息
 输入参数  : 
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :
 修改历史  :
*****************************************************************************/
VOS_UINT32 AT_PS_ReportDefaultApraInfo(TAF_UINT8 ucIndex)
{
    /*多WAN与原单WAN兼容处理，可以处理使用查询命令查询到第一个WAN的地址信息*/
    gucAtParaIndex = 1;
    gastAtParaList[0].ulParaValue = AT_PS_USER_CID_1;
    
    /* 查询指定CID的实体PDP上下文 */
    return AT_PS_ReportApraInfo(ucIndex);
}

VOS_UINT8 AT_GetIPv6VerFlag(VOS_VOID)
{
    return g_ucIPv6VerFlag;
}


VOS_VOID AT_SetIPv6VerFlag( VOS_UINT8 ucFlag )
{
    g_ucIPv6VerFlag = ucFlag;
}


VOS_UINT32 At_CheckSpecRatOrderInModeList(AT_SYSCFGEX_RAT_ORDER_STRU *pstSysCfgExRatOrder)
{
    VOS_UINT8                   i, j;
    VOS_UINT8                  *pucTemp;
    VOS_UINT8                   aucModeList[MODE_LIST_MAX_NUM][MODE_LIST_MAX_LEN] = {0};
    VOS_UINT8                   ucListItemNum = AT_GetSyscfgexModeListItemNum();
    VOS_UINT8                  *paucModeList = AT_GetSyscfgexModeList();
    MBB_MEM_CPY( aucModeList, paucModeList, (MODE_LIST_MAX_NUM * MODE_LIST_MAX_LEN) );

    if ( VOS_NULL_PTR == pstSysCfgExRatOrder )
    {
        return VOS_FALSE;
    }

    for ( i = 0; i < ucListItemNum; i++ )
    {
        /*进行匹配*/
        pucTemp = aucModeList[i];
        for ( j = 0; j < pstSysCfgExRatOrder->ucRatOrderNum; j++ )
        {
            if ( (pstSysCfgExRatOrder->aenRatOrder[j] == pucTemp[j])
            /*对于99需要额外判断*/
              || ((99 == pstSysCfgExRatOrder->aenRatOrder[j]) && (0x99 == pucTemp[j])) )
            {
                continue;
            }
            else
            {
                break;
            }
        }
        /*匹配成功*/
        if ( (j == pstSysCfgExRatOrder->ucRatOrderNum) && (0xFF == pucTemp[j]) )
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_UINT32 Taf_NwScanHandle(
    MN_CLIENT_ID_T                         ClientId,
    MN_OPERATION_ID_T                   OpId,
    TAF_PH_NW_SCAN_STRU              stNwScanPara
)
{
    VOS_UINT32 ulRst = TAF_SUCCESS;

    /* 向MMA发送设置消息*/
    ulRst = MN_FillAndSndAppReqMsg( ClientId, OpId, TAF_MSG_MMA_NWSCAN_HANDLE,
                                  &stNwScanPara, sizeof(stNwScanPara), I0_WUEPS_PID_MMA );

    return ulRst;
}



TAF_UINT32 TAF_EONSGetNWName ( MN_CLIENT_ID_T           ClientId,
                                 MN_OPERATION_ID_T        OpId,
                                 AT_TAF_PLMN_ID stPlmn)
{
    /*定义返回值类型*/
    VOS_UINT32                           ulRst;

    /* 发送消息 TAF_MSG_GET_PLMN_LIST 给 MMA 处理， */
    ulRst = MN_FillAndSndAppReqMsg(ClientId,OpId,
                                         TAF_MSG_SET_EONS_MSG,
                                         (VOS_VOID*)&stPlmn,
                                         sizeof(AT_TAF_PLMN_ID),
                                         I0_WUEPS_PID_MMA);
    return ulRst;
}


TAF_PDP_TYPE_ENUM_UINT8 AT_GetPdpTypeForNdisDialup(VOS_VOID)
{
    TAF_PDP_TYPE_ENUM_UINT8      enPdpType = TAF_PDP_IPV4;
    TAF_PDP_TYPE_ENUM_UINT8      ucIpv6Capability = AT_IPV6_CAPABILITY_IPV4_ONLY;

#if ( FEATURE_ON == FEATURE_IPV6 )
    ucIpv6Capability = AT_GetIpv6Capability();
    if( AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP == ucIpv6Capability )
    {
        enPdpType = TAF_PDP_IPV4V6;
    }
    else if ( AT_IPV6_CAPABILITY_IPV6_ONLY == ucIpv6Capability )
    {
        enPdpType = TAF_PDP_IPV6;
    }
    else
#endif /*FEATURE_ON == FEATURE_IPV6*/
    {
        enPdpType = TAF_PDP_IPV4;
    }

    return enPdpType;
}

VOS_VOID AT_PS_ProcIpv4CallRejectEx(
    VOS_UINT8                           ucCallId,
    TAF_PS_CALL_PDP_ACTIVATE_REJ_STRU  *pstEvent
)
{
    /* 用户发起IPv4v6类型的PDP激活, 而且被网络拒绝, 原因为28, 协议栈需要
       分别发起IPv4/IPv6类型的PDP激活, 协议栈首先发起IPv4, 再发起IPv6,
       如果IPv4类型的PDP激活再次被网络拒绝, 协议栈还需要尝试IPV6类型的
       PDP激活为了防止PDP激活嵌套, 如果IPv6类型的PDP激活失败, 将不再尝试
       IPv4类型的PDP激活 */

    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;
    pstCallEntity = AT_PS_GetCallEntity(pstEvent->stCtrl.usClientId, ucCallId);


#if (FEATURE_ON == FEATURE_IPV6)
    AT_PDP_STATE_ENUM_U8                enPreCallState;

    enPreCallState = AT_PS_GetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4);
#endif

    /* 设置IPv4对应的CID为无效 */
    AT_PS_SetCid2CurrCall(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PS_CALL_INVALID_CID);

    /* 将IPv4类型的PDP状态切换到IDLE */
    AT_PS_SetCallStateByType(pstEvent->stCtrl.usClientId, ucCallId, TAF_PDP_IPV4, AT_PDP_STATE_IDLE);

    /*NDIS双栈拨号，保存当前ipv4被拒原因值*/
    if((AT_CLIENT_ID_NDIS == pstEvent->stCtrl.usClientId) && (TAF_PDP_IPV4V6 == pstCallEntity->stUsrDialParam.enPdpType))
    {
        pstCallEntity->enIpv4Cause = pstEvent->enCause;
    }
    else
    {
        /* 上报IPv4拨号失败 */

    /* 上报IPv4拨号失败 */
    AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                             ucCallId,
                             TAF_PDP_IPV4,
                             pstEvent->enCause);

        /* 释放CALLID和CID的映射关系 */
        AT_PS_FreeCallIdToCid(pstEvent->stCtrl.usClientId, pstEvent->ucCid);

    }


#if (FEATURE_ON == FEATURE_IPV6)
    if ( (AT_PS_IsUsrDialTypeDualStack(pstEvent->stCtrl.usClientId, ucCallId))
      && (AT_PDP_STATE_ACTING == enPreCallState) )
    {
        if (VOS_OK == AT_PS_ProcIpv4ConnFailFallback(ucCallId, pstEvent))
        {
            return;
        }
        else
        {
            /* 记录呼叫错误码 */
            AT_PS_SetPsCallErrCause(pstEvent->stCtrl.usClientId, TAF_PS_CAUSE_UNKNOWN);
            /*NDIS双栈拨号回退ipv4被拒，发起ipv6失败时，如果实体中ipv4原因值不为0，上报ipv4失败*/
            if((AT_CLIENT_ID_NDIS == pstEvent->stCtrl.usClientId) && (TAF_PS_CAUSE_SUCCESS != pstCallEntity->enIpv4Cause))
            {
                AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                         ucCallId,
                                         TAF_PDP_IPV4,
                                         pstCallEntity->enIpv4Cause);
            }


            /* 上报IPv6拨号失败 */
            AT_PS_SndCallEndedResult(pstEvent->stCtrl.usClientId,
                                     ucCallId,
                                     TAF_PDP_IPV6,
                                     AT_PS_GetPsCallErrCause(pstEvent->stCtrl.usClientId));
        }
    }
#endif

    /* 释放呼叫实体 */
    AT_PS_FreeCallEntity(pstEvent->stCtrl.usClientId, ucCallId);

    return;
}

VOS_VOID AT_PS_SndIPV4FailedResult(VOS_UINT8 ucCallId, VOS_UINT16 usClientId)
{
    AT_PS_CALL_ENTITY_STRU             *pstCallEntity = VOS_NULL_PTR;

    pstCallEntity = AT_PS_GetCallEntity(usClientId, ucCallId);

    /*NDIS双栈回退，ipv6被拒，上一次拒绝的ipv4需要上报，在此处上报ipv4失败状态*/
    if((TAF_PS_CAUSE_SUCCESS != pstCallEntity->enIpv4Cause) && (AT_CLIENT_ID_NDIS == usClientId))
    {
        AT_PS_SndCallEndedResult(usClientId,
                                ucCallId,
                                TAF_PDP_IPV4,
                                pstCallEntity->enIpv4Cause);
    }
}


VOS_UINT32 AT_PS_ValidateDialParamEx(VOS_UINT8 ucIndex)
{
    AT_PS_DATA_CHANL_CFG_STRU          *pstChanCfg = VOS_NULL_PTR;
    VOS_UINT8                           aucIpv4Addr[TAF_IPV4_ADDR_LEN];


    /* 检查命令类型 */
    if (AT_CMD_OPT_SET_CMD_NO_PARA == g_stATParseCmd.ucCmdOptType)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: No parameter input.");
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /* 检查参数个数 */
    if (gucAtParaIndex > 7)  /*拨号加上IP Addr可以多达7个参数*/
    {
        AT_NORM_LOG1("AT_PS_ValidateDialParam: Parameter number is %d.\n", gucAtParaIndex);
        return AT_TOO_MANY_PARA;
    }

    /* 检查 CID */
    if (0 == gastAtParaList[0].usParaLen)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing CID.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 拨号 CONN: 该参数不能省略, 1表示建立连接, 0表示断开断开连接 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        AT_NORM_LOG("AT_PS_ValidateDialParam: Missing connect state.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查 APN */
    if (0 != gastAtParaList[2].usParaLen)
    {
        /* APN长度检查 */
        if (gastAtParaList[2].usParaLen > TAF_MAX_APN_LEN)
        {
            AT_NORM_LOG("AT_PS_ValidateDialParam: APN is too long.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* APN格式检查 */
        if (VOS_OK != AT_CheckApnFormat(gastAtParaList[2].aucPara,
                                        gastAtParaList[2].usParaLen))
        {
            AT_NORM_LOG("AT_PS_ValidateDialParam: Format of APN is wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 检查 Username */
    if (gastAtParaList[3].usParaLen > TAF_MAX_GW_AUTH_USERNAME_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查 Password */
    if (gastAtParaList[4].usParaLen > TAF_MAX_GW_AUTH_PASSWORD_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*检查IP地址*/
    if (gastAtParaList[6].usParaLen >= TAF_MAX_IPV4_ADDR_STR_LEN) /*IP地址不能大于 15*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    else
    {
        if (gastAtParaList[6].usParaLen > 0) /*存在IP地址*/
        {
            if (VOS_OK != AT_Ipv4AddrAtoi((VOS_CHAR *)gastAtParaList[6].aucPara, aucIpv4Addr)) /*检查IP地址格式*/
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }

    /* 检查通道映射 */
    pstChanCfg = AT_PS_GetDataChanlCfg(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
    /*E5,hilink,stick形态产品未分配通道时，统一进行静态data channel分配，避免app/pcui拨号问题，兼容使用AT^CHDATA指定*/
    if ( ((VOS_FALSE == pstChanCfg->ulUsed) || (AT_PS_INVALID_RMNET_ID == pstChanCfg->ulRmNetId)) )
    {
        pstChanCfg->ulUsed = VOS_TRUE;
        pstChanCfg->ulRmNetId = (gastAtParaList[0].ulParaValue - TAF_MIN_CID) + RNIC_RM_NET_ID_0;
    }
    else if ( (VOS_FALSE == pstChanCfg->ulUsed)
      || (AT_PS_INVALID_RMNET_ID == pstChanCfg->ulRmNetId) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_PS_ParseUsrDialParamEx(VOS_UINT8 ucIndex, 
    AT_DIAL_PARAM_STRU *pstUsrDialParam, 
    TAF_PDP_PRIM_CONTEXT_STRU* stPdpCtxInfo)
{
    VOS_UINT32                          ulRslt;
    /* 设置<PDP_addr> */
    if ( gastAtParaList[6].usParaLen > 0 )  /*存在静态IP*/
    {
        /* IP地址格式已经统一在前面检查 */
        MBB_MEM_CPY(pstUsrDialParam->aucIPv4Addr, 
                    gastAtParaList[6].aucPara, TAF_MAX_IPV4_ADDR_STR_LEN);/*保存*/
        pstUsrDialParam->aucIPv4Addr[TAF_MAX_IPV4_ADDR_STR_LEN - 1]  = '\0';
        pstUsrDialParam->ulIPv4ValidFlag                        = VOS_TRUE;
    }

    if ( AT_IPV6_FIRST_VERSION == AT_GetIPv6VerFlag() )
    {
        /*恢复为2.0方案，以备下一次拨号*/
        AT_SetIPv6VerFlag( AT_IPV6_SECOND_VERSION );

        /*从CID对应的全局变量中获取PDP类型*/
#ifdef DMT
        ulRslt = MN_APS_GetPdpCidPara(stPdpCtxInfo, pstUsrDialParam->ucCid);
#else
        ulRslt = TAF_AGENT_GetPdpCidPara(stPdpCtxInfo, ucIndex, pstUsrDialParam->ucCid);
#endif

        if ( (VOS_OK == ulRslt)
          && (AT_PS_IS_PDP_TYPE_SUPPORT(stPdpCtxInfo->stPdpAddr.enPdpType)) )
        {
            pstUsrDialParam->enPdpType = stPdpCtxInfo->stPdpAddr.enPdpType;
        }
        else
        {
            pstUsrDialParam->enPdpType = TAF_PDP_IPV4;
        }

#if( FEATURE_ON == FEATURE_IPV6 )
        if (VOS_OK != AT_CheckIpv6Capability(pstUsrDialParam->enPdpType))
        {
            AT_INFO_LOG("AT_PS_ParseUsrDialParam: PDP type is not supported.");
            return VOS_ERR;
        }
#endif /*FEATURE_ON == FEATURE_IPV6*/
    }
    else
    {
        pstUsrDialParam->enPdpType= AT_GetPdpTypeForNdisDialup();
    }
    return VOS_OK;
}


VOS_VOID AT_PS_HangupAllCall(
    VOS_UINT16                          usClientId
)
{
    VOS_UINT8 ucCallId;
    AT_PS_USER_INFO_STRU               *pstUserInfo = VOS_NULL_PTR;
    
    for ( ucCallId = 0; ucCallId < AT_PS_MAX_CALL_NUM; ucCallId++ )
    {
        pstUserInfo = AT_PS_GetUserInfo(usClientId, ucCallId);
                
        if (VOS_OK != AT_PS_HangupCall(pstUserInfo->enUserIndex, ucCallId))
        {
            AT_ERR_LOG("AT_PS_HangupAllCall: Hangup call failed.");
        }
    }
    
    return;
}


VOS_UINT32 AT_PS_CheckSyscfgexModeRestrictPara(VOS_UINT32* ulRst, AT_SYSCFGEX_RAT_ORDER_STRU* stSyscfgExRatOrder)
{
    VOS_UINT32 ulRet;
    VOS_UINT8 ucRestrict = AT_GetSyscfgexModeRestrictFlag();
    if ( VOS_TRUE == ucRestrict )
    {
        /*仅列表中设置的组合可以设置*/
        ulRet = At_CheckSpecRatOrderInModeList( stSyscfgExRatOrder );
        if ( VOS_FALSE == ulRet )
        {
            *ulRst = AT_CME_INCORRECT_PARAMETERS;
            return VOS_FALSE;
        }
    }
    return VOS_TRUE;
}


TAF_UINT32 AT_SetDsFlowQryParaEx(TAF_UINT8 ucIndex)
{

    VOS_UINT8           ucUsrCid = TAF_MAX_CID; /*默认表示查询所有的CID流量*/
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD == g_stATParseCmd.ucCmdOptType )
    {
        if (0 == gastAtParaList[0].usParaLen)
        {
            AT_INFO_LOG("AT_AppCheckDialParam:cid not ext");
            return AT_CME_INCORRECT_PARAMETERS;
        }
        ucUsrCid = gastAtParaList[0].aucPara[0] - '0';
    }
    else if ( AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

   /*  执行命令操作  */
    if ( VOS_OK != TAF_PS_GetDsFlowInfo(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, ucUsrCid, 0))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_DSFLOWQRY_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_GetNdisDialParamEx(
    TAF_PS_DIAL_PARA_STRU              *pstDialParaInfo,
    VOS_UINT8                           ucIndex
)
{
    TAF_PDP_PRIM_CONTEXT_STRU                    stPdpCidInfo;
    VOS_UINT32                          ulRet;


    MBB_MEM_SET(&stPdpCidInfo, 0x00, sizeof(stPdpCidInfo));


    pstDialParaInfo->ucCid      = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* Modified by l60609 for DSDA PhaseII, 2012-12-21, begin */
#ifdef DMT
    ulRet = MN_APS_GetPdpCidPara(&stPdpCidInfo, pstDialParaInfo->ucCid);
#else
    ulRet = TAF_AGENT_GetPdpCidPara(&stPdpCidInfo, ucIndex, pstDialParaInfo->ucCid);
#endif
    /* Modified by l60609 for DSDA PhaseII, 2012-12-21, end */
    if (VOS_OK != ulRet)
    {
        return AT_FAILURE;
    }

    if ( AT_IPV6_FIRST_VERSION == AT_GetIPv6VerFlag() )
    {
        AT_SetIPv6VerFlag( AT_IPV6_SECOND_VERSION );

        /* 获取PDP类型 */
        if ( VOS_TRUE == gstAtNdisAddParam.ucPdpTypeValidFlag )
        {
            /*对应^NDISADD命令*/
            pstDialParaInfo->enPdpType = gstAtNdisAddParam.enPdpType;
        }
        else if ( (TAF_PDP_IPV4 == stPdpCidInfo.stPdpAddr.enPdpType)   /*从CID对应的全局变量中获取*/
               || (TAF_PDP_IPV6 == stPdpCidInfo.stPdpAddr.enPdpType)
               || (TAF_PDP_IPV4V6 == stPdpCidInfo.stPdpAddr.enPdpType) )
        {
#if ( FEATURE_OFF == FEATURE_IPV6 )
            if ( (TAF_PDP_IPV6 == stPdpCidInfo.stPdpAddr.enPdpType)
              || (TAF_PDP_IPV4V6 == stPdpCidInfo.stPdpAddr.enPdpType) )
            {
                return AT_FAILURE;
            }
#endif /*FEATURE_OFF == FEATURE_IPV6*/
            pstDialParaInfo->enPdpType = stPdpCidInfo.stPdpAddr.enPdpType;
        }
        else
        {
            /* 默认使用IPV4 */
            pstDialParaInfo->enPdpType = TAF_PDP_IPV4;
        }
#if ( FEATURE_ON == FEATURE_IPV6 )
        if ( VOS_ERR == AT_CheckIpv6Capability(pstDialParaInfo->enPdpType) )
        {
            AT_INFO_LOG( "AT_NidsGetDialParam:PDP type is error" );
            return AT_FAILURE;
        }
#endif /*FEATURE_ON == FEATURE_IPV6*/
    }
    else
    {
        pstDialParaInfo->enPdpType = AT_GetPdpTypeForNdisDialup();
    }

    if ( 0 != gastAtParaList[2].usParaLen )
    {
        pstDialParaInfo->bitOpApn       = VOS_TRUE;

        MBB_MEM_CPY(pstDialParaInfo->aucApn,
                   gastAtParaList[2].aucPara,
                   gastAtParaList[2].usParaLen);
        pstDialParaInfo->aucApn[gastAtParaList[2].usParaLen] = 0;
    }

    if ( 0 != gastAtParaList[3].usParaLen )
    {
        pstDialParaInfo->bitOpUserName  = VOS_TRUE;

        MBB_MEM_CPY(pstDialParaInfo->aucUserName,
                   gastAtParaList[3].aucPara,
                   gastAtParaList[3].usParaLen);
        pstDialParaInfo->aucUserName[gastAtParaList[3].usParaLen] = 0;
    }

    if ( 0 != gastAtParaList[4].usParaLen )
    {
        pstDialParaInfo->bitOpPassWord  = VOS_TRUE;

        MBB_MEM_CPY(pstDialParaInfo->aucPassWord,
                   gastAtParaList[4].aucPara,
                   gastAtParaList[4].usParaLen);
        pstDialParaInfo->aucPassWord[gastAtParaList[4].usParaLen] = 0;
    }

    /* Auth Type */
    if ( gastAtParaList[5].usParaLen > 0 )
    {
        pstDialParaInfo->bitOpAuthType  = VOS_TRUE;
        pstDialParaInfo->enAuthType     = AT_CtrlGetPDPAuthType(gastAtParaList[5].ulParaValue,
                                                                gastAtParaList[5].usParaLen);
    }
    else
    {
        /* 如果用户名和密码长度均不为0, 且鉴权类型未设置, 则默认使用CHAP类型 */
        if ( (0 != gastAtParaList[3].usParaLen)
          && (0 != gastAtParaList[4].usParaLen) )
        {
            pstDialParaInfo->bitOpAuthType = VOS_TRUE;
            pstDialParaInfo->enAuthType    = PPP_CHAP_AUTH_TYPE;
        }
    }

    return AT_SUCCESS;
}

#if (FEATURE_ON == MBB_FEATURE_MPDP) 

VOS_UINT8 AT_GetUsbNetNum(VOS_VOID)
{
    return g_ActUsbNetNum;
}

VOS_VOID AT_IncreaseNumWhenAct(VOS_VOID)
{
    g_ActUsbNetNum++;
}

VOS_VOID AT_DecreaseNumWhenDeact(VOS_VOID)
{
    if (g_ActUsbNetNum >= 1)
    {
        g_ActUsbNetNum--;
    }
}
/*****************************************************************************
 函 数 名  : AT_PS_ProcDialCmdEx
 功能描述  : 处理支持MPDP时的拨号命令，简单封装，供AT_PS_ProcDialCmdMpdp调用
 输入参数  : ucIndex 索引值
 输出参数  : ulResult : 返回结果
 返 回 值  : AT_APS_IPV6_RA_INFO_STRU* pIPv6RaInfo
 调用函数  :
 被调函数  : 
*****************************************************************************/
VOS_UINT32 AT_PS_ProcDialCmdEx(VOS_UINT32* ulResult, VOS_UINT8 ucIndex)
{
    VOS_UINT32                                                  ulReturn;
    VOS_UINT8                                                    ucCid;
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId;
    NDIS_RM_NET_ID_ENUM_UINT8                   enNdisRmNetId;
    AT_MODEM_PS_CTX_STRU                          *pstPsModemCtx = VOS_NULL_PTR;
    ucCid = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 检查 CID */
    if (0 == gastAtParaList[0].usParaLen)
    {
        *ulResult =  AT_CME_INCORRECT_PARAMETERS;
        return VOS_FALSE;
    }

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(ucIndex);

    /*拨号CID的PDP若已经激活，则拨号命令直接返回OK */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
      && (VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulRmNetActFlg))
    {
        *ulResult = AT_OK;
        return VOS_FALSE;
    }
    
    enDataChannelId = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    
    /* 获取网卡ID */
    ulReturn = AT_PS_GetNdisRmNetIdFromChDataValue(ucIndex, enDataChannelId, &enNdisRmNetId);
    if (VOS_OK != ulReturn)
    {
        *ulResult = AT_ERROR;
        return VOS_FALSE;
    }
    
    /* 配置数传通道映射表 */
    pstPsModemCtx->astChannelCfg[ucCid].ulUsed     = VOS_TRUE;
    pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId  = enNdisRmNetId;
    pstPsModemCtx->astChannelCfg[ucCid].ulHandle   = g_ulAtUdiNdisMpdpHdl[ucCid - 1];
    return VOS_TRUE;
}
/*****************************************************************************
 函 数 名  : AT_PS_ProcDialCmdMpdp
 功能描述  : 处理支持MPDP时的拨号命令
 输入参数  : ucIndex 索引值
 输出参数  : ulResult : 返回结果
 返 回 值  : AT_APS_IPV6_RA_INFO_STRU* pIPv6RaInfo
 调用函数  :
 被调函数  : 
*****************************************************************************/
VOS_UINT32 AT_PS_ProcDialCmdMpdp(VOS_UINT32* ulResult, VOS_UINT8 ucIndex)
{   
#if ( FEATURE_ON == MBB_FEATURE_GATEWAY )
    if ((0 != gastAtParaList[1].ulParaValue)
        && ((AT_NDIS_USER == gastAtClientTab[ucIndex].UserType) || (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)))
    {
        return AT_PS_ProcDialCmdEx(ulResult, ucIndex);
    }
#else
    if ((0 != gastAtParaList[1].ulParaValue) && (AT_NDIS_USER == gastAtClientTab[ucIndex].UserType))
    {
        return AT_PS_ProcDialCmdEx(ulResult, ucIndex);
    }
#endif   
    return VOS_TRUE;
}


VOS_UINT32 AT_SendNdisIPv6PdnInfoCfgReq(
    MODEM_ID_ENUM_UINT16                 enModemId,
    TAF_PS_IPV6_INFO_IND_STRU           *pIPv6RaNotify,
    UDI_HANDLE                           ulHandle
)
{
    AT_NDIS_PDNINFO_CFG_REQ_STRU        stNdisCfgReq;
    VOS_UINT32                          ulSpeed;
    AT_PDP_ENTITY_STRU                 *pstNdisPdpEntity;

    /* 初始化 */
    pstNdisPdpEntity = AT_NDIS_GetPdpEntInfoAddr();
    MBB_MEM_SET(&stNdisCfgReq, 0x00, sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));

    stNdisCfgReq.ulHandle               = ulHandle;

    stNdisCfgReq.bitOpIpv6PdnInfo       = VOS_TRUE;
    /* Modified by l60609 for DSDA Phase II, 2012-12-27, Begin */
    stNdisCfgReq.enModemId              = enModemId;
    /* Modified by l60609 for DSDA Phase II, 2012-12-27, End */
    stNdisCfgReq.ucRabId                = pIPv6RaNotify->ucRabId;

    /* 填充主副DNS */
    stNdisCfgReq.stIpv6PdnInfo.stDnsSer.ucSerNum    = 0;
    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6PriDns)
    {
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stDnsSer.aucPriServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucIpv6PrimDNS,
                    TAF_IPV6_ADDR_LEN);
        stNdisCfgReq.stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6SecDns)
    {
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stDnsSer.aucSecServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucIpv6SecDNS,
                    TAF_IPV6_ADDR_LEN);
        stNdisCfgReq.stIpv6PdnInfo.stDnsSer.ucSerNum += 1;
    }

    /* 填充MTU */
    if (VOS_TRUE == pIPv6RaNotify->stIpv6RaInfo.bitOpMtu)
    {
        stNdisCfgReq.stIpv6PdnInfo.ulBitOpMtu   = VOS_TRUE;
        stNdisCfgReq.stIpv6PdnInfo.ulMtu        = pIPv6RaNotify->stIpv6RaInfo.ulMtu;
    }

    stNdisCfgReq.stIpv6PdnInfo.ulBitCurHopLimit = pIPv6RaNotify->stIpv6RaInfo.ulBitCurHopLimit;
    stNdisCfgReq.stIpv6PdnInfo.ulBitM           = pIPv6RaNotify->stIpv6RaInfo.ulBitM;
    stNdisCfgReq.stIpv6PdnInfo.ulBitO           = pIPv6RaNotify->stIpv6RaInfo.ulBitO;
    stNdisCfgReq.stIpv6PdnInfo.ulPrefixNum      = pIPv6RaNotify->stIpv6RaInfo.ulPrefixNum;
    MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.astPrefixList,
                pIPv6RaNotify->stIpv6RaInfo.astPrefixList,
                sizeof(TAF_PDP_IPV6_PREFIX_STRU)*TAF_MAX_PREFIX_NUM_IN_RA);

    /* 填写INTERFACE，取IPV6地址的后8字节来填写INTERFACE */
    MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.aucInterfaceId,
                pstNdisPdpEntity->stIpv6Dhcp.aucIpv6Addr,
                sizeof(VOS_UINT8)*AT_NDIS_IPV6_IFID_LENGTH);

    /* 填充主副PCSCF地址  */
    stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.ucSerNum      = 0;
    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6PriPCSCF)
    {
        stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.ucSerNum += 1;
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.aucPriServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucPrimPcscfAddr,
                    sizeof(pstNdisPdpEntity->stIpv6Dhcp.aucPrimPcscfAddr));
    }

    if (VOS_TRUE == pstNdisPdpEntity->stIpv6Dhcp.bitOpIpv6SecPCSCF)
    {
        stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.ucSerNum += 1;
        MBB_MEM_CPY(stNdisCfgReq.stIpv6PdnInfo.stPcscfSer.aucSecServer,
                    pstNdisPdpEntity->stIpv6Dhcp.aucSecPcscfAddr,
                    sizeof(pstNdisPdpEntity->stIpv6Dhcp.aucSecPcscfAddr));
    }

    /* 获取接入理论带宽*/
    if (VOS_OK != AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &ulSpeed))
    {
        AT_ERR_LOG("AT_SendNdisIPv6PdnInfoCfgReq : ERROR : AT_GetDisplayRate Error!");
        ulSpeed = AT_DEF_DISPLAY_SPEED;
    }

    stNdisCfgReq.ulMaxRxbps                 = ulSpeed;
    stNdisCfgReq.ulMaxTxbps                 = ulSpeed;

    /* 发送消息 */
    if (ERR_MSP_SUCCESS != AT_FwSendClientMsg(PS_PID_APP_NDIS,
                                            ID_AT_NDIS_PDNINFO_CFG_REQ,
                                            (VOS_UINT16)sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU),
                                            (VOS_UINT8*)&stNdisCfgReq))
    {
        AT_ERR_LOG("AT_SendNdisIPv6PdnInfoCfgReq: Send client msg fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SendNdisIPv4PdnInfoCfgReqEx(MN_CLIENT_ID_T usClientId,
    AT_IPV4_DHCP_PARAM_STRU            *pstIPv4DhcpParam,
    UDI_HANDLE                          ulHandle)
{
    AT_NDIS_PDNINFO_CFG_REQ_STRU        stNdisCfgReq;
    VOS_UINT32                          ulSpeed;
    VOS_UINT32                          ulRelt;

    /* 初始化 */
    MBB_MEM_SET(&stNdisCfgReq, 0x00, sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU));
    stNdisCfgReq.enModemId              = MODEM_ID_0;

    ulRelt = AT_GetModemIdFromClient((AT_CLIENT_TAB_INDEX_UINT8)usClientId, &stNdisCfgReq.enModemId);

    if (VOS_OK != ulRelt)
    {
        AT_ERR_LOG("AT_SendNdisIPv4PdnInfoCfgReq:Get Modem Id fail");
        return VOS_ERR;
    }

    stNdisCfgReq.bitOpIpv4PdnInfo       = VOS_TRUE;
    stNdisCfgReq.ulHandle               = ulHandle;
    /* 构造消息 */
    if (0 != pstIPv4DhcpParam->ucRabId )
    {
        stNdisCfgReq.ucRabId = pstIPv4DhcpParam->ucRabId;
    }

    /* 填写IPv4地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4Addr)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpPdnAddr     = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stPDNAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4Addr);
    }

    /* 填写掩码地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4NetMask)
    {
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stSubnetMask.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4NetMask);
    }

    /* 填写网关地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4GateWay)
    {
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stGateWayAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4GateWay);
    }

    /* 填写主DNS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4PrimDNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpDnsPrim     = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stDnsPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimDNS);

    }

    /* 填写辅DNS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4SecDNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpDnsSec      = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stDnsSecAddrInfo.aucIpV4Addr,
                          pstIPv4DhcpParam->ulIpv4SecDNS);

    }

    /* 填写主WINS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4PrimWINNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpWinsPrim    = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stWinsPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimWINNS);
    }

    /* 填写辅WINS地址 */
    if (0 != pstIPv4DhcpParam->ulIpv4SecWINNS)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpWinsSec     = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stWinsSecAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4SecWINNS);
    }

    /* 填写主PCSCF地址 */
    if (VOS_TRUE == pstIPv4DhcpParam->bitOpIpv4PriPCSCF)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpPcscfPrim   = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stPcscfPrimAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4PrimPCSCF);
    }

    /* 填写副PCSCF地址 */
    if (VOS_TRUE == pstIPv4DhcpParam->bitOpIpv4Secpcscf)
    {
        stNdisCfgReq.stIpv4PdnInfo.bitOpPcscfSec    = VOS_TRUE;
        (VOS_VOID)AT_PutNetworkAddr32(stNdisCfgReq.stIpv4PdnInfo.stPcscfSecAddrInfo.aucIpV4Addr,
                            pstIPv4DhcpParam->ulIpv4SecPCSCF);
    }

    /* 获取接入理论带宽*/
    if (VOS_OK != AT_GetDisplayRate(AT_CLIENT_ID_NDIS, &ulSpeed))
    {
        AT_ERR_LOG("AT_SendNdisIPv4PdnInfoCfgReq : ERROR : AT_GetDisplayRate Error!");
        ulSpeed = AT_DEF_DISPLAY_SPEED;
    }

    stNdisCfgReq.ulMaxRxbps                 = ulSpeed;
    stNdisCfgReq.ulMaxTxbps                 = ulSpeed;

    /* 发送消息 */
    if (ERR_MSP_SUCCESS != AT_FwSendClientMsg(PS_PID_APP_NDIS,
                                            ID_AT_NDIS_PDNINFO_CFG_REQ,
                                            (VOS_UINT16)sizeof(AT_NDIS_PDNINFO_CFG_REQ_STRU),
                                            (VOS_UINT8*)&stNdisCfgReq))
    {
        AT_ERR_LOG("AT_SendNdisIPv4PdnInfoCfgReq: Send client msg fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID AT_PS_UpdateDnsInfo(AT_DIAL_PARAM_STRU* stUsrDialParam)
{
    if(VOS_TRUE == gstAtNdisAddParam.ulPrimIPv4DNSValidFlag)
    {
        MBB_MEM_CPY(stUsrDialParam->aucPrimIPv4DNSAddr,
                    gstAtNdisAddParam.aucPrimIPv4DNSAddr,
                    TAF_MAX_IPV4_ADDR_STR_LEN);
        stUsrDialParam->ulPrimIPv4DNSValidFlag = gstAtNdisAddParam.ulPrimIPv4DNSValidFlag;
    }
    if(VOS_TRUE == gstAtNdisAddParam.ulSndIPv4DNSValidFlag)
    {
        MBB_MEM_CPY(stUsrDialParam->aucSndIPv4DNSAddr,
                    gstAtNdisAddParam.aucSndIPv4DNSAddr,
                    TAF_MAX_IPV4_ADDR_STR_LEN);
        stUsrDialParam->ulSndIPv4DNSValidFlag = gstAtNdisAddParam.ulSndIPv4DNSValidFlag;
    }
}

VOS_VOID AT_PS_UpdateUserDialDnsInfo(VOS_UINT8 ucIndex, AT_DIAL_PARAM_STRU* stUsrDialParam)
{
#if ( FEATURE_ON == MBB_FEATURE_GATEWAY )
    if ((AT_PS_USER_CID_1 == gastAtParaList[0].ulParaValue)
        && ((AT_NDIS_USER == gastAtClientTab[ucIndex].UserType) || (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)))
    {
        AT_PS_UpdateDnsInfo(stUsrDialParam);
    }
#else
    if ((AT_PS_USER_CID_1 == gastAtParaList[0].ulParaValue) && (AT_NDIS_USER == gastAtClientTab[ucIndex].UserType))
    {
        AT_PS_UpdateDnsInfo(stUsrDialParam);
    }
#endif
}

VOS_VOID AT_OpenUsbNdisMpdp(UDI_OPEN_PARAM* stParam)
{
    VOS_UINT32                          Index = 0;
    UDI_DEVICE_ID ExNcmDevIdArray[MAX_NDIS_NET] = {
                                     UDI_NCM_NDIS_ID, UDI_NCM_NDIS1_ID, UDI_NCM_NDIS2_ID, UDI_NCM_NDIS3_ID, 
                                     UDI_NCM_NDIS4_ID, UDI_NCM_NDIS5_ID, UDI_NCM_NDIS6_ID, UDI_NCM_NDIS7_ID,};
    if (VOS_OK != DRV_UDI_IOCTL(g_ulAtUdiNdisHdl, NCM_IOCTL_GET_MBIM_FLAG, (VOS_VOID*)&gMbimFeatureFlag))
    {
        gMbimFeatureFlag = FALSE;
    }  

        /* g_ulAtUdiNdisMpdpHdl[0] 等于 g_ulAtUdiNdisHdl */
    g_ulAtUdiNdisMpdpHdl[0] = g_ulAtUdiNdisHdl;

    for (Index = 1; Index <= g_MpdpNum; Index++)
    {
        stParam->devid = ExNcmDevIdArray[Index];
        g_ulAtUdiNdisMpdpHdl[Index] = DRV_UDI_OPEN(stParam);

        if (UDI_INVALID_HANDLE == g_ulAtUdiNdisMpdpHdl[Index]) 
        {
            break;
        }
    }
}

VOS_VOID AT_CloseUsbNdisMpdp(VOS_VOID)
{
    VOS_UINT32  Index = 0;

    /* g_ulAtUdiNdisMpdpHdl[0] 等于 g_ulAtUdiNdisHdl */
    g_ulAtUdiNdisMpdpHdl[0] = UDI_INVALID_HANDLE; 
    for (Index = 1; Index < g_MpdpNum; Index++)
    {
 
        if ((UDI_INVALID_HANDLE != g_ulAtUdiNdisMpdpHdl[Index]) && (0 != g_ulAtUdiNdisMpdpHdl[Index]))
        {
            (VOS_VOID)DRV_UDI_CLOSE(g_ulAtUdiNdisMpdpHdl[Index]);
            g_ulAtUdiNdisMpdpHdl[Index] = UDI_INVALID_HANDLE;
        }
    }
    return;
}

VOS_INT AT_UsbCtrlBrkReqCBMpdp(VOS_VOID)
{
    VOS_UINT8  ucCid;
    VOS_UINT8  ucCallId;

    for( ucCid = 1; ucCid <= TAF_MAX_CID; ucCid++)
    {
        /*获取ucCid对应的ucCallId,由于MPDP目前只支持ndis口拨号，所以Index使用NDIS的*/
        ucCallId = AT_PS_TransCidToCallId(AT_CLIENT_TAB_NDIS_INDEX, ucCid);
        if (VOS_FALSE == AT_PS_IsCallIdValid(AT_CLIENT_TAB_NDIS_INDEX, ucCallId))
        {
            AT_ERR_LOG("AT_PS_ProcCallEndedEvent:ERROR: CallId is invalid!");
            continue;
        }
        else
        {
            /*调用拨号断开的函数*/
            (VOS_VOID)AT_PS_HangupCall(AT_CLIENT_TAB_NDIS_INDEX, ucCallId);
        }
    }
    return 0;
} 


#if (FEATURE_ON == MBB_FEATURE_UNI_PS_CALL)

VOS_UINT8 AT_PS_FindDialCid(VOS_UINT16 ucIndex, const AT_DIAL_PARAM_STRU *pstDialParam)
{
    VOS_UINT8                           ucCid;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucIpv4Cid;
    VOS_UINT8                           ucIpv6Cid;
    VOS_UINT32                          IsDialGoingUp;
    VOS_UINT32                          IsDialGoingDwon;
    
    ucCid = pstDialParam->ucCid;
    
    for ( ucCallId = 0; ucCallId < AT_PS_MAX_CALL_NUM; ucCallId += 1 )
    {
        IsDialGoingUp = AT_PS_IsLinkGoingUp(ucIndex, ucCallId);
        IsDialGoingDwon = AT_PS_IsLinkGoingDown(ucIndex, ucCallId);
        
        if (( VOS_TRUE == IsDialGoingUp ) || ( VOS_TRUE == IsDialGoingDwon ))
        {
            ucIpv4Cid = AT_PS_GetCidByCallType(ucIndex, ucCallId, TAF_PDP_IPV4);
            ucIpv6Cid = AT_PS_GetCidByCallType(ucIndex, ucCallId, TAF_PDP_IPV6);
            
            if ( (ucIpv4Cid == pstDialParam->ucCid) || (ucIpv6Cid == pstDialParam->ucCid) )
            {
                /*CID已经被占用*/
                ucCid = AT_PS_CALL_INVALID_CID;
                break;
            }
        }
                
    }
    
    /* CID未被占用，直接返回*/
    if ( AT_PS_CALL_INVALID_CID != ucCid )
    {
        return ucCid;
    }
    
    /*CID被占用，查找可用的拨号CID*/
    if (VOS_OK != TAF_AGENT_FindCidForDial(ucIndex, &ucCid))
    {
        return AT_PS_CALL_INVALID_CID;
    }
    
    return ucCid;
}
#endif 


VOS_UINT32 AT_DeRegNdisFCPointEx(
    VOS_UINT8                           ucRabId,
    MODEM_ID_ENUM_UINT16                enModemId,
    FC_ID_ENUM_UINT32                   enFcId
)
{
    VOS_UINT32                          ulRet;

    /* 在调用FC_DeRegPoint前,先调用FC_ChannelMapDelete */
    FC_ChannelMapDelete(ucRabId, enModemId);
    
    ulRet = FC_DeRegPoint(enFcId, enModemId);
    if (VOS_OK != ulRet)
    {
        AT_ERR_LOG("AT_DeRegNdisFCPoint: ERROR: de reg point Failed.");
        return VOS_ERR;
    }

    /* 清除FCID与FC Pri的映射关系 */
    g_stFcIdMaptoFcPri[enFcId].ulUsed      = VOS_FALSE;
    g_stFcIdMaptoFcPri[enFcId].ulFcPri     = FC_PRI_BUTT;
    /* 有一张网卡上多个RABID的情况，所以需要将对应的RABID掩码清除掉 */
    g_stFcIdMaptoFcPri[enFcId].ulRabIdMask &= ~((VOS_UINT32)1 << ucRabId);
    g_stFcIdMaptoFcPri[enFcId].enModemId   = MODEM_ID_BUTT;

    /* 勾流控消息 */
    AT_MNTN_TraceDeregFcPoint(AT_CLIENT_TAB_NDIS_INDEX, AT_FC_POINT_TYPE_NDIS);

    return VOS_OK;
} 


FC_ID_ENUM_UINT32 AT_PS_GetFcIdFromNdisByRmNetId(VOS_UINT32 ulRmNetId)
{
    switch (ulRmNetId)
    {
        case NDIS_RM_NET_ID_0:
            return FC_ID_NIC_1;

        case NDIS_RM_NET_ID_1:
            return FC_ID_NIC_2;

        case NDIS_RM_NET_ID_2:
            return FC_ID_NIC_3;

        case NDIS_RM_NET_ID_3:
            return FC_ID_NIC_4;

        case NDIS_RM_NET_ID_4:
            return FC_ID_NIC_5;

        case NDIS_RM_NET_ID_5:
            return FC_ID_NIC_6;
            
        case NDIS_RM_NET_ID_6:
            return FC_ID_NIC_7;

        case NDIS_RM_NET_ID_7:
            return FC_ID_NIC_8;

        default:
            AT_WARN_LOG("AT_PS_GetFcIdFromRnidRmNetId: WARNING: data channel id is abnormal.");
            return FC_ID_BUTT;
    }
}

FC_ID_ENUM_UINT32 AT_GetDefaultFcID(AT_MODEM_PS_CTX_STRU *pstModemPsCtx, VOS_UINT8 ucCallId, VOS_UINT32 ulRmNetId)
{
    AT_USER_TYPE                     ucUserType;
    FC_ID_ENUM_UINT32                   enDefaultFcId;
    /*获取对应的用户类型*/
    ucUserType = pstModemPsCtx->astCallEntity[ucCallId].stUserInfo.ucUsrType;

    enDefaultFcId = FC_ID_NIC_1;
    /* 获取网卡ID对应的FC ID */
    if(AT_NDIS_USER == ucUserType) 
    {
        enDefaultFcId = AT_PS_GetFcIdFromNdisByRmNetId(ulRmNetId);
    }
    else if(AT_APP_USER == ucUserType)
    {
        enDefaultFcId = AT_PS_GetFcIdFromRnicByRmNetId(ulRmNetId);
    }
    return enDefaultFcId;
}

VOS_UINT32 AT_PS_GetNdisRmNetIdFromChDataValue(VOS_UINT8  ucIndex,
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId,
    RNIC_RM_NET_ID_ENUM_UINT8          *penNdisRmNetId)
{
    VOS_INT32                          i;
    MODEM_ID_ENUM_UINT16                enModemId;
    AT_CHDATA_NDIS_RMNET_ID_STRU       *pstChdataNdisRmNetIdTab;
    VOS_UINT32                          ulRslt;
    /*获取NDIS RNNET ID的表*/
    pstChdataNdisRmNetIdTab = g_astAtChdataNdisRmNetIdTab;

    *penNdisRmNetId = NDIS_RM_NET_ID_0;

    ulRslt = AT_GetModemIdFromClient(ucIndex, &enModemId);

    if (VOS_OK != ulRslt)
    {
        return VOS_ERR;
    }

    /*信道超出范围，直接返回ERROR*/
    if ((enDataChannelId < AT_CH_DATA_CHANNEL_ID_1)
    || (enDataChannelId > AT_CH_DATA_CHANNEL_ID_7))
    {
        return VOS_ERR;
    }

    /*  以上判断已能保证enDataChannelId的有效性，所以RM NET ID一定能在表中找到 */
    for (i = 0; i < ARRAY_SIZE(g_astAtChdataNdisRmNetIdTab); i++)
    {
        if (enDataChannelId == pstChdataNdisRmNetIdTab[i].enChdataValue)
        {
            *penNdisRmNetId = pstChdataNdisRmNetIdTab[i].enNdisRmNetId;
            break;
        }
    }

    return VOS_OK;
}


VOS_VOID AT_ReadWinblueProfileType(VOS_VOID)
{
    NAS_WINBLUE_PROFILE_TYPE_STRU stWinblueProfileType;
    VOS_UINT8  ucRslt;

/*lint -e160 -e522 -e506*/
    MBB_MEM_SET(&stWinblueProfileType, 0x00, sizeof(NAS_WINBLUE_PROFILE_TYPE_STRU));
/*lint +e160 +e522 +e506*/
    /*目前协议栈用到NV50424的参数为ucMaxPdpSession,后续可能用到的是IPV4 MTU和IPV6 MTU*/
    ucRslt = NV_ReadEx(MODEM_ID_0, NV_ID_WINBLUE_PROFILE, &stWinblueProfileType,
                              sizeof(NAS_WINBLUE_PROFILE_TYPE_STRU));
    if((NV_OK == ucRslt) && (VOS_TRUE == stWinblueProfileType.MBIMEnable))
    {
        g_MpdpNum = stWinblueProfileType.MaxPDPSession;
    }
    else
    {
        g_MpdpNum = MAX_NDIS_NET;
        AT_WARN_LOG("NV read fail!");
    }
    return;

}


VOS_VOID AT_PS_SndNdisPdpActInd(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU        *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT8                                     callId;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);
    
    callId = AT_PS_TransCidToCallId(pstEvent->stCtrl.usClientId, ucCid);

    /*检查callId有效性*/
    if (VOS_FALSE == AT_PS_IsCallIdValid(pstEvent->stCtrl.usClientId, callId))
    {
        return;
    }

    /* 判断网卡的有效性 */
    if (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId >= NDIS_RM_NET_ID_BUTT)
    {
        return;
    }

    /*激活网卡*/
    (VOS_VOID)AT_ActiveUsbNet();

    /* 向NDIS发送PDP IPv4已经激活事件 */
    if (TAF_PDP_IPV4 == (enPdpType & TAF_PDP_IPV4))
    {
        /* 把IPV4的PDN信息发送给NDIS模块 */
        if (AT_PS_MAX_CALL_NUM <= callId  || TAF_MAX_CID < ucCid )
        {
            return ;
        }
        
        (VOS_VOID)AT_SendNdisIPv4PdnInfoCfgReqEx(pstEvent->stCtrl.usClientId,
                                     &(pstPsModemCtx->astCallEntity[callId].stIpv4DhcpInfo),
                                     pstPsModemCtx->astChannelCfg[ucCid].ulHandle);
    }

    return;

}

VOS_VOID AT_PS_SndNdisPdpDeactInd(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 判断网卡的有效性 */
    if (pstPsModemCtx->astChannelCfg[ucCid].ulHandle == UDI_INVALID_HANDLE)
    {
        return;
    }
    /*向NDIS发送释放PDN消息*/
    (VOS_VOID)AT_SendNdisRelReq(pstEvent);

    /*去激活网卡*/
    (VOS_VOID)AT_DeActiveUsbNet();
    return;
}

VOS_VOID AT_PS_RegNdisFCPoint(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_ACTIVATE_CNF_STRU   *pstEvent
)
{
    VOS_UINT32                          ulRslt;
    AT_FCID_MAP_STRU                    stFCPriOrg;
    FC_ID_ENUM_UINT32                   enDefaultFcId;
    VOS_UINT32                          ulRmNetId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 寻找配套的通道ID */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
     && (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId < NDIS_RM_NET_ID_BUTT))
    {
        ulRmNetId = pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId;
    }
    else
    {
        AT_ERR_LOG("AT_PS_RegNdisFCPoint: data channel id is abnormal.\n");
        return;
    }

    /* 上述分支已能保证网卡的有效性 */
    enDefaultFcId = AT_PS_GetFcIdFromNdisByRmNetId(ulRmNetId);

    ulRslt = AT_GetFcPriFromMap(enDefaultFcId ,&stFCPriOrg);
    if (VOS_OK == ulRslt)
    {
        /* 如果FC ID未注册，那么注册该流控点 */
        if (VOS_TRUE != stFCPriOrg.ulUsed)
        {
            /* 注册NDIS拨号使用的流控点 */
            (VOS_VOID)AT_RegNdisFCPoint(pstEvent, enDefaultFcId, MODEM_ID_0);
        }
        else
        {
            AT_NORM_LOG("AT_PS_RegNidsFCPoint: No need to change the default QOS priority.");
        }
    }
    return;
}


VOS_VOID AT_PS_DeRegNdisFCPoint(
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_PDP_DEACTIVATE_CNF_STRU *pstEvent
)
{
    FC_ID_ENUM_UINT32                   enDefaultFcId;
    VOS_UINT32                          ulRmNetId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(pstEvent->stCtrl.usClientId);

    /* 寻找配套的通道ID */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
     && (pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId < NDIS_RM_NET_ID_BUTT))
    {
        ulRmNetId = (VOS_UINT8)pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId;
    }
    else
    {
        AT_ERR_LOG("AT_PS_DeRegAppFCPoint: data channel id is abnormal.\n");
        return;
    }

    /* 上述分支已能保证网卡的有效性 */
    enDefaultFcId = AT_PS_GetFcIdFromNdisByRmNetId(ulRmNetId);

    /* 去注册NDIS拨号使用的流控点 */
    (VOS_VOID)AT_DeRegNdisFCPointEx(pstEvent->ucRabId,MODEM_ID_0,enDefaultFcId);

    return;

}
#endif/*FEATURE_ON == MBB_FEATURE_MPDP*/

VOS_VOID AT_PS_ReportNDISSTAT(
    VOS_UINT8                           ucCid,
    AT_PDP_STATUS_ENUM_UINT32           enStat,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             ucPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucAtStrIpv4[] = "IPV4";
#if(FEATURE_ON == FEATURE_IPV6)
    VOS_UINT8                           aucAtStrIpv6[] = "IPV6";
#endif

    VOS_UINT16                          us3gppSmCause;

    usLength = 0;

    if (AT_PDP_STATUS_DEACT == enStat)
    {
        us3gppSmCause = AT_Get3gppSmCauseByPsCause(enCause);

        switch (ucPdpType)
        {
            case TAF_PDP_IPV4:
#ifdef BSP_CONFIG_BOARD_K5160
                if ( AT_USBCOM_USER == g_stAtNdisUserType )
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                }
                else
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);
                }
#else
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);
#endif/*BSP_CONFIG_BOARD_K5160*/
                break;

#if(FEATURE_ON == FEATURE_IPV6)
            case TAF_PDP_IPV6:
#ifdef BSP_CONFIG_BOARD_K5160
                if ( AT_USBCOM_USER == g_stAtNdisUserType )
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                }
                else
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
                }
#else
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
#endif/*BSP_CONFIG_BOARD_K5160*/
                break;

            case TAF_PDP_IPV4V6:
#ifdef BSP_CONFIG_BOARD_K5160
                if ( AT_USBCOM_USER == g_stAtNdisUserType )
                {
                    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                
                    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                    usLength  = 0;
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                }
                else
                {
                    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);

                   At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                   usLength  = 0;

                   usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
                }
#else

                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);

                At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                usLength  = 0;

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
#endif/*BSP_CONFIG_BOARD_K5160*/
                break;
#endif

            default:
                AT_ERR_LOG("AT_NdisStateChangeProc:ERROR: Wrong PDP type!");
                return;
        }
    }
    else
    {
        switch (ucPdpType)
        {
            case TAF_PDP_IPV4:
#ifdef BSP_CONFIG_BOARD_K5160
                if ( AT_USBCOM_USER == g_stAtNdisUserType )
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                }
                else
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);
                }
#else
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);
#endif/*BSP_CONFIG_BOARD_K5160*/
                break;

#if(FEATURE_ON == FEATURE_IPV6)
            case TAF_PDP_IPV6:
#ifdef BSP_CONFIG_BOARD_K5160
                if ( AT_USBCOM_USER == g_stAtNdisUserType )
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                }
                else
                {
                    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
                }
#else
                usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
#endif/*BSP_CONFIG_BOARD_K5160*/
                break;

            case TAF_PDP_IPV4V6:
#ifdef BSP_CONFIG_BOARD_K5160
                if ( AT_USBCOM_USER == g_stAtNdisUserType )
                {
                    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                
                    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                    usLength  = 0;
                    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\",%d%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            g_stAtNdisCid,
                                            gaucAtCrLf);
                }
                else
                {
                    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);

                   At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                   usLength  = 0;

                   usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,%d,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            us3gppSmCause,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
                }
#else
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv4,
                                            gaucAtCrLf);

                At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

                usLength  = 0;

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%s^NDISSTAT:%d,,,\"%s\"%s",
                                            gaucAtCrLf,
                                            enStat,
                                            aucAtStrIpv6,
                                            gaucAtCrLf);
#endif/*BSP_CONFIG_BOARD_K5160*/
                break;
#endif

            default:
                AT_ERR_LOG("AT_NdisStateChangeProc:ERROR: Wrong PDP type!");
                return;
        }
    }

    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}


VOS_VOID AT_PS_ReportDconnNDISSTATEX(
    VOS_UINT8                           ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucAtStrIpv4[] = "IPV4";
#if(FEATURE_ON == FEATURE_IPV6)
    VOS_UINT8                           aucAtStrIpv6[] = "IPV6";
#endif
    usLength = 0;
#if (FEATURE_ON == MBB_FEATURE_MPDP) 
    if(((AT_CLIENT_TAB_NDIS_INDEX == ucPortIndex) || (AT_CLIENT_TAB_PCUI_INDEX == ucPortIndex)) && (FALSE == IS_MBIM_OS()))
    {
        AT_PS_ReportNDISSTAT(ucCid, AT_PDP_STATUS_ACT, ucPortIndex, enPdpType, TAF_PS_CAUSE_SUCCESS);
        return;
    }
#endif
    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);
            break;

#if (FEATURE_ON == FEATURE_IPV6)
        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,1,,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);
            break;
#endif

        default:
            AT_ERR_LOG("AT_PS_ReportDCONN: PDP type is invalid in ^NDISSTATEX.");
            return;
    }

    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}

VOS_VOID AT_PS_ReportDendNDISSTATEX(
    VOS_UINT8                           ucCid,
    VOS_UINT8                           ucPortIndex,
    TAF_PDP_TYPE_ENUM_UINT8             enPdpType,
    TAF_PS_CAUSE_ENUM_UINT32            enCause
)
{
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucAtStrIpv4[] = "IPV4";
#if(FEATURE_ON == FEATURE_IPV6)
    VOS_UINT8                           aucAtStrIpv6[] = "IPV6";
#endif
    VOS_UINT16                          us3gppSmCause;
    /*得到3gpp的原因值*/
    us3gppSmCause = AT_Get3gppSmCauseByPsCause(enCause);

    usLength = 0;
#if (FEATURE_ON == MBB_FEATURE_MPDP) 
    if(((AT_CLIENT_TAB_NDIS_INDEX == ucPortIndex) || (AT_CLIENT_TAB_PCUI_INDEX == ucPortIndex)) && (FALSE == IS_MBIM_OS()))
    {
        AT_PS_ReportNDISSTAT(ucCid, AT_PDP_STATUS_DEACT, ucPortIndex, enPdpType, enCause);
        return;
    }
#endif

    switch (enPdpType)
    {
        case TAF_PDP_IPV4:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);

            break;
#if (FEATURE_ON == FEATURE_IPV6)
        case TAF_PDP_IPV6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);
            break;

        case TAF_PDP_IPV4V6:
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv4,
                                               gaucAtCrLf);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%s^NDISSTATEX:%d,0,%d,,\"%s\"%s",
                                               gaucAtCrLf,
                                               ucCid,
                                               us3gppSmCause,
                                               aucAtStrIpv6,
                                               gaucAtCrLf);
            break;
#endif

        default:
            AT_ERR_LOG("AT_PS_ReportDendNDISSTAT: PDP type is invalid in ^NDISSTATEX.");
            return;
    }
#if (FEATURE_ON == MBB_MLOG)
    mlog_print("at", mlog_lv_info, "Disconnect NDISSTATEX: %d, %d, %d",  ucCid, us3gppSmCause,enPdpType);
#endif
    At_SendResultData(ucPortIndex, pgucAtSndCodeAddr, usLength);

    return;
}

VOS_UINT32 AT_QryNdisStatParaEx( VOS_UINT8 ucIndex )
{
    AT_PS_CALL_ENTITY_STRU                  *pstCallEntity;
    VOS_UINT8                               ucCallId;
    VOS_UINT8                               ucRslt;
    VOS_UINT16                              usLength = 0;

    VOS_UINT8                               aucAtStrIpv4[] = "IPV4";
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;
#if (FEATURE_ON == FEATURE_IPV6)
    VOS_UINT8                               aucAtStrIpv6[] = "IPV6";
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;
#endif /*FEATURE_ON == FEATURE_IPV6*/

#ifdef BSP_CONFIG_BOARD_K5160
    ucCallId = AT_PS_TransCidToCallId( ucIndex, g_stAtNdisCid );
#else
    ucCallId = AT_PS_TransCidToCallId( ucIndex, AT_PS_USER_CID_1 );
#endif/*BSP_CONFIG_BOARD_K5160*/

    ucRslt = AT_PS_IsCallIdValid( ucIndex, ucCallId );
    if ( VOS_TRUE == ucRslt )
    {
        pstCallEntity = AT_PS_GetCallEntity( ucIndex, ucCallId );
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
#if (FEATURE_ON == FEATURE_IPV6)
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
#endif /*FEATURE_ON == FEATURE_IPV6*/
    }

#if ( FEATURE_ON == FEATURE_IPV6 )
    switch ( AT_GetIpv6Capability() )
    {
        case AT_IPV6_CAPABILITY_IPV4_ONLY:
            usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%s: ",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName );
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",
                                                enIpv4Status,
                                                aucAtStrIpv4 );
            break;

        case AT_IPV6_CAPABILITY_IPV6_ONLY:
            usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%s: ",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName );

#ifdef BSP_CONFIG_BOARD_K5160
            if ( AT_USBCOM_USER == g_stAtNdisUserType )
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%d,,,\"%s\",%d",
                                               enIpv4Status,
                                               aucAtStrIpv6,
                                               g_stAtNdisCid);
            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%d,,,\"%s\"",
                                               enIpv4Status,
                                               aucAtStrIpv6);
            }
#else
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",
                                                enIpv6Status,
                                                aucAtStrIpv6 );
#endif/*BSP_CONFIG_BOARD_K5160*/
            break;

        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
             usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%s: ",
                                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName );
#ifdef BSP_CONFIG_BOARD_K5160
            if ( AT_USBCOM_USER == g_stAtNdisUserType )
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            "%d,,,\"%s\"",
                                            enIpv4Status,
                                            aucAtStrIpv4);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                            ",%d,,,\"%s\",%d",
                                            enIpv6Status,
                                            aucAtStrIpv6,
                                            g_stAtNdisCid);

            }
            else
            {
                usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",
                                                enIpv4Status,
                                                aucAtStrIpv4 );
                usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                ",%d,,,\"%s\"",
                                                enIpv6Status,
                                                aucAtStrIpv6 );
            }
#else
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                "%d,,,\"%s\"",
                                                enIpv4Status,
                                                aucAtStrIpv4 );
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                ",%d,,,\"%s\"",
                                                enIpv6Status,
                                                aucAtStrIpv6 );
#endif/*BSP_CONFIG_BOARD_K5160*/
            break;

        default:
#ifdef BSP_CONFIG_BOARD_K5160

            usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%s: ",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            if ( AT_USBCOM_USER == g_stAtNdisUserType ) 
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%d,,,\"%s\",%d",
                                               enIpv4Status,
                                               aucAtStrIpv4,
                                               g_stAtNdisCid);    
            }
#endif/*BSP_CONFIG_BOARD_K5160*/
            break;
    }
#else /*FEATURE_ON == FEATURE_IPV6*/
    usLength  = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        "%s: ",
                                        g_stParseContext[ucIndex].pstCmdElement->pszCmdName );
#ifdef BSP_CONFIG_BOARD_K5160
    if ( AT_USBCOM_USER == g_stAtNdisUserType ) 
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\",%d",
                                       enIpv4Status,
                                       aucAtStrIpv4,
                                       g_stAtNdisCid);    
    }
    else
    {
       usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",
                                       enIpv4Status,
                                       aucAtStrIpv4);
    }
#else
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                        (VOS_CHAR*)pgucAtSndCodeAddr,
                                        (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                        "%d,,,\"%s\"",
                                        enIpv4Status,
                                        aucAtStrIpv4 );
#endif/*BSP_CONFIG_BOARD_K5160*/

#endif /*FEATURE_ON == FEATURE_IPV6*/
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetNdisStatPara(VOS_UINT8 ucIndex)
{
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity;
    VOS_UINT8                               ucCallId;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucRslt;
    VOS_UINT8                               aucAtStrIpv4[] = "IPV4";
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;

#if (FEATURE_ON == FEATURE_IPV6)
    VOS_UINT8                               aucAtStrIpv6[] = "IPV6";
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;
#endif
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数错误 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    ucCallId = AT_PS_TransCidToCallId(ucIndex, (VOS_UINT8)gastAtParaList[0].ulParaValue);
    ucRslt = AT_PS_IsCallIdValid(ucIndex, ucCallId);
    if (VOS_TRUE == ucRslt)
    {
        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
#if (FEATURE_ON == FEATURE_IPV6)
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
#endif
    }
    
    usLength = 0;
 
#if (FEATURE_ON == FEATURE_IPV6)
    switch(AT_GetIpv6Capability())
    {
        case AT_IPV6_CAPABILITY_IPV4_ONLY:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",
                                       enIpv4Status,
                                       aucAtStrIpv4);

            break;
        case AT_IPV6_CAPABILITY_IPV6_ONLY:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",
                                       enIpv6Status,
                                       aucAtStrIpv6);
            break;
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",
                                       enIpv4Status,
                                       aucAtStrIpv4);                
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",
                                       enIpv6Status,
                                       aucAtStrIpv6);
            break;
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d,,,\"%s\"",
                                       enIpv4Status,
                                       aucAtStrIpv4);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       ",%d,,,\"%s\"",
                                       enIpv6Status,
                                       aucAtStrIpv6);
            break;
        default:
            break;
    }

#else
    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR*)pgucAtSndCodeAddr,
                           (VOS_CHAR*)pgucAtSndCodeAddr,
                           "%s: ",
                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                           (VOS_CHAR*)pgucAtSndCodeAddr,
                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                           "%d,,,\"%s\"",
                           enIpv4Status,
                           aucAtStrIpv4);
#endif
    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}

/*****************************************************************************
 函 数 名  : At_TestNdisstatqry
 功能描述  : ^Ndisstatqry的测试命令，返回当前拨上号的cid
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
*****************************************************************************/
VOS_UINT32 At_TestNdisstatqry(VOS_UINT8 ucIndex)
{
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucCallId;
    AT_PS_CALL_ENTITY_STRU                 *pstCallEntity;
    VOS_UINT32                              ulTmp;
    VOS_UINT8                               ucRslt;
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status = AT_PDP_STATUS_DEACT;
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status = AT_PDP_STATUS_DEACT;
    VOS_UINT32                              ulNum;
    
    usLength    = 0;
    ulNum = 0;
    /* 参数检查 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                    (TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", "(");

    for(ulTmp = 1;ulTmp < AT_PS_MAX_CALL_NUM; ulTmp++)
    {
    
        ucCallId = 0;
        enIpv4Status = AT_PDP_STATUS_DEACT;
        enIpv6Status = AT_PDP_STATUS_DEACT;
        
        ucCallId = AT_PS_TransCidToCallId(ucIndex, ulTmp);
        ucRslt = AT_PS_IsCallIdValid(ucIndex, ucCallId);
        if (VOS_TRUE != ucRslt)
        {
            continue;
        }

        pstCallEntity = AT_PS_GetCallEntity(ucIndex, ucCallId);
        enIpv4Status = AT_NdisGetConnStatus( pstCallEntity->enIpv4State );
#if (FEATURE_ON == FEATURE_IPV6)
        enIpv6Status = AT_NdisGetConnStatus( pstCallEntity->enIpv6State );
#endif
        if((AT_PDP_STATUS_ACT == enIpv4Status) || (AT_PDP_STATUS_ACT == enIpv6Status))
        {
        
            if (0 == ulNum )
            {   /*如果是第一个CID，则CID前不打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%d",ulTmp);
                ulNum++; 
            }
            else
            {   /*如果不是第一个CID，则CID前打印逗号*/
                usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,",%d",ulTmp);
            }

        }
    }
    
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                    (TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,"%s", ")");
    gstAtSendData.usBufLen  = usLength;
    
    return AT_OK;
    
}



VOS_UINT32 At_CheckDlckPara(VOS_VOID)
{
    if (0 == gastAtParaList[0].usParaLen) 
    {
        return AT_FAILURE;
    }

    /*DLCK命令在修改模式下最多有三个参数*/
    if(3 < gucAtParaIndex)
    {
        return AT_FAILURE;
    }

    /*操作模式检查*/
    if(DEVICE_LOCK_MODE_MAX <= gastAtParaList[0].ulParaValue)
    {
        return AT_FAILURE;
    }

    /*密码长度和非法值检查*/
    if ((MIN_DLCK_CODE_LEN > gastAtParaList[1].usParaLen) || (MAX_DLCK_CODE_LEN < gastAtParaList[1].usParaLen))
    {
        return AT_FAILURE;
    }

    if (AT_SUCCESS != At_CheckNumString(gastAtParaList[1].aucPara,gastAtParaList[1].usParaLen))
    {
        return AT_FAILURE;
    }

    /*在修改模式下新密码长度和非法值检查*/
    if(DEVICE_LOCK_MODE_MODIFY == gastAtParaList[0].ulParaValue)
    {
        /*检查密码长度*/
        if ((MIN_DLCK_CODE_LEN > gastAtParaList[2].usParaLen) || (MAX_DLCK_CODE_LEN < gastAtParaList[2].usParaLen))
        {
            return AT_FAILURE;
        }
        /*检查密码非法值*/
        if (AT_SUCCESS != At_CheckNumString(gastAtParaList[2].aucPara,gastAtParaList[2].usParaLen))
        {
            return AT_FAILURE;
        }
    }
    else
    {
        /*其他模式下不允许有新密码*/
        if(0 != gastAtParaList[2].usParaLen)
        {
            return AT_FAILURE;
        }
    }

    return AT_SUCCESS;
}


VOS_UINT32  AT_SetDlckPara (VOS_UINT8 ucIndex)
{
    VOS_UINT32    ulResult = AT_SUCCESS;
    TAF_ME_PERSONALISATION_DATA_STRU    stMePersonalisationData = {0};

    /* 参数检查 */
    ulResult = At_CheckDlckPara();
    if(AT_SUCCESS != ulResult)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*填充消息*/
    stMePersonalisationData.aucmode = gastAtParaList[0].ulParaValue;

    MBB_MEM_CPY(stMePersonalisationData.aucOldPwd,
               gastAtParaList[1].aucPara,
               TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);
       
    if(DEVICE_LOCK_MODE_MODIFY == stMePersonalisationData.aucmode)
    {
        MBB_MEM_CPY(stMePersonalisationData.aucNewPwd,
                   gastAtParaList[2].aucPara,
                   TAF_PH_ME_PERSONALISATION_PWD_LEN_MAX);
    }

    /* 安全命令类型为设置密码 */
    stMePersonalisationData.CmdType        = TAF_ME_PERSONALISATION_SET;
    /* 锁卡操作为DLCK设置 */
    stMePersonalisationData.MePersonalType = TAF_OPERATOR_PERSONALISATION_SETDLCK;
     /* 执行命令操作 */
    if(AT_SUCCESS == Taf_MePersonalisationHandle(gastAtClientTab[ucIndex].usClientId, 0,&stMePersonalisationData))
    {
        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CARD_DLCK_SET;
        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }
    else
    {
        return AT_ERROR;
    }
}


TAF_UINT32 AT_QryDlckPara(TAF_UINT8 ucIndex)
{
    TAF_ME_PERSONALISATION_DATA_STRU    stMePersonalisationData = {0};

    /* 安全命令类型为查询 */
    stMePersonalisationData.CmdType     = TAF_ME_PERSONALISATION_QUERY;
    /* 锁卡操作为DLCK查询 */
    stMePersonalisationData.MePersonalType = TAF_OPERATOR_PERSONALISATION_DLCK;
    /* 执行命令操作 */
    if(AT_SUCCESS == Taf_MePersonalisationHandle(gastAtClientTab[ucIndex].usClientId, 0,&stMePersonalisationData))
    {
        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CARD_DLCK_READ;
        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_CmdTestDlck( VOS_UINT8 ucIndex )
{
    return AT_OK;
}


VOS_UINT32 AT_CheckNwScanPara( TAF_PH_NW_SCAN_STRU *pstNwScanPara )
{
    /* 判断参数个数*/
    if ( gucAtParaIndex > AT_NWSCAN_MAX_PARA_NUM )
    {
        return AT_TOO_MANY_PARA;
    }

    /* 当前参数band_switch和channel不支持 */
    /* 获取接入技术*/
    switch( gastAtParaList[0].ulParaValue )
    {
        case AT_NWSCAN_GSM:
        case AT_NWSCAN_EDGE:
        case AT_NWSCAN_WCDMA:
        {
            /* 在BTU工位上会同时测试GSM和EDGE，因此在一起设置*/
            pstNwScanPara->stNwScanExPara.stRatOrder.ucRatOrderNum = AT_NWSCAN_BTU_RAT_NUM;
            pstNwScanPara->ucUtranMode = TAF_PH_IS_WCDMA_RAT;
            pstNwScanPara->stNwScanExPara.stRatOrder.aenRatOrder[0] = TAF_PH_RAT_WCDMA;
            pstNwScanPara->stNwScanExPara.stRatOrder.aenRatOrder[1] = TAF_PH_RAT_GSM;
            break;
        }
#if ( FEATURE_ON == FEATURE_UE_MODE_TDS )
        case AT_NWSCAN_TDSCDMA:
        {
            pstNwScanPara->stNwScanExPara.stRatOrder.ucRatOrderNum = AT_NWSCAN_BTT_BTL_RAT_NUM;
            pstNwScanPara->ucUtranMode = TAF_PH_IS_TDSCDMA_RAT;
            pstNwScanPara->stNwScanExPara.stRatOrder.aenRatOrder[0] = TAF_PH_RAT_WCDMA;
            break;
        }
#endif
        case AT_NWSCAN_CDMA:
        {
            /* 当前该命令不支持*/
            return AT_CME_INCORRECT_PARAMETERS;
        }
#if ( FEATURE_ON == FEATURE_LTE )
        case AT_NWSCAN_LTE_FDD:
        case AT_NWSCAN_LTE_TDD:
        {
            pstNwScanPara->stNwScanExPara.stRatOrder.ucRatOrderNum = AT_NWSCAN_BTT_BTL_RAT_NUM;
            pstNwScanPara->stNwScanExPara.stRatOrder.aenRatOrder[0] = TAF_PH_RAT_LTE;
            break;
        }
#endif
        default:
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 设置命令类型*/
    /*pstNwScanPara->stNwScanExPara.ucCmdType = TAF_PH_CMD_SET;*/

    return AT_OK;
}


VOS_UINT32 AT_SetNwScanPara( VOS_UINT8 ucIndex )
{
    VOS_UINT32    ulResult;
    TAF_PH_NW_SCAN_STRU    stNwScanSetPara;
    
    /* 命令类型检查*/
    if ( AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    MBB_MEM_SET( &stNwScanSetPara, 0, sizeof(stNwScanSetPara) );
   
    /* 参数检查*/
    ulResult = AT_CheckNwScanPara( &stNwScanSetPara );
    if ( AT_OK != ulResult )
    {
        return ulResult;
    }

    /* 发送设置消息*/
    ulResult = Taf_NwScanHandle( gastAtClientTab[ucIndex].usClientId, 0, stNwScanSetPara );
    if ( AT_SUCCESS == ulResult )
    {
        /* 指示当前用户的命令操作类型为设置命令*/
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NWSCAN_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 At_QryMmPlmnTimeZonePara(VOS_UINT8 ucIndex)
{
    if(AT_SUCCESS == Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   TAF_MM_PLMN_TIMEZONE_QRY_PARA,
                                   TAF_NULL_PTR))
    {
        MBB_AT_ACORE_COMMON_DEBUG_STR("AT_WAIT_ASYNC_RETURN");
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PLMN_TIMEZONE_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        MBB_AT_ACORE_COMMON_DEBUG_STR("AT_ERROR");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryNWTimePara (VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;
    NAS_MM_INFO_IND_STRU* pstTimeInfo = At_GetTimeInfo(ucIndex);
    if (pstTimeInfo != NULL)
    {
        if ( NAS_MM_INFO_IE_UTLTZ == (pstTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) )
        {
            usLength = At_PrintNwTimeInfo( pstTimeInfo, usLength,
                            (VOS_CHAR*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName, ":", NULL, TIME_FORMAT_QRY_NWTIME);
            gstAtSendData.usBufLen = usLength;
            return AT_OK;
        }        
    }
    return At_QryMmPlmnTimeZonePara( ucIndex );
}


VOS_UINT32 AT_QryCurrSysMode( VOS_UINT8 ucIndex )
{
    VOS_UINT32 ReturnMode = TAF_PH_INFO_NONE_RAT;
    TAF_AGENT_SYS_MODE_STRU stSysMode = {0};
    if (VOS_OK == TAF_AGENT_GetSysMode(ucIndex, &stSysMode))
    {
        MBB_AT_ACORE_COMMON_DEBUG("stSysMode.enRatType", stSysMode.enRatType);
        switch (stSysMode.enRatType)
        {
            case TAF_PH_RAT_GSM:
                ReturnMode = TAF_PH_INFO_GSM_RAT;
                break;
            case TAF_PH_RAT_WCDMA:
                ReturnMode = TAF_PH_INFO_WCDMA_RAT;
                break;
            case TAF_PH_RAT_LTE:
                ReturnMode = TAF_PH_INFO_LTE_RAT;
                break;
            default:
                break;
        }
    }
    else
    {
        MBB_AT_ACORE_COMMON_DEBUG("TAF_AGENT_GetSysMode return VOS_ERR", VOS_ERR);
    }
    return ReturnMode;
}


VOS_UINT32 AT_QryLteRsrp( VOS_UINT8 ucIndex )
{
    TAF_PH_INFO_RAT_TYPE ucRat = TAF_PH_INFO_NONE_RAT;
    VOS_UINT32           ulRet = VOS_OK;

     /*L4A*/
    L4A_CSQ_INFO_REQ_STRU stCsqReq  = {0};
    stCsqReq.stCtrl.ulClientId      = gastAtClientTab[ucIndex].usClientId;
    stCsqReq.stCtrl.ulOpId          = 0;
    stCsqReq.stCtrl.ulPid           = WUEPS_PID_AT;

    ucRat = AT_QryCurrSysMode(ucIndex);

    if(TAF_PH_INFO_LTE_RAT == ucRat) 
    {
       ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_LTERSRP_INFO_REQ, (VOS_VOID*)(&stCsqReq), sizeof(stCsqReq));
    
        if(ERR_MSP_SUCCESS == ulRet)
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTERSRP_READ;
            return AT_WAIT_ASYNC_RETURN;
        }
        else
        {
            return AT_ERROR;
        }
    }
    else 
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestLteRsrpPara(VOS_UINT8 ucIndex)
{
    /* 判断单板的接入模式，根据当前注册的网络模式判断是否返回RSRP RSRQ 范围 **/
    if(TAF_PH_INFO_LTE_RAT == AT_QryCurrSysMode(ucIndex))
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                                       "%s:(%d,%d),(%d,%d)",
                                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                       AT_RSRP_MIN,
                                                       AT_RSRP_MAX,
                                                       AT_RSRQ_MIN,
                                                       AT_RSRQ_MAX);
    }
    else
    {
        return AT_ERROR;
    }
    return AT_OK;

}

/*****************************************************************************
 函 数 名  : AtQryLHcsqPara
 功能描述  : ^hcsq查询命令对LTE的处理
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_QryLHcsqPara(VOS_UINT8 ucIndex)
{
     VOS_UINT32 ulRet                = VOS_OK;

    /*L4A*/
    L4A_CSQ_INFO_REQ_STRU stCsqReq  = {0};

    /*赋值*/
    stCsqReq.stCtrl.ulClientId      = gastAtClientTab[ucIndex].usClientId;
    stCsqReq.stCtrl.ulOpId          = 0;
    stCsqReq.stCtrl.ulPid             = WUEPS_PID_AT;

    /*发送消息到L4A*/
    ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_HCSQ_INFO_REQ, (VOS_VOID*)(&stCsqReq), sizeof(stCsqReq));
    
    if(ERR_MSP_SUCCESS == ulRet)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HCSQ_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
/*****************************************************************************
 函 数 名  : AtQryLCindPara
 功能描述  : +Cind查询命令对LTE的处理
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_QryLCindPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ulRet                = VOS_OK;
    /*L4A*/
    L4A_CSQ_INFO_REQ_STRU stCsqReq  = {0};
    /*赋值*/
    stCsqReq.stCtrl.ulClientId      = gastAtClientTab[ucIndex].usClientId;
    stCsqReq.stCtrl.ulOpId          = 0;
    stCsqReq.stCtrl.ulPid           = WUEPS_PID_AT;
    /*发送消息到L4A*/
    ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_CIND_INFO_REQ, (VOS_VOID*)(&stCsqReq), sizeof(stCsqReq));
    if(ERR_MSP_SUCCESS == ulRet)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CIND_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}
/*****************************************************************************
 函 数 名      : At_QryLAntPara
 功能描述  : +ant查询命令对LTE的处理
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值      : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_QryLAntPara(VOS_UINT8 ucIndex)
{
     VOS_UINT32 ulRet                = VOS_OK;

    /*L4A*/
    L4A_CSQ_INFO_REQ_STRU stCsqReq = {0};

    /*赋值*/
    stCsqReq.stCtrl.ulClientId      = gastAtClientTab[ucIndex].usClientId;
    stCsqReq.stCtrl.ulOpId          = 0;
    stCsqReq.stCtrl.ulPid           = WUEPS_PID_AT;

    /*发送消息到L4A*/
    ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_ANT_INFO_REQ, (VOS_VOID*)(&stCsqReq), sizeof(stCsqReq));
    
    if(ERR_MSP_SUCCESS == ulRet)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ANT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 AT_QryHcsqPara (VOS_UINT8 ucIndex)
{
 #if  (FEATURE_ON == FEATURE_LTE)
    TAF_PH_INFO_RAT_TYPE ucRat = TAF_PH_INFO_NONE_RAT;

    ucRat = AT_QryCurrSysMode(ucIndex);

    if(TAF_PH_INFO_LTE_RAT == ucRat)
    {
        return At_QryLHcsqPara(ucIndex);
    }
#endif
    if (AT_SUCCESS == Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId,0,TAF_PH_HCSQ_PARA,TAF_NULL_PTR))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HCSQ_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_QryHcsqPara: Taf_ParaQuery fail.");
        return AT_ERROR;
    }
}


VOS_UINT32 AT_TestHcsqPara(VOS_UINT8 ucIndex)
{
#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s:\"NOSERVICE\" , \"GSM\" ,\"WCDMA\" ,\"TD-SCDMA\" ,\"LTE\" ",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
#else
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s:\"NOSERVICE\" , \"GSM\" ,\"WCDMA\" ,\"LTE\" ",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
#endif
    
    return (VOS_UINT32)AT_OK;
}
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)

VOS_UINT32 At_SetCmerPara(VOS_UINT8 ucIndex)
{
   /* 参数过多 */
    if(AT_CMER_PARA_COUNT_MAX != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 输入参数取值检查, 必须为 0/1 0/3 */
    if ((AT_CMER_RPT_MODE_SWITCH_ON != gastAtParaList[0].ulParaValue) 
        && (AT_CMER_RPT_MODE_SWITCH_OFF != gastAtParaList[0].ulParaValue))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    if ((AT_CMER_RPT_SWITCH_ON != gastAtParaList[3].ulParaValue) 
        && (AT_CMER_RPT_SWITCH_OFF != gastAtParaList[3].ulParaValue))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    if ((AT_CMER_RPT_SWITCH_OFF != gastAtParaList[1].ulParaValue)
        || (AT_CMER_RPT_SWITCH_OFF != gastAtParaList[2].ulParaValue)
        || (AT_CMER_RPT_SWITCH_OFF != gastAtParaList[4].ulParaValue))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*需要全局变量保存+CMER参数数值*/
    g_ucCmerpara.mode = gastAtParaList[0].ulParaValue;
    g_ucCmerpara.ind = gastAtParaList[3].ulParaValue;
    return AT_OK;
}

VOS_UINT32 At_SetCindPara(TAF_UINT8 ucIndex)
{
    /* 参数过多过少 */
    if((AT_CIND_PARA_COUNT_MAX < gucAtParaIndex) || (AT_CIND_PARA_COUNT_MIN > gucAtParaIndex))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 输入参数取值检查, 必须为 0/1 */
    if ((AT_CIND_RPT_SWITCH_OFF != gastAtParaList[1].ulParaValue) && (AT_CIND_RPT_SWITCH_ON != gastAtParaList[1].ulParaValue)
    || (AT_CIND_RPT_SWITCH_OFF != gastAtParaList[2].ulParaValue) && (AT_CIND_RPT_SWITCH_ON != gastAtParaList[2].ulParaValue))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    if ((AT_CIND_RPT_SWITCH_OFF != gastAtParaList[0].ulParaValue) || (AT_CIND_RPT_SWITCH_OFF != gastAtParaList[3].ulParaValue)
    || (AT_CIND_RPT_SWITCH_OFF != gastAtParaList[4].ulParaValue) || (AT_CIND_RPT_SWITCH_OFF != gastAtParaList[5].ulParaValue)
    || (AT_CIND_RPT_SWITCH_OFF != gastAtParaList[6].ulParaValue) || (AT_CIND_RPT_SWITCH_OFF != gastAtParaList[7].ulParaValue))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*需要全局变量保存+CIND参数数值*/
    g_ucCindpara.signal = gastAtParaList[1].ulParaValue;
    g_ucCindpara.service = gastAtParaList[2].ulParaValue;
    return AT_OK;
}

VOS_UINT32 At_QryCindPara(TAF_UINT8 ucIndex)
{
#if (FEATURE_ON == FEATURE_LTE)
    TAF_PH_INFO_RAT_TYPE ucRat = TAF_PH_INFO_NONE_RAT;
    ucRat = AT_QryCurrSysMode(ucIndex);
    if(TAF_PH_INFO_LTE_RAT == ucRat)
    {
        return At_QryLCindPara(ucIndex);
    }
#endif
    if (AT_SUCCESS == Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId, 0, TAF_PH_CIND_VALUE_PARA, TAF_NULL_PTR))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CIND_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_QryCindPara: Taf_ParaQuery fail.");
        return AT_ERROR;
    }
}

VOS_UINT32 At_TestCindPara(TAF_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN, (TAF_CHAR *)pgucAtSndCodeAddr, (TAF_CHAR *)pgucAtSndCodeAddr,
                                      "%s:(\"battchg\",(0-5)),(\"signal\",(0-4)),(\"service\",(0-1)),(\"call\",(0-1)),(\"roam\",(0-1)),(\"smsfull\",(0-2)),(\"packet\",(0-1)),(\"callsetup\",(0-3))",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    return (VOS_UINT32)AT_OK;
}
/*****************************************************************************
 函 数 名  : AT_QryAnt
 功能描述  :
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 添加 AT*ANT
*****************************************************************************/
VOS_UINT32 AT_QryAnt( VOS_UINT8 ucIndex )
{
#if (FEATURE_ON == FEATURE_LTE)
    TAF_PH_INFO_RAT_TYPE ucRat = TAF_PH_INFO_NONE_RAT;

    ucRat = AT_QryCurrSysMode(ucIndex);

    if(TAF_PH_INFO_LTE_RAT == ucRat)
    {
        return At_QryLAntPara(ucIndex);/*修改*/
    }
#endif
    /* 给MMA发送消息，查询新参数类型TAF_PH_ANT_VALUE_PARA */
    if(AT_SUCCESS == Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   TAF_PH_ANT_VALUE_PARA,
                                   TAF_NULL_PTR))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ANT_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_QryAnt: Taf_ParaQuery fail.");
        return AT_ERROR;
    }
}
#endif

 VOS_UINT32 At_CheckEonsPara(AT_TAF_PLMN_ID* ptrPlmnID)
 {
    VOS_UINT8 ucParaTmpLen = 0;
    VOS_UINT8 i = 0;
    VOS_UINT16 ulPlmnNameLen = 0;
    VOS_UINT8 ulIndex = 0;

    /* 参数过多，返回参数错误*/
    if (3 < gucAtParaIndex || 0 == gucAtParaIndex)
    {
        AT_WARN_LOG("At_CheckEonsPara: the aucPara too much.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*balong AT机制在入参传入时已经检查完毕，此处可以直接赋值*/
    ptrPlmnID->EonsType = (TAF_UINT8)gastAtParaList[0].ulParaValue;
    /*<plmn_name_len>：当<type>不为5时，不下发此字段,此字段必须是空*/
    if(AT_EONS_TYPE_MODULE != ptrPlmnID->EonsType 
            && 0 != gastAtParaList[2].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    } 

    /*<plmn_id>: 当Type为5时，此字段必须是空*/
    if (AT_EONS_TYPE_MODULE == ptrPlmnID->EonsType
         && 0 != gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <plmn_id>: 当Type为5时, 检查输入的plmn_name_len是否合法 */
    if(AT_EONS_TYPE_MODULE == ptrPlmnID->EonsType)
    {
        for (ulIndex = 0; ulIndex < gastAtParaList[2].usParaLen; ulIndex++)
        {
            if((0x30 <= gastAtParaList[2].aucPara[ulIndex]) && (gastAtParaList[2].aucPara[ulIndex] <= 0x39))
            {
                ulPlmnNameLen = ulPlmnNameLen * 10 + gastAtParaList[2].aucPara[ulIndex] - 0x30;
            }
            else
            {
                AT_WARN_LOG("At_CheckEonsPara: the third aucPara isn't 0-9.");
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }
    }


    /* 第二个参数为空，获取入驻的PLMN的信息 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        /* 将入驻的PLMN 信息获取 ，仅以此为标识*/
        ptrPlmnID->PlmnLen = 0;
    }
    else
    {
        ucParaTmpLen = (VOS_UINT8)gastAtParaList[1].usParaLen;
        MBB_AT_ACORE_COMMON_DEBUG("ucParaTmpLen", ucParaTmpLen);
        
        /*如果当前PLMN LEN 去掉引号，不是5位或者6位返回参数错误*/
        if ((ucParaTmpLen < MMA_PLMN_ID_LEN_5) || (ucParaTmpLen > MMA_PLMN_ID_LEN_6))
        {
            AT_WARN_LOG("At_CheckEonsPara: the second aucPara len is wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /*需要判定如果传入的是非数字ID则返回参数错误*/
        for (i = 0;i < ucParaTmpLen; i++)
        {
            if((0x30 <= gastAtParaList[1].aucPara[i]) && (gastAtParaList[1].aucPara[i] <= 0x39))
            {
                ptrPlmnID->PLMNId[i] = (VOS_CHAR)gastAtParaList[1].aucPara[i];
            }
            else
            {
                AT_WARN_LOG("At_CheckEonsPara: the second aucPara isn't 0-9.");
                return AT_CME_INCORRECT_PARAMETERS;
            }
        }

        ptrPlmnID->PlmnLen = ucParaTmpLen;

    }
     /*Eons=5，参数检查的时候，记住参数是5时候的长度定制：此字段表示长名称和短
     *名称最大支持的长度，当此字段不下发时，长短名称的最大支持长度默认为20。模块
     *产品不对此字段进行扩展。
     */
    switch(ptrPlmnID->EonsType)
    {
    case AT_EONS_TYPE_MODULE:
        ptrPlmnID->PlmnNameLen.ucEonsType = AT_EONS_TYPE_MODULE;
        ptrPlmnID->PlmnNameLen.ulLsNameLen = (gastAtParaList[2].usParaLen? 
            ulPlmnNameLen : AT_ENOS_PLMN_NAME_DEFAULT_LEN);
        break;      
    default:
        /*For customize,for future...*/
        break;
    }

    return AT_SUCCESS;
}


VOS_UINT32  At_SetEonsPara(TAF_UINT8 ucIndex)
{
   
    VOS_UINT32 ulresult = AT_CME_INCORRECT_PARAMETERS;
    AT_TAF_PLMN_ID stPlmnPara = {0};

    /*检查EONS 的入参*/
    ulresult = At_CheckEonsPara(&stPlmnPara); 
    if(AT_SUCCESS != ulresult)
    {
        AT_WARN_LOG("At_SetEonsPara: At_CheckEonsPara fail.");
        return ulresult;
    }

    /* 调用A核获取LIST 的函数 */   
    if(AT_SUCCESS == TAF_EONSGetNWName(gastAtClientTab[ucIndex].usClientId, gastAtClientTab[ucIndex].opId,stPlmnPara))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EONS_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("At_SetCopnPara return ERROR.");
        return AT_ERROR;
    }

}


VOS_UINT32 AT_TestEonsPara(VOS_UINT8 ucIndex)
{
    /*合入测试命令支持1-4*/
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR*)pgucAtSndCodeAddr,(TAF_CHAR*)pgucAtSndCodeAddr,
                                    "%s:1-5",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    return AT_OK;
}

/*****************************************************************************
 函 数 名  : At_SetHfreqinfo
 功能描述  : AT^Hfreqinfo=<value>(用于设置是否使能主动上报)
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_SetHFreqinfo(VOS_UINT8 ucIndex)
{
    TAF_UINT8 ucReprotMode = AT_HFREQINFO_NO_REPORT;
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_ERROR;
    }
    
    /*参数长度不为1*/
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }

    ucReprotMode = (TAF_UINT8)(gastAtParaList[0].ulParaValue);

    /*1表示使能主动上报，0表示禁止主动上报*/
    if((AT_HFREQINFO_NO_REPORT != ucReprotMode) &&  (AT_HFREQINFO_REPORT != ucReprotMode))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_AtHFreqinforeport = ucReprotMode;

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : At_QryHFreqinfo
 功能描述  : ^HFreqinfo查询命令处理,调用lwcrash函数接口
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
TAF_UINT32 At_QryHFreqinfo(TAF_UINT8 ucIndex)
{
    TAF_UINT32 ulRat = TAF_PH_INFO_NONE_RAT;
    TAF_UINT32 ulRet = ERR_MSP_SUCCESS;
    L4A_READ_LWCLASH_REQ_STRU stLwclash = {0};

     ulRat = AT_QryCurrSysMode(ucIndex);
     
    stLwclash.stCtrl.ulClientId = gastAtClientTab[ucIndex].usClientId;
    stLwclash.stCtrl.ulOpId = 0;
    stLwclash.stCtrl.ulPid = WUEPS_PID_AT;

    if(TAF_PH_INFO_LTE_RAT != ulRat)
    {
        MBB_AT_ACORE_COMMON_DEBUG_STR("current rat is not LTE");
        return AT_ERROR;
    }
    
    ulRet = atSendDataMsg(MSP_L4_L4A_PID, ID_MSG_L4A_LWCLASHQRY_REQ, (VOS_VOID*)(&stLwclash), sizeof(L4A_READ_LWCLASH_REQ_STRU));
    if(ERR_MSP_SUCCESS == ulRet)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_HFREQINFO_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

#if (FEATURE_ON == MBB_WPG_LTXPOWER)

VOS_UINT32 At_QryLTxPower(TAF_UINT8 ucIndex)
{
    VOS_UINT32              ulRst;
    AT_MTA_RESERVE_STRU     stReserved = {0};

    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTXPOWER_QRY_REQ,
                                   &stReserved,
                                   sizeof(AT_MTA_RESERVE_STRU),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTXPOWER_QRY;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}
#endif/*end (FEATURE_ON == MBB_WPG_LTXPOWER)*/

/*****************************************************************************
 函 数 名  :  At_TestHFreqinfo
 功能描述  : ^HFREQINFO命令的测试命令
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_TestHFreqinfo(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s:(0,1),(%d)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       AT_HFREQINFO_RAT_TYPE_LTE);
    return AT_OK;
}


VOS_UINT32 At_SetSmsregstPara(VOS_UINT8 ucIndex)
{
    SMS_AUTO_REG_STRU stSmsAutoReg = {0};

    /*检查定制NV是否开启*/
    if((NV_OK != NV_Read(NV_ID_SMS_AUTO_REG,&stSmsAutoReg,sizeof(SMS_AUTO_REG_STRU)))
    || (VOS_FALSE == stSmsAutoReg.ucNvActiveFlag))
    {
        return AT_ERROR;
    }

    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (1 < gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*IMSI长度检查和非法值检查*/
    if( MAX_IMSI_LEN != gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_SUCCESS != At_CheckNumString(gastAtParaList[0].aucPara,gastAtParaList[0].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*写入NV*/
    stSmsAutoReg.ucSmsRegFlag = VOS_TRUE;
    MBB_MEM_CPY(stSmsAutoReg.ucSmsRegImsi, gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    
    if(NV_OK != NV_Write(NV_ID_SMS_AUTO_REG,&stSmsAutoReg,sizeof(SMS_AUTO_REG_STRU)))
    {
        return AT_ERROR;
    }
    else
    {
        return AT_OK;
    }
}


VOS_UINT32 At_QrySmsregstPara(VOS_UINT8 ucIndex)
{
    SMS_AUTO_REG_STRU stSmsAutoReg = {0};
    VOS_UINT8 aucRegImsi[MAX_IMSI_LEN + 1] = {0};

    if((NV_OK != NV_Read(NV_ID_SMS_AUTO_REG,&stSmsAutoReg,sizeof(SMS_AUTO_REG_STRU)))
    || (VOS_FALSE == stSmsAutoReg.ucNvActiveFlag))
    {
        return AT_ERROR;
    }

    if(VOS_FALSE == stSmsAutoReg.ucSmsRegFlag)
    {
        /*lint -e160 -e506 -e522*/
        MBB_MEM_SET(aucRegImsi, '0', MAX_IMSI_LEN);
        /*lint +e160 +e506 +e522*/

        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    aucRegImsi);

        return AT_OK;
    }
    else
    {
        MBB_MEM_CPY(aucRegImsi, stSmsAutoReg.ucSmsRegImsi, MAX_IMSI_LEN);
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    aucRegImsi);

        return AT_OK;
    }
}


VOS_UINT32 At_TestSmsregst(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr,
                    "%s: (000000000000000-999999999999999)",
                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}
/*****************************************************************************
 函 数 名  :  AT_UpdateCclkInfo
 功能描述  : 更新CCLK信息
 输入参数  : VOS_UINT8                        *pucDateStr
            VOS_UINT32                        ulDateStrLen
 输出参数  : 无
 返 回 值  : AT_ERROR   日期有效性检查失败
             AT_OK      日期有效性检查成功

*****************************************************************************/
VOS_VOID AT_UpdateCclkInfo(NAS_MM_INFO_IND_STRU *pStMmInfo)
{
    gstCCLKInfo.ulTimeSeconds = pStMmInfo->ulTimeSeconds;
    gstCCLKInfo.ucIeFlg = pStMmInfo->ucIeFlg;
    MBB_MEM_CPY(&gstCCLKInfo.stUniversalTimeandLocalTimeZone,
                    &pStMmInfo->stUniversalTimeandLocalTimeZone,
                    sizeof(gstCCLKInfo.stUniversalTimeandLocalTimeZone));
}


VOS_UINT32 AT_CheckCclkDataFormat(
    VOS_UINT8                           *pucDateStr,
    VOS_UINT16                          ulDateStrLen
)
{
    VOS_UINT8                           ucFirstJuncture;
    VOS_UINT8                           ucSecondJuncture;
    VOS_UINT8                           ucFirstColon;
    VOS_UINT8                           ucSecondColon;
    VOS_UINT8                           ucComma;
    VOS_UINT8                           ucPlus;
 
    /* 格式为yyyy/mm/dd,hh:mm:ss+tz 或者yyyy/mm/dd,hh:mm:ss的日期字符串: 第5个字节为'-', 第8个字节为'-' */
    ucFirstJuncture      = *(pucDateStr + 4);
    ucSecondJuncture  = *(pucDateStr + 7);
    if(('/' != ucFirstJuncture) || ('/' != ucSecondJuncture))
    {
        return AT_ERROR;
    }
    ucComma = *(pucDateStr + 10);
    if(',' != ucComma)
    {
        return AT_ERROR;
    }
    /* 格式为yyyy/mm/dd,hh:mm:ss+tz 或者yyyy/mm/dd,hh:mm:ss的日期字符串: 第14个字节为':', 第17个字节为':' */
    ucFirstColon = *(pucDateStr + 13);
    ucSecondColon = *(pucDateStr + 16);
    if((':' != ucFirstColon) || (':' != ucSecondColon))
    {
        return AT_ERROR;
    }

    /* 格式为yyyy/mm/dd,hh:mm:ss+tz 的日期字符串: 第20个字节为'+'/'-'*/
    if(sizeof("yyyy/mm/dd,hh:mm:ss+tz") - 1 == ulDateStrLen)
    {
        ucPlus = *(pucDateStr + 19);
        if(('+' != ucPlus) && ('-' != ucPlus))
        {
            return AT_ERROR;
        }
        
    }

    return AT_OK;
}
/*****************************************************************************
 函 数 名  : AT_GetTimeZone
 功能描述  : 获取时区
 输入参数  : VOS_UINT8                           *pucDateStr
             VOS_UINT32                          ulDateStrLen
 输出参数  : AT_DATE_STRU                        *pstDate
 返 回 值  : AT_ERROR   获取时区失败
             AT_OK      获取时区成功

*****************************************************************************/
VOS_UINT32  AT_GetTimeZone(
    VOS_UINT8                           *pucDateStr,
    VOS_UINT32                          ulDateStrLen,
    AT_DATE_STRU                        *pstDate
)
{
    VOS_UINT32                          ulRet;
    pstDate->slTimeZone = AT_INVALID_TZ_VALUE;
    /*检查是否有时区参数*/
    if (sizeof("yyyy/mm/dd,hh:mm:ss+tz") - 1 > ulDateStrLen)
    {
        return AT_OK;
    }
    
    ulRet = At_Auc2ul((pucDateStr + 20), 2, (TAF_UINT32*)&pstDate->slTimeZone); /*得到2位时区数字*/
    if (AT_SUCCESS != ulRet)
    {
        return AT_ERROR;
    }
        
    if ('+' == *(pucDateStr + 19))  /*判断时区符号*/
    {
        /*nothing need to do*/
    }
    else if ('-' == *(pucDateStr + 19)) /*判断时区符号*/
    {
        pstDate->slTimeZone = - pstDate->slTimeZone;
    }
    else
    {
        return AT_ERROR;
    }
    
    return AT_OK;
}
/*****************************************************************************
 函 数 名  : At_SetCCLK
 功能描述  : AT+CCLK=<value>(用于设置时间信息)
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_SetCCLK(VOS_UINT8 ucIndex)
{
    TAF_UINT32 ulRet = 0;
    AT_DATE_STRU stDate;
    
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    
    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*参数长度合法*/
    if ((sizeof("yyyy/mm/dd,hh:mm:ss") - 1 != gastAtParaList[0].usParaLen)
        && (sizeof("yyyy/mm/dd,hh:mm:ss+tz") - 1 != gastAtParaList[0].usParaLen)
        )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*lint --e{160,506,522}*/
    MBB_MEM_SET( (VOS_VOID*)&stDate, 0, sizeof(stDate) );
    /*日期有效性检查*/
    ulRet = AT_CheckCclkDataFormat(gastAtParaList[0].aucPara,gastAtParaList[0].usParaLen);
    if(AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_GetDate(gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen, &stDate);
    if (AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_GetTimeZone(gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen, &stDate);
    if (AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRet = AT_CheckDate(&stDate);
    if (AT_OK != ulRet)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    /*时区有效性检查 */
    if ((stDate.slTimeZone > 96 && ( AT_INVALID_TZ_VALUE != stDate.slTimeZone )) 
        || (stDate.slTimeZone < -96)) /*时区的有效范围为[-96,96]*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ( (stDate.ulYear < 2000) || (stDate.ulYear > 2100) ) /*规范要求起始时间是2000年-2100年*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    gstCCLKInfo.ulTimeSeconds = OM_GetSeconds();
    gstCCLKInfo.ucIeFlg = NAS_MM_INFO_IE_UTLTZ;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucYear = (TAF_UINT8)(stDate.ulYear - 2000); /*以默认的2000年开始计算*/
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucMonth = (TAF_UINT8)stDate.ulMonth;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucDay = (TAF_UINT8)stDate.ulDay;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucHour = (TAF_UINT8)stDate.ulHour;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucMinute = (TAF_UINT8)stDate.ulMunite;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.ucSecond = (TAF_UINT8)stDate.ulSecond;
    gstCCLKInfo.stUniversalTimeandLocalTimeZone.cTimeZone = (TAF_INT8)stDate.slTimeZone;    
    
    return AT_OK;
}



VOS_UINT32 At_QryCCLK(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;
    NAS_MM_INFO_IND_STRU* pstTimeInfo = &gstCCLKInfo;

    if ( NAS_MM_INFO_IE_UTLTZ == (pstTimeInfo->ucIeFlg & NAS_MM_INFO_IE_UTLTZ) )
    {
        usLength = At_PrintNwTimeInfo( pstTimeInfo, usLength,
                        (VOS_CHAR*)g_stParseContext[ucIndex].pstCmdElement->pszCmdName, ": \"", "\"", TIME_FORMAT_QRY_CCLK);
        gstAtSendData.usBufLen = usLength;
        return AT_OK;
    }
    
    return AT_ERROR;
}

/*****************************************************************************
 Function     : AT_QryDataclassPara
 Description  : 此函数模仿^SFEATURE查询函数。
                WCDMA、GPRS、GSM 默认支持，不做判断。
 Input        : VOS_UINT8 ucIndex
 Output       : 
 Return Value : VOS_UINT32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/1
    Author       : wenlong
    Modification : Created function
*****************************************************************************/
VOS_UINT32 AT_QryDataclassPara( VOS_UINT8 ucIndex )
{
    AT_NVIM_UE_CAPA_STRU        stUECapa = {0};
    VOS_UINT32                  ulDataclass = 0;
    VOS_UINT32                  ulHspa = 0;
    VOS_INT32                    i = 0;
    VOS_UINT16    usEgprsFlag = 0;
    VOS_UINT8                   aucDataclassLteName[AT_DATACLASSLTE_MAX][AT_DATACLASS_NAME_LEN_MAX] = {"LTE"} ;
    VOS_UINT32                  ulDataclassLte = 0;
    VOS_UINT8     aucDataclassName[AT_DATACLASS_MAX + 1][AT_DATACLASS_NAME_LEN_MAX] =
                                {
                                    "GSM",
                                    "GPRS",
                                    "EDGE",
                                    "WCDMA",
                                    "HSDPA",
                                    "HSUPA",
                                    "HSPA",
                                    "HSPA+",
                                    "DC-HSPA+",
                                };
    MBB_MEM_SET(&stUECapa, 0x00, sizeof(AT_NVIM_UE_CAPA_STRU));

    /* 读取was_nv: 9008支持能力 */
    if (NV_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_WAS_RadioAccess_Capa_New, 
                            &stUECapa, sizeof(AT_NVIM_UE_CAPA_STRU)))
    {
        return AT_ERROR;
    }

    /* DC-HSPA+是否支持 */
    if ((AT_DATACLASS_ENASRELINDICATOR_R8 <= stUECapa.enAsRelIndicator)
         && (AT_DATACLASS_SUPPORT == stUECapa.enMultiCellSupport)
         && (AT_DATACLASS_SUPPORT == stUECapa.enAdjFreqMeasWithoutCmprMode)
         && (AT_DATACLASS_NOT_SUPPORT != stUECapa.ucHSDSCHPhyCategoryExt)
         && (AT_DATACLASS_NOT_SUPPORT != stUECapa.ucHSDSCHPhyCategoryExt2))
    {
         ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_DC_HSPAPLUS);
    }

    /* HSPA+是否支持 */
    if ((AT_DATACLASS_ENASRELINDICATOR_R7 <= stUECapa.enAsRelIndicator)
         && (AT_DATACLASS_SUPPORT == stUECapa.enMacEhsSupport)
         && (AT_DATACLASS_NOT_SUPPORT != stUECapa.ucHSDSCHPhyCategoryExt))
    {
         ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSPAPLUS);
    }

    /* DPA是否支持 */
    if (AT_DATACLASS_ENASRELINDICATOR_R5 <= stUECapa.enAsRelIndicator)
    {
        if (AT_DATACLASS_HSPASTATUS_ACTIVED == stUECapa.ulHspaStatus)
        {
            if (AT_DATACLASS_SUPPORT == stUECapa.enHSDSCHSupport)
            {
                ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
                ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
            }
        }
        else
        {
            ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
            ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA);
        }
    }

    /*UPA是否支持*/
    if (AT_DATACLASS_ENASRELINDICATOR_R6 <= stUECapa.enAsRelIndicator)
    {
        if (AT_DATACLASS_HSPASTATUS_ACTIVED == stUECapa.ulHspaStatus)
        {

            /* 这个地方与^SFEATURE的判断不一样 */
            if (AT_DATACLASS_SUPPORT == stUECapa.enEDCHSupport)
            {
                ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
                ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
            }
        }
        else
        {
            ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
            ulHspa      |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA);
        }
    }

    /* 如果支持 DPA和UPA，则支持HSPA */
    if (((AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSUPA) | (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSDPA)) == ulHspa)
    {
        ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_HSPA);
    }

    /* WCDMA设置为支持 */    
    ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_WCDMA);

    /*v7平台产品可能会定制不支持GSM，做此判断*/
    if(VOS_TRUE == g_MbbIsRatSupport.ucGsmSupport)
    {
        /* EDGE是否支持 */
        if (NV_OK != NV_Read(en_NV_Item_Egprs_Flag, &usEgprsFlag,
                        sizeof(VOS_UINT16)))
        {
            return AT_ERROR;
        }
        else
        {
            if (AT_DATACLASS_NOT_SUPPORT != usEgprsFlag)
            {
                ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_EDGE);        
            }
        }
    
        /* GPRS设置为支持 */
        ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_GPRS);
    
        /* GSM设置为支持 */
        ulDataclass |= (AT_DATACLASS_BASE_VALUE << AT_DATACLASS_GSM);
    }

    if(VOS_TRUE == g_MbbIsRatSupport.ucLteSupport)
    {
        ulDataclassLte = AT_DATACLASS_LTE;
    }
    /* 将当前所有支持的能力等级aucDataclass与当前支持的最高等级从高位按位与，
      第一个不为0的数则是当前支持的最大能力等级，让后利用得到的i值，选出对应的字符串 */
    for(i = AT_DATACLASS_MAX; i >= 0; i--)
    {
        if (0 != ((AT_DATACLASS_BASE_VALUE << i) & ulDataclass))
        {
            break;
        }
    }
    if(i < 0)
    {
        return AT_ERROR;
    }
    
    if(AT_DATACLASS_LTE == ulDataclassLte)
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               (TAF_CHAR *)pgucAtSndCodeAddr,
                                               "%s:,,0x%08x,%s,,,,,%x,%s",/*lint !e64 !e119*/
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               ulDataclass,
                                               aucDataclassName[i],
                                               ulDataclassLte,
                                               aucDataclassLteName[0]);/*lint !e64 !e119*/
        }
    else
    {
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           (TAF_CHAR *)pgucAtSndCodeAddr,
                                           "%s:,,0x%08x,%s,,,,,,",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           ulDataclass,
                                           aucDataclassName[i]);
    }
    return AT_OK;    
    
}


static TAF_UINT32 AT_TestCsimPara( TAF_UINT8 ucIndex )
{
    if(!AT_IsCSIMCustommed())
    {
        return AT_CMD_NOT_SUPPORT;
    }
    
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
               (TAF_CHAR*)pgucAtSndCodeAddr, (TAF_CHAR*)pgucAtSndCodeAddr,
               "%s: %s",
               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
               g_stParseContext[ucIndex].pstCmdElement->pszParam);
    
    return AT_OK;
}

VOS_UINT32 At_SetSimPowerPara (VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usFlag;
    USIMM_MsgBlock                   *pstMsg = VOS_NULL_PTR;

    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if(gucAtParaIndex > 1)
    {
        return AT_TOO_MANY_PARA;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[0].ulParaValue)
    {
        case AT_SIMPOWER_ON:
            pstMsg = (USIMM_MsgBlock *)VOS_AllocMsg(WUEPS_PID_AT,
                        sizeof(USIMM_MsgBlock)-VOS_MSG_HEAD_LENGTH);

            if(VOS_NULL_PTR == pstMsg)
            {
                LogPrint("USIMM_InitCardStart:AllocMsg Failed.");

                return VOS_ERR;
            }

            pstMsg->ulReceiverPid      = WUEPS_PID_USIM;
            pstMsg->enMsgType          = USIMM_CMDTYPE_INITSTART;

            if (VOS_OK != VOS_SendMsg(WUEPS_PID_AT, pstMsg))
            {
                LogPrint("USIMM_InitCardStart:sndmsg Failed.");
                return VOS_ERR;
            }

            break;

        case AT_SIMPOWER_OFF:    /*SIM卡下电命令*/
            /*为消息申请内存*/
            pstMsg = (USIMM_MsgBlock *)VOS_AllocMsg( WUEPS_PID_AT,
                        sizeof(USIMM_MsgBlock) - VOS_MSG_HEAD_LENGTH );
            if( VOS_NULL_PTR == pstMsg )
            {
                LogPrint( "USIMM_DeactivateCardStart: Allocate Msg Failed." );
                return VOS_ERR;
            }

            /*构造消息并发送*/
            pstMsg->ulReceiverPid      = WUEPS_PID_USIM;
            pstMsg->enMsgType          = USIMM_CMDTYPE_DEACTIVE_REQ;
            if ( VOS_OK != VOS_SendMsg( WUEPS_PID_AT, pstMsg ) )
            {
                LogPrint( "USIMM_DeactivateCardStart: Send Msg Failed." );
                return VOS_ERR;
            }

            break;

        default:

            return AT_CME_INCORRECT_PARAMETERS;
            break;
    }

    return AT_OK;
}

VOS_UINT32 At_TestSimPowerPara (VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "^SIMPOWER:(0,1)");
    return AT_OK;
}
#if (FEATURE_ON == MBB_FEATURE_BIP_TEST)
static TAF_UINT8 gucBipTestClientIndex = AT_MAX_CLIENT_NUM;

VOS_UINT32 AT_Hex_Convert(VOS_UINT16 usLen, const VOS_UINT8 *src, VOS_UINT8 *dst)
{
    VOS_UINT32 ulReslt = AT_FAILURE;
    VOS_UINT8 hexHigh = 0;
    VOS_UINT8 hexLow = 0;

    if ((NULL == src) || (NULL == dst))
    {
        return ulReslt;
    }

    while(usLen > 1)
    {
        ulReslt = AT_ConvertCharToHex(*src, &hexHigh);
        if ( AT_SUCCESS != ulReslt )
        {
            break;
        }
        ulReslt = AT_ConvertCharToHex(*(src + 1), &hexLow);
        if ( AT_SUCCESS != ulReslt )
        {
            break;
        }
        
        *dst++ = ((hexHigh & 0x0F) << 4) | (hexLow & 0x0F);
        src += 2;   /*2 ASCII to 1 Hex*/
        usLen -= 2;  /*2 ASCII to 1 Hex*/
    }
    
    return ulReslt;
}


TAF_UINT32 At_SetBipTestCmd( TAF_UINT8 ucIndex )
{
    TAF_UINT32 ulRslt = AT_ERROR;
    TAF_UINT16 usDataLen;
    TAF_UINT8 *pucData = NULL;
    VOS_UINT8 cmdType;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)    /* 参数检查 */
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (gucAtParaIndex > 1)/* 参数过多 */
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    if (gastAtParaList[0].usParaLen < 12)  /*命令最少需要12个字符长度*/
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    
    usDataLen = (gastAtParaList[0].usParaLen + 1) / 2;  /* 2 ASCII to 1 Hex, so half buffer size is enough*/

    pucData = (TAF_UINT8*)VOS_MemAlloc(WUEPS_PID_AT, DYNAMIC_MEM_PT, usDataLen);
    if (VOS_NULL_PTR == pucData)
    {
        return AT_ERROR;
    }

    ulRslt = AT_Hex_Convert(gastAtParaList[0].usParaLen, gastAtParaList[0].aucPara, pucData);
    if (AT_SUCCESS != ulRslt)
    {
        (VOS_VOID)VOS_MemFree(WUEPS_PID_AT, pucData);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    gucBipTestClientIndex = ucIndex;
    cmdType = pucData[5];   /*get bip command type*/
    ulRslt = STUB_USIMM_SatDataInd(cmdType, usDataLen, pucData);

    ulRslt  = ( VOS_OK == ulRslt )? AT_OK : AT_ERROR;

    (VOS_VOID)VOS_MemFree(WUEPS_PID_AT, pucData);

    return ulRslt;
}


VOS_VOID AT_BipTestResultReport(VOS_VOID *data, BSP_U32 u32Len)
{
    BIP_TestCmdResult_STRU  *pstBipResult = (BIP_TestCmdResult_STRU*)data;
    VOS_UINT16 usLength = 0;
    VOS_UINT8  ucIndex  = 0;
    
    if ( (NULL == data) || (u32Len < sizeof(BIP_TestCmdResult_STRU)) )
    {
        return;
    }
    
    /*打印BIP命令码字*/
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s^BIP:%02x",
                                       gaucAtCrLf,
                                       pstBipResult->ucCmdType);

    /*打印结果码字*/
    for (ucIndex = 0; ucIndex < pstBipResult->ucResultCodeLen; ucIndex++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%02x",
                                       pstBipResult->aucResultCode[ucIndex]);
    }
    
    /*打印附加信息*/
    for (ucIndex = 0; ucIndex < pstBipResult->ucExtendInfoLen; ucIndex++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%04x",
                                       pstBipResult->ausExtInfoData[ucIndex]);
    }

    /*打印结束换行符*/
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (TAF_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);
    
    gstAtSendData.usBufLen = usLength;
    /*打印到串口，STICK使用后台占用PCUI口时，便于串口中调测查看*/
    MBB_AT_ACORE_COMMON_DEBUG("pgucAtSndCodeAddr", pgucAtSndCodeAddr);
    
    /* 回复用户命令结果 */
    At_SendResultData(gucBipTestClientIndex, pgucAtSndCodeAddr, usLength);

    return;
}

#if(FEATURE_ON == MBB_FEATURE_VDF_SIMLOCKEX)

extern VOS_UINT32 At_ParseSimLockPara(
    VOS_UINT8                           *pucData,
    VOS_UINT16                          usLen,
    AT_PARSE_PARA_TYPE_STRU             *pstParalist,
    VOS_UINT32                          ulParaCnt
);


VOS_UINT32 AT_GetSimLockExStatus(VOS_UINT8 ucIndex)
{

    /* 发消息到C核获取SIMLOCK 状态信息 */
    if(TAF_SUCCESS != Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId, 0,
                                    TAF_PH_SIMLOCK_VALUE_PARA, VOS_NULL_PTR))
    {
        AT_WARN_LOG("AT_GetSimLockStatus: Taf_ParaQuery fail.");
        return VOS_ERR;
    }

    /* ^SIMLOCKEX=2查询UE的锁卡状态不在AT命令处理的主流程，需要本地启动保护定时器并更新端口状态 */
    if (AT_SUCCESS != At_StartTimer(AT_SET_PARA_TIME, ucIndex))
    {
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt   = AT_CMD_SIMLOCKEXSTATUS_READ;

    g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_PEND;

    return VOS_OK;
}


VOS_UINT32 At_DispatchSimlockExCmd(
    VOS_UINT8                           ucIndex,
    VOS_UINT32                          ulParaCnt,
    AT_PARSE_PARA_TYPE_STRU             *pstParalist
)
{
    VOS_UINT8                           ucSimLockOP;

    if ((ulParaCnt < 1) || (VOS_NULL_PTR == pstParalist))
    {
        At_FormatResultData(ucIndex, AT_ERROR);
        return AT_FAILURE;
    }

    if ((1 != pstParalist[0].usParaLen)
     || (pstParalist[0].aucPara[0] <'0')
     || (pstParalist[0].aucPara[0] > '2'))
    {
        /*输出错误*/
        At_FormatResultData(ucIndex, AT_ERROR);
        return AT_FAILURE;
    }

    ucSimLockOP = pstParalist[0].aucPara[0] - '0';

    if (AT_SIMLOCK_OPRT_UNLOCK == ucSimLockOP)
    {
        At_UnlockSimLock(ucIndex, ulParaCnt,pstParalist);
    }
    else if (AT_SIMLOCK_OPRT_SET_PLMN_INFO == ucSimLockOP)
    {
        At_SetSimLockPlmnInfo(ucIndex, ulParaCnt,pstParalist);
    }
    else
    {
        /* Added by f62575 for B050 Project, 2012-2-3, Begin   */
        AT_GetSimLockExStatus(ucIndex);
        /* Added by f62575 for B050 Project, 2012-2-3, end   */
    }
    return AT_SUCCESS;
}



VOS_UINT32 At_ProcSimLockExPara(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                           *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_INT8                            cRet;
    VOS_UINT16                          usCmdLen;
    VOS_UINT8                           *pucDataPara = TAF_NULL_PTR;
    VOS_UINT32                          ulParaCnt;
    AT_PARSE_PARA_TYPE_STRU             *pstParalist;
    VOS_UINT32                          ulRslt;

    if (0 == usLen)
    {
        return AT_FAILURE;
    }

    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usLen);

    if (VOS_NULL_PTR == pucDataPara)
    {
        AT_ERR_LOG("At_ProcSimLockPara: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    PS_MEM_CPY(pucDataPara, pucData, usLen);

    /* 待处理的字符串长度小于等于"AT^SIMLOCKEX"长度直接返回AT_FAILURE */
    usCmdLen =(VOS_UINT16)VOS_StrLen("AT^SIMLOCKEX=");

    if (usLen <= usCmdLen)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    /* 待处理的字符串头部不是"AT^SIMLOCKEX"直接返回AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^SIMLOCKEX=", usCmdLen);

    if (0 != cRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    /* 检测参数个数 */
    ulParaCnt = At_GetParaCnt(pucDataPara, usLen);

    if (ulParaCnt < 1)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_ERROR);
        return AT_SUCCESS;
    }

    pstParalist = (AT_PARSE_PARA_TYPE_STRU*)PS_MEM_ALLOC(WUEPS_PID_AT,
                                (ulParaCnt * sizeof(AT_PARSE_PARA_TYPE_STRU)));

    if (VOS_NULL_PTR == pstParalist)
    {
        AT_ERR_LOG("At_ProcSimLockPara: pstParalist Memory malloc failed!");
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_ERROR);
        return AT_SUCCESS;
    }
    else
    {
        PS_MEM_SET(pstParalist, 0x00, (ulParaCnt * sizeof(AT_PARSE_PARA_TYPE_STRU)));
    }

    /* 将 At^simlockex的参数解析到 At格式的参数列表中 */
    ulRslt = At_ParseSimLockPara((pucDataPara + usCmdLen), (usLen - usCmdLen),
                                                    pstParalist, ulParaCnt);

    if (AT_SUCCESS == ulRslt)
    {
        /* 根据at^simlockex=oprt,paralist中的oprt分发 Simlockex的命令处理 */
        At_DispatchSimlockExCmd(ucIndex, ulParaCnt, pstParalist);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    PS_MEM_FREE(WUEPS_PID_AT,pstParalist);
    PS_MEM_FREE(WUEPS_PID_AT,pucDataPara);

    return AT_SUCCESS;

}
#endif


#endif

#if(FEATURE_ON == MBB_FEATURE_CELLROAM)

VOS_UINT32  AT_QryCellRoamPara ( VOS_UINT8 ucIndex )
{
    if (AT_SUCCESS == Taf_ParaQuery(gastAtClientTab[ucIndex].usClientId,0,TAF_PH_CELLROAM_PARA,TAF_NULL_PTR))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CELLROAM_READ;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}

TAF_VOID At_QryParaRspCellRoamProc(
    TAF_UINT8                           ucIndex,
    TAF_UINT8                           OpId,
    TAF_VOID                            *pPara
)
{
    TAF_UINT32                          ulResult = AT_FAILURE;
    TAF_UINT16                          usLength = 0;

    TAF_PH_CELLROAM_STRU                  stCellRoam;

    PS_MEM_SET(&stCellRoam, 0, sizeof(TAF_PH_CELLROAM_STRU));
    PS_MEM_CPY(&stCellRoam, pPara, sizeof(TAF_PH_CELLROAM_STRU));

    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,(TAF_CHAR *)pgucAtSndCodeAddr + usLength,
        "%s:%d,%d",g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                 stCellRoam.RoamMode,
                 stCellRoam.RaMode);

    ulResult = AT_OK;
    gstAtSendData.usBufLen = usLength;
    At_FormatResultData(ucIndex,ulResult);

}
#endif/*MBB_FEATURE_CELLROAM*/

AT_PAR_CMD_ELEMENT_STRU g_astAtPrivateMbbCmdTbl[] = {
#if(FEATURE_ON == MBB_WPG_COMMON)
    {AT_CMD_NDISSTATQRY,
    AT_SetNdisStatPara , AT_SET_PARA_TIME, AT_QryNdisStatPara, AT_QRY_PARA_TIME, At_TestNdisstatqry, AT_TEST_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NDISSTATQRY", (VOS_UINT8*)"(1-11)"},
#if(FEATURE_ON == MBB_FEATURE_VDF_SIMLOCKEX)
    {AT_CMD_SIMLOCKEX,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,    At_QrySimLockPlmnInfo, AT_NOT_SET_TIME,   VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SIMLOCKEX",  VOS_NULL_PTR},
#endif
#endif/*FEATURE_ON == MBB_FEATURE_MPDP*/
    
#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
    {AT_CMD_LTEPROFILE,
    At_SetLteProfilePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_TestLteProfilePara, AT_TEST_PARA_TIME,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8*)"^LTEPROFILE", 
    (VOS_UINT8*)"(0,1),(0-65535),(\"IP\",\"IPV6\",\"IPV4V6\"),(@ImsiPrefix),(APN),(UserName),(UserPwd),(0-3),(@profileName)"},

    {AT_CMD_LTEAPNATTACH,
    AT_SetLteApnAttachSwitch, AT_SET_PARA_TIME, AT_QryApnAttachSwitch, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
    (VOS_UINT8 *)"^LTEAPNATTACH", (VOS_UINT8 *)"(0,1)"},
#endif/*FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST*/
#if (FEATURE_ON == MBB_WPG_HFEATURESTAT)
    {AT_CMD_HFEATURESTAT,
    At_SetHFeaturestat, AT_SET_PARA_TIME, At_QryHFeaturestat, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HFEATURESTAT", (VOS_UINT8*)"(1)"},
#endif/*FEATURE_ON == MBB_WPG_HFEATURESTAT*/

    /*BIP*/
#if (FEATURE_ON == MBB_FEATURE_BIP_TEST)
    {AT_CMD_BIPCMD,
    At_SetBipTestCmd,     AT_SET_PARA_TIME,   TAF_NULL_PTR,    AT_NOT_SET_TIME,   At_CmdTestProcOK, AT_SET_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE | CMD_TBL_NO_LIMITED,
    (TAF_UINT8*)"^BIPCMD",    (TAF_UINT8*)"(text)"},
#endif

    {AT_CMD_DLCK,
    AT_SetDlckPara,     AT_SET_PARA_TIME,    AT_QryDlckPara,    AT_QRY_PARA_TIME,    At_CmdTestDlck,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^DLCK",  (VOS_UINT8*)"(0-3),(oldpassword),(newpassword)"},

    {AT_CMD_NWSCAN,
    AT_SetNwScanPara, AT_NWSCAN_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, 
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    (VOS_UINT32)AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"^NWSCAN", (VOS_UINT8*)"(0-7),(@band),(0-65535)"},

    {(VOS_UINT32)AT_CMD_NWTIME,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,  AT_QryNWTimePara, AT_QRY_PARA_TIME,   
    VOS_NULL_PTR, AT_NOT_SET_TIME, /*lint !e64*/
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    (VOS_UINT32)AT_CME_INCORRECT_PARAMETERS, (VOS_UINT32)CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^NWTIME",   VOS_NULL_PTR},

#if(FEATURE_ON == FEATURE_LTE)
    {AT_CMD_LTERSRP,
    VOS_NULL_PTR,           AT_NOT_SET_TIME,  AT_QryLteRsrp,            AT_QRY_PARA_TIME,   
    AT_TestLteRsrpPara, AT_TEST_PARA_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"^LTERSRP", VOS_NULL_PTR},
#endif

    {(VOS_UINT32)AT_CMD_HCSQ,
    VOS_NULL_PTR,    AT_NOT_SET_TIME,  AT_QryHcsqPara, AT_QRY_PARA_TIME,   
    AT_TestHcsqPara, AT_NOT_SET_TIME, /*lint !e64*/
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    (VOS_UINT32)AT_CME_INCORRECT_PARAMETERS, (VOS_UINT32)CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HCSQ",   VOS_NULL_PTR},
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    {AT_CMD_CMER,
    At_SetCmerPara,     AT_SET_PARA_TIME, VOS_NULL_PTR,   AT_NOT_SET_TIME,     VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"+CMER", (VOS_UINT8*)"(0,3),(0),(0),(0,1),(0)"},
    {AT_CMD_CIND,
    At_SetCindPara,     AT_SET_PARA_TIME, At_QryCindPara,   AT_QRY_PARA_TIME,   At_TestCindPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"+CIND", (VOS_UINT8*)"(0),(0,1),(0,1),(0),(0),(0),(0),(0)"},
#endif

    {AT_CMD_EONS,
    At_SetEonsPara,     AT_SET_PARA_TIME,  VOS_NULL_PTR,  AT_NOT_SET_TIME,   
    AT_TestEonsPara, AT_NOT_SET_TIME, 
    VOS_NULL_PTR,    AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"^EONS",      (VOS_UINT8*)"(1,2,3,4,5),(@plmn)"},

   {AT_CMD_HFREQINFO,
    At_SetHFreqinfo,     AT_SET_PARA_TIME,  At_QryHFreqinfo, AT_QRY_PARA_TIME, At_TestHFreqinfo, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HFREQINFO",    (VOS_UINT8*)"(0,1)"},

#if (FEATURE_ON == MBB_WPG_LTXPOWER)
   {AT_CMD_LTXPOWER,
    VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryLTxPower, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^LTXPOWER", VOS_NULL_PTR},
#endif/*end (FEATURE_ON == MBB_WPG_LTXPOWER)*/

    {AT_CMD_SMSREGST,
    At_SetSmsregstPara,     AT_SET_PARA_TIME, At_QrySmsregstPara,   AT_QRY_PARA_TIME,
    At_TestSmsregst, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,  CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^SMSREGST",    (VOS_UINT8*)"(@imsi)"},

#if (FEATURE_ON == MBB_FEATURE_FAKE_ON_OFF)
    {AT_CMD_CFUN,
    At_SetCfunPara,     AT_SET_PARA_TIME, At_QryCfunPara,   AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"+CFUN",       (VOS_UINT8*)"(0,1,4,5,6,7,8,10,11),(0,1)"},
#else /*FEATURE_ON == MBB_FEATURE_FAKE_ON_OFF*/
    {AT_CMD_CFUN,
    At_SetCfunPara,     AT_SET_PARA_TIME, At_QryCfunPara,   AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_E5_IS_LOCKED | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"+CFUN",       (VOS_UINT8*)"(0,1,4,5,6,7,8),(0,1)"},
#endif /*FEATURE_ON == MBB_FEATURE_FAKE_ON_OFF*/

    {AT_CMD_CCLK,
     At_SetCCLK,     AT_SET_PARA_TIME,  At_QryCCLK, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
     VOS_NULL_PTR,   AT_NOT_SET_TIME,
     AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
     (VOS_UINT8*)"+CCLK",    (VOS_UINT8*)"(@time)"},

    {AT_CMD_CSIM,
    At_SetCsimPara, AT_SET_PARA_TIME, TAF_NULL_PTR,    AT_NOT_SET_TIME,  AT_TestCsimPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5 | CMD_TBL_PIN_IS_LOCKED,
    (TAF_UINT8*)"+CSIM",    (TAF_UINT8*)"(1-520),(cmd)"},

    {AT_CMD_DATACLASS,
     VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryDataclassPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
     VOS_NULL_PTR,   AT_NOT_SET_TIME,
     AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
     (TAF_UINT8*)"^DATACLASS",   VOS_NULL_PTR},
#if(FEATURE_ON == MBB_FEATURE_CELLROAM)
    {AT_CMD_CELLROAM,
    VOS_NULL_PTR,       AT_NOT_SET_TIME,  AT_QryCellRoamPara,               AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,       CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^CELLROAM",    (VOS_UINT8*)"(0-2),(0-3)"},
#endif/*MBB_FEATURE_CELLROAM*/

    {AT_CMD_SIMPOWER,
    At_SetSimPowerPara,AT_SET_PARA_TIME, VOS_NULL_PTR,AT_NOT_SET_TIME,At_TestSimPowerPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_E5,
    (VOS_UINT8*)"^SIMPOWER", (VOS_UINT8*)"(0-1)"},
#if(FEATURE_ON == MBB_FEATURE_BOX_FTEN)
    {AT_CMD_ANT,
    VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryAnt, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
    (VOS_UINT8*)"*ANT", (VOS_UINT8*)"(0,1,2,3,4,99)"},
#endif
#if (FEATURE_ON == MBB_OPERATOR_INDUSTRYCARD)
    {AT_CMD_UPDATEMODE,
    AT_SetUpdateModePara, AT_NOT_SET_TIME, AT_QryUpdateModePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^UPDATEMODE", (VOS_UINT8 *)"(0,1),(0,1)"},

    {AT_CMD_UPDATECHECK,
    AT_UpdateCheck, AT_NOT_SET_TIME, AT_QryUpdateCheck ,AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^UPDATECHECK", (VOS_UINT8*)"(0-8)"},

    {AT_CMD_UPDATEREQ,
    AT_UpdateReq, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^UPDATEREQ", VOS_NULL_PTR},

#endif
};

VOS_UINT32 At_RegisterExPrivateMbbCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astAtPrivateMbbCmdTbl, sizeof(g_astAtPrivateMbbCmdTbl)/sizeof(g_astAtPrivateMbbCmdTbl[0]));
}
#if (FEATURE_ON == MBB_FEATURE_VSIM_HUK)

VOS_UINT32 TAF_MasterKeyHukSet (MN_CLIENT_ID_T ClientId,
                                 MN_OPERATION_ID_T OpId, 
                                 TAF_PH_MASTER_KEY_HUK_STRU *pstMasterKey)
{
    VOS_UINT32 ulRst = TAF_SUCCESS;

    if(NULL == pstMasterKey)
    {
        vos_printf("[TAF_MasterKeyHukSet] : parameter is null!\n");
        return TAF_FAILURE;
    }

#ifdef VSIM_DEBUG_INFO
    vos_printf("[TAF_MasterKeyHukSet] : TAF_MasterKeyHukSet enter, sizeof(TAF_PH_MASTER_KEY_HUK_STRU) = %d!!!\n", sizeof(TAF_PH_MASTER_KEY_HUK_STRU));
#endif

    /* 向MMA发送设置消息*/
    ulRst = MN_FillAndSndAppReqMsg( ClientId, OpId, TAF_MSG_MMA_MASTER_KEY_HUK_HANDLE,
                                  pstMasterKey, sizeof(TAF_PH_MASTER_KEY_HUK_STRU), I0_WUEPS_PID_MMA );

    return ulRst;
}

VOS_UINT32 AT_DieIdHukSet(MN_CLIENT_ID_T ClientId, MN_OPERATION_ID_T OpId, TAF_PH_DIE_ID_HUK_STRU *DieId)
{

    VOS_UINT32 ret = TAF_SUCCESS;

    if(NULL == DieId)
    {
        vos_printf("[AT_DieIdHukSet] : DieId is null!\n");
        return TAF_FAILURE;
    }

    ret = MN_FillAndSndAppReqMsg(ClientId, OpId, TAF_MSG_MMA_DIE_ID_HUK_HANDLE, DieId, sizeof(TAF_PH_DIE_ID_HUK_STRU), I0_WUEPS_PID_MMA);

    return ret;
}

VOS_UINT32 AT_DieIdInfoSet(MN_CLIENT_ID_T ClientId, MN_OPERATION_ID_T OpId, TAF_PH_DIE_ID_INFO_STRU *DieIdInfo)
{

    VOS_UINT32 ret = TAF_SUCCESS;

    if(NULL == DieIdInfo)
    {
        vos_printf("[AT_DieIdInfoSet] : DieId is null!\n");
        return TAF_FAILURE;
    }

    ret = MN_FillAndSndAppReqMsg(ClientId, OpId, TAF_MSG_MMA_DIE_ID_INFO_HANDLE, DieIdInfo, sizeof(TAF_PH_DIE_ID_INFO_STRU), I0_WUEPS_PID_MMA);

    return ret;
}
#endif /*FEATURE_ON == MBB_FEATURE_VSIM_HUK*/
#if (FEATURE_ON == MBB_FEATURE_FAKE_ON_OFF)

VOS_UINT32  AT_RcvMmaCfunSimStatusInd(
    VOS_VOID                           *pMsg)
{
    MODEM_ID_ENUM_UINT16                            enModemId = MODEM_ID_BUTT;
    TAF_MMA_CFUN_USIM_STATUS_IND_STRU              *pstCfunUsimStatusInd = VOS_NULL_PTR;
    AT_USIM_INFO_CTX_STRU                          *pstUsimInfoCtx = VOS_NULL_PTR;

    if (VOS_NULL_PTR == pMsg)
    {
        return VOS_ERR;
    }

    pstCfunUsimStatusInd = (TAF_MMA_CFUN_USIM_STATUS_IND_STRU*)pMsg;

    enModemId = VOS_GetModemIDFromPid(pstCfunUsimStatusInd->ulSenderPid);

    if (enModemId >= MODEM_ID_BUTT)
    {
        return VOS_ERR;
    }

    pstUsimInfoCtx = AT_GetUsimInfoCtxFromModemId(enModemId);

    /*把C核传过来的SIM状态更新给A核的全局变量*/
    pstUsimInfoCtx->enCardStatus = pstCfunUsimStatusInd->ucSimServiceStatus;
    return VOS_OK;
}
#endif/*FEATURE_ON == MBB_FEATURE_FAKE_ON_OFF*/

#if (FEATURE_ON == MBB_OPERATOR_INDUSTRYCARD)

VOS_UINT32 AT_SetUpdateModePara(VOS_UINT8 ucIndex)
{
    /*UpdateMode共有两个元素，自动升级开关、强制升级开关*/
    TAF_UINT8              UpdateMode[UPDATE_MODE_DATA_LEN + 1] = {0};
    VOS_UINT16              usLength = 0;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数过多检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_ERROR;
    }

    PS_MEM_SET(UpdateMode, 0, sizeof(UpdateMode));
    UpdateMode[0] = (TAF_UINT8)gastAtParaList[0].aucPara[0] - '0';
    UpdateMode[1] = (TAF_UINT8)gastAtParaList[1].aucPara[0] - '0';
    UpdateMode[2] = '\0';

    if (AT_CLIENT_TAB_APP_INDEX == ucIndex)
    {
        /*由app侧更新升级模式的全局变量*/
        g_UpdateModeIsNewFlag = APP_INIT_UPDATE_MODE_OK;
        PS_MEM_SET(g_UpdateModeValue, 0, sizeof(g_UpdateModeValue));
        memcpy(g_UpdateModeValue, UpdateMode, UPDATE_MODE_DATA_LEN);
    }
    else
    {
        /*等待app测初始化完成，^UPDATEMODE设置指令才生效*/
        if (!g_UpdateModeIsNewFlag)
        {
            return AT_BUSY;
        }

        /*将^UPDATEMODE:组装进入传输字符串*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)gaucAtCrLf);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)"^UPDATEMODE:");
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%d,%d", UpdateMode[0], UpdateMode[1]);

        /*主动上报AT后面需要添加换行结束符号*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)gaucAtCrLf);
        At_SendResultData(AT_CLIENT_TAB_APP_INDEX, pgucAtSndCodeAddr, usLength);
    }

    return AT_OK;
}



VOS_UINT32 AT_QryUpdateModePara(VOS_UINT8 ucIndex)
{
    /* 命令类型判断 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /*上电后应用先更新g_UpdateModeIsNewFlag变量，AT^UPDATEMODE?指令才可用*/
    if (!g_UpdateModeIsNewFlag)
    {
        vos_printf("AT_SetUpdateModePara: g_UpdateModeIsNewFlag = 0\n");
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                             (TAF_CHAR*)pgucAtSndCodeAddr,
                             (TAF_CHAR*)pgucAtSndCodeAddr,
                             "%s:%d,%d",
                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                             g_UpdateModeValue[0], g_UpdateModeValue[1]);
    return AT_OK;
}


VOS_UINT32 AT_UpdateCheck( VOS_UINT8 ucIndex)
{
    VOS_UINT8 UpdateCheckValue = 0;
    VOS_UINT16 usLength = 0;

    /* 参数过多 */
    if (1 < gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /*应用层触发AT上报给PCUI*/
    if (AT_CLIENT_TAB_APP_INDEX == ucIndex)
    {
        UpdateCheckValue = gastAtParaList[0].ulParaValue;

        if ((UPDATE_VERSION_STAT_MAX < UpdateCheckValue) || (0 == gucAtParaIndex))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        g_UpdateCheckValue  = UpdateCheckValue;

        /*主动上报‘有新版本’、‘新版本正在下载’、‘新版本成功下载’、
        ‘版本下载失败’、‘版本安装失败’五种状态*/
        if ((UPDATE_STATE_HAVE_NEW_VERSION == g_UpdateCheckValue) || (UPDATE_STATE_DLOADING_NEW_VERSION == g_UpdateCheckValue) \
            || (UPDATE_STATE_COMPLETED_DLOAD_NEW_VERSION == g_UpdateCheckValue) \
            || (UPDATE_STATE_FAILED_TO_DLOAD_VERSION == g_UpdateCheckValue) \
            || (UPDATE_STATE_FAILED_TO_INSTALL_VERSION == g_UpdateCheckValue))
        {
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s",
                                               (VOS_CHAR*)gaucAtCrLf);
            /*将^UPDATECHECK:组装进入传输字符串*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s:%d",
                                               (VOS_CHAR*)"^UPDATECHECK", g_UpdateCheckValue);

            /*主动上报AT后面需要添加换行结束符号*/
            usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s",
                                               (VOS_CHAR*)gaucAtCrLf);
            At_SendResultData(AT_CLIENT_TAB_PCUI_INDEX, pgucAtSndCodeAddr, usLength);
        }

        return AT_OK;
    }
    else if (0 == gastAtParaList[0].usParaLen)
    {

        /*将^UPDATECHECK:组装进入传输字符串*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)gaucAtCrLf);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)"^UPDATECHECK");
        /*主动上报AT后面需要添加换行结束符号*/

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)gaucAtCrLf);
        At_SendResultData(AT_CLIENT_TAB_APP_INDEX, pgucAtSndCodeAddr, usLength);
        return AT_OK;
    }

    return AT_CME_INCORRECT_PARAMETERS;

}


VOS_UINT32 AT_QryUpdateCheck(VOS_UINT8 ucIndex)
{
    VOS_UINT8 UpdateCheckValue = 0;
    UpdateCheckValue = gastAtParaList[0].ulParaValue;

    if (UPDATE_VERSION_STAT_MAX < UpdateCheckValue )
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                             (TAF_CHAR*)pgucAtSndCodeAddr,
                             (TAF_CHAR*)pgucAtSndCodeAddr,
                             "%s:%d",
                             g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                             g_UpdateCheckValue);
    return AT_OK;

}



VOS_UINT32 AT_UpdateReq (VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLength = 0;

    if (AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    if (UPDATE_STATE_HAVE_NEW_VERSION == g_UpdateCheckValue)
    {
        /*将^UPDATEREQ:组装进入传输字符串*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)gaucAtCrLf);

        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)"^UPDATEREQ");

        /*主动上报AT后面需要添加换行结束符号*/
        usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s",
                                           (VOS_CHAR*)gaucAtCrLf);
        At_SendResultData(AT_CLIENT_TAB_APP_INDEX, pgucAtSndCodeAddr, usLength);
        return AT_OK;
    }

    return AT_ERROR;
}

#endif /*FEATURE_ON == MBB_OPERATOR_INDUSTRYCARD*/
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



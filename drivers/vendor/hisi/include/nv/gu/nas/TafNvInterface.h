/******************************************************************************

  Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       : NasNvInterface.h
  Description     : NasNvInterface.h header file
  History         :

******************************************************************************/

#ifndef __TAFNVINTERFACE_H__
#define __TAFNVINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 Macro
*****************************************************************************/

#define AT_NVIM_SETZ_LEN                (16)
#define AT_NOTSUPPORT_STR_LEN           (16)

/* ���ȼ���������� */
#define ADS_UL_QUEUE_SCHEDULER_PRI_MAX  (9)

/*WIFI ��� NV����*/
#define AT_WIFI_BASIC_NV_LEN            (116)
#define AT_WIFI_SEC_NV_LEN              (205)

/*WIFI SSID KEY��󳤶�*/
#define AT_WIFI_SSID_LEN_MAX            (33)
#define AT_WIFI_KEY_LEN_MAX             (27)

/* Add by z60575 for multi_ssid, 2012-9-5 begin */
/* ��Ȩģʽ�ַ������� */
#define AT_WIFI_WLAUTHMODE_LEN          (16)

/* ����ģʽ�ַ������� */
#define AT_WIFI_ENCRYPTIONMODES_LEN     (5)

/* WPA�������ַ������� */
#define AT_WIFI_WLWPAPSK_LEN            (65)

/* ���֧��4��SSID */
#define AT_WIFI_MAX_SSID_NUM            (4)

#define AT_WIFI_KEY_NUM                 (AT_WIFI_MAX_SSID_NUM)

#define TAF_CBA_NVIM_MAX_ETWS_DUP_DETECT_SPEC_MCC_NUM            (5)                 /* ��Чʱ���������������ָ��MCC���� */
#define TAF_CBA_NV_MAX_USER_SPEC_ETWS_MSGID_RANGE_NUM            (2)

/* 9130��չIPv6���˴�����չԭ��ֵ������ */
#define TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM      (20)

#define AT_AP_NVIM_XML_RPT_SRV_URL_LEN              (127)
/* Added by l60609 for XML, 2011-08-11 Begin */
#define AT_AP_XML_RPT_SRV_URL_LEN                   (127)
#define AT_AP_XML_RPT_SRV_URL_STR_LEN               (AT_AP_XML_RPT_SRV_URL_LEN + 1)
#define AT_AP_XML_RPT_INFO_TYPE_LEN                 (127)

/*WEB UI ������󳤶�*/
#define AT_WEBUI_PWD_MAX                            (16)
#define AT_WEBUI_PWD_MAX_SET                        (0)
#define AT_WEBUI_PWD_VERIFY                         (1)

#define AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN            (127)
#define AT_AP_NVIM_XML_RPT_INFO_TYPE_STR_LEN        (AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN + 1)
/* PRODUCT NAME*/
#define AT_PRODUCT_NAME_MAX_NUM                     (29)
#define AT_PRODUCT_NAME_LENGHT                      (AT_PRODUCT_NAME_MAX_NUM + 1)

#define TAF_NVIM_DFS_MAX_PROFILE_NUM                (8)

#define TAF_NVIM_DIFF_DFS_NUM                       (8)

#define TAF_NVIM_MAX_APN_LEN                        (99)
#define TAF_NVIM_MAX_APN_STR_LEN                    (TAF_NVIM_MAX_APN_LEN + 1)

#define AT_MAX_ABORT_CMD_STR_LEN                    (16)
#define AT_MAX_ABORT_RSP_STR_LEN                    (16)

#define AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM       (8)

#define AT_NVIM_RIGHT_PWD_LEN                       (16)

#define TAF_PH_NVIM_MAX_GUL_RAT_NUM                 (3)                 /*AT^syscfgex��acqorder�����Ľ��뼼������ */

#define TAF_NVIM_ITEM_IMEI_SIZE                     (16)

#define AT_DISSD_PWD_LEN                            (16)

#define AT_OPWORD_PWD_LEN                           (16)

#define AT_FACINFO_INFO1_LENGTH                     (128)
#define AT_FACINFO_INFO2_LENGTH                     (128)
#define AT_FACINFO_STRING_LENGTH        \
((AT_FACINFO_INFO1_LENGTH + 1) + (AT_FACINFO_INFO2_LENGTH + 1))

#define AT_FACINFO_INFO1_STR_LENGTH                 (AT_FACINFO_INFO1_LENGTH + 1)
#define AT_FACINFO_INFO2_STR_LENGTH                 (AT_FACINFO_INFO2_LENGTH + 1)

#define AT_MDATE_STRING_LENGTH                      (20)

#define MMA_FORB_BAND_NV_MAX_SIZE                   (10)     /* FobBand��NV���� */

#define MMA_OPERTOR_NAME_MAX_SIZE                   (256)

#define TAF_PH_WCDMA_CLASSMAEK1_LEN                         (2)
#define TAF_PH_WCDMA_CLASSMAEK2_LEN                         (4)
#define TAF_PH_WCDMA_CLASSMAEK3_LEN                         (16)

#define TAF_MAX_MFR_ID_LEN                                  (31)
#define TAF_MAX_MFR_ID_STR_LEN                              (TAF_MAX_MFR_ID_LEN + 1)

#define NAS_MMA_NVIM_OPERATOR_NAME_LEN                      (360)

#define TAF_NVIM_ME_PERSONALISATION_PWD_LEN_MAX             (8)

#define TAF_NVIM_MAX_IMSI_LEN                               (15)
#define TAF_NVIM_MAX_IMSI_STR_LEN                           (TAF_NVIM_MAX_IMSI_LEN + 1)

#define TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN             (160)

/*^AUTHDATA�������û������������󳤶�*/
#define TAF_NVIM_MAX_NDIS_USERNAME_LEN_OF_AUTHDATA          (128)                            /* USERNAME Ϊ127 */
#define TAF_NVIM_MAX_NDIS_PASSWORD_LEN_OF_AUTHDATA          (128)                            /* PASSWORD Ϊ127 */

/*^AUTHDATA������<PLMN>��������󳤶�*/
#define TAF_NVIM_MAX_NDIS_PLMN_LEN                          (7)

#define TAF_NVIM_PDP_PARA_LEN                               (1952)

#define TAF_PH_PRODUCT_NAME_LEN                             (15)
#define TAF_PH_PRODUCT_NAME_STR_LEN                         (TAF_PH_PRODUCT_NAME_LEN + 1)

/* ����ҵ������ṹ��NV���д洢��λ�� */
#define MN_MSG_SRV_PARAM_LEN                                (8)                 /* ����ҵ������ṹ��NV���д洢�ĳ��� */
#define MN_MSG_SRV_RCV_SM_ACT_OFFSET                        (0)                 /* ���Ž����ϱ���ʽ��NV���д洢��ƫ�� */
#define MN_MSG_SRV_RCV_SM_MEM_STORE_OFFSET                  (1)                 /* ���Ž��մ洢������NV���д洢��ƫ�� */
#define MN_MSG_SRV_RCV_STARPT_ACT_OFFSET                    (2)                 /* ����״̬��������ϱ���ʽc */
#define MN_MSG_SRV_RCV_STARPT_MEM_STORE_OFFSET              (3)                 /* ����״̬������մ洢������NV���д洢��ƫ�� */
#define MN_MSG_SRV_CBM_MEM_STORE_OFFSET                     (4)                 /* �㲥���Ž��մ洢������NV���д洢��ƫ�� */
#define MN_MSG_SRV_APP_MEM_STATUS_OFFSET                    (5)                 /* APP���Ž��մ洢���ʿ��ñ�־��NV���д洢��ƫ�ƣ���Ӧ�ֽ�ȡֵ����0:�洢���洢���ʲ����� 1:�洢���д洢���ʿ��� */
#define MN_MSG_SRV_SM_MEM_ENABLE_OFFSET                     (6)                 /* ���Ž��մ洢������NV���д洢��־��NV���д洢��ƫ�ƣ���Ӧ�ֽ�ȡֵ����0:��ʹ�� 1:ʹ�� */
#define MN_MSG_SRV_MO_DOMAIN_PROTOCOL_OFFSET                (7)                 /* ���ŷ�����Э��Ҫ��ʵ�ֵ�ƫ�ƣ���Ӧ�ֽ�ȡֵ����0:��ʹ�� 1:ʹ�� */

#define MN_MSG_MAX_EF_LEN                                   (255)
#define MN_MSG_EFSMSS_PARA_LEN                              (256)
#define MN_MSG_EFSMSP_PARA_LEN                              (256)

#define TAF_PH_SIMLOCK_PLMN_STR_LEN                         (8)                 /* Plmn �Ŷγ��� */
#define TAF_MAX_SIM_LOCK_RANGE_NUM                          (20)

/* WINS������NV��Ľṹ�� */
#define WINS_CONFIG_DISABLE                                 (0)                 /* WINS��ʹ�� */
#define WINS_CONFIG_ENABLE                                  (1)                 /* WINSʹ�� */

#define TAF_CBA_NVIM_MAX_CBMID_RANGE_NUM                    (100)               /* �û������������ϢID��Χ�������ڽ���ģʽ�»�Ҫ�ܵ�CBMIR�ļ���С���� */
#define TAF_CBA_NVIM_MAX_LABEL_NUM                          (16)                /* �û����������������Ϣ�ĳ��ȣ���λBYTE */

#define TAF_SVN_DATA_LENGTH                                 (2)                 /* SVN��Ч���ݳ��� */

/*  ��ǰ֧�ֵ�UMTS��codec���ͽ�����3��,��Э���л�����չ����˴˴�Ԥ����չ���ֶ� */
#define MN_CALL_MAX_UMTS_CODEC_TYPE_NUM                     (7)

#define MN_CALL_NVIM_BC_MAX_SPH_VER_NUM                     (6)
#define MN_CALL_NVIM_MAX_CUSTOM_ECC_NUM                     (20)                /* �û����ƵĽ���������������� */

#define MN_CALL_NVIM_MAX_BCD_NUM_LEN                        (20)

#define TAF_AT_NVIM_CLIENT_CONFIG_LEN                       (64)

#define TAF_CBA_NVIM_MAX_CBMID_RANGE_NUM                    (100)               /* �û������������ϢID��Χ�������ڽ���ģʽ�»�Ҫ�ܵ�CBMIR�ļ���С���� */

#define TAF_NVIM_CBA_MAX_LABEL_NUM                          (16)

#define MTA_BODY_SAR_WBAND_MAX_NUM                          (5)
#define MTA_BODY_SAR_GBAND_MAX_NUM                          (4)

#define MTC_RF_LCD_MIPICLK_MAX_NUM                          (8)                 /* MIPICLK������ */
#define MTC_RF_LCD_MIPICLK_FREQ_MAX_NUM                     (8)                 /* ÿ��MIPICLKӰ������Ƶ�ʸ��� */

typedef VOS_UINT8  MN_CALL_STATE_ENUM_UINT8;

typedef VOS_UINT32  MMA_QUICK_START_STA_UINT32;
/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/
/* ME Storage Function On or Off*/
enum MN_MSG_ME_STORAGE_STATUS_ENUM
{
    MN_MSG_ME_STORAGE_DISABLE           = 0x00,
    MN_MSG_ME_STORAGE_ENABLE            = 0x01,
    MN_MSG_ME_STORAGE_BUTT
};
typedef VOS_UINT8 MN_MSG_ME_STORAGE_STATUS_ENUM_UINT8;

enum AT_UART_LINK_TYPE_ENUM
{
    AT_UART_LINK_TYPE_OM        = 1,          /* OMģʽ */
    AT_UART_LINK_TYPE_AT        = 2,          /* ATģʽ */
    AT_UART_LINK_TYPE_BUTT
};
typedef VOS_UINT16 AT_UART_LINK_TYPE_ENUM_UINT16;

enum MMA_CUSTOM_CARDLOCK_OPERATOR_ENUM
{
    MMA_CUSTOM_CARDLOCK_NO_AVAILABLE   = 0,                                     /* 0��Ĭ�ϲ��򿪣�ʹ�ÿ�����д���SIMLOCK */
    MMA_CUSTOM_CARDLOCK_EGYPT_VDF,                                              /* 1���򿪰���VDF���Ƶ�37��SIMLOCK */
    MMA_CUSTOM_CARDLOCK_NORWAY_NETCOM,                                          /* 2����Ų��Netcomm��SIMLOCK */
    MMA_CUSTOM_CARDLOCK_MEXICO_TELCEL,                                          /* 3����ī����TELCEL�����������Ŷ�334020 */
    MMA_CUSTOM_CARDLOCK_DOMINICA_TELCEL,                                        /* 4���򿪶������TELCEL�����������Ŷ�37002��33870��42502 */
    MMA_CUSTOM_CARDLOCK_BUTT
};



enum MTA_WCDMA_BAND_ENUM
{
    MTA_WCDMA_I_2100                    = 0x0001,
    MTA_WCDMA_II_1900,
    MTA_WCDMA_III_1800,
    MTA_WCDMA_IV_1700,
    MTA_WCDMA_V_850,
    MTA_WCDMA_VI_800,
    MTA_WCDMA_VII_2600,
    MTA_WCDMA_VIII_900,
    MTA_WCDMA_IX_J1700,
    /* Ƶ���ݲ�֧��
    MTA_WCDMA_X,
    */
    MTA_WCDMA_XI_1500                   = 0x000B,
    /* ����Ƶ���ݲ�֧��
    MTA_WCDMA_XII,
    MTA_WCDMA_XIII,
    MTA_WCDMA_XIV,
    MTA_WCDMA_XV,
    MTA_WCDMA_XVI,
    MTA_WCDMA_XVII,
    MTA_WCDMA_XVIII,
    */
    MTA_WCDMA_XIX_850                   = 0x0013,

    MTA_WCDMA_BAND_BUTT
};
typedef VOS_UINT16 MTA_WCDMA_BAND_ENUM_UINT16;


enum TAF_NVIM_LC_WORK_CFG_ENUM
{
    TAF_NVIM_LC_INDEPENT_WORK = 0,
    TAF_NVIM_LC_INTER_WORK    = 1,
    TAF_NVIM_LC_WORK_CFG_BUTT
};
typedef VOS_UINT8 TAF_NVIM_LC_WORK_CFG_ENUM_UINT8;


enum TAF_NVIM_LC_RAT_COMBINED_ENUM
{
    TAF_NVIM_LC_RAT_COMBINED_GUL  = 0x55,
    TAF_NVIM_LC_RAT_COMBINED_CL   = 0xAA,
    TAF_NVIM_LC_RAT_COMBINED_BUTT
};
typedef VOS_UINT8 TAF_NVIM_LC_RAT_COMBINED_ENUM_UINT8;


enum MTC_PS_TRANSFER_ENUM
{
    MTC_PS_TRANSFER_NONE                = 0x00,                                 /* ��PS��Ǩ�Ʋ��� */
    MTC_PS_TRANSFER_LOST_AREA           = 0x01,                                 /* ������ѡ���� */
    MTC_PS_TRANSFER_OFF_AREA            = 0x02,                                 /* ������ѡ���� */

    MTC_PS_TRANSFER_SOLUTION_BUTT
};
typedef VOS_UINT8 MTC_PS_TRANSFER_ENUM_UINT8;


enum TAF_FLASH_DIRECTORY_TYPE_ENUM
{

    /* V3R3�汾��E5��STICK */
    TAF_FLASH_DIRECTORY_TYPE_V3R3E5_V3R3STICK               = 0x00,

    /* V7R2�汾��V3R3�汾��M2M��Ʒ��̬ */
    TAF_FLASH_DIRECTORY_TYPE_V7R2_V3R3M2M                   = 0x01,

    /* V9R1�ֻ� */
    TAF_FLASH_DIRECTORY_TYPE_V9R1PHONE                      = 0x02,

    /* FLASH�ļ��洢��һ·������ǰ��������K3V3V8R1�汾 */
    TAF_FLASH_DIRECTORY_TYPE_K3V3V8R1                       = 0x03,

    TAF_FLASH_DIRECTORY_TYPE_BUTT
};
typedef VOS_UINT8 TAF_FLASH_DIRECTORY_TYPE_ENUM_UINT16;


enum TAF_NVIM_RAT_MODE_ENUM
{
    TAF_NVIM_RAT_MODE_GSM               = 0x01,
    TAF_NVIM_RAT_MODE_WCDMA,
    TAF_NVIM_RAT_MODE_LTE,
    TAF_NVIM_RAT_MODE_CDMA1X,
    TAF_NVIM_RAT_MODE_TDSCDMA,
    TAF_NVIM_RAT_MODE_WIMAX,
    TAF_NVIM_RAT_MODE_EVDO,

    TAF_NVIM_RAT_MODE_BUTT
};
typedef VOS_UINT8 TAF_NVIM_RAT_MODE_ENUM_UINT8;


enum TAF_NVIM_GSM_BAND_ENUM
{
    TAF_NVIM_GSM_BAND_850               = 0,
    TAF_NVIM_GSM_BAND_900,
    TAF_NVIM_GSM_BAND_1800,
    TAF_NVIM_GSM_BAND_1900,

    TAF_NVIM_GSM_BAND_BUTT
};
typedef VOS_UINT16 TAF_NVIM_GSM_BAND_ENUM_UINT16;


/*****************************************************************************
  5 STRUCT
*****************************************************************************/

typedef struct
{
    VOS_CHAR    acTz[AT_NVIM_SETZ_LEN];
}TAF_AT_TZ_STRU;


typedef struct
{
    VOS_CHAR    acErrorText[AT_NOTSUPPORT_STR_LEN];
}TAF_AT_NOT_SUPPORT_CMD_ERROR_TEXT_STRU;


typedef struct
{
    /* ��NV���״̬ 0:δʹ�ܣ���ʾ���������ȼ����ȼ����ȴ��� 1:ʹ�ܣ��������ȼ��㷨 */
    VOS_UINT32                              ulStatus;

    /* ���ж������ȼ��ļ�Ȩ�� */
    VOS_UINT16                              ausPriWeightedNum[ADS_UL_QUEUE_SCHEDULER_PRI_MAX];
    VOS_UINT8                               aucRsv[2];
}ADS_UL_QUEUE_SCHEDULER_PRI_NV_STRU;



typedef struct
{

    VOS_UINT8    aucWifiSsid[AT_WIFI_MAX_SSID_NUM][AT_WIFI_SSID_LEN_MAX];
    VOS_UINT8    aucReserved[84];
}TAF_AT_MULTI_WIFI_SSID_STRU;


typedef struct
{
    VOS_UINT8    aucWifiAuthmode[AT_WIFI_WLAUTHMODE_LEN];
    VOS_UINT8    aucWifiBasicencryptionmodes[AT_WIFI_ENCRYPTIONMODES_LEN];
    VOS_UINT8    aucWifiWpaencryptionmodes[AT_WIFI_ENCRYPTIONMODES_LEN];
    VOS_UINT8    aucWifiWepKey1[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    aucWifiWepKey2[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    aucWifiWepKey3[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    aucWifiWepKey4[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    ucWifiWepKeyIndex[AT_WIFI_MAX_SSID_NUM];
    VOS_UINT8    aucWifiWpapsk[AT_WIFI_MAX_SSID_NUM][AT_WIFI_WLWPAPSK_LEN];
    VOS_UINT8    ucWifiWpsenbl;
    VOS_UINT8    ucWifiWpscfg;
}TAF_AT_MULTI_WIFI_SEC_STRU;


typedef struct
{
    VOS_UINT8 aucApRptSrvUrl[AT_AP_XML_RPT_SRV_URL_STR_LEN];
}TAF_AT_NVIM_AP_RPT_SRV_URL_STRU;


typedef struct
{
    VOS_UINT8 aucWebPwd[AT_WEBUI_PWD_MAX];
}TAF_AT_NVIM_WEB_ADMIN_PASSWORD_STRU;


typedef struct
{
    VOS_UINT8 aucApXmlInfoType[AT_AP_NVIM_XML_RPT_INFO_TYPE_STR_LEN];
}TAF_AT_NVIM_AP_XML_INFO_TYPE_STRU;


typedef struct
{
    VOS_UINT32  ulNvStatus;
    VOS_UINT8   aucProductId[AT_PRODUCT_NAME_LENGHT];   /* product id */
}TAF_AT_PRODUCT_ID_STRU;

/* ����TIM���Ŵ�����NV��ṹ�� */

typedef struct
{
    VOS_UINT8                           ucStatus;                               /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucErrCodeRpt;                           /*�������ϱ���־1: 0�����ϱ�,  1���ϱ�*/
}NAS_NV_PPP_DIAL_ERR_CODE_STRU;


typedef struct
{
    VOS_UINT8                           ucAbortEnableFlg;                           /* AT��Ͽ��ر�־ */
    VOS_UINT8                           ucRsv;
    VOS_UINT8                           aucAbortAtCmdStr[AT_MAX_ABORT_CMD_STR_LEN]; /* ���AT������ */
    VOS_UINT8                           aucAbortAtRspStr[AT_MAX_ABORT_RSP_STR_LEN]; /* �������ķ��ؽ�� */
}AT_NVIM_ABORT_CMD_PARA_STRU;


typedef struct
{
    VOS_UINT8                   ucParaNum;                                      /* ��������������� */
    VOS_UINT8                   ucRsv[3];                                       /* ����λ */
    VOS_INT16                   asPower[AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM];      /* GƵ�ι�������ֵ */
    VOS_UINT32                  aulBand[AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM];      /* GƵ��λ�� */
}AT_BODYSARGSM_SET_PARA_STRU;


typedef struct
{
    VOS_UINT32                          enRightOpenFlg;
    VOS_INT8                            acPassword[AT_NVIM_RIGHT_PWD_LEN];
}TAF_AT_NVIM_RIGHT_OPEN_FLAG_STRU;


typedef struct
{
    VOS_UINT32 ulDSTotalSendFluxLow;
    VOS_UINT32 ulDSTotalSendFluxHigh;
    VOS_UINT32 ulDSTotalReceiveFluxLow;
    VOS_UINT32 ulDSTotalReceiveFluxHigh;
    TAF_AT_NVIM_RIGHT_OPEN_FLAG_STRU stRightPwd;
}NAS_NV_RABM_TOTAL_RX_BYTES_STRU;


typedef struct
{
    VOS_UINT8                          aucImei[TAF_NVIM_ITEM_IMEI_SIZE];
}IMEI_STRU;


typedef struct
{
    VOS_INT8                           acATE5DissdPwd[AT_DISSD_PWD_LEN];
}TAF_AT_NVIM_DISSD_PWD_STRU;


typedef struct
{
    VOS_INT8                           acATOpwordPwd[AT_OPWORD_PWD_LEN];
}TAF_AT_NVIM_DISLOG_PWD_NEW_STRU;


typedef struct
{
    VOS_UINT16  usEqver;
}TAF_AT_EQ_VER_STRU;


typedef struct
{
    VOS_UINT16  usCsver;
}TAF_NVIM_CS_VER_STRU;


typedef struct
{
    VOS_UINT8   ucEnableFlag;
    VOS_UINT8   ucReserve;
}NAS_RABM_NVIM_FASTDORM_ENABLE_FLG_STRU;

/*****************************************************************************
 �ṹ����   : AT_IPV6_CAPABILITY_STRU
 Э�����   :
 ASN.1 ���� :
 �ṹ˵��   : IPV6����NV����ƽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucStatus;           /* NV��Ч��־, 1: ��Ч��0����Ч */
    VOS_UINT8                           ucIpv6Capablity;    /* IPV6���� */
    VOS_UINT8                           aucReversed[2];     /* ���ֽڶ��� */

} AT_NV_IPV6_CAPABILITY_STRU;


typedef struct
{

    VOS_UINT8   ucGsmConnectRate;
    VOS_UINT8   ucGprsConnectRate;
    VOS_UINT8   ucEdgeConnectRate;
    VOS_UINT8   ucWcdmaConnectRate;
    VOS_UINT8   ucDpaConnectRate;
    VOS_UINT8   ucReserve;
}AT_NVIM_DIAL_CONNECT_DISPLAY_RATE_STRU;

/*****************************************************************************
 �ṹ��    : AT_TRAFFIC_CLASS_CUSTOMIZE_STRU
 �ṹ˵��  : ���ڶ���PDP����������QoS�� Traffic Class��ֵ�Ľṹ��
*****************************************************************************/

typedef struct
{
    VOS_UINT8                          ucStatus;                         /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                          ucTrafficClass;                   /* Traffic Class��ֵ */
}AT_TRAFFIC_CLASS_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־,  */
    VOS_UINT8                           ucSsCmdCustomize;
    VOS_UINT8                           aucReserved1[2];
} AT_SS_CUSTOMIZE_PARA_STRU;


typedef struct
{
    VOS_UINT32 ulCimiPortCfg;
}TAF_AT_NVIM_CIMI_PORT_CFG_STRU;


typedef struct
{
    VOS_UINT32 ulMuxReportCfg;
}TAF_AT_NVIM_MUX_REPORT_CFG_STRU;


typedef struct
{
    VOS_UINT32 ulTotalMsg;
}NAS_MN_NVIM_TOTAL_MSG_STRU;


typedef struct
{
    VOS_UINT8   ucApXmlRptFlg;
    VOS_UINT8   ucReserve[3];
}TAF_AT_NVIM_AP_XML_RPT_FLG_STRU;

/* Added by l60609 for XML, 2011-08-11 End */


typedef struct
{
    VOS_UINT16                          usModemId;                              /* �ö˿������ĸ�modem */
    VOS_UINT8                           ucReportFlg;                            /* �ö˿��Ƿ����������ϱ���VOS_TRUEΪ������VOS_FALSEΪ��������Ĭ������ */
    VOS_UINT8                           aucRsv[1];
}AT_NVIM_CLIENT_CONFIGURATION_STRU;


typedef struct
{
    VOS_UINT8 aucForband[MMA_FORB_BAND_NV_MAX_SIZE];
}NAS_MMA_NVIM_FORBAND_STRU;



typedef struct
{
    VOS_UINT8 aucRcvData[MMA_OPERTOR_NAME_MAX_SIZE];
}NAS_MMA_NVIM_OPERATOR_NAME_STRU;


typedef struct
{
    MMA_QUICK_START_STA_UINT32          ulQuickStartSta;
}NAS_NVIM_FOLLOWON_OPENSPEED_FLAG_STRU;


typedef struct
{
    VOS_UINT8 aucClassmark1[TAF_PH_WCDMA_CLASSMAEK1_LEN];
}NAS_MMA_NVIM_CLASSMARK1_STRU;


typedef struct
{
    VOS_UINT8 aucClassmark2[TAF_PH_WCDMA_CLASSMAEK2_LEN];
}NAS_MMA_NVIM_CLASSMARK2_STRU;


typedef struct
{
    VOS_UINT8 aucClassmark3[TAF_PH_WCDMA_CLASSMAEK3_LEN];
}NAS_MMA_NVIM_CLASSMARK3_STRU;


typedef struct
{
    VOS_UINT8   aucSmsServicePara[MN_MSG_SRV_PARAM_LEN];
}TAF_NVIM_SMS_SERVICE_PARA_STRU;


typedef struct
{
    VOS_UINT8   aucSmsEfsmssPara[MN_MSG_EFSMSS_PARA_LEN];
}TAF_MMA_NVIM_SMS_EFSMSS_PARA_STRU;


typedef struct
{
    VOS_UINT8   aucSmsEfsmspPara[MN_MSG_EFSMSP_PARA_LEN];
}TAF_MMA_NVIM_SMS_EFSMSP_PARA_STRU;


typedef struct
{
    VOS_UINT8  aucPwd[TAF_NVIM_ME_PERSONALISATION_PWD_LEN_MAX];
}TAF_MMA_SIM_PERSONAL_PWD_STRU;


typedef struct
{
    VOS_UINT8  aucImsiStr[TAF_NVIM_MAX_IMSI_STR_LEN];
}NAS_MMA_SIM_PERSONAL_IMST_STRU;


typedef struct
{
    VOS_UINT8  aucDisplaySpnFlag[2];
}NAS_MMA_NVIM_DISPLAY_SPN_FLAG_STRU;


typedef struct
{
    VOS_UINT8   ucVaild;
    VOS_UINT8   ucReserved;
}TAF_AT_NVIM_RXDIV_CONFIG_STRU;


typedef struct
{
    VOS_INT8    cStatus;
    VOS_UINT8   ucEncodeType;
    VOS_UINT32  ulLength;
    VOS_UINT8   ucData[TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN];
}TAF_AT_NVIM_SMS_ACTIVE_MESSAGE_STRU;

/*8301-8312���ýṹ��*/

typedef struct
{
    VOS_UINT8                           ucAuthType;
    VOS_UINT8                           aucPlmn[TAF_NVIM_MAX_NDIS_PLMN_LEN];
    VOS_UINT8                           aucPassword[TAF_NVIM_MAX_NDIS_PASSWORD_LEN_OF_AUTHDATA];
    VOS_UINT8                           aucUsername[TAF_NVIM_MAX_NDIS_USERNAME_LEN_OF_AUTHDATA];
}TAF_NVIM_NDIS_AUTHDATA_STRU;


typedef struct
{
    VOS_UINT8                           ucUsed;                                             /*0��δʹ�ã�1��ʹ��*/
    VOS_UINT8                           aucRsv[3];
    TAF_NVIM_NDIS_AUTHDATA_STRU         stAuthData;                                         /*��Ȩ����*/
}TAF_NVIM_NDIS_AUTHDATA_TABLE_STRU;

/*8451-8462���ýṹ�� en_NV_Item_Taf_PdpPara_0*/

typedef struct
{
    VOS_UINT8          aucPdpPara[TAF_NVIM_PDP_PARA_LEN];
}TAF_NVIM_PDP_PARA_STRU;

/* en_NV_Item_ProductName 8205 */

typedef struct
{
    VOS_UINT8          aucProductName[TAF_PH_PRODUCT_NAME_STR_LEN];
}TAF_PH_PRODUCT_NAME_STRU;

/*en_NV_Item_Imei_Svn 8337*/

typedef struct
{
    VOS_UINT8                           ucActiveFlag;
    VOS_UINT8                           aucSvn[TAF_SVN_DATA_LENGTH];
    VOS_UINT8                           aucReserve[1];
}TAF_SVN_DATA_STRU;

/*en_NV_Item_SMS_MO_RETRY_PERIOD 8293*/

typedef struct
{
    VOS_UINT8                           ucActFlg;                                           /* NVIM�и����Ƿ񼤻� */
    VOS_UINT8                           ucReserved[3];
    VOS_UINT32                          ulRetryPeriod;                                      /*�����ط�������*/
}MN_MSG_NVIM_RETRY_PERIOD_STRU;

/*en_NV_Item_SMS_MO_RETRY_INTERVAL 8294*/

typedef struct
{
    VOS_UINT8                           ucActFlg;                               /* NVIM�и����Ƿ񼤻� */
    VOS_UINT8                           ucReserved[3];
    VOS_UINT32                          ulRetryInterval;                        /*�����ط���ʱ����*/
}MN_MSG_NVIM_RETRY_INTERVAL_STRU;

/*en_NV_Item_SMS_SEND_DOMAIN 8295*/
/* NVIM���ж��ŷ�����Ľṹ */

typedef struct
{
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           ucSendDomain;
}AT_NVIM_SEND_DOMAIN_STRU;

/*en_NV_Item_WINS_Config 8297*/

typedef struct
{
    VOS_UINT8   ucStatus;        /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8   ucWins;          /* WINSʹ�ܱ��: 0��Disable,  1��Enable */
}WINS_CONFIG_STRU;

/*en_NV_Item_CustomizeSimLockPlmnInfo 8267*/

typedef struct
{
    VOS_UINT8                           ucMncNum;
    VOS_UINT8                           aucRangeBegin[TAF_PH_SIMLOCK_PLMN_STR_LEN];
    VOS_UINT8                           aucRangeEnd[TAF_PH_SIMLOCK_PLMN_STR_LEN];
}TAF_CUSTOM_SIM_LOCK_PLMN_RANGE_STRU;


typedef struct
{
    VOS_UINT32                          ulStatus;/*�Ƿ񼤻0�����1���� */
    TAF_CUSTOM_SIM_LOCK_PLMN_RANGE_STRU astSimLockPlmnRange[TAF_MAX_SIM_LOCK_RANGE_NUM];
}TAF_CUSTOM_SIM_LOCK_PLMN_INFO_STRU;

/* en_NV_Item_CardlockStatus 8268 */

typedef struct
{
    VOS_UINT32                          ulStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT32                          ulCardlockStatus;    /**/
    VOS_UINT32                          ulRemainUnlockTimes; /*����ʣ�����*/
}TAF_NVIM_CUSTOM_CARDLOCK_STATUS_STRU;

/*en_NV_Item_CustomizeSimLockMaxTimes 8269*/

typedef struct
{
    VOS_UINT32                          ulStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT32                          ulLockMaxTimes;
}TAF_CUSTOM_SIM_LOCK_MAX_TIMES_STRU;

/*en_NV_Item_CCA_TelePara 8230*/

/*Ϊ����V1R1NV��ƥ��ǰ15���ֽڱ���*/
typedef struct
{
    VOS_UINT8               aucRsv[15];              /*NV��Ľṹ�У�4�ֽڶ��뷽ʽ���пն�Ҫ�ֶ�����*/
    VOS_UINT8               ucS0TimerLen;
} TAF_CCA_TELE_PARA_STRU;

/*en_NV_Item_PS_TelePara 8231*/

typedef struct
{
    VOS_UINT8         AnsMode;
    VOS_UINT8         AnsType;
    VOS_UINT16        ClientId;
}TAF_APS_NVIM_PS_ANS_MODE_STRU;

/*en_NV_Item_User_Set_Freqbands 8265*/
/*��ȡbandֵ����MS��RACIEZ�л��*/
/*
  80��CM_BAND_PREF_GSM_DCS_1800��              GSM DCS systems
  100��CM_BAND_PREF_GSM_EGSM_900��             Extended GSM 900
  200��CM_BAND_PREF_GSM_PGSM_900��             Primary GSM 900
  100000��CM_BAND_PREF_GSM_RGSM_900��          GSM Railway GSM 900
  200000��CM_BAND_PREF_GSM_PCS_1900��          GSM PCS
  400000��CM_BAND_PREF_WCDMA_I_IMT_2000��      WCDMA IMT 2000
  3FFFFFFF��CM_BAND_PREF_ANY��                 �κ�Ƶ��
  40000000��CM_BAND_PREF_NO_CHANGE��           Ƶ�����仯
*/

typedef struct
{
    VOS_UINT32 ulBandLow; /*��32λ*/
    VOS_UINT32 ulBandHigh;/*��32λ*/
} TAF_MMA_NVIM_USER_SET_PREF_BAND64;


typedef struct
{
    VOS_UINT16  usReportRegActFlg;
}TAF_AT_NVIM_REPORT_REG_ACT_FLG_STRU;


typedef struct
{
    VOS_UINT16  usMePersonalActFlag;
}NAS_MMA_ME_PERSONAL_ACT_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           ucCbStatus;         /* CBSҵ���Ƿ����ñ�־*/

    VOS_UINT8                           ucDupDetectCfg;     /* �ظ��������� */

    VOS_UINT8                           ucRptAppFullPageFlg; /* VOS_TRUE: �ϱ���ҳ��88���ֽ�; VOS_FALSE: �ϱ�ʵ����Ч�ֽ� */

    VOS_UINT8                           ucRsv;              /* NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն� */
}TAF_CBA_NVIM_CBS_SERVICE_PARM_STRU;


typedef struct
{
    VOS_UINT32                          ulCommDebugFlag;
}TAF_AT_NVIM_COMMDEGBUG_CFG_STRU;


typedef struct
{
    AT_UART_LINK_TYPE_ENUM_UINT16       enUartLinkType;
}TAF_AT_NVIM_DEFAULT_LINK_OF_UART_STRU;


typedef struct
{
    VOS_UINT8 aucMfrId[TAF_MAX_MFR_ID_STR_LEN];
}TAF_PH_FMR_ID_STRU;


typedef struct
{
    VOS_UINT8 aucOperatorName[NAS_MMA_NVIM_OPERATOR_NAME_LEN];
}NAS_MMA_OPERATOR_NAME_STRU;


typedef struct
{
    VOS_UINT8                           aucFactInfo1[AT_FACINFO_INFO1_STR_LENGTH];
    VOS_UINT8                           aucFactInfo2[AT_FACINFO_INFO2_STR_LENGTH];
}TAF_AT_NVIM_FACTORY_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucMDate[AT_MDATE_STRING_LENGTH];
}TAF_AT_NVIM_MANUFACTURE_DATE_STRU;


typedef struct
{
    VOS_UINT16 usPcVoiceSupportFlag; /*Range:[0,1]*/
}APP_VC_NVIM_PC_VOICE_SUPPORT_FLAG_STRU;




typedef struct
{
    MN_MSG_ME_STORAGE_STATUS_ENUM_UINT8 enMeStorageStatus;                      /* ME�洢���Ź����Ƿ����ñ�־*/
    VOS_UINT8                           aucReserve[1];                          /* NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն� */
    VOS_UINT16                          usMeStorageNum;                         /* ME�洢�������� */
}MN_MSG_ME_STORAGE_PARM_STRU;

typedef struct
{
    VOS_UINT32      ulDSLastLinkTime;                       /*DS���һ������ʱ��*/
    VOS_UINT32      ulDSTotalSendFluxLow;                   /*DS�ۼƷ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalSendFluxHigh;                  /*DS�ۼƷ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalLinkTime;                      /*DS�ۼ�����ʱ��*/
    VOS_UINT32      ulDSTotalReceiveFluxLow;                /*DS�ۼƽ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalReceiveFluxHigh;               /*DS�ۼƽ����������ĸ��ֽ�*/

} TAF_APS_DSFLOW_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlg;/* MN_MSG_NVIM_ITEM_ACTIVE */
    VOS_UINT8                           aucReserved[3];
}MN_MSG_DISCARD_DELIVER_MSG_STRU;

typedef struct
{
    VOS_UINT8                           ucActFlg;/* MN_MSG_NVIM_ITEM_ACTIVE */
    VOS_UINT8                           aucReserved[3];
}MN_MSG_REPLACE_DELIVER_MSG_STRU;

/* Added by z40661 for AMR-WB , 2012-02-09 , end */

typedef struct
{
    VOS_UINT8       ucDsFlowStatsRptCtrl;                   /* ��ʶ�Ƿ���������ϱ� */
    VOS_UINT8       ucDsFlowStatsSave2NvCtrl;               /* ��ʶ�Ƿ���Ҫ������ʷ������Ϣ��NV�� */
    VOS_UINT8       ucDsFlowSavePeriod;                     /* ����дNV������ */
    VOS_UINT8       aucReserve[1];                          /* ����λ*/
} TAF_APS_DSFLOW_STATS_CTRL_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucRatOrderNum;                          /* syscfgex�����õ�acqoder�е�ָʾ���� */
    VOS_UINT8                           aenRatOrder[TAF_PH_NVIM_MAX_GUL_RAT_NUM];    /* at^syscfgex�����õ�acqoder��˳�� */
}TAF_PH_NVIM_RAT_ORDER_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           aucReserved[3];
}MN_MSG_NVIM_MO_SMS_CTRL_STRU;



typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;                        /* en_NV_Item_PDP_Actving_Limit NV���Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucReserved;                             /* ���� */
}TAF_APS_NVIM_PDP_ACT_LIMIT_STRU;

typedef struct
{
    VOS_UINT8                           ucCategory;                             /* �������������� */
    VOS_UINT8                           ucValidSimPresent;                      /* �������������п�ʱ��Ч�����޿�ʱ��Ч��1�п�ʱ��Ч��0�޿�ʱ��Ч */
    VOS_UINT8                           ucReserved;
    VOS_UINT8                           ucEccNumLen;
    VOS_UINT8                           aucEccNum[MN_CALL_NVIM_MAX_BCD_NUM_LEN];
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
} MN_CALL_NVIM_CUSTOM_ECC_NUM_STRU;



typedef struct
{
    VOS_UINT8                           ucEccNumCount;
    VOS_UINT8                           aucReserve[3];
    MN_CALL_NVIM_CUSTOM_ECC_NUM_STRU    astCustomEccNumList[MN_CALL_NVIM_MAX_CUSTOM_ECC_NUM];
} MN_CALL_NVIM_CUSTOM_ECC_NUM_LIST_STRU;


typedef struct
{
    VOS_UINT8                           ucSimCallCtrlSupportFlg;                /* SIM��Call Controlҵ��*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_CALL_NVIM_SIM_CALL_CONTROL_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucCallDeflectionSupportFlg;             /* ֧�ֺ���ƫתҵ��*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_CALL_NVIM_CALL_DEFLECTION_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucAlsSupportFlg;                        /* ֧����·�л�ҵ��*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_CALL_NVIM_ALS_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucGetCsmpParaFromUsimSupportFlg;        /*��(U)SIM���ж�ȡCSMP����*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_MSG_GET_CSMP_PARA_FROM_USIM_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucSmsPpDownlodSupportFlg;               /*�����Ƿ�֧�ֶ��ŵ�PP-DOWNLOAD����*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_MSG_SMS_PP_DOWNLOAD_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucSmsNvSmsRexitSupportFlg;              /*����������ΪPS ONLYʱ���Ƿ�֧��CS����źͺ���ҵ��(�������г���) */
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_MSG_SMS_NVIM_SMSREXIST_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucSmsStatusInEfsmsSupportFlg;           /* ����NVIM���Ƿ��ܱ������״̬����*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_MSG_SMS_STATUS_IN_EFSMS_SUPPORT_FLG_STRU;


/* Added by z40661 for AMR-WB , 2012-02-09 , begin */



typedef struct
{
    VOS_UINT8                           ucCodecTypeNum;
    VOS_UINT8                           aucCodecType[MN_CALL_NVIM_BC_MAX_SPH_VER_NUM];
    VOS_UINT8                           ucReserve;
}MN_CALL_NIMV_ITEM_CODEC_TYPE_STRU;

/* Added by z40661 for AMR-WB , 2012-02-09 , end */

	/* Added by f62575 for C50_IPC Project, 2012/02/23, begin */

typedef struct
{
    VOS_UINT32       ulMeStatus;
}TAF_FDN_NVIM_CONFIG_STRU;
/* Added by f62575 for C50_IPC Project, 2012/02/23, end   */


typedef struct
{
    VOS_UINT8                           ucVoiceCallNotSupportedCause;
    VOS_UINT8                           ucVideoCallNotSupportedCause;
    VOS_UINT8                           aucReserved1[2];
}TAF_CALL_NVIM_CALL_NOT_SUPPORTED_CAUSE_STRU;



typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;                        /* en_NV_Item_Network_Selection_Menu_Ctrl_Para NV���Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucReserved;                             /* ���� */
}TAF_MMA_NVIM_REPORT_PLMN_SUPPORT_FLG_STRU;

	
typedef struct
{
    VOS_UINT8                           ucNotDisplayLocalNetworkNameFlg;
    VOS_UINT8                           ucReserved;
}TAF_MMA_NOT_DISPLAY_LOCAL_NETWORKNAME_NVIM_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlag;
    VOS_UINT8                           enMtCustomize;
    VOS_UINT8                           aucReserved1[2];
}MN_MSG_MT_CUSTOMIZE_INFO_STRU;



typedef struct
{
    VOS_UINT8  ucCnt;
    VOS_UINT8  aucUmtsCodec[MN_CALL_MAX_UMTS_CODEC_TYPE_NUM];
} MN_CALL_UMTS_CODEC_TYPE_STRU;

typedef struct
{
    VOS_UINT8                           ucStatus;/*�Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucCardLockPerm;
}MMA_CUSTOM_CARDLOCK_PERM_STRU;


typedef struct
{
    VOS_UINT16                          usMsgIdFrom;                            /*Cell broadcast message id value range from  */
    VOS_UINT16                          usMsgIdTo;                              /*Cell broadcast message id value range to    */
}TAF_CBA_NVIM_ETWS_MSGID_RANGE_STRU;

typedef struct
{
    VOS_UINT8                                               ucEnhDupDetcFlg;    /* DoCoMo��ǿ���ظ���ⶨ�����ԣ���ETWS����֪ͨ����ǿ���ظ���������Ƿ񼤻�, VOS_TRUE:����,VOS_FALSE:δ����.
                                                                                        1. ����֪ͨ,������ͨCBS��Ϣ, �ظ����ʱ��Ҫ�ж�PLMN
                                                                                        2. ����֪ͨ,������ͨCBS��Ϣ, �ظ�����ʱ������ݵ�ǰPLMN��MCC��������
                                                                                        3. ETWS��Ϣ����ͨCBS��Ϣ����ͬ�Ĺ������ظ����˺�ʱЧ��� */
    VOS_UINT8                                               ucRsv;

    VOS_UINT16                                              usNormalTimeLen;   /* ��λ:����, ��������δ����ʱʹ�ô���Чʱ�� */

    VOS_UINT16                                              usSpecMccTimeLen;  /* ��λ:����, ��ǿ���ظ���ⶨ�����Լ���ʱ��/��֪ͨ/CBS��MCC��ָ��Mcc����ʹ�ô���Чʱ�� */
    VOS_UINT16                                              usOtherMccTimeLen; /* ��λ:����, ��ǿ���ظ���ⶨ�����Լ���ʱ��/��֪ͨ/CBS��MCC����ָ��Mcc����ʹ�ô���Чʱ�� */
    VOS_UINT32                                              aulSpecMcc[TAF_CBA_NVIM_MAX_ETWS_DUP_DETECT_SPEC_MCC_NUM];  /* ��ǿ���ظ���ⶨ��������ص�MCC */
}TAF_CBA_NVIM_ETWS_DUP_DETC_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucReportEccNumFlg;                      /* 1:֧����APP�ϱ�����������, 0:��֧����APP�ϱ�����������*/
    VOS_UINT8                           ucReserved;                             /* ����*/
}MN_CALL_NVIM_REPORT_ECC_NUM_SUPPORT_FLG_STRU;



typedef struct
{
    VOS_UINT8                           ucStatus;/*�Ƿ񼤻0�����1���� */
    MN_CALL_STATE_ENUM_UINT8            enCardLockOperator;
}MMA_CUSTOM_CARDLOCK_OPERATOR_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucVpCfgState;                           /* ��ǰNV����ֵ */
}MN_CALL_NV_ITEM_VIDEO_CALL_STRU;
/* Added by f62575 for AT Project, 2011-10-27, begin */


typedef struct
{
    VOS_UINT8                           ucActFlg;                               /* NVIM�и����Ƿ񼤻� */
    VOS_UINT8                           enClass0Tailor;
}MN_MSG_NVIM_CLASS0_TAILOR_STRU;
/* Added by f62575 for AT Project, 2011-10-27, end */
/* �ն˿���������: 2G���������ֹSpare_bit3�Ȳ��� */
typedef struct
{
    VOS_UINT8   ucStatus;       /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8   ucDisable2GBit3;
}APS_2G_DISABLE_BIT3_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT16                  usMultiSimCallConf;                             /*oΪ��������ʱ�ϱ���1��������ʱ���ϱ�*/
}MN_CALL_NV_ITEM_MULTISIM_CALLCON_STRU;



typedef struct
{
    VOS_UINT8   ucStatus;        /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8   ucUssdTransMode;
}TAF_USSD_NVIM_TRANS_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucIsCallRedialSupportFlg;   /* �Ƿ�֧�ֺ����ؽ����ܣ�VOS_TRUE:֧�֣�VOS_FALSE:��֧��*/
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT32                          ulCallRedialPeriod;         /* �����ؽ����Դ�ʱ�������ؽ����ʱ������λ���� */
    VOS_UINT32                          ulCallRedialInterval;       /* �����ؽ����Դ�ʱ�������ز������λ���� */
}MN_CALL_REDIAL_CFG_STRU;
	
typedef struct
{
    VOS_INT16                           sHandSetVolValue;
    VOS_INT16                           sHandsFreeVolValue;
    VOS_INT16                           sCarFreeVolValue;
    VOS_INT16                           sEarphoneVolValue;
    VOS_INT16                           sBlueToothVolValue;
    VOS_INT16                           sPcVoiceVolValue;
    VOS_INT16                           sHeadPhoneVolValue;
    VOS_INT16                           sSuperFreeVolValue;
    VOS_INT16                           sSmartTalkVolValue;
    VOS_INT16                           sRsv[7];
} APP_VC_NV_CLVL_VOLUME_STRU;

/* �ն˿���������: ����Ĭ��APN���� */
typedef struct
{
    VOS_UINT8   ucStatus;       /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8   aucApn[TAF_NVIM_MAX_APN_STR_LEN];       /* APN�Ӹ�����ĵ�һ���ֽڿ�ʼд�룬������'\0'��Ϊ��β */
    VOS_UINT8   aucRsv[3];
}APS_APN_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT8                                               ucEtwsEnableFlg;    /* ETWS���Կ���, VOS_TRUE:����, VOS_FALSE:δ���� */
    VOS_UINT8                                               ucRsv;
    VOS_UINT16                                              usTempEnableCbsTimeLen; /* ��λ:����, CBS����δ����ʱ,�յ���֪ͨ����ʱʹ��CBS�Խ��մ�֪ͨ��ʱ�� */

    TAF_CBA_NVIM_ETWS_DUP_DETC_CFG_STRU                     stDupDetcCfg;       /* TAF_CBA_NV_MAX_USER_SPEC_ETWS_MSGID_RANGE_NUM Duplication Detection Time ������ */
    TAF_CBA_NVIM_ETWS_MSGID_RANGE_STRU                      astSpecEtwsMsgIdList[TAF_CBA_NV_MAX_USER_SPEC_ETWS_MSGID_RANGE_NUM];   /* �û����Ƶ�ETWS��ϢID��Χ */
}TAF_CBA_NVIM_ETWS_CFG_STRU;


typedef struct
{
    VOS_UINT8                           enAlsLine;
    VOS_UINT8                           aucMmaImsi[9];
}MN_CALL_ALS_LINE_INFO_STRU;


typedef struct
{
    VOS_UINT8                          aucAtClientConfig[TAF_AT_NVIM_CLIENT_CONFIG_LEN];
}TAF_AT_NVIM_AT_CLIENT_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucECallNotifySupport;                   /* �����������¶ȱ�������NV�� */
    VOS_UINT8                           ucSpyStatusIndSupport;                  /* �±�״̬�ϱ�ʹ��NV�� */
    VOS_UINT8                           aucReserved[2];                          /* ������ */
}TAF_TEMP_PROTECT_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucNetSelMenuFlg;                        /* ����ѡ��˵����ƣ�VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucRatBalancingFlg;                      /* ���뼼��ƽ����ƣ�VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           aucReserved[2];                         /* ���� */
}NVIM_ATT_ENS_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          aulUserSetLtebands[2];
}TAF_NVIM_USER_SET_LTEBANDS_STRU;


typedef struct
{
    VOS_UINT32                          ulDSTotalSendFluxLow;
    VOS_UINT32                          ulDSTotalSendFluxHigh;
    VOS_UINT32                          ulDSTotalReceiveFluxLow;
    VOS_UINT32                          ulDSTotalReceiveFluxHig;
}TAF_NVIM_RABM_TOTAL_RX_BYTES_STRU;


typedef struct
{
    VOS_UINT8                          aucRoamWhiteListFlag[2];
}TAF_NVIM_E5_ROAM_WHITE_LIST_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT32                         ulNdisDialUpAdd;
}TAF_NVIM_NDIS_DIALUP_ADDRESS_STRU;

/*nv 9130, Ϊipv6��Ŀ����nvԤ��*/

typedef struct
{
    VOS_UINT32                          ulActiveFlag;
    VOS_UINT8                           aucSmCause[TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM];

} TAF_NV_IPV6_FALLBACK_EXT_CAUSE_STRU;


typedef struct
{
    VOS_UINT32                          ulIpv6RouterMtu;
}TAF_NDIS_NV_IPV6_ROUTER_MTU_STRU;


typedef struct
{
    VOS_INT32                           lValue;
    VOS_INT8                            acPassword[16];
}TAF_AT_NV_DISLOG_PWD_STRU;


typedef struct
{
    VOS_UINT32                          ulE5RightFlag;
}TAF_AT_NV_E5_RIGHT_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulDissdFlag;
}TAF_AT_NV_DISSD_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulOmPortNum;
}TAF_AT_NV_OM_PORT_NUM_STRU;


typedef struct
{
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucData[53];
}TAF_AT_NV_RA_CAPABILITY_STRU;


typedef struct
{
    VOS_UINT8                           aucUmtsAuth[2];
}TAF_AT_NV_UMTS_AUTH_STRU;


typedef struct
{
    VOS_UINT8                           aucGmmInfo[2];
}TAF_AT_NV_GMM_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucMmInfo[2];
}TAF_AT_NV_MM_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucSmsText[16];
}TAF_AT_NV_SMS_TEXT_STRU;


typedef struct
{
    VOS_UINT32                          ulStatus;
    VOS_UINT32                          ulGprsRecentActTime;
}TAF_AT_NV_CUSTOM_GPRS_RECENT_ACT_TIMER_STRU;


typedef struct
{
    VOS_UINT16                          usPsDelayFlag;
}TAF_AT_NV_PS_DELAY_FLAG_STRU;


typedef struct
{
    VOS_UINT8                          aucEhplmnSupportFlag[2];
}TAF_AT_NV_EHPLMN_SUPPORT_FLAG_STRU;


typedef struct
{
    VOS_UINT8                          aucBgFsFbsRatio[4];
}TAF_AT_NV_BG_FS_FBS_RATIO_STRU;


typedef struct
{
    VOS_UINT16                          usEplmnUseRatFlag;
}TAF_AT_NV_EPLMN_USE_RAT_FLAG_STRU;

typedef struct
{
    VOS_UINT8                                               ucLabel[TAF_NVIM_CBA_MAX_LABEL_NUM]; /* С���㲥��Ϣid��Χ��ǩ */
    VOS_UINT16                                              usMsgIdFrom;                    /* С���㲥��ϢID�Ŀ�ʼ���  */
    VOS_UINT16                                              usMsgIdTo;                      /* С���㲥��ϢID�Ľ������ */
    VOS_UINT32                                              ulRcvMode;                      /* ÿ��CBMI RANGE �Ľ���ģʽ, Ŀǰ��֧�� ACCEPT��ģʽ */
}TAF_CBA_NVIM_CBMI_RANGE_STRU;

typedef struct
{
    VOS_UINT32                          ulCbmirNum;                             /* С���㲥��Ϣ��ID���� */
    TAF_CBA_NVIM_CBMI_RANGE_STRU        astCbmiRangeInfo[TAF_CBA_NVIM_MAX_CBMID_RANGE_NUM]; /* С���㲥��Ϣ�ķ�Χ��Ϣ */
}TAF_CBA_NVIM_CBMI_RANGE_LIST_STRU;



typedef struct
{
    VOS_UINT16 usPlatform;
}NAS_NVIM_PLATFORM_STRU;


typedef struct
{
    VOS_UINT8                           ucMuxSupportFlg;
    VOS_UINT8                           ucReserved;
}TAF_AT_NVIM_MUX_SUPPORT_FLG_STRU;

/*����USSDת���ַ���*/
typedef struct
{
    VOS_UINT8                     ucStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT8                     ucAlphaTransMode;    /* �ַ���ת��*/
}SSA_NV_ITEM_ALPHA_to_ASCII_STRU;


typedef struct
{
    VOS_UINT8                           ucCbStatus;         /* CBSҵ���Ƿ����ñ�־*/

    VOS_UINT8                           ucDupDetectCfg;     /* �ظ��������� */


    VOS_UINT8                           ucRptAppFullPageFlg; /* VOS_TRUE: �ϱ���ҳ��88���ֽ�; VOS_FALSE: �ϱ�ʵ����Ч�ֽ� */

    VOS_UINT8                           ucRsv;              /* NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն� */
}TAF_CBA_CBS_SERVICE_PARM_STRU;


typedef struct
{
    MTA_WCDMA_BAND_ENUM_UINT16          enBand;                     /* WCDMAƵ�� */
    VOS_INT16                           sPower;                     /* ��������ֵ */
}MTA_BODY_SAR_W_PARA_STRU;


typedef struct
{
    VOS_INT16                           sGPRSPower;                 /* GPRS��������ֵ */
    VOS_INT16                           sEDGEPower;                 /* EDGE��������ֵ */
}MTA_BODY_SAR_G_PARA_STRU;


typedef struct
{
    VOS_UINT32                          ulGBandMask;                                /* GSM Band��Maskλ */
    VOS_UINT16                          usWBandNum;                                 /* WCDMA��Band���� */
    VOS_UINT16                          ausReserved1[1];                            /* ����λ */
    MTA_BODY_SAR_G_PARA_STRU            astGBandPara[MTA_BODY_SAR_GBAND_MAX_NUM];   /* GSM��������ֵ */
    MTA_BODY_SAR_W_PARA_STRU            astWBandPara[MTA_BODY_SAR_WBAND_MAX_NUM];   /* WCDMA��������ֵ */
}MTA_BODY_SAR_PARA_STRU;

typedef struct
{
    VOS_UINT32  ulNvStatus;
    VOS_UINT8   aucFirstPortStyle[17];   /* �豸�л�ǰ�˿���̬ */
    VOS_UINT8   aucRewindPortStyle[17];  /* �豸�л���˿���̬ */
    VOS_UINT8   aucReserved[22];         /* ���� */
}AT_DYNAMIC_PID_TYPE_STRU;

/* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, begin */

typedef struct
{
    VOS_UINT8                           ucEnableFlag;       /* �Ƿ�ʹ��Share PDP����, 0 - ��ֹ;  1 - ʹ�� */
    VOS_UINT8                           ucReserved;         /* ���� */
    VOS_UINT16                          usAgingTimeLen;     /* �ϻ�ʱ�䣬��λΪ�� */

} TAF_NVIM_SHARE_PDP_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlag;  /* AT+CFUN=0���ػ���ȥ����(U)SIM������ʹ�ܱ�ʶ, 0 - ��ֹ, 1 - ʹ��*/
    VOS_UINT8                           aucReserved[3];
} TAF_NVIM_DEACT_SIM_WHEN_POWEROFF_STRU;
/* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, end */


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlag;                       /* 0: nv��δ���1:nv��� */
    VOS_UINT8                           ucWaitImsVoiceAvailTimerLen;            /* �ȴ�IMS VOICE�Ŀ���ָʾ�Ķ�ʱ��ʱ��,��λΪ�뼶,��Ҫת��Ϊ���� */
    VOS_UINT8                           aucRsv[2];
}TAF_NVIM_WAIT_IMS_VOICE_AVAIL_TIMER_LEN_STRU;




typedef struct
{
    VOS_UINT32                          ulValue;
} TAF_NV_SCI_CFG_STRU;


typedef struct
{
    VOS_UINT8                          ucNvimValid;
    VOS_UINT8                          ucSmsClosePathFlg;
} TAF_NVIM_SMS_CLOSE_PATH_CFG_STRU;



typedef struct
{
    /* λ���� :0�����1:����
    bit0�����ƶ�˫����B39/B3���ų�ͻ����
    bit1��Notch Bypass���Կ���
    */
    VOS_UINT8                          ucSolutionMask;
    VOS_UINT8                          aucAdditonCfg[3];
} MTC_NV_RF_INTRUSION_CFG_STRU;

/* Added by f62575 for VSIM FEATURE, 2013-8-29, begin */

typedef struct
{
    VOS_UINT8                           ucVsimCtrlFlg;
    VOS_UINT8                           aucReserved1[1];
} TAF_NVIM_VSIM_CFG_STRU;

/* Added by f62575 for VSIM FEATURE, 2013-8-29, end */

typedef struct
{
    VOS_UINT8                           ucIsSsRetrySupportFlg;                  /* �Ƿ�֧��ss�ط����ܣ�VOS_TRUE:֧�֣�VOS_FALSE:��֧��*/
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT32                          ulSsRetryPeriod;                        /* ss�ط����Դ�ʱ��ss�ط����ʱ������λ��,Ĭ��30s */
    VOS_UINT32                          ulSsRetryInterval;                      /* ss�ط����Դ�ʱ��ss�ط������λ�룬Ĭ��5s */
}TAF_SSA_NVIM_RETRY_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulSmsRiOnInterval;      /* ����RI�ߵ�ƽ����ʱ��(ms) */
    VOS_UINT32                          ulSmsRiOffInterval;     /* ����RI�͵�ƽ����ʱ��(ms) */

    VOS_UINT32                          ulVoiceRiOnInterval;    /* ����RI�ߵ�ƽ����ʱ��(ms) */
    VOS_UINT32                          ulVoiceRiOffInterval;   /* ����RI�͵�ƽ����ʱ��(ms) */
    VOS_UINT8                           ucVoiceRiCycleTimes;    /* ����RI�������ڴ���     */
    VOS_UINT8                           aucReserved[3];

} TAF_NV_UART_RI_STRU;


typedef struct
{
    VOS_UINT8                           ucFormat;               /* UART ����λֹͣλλ�� */
    VOS_UINT8                           ucParity;               /* UARTУ�鷽ʽ */
    VOS_UINT8                           aucReserved[2];

} TAF_NV_UART_FRAME_STRU;


typedef struct
{
    VOS_UINT32                          ulBaudRate;             /* UART������ */
    TAF_NV_UART_FRAME_STRU              stFrame;                /* UART֡��ʽ */
    TAF_NV_UART_RI_STRU                 stRiConfig;             /* UART Ring������ */

} TAF_NV_UART_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulDlRate;
    VOS_UINT32                          ulUlRate;
    VOS_UINT32                          ulDdrBand;
} TAF_NV_DFS_RATE_BAND_STRU;


typedef struct
{
    VOS_UINT32                          ulProfileNum;
    TAF_NV_DFS_RATE_BAND_STRU           astProfile[TAF_NVIM_DFS_MAX_PROFILE_NUM];
} TAF_NV_DFS_DSFLOW_RATE_CONFIG_STRU;

typedef struct
{
    TAF_NV_DFS_DSFLOW_RATE_CONFIG_STRU          astDfsConfig[TAF_NVIM_DIFF_DFS_NUM];
} TAF_NV_MULTI_DFS_DSFLOW_RATE_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulFlagValue;
    VOS_UINT32                          ulReserved;
} TAF_NV_VOICE_TEST_FLAG_STRU;


typedef struct
{
    VOS_UINT8                          ucSmsDomain;
    VOS_UINT8                          aucReserved[3];
} TAF_NVIM_SMS_DOMAIN_STRU;



typedef struct
{
    VOS_UINT8                           ucEnableFlg;
    VOS_UINT8                           aucReserved[3];
} TAF_NV_PORT_BUFF_CFG_STRU;



typedef struct
{
    VOS_UINT8                                     ucLCEnableFlg;
    TAF_NVIM_LC_RAT_COMBINED_ENUM_UINT8           enRatCombined;
    TAF_NVIM_LC_WORK_CFG_ENUM_UINT8               enLCWorkCfg;
    VOS_UINT8                                     aucReserved[1];
}TAF_NV_LC_CTRL_PARA_STRU;


typedef struct
{
    VOS_UINT8                           ucCallRedial;
    VOS_UINT8                           ucSmsRedial;
    VOS_UINT8                           ucSsRedial;
    VOS_UINT8                           aucReserve[1];
}TAF_NV_SWITCH_DOMAIN_REDIAL_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucImsRoamingFlg;
    VOS_UINT8                           aucReserve[3];
}TAF_NV_IMS_ROAMING_SUPPORT_STRU;


typedef struct
{
    VOS_UINT8                           ucUssdOnImsSupportFlag;
    VOS_UINT8                           aucReserve[3];
}TAF_NV_IMS_USSD_SUPPORT_STRU;


typedef struct
{
    MTC_PS_TRANSFER_ENUM_UINT8         enSolutionCfg;
    VOS_UINT8                          aucAdditonCfg[3];
} MTC_NV_PS_TRANSFER_CFG_STRU;


typedef struct
{
    TAF_FLASH_DIRECTORY_TYPE_ENUM_UINT16                     enFlashDirectoryType;
    VOS_UINT8                                                aucReserved1[2];
}TAF_NV_FLASH_DIRECTORY_TYPE_STRU;


typedef struct
{
    VOS_UINT32                          ulTimerInterval;                        /* ʱ����ֵ,��λ�� */
}MTC_RF_LCD_TIMER_INTERVAL_STRU;


typedef struct
{
    VOS_UINT32                          ulMipiClk;                              /* MipiClkֵ */
    VOS_UINT32                          aulFreq[MTC_RF_LCD_MIPICLK_FREQ_MAX_NUM];/* ulMipiClkֵӰ���Ƶ�ʱ� */
}MTC_NVIM_RF_LCD_MIPICLK_FREQ_STRU;


typedef struct
{
    VOS_UINT16                          usEnableBitMap;                         /* ʹ�ܿ��أ�ÿ��bit������ͬ���㷨����0����ʹ�� */
    VOS_UINT16                          usFreqWidth;                            /* ���� ��λ100KHZ */
    MTC_NVIM_RF_LCD_MIPICLK_FREQ_STRU   astRfMipiClkFreqList[MTC_RF_LCD_MIPICLK_MAX_NUM]; /* ��λ100KHZ */
} MTC_NVIM_RF_LCD_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucMode;                                 /* JAM���õ�ģʽ��0:�ر�, 1:�� */
    VOS_UINT8                           ucMethod;                               /* JAM���ʹ�õķ�����1:����1��2:����2,Ŀǰֻ֧��2 */
    VOS_UINT8                           ucFreqNum;                              /* �����Ҫ�ﵽ��Ƶ�������ȡֵ��Χ:[0,255] */
    VOS_UINT8                           ucThreshold;                            /* �����Ҫ�ﵽ��Ƶ�����ֵ��ȡֵ��Χ:[0,70] */
    VOS_UINT8                           ucJamDetectingTmrLen;                   /* ǿ�ź�Ƶ������������޺󣬼��ͬ������Ķ�ʱ��ʱ��(s) */
    VOS_UINT8                           ucJamDetectedTmrLen;                    /* ��⵽���ź󣬵ȴ�����������ʱ��(s) */
    VOS_UINT8                           aucRsv[2];
}NV_NAS_JAM_DETECT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* 1:��Ƶ���ܴ򿪣�0:���ܹر� */
    TAF_NVIM_RAT_MODE_ENUM_UINT8        enRatMode;
    VOS_UINT16                          usLockedFreq;
    TAF_NVIM_GSM_BAND_ENUM_UINT16       enBand;
    VOS_UINT8                           aucReserved[2];
} TAF_NVIM_FREQ_LOCK_CFG_STRU;

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of NasNvInterface.h */

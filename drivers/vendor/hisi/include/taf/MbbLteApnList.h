
#ifndef __MBB_APN_PROFILE_LIST_H__
#define __MBB_APN_PROFILE_LIST_H__

#ifdef  __cplusplus
    #if  __cplusplus
    extern "C"{
    #endif
#endif

#define    LTE_CUSTOM_ATTACH_PROFILE_KEY_LEN                32 /* 32 * 8 = 256bit*/
#define    LTE_AES_BLOCK_BYTES    16

#define    LTE_ATTACH_APN_NV_NUM   6

#define    LTE_CUSTOM_ATTACH_PROFILE_NAME_LEN               22
#define    LTE_CUSTOM_ATTACH_PROFILE_IMSI_PREFIX_LEN        10
#define    LTE_CUSTOM_ATTACH_PROFILE_IMSI_PREFIX_BCD_LEN    5
#define    LTE_CUSTOM_ATTACH_PROFILE_APN_LEN                32
#define    LTE_CUSTOM_ATTACH_PROFILE_USR_LEN                32
#define    LTE_CUSTOM_ATTACH_PROFILE_PWD_LEN                32
/*** the following must >= sizeof(NV_LTE_ATTACH_PROFILE_STRU)
 and >= (sizeof(LTE_ATTACH_PROFILE_DECRYPT_DATA_ST) + 15)/16*16 ***/
#define    LTE_CUSTOM_ATTACH_PROFILE_ENCRYPED_DATA_LEN      136 

typedef struct{
    const    char *pucName;
    unsigned char ucPdpType;
    unsigned char ucAuthType;
    unsigned char ucImsiPrefixLen;
    unsigned char aucImsiPrefixBcd[LTE_CUSTOM_ATTACH_PROFILE_IMSI_PREFIX_BCD_LEN];
}LTE_ATTACH_PROFILE_PLAIN_DATA_ST;

typedef struct{
    unsigned char ucApnLen;
    unsigned char aucApn[LTE_CUSTOM_ATTACH_PROFILE_APN_LEN];
    unsigned char ucUserNameLen;
    unsigned char aucUserName[LTE_CUSTOM_ATTACH_PROFILE_USR_LEN];
    unsigned char ucPwdLen;
    unsigned char aucPwd[LTE_CUSTOM_ATTACH_PROFILE_PWD_LEN];
    unsigned char aucReserve[1];                           /*align*/
}LTE_ATTACH_PROFILE_DECRYPT_DATA_ST;

typedef struct{
    unsigned int                      ulFlags;
    LTE_ATTACH_PROFILE_PLAIN_DATA_ST  stPlainData;
    const unsigned char               aucEncData[LTE_CUSTOM_ATTACH_PROFILE_ENCRYPED_DATA_LEN];
    const void  *pstDecInfo;
}ENCRYPT_LTE_ATTACH_PROFILE_ST;

ENCRYPT_LTE_ATTACH_PROFILE_ST *LTE_GetCustomAttachProfileTable(VOS_VOID);
unsigned int LTE_GetCustomAttachProfileTableCount(VOS_VOID);
extern const unsigned char g_aucIv[];
extern const unsigned char g_aucIn[];

#ifdef  __cplusplus
    #if  __cplusplus
    }
    #endif
#endif

#endif /* __MBB_APN_PROFILE_LIST_H__ */



#ifndef __VSIM_H__
#define __VSIM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 宏定义 节点名称*/
#define DRV_VSIM_PROC_FILE    "vsim_ddr_data"

#define VSIM_HUK_ENCODE_MAX_LEN 512
#define VSIM_KEYLEN_MAX         128
#define NV_VSIM_DIEID_MAXLEN    64

typedef struct
{
    unsigned long                      ulLen;                            /*数据长度*/
    unsigned char                      aucVal[VSIM_HUK_ENCODE_MAX_LEN+1];/*数据最大为512*/
}SI_PIH_VSIM_HUK_VAL_STRU;

typedef struct
{
    unsigned long                           ulKeyLen;
    unsigned char                           aucKey[VSIM_KEYLEN_MAX];
}VSIM_KEYDATA_STRU;

typedef struct
{
    unsigned char                       enKeyIndex;   /*卡索引*/
    unsigned char                       aucRsv[3];     /*补位*/
    VSIM_KEYDATA_STRU                   stSharekey;        /*共享秘钥*/
} SI_PIH_HVSHDH_SET_CNF_STRU;

typedef struct
{
    unsigned long                           ulNumLen;
    unsigned char                           aucNum[NV_VSIM_DIEID_MAXLEN+1];
} SI_PIH_HVDIEID_QRY_CNF_STRU;

typedef struct
{
    unsigned long                 magic_start;                 /* 分区起始Magic Code 0x5B5B5B5B*/
    unsigned long                 EventType;                   /* 事件类型 */
    union
    {
        SI_PIH_VSIM_HUK_VAL_STRU      HvadhCnf;                    /*517 BYTE*/
        SI_PIH_HVSHDH_SET_CNF_STRU    HvshdhCnf;                   /*136 BYTE*/
        SI_PIH_HVDIEID_QRY_CNF_STRU   HvdieIDCnf;                  /*69  BYTE*/
    }VsimDataEvent;
    unsigned long                 magic_end;                   /* 分区起始Magic Code 0x5B5B5B5B*/
}TAF_VSIM_DATA_AREA;


#ifdef __cplusplus
}
#endif

#endif


#ifndef __DRV_CYPTO_H__
#define __DRV_CYPTO_H__

#include "drv_comm.h"


/*************************MD5相关 START*******************************/
/*为了兼容以前的版本其他模块的使用*/
#if (FEATURE_ON == MBB_SIMLOCK_THREE)
/*****************************************************************************
 函 数 名  : VerifySIMLock
 功能描述  : 判断当前解锁码是否正确 .
 输入参数  : imei       - 单板IMEI号
             unlockcode - 解锁码
 输出参数  : 无。
 返 回 值  : 1：   解锁成功
             0:    解锁失败
*****************************************************************************/
#define DRV_CARDLOCK_MD5_VERIFY(unlockcode, imei)  VerifySL(unlockcode, imei)
extern int VerifySL(char* UnlockCode, char* Key);

/*****************************************************************************
 函 数 名  : VerifyCARDLock
 功能描述  : 判断当前解锁码是否正确 .
 输入参数  : imei       - 单板IMEI号
             unlockcode - 解锁码
 输出参数  : 无。
 返 回 值  : 1：   解锁成功
             0:    解锁失败
*****************************************************************************/
#define DRV_SIMLOCK_MD5_VERIFY(unlockcode, imei)  VerifyCL(unlockcode, imei)
extern int VerifyCL(char* UnlockCode, char* Key);



extern int GetAuthVer(void);
#define DRV_GET_AUTH_VER()  GetAuthVer()

extern BSP_U32 combine_two_str_get_hash(BSP_U8* str1, BSP_U32 str1_len, \
    BSP_U8* str2, BSP_U32 str2_len, BSP_U8* result_hash, BSP_U32 result_hash_len);
#define DRV_COMBINE_TWO_STR_GET_HASH(str1, str1_len, str2, str2_len, result_hash, result_hash_len) \
    combine_two_str_get_hash(str1, str1_len, str2, str2_len, result_hash, result_hash_len)

extern BSP_U32 check_hash_invalid(BSP_U8* hash_buf);
#define DRV_CHECK_HASH_INVALID(hash_buf) check_hash_invalid(hash_buf)

#endif

#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
extern int VerifyCL(char* UnlockCode, char* Key);
extern int VerifySL(char* UnlockCode, char* Key);
extern int GetAuthVer(void);
extern void drv_hwlock_init( void );
#define DRV_GET_AUTH_VER()                                  GetAuthVer()
#define DRV_CARDLOCK_MD5_VERIFY(unlockcode, imei)           VerifySL(unlockcode, imei)
#define DRV_SIMLOCK_MD5_VERIFY(unlockcode, imei)            VerifyCL(unlockcode, imei)
#define DRV_HW_LOCK_VERIFY_PROC(lock_para, para_len, type)  hwlock_verify_request(lock_para, para_len, type)
#endif/*FEATURE_ON == MBB_SIMLOCK_FOUR*/

/*************************MD5相关 END*********************************/
/*for create_crypto_key,hash algorithm enum*/
typedef enum 
{
    CREATE_CRYPTO_KEY_ALGORITHM_MD5 = 0x0,
    CREATE_CRYPTO_KEY_ALGORITHM_SHA1,
    CREATE_CRYPTO_KEY_ALGORITHM_SHA256,
    CREATE_CRYPTO_KEY_ALGORITHM_MAX
}CREATE_CRYPTO_KEY_ALGORITHM;

/*for crypto_hash,hash algorithm enum*/
typedef enum 
{
    CRYPTO_ALGORITHM_MD5 = 0x0,
    CRYPTO_ALGORITHM_SHA1,
    CRYPTO_ALGORITHM_SHA256,
    CRYPTO_ALGORITHM_MAX
}CRYPTO_HASH_ALGORITHM;

/*for crypto_encrypt,aes algorithm enum*/
typedef enum 
{
    CRYPTO_ENCRYPT_ALGORITHM_AES_ECB = 0x0,
#if(FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
    CRYPTO_ENCRYPT_ALGORITHM_AES_CBC = 0x1,/*8fix IV,8 input IV*/
    CRYPTO_ENCRYPT_ALGORITHM_AES_CBC_ORI = 0x2, /* ori 10bytes IV*/
#endif
    CRYPTO_ENCRYPT_ALGORITHM_MAX
}CRYPTO_ENCRYPT_ALGORITHM;

#if (FEATURE_ON == MBB_WPG_LTE_ATTACH_APN_LIST)
int FOTA_AES_Crypto_Decrypt (
    char *cipher_data,
    int cipher_len,
    CRYPTO_ENCRYPT_ALGORITHM algorithm,
    char *key,
    int klen,
    char *data,
    int *len,
    unsigned char *iv);

int wb_aes_decrypt_cbc(
        const unsigned char* iv,
        const unsigned char* input, unsigned int in_len,
        unsigned char* output, unsigned int* out_len);
#endif


int create_crypto_key(char *data, int len, CREATE_CRYPTO_KEY_ALGORITHM algorithm, char *key, int *klen);
#define CREATE_CRYPTO_KEY(data,len,algorithm,key,klen)  create_crypto_key(data,len,algorithm,key,klen)


int crypto_hash(char *data, int len, CRYPTO_HASH_ALGORITHM algorithm, char *hash, int *hlen);
#define CRYPTO_HASH(data,len,algorithm,hash,hlen)  crypto_hash(data,len,algorithm,hash,hlen)


int crypto_encrypt (char *data, int len, CRYPTO_ENCRYPT_ALGORITHM algorithm, char *key, int klen, char *cipher_data, int *cipher_len);
#define CRYPTO_ENCRYPT(data,len,algorithm,key,klen,cipher_data,cipher_len)  \
crypto_encrypt(data,len,algorithm,key,klen,cipher_data,cipher_len)

/*****************************************************************************
* 函 数 名  : crypto_decrypt
*
* 功能描述  : 使用指定的密钥和指定的算法对输入的数据解密，输出解密后的数据。
*             当前支持AES-ECB算法。
*
* 输入参数  : 
*             cipher_data: 待密的数据的存放buffer。
*             cipher_len:  待解密的数据的实际长度。(byte)
*             algorithm:   所用解密算法，暂只提供AES-ECB。
*             key:         密钥buffer。
*             klen:        密钥buffer长度。(byte)
*             len:  解密后的数据的存放buffer的buffer size。(byte)(没有检查)
*
* 输出参数  : 
*             data:        解密后的数据。
*             len:         解密后的数据长度。(byte)
*
* 返 回 值  : BSP_OK:      解密成功。
*             BSP_ERROR:   解密失败。
*
* 其它说明  : len为输入/输出参数，传入的len变量所用内存必须可写回。
*             所以避免直接传入类似sizeof()的函数调用结果。
*
*****************************************************************************/
extern int crypto_decrypt (char *cipher_data,int cipher_len,CRYPTO_ENCRYPT_ALGORITHM algorithm, char *key, int klen, char *data, int *len);
#define CRYPTO_DECRYPT(cipher_data,cipher_len,algorithm, key, klen, data, len)  \
crypto_decrypt(cipher_data,cipher_len,algorithm, key, klen, data, len)



int crypto_rsa_encrypt (char *data, int len, char *rsa_key, int rsa_klen, char *cipher_data, int *cipher_len);
#define CRYPTO_RSA_ENCRYT(data,len,rsa_key,rsa_klen,cipher_data,cihper_len) \
crypto_rsa_encrypt(data,len,rsa_key,rsa_klen,cipher_data,cihper_len)


int crypto_rsa_decrypt (char *cipher_data, int cipher_len, char *rsa_key, int rsa_klen, char *data, int *len);
#define CRYPTO_RSA_DECRYPT(cipher_data,cihper_len,rsa_key,rsa_klen,data,len) \
crypto_rsa_decrypt(cipher_data,cihper_len,rsa_key,rsa_klen,data,len)

/*****************************************************************************
* 函 数 名  : crypto_rand
*
* 功能描述  : 随机数生成接口
* 输入参数  : rand_data:随机数存放buffer
*                           len:期望得到的随机数字节数
*
* 输出参数  : 
*
* 返 回 值  :  BSP_OK--获取随机数成功;BSP_ERROR--获取失败
*
* 其它说明  : 内存由调用者申请
*
*****************************************************************************/
int crypto_rand (char *rand_data, int len);
#define CRYPTO_RAND(rand_data,len)  crypto_rand(rand_data,len)


void _Clarinet_MD5Init(void *mdContext);
void _Clarinet_MD5Update(void *mdContext, unsigned char *inBuf, unsigned int inLen);
void _Clarinet_MD5Final(unsigned char *hash, void *mdContext);

#endif


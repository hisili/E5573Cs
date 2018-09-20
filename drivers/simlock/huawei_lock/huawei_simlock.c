/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2015-2016. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include "DrvInterface.h"
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#include "huawei_simlock.h"
#include <crypto/hash.h>
#include "product_nv_id.h"
#include "product_nv_def.h"
#include "bsp_nvim.h"
#include "bsp_sram.h"
#include "bsp_icc.h"
#include <bsp_cpufreq.h>
#include <linux/mlog_lib.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
/*hwlock密码校验消息结构体定义*/
typedef struct
{
    unsigned char  lock_para[DRV_HWOCK_LEN_MAX];/*密码字符串缓存*/
    unsigned int para_len;/*密码长度*/
    unsigned int type;
} DRV_HWLOCK_MSG_STRU;

#ifdef  __cplusplus
 #if  __cplusplus
extern "C" {
 #endif
#endif


void hwlock_get_rtc_time(struct rtc_time *tm)
{
    struct timex  txc;
    (void)do_gettimeofday(&(txc.time));
    (void)rtc_time_to_tm(txc.time.tv_sec,tm);
}


static int hmac_sha256(unsigned char *key, unsigned int ksize, unsigned char *plaintext, unsigned int psize, unsigned char *output)
{
    int ret = 0;
    struct crypto_shash *tfm;
    if ( (NULL == key) || (NULL == plaintext) || (NULL == output) )
    {
        printk(KERN_ERR "[HWLOCK] key or plaintext or output is null err!\r\n");
        return -1;
    }

    if (!ksize)
    {
        return -EINVAL;
    }

    /*psize 参数为int型，不做校验*/

    tfm = crypto_alloc_shash("hmac(sha256)", 0, 0);
    if (IS_ERR(tfm))
    {
        printk(KERN_ERR "[HWLOCK] alloc ahash failed: err %ld", PTR_ERR(tfm));
        return PTR_ERR(tfm);
    }

    ret = crypto_shash_setkey(tfm, key, ksize);
    if (ret)
    {
        printk(KERN_ERR "[HWLOCK] ahash setkey failed: err %d", ret);
    }
    else
    {
        struct
        {
            struct shash_desc shash;
            char ctx[crypto_shash_descsize(tfm)];
        }desc;

        desc.shash.tfm = tfm;
        desc.shash.flags = CRYPTO_TFM_REQ_MAY_SLEEP;
        ret = crypto_shash_digest(&desc.shash, plaintext, psize, output);
    }

    crypto_free_shash(tfm);
    return ret;
}


static int PBKDF2(char * pwd, unsigned int pwd_len, char* salt, unsigned int salt_len, char* out, \
    unsigned int out_len ,unsigned int iterate_cnt)
{
    unsigned int i = 0;
    unsigned int j = 0;
    char hmac_result[SHA256_OUT_LEN] = {0};
    char pbkdf2_buf[SHA256_OUT_LEN] = {0};
    char hmac_salt[PBKDF2_SALT_LEN_MAX] = {0}; /*实际占用32+4字节*/
    char constant[PBKDF2_CONSTANT_LEN] = {0,0,0,1}; 
    int ret = 0;

    if ( (NULL == pwd) || (NULL == salt) || (NULL == out) )
    {
        printk(KERN_ERR "[HWLOCK] pointer is NOT correct!\r\n" );
        return -1;
    }
    
    if ( out_len != PBKDF2_OUT_LEN ) 
    {
        printk(KERN_ERR "[HWLOCK] out_len is NOT correct!\r\n" );
        return -1;
    }

    if (salt_len > PBKDF2_SALT_LEN_MAX)
    {
        printk(KERN_ERR "[HWLOCK] salt_len NOT correct!\r\n" );
        return -1;
    }

    if (strlen(pwd) != pwd_len)
    {
        printk(KERN_ERR "[HWLOCK] pwd_len NOT correct!\r\n" );
        return -1;
    }

    if (SET_ITERATE_CNT > iterate_cnt)
    {
        printk(KERN_ERR "[HWLOCK] iterate_cnt NOT correct!\r\n" );
        return -1;
    }

    /* 海思平台提升CPU频率，加速计算速度 */
    (void)pwrctrl_dfs_set_profile(BALONG_FREQ_MAX);
    
    /*复制32字节盐值*/
    memcpy( hmac_salt, salt, SHA256_OUT_LEN );

    /*在初始盐值之后复制4字节常量,生成最终的36字节盐值*/
    memcpy(hmac_salt + SHA256_OUT_LEN, constant, sizeof(constant) );
    ret = hmac_sha256(pwd, pwd_len, hmac_salt, SHA256_OUT_LEN + sizeof(constant), hmac_result);
    if (0 != ret)
    {
        printk(KERN_ERR "[HWLOCK] ret is %d!\r\n", ret);
        return -1;
    }
    memset(hmac_salt, '\0', sizeof(hmac_salt));
    for ( j = 0; j < SHA256_OUT_LEN; j++ )
    {
        pbkdf2_buf[j] ^= hmac_result[j];
    }

    for ( i = 1; i <= iterate_cnt; i++ )
    {
        memcpy(hmac_salt, hmac_result, SHA256_OUT_LEN);
        ret = hmac_sha256(pwd, pwd_len, hmac_salt, SHA256_OUT_LEN, hmac_result);
        if (0 != ret)
        {
            printk(KERN_ERR "[HWLOCK] ret is %d!\r\n", ret);
            return -1;
        }

        for ( j = 0; j < SHA256_OUT_LEN; j++ )
        {
            pbkdf2_buf[j] ^= hmac_result[j];
        }
    }

    /*复制到输出参数中*/
    memcpy( (void*)out, pbkdf2_buf, out_len );

    return 0;
}


static unsigned int check_radom_invalid(unsigned char* random_buf)
{
    unsigned int  i = 0;
    if (NULL == random_buf)
    {
        printk(KERN_ERR "[HWLOCK]  random_buf is null err!\r\n");
        return -1;
    }

    for (i = 0; i < DRV_RANDOM_LEN; i++)
    {
        if (0 != random_buf[i])
        {
            return 0;
        }
    }
    printk(KERN_ERR "[HWLOCK]  random in random_buf is invalid!\r\n");
    return -1;
}


static int get_random (unsigned char* random_buf)
{
    int ret = 0;

    if (NULL == random_buf)
    {
        return -1;
    }

    get_random_bytes(&random_buf[0], DRV_RANDOM_LEN);
    ret = check_radom_invalid(&random_buf[0]);
    if ( 0 != ret )
    {
        get_random_bytes(&random_buf[0], DRV_RANDOM_LEN);
    }

    return 0;
}


unsigned int get_pbkdf2_times_random(void)
{
    unsigned int random_num = 0;

    get_random_bytes(&random_num, DRV_PBKDF2_TIMES_RANDOM_LEN);
    random_num = random_num % SET_ITERATE_CNT + SET_ITERATE_CNT;
    return random_num;
}


static int drv_hwlock_check_digit(char num)
{
    if( (num >= '0') && (num <= '9') )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}


static int drv_hwlock_check_capital_letter(char num)
{
    char letter_tab[] = {'A', 'B', 'D', 'E', 'F', 'G', 'H', 'J', 'L', 'M', 'N', 'Q', 'R', 'T', 'Y'};
    unsigned int loop = 0;
    unsigned int max_num = sizeof(letter_tab);
    for (loop = 0; loop < max_num; loop++)
    {
        if (num == letter_tab[loop])
        {
            return 0;
        }
    }

    return -1;
}


static int drv_hwlock_check_lowercase(char num)
{
    char letter_tab[] = {'a', 'b', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'm', 'n', 'q', 'r', 't', 'y'};
    unsigned int loop = 0;
    unsigned int max_num = sizeof(letter_tab);

    for(loop = 0; loop < max_num; loop++)
    {
        if (num == letter_tab[loop])
        {
            return 0;
        }
    }
    return -1;
}

#define HWLOCK_STR_VALIDE_CODE_TYPE    (2)

static int drv_check_hwlock_string(char *pt_data, unsigned int len)
{
    unsigned int loop = 0;
    unsigned int lowercase_count_flag = 0;  /*字串中包含小写字母标志*/
    unsigned int capital_letter_count_flag = 0;  /*字串中包含大写字母标志*/
    unsigned int digit_count_flag = 0;  /*字串中包含数字标志*/
    char *pt_temp = pt_data;
    int ret = -1;
    if( (NULL == pt_data) || (0 == len) )
    {
        printk(KERN_ERR "[HWLOCK] drv check hwlock_string para err!\r\n");
        return -1;
    }
    if (strlen(pt_data) != len)
    {
        printk(KERN_ERR "[HWLOCK] drv check hwlock string len is not equal with input err!\r\n");
        return -1;
    }

    for ( loop = 0; loop < len; loop++)
    {
        if ( 0 == drv_hwlock_check_digit(pt_temp[loop]) )
        {
            digit_count_flag = 1;
            continue;
        }
        else if ( 0 == drv_hwlock_check_capital_letter(pt_temp[loop]) )
        {
            capital_letter_count_flag = 1;
            continue;
        }
        else if ( 0 == drv_hwlock_check_lowercase(pt_temp[loop]) )
        {
            lowercase_count_flag = 1;
            continue;
        }
        else
        {
            /*字符为非数字非合法大、小写字符*/
			printk(KERN_INFO "[HWLOCK]drv check hwlock string CHAR IS INVALID!\r\n");
            return -1;
        }
    }

    ret = digit_count_flag + capital_letter_count_flag + lowercase_count_flag;
    /*有2种以上字符认为符合要求*/
    if ( ret >= HWLOCK_STR_VALIDE_CODE_TYPE )
    {
        return 0;
    }

    return -1;
}


int hw_lock_set_proc(char *lock_para, unsigned int para_len, unsigned int type)
{
    struct rtc_time tm;
    NV_AUHT_SIMLOCK_STWICH_STRU hw_lock;
    unsigned int hw_lock_id = 0;
    int result = -1;
    unsigned char result_hash[DRV_HWLOCK_HASH_LEN] = {0};/*（保存密码hash值 + 随机数hash值）的hash值 */
    unsigned char random_buf[DRV_RANDOM_LEN] = {0};
#if ( FEATURE_ON == MBB_SIMLOCK_FOUR_AES)
    unsigned int random_encrypt_len = 0;
#endif
    confidential_nv_opr_info *smem_data = NULL;
    unsigned int iterate_cnt = 0;    /*保存PBKDF2次数*/
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;
    memset(&hw_lock, 0, sizeof(NV_AUHT_SIMLOCK_STWICH_STRU));
    memset(&tm, 0, sizeof(struct rtc_time));
    if (NULL == lock_para)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock lock_para is err!\r\n");
        result = -1;
        goto hw_lock_set_exit;
    }

    if ( (strlen(lock_para) != para_len) || (DRV_HWOCK_LEN_MAX < para_len) )
    {
        printk(KERN_ERR "[HWLOCK] set hwlock para len err!\r\n");
        result = -1;
        goto hw_lock_set_exit;
    }

    result = drv_check_hwlock_string(lock_para, para_len);
    if (0 != result)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock str check invalid!\r\n");
        result = -1;
        goto hw_lock_set_exit;
    }

    if (HW_LOCK_OEM_TYPE == type)
    {
        hw_lock_id = NV_HUAWEI_OEMLOCK_I;
    }
    else if (HW_LOCK_SIMLOCK_TYPE == type)
    {
        hw_lock_id = NV_HUAWEI_SIMLOCK_I;
    }
    else
    {
        printk(KERN_ERR "[HWLOCK] set hwlock HW_LOCK_OEM_TYPE err!\r\n");
        result = -1;
        goto hw_lock_set_exit;
    }

    /*获取时间随机数的hash值，保存到random_buf*/
    result = get_random(random_buf);
    if (0 != result)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock get_random is error\r\n");
        result = -1;
        goto hw_lock_set_exit;
    }

    iterate_cnt = get_pbkdf2_times_random();
    if (SET_ITERATE_CNT > iterate_cnt)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock get times random error\r\n");
        iterate_cnt = SET_ITERATE_CNT;
    }
    /*将随机数次数保存到nv buf*/
    *(unsigned int *)(&hw_lock.reserved[4]) = iterate_cnt;
#if ( FEATURE_ON == MBB_SIMLOCK_FOUR_AES)
    /*随机数密文保存到hw_lock.nv_lock21*/
    result = wb_aes_encrypt_cbc(SALT_IV, random_buf, DRV_RANDOM_LEN, (unsigned char *)hw_lock.nv_lock21, &random_encrypt_len);
    if (0 != result)
    {
        printk(KERN_ERR "[HWLOCK] aes encrypt result = %d err!\r\n", result);
        result = -1;
        goto hw_lock_set_exit;
    }

    if (DRV_ASE_LEN != random_encrypt_len)
    {
        printk(KERN_ERR "[HWLOCK] aes encrypt return len = %d err!\r\n", random_encrypt_len);
        result = -1;
        goto hw_lock_set_exit;
    }
#else

   memcpy((unsigned char *)hw_lock.nv_lock21,  random_buf, DRV_RANDOM_LEN);
#endif
    result = PBKDF2(lock_para, para_len, random_buf, DRV_RANDOM_LEN, result_hash, DRV_HWLOCK_HASH_LEN ,iterate_cnt);
    if (0 != result)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock algorithm  process error,result = %d!\r\n", result);
        result = -1;
        goto hw_lock_set_exit;
    }

    hw_lock.reserved[HWLOCK_ENABLE_FLAG] = DRV_OEM_SIMLOCK_ENABLE;
    memcpy((unsigned char *)hw_lock.nv_lock30, result_hash, DRV_HWLOCK_HASH_LEN);

    if (NULL == smem_data)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock smem flag malloc fail!\n");
        result = -1;
        goto hw_lock_set_exit;
    }

    /*设置机要nv授权标记，授权读取nv*/
    smem_data->smem_confidential_nv_opr_flag = SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM;

    result = bsp_nvm_write(hw_lock_id, (unsigned char*)&hw_lock, sizeof(hw_lock));
    
    if (NV_OK != result)
    {
        printk(KERN_ERR "[HWLOCK] set hwlock:ERROR:write OEMLOCK nv Fail,result = %d!\r\n", result);
        result = -1;
        goto hw_lock_set_exit;
    }
    else
    {
        result = 0;
    }
    
hw_lock_set_exit:
    memset(&hw_lock, 0, sizeof(NV_AUHT_SIMLOCK_STWICH_STRU));
    memset(result_hash, 0, sizeof(result_hash));
    memset(random_buf, 0, sizeof(random_buf));

#if (FEATURE_ON == MBB_MLOG)
    (void)hwlock_get_rtc_time(&tm);
#if (FEATURE_ON == MBB_FACTORY)
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "UTC time :%d-%d-%d %d:%d:%d ", \
        tm.tm_year + YEAR_BASE,tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "factory set hwlock ret is %d.\r\n", result);
#else
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "UTC time :%d-%d-%d %d:%d:%d ", \
        tm.tm_year + YEAR_BASE,tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "release set hwlock ret is %d.\r\n", result);
#endif
#endif
    return result;
    
}


int hw_lock_verify_proc(unsigned char *lock_para, unsigned int para_len, unsigned int type)
{
    struct rtc_time tm;
    int result = -1;
    unsigned int hw_lock_id = 0;
    NV_AUHT_SIMLOCK_STWICH_STRU hw_lock;
    unsigned char result_hash[DRV_HWLOCK_HASH_LEN] = {0};/*保存PBKDF2计算结果*/

    unsigned char random_buf[DRV_RANDOM_LEN] = {0};
#if ( FEATURE_ON == MBB_SIMLOCK_FOUR_AES)
    unsigned int random_decrypt_len = 0;
#endif
    confidential_nv_opr_info *smem_data = NULL;
    unsigned int iterate_cnt = 0;    /*保存PBKDF2次数*/
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;

    memset(&hw_lock, 0, sizeof(NV_AUHT_SIMLOCK_STWICH_STRU));
    memset(&tm, 0, sizeof(struct rtc_time));
    if (NULL == lock_para)
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock para err!\r\n");
        result = -1;
        goto hw_lock_verify_exit;
    }

    if ( (strlen(lock_para) != para_len)  || (DRV_HWOCK_LEN_MAX < para_len) )
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock para len err!\r\n");
        result = -1;
        goto hw_lock_verify_exit;
    }

    if (HW_LOCK_OEM_TYPE == type)
    {
        hw_lock_id = NV_HUAWEI_OEMLOCK_I;
    }
    else if (HW_LOCK_SIMLOCK_TYPE == type)
    {
        hw_lock_id = NV_HUAWEI_SIMLOCK_I;
    }
    else
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock type err!\r\n");
        result = -1;
        goto hw_lock_verify_exit;
    }

    if (NULL == smem_data)
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock smem nv malloc fail!\n");
        result = -1;
        goto hw_lock_verify_exit;
    }
    /*设置纪要nv授权标记，授权读取nv*/
    smem_data->smem_confidential_nv_opr_flag = SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM;

    result = bsp_nvm_read(hw_lock_id, (unsigned char*)&hw_lock, sizeof(hw_lock));
    if (NV_OK != result)
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock :ERROR:read nv hw_lock Fail.r\n");
        result = -1;
        goto hw_lock_verify_exit;
    }

    iterate_cnt = *(unsigned int *)(&hw_lock.reserved[4]);
    if (SET_ITERATE_CNT > iterate_cnt)
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock get times random invalid\r\n");
        iterate_cnt = SET_ITERATE_CNT;
    }

#if ( FEATURE_ON == MBB_SIMLOCK_FOUR_AES)
    /*解密随机数盐值*/
    result = wb_aes_decrypt_cbc_simlock(SALT_IV, hw_lock.nv_lock21, DRV_ASE_LEN, random_buf, &random_decrypt_len);
    if (0 != result)
    {
        printk(KERN_ERR "[HWLOCK] aes decrypt result = %derr!\r\n", result);
        result = -1;
        goto hw_lock_verify_exit;
    }

    if (DRV_RANDOM_LEN != random_decrypt_len)
    {
        printk(KERN_ERR "[HWLOCK] aes encrypt return len = %d err!\r\n", random_decrypt_len);
        result = -1;
        goto hw_lock_verify_exit;
    }
#else

    memcpy(random_buf, (unsigned char *)hw_lock.nv_lock21, DRV_RANDOM_LEN);
#endif
    /*hwlock 密码校验计算*/
    result = PBKDF2(lock_para, para_len, random_buf, DRV_RANDOM_LEN, result_hash, DRV_HWLOCK_HASH_LEN ,iterate_cnt);
    if (0 != result)
    {
        printk(KERN_ERR "[HWLOCK] verify hwlock algorithm process error!\r\n");
        result = -1;
        goto hw_lock_verify_exit;
    }

    /*把hash后的值和产线写入NV的密码进行比较，如果一致则解锁成功，否则失败*/
    result = memcmp((unsigned char *)result_hash, (unsigned char *)hw_lock.nv_lock30, DRV_HWLOCK_HASH_LEN);

hw_lock_verify_exit:
    memset(&hw_lock, 0, sizeof(NV_AUHT_SIMLOCK_STWICH_STRU));
    memset(result_hash, 0, sizeof(result_hash));
    memset(random_buf, 0, sizeof(random_buf));

#if (FEATURE_ON == MBB_MLOG)
    (void)hwlock_get_rtc_time(&tm);
#if (FEATURE_ON == MBB_FACTORY)
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "UTC time :%d-%d-%d %d:%d:%d ", \
        tm.tm_year + YEAR_BASE,tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "factory verify hwlock ret is %d.\r\n", result);
#else
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "UTC time :%d-%d-%d %d:%d:%d ", \
        tm.tm_year + YEAR_BASE,tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    mlog_print(MLOG_SIMLOCK, mlog_lv_fatal, "release verify hwlock ret is %d.\r\n", result);
#endif
#endif

    return result;
}


static int drv_hwlock_verify_ack(int result )
{
    /*result--取值为整型，不需要进行参数合法性校验*/

    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_VERIFY_HWLOCK_RSP;
    unsigned int ret_val;
    int msg;
    /*填充校验结果*/
    msg = result;

    ret_val = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (unsigned char*)&msg, sizeof(msg));
    if (ret_val != (int)sizeof(msg))
    {
        printk(KERN_ERR "[HWLOCK] simlock : send verify ack fail !!!\n");
        return -1;
    }

    return 0;
}


static int drv_icc_hwlock_verify(unsigned int chanid ,unsigned int len,void* pdata)
{
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_VERIFY_FUNC_HWLOCK;
    unsigned int key_len = 0;
    DRV_HWLOCK_MSG_STRU hwlock_msg;

    int ret_val = BSP_ERROR;
    int ret_result =  BSP_ERROR;

    memset((void*)(&hwlock_msg), 0, sizeof(DRV_HWLOCK_MSG_STRU));
    if ( icc_channel_id != chanid )
    {
        printk(KERN_ERR "[HWLOCK] simlock: channel_id doesn't match\n");
        return -1;
    }

    if (len > sizeof(DRV_HWLOCK_MSG_STRU))
    {
        /*满足编程规范增加判断*/
        printk(KERN_INFO "[HWLOCK] %s :  len is longer than hwlock_msg len\r\n", __func__);
    }

    if (NULL == pdata)
    {
        /*满足编程规范增加判断*/
        printk(KERN_INFO "[HWLOCK]%s :  pdata is null!\r\n", __func__);
    }

    key_len = bsp_icc_read(icc_channel_id, (unsigned char*)(&hwlock_msg), sizeof(hwlock_msg));
    
    if (key_len != sizeof(hwlock_msg))
    {
        printk(KERN_ERR "[HWLOCK]read len(%d) != expected len(%d) !\n", key_len, sizeof(hwlock_msg));
        return -1;
    }

    ret_result = hw_lock_verify_proc(hwlock_msg.lock_para, hwlock_msg.para_len , hwlock_msg.type);

    /*对A核的回复信息*/
    ret_val = drv_hwlock_verify_ack(ret_result);

    return ret_val;
}


static int __init drv_verify_simlock_acore_init(void)
{
    int ret_val = 0;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_VERIFY_FUNC_HWLOCK;

    ret_val = bsp_icc_event_register(icc_channel_id, \
        (read_cb_func)drv_icc_hwlock_verify, NULL, NULL, NULL);

    if ( 0 != ret_val )
    { 
        printk(KERN_ERR "[HWLOCK] drv verify simlock acore init fail:%d\n", ret_val);
        return -1;
    }

    printk(KERN_INFO "[HWLOCK] drv verify simlock init ok!\r\n");
    return 0;
}

module_init(drv_verify_simlock_acore_init);

#ifdef  __cplusplus
 #if  __cplusplus
}
 #endif
#endif

#endif/*end for MBB_SIMLOCK_FOUR*/


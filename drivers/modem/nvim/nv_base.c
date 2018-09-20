


/*lint -save -e537*/
#include <linux/kthread.h>
#include <osl_thread.h>
#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_xml_dec.h"
#include "NVIM_ResumeId.h"
#include "mbb_leds.h"

#define FACTORY_INFO_LEN              (78)
#define MMI_TEST_FLAG_OFFSET          (24)
#define MMI_TEST_FLAG_LEN             (4)
#define NV_ID_MSP_FACTORY_INFO        (114)
u32 g_clean_mmi_flag = 0;
#include "product_nv_id.h"
#include "bsp_sram.h"

/*lint -restore +e537*/


#include <power_com.h>  /*fastboot引用的异常信息结构体必备头文件*/
#include "bsp_sram.h"

typedef enum
{
    NV_RESTRORE_SUCCESS,
    NV_RESTRORE_FAIL,
}NV_RESTORE_STATUS;
bool nv_isSecListNv(u16 itemid)
{
    /*lint -save -e958*/
    u16 i = 0;

    /*lint -restore*/
    for(i = 0;i < bsp_nvm_getRevertNum(NV_SECURE_ITEM);i++)
    {
        if(itemid == g_ausNvResumeSecureIdList[i])
        {
            return true;
        }
    }
    return false;
}


/*lint -save -e713 -e830*/
u32 nv_readEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret = NV_ERROR;
    struct nv_file_list_info_stru file_info;
    struct nv_ref_data_info_stru  ref_info;
    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;
    nv_debug(NV_FUN_READ_EX,0,itemid,modem_id,datalen);

    if((NULL == pdata)||(0 == datalen))
    {
        nv_debug(NV_FUN_READ_EX,1,itemid,0,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }
    /*如果是datalock密码或simlock密码NV,在未授权时禁止读取*/
    if ( (NV_HUAWEI_OEMLOCK_I == itemid) || (NV_HUAWEI_SIMLOCK_I == itemid) )
    {
        if (NULL == smem_data)
        {
            printk(KERN_ERR "smem_confidential_nv_opr_flag malloc fail!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
        if(SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM == smem_data->smem_confidential_nv_opr_flag)
        {
            /*机要NV读取后，取消授权*/
            smem_data->smem_confidential_nv_opr_flag = SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR;
        }
        else
        {
            printk(KERN_ERR  "smem_confidential_nv_opr_flag invalid!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
    }
    ret = nv_search_byid(itemid,((u8*)NV_GLOBAL_CTRL_INFO_ADDR),&ref_info,&file_info);
    if(ret)
    {
        printf("\n[%s]:can not find 0x%x !\n",__FUNCTION__,itemid);
        return BSP_ERR_NV_NO_THIS_ID;
    }

    if((offset + datalen) > ref_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_READ_EX,3,offset,datalen,ref_info.nv_len);
        goto nv_readEx_err;
    }
    if(modem_id <= ref_info.modem_num)
    {
        ref_info.nv_off += (modem_id - NV_USIMM_CARD_1)*ref_info.nv_len;
    }
    else
    {
        ret = BSP_ERR_NV_INVALID_PARAM;
        nv_debug(NV_FUN_READ_EX,4,ret,itemid,modem_id);
        goto nv_readEx_err;
    }

    ret = nv_read_from_mem(pdata, datalen,file_info.file_id,(ref_info.nv_off+offset));
    if(ret)
    {
        nv_debug(NV_FUN_READ_EX,5,offset,datalen,ref_info.nv_len);
        goto nv_readEx_err;
    }
    /*lint -save -e578 -e530*/
    nv_debug_trace(pdata, datalen);
    /*lint -restore +e578 +e530*/

    return NV_OK;
nv_readEx_err:
    nv_mntn_record("\n[%s]:[0x%x]:[%d]\n",__FUNCTION__,itemid,modem_id);
    nv_help(NV_FUN_READ_EX);
    return ret;
}

u32 nv_writeEx(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    u32 ret = NV_ERROR;
    struct nv_file_list_info_stru file_info;
    struct nv_ref_data_info_stru  ref_info;
    u32 nv_offset = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    confidential_nv_opr_info *smem_data = NULL;
    smem_data = (confidential_nv_opr_info *)SRAM_CONFIDENTIAL_NV_OPR_ADDR;

    nv_debug(NV_FUN_WRITE_EX,0,itemid,modem_id,datalen);

    if((NULL == pdata)||(0 == datalen))
    {
        nv_debug(NV_FUN_WRITE_EX,1,itemid,datalen,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }
    /*如果是datalock密码或simlock密码NV禁止写入*/
    if ( (NV_HUAWEI_OEMLOCK_I == itemid) || (NV_HUAWEI_SIMLOCK_I == itemid) )
    {
        if (NULL == smem_data)
        {
            printk(KERN_ERR "smem_confidential_nv_opr_flag malloc fail!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }

        if (SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM == smem_data->smem_confidential_nv_opr_flag)
        {
            /*机要NV操作后，取消授权*/
            smem_data->smem_confidential_nv_opr_flag = SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR;
        }
        else
        {
            printk(KERN_ERR "smem_confidential_nv_opr_flag invalid!\n");
            return BSP_ERR_NV_NO_THIS_ID;
        }
    }

    ret = nv_search_byid(itemid,((u8*)NV_GLOBAL_CTRL_INFO_ADDR),&ref_info,&file_info);
    if(ret)
    {

        printf("\n[%s]:can not find 0x%x !\n",__FUNCTION__,itemid);
        return BSP_ERR_NV_NO_THIS_ID;
    }

    /*lint -save -e578 -e530*/
    nv_debug_trace(pdata, datalen);
    /*lint -restore +e578 +e530*/

    if((datalen + offset) >ref_info.nv_len)
    {
        ret = BSP_ERR_NV_ITEM_LEN_ERR;
        nv_debug(NV_FUN_WRITE_EX,3,itemid,datalen,ref_info.nv_len);
        goto nv_writeEx_err;
    }
    if(modem_id <= ref_info.modem_num)
    {
        ref_info.nv_off += (modem_id - NV_USIMM_CARD_1)*ref_info.nv_len;
    }
    else
    {

        ret = BSP_ERR_NV_INVALID_PARAM;
        nv_debug(NV_FUN_WRITE_EX,4,itemid,ret,modem_id);
        goto nv_writeEx_err;
    }
    nv_offset = ddr_info->file_info[file_info.file_id -1].offset+offset+ref_info.nv_off;

    /*IPC锁保护，防止在校验CRC时写NV操作还没有完成*/
    nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    ret = nv_check_nv_data_crc(nv_offset, datalen);
    nv_ipc_sem_give(IPC_SEM_NV_CRC);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX, 5, itemid,datalen,0);
        ret = nv_resume_ddr_from_img();
        if(ret)
        {
            nv_debug(NV_FUN_WRITE_EX,6, itemid, ret, modem_id);
            goto nv_writeEx_err;
        }
    }

    ret = nv_write_to_mem(pdata,datalen,file_info.file_id,ref_info.nv_off+offset);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX,7,itemid,datalen,0);
        goto nv_writeEx_err;
    }

    ret = nv_write_to_file(&ref_info);
    if(ret)
    {
        nv_debug(NV_FUN_WRITE_EX,8,itemid,datalen,ret);
        goto nv_writeEx_err;
    }
    nv_AddListNode(itemid);
    return NV_OK;
nv_writeEx_err:
    nv_mntn_record("\n[%s]:[0x%x]:[%d]\n",__FUNCTION__,itemid,modem_id);
    nv_help(NV_FUN_WRITE_EX);
    return ret;
}

u32 bsp_nvm_get_nv_num(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    return ctrl_info->ref_count;
}

u32 bsp_nvm_get_nvidlist(NV_LIST_INFO_STRU*  nvlist)
{
    u32 i;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);

    if(NULL == nvlist)
    {
        return NV_ERROR;
    }

    for(i = 0;i<ctrl_info->ref_count;i++)
    {
        nvlist[i].usNvId       = ref_info[i].itemid;
        nvlist[i].ucNvModemNum = ref_info[i].modem_num;
    }
    return NV_OK;
}

u32 bsp_nvm_get_len(u32 itemid,u32* len)
{
    u32 ret  = NV_ERROR;
    struct nv_ref_data_info_stru ref_info = {0};
    struct nv_file_list_info_stru file_info = {0};

    nv_debug(NV_API_GETLEN,0,itemid,0,0);
    if(NULL == len)
    {
        nv_debug(NV_API_GETLEN,1,itemid,0,0);
        return BSP_ERR_NV_INVALID_PARAM;
    }

    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        nv_debug(NV_API_GETLEN,3,itemid,0,0);
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }
    ret = nv_search_byid(itemid,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info, &file_info);
    if(NV_OK == ret)
    {
        *len = ref_info.nv_len;
        return NV_OK;
    }
    return ret;
}

u32 bsp_nvm_authgetlen(u32 itemid,u32* len)
{
    return bsp_nvm_get_len(itemid,len);
}



u32 bsp_nvm_dcread_direct(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_auth_dcread(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return bsp_nvm_dcread(modem_id,itemid,pdata,datalen);
}

u32 bsp_nvm_dcreadpart(u32 modem_id,u32 itemid,u32 offset,u8* pdata,u32 datalen)
{
    /*check init state*/
    if(false == nv_read_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_readEx(modem_id,itemid,offset,pdata,datalen);
}

u32 bsp_nvm_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    if(false == nv_write_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}

u32 bsp_nvm_auth_dcwrite(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return bsp_nvm_dcwrite(modem_id,itemid,pdata,datalen);
}

u32 bsp_nvm_dcwritepart(u32 modem_id,u32 itemid, u32 offset,u8* pdata,u32 datalen)
{
    if(false == nv_write_right(itemid))
    {
        return BSP_ERR_NV_MEM_INIT_FAIL;
    }

    return nv_writeEx(modem_id,itemid,offset,pdata,datalen);
}

u32 bsp_nvm_dcwrite_direct(u32 modem_id,u32 itemid, u8* pdata,u32 datalen)
{
    return nv_writeEx(modem_id,itemid,0,pdata,datalen);
}



/*lint -save -e529*/
u32 bsp_nvm_flushEx(u32 off,u32 len,u32 itemid)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 crc_count = 0;
    u32 skip_crc_count = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;

    nv_debug(NV_API_FLUSH,0,off,len,itemid);
    if((off+len) > (ddr_info->file_len))
    {
        nv_debug(NV_API_FLUSH,1,off,len,ddr_info->file_len);
        goto nv_flush_err;
    }

    if(nv_file_access((s8*)NV_IMG_PATH,0))
    {
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_WRITE);
        if(NULL == fp)
        {
            nv_debug(NV_API_FLUSH, 6, ret,0,0);
            goto nv_flush_err;
        }
        nv_file_close(fp);
        return bsp_nvm_flushEn();
    }
    else
    {
        fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_RW);
        if(NULL == fp)
        {
            ret = BSP_ERR_NV_NO_FILE;
            nv_debug(NV_API_FLUSH,2,ret,0,0);
            goto nv_flush_err;
        }
    }

    nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    /*如果支持CRC校验码则需要将CRC校验码写入文件*/
    if(NV_CRC_CHECK_YES)
    {
        crc_count = NV_CRC_CODE_COUNT((off + len - ctrl_info->ctrl_size)) - (off- ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
        skip_crc_count = (off - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
        len = crc_count * NV_CRC32_CHECK_SIZE;
        if(off + len > ddr_info->file_len)
        {
            len = len - (off + len - ddr_info->file_len);
        }
        off = ctrl_info->ctrl_size + (skip_crc_count)*NV_CRC32_CHECK_SIZE;
        (void)nv_file_seek(fp, off ,SEEK_SET);/*jump to write*/
        ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR + off,1,len,fp);
        if(ret != len)
        {
            nv_ipc_sem_give(IPC_SEM_NV_CRC);
            nv_file_close(fp);
            nv_debug(NV_API_FLUSH,3,0,ret,len);
            ret = BSP_ERR_NV_WRITE_FILE_FAIL;
            goto nv_flush_err;
        }
    
    
        (void)nv_file_seek(fp, ddr_info->file_len + (skip_crc_count + 1)* sizeof(u32) ,SEEK_SET);/*jump to write*/
        len = crc_count * sizeof(u32);
        ret = (u32)nv_file_write((u8*)(NV_DDR_CRC_CODE_OFFSET + skip_crc_count),1,len,fp);
        if(ret != len)
        {
            nv_ipc_sem_give(IPC_SEM_NV_CRC);
            nv_debug(NV_API_FLUSH,4,0,ret,len);
            ret = BSP_ERR_NV_WRITE_FILE_FAIL;
            nv_file_close(fp);
            goto nv_flush_err;
        }
    }
    else
    {
        (void)nv_file_seek(fp,(s32)off,SEEK_SET);
        ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR+off,1,len,fp);

        if(ret != len)
        {
            nv_ipc_sem_give(IPC_SEM_NV_CRC);
            nv_file_close(fp);
            nv_debug(NV_API_FLUSH,5,0,ret,len);
            ret = BSP_ERR_NV_WRITE_FILE_FAIL;
            goto nv_flush_err;
        }
    }
    nv_ipc_sem_give(IPC_SEM_NV_CRC);

    nv_file_close(fp);

    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s] len :0x%x, off :0x%x\n",__FUNCTION__,len,off);
    nv_help(NV_API_FLUSH);
    return ret;
}
/*lint -restore +e529*/

u32 bsp_nvm_flush(void)
{
    u32 ret;
    ret = nv_flushList();
    if(ret)
    {
        return ret;
    }
    return NV_OK;
}

u32 bsp_nvm_flushEn(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 writeLen = 0;

    nv_create_flag_file((s8*)NV_IMG_FLAG_PATH);
    nv_debug(NV_API_FLUSH,0,0,0,0);
    fp = nv_file_open((s8*)NV_IMG_PATH,(s8*)NV_FILE_WRITE);
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_API_FLUSH,1,ret,0,0);
        goto nv_flush_err;
    }

    if(NV_CRC_CHECK_YES)
    {
        writeLen = ddr_info->file_len + NV_CRC_CODE_COUNT(ddr_info->file_len - ctrl_info->ctrl_size)*sizeof(u32) + sizeof(u32);/*sizeof是文件尾结束符*/
    }
    else
    {
        writeLen = ddr_info->file_len;
    } 
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1, writeLen, fp);
    nv_file_close(fp);
    fp = NULL;
    if(ret != writeLen)
    {
        nv_debug(NV_API_FLUSH,2,(u32)(unsigned long)fp,ret,writeLen);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }
    nv_delete_flag_file((s8*)NV_IMG_FLAG_PATH);

    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_FLUSH);
    return ret;
}


u32 bsp_nvm_flushSys(u32 itemid)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 ulTotalLen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_create_flag_file((s8*)NV_SYS_FLAG_PATH);

    nv_debug(NV_FUN_FLUSH_SYS,0,0,0,0);
    if(nv_file_access((s8*)NV_FILE_SYS_NV_PATH,0))
    {
        fp = nv_file_open((s8*)NV_FILE_SYS_NV_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_FILE_SYS_NV_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        nv_debug(NV_FUN_FLUSH_SYS,1,ret,0,0);
        ret = BSP_ERR_NV_NO_FILE;
        goto nv_flush_err;
    }
    ulTotalLen = ddr_info->file_len;
    /*在nvdload分区文件末尾置标志0xabcd8765*/
    *( unsigned int* )( (u8*)NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_len )
        = ( unsigned int )NV_FILE_TAIL_MAGIC_NUM;
    ulTotalLen += sizeof(unsigned int);
    /*系统分区数据不做CRC校验，因此回写时不考虑CRC校验码的存放位置*/
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ulTotalLen,fp);
    nv_file_close(fp);
    if(ret != ulTotalLen)
    {
        nv_debug(NV_FUN_FLUSH_SYS,3,ret,ulTotalLen,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_flush_err;
    }

    nv_delete_flag_file((s8*)NV_SYS_FLAG_PATH);
    return NV_OK;

nv_flush_err:
    nv_mntn_record("\n[%s]\n",__func__);
    nv_help(NV_FUN_FLUSH_SYS);
    return ret;
}



u32 bsp_nvm_backup(void)
{
    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    FILE* fp = NULL;
    u32 writeLen = 0;
    nv_debug(NV_API_BACKUP,0,0,0,0);

    if( (ddr_info->acore_init_state != NV_INIT_OK)&&
        (ddr_info->acore_init_state != NV_KERNEL_INIT_DOING))
    {
        return NV_ERROR;
    }

    nv_create_flag_file((s8*)NV_BACK_FLAG_PATH);

    if(nv_file_access((s8*)NV_BACK_PATH,0))
    {
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_BACK_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_API_BACKUP,1,ret,0,0);
        goto nv_backup_fail;
    }

    /*在写文件前增加crc校验互斥锁,保证写入文件前内存crc未正在更新*/
    nv_ipc_sem_take(IPC_SEM_NV_CRC, IPC_SME_TIME_OUT);
    if(NV_CRC_CHECK_YES)
    {
        writeLen = ddr_info->file_len + NV_CRC_CODE_COUNT(ddr_info->file_len - ctrl_info->ctrl_size)*sizeof(u32) + sizeof(u32);
    }
    else
    {
        writeLen = ddr_info->file_len;
    }
    ret = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,writeLen,fp);
    nv_file_close(fp);
    nv_ipc_sem_give(IPC_SEM_NV_CRC);

    fp = NULL;
    if(ret != writeLen)
    {
        nv_debug(NV_API_BACKUP,3,ret,writeLen,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_backup_fail;
    }

    nv_delete_flag_file((s8*)NV_BACK_FLAG_PATH);

    return NV_OK;
nv_backup_fail:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_BACKUP);
    return ret;

}

u32 bsp_nvm_revert_user(void)
{
    return nv_revert_data(NV_BACK_PATH,g_ausNvResumeUserIdList,\
        bsp_nvm_getRevertNum(NV_USER_ITEM));
}


u32 bsp_nvm_revert_manufacture(void)
{
    return nv_revert_data(NV_BACK_PATH,g_ausNvResumeManufactureIdList,\
        bsp_nvm_getRevertNum(NV_MANUFACTURE_ITEM));
}

u32 bsp_nvm_revert_secure(void)
{
    return nv_revert_data(NV_BACK_PATH,g_ausNvResumeSecureIdList,\
        bsp_nvm_getRevertNum(NV_SECURE_ITEM));
}

u32 bsp_nvm_revert(void)
{
    u32 ret  = NV_ERROR;

    nv_debug(NV_API_REVERT,0,0,0,0);

    nv_printf("enter to revert nv !\n");
    ret = bsp_nvm_revert_user();
    if(ret)
    {
        nv_debug(NV_API_REVERT,1,ret,0,0);
        goto nv_revert_fail;
    }

    ret = bsp_nvm_revert_manufacture();
    if(ret)
    {
        nv_debug(NV_API_REVERT,2,ret,0,0);
        goto nv_revert_fail;
    }
    ret = bsp_nvm_revert_secure();
    if(ret)
    {
        nv_debug(NV_API_REVERT,3,ret,0,0);
        goto nv_revert_fail;
    }
    nv_printf("revert nv end !\n");

    return NV_OK;
nv_revert_fail:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_API_REVERT);
    return ret;
}




u32 bsp_nvm_update_default(void)
{
    u32 ret = NV_ERROR;
    FILE* fp = NULL;
    u32 datalen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_debug(NV_FUN_UPDATE_DEFAULT,0,0,0,0);

    if(ddr_info->acore_init_state != NV_INIT_OK)
    {
        return NV_ERROR;
    }


    if(nv_file_access((s8*)NV_DEFAULT_PATH,0))
    {
        fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_WRITE);
    }
    else
    {
        fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_RW);
    }
    if(NULL == fp)
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_FUN_UPDATE_DEFAULT,2,ret,0,0);
        goto nv_update_default_err;
    }

    datalen = (u32)nv_file_write((u8*)NV_GLOBAL_CTRL_INFO_ADDR,1,ddr_info->file_len,fp);

    nv_file_close(fp);
    if(datalen != ddr_info->file_len)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,3,ret,ddr_info->file_len,0);
        ret = BSP_ERR_NV_WRITE_FILE_FAIL;
        goto nv_update_default_err;
    }

    ret = bsp_nvm_backup();
    if(ret)
    {
        nv_debug(NV_FUN_UPDATE_DEFAULT,4,ret,0,0);
        goto nv_update_default_err;
    }


    return NV_OK;
nv_update_default_err:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_FUN_UPDATE_DEFAULT);
    return ret;
}


void bsp_nvm_remove_dload_packet(void)
{
    if(!nv_file_access(NV_DLOAD_PATH,0))
    {
        nv_file_remove(NV_DLOAD_PATH);
    }

    if(!nv_file_access(NV_XNV_CARD1_PATH,0))
    {
        nv_file_remove(NV_XNV_CARD1_PATH);
    }

    if(!nv_file_access(NV_XNV_CARD2_PATH,0))
    {
        nv_file_remove(NV_XNV_CARD2_PATH);
    }

    if(!nv_file_access(NV_CUST_CARD1_PATH,0))
    {
        nv_file_remove(NV_CUST_CARD1_PATH);
    }

    if(!nv_file_access(NV_CUST_CARD2_PATH,0))
    {
        nv_file_remove(NV_CUST_CARD2_PATH);
    }

    if(!nv_file_access(NV_XNV_CARD1_MAP_PATH,0))
    {
        nv_file_remove(NV_XNV_CARD1_MAP_PATH);
    }

    if(!nv_file_access(NV_XNV_CARD2_MAP_PATH,0))
    {
        nv_file_remove(NV_XNV_CARD2_MAP_PATH);
    }
}



u32 bsp_nvm_revert_defaultEx(const s8* path)
{
    u32 ret = NV_ERROR;
    u32 i = 0;
    FILE* fp = NULL;
    struct nv_ctrl_file_info_stru  manu_ctrl_file = {0};
    u8* ctrl_file_data = NULL;
    struct nv_global_ddr_info_stru manu_ddr_info = {0};



    nv_debug(NV_FUN_REVERT_DEFAULT,0,0,0,0);
    fp = nv_file_open((s8*)path,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,1,0,0,0);
        return BSP_ERR_NV_NO_FILE;
    }

    ret = (u32)nv_file_read((u8*)&manu_ctrl_file,1,sizeof(manu_ctrl_file),fp);
    if(ret != sizeof(manu_ctrl_file))
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,2,ret,0,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto file_close_err;
    }
    nv_file_seek(fp,0,SEEK_SET);
    ctrl_file_data = (u8*)nv_malloc(manu_ctrl_file.ctrl_size+1);
    if(NULL == ctrl_file_data)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,3,BSP_ERR_NV_MALLOC_FAIL,0,0);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto file_close_err;
    }

    ret = (u32)nv_file_read(ctrl_file_data,1,manu_ctrl_file.ctrl_size,fp);
    if(ret != manu_ctrl_file.ctrl_size)
    {
        nv_error_printf("ret 0x%x,ctrl size 0x%x\n",ret,manu_ctrl_file.ctrl_size);
        nv_debug(NV_FUN_REVERT_DEFAULT,4,ret,manu_ctrl_file.ctrl_size,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto free_ctrl_file;
    }

    ret = nv_init_file_info((u8*)ctrl_file_data,(u8*)&manu_ddr_info);
    if(ret)
    {
        nv_debug(NV_FUN_REVERT_DEFAULT,5,ret,0,0);
        ret = BSP_ERR_NV_MEM_INIT_FAIL;
        goto free_ctrl_file;
    }
    for(i = 0;i<manu_ctrl_file.file_num;i++)
    {
        ret = nv_revert_default(fp,manu_ddr_info.file_info[i].size);
        if(ret)
        {
            nv_debug(NV_FUN_REVERT_DEFAULT,6,ret,manu_ddr_info.file_info[i].size,0);
            goto free_ctrl_file;
        }
    }
    nv_file_close(fp);
    nv_free(ctrl_file_data);


    return NV_OK;
free_ctrl_file:
    nv_free(ctrl_file_data);
file_close_err:
    nv_file_close(fp);
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    nv_help(NV_FUN_REVERT_DEFAULT);
    return ret;

}


u32 bsp_nvm_revert_default(void)
{
    u32 ret = NV_ERROR;

    printf("enter to set default nv !\r\n");
    
    ret = nv_revert_data_with_crc(NV_DEFAULT_PATH,g_ausNvResumeDefualtIdList,\
             bsp_nvm_getRevertNum(NV_MBB_DEFUALT_ITEM));
    if(ret)
    {
        printf("Set default nv nv_revert_data error!\r\n");
        goto err_out;
    }
    
    ret = bsp_nvm_flushEn();
    ret |= bsp_nvm_flushSys(NV_ERROR);
    if(ret)
    {
        printf("Set default nv bsp_nvm_flush error!\r\n");
        goto err_out;
    }

err_out:
    return ret;
}



/*lint -save -e438*/
u32 bsp_nvm_key_check(void)
{
    FILE* fp = NULL;
    u32 ret = NV_ERROR;
    u32 datalen = 0;        /*read file len*/
    u32 file_offset = 0;
    u8* bak_ctrl_file = NULL;
    u8* bak_data = NULL;           /*single nv data ,max len 2048byte*/
    u8* mem_data = NULL;
    struct nv_ctrl_file_info_stru    bak_ctrl_info = {0};   /*bak file ctrl file head*/
    struct nv_file_list_info_stru    bak_file_info  = {0};
    struct nv_global_ddr_info_stru   bak_ddr_info   = {0};
    struct nv_ref_data_info_stru     bak_ref_info   = {0};

    struct nv_ref_data_info_stru    mem_ref_info  = {0};
    struct nv_file_list_info_stru   mem_file_info = {0};

    if(nv_file_access((s8*)NV_DEFAULT_PATH,0))  /*没有文件则直接返回ok*/
    {
        return NV_OK;
    }

    nv_debug(NV_FUN_KEY_CHECK,0,0,0,0);
    fp = nv_file_open((s8*)NV_DEFAULT_PATH,(s8*)NV_FILE_READ);
    if(NULL == fp)
    {
        nv_debug(NV_FUN_KEY_CHECK,1,0,0,0);
        return BSP_ERR_NV_NO_FILE;
    }

    /*first read ctrl file head*/
    datalen = (u32)nv_file_read((u8*)(&bak_ctrl_info),1,sizeof(bak_ctrl_info),fp);
    if(datalen != sizeof(bak_ctrl_info))
    {
        nv_debug(NV_FUN_KEY_CHECK,2,datalen,0,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto close_file;
    }

    if(bak_ctrl_info.magicnum != NV_CTRL_FILE_MAGIC_NUM)
    {
        nv_debug(NV_FUN_KEY_CHECK,3,bak_ctrl_info.magicnum,0,0);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto close_file;
    }

    bak_ctrl_file = (u8*)nv_malloc(bak_ctrl_info.ctrl_size);
    if(NULL == bak_ctrl_file)
    {
        nv_debug(NV_FUN_KEY_CHECK,4,bak_ctrl_info.ctrl_size,0,0);
        ret = BSP_ERR_NV_MALLOC_FAIL;
        goto close_file;
    }
    /*second :read all ctrl file*/
    nv_file_seek(fp,0,SEEK_SET); /*jump to file head*/
    ret = (u32)nv_file_read(bak_ctrl_file,1,bak_ctrl_info.ctrl_size,fp);
    if(ret != bak_ctrl_info.ctrl_size)
    {
        nv_debug(NV_FUN_KEY_CHECK,5,ret,bak_ctrl_info.ctrl_size,0);
        ret = BSP_ERR_NV_READ_FILE_FAIL;
        goto free_ctrl_data;

    }
    /*third :init ctrl file info to bak_ddr_info*/
    ret = nv_init_file_info((u8*)bak_ctrl_file,(u8*)&bak_ddr_info);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,6,ret,0,0);
        goto free_ctrl_data;
    }

    /*forth :look for imei id in bak & cur mem*/
    ret = nv_search_byid(NV_ID_DRV_IMEI,bak_ctrl_file,&bak_ref_info,&bak_file_info);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,7,ret,0,0);
        goto free_ctrl_data;
    }
    ret = nv_search_byid(NV_ID_DRV_IMEI,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&mem_ref_info,&mem_file_info);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,8,ret,0,0);
        goto free_ctrl_data;
    }

    /*compare info*/
    if( (mem_ref_info.nv_len    != bak_ref_info.nv_len) ||
        (mem_ref_info.modem_num != bak_ref_info.modem_num)
        )
    {
        nv_debug(NV_FUN_KEY_CHECK,9,bak_ref_info.nv_len,bak_ref_info.modem_num,mem_ref_info.nv_len);
        ret = BSP_ERR_NV_FILE_ERROR;
        goto free_ctrl_data;
    }

    if(   (mem_ref_info.modem_num == NV_USIMM_CARD_2)
        &&(bak_ref_info.modem_num == NV_USIMM_CARD_2))
    {
        datalen = mem_ref_info.nv_len * 2;
    }
    else
    {
        datalen = mem_ref_info.nv_len;
    }


    bak_data = (u8*)nv_malloc(datalen);
    mem_data = (u8*)nv_malloc(datalen);
    if((NULL == bak_data)||(mem_data == NULL))
    {
        nv_debug(NV_FUN_KEY_CHECK,10,0,0,0);
        goto free_ctrl_data;
    }

    /*count data offset in bak file*/
    file_offset = bak_ddr_info.file_info[bak_file_info.file_id-1].offset +bak_ref_info.nv_off;

    nv_file_seek(fp,(s32)file_offset,SEEK_SET);
    ret = (u32)nv_file_read(bak_data,1,datalen,fp);/*把数据从文件中指定偏移处读出*/
    if(ret != datalen)
    {
        nv_debug(NV_FUN_KEY_CHECK,11,ret,datalen,0);
        goto free_data;
    }

    ret = nv_read_from_mem(mem_data,datalen,mem_file_info.file_id,mem_ref_info.nv_off);
    if(ret)
    {
        nv_debug(NV_FUN_KEY_CHECK,12,0,0,0);
        goto free_data;
    }

    nv_file_close(fp);
    ret = (u32)memcmp(mem_data,bak_data,datalen);  /*比较数据差异*/
    if(ret)
    {
        ret = bsp_nvm_revert_defaultEx((s8*)NV_DEFAULT_PATH);/* [false alarm]:ret is in using */

        ret |= bsp_nvm_flush();/* [false alarm]:ret is in using */
        ret |= bsp_nvm_flushSys(NV_ERROR);/* [false alarm]:ret is in using */
    }
    nv_free(mem_data);
    nv_free(bak_data);
    nv_free(bak_ctrl_file);


    return NV_OK;
free_data:
    nv_free(mem_data);
    nv_free(bak_data);
free_ctrl_data:
    nv_free(bak_ctrl_file);
close_file:
    nv_file_close(fp);
    nv_mntn_record("\n%s\n",__func__);
    nv_help(NV_FUN_KEY_CHECK);
    return ret;
}
/*lint -restore -e747*/


s32 bsp_nvm_icc_task(void* parm)
{
    s32 ret = -1;
    struct nv_icc_stru icc_req;
    /* coverity[var_decl] */
    struct nv_icc_stru icc_cnf;
    u32 chanid;


    /* coverity[no_escape] */
    for(;;)
    {
        osl_sem_down(&g_nv_ctrl.task_sem);
        memset(g_nv_ctrl.nv_icc_buf,0,NV_ICC_BUF_LEN);
        memset(&icc_req,0,sizeof(icc_req));

        g_nv_ctrl.opState = NV_OPS_STATE;
        wake_lock(&g_nv_ctrl.wake_lock);

        /*如果当前处于睡眠状态，则等待唤醒处理*/
        if(g_nv_ctrl.pmState == NV_SLEEP_STATE)
        {
            printk("%s cur state in sleeping,wait for resume end!\n",__func__);
            continue;
        }

        chanid = ICC_CHN_NV << 16 | NV_RECV_FUNC_AC;
        ret = bsp_icc_read(chanid,g_nv_ctrl.nv_icc_buf,NV_ICC_BUF_LEN);
        if(((u32)ret > NV_ICC_BUF_LEN)||(ret <= 0))
        {
            nv_debug_printf("bsp icc read error, chanid :0x%x ret :0x%x\n",chanid,ret);

            chanid = ICC_CHN_MCORE_ACORE << 16 | NV_RECV_FUNC_AM;
            ret = bsp_icc_read(chanid,g_nv_ctrl.nv_icc_buf,NV_ICC_BUF_LEN);
            if(((u32)ret > NV_ICC_BUF_LEN)||(ret <= 0))
            {
                g_nv_ctrl.opState = NV_IDLE_STATE;
                wake_unlock(&g_nv_ctrl.wake_lock);
                nv_debug_printf("bsp icc read error, chanid :0x%x ret :0x%x\n",chanid,ret);
                continue;
            }
        }

        g_nv_ctrl.task_proc_count ++;

        memcpy(&icc_req,g_nv_ctrl.nv_icc_buf,sizeof(icc_req));
        /*lint -save -e578 -e530*/
        nv_debug_trace(&icc_req, sizeof(icc_req));
        /*lint -restore +e578 +e530*/
        if(icc_req.msg_type == NV_ICC_REQ)
        {
            icc_cnf.ret = nv_flushList();
            /*如果工具侧通过核间通信要求写的是机要nv，则启动备份*/
            if(true == nv_isSecListNv(icc_req.itemid))
            {
                ret = bsp_nvm_backup();
            }
            if(NV_ERROR == ret )
            {
                printf("nvm_backup icc Err!\n");
            }
        }
        else if(icc_req.msg_type == NV_ICC_REQ_SYS)
        {
            icc_cnf.ret = bsp_nvm_flushSys(icc_req.itemid);
        }
        else if(icc_req.msg_type == NV_ICC_RESUME)
        {
            icc_cnf.ret = nv_resume_ddr_from_img();
        }
        else if(icc_req.msg_type == NV_ICC_REQ_FLUSH)
        {
            icc_cnf.ret = nv_flushList();
        }
        else
        {
            printf("[%s] invalid parameter :0x%x\n",__func__,icc_req.msg_type);
            wake_unlock(&g_nv_ctrl.wake_lock);
            osl_sem_up(&g_nv_ctrl.task_sem);
            continue;
        }

        nv_pm_trace(icc_req.itemid,icc_req.slice);

        icc_cnf.msg_type = NV_ICC_CNF;
        icc_cnf.data_off = icc_req.data_off;
        icc_cnf.data_len = icc_req.data_len;
        icc_cnf.itemid   = icc_req.itemid;
        icc_cnf.slice    = icc_req.slice;

        /* coverity[uninit_use_in_call] */
        ret = (s32)nv_icc_send(chanid,(u8*)&icc_cnf,sizeof(icc_cnf));
        if(ret)
        {
            printf("[%s] icc send error !\n",__func__);
        }
        wake_unlock(&g_nv_ctrl.wake_lock);
        osl_sem_up(&g_nv_ctrl.task_sem);
    }
}


u32 bsp_nvm_xml_decode(void)
{
    u32 ret = NV_ERROR;

    if(!nv_file_access(NV_XNV_CARD1_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD1_PATH,NV_XNV_CARD1_MAP_PATH,NV_USIMM_CARD_1);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_XNV_CARD2_PATH,0))
    {
        ret = nv_xml_decode(NV_XNV_CARD2_PATH,NV_XNV_CARD2_MAP_PATH,NV_USIMM_CARD_2);
        if(ret)
        {
            return ret;
        }
    }


    /*CUST XML 无对应MAP文件，传入空值即可*/
    if(!nv_file_access(NV_CUST_CARD1_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD1_PATH,NULL,NV_USIMM_CARD_1);
        if(ret)
        {
            return ret;
        }
    }

    if(!nv_file_access(NV_CUST_CARD2_PATH,0))
    {
        ret = nv_xml_decode(NV_CUST_CARD2_PATH,NULL,NV_USIMM_CARD_2);
        if(ret)
        {
            return ret;
        }
    }

    return NV_OK;
}


u32 bsp_nvm_reload(void)
{
    u32 ret;
    FILE* fp;
    u32 datalen;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_debug(NV_FUN_MEM_INIT,0,NV_GLOBAL_INFO_ADDR,NV_GLOBAL_CTRL_INFO_ADDR,0);

    /*重新加载之前清理各分区状态位*/
    ddr_info->mem_file_type &= ~(0x1 << NV_MEM_DATA_NVSYS_IMG);
    ddr_info->mem_file_type &= ~(0x1 << NV_MEM_DATA_NVBACK);

    if(!nv_file_access(NV_IMG_PATH,0))
    {
        nv_mntn_record("load from %s\n",NV_IMG_PATH);
        fp = nv_file_open(NV_IMG_PATH,NV_FILE_READ);
        if(NULL == fp)
        {
            ret = BSP_ERR_NV_NO_FILE;
            nv_debug(NV_FUN_MEM_INIT,1,ret,0,0);
            goto load_back_file;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen, NV_FILE_SYS_NV);
        nv_file_close(fp);
        if(ret)
        {
            nv_mntn_record("%s :nv_read_from_file error ret = %d\n",NV_IMG_PATH,ret);
            nv_debug(NV_FUN_MEM_INIT,2,ret,0,0);
            goto load_back_file;
        }
        ret = nv_set_crc_offset();
        if(ret)
        {
            nv_mntn_record("%s :nv_set_crc_offset error ret = %d\n",NV_IMG_PATH,ret);
            nv_debug(NV_FUN_MEM_INIT,3,ret,0,0);
            goto load_back_file;
        }
        ret = nv_check_ddr_crc();
        if(ret)
        {
            nv_mntn_record("%s :nv_check_ddr_crc error ret = %d\n",NV_IMG_PATH,ret);
            nv_debug(NV_FUN_MEM_INIT,4,ret,0,0);
            goto load_back_file;
        }

        ddr_info->mem_file_type |= 0x1 << NV_MEM_DATA_NVSYS_IMG;
        return NV_OK;

    }


/*hi3630版本如果mnvm2:0目录下没有，则从modem_log目录中查找*/



load_back_file:
    if(!nv_file_access(NV_BACK_PATH,0))
    {
        nv_mntn_record("load from %s\n",NV_BACK_PATH);
        fp = nv_file_open(NV_BACK_PATH,NV_FILE_READ);
        if(NULL == fp)
        {
            ret = BSP_ERR_NV_NO_FILE;
            nv_debug(NV_FUN_MEM_INIT,5,ret,0,0);
            goto reload_err_out;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen, NV_FILE_BACKUP);
        nv_file_close(fp);
        if(ret)
        {
            nv_mntn_record("%s :nv_read_from_file error ret = %d\n",NV_BACK_PATH,ret);
            nv_debug(NV_FUN_MEM_INIT,6,ret,0,0);
            goto reload_err_out;
        }
        ret = nv_set_crc_offset();
        if(ret)
        {   
            nv_mntn_record("%s :nv_set_crc_offset error ret = %d\n",NV_BACK_PATH,ret);
            nv_debug(NV_FUN_MEM_INIT,7,ret,0,0);
            goto reload_err_out;
        }

        ret = nv_check_ddr_crc();
        if(ret)
        {
            nv_mntn_record("%s :nv_check_ddr_crc error ret = %d\n",NV_BACK_PATH,ret);
            nv_debug(NV_FUN_MEM_INIT,8,ret,0,0);
            goto reload_err_out;
        }
        /*从备份区加载需要首先写入工作区*/
        ret = bsp_nvm_flushEn();
        if(ret)
        {
            nv_debug(NV_FUN_MEM_INIT,9,0,0,0);
            goto reload_err_out;
        }
        ddr_info->mem_file_type |= 0x1 << NV_MEM_DATA_NVBACK;
    }
    else
    {
        ddr_info->mem_file_type = 0;
    }
    return NV_OK;
reload_err_out:
    nv_mntn_record("\n%s\n",__func__);
    nv_mntn_record("load nvimg and nvbak failed, nv_load_err_proc\n");
    ret = nv_load_err_proc();
    if(ret)
    {
        nv_mntn_record("%s %d ,err revert proc ,ret :0x%x\n",__func__,__LINE__,ret);
        nv_help(NV_FUN_MEM_INIT);
        return ret;
    }
    ddr_info->mem_file_type |= 0x1 << NV_MEM_DATA_NVSYS_IMG;
    return NV_OK;
}



s32 bsp_nvm_restore_online_handle(NV_RESTORE_STATUS stuType)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        printf("Dload smem_data malloc fail!\n");
        return -1;
    }

    printf("smem_data->smem_online_upgrade_flag :0x%x\n", 
                smem_data->smem_online_upgrade_flag);
    printf("smem_data->smem_multiupg_flag :0x%x\n", 
                smem_data->smem_multiupg_flag);
    if(SMEM_ONNR_FLAG_NUM == smem_data->smem_online_upgrade_flag)
    {
        if(NV_RESTRORE_SUCCESS == stuType)
        {
            /*在线升级NV自动恢复阶段魔术字清零*/
            smem_data->smem_online_upgrade_flag = 0;

            /*组播升级不重启*/
            if(SMEM_MULTIUPG_FLAG_NUM == smem_data->smem_multiupg_flag)
            {
                smem_data->smem_multiupg_flag = 0;

                printf("MULTI UPG success, do not reboot.\n");
            }
            else
            {
                smem_data->smem_switch_pcui_flag = 0;
                printf("MBB:Online Upgrade Sucessful,reboot.\n");
                /*单板重启进入正常模式*/
                BSP_OM_SoftReboot();
            }
        }
        else
        {
            if(SMEM_MULTIUPG_FLAG_NUM == smem_data->smem_multiupg_flag)
            {
                smem_data->smem_multiupg_flag = 0;
            }

            printf("MBB:Online Upgrade failed !\n");
        }
    }
    return 0;
}



u32 clean_mmi_nv_flag(void)
{
    u32 ret = NV_OK;
    /*出错的时候直接返回非0值，正确的时候返回NV_OK*/
    return ret;
}


u32 bsp_nvm_upgrade(void)
{
    u32 ret;
    FILE* fp;
    u32 datalen = 0;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    u32 mem_type = 0;
    bool RevertFlag = false;

    ddr_info->mem_file_type &= ~(0x1 << NV_MEM_DATA_NVDLOAD); /*清理下载分区内存标志位*/
    mem_type                 = ddr_info->mem_file_type;       /*记录当前内存数据标志位*/

    nv_debug(NV_FUN_UPGRADE_PROC,0,0,0,0);

    /*如果升级包中存在以下几个文件，则均认为升级操作，均需要进行校准nv数据恢复*/
    if(((!nv_file_access(NV_DLOAD_PATH,0)) ||
        (!nv_file_access(NV_XNV_CARD1_PATH,0)) ||
        (!nv_file_access(NV_XNV_CARD2_PATH,0)) ||
        (!nv_file_access(NV_CUST_CARD1_PATH,0)) ||
        (!nv_file_access(NV_CUST_CARD2_PATH,0))
        )&&(mem_type)
       )
    {
        ret = bsp_nvm_backup();
        if(ret)
        {
            nv_mntn_record("%s :bsp_nvm_backup error ret = %d\n",__func__,ret);
            nv_debug(NV_FUN_UPGRADE_PROC,1,ret,0,0);
            goto upgrade_fail_out;
        }
        RevertFlag = true;    /*记录升级过程中的备份动作*/
        g_clean_mmi_flag = 1;
    }

    if(!nv_file_access(NV_DLOAD_PATH,0))
    {
        fp = nv_file_open(NV_DLOAD_PATH,NV_FILE_READ);
        if(NULL == fp)
        {
            ret = BSP_ERR_NV_NO_FILE;
            nv_debug(NV_FUN_UPGRADE_PROC,2,ret,0,0);
            goto upgrade_fail_out;
        }

        ret = nv_read_from_file(fp,(u8*)NV_GLOBAL_CTRL_INFO_ADDR,&datalen, NV_FILE_DLOAD);
        ret = nv_set_crc_offset();
        if(ret)
        {
            nv_mntn_record("%s :nv_set_crc_offset error ret = %d\n",__func__,ret);
            nv_printf("error set crc offset\n");
            nv_debug(NV_FUN_UPGRADE_PROC,3,ret,0,0);
            goto upgrade_fail_out;
        }
        nv_file_close(fp);
        if(ret)
        {
            nv_debug(NV_FUN_UPGRADE_PROC,4,ret,datalen,0);
            goto upgrade_fail_out;
        }
        if(false == nv_dload_file_check())
        {
            nv_mntn_record("%s :nv_dload_file_check error \n",__func__);
            nv_debug(NV_FUN_UPGRADE_PROC,5,ret,0,0);
            goto upgrade_fail_out;
        }

        ddr_info->mem_file_type |= (0x1 << NV_MEM_DATA_NVDLOAD);
    }
    else if(!mem_type)  /*内存与升级包中均没有数据，则需要返回错误*/
    {
        ret = BSP_ERR_NV_NO_FILE;
        nv_debug(NV_FUN_UPGRADE_PROC,6,ret,0,0);
        goto upgrade_fail_out;
    }

    ret = bsp_nvm_xml_decode();
    if(ret)
    {
        nv_mntn_record("%s :bsp_nvm_xml_decode error ret = %d\n",__func__,ret);
        nv_debug(NV_FUN_UPGRADE_PROC,7,ret,0,0);
        goto upgrade_fail_out;
    }

/*lint -save -e731*/
    if(RevertFlag == true)/*与备份动作同步，如果出现备份操作，则同样需要进行恢复动作*/
    {
        ret = bsp_nvm_revert();
        if(ret)
        {
            nv_mntn_record("%s :bsp_nvm_revert error ret = %d\n",__func__,ret);
            nv_debug(NV_FUN_UPGRADE_PROC,8,ret,0,0);
            goto upgrade_fail_out;
        }
    }
    ret = nv_make_ddr_crc();
    if(ret)
    {
        nv_mntn_record("%s :nv_make_ddr_crc error ret = %d\n",__func__,ret);
        nv_debug(NV_FUN_UPGRADE_PROC,9,ret,0,0);
        goto upgrade_fail_out;
    }

    mem_type = ddr_info->mem_file_type &(~(0x1 << NV_MEM_DATA_NVSYS_IMG));/*内存中是否存在非sys分区及工作分区数据*/
    if(mem_type)
    {
        ret = bsp_nvm_flushEn();
        if(ret)
        {
            nv_debug(NV_FUN_UPGRADE_PROC,10,ret,0,0);
            goto upgrade_fail_out;
        }
    }

    if(RevertFlag == true)/*只有在升级情况下更新备份区*/
    {
        ret = bsp_nvm_backup();
        if(ret)
        {
            nv_debug(NV_FUN_UPGRADE_PROC,11,ret,0,0);
            goto upgrade_fail_out;
        }
    }
/*lint -restore +e731*/

    /*检查是否有升级包数据，进行删除*/
    bsp_nvm_remove_dload_packet();

    if(mem_type)/*不能每次启动都要刷，要减少flushsys 次数*/
    {
        ret = bsp_nvm_flushSys(NV_ERROR);
        if(ret)
        {
            nv_debug(NV_FUN_UPGRADE_PROC,12,ret,0,0);
            goto upgrade_fail_out;
        }
    }

    return NV_OK;
upgrade_fail_out:
    nv_mntn_record("\n%s\n",__func__);
    nv_help(NV_FUN_UPGRADE_PROC);
    return NV_ERROR;
}


s32 bsp_nvm_kernel_init(void)
{
    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    nv_debug(NV_FUN_KERNEL_INIT,0,0,0,0);
    
    
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printf("nv_file_init: smem_data is NULL \n");
        return -1;  

    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，屏蔽nv模块的启动*/
        printf("entry update not init nvim !\n");
        return -1;  
    }

    /*sem & lock init*/
    spin_lock_init(&g_nv_ctrl.spinlock);
    osl_sem_init(0,&g_nv_ctrl.task_sem);
    osl_sem_init(1,&g_nv_ctrl.rw_sem);
    osl_sem_init(0,&g_nv_ctrl.cc_sem);
    wake_lock_init(&g_nv_ctrl.wake_lock,WAKE_LOCK_SUSPEND,"nv_wakelock");
    g_nv_ctrl.shared_addr = NV_GLOBAL_INFO_ADDR;

    nv_mntn_record("Balong nv init  start!\n");

    /*file info init*/
    ret = nv_file_init();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,1,ret,0,0);
        goto nv_init_fail;
    }
    /*check boot init state*/
    if((ddr_info->acore_init_state != NV_BOOT_INIT_OK))
    {
        nv_mntn_record("fast boot nv init fail !\n");
        nv_show_fastboot_err();
        memset(ddr_info,0,sizeof(struct nv_global_ddr_info_stru));
    }

    ddr_info->acore_init_state = NV_KERNEL_INIT_DOING;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    /*初始化开始，从新加载数据到内存中*/
    ret = bsp_nvm_reload();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,2,ret,0,0);
        goto nv_init_fail;
    }

    /*升级处理流程*/
    ret = bsp_nvm_upgrade();
    if(ret)
    {
        nv_debug(NV_FUN_KERNEL_INIT,3,ret,0,0);
        goto nv_init_fail;
    }
    ddr_info->acore_init_state = NV_INIT_OK;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);
    INIT_LIST_HEAD(&g_nv_ctrl.stList);
/*lint -save -e740*/
/*  处理单板的CAT等级，如果处理失败，则不再做处理  */

    ret = (u32)osl_task_init("drv_nv",15,1024,bsp_nvm_icc_task,NULL,(u32*)&g_nv_ctrl.task_id);
    if(ret)
    {
        nv_mntn_record("[%s]:nv task init err! ret :0x%x\n",__func__,ret);
        goto nv_init_fail;
    }
/*lint -restore +e740*/

    if(   (ret = nv_icc_chan_init(NV_RECV_FUNC_AC))\
        ||(ret = nv_icc_chan_init(NV_RECV_FUNC_AM))\
        )
    {
        goto nv_init_fail;
    }

    /*to do:nv id use macro define*/
    ret = bsp_nvm_read(NV_ID_MSP_FLASH_LESS_MID_THRED,(u8*)(&(g_nv_ctrl.mid_prio)),sizeof(u32));
    if(ret)
    {
        g_nv_ctrl.mid_prio = 20;
        printf("read 0x%x fail,use default value! ret :0x%x\n",NV_ID_MSP_FLASH_LESS_MID_THRED,ret);
    }
    nvchar_init();
    /*在一键升级备份恢复NV的时候对烧片版本的MMI标记进行清0操作*/
        if(1 == g_clean_mmi_flag)
        {
            ret = clean_mmi_nv_flag();
            if(ret)
            {
                /*5为debug级别*/
                nv_debug(NV_FUN_KERNEL_INIT,5,ret,0,0);
                goto nv_init_fail;
            }
        }
        g_clean_mmi_flag = 0;
    nv_mntn_record("Balong nv init ok!\n");
    ret = bsp_nvm_restore_online_handle(NV_RESTRORE_SUCCESS);
    if(ret)
    {
        return ret;
    }
    return NV_OK;

nv_init_fail:
    nv_mntn_record("\n[%s]\n",__FUNCTION__);
    ddr_info->acore_init_state = NV_INIT_FAIL;
    nv_help(NV_FUN_KERNEL_INIT);
    show_ddr_info();
    bsp_nvm_restore_online_handle(NV_RESTRORE_FAIL);
    return -1;
}


s32 bsp_nvm_remain_init(void)
{
    return 0;
}

/*lint -save -e529*/
static void bsp_nvm_exit(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    /*关机写数据*/
    (void)bsp_nvm_flush();
    (void)bsp_nvm_backup();
    /*清除标志*/
    memset(ddr_info,0,sizeof(struct nv_global_ddr_info_stru));
}
/*lint -restore +e529*/


u32 nvm_read_rand(u32 nvid)
{
    u32 ret;
    u8* tempdata;
    u32 i= 0;
    struct nv_ref_data_info_stru ref_info = {0};
    struct nv_file_list_info_stru file_info = {0};

    ret = nv_search_byid(nvid, (u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info,&file_info);
    if(NV_OK != ret)
    {
        return ret;
    }
    printf("[0x%x]:len 0x%x,off 0x%x,file id %d\n",nvid,ref_info.nv_len,ref_info.nv_off,ref_info.file_id);

    tempdata = (u8*)nv_malloc((u32)(ref_info.nv_len) +1);
    if(NULL == tempdata)
    {
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    ret = bsp_nvm_read(nvid,tempdata,ref_info.nv_len);
    if(NV_OK != ret)
    {
        nv_free(tempdata);
        return BSP_ERR_NV_READ_DATA_FAIL;
    }

    for(i=0;i<ref_info.nv_len;i++)
    {
        if((i%32) == 0)
        {
            printf("\n");
        }
        printf("%02x ",(u8)(*(tempdata+i)));
    }
    nv_free(tempdata);
    printf("\n\n");
    return 0;
}


u32 nvm_read_randex(u32 nvid,u32 modem_id)
{
	u32 ret;    u8* tempdata;    u32 i= 0;
	struct nv_ref_data_info_stru ref_info = {0};
	struct nv_file_list_info_stru file_info = {0};

	ret = nv_search_byid(nvid, (u8*)NV_GLOBAL_CTRL_INFO_ADDR,&ref_info,&file_info);
	if(NV_OK != ret)
	{
		return ret;
	}
	if(ref_info.nv_len == 0)
	{
		return NV_ERROR;
	}

	printf("[0x%x]:len 0x%x,off 0x%x,file id %d\n",nvid,ref_info.nv_len,ref_info.nv_off,ref_info.file_id);
	printf("[0x%x]:dsda 0x%x\n",nvid,ref_info.modem_num);

	tempdata = (u8*)nv_malloc((u32)(ref_info.nv_len) +1);
	if(NULL == tempdata)
	{
		return BSP_ERR_NV_MALLOC_FAIL;
	}
	ret = bsp_nvm_dcread(modem_id,nvid,tempdata,ref_info.nv_len);
	if(NV_OK != ret)
	{
		nv_free(tempdata);
		return BSP_ERR_NV_READ_DATA_FAIL;
	}

	for(i=0;i<ref_info.nv_len;i++)
	{
		if((i%32) == 0)
		{
			printf("\n");
		}
		printf("%02x ",(u8)(*(tempdata+i)));
	}

	printf("\n\n");
	nv_free(tempdata);

	return 0;

}

/*lint -save -e19*/
module_init(bsp_nvm_kernel_init);
module_exit(bsp_nvm_exit);
/*lint -restore +e19*/

/*lint -restore*/





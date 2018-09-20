



/*lint -save -e537*/
#include "bsp_nvim.h"
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_xml_dec.h"
#include "bsp_hardtimer.h"
/*lint -restore +e537*/


u32 nv_kernel_init_test(void);
u32 nv_remain_init_test(void);
u32 nv_read_part_test(u32 nvid, u32 off, u32 len);
u32 nv_read_rand_test(u32 nvid);
u32 nv_get_len_test01(void);
u32 nv_get_len_test02(void);
u32 nv_read_test_01(void);
u32 nv_read_test_02(void);
u32 nv_read_test_03(void);
u32 nv_read_test_04(void);
u32 nv_read_test_05(void);
u32 nv_read_test_06(void);
u32 nv_read_test_07(void);
u32 nv_write_test01(void);
u32 nv_write_test02(void);
u32 nv_write_test03(void);
u32 nv_write_test04(void);
u32 nv_write_test05(void);
u32 nv_backup_test_00(void);
u32 nv_revert_test(void);
u32 nv_revert_manuf_version_test(void);
u32 nv_test_destory_img_data(s8* path, s8 * flagPath, u32 off);
u32 nv_test_destory_bak_data(s8* path, s8 * flagPath, u32 off);
u32 nv_test_check_ddr_crc_once(void);
u32 nv_test_make_check_ddr_crc(void);
u32 nv_test_resume_ddr_from_img_00(void);
u32 nv_test_resume_ddr_from_img_01(void);
u32 nv_test_resume_ddr_from_bak_00(void);
u32 nv_test_resume_ddr_from_bak_01(void);
u32 nv_crc_write_test00(void);
u32 nv_crc_write_test01(void);
u32 nv_crc_write_test02(void);
u32 nv_crc_write_test03(void);
u32 nv_test_find_edge_nv(struct nv_ref_data_info_stru nvArray[10]);
u32 nv_crc_write_test04(void);
u32 nv_crc_write_test05(void);
u32 nv_reload_data_from_bak_test(void);
u32 nv_flush_test_00(void);
u32 nv_imei_check_test(void);

u32 nv_kernel_init_test(void)
{
    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    if(ddr_info->acore_init_state == NV_INIT_OK)
    {
        return NV_OK;
    }

    ret = (u32)bsp_nvm_kernel_init();

    return ret;

}

u32 nv_remain_init_test(void)
{
    u32 ret = NV_ERROR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;

    if(ddr_info->acore_init_state == NV_INIT_OK)
    {
        return NV_OK;
    }

    ret = (u32)bsp_nvm_remain_init();

    return ret;

}


u32 nv_read_part_test(u32 nvid,u32 off,u32 len)
{
    u32 ret;
    u8* tempdata;
    u32 i= 0;

    tempdata = (u8*)nv_malloc(len +1);
    if(NULL == tempdata)
    {
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    ret = bsp_nvm_readpart(nvid,off,tempdata,len);
    if(NV_OK != ret)
    {
        nv_free(tempdata);
        return BSP_ERR_NV_READ_DATA_FAIL;
    }

    for(i=0;i<len;i++)
    {
        if((i%16) == 0)
        {
            printf("\n");
        }
        printf("%2.2x ",(u8)(*(tempdata+i)));
    }

    printf("\n");
    nv_free(tempdata);
    return NV_OK;
}

u32 nv_get_len_test01(void)
{
    u32 ret;
    u32 len = 0;
    u32 itemid = 0xd007;

    ret = bsp_nvm_get_len(itemid,&len);
    return ret;
}

u32 nv_get_len_test02(void)
{
    u32 ret= NV_ERROR;
    u32 len = 0;
    u32 itemid = 0xffff;

    ret = bsp_nvm_get_len(itemid,&len);
    if(ret == BSP_ERR_NV_NO_THIS_ID)
    {
        return NV_OK;
    }
    return ret;
}

u32 nv_read_rand_test(u32 nvid)
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
    if(ref_info.nv_len == 0)
    {
        return NV_ERROR;
    }

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
        if((i%16) == 0)
        {
            printf("\n");
        }
        printf("%2.2x ",(u8)(*(tempdata+i)));
    }

    printf("\n");
    nv_free(tempdata);
    return NV_OK;
}

u32 nv_read_test_01(void)
{
    u32 itemid = 0xffff;
    u32 ret;

    ret = nv_read_rand_test(itemid);

    if(ret == BSP_ERR_NV_NO_THIS_ID)
    {
        return NV_OK;
    }

    return ret;
}

u32 nv_read_test_02(void)
{
    u32 itemid = 0xd007;
    u32 ret = NV_ERROR;

    u32 datalen  = 5;
    u32 off = 0;

    ret = nv_read_part_test(itemid,off,datalen);
    if(ret == BSP_ERR_NV_READ_DATA_FAIL)
    {
        return NV_OK;
    }

    return NV_ERROR;
}

u32 nv_read_test_03(void)
{
    u32 itemid = 0xd007;
    u32 ret = NV_ERROR;

    u32 datalen  = 1;
    u32 off = 4;

    ret = nv_read_part_test(itemid,off,datalen);
    if(ret == BSP_ERR_NV_READ_DATA_FAIL)
    {
        return NV_OK;
    }

    return NV_ERROR;
}

u32 nv_read_test_04(void)
{
    u32 itemid = 0xd007;
    u32 ret = NV_ERROR;

    u32 datalen  = 4;
    u32 off = 0;

    ret = nv_read_part_test(itemid,off,datalen);

    return ret;
}

u32 nv_read_test_05(void)
{
    u32 itemid = 0xd007;
    u32 ret = NV_ERROR;

    ret = nv_read_rand_test(itemid);

    return ret;
}

u32 nv_read_test_06(void)
{
    u32 itemid = 0xd007;
    u32 ret = NV_ERROR;
    u8* pdata = NULL;

    ret = bsp_nvm_read(itemid,pdata,sizeof(u32));
    if(ret == BSP_ERR_NV_INVALID_PARAM)
    {
        return NV_OK;
    }
    return ret;
}

u32 nv_read_test_07(void)
{
    u32 itemid = 0xd007;
    u32 ret = NV_ERROR;
    u32 pdata = 0;

    ret = bsp_nvm_read(itemid,(u8*)&pdata,1);
    if(ret == BSP_ERR_NV_ITEM_LEN_ERR)
    {
        return NV_OK;
    }
    return ret;
}

u32 nv_write_test01(void)
{
    u32 ret = NV_OK;
    u32 i = 0;
    u32 read_data = 0;
    u32 write_data = 0;

    for(i = 0;i<100;i++)
    {
         printf("\n****************第%d次读写开始**************************\n",i);
         write_data = i+60;
         ret = bsp_nvm_write(0xd007,(u8*)&write_data,sizeof(u32));
         if(NV_OK != ret)
         {
             printf("****write error*****i = %d\n",i);
             return ret;
         }
         ret = bsp_nvm_read(0xd007,(u8*)&read_data,sizeof(u32));
         if(ret)
         {
             printf("****read error*****i = %d\n",i);
             return ret;
         }
         if(read_data != write_data)
         {
            return NV_ERROR;
         }
    }
    return NV_OK;
}

u32 nv_write_test02(void)
{
    u32 ret = NV_OK;
    u32 i = 0;
    u32 read_data = 0;
    u32 write_data = 0;

    for(i = 0;i<100;i++)
    {
         printf("\n****************第%d次读写开始**************************\n",i);
         write_data = i+60;
         ret = bsp_nvm_write(0xd007,(u8*)&write_data,sizeof(u32));
         if(NV_OK != ret)
         {
             printf("****write error*****i = %d\n",i);
             return ret;
         }
         ret = bsp_nvm_read(0xd007,(u8*)&read_data,sizeof(u32));
         if(ret)
         {
             printf("****read error*****i = %d\n",i);
             return ret;
         }
    }
    return NV_OK;
}

u32 nv_write_test03(void)
{
    u32 ret = NV_ERROR;
    u32 itemid = 0xd007;

    u8* pdata = NULL;

    ret = bsp_nvm_write(itemid,pdata,sizeof(u32));
    if(ret == BSP_ERR_NV_INVALID_PARAM)
    {
        return NV_OK;
    }
    return ret;
}

u32 nv_write_test04(void)
{
    u32 ret = NV_ERROR;
    u32 itemid = 0xffff;

    u32 datalen  = 0;

    ret = bsp_nvm_write(itemid,(u8*)&datalen,sizeof(u32));
    if(ret == BSP_ERR_NV_NO_THIS_ID)
    {
        return NV_OK;
    }
    return ret;
}

u32 nv_write_test05(void)
{
    u32 ret = NV_ERROR;
    u32 itemid = 0xd007;
    u32 data = 56;

    ret = bsp_nvm_write(itemid,(u8*)&data,sizeof(u16));
    if(ret == BSP_ERR_NV_ITEM_LEN_ERR)
    {
        return NV_OK;
    }
    return ret;
}

u32 nv_write_test06(void);
u32 nv_write_test06(void)
{
    u32 ret = NV_ERROR;
    u32 itemid = 0xd007;
    u32 data = 56;

    ret = bsp_nvm_write(itemid,(u8*)&data,sizeof(u32));

    return ret;

}
u32 nv_write_test07(u32 count, u32 data);
u32 nv_write_test07(u32 count, u32 data)
{
    u32 ret = NV_ERROR;
    u32 itemid = 0xD007;
    u32 i = 0;
    for(i = 0;i < count; i++)
    {
        ret = bsp_nvm_write(itemid,(u8*)&data,sizeof(u32));
    }
    return ret;
}

u32 nv_write_test_08(u32 itemid)
{
    struct nv_file_list_info_stru file_info = {};
    struct nv_ref_data_info_stru ref_info = {};
    u8* pData = NULL;
    u32 ret = 0;

    pData = (u8*)nv_malloc(2*2048);
    if(NULL == pData)
    {
        nv_printf("alloc error\n");
        return NV_ERROR;
    }
    ret = nv_search_byid(itemid, (u8 *)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
    if(ret)
    {
        nv_printf("nv_search_byid error\n");
        nv_free(pData);
        return ret;
    }
    ret = bsp_nvm_read(itemid, pData, ref_info.nv_len);
    if(ret)
    {
        nv_printf("bsp_nvm_read error, ret = 0x%x 1111\n", ret);
        return ret;
    }
    pData[0]++;
    ret = bsp_nvm_write(itemid, pData, ref_info.nv_len);
    if(ret)
    {
        nv_printf("bsp_nvm_read error, ret = 0x%x 2222\n", ret);
        return ret;
    }
    nv_free(pData);
    return NV_OK;
}

u32 nv_backup_test_00(void)
{
    u32 ret = NV_ERROR;

    ret = bsp_nvm_backup();

    return ret;
}

u32 nv_revert_test(void)
{
    u32 ret = NV_ERROR;

    ret = bsp_nvm_backup();

    if(ret)
    {
        printf("backup failed!\n");
        return ret;
    }

    ret = bsp_nvm_revert();
    return ret;
}

u32 nv_revert_manuf_version_test(void)
{
    u32 ret = NV_ERROR;

    ret = bsp_nvm_update_default();
    if(ret)
    {
        printf("update default failed !\n");
        return ret;
    }
    ret = bsp_nvm_revert_default();

    return ret;
}
u32 nv_read_write_test(void);
u32 nv_read_write_test(void)
{
    u32 i;
    u32 ret;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);
    u8* pdata;

    pdata = (u8*)nv_malloc(3072);
    if(NULL == pdata)
    {
        return NV_ERROR;
    }
    for(i = 0;i<ctrl_info->ref_count;i++)
    {
        printf("*****************read & write 0x%x****************\n",ref_info->itemid);
        ret = bsp_nvm_read(ref_info->itemid,pdata,ref_info->nv_len);
        if(ret)
        {
            nv_free(pdata);
            return ret;
        }
        ret = bsp_nvm_write(ref_info->itemid,pdata,ref_info->nv_len);
        if(ret)
        {
            nv_free(pdata);
            return ret;
        }
        ref_info++;
    }
    nv_free(pdata);
    return NV_OK;
}
u32 nv_function_test(void);
u32 nv_function_test(void)
{
    u32 i;
    u32 ret;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);
    u8* pdata;
    u32 start,end;


    start = bsp_get_slice_value();

    pdata = (u8*)nv_malloc(3072);
    if(NULL == pdata)
    {
        return NV_ERROR;
    }
    for(i = 0;i<100;i++)
    {
        printf("*****************read & write 0x%x****************\n",ref_info->itemid);
        ret = bsp_nvm_read(ref_info->itemid,pdata,ref_info->nv_len);
        if(ret)
        {
            nv_free(pdata);
            return ret;
        }
        ret = bsp_nvm_write(ref_info->itemid,pdata,ref_info->nv_len);
        if(ret)
        {
            nv_free(pdata);
            return ret;
        }
        ref_info++;
    }
    end = bsp_get_slice_value();
    nv_free(pdata);

    printf("[%s]:slice 0x%x\n",__func__,end-start);
    return NV_OK;
}

u32 nv_imei_check_test(void)
{
    u32 ret = 0;

    ret = nv_imei_data_comp((s8*)NV_IMG_PATH);
    if(ret)
    {
        printf("1111 ret = 0x%x\n", ret);
    }

    ret = nv_imei_data_comp((s8*)NV_BACK_PATH);
    if(ret)
    {
        printf("1111 ret = 0x%x\n", ret);
    }
    return NV_OK;
}

/*CRC test start*/
/*测试生成的CRC校验码是否正确*/

u32 nv_test_check_ddr_crc_once(void)
{
    u32 ret = 0;
    u32 startSlice = 0;
    u32 endSlice = 0;

    startSlice = bsp_get_slice_value();
    ret = nv_check_ddr_crc();
    endSlice = bsp_get_slice_value();
    if(ret)
    {
        printf("[%s]: ret = 0x%x\n",__func__,ret);
        return ret;
    }
    printf("[%s]: Slice:0x%x\n",__func__,endSlice-startSlice);
    return NV_OK;
}
/*反复生成并测试nv 的CRC校验码*/

u32 nv_test_make_check_ddr_crc(void)
{
    u32 ret = 0;
    u32 startSlice = 0;
    u32 endSlice = 0;
    u32 i = 0;

    printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    startSlice = bsp_get_slice_value();
    for(i = 0; i < 100; i++)
    {
        printf("\n****************第%d次生成CRC开始**************************\n", i);
        ret = nv_make_ddr_crc();
        if(ret)
        {
            printf("[%s]: ret = 0x%x\n",__func__,ret);
            return ret;
        }
        ret = nv_check_ddr_crc();
        if(ret)
        {
            printf("[%s]: ret = 0x%x\n",__func__,ret);
            return ret;
        }
    }
    endSlice = bsp_get_slice_value();
    printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    printf("[%s]: Slice:0x%x\n",__func__,endSlice-startSlice);
    return NV_OK;
}
/*破坏指定文件的指定位置的数据 
**off:相对于ctrl文件头的偏移
*/
u32 nv_test_destory_img_data(s8* path, s8 * flagPath, u32 off)
{
    FILE *fp = NULL;
    u32 ret = 0;
    u32 fpdata = 0;

    fp = nv_file_open((s8*)path,(s8*)NV_FILE_RW);
    if(!fp)
    {
        nv_file_close(fp);
        printf("[%s]: open file fail\n",__func__);
        return BSP_ERR_NV_NO_FILE;
    }
    ret = nv_file_seek(fp,(s32)off,SEEK_SET);/*jump to write*/
    if(ret)
    {
        nv_file_close(fp);
        printf("[%s]: seek file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    ret = nv_file_read((u8 *)&fpdata, 1, sizeof(u32), fp);
    if(ret != sizeof(u32))
    {
        nv_file_close(fp);
        printf("[%s]: read file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }
    fpdata+=2;
    ret = (u32)nv_file_write((u8*)&fpdata, 1, sizeof(u32), fp);
    if(ret != sizeof(u32))
    {
        nv_file_close(fp);
        printf("[%s]: write file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }
    nv_file_close(fp);
    return NV_OK;
}

/*破坏备份区的NV数据*/
u32 nv_test_destory_bak_data(s8* path, s8 * flagPath, u32 off)
{
    FILE *fp = NULL;
    u32 ret = 0;
    u32 fpdata = 0;

    fp = nv_file_open((s8*)path,(s8*)NV_FILE_READ);
    if(!fp)
    {
        nv_file_close(fp);
        printf("[%s]: open file fail\n",__func__);
        return BSP_ERR_NV_NO_FILE;
    }
    ret = nv_file_seek(fp,(s32)off,SEEK_SET);/*jump to write*/
    if(ret)
    {
        nv_file_close(fp);
        printf("[%s]: seek file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }

    ret = nv_file_read((u8*)&fpdata, 1, sizeof(u32), fp);
    if(ret != sizeof(u32))
    {
        nv_file_close(fp);
        printf("[%s]: read file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }
    nv_file_close(fp);
    fp = nv_file_open((s8*)path,(s8*)NV_FILE_WRITE);
    if(!fp)
    {
        nv_file_close(fp);
        printf("[%s]: open file fail\n",__func__);
        return BSP_ERR_NV_NO_FILE;
    }
    ret = nv_file_seek(fp,(s32)off,SEEK_SET);/*jump to write*/
    if(ret)
    {
        nv_file_close(fp);
        printf("[%s]: seek file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_READ_FILE_FAIL;
    }
    fpdata+=2;
    ret = (u32)nv_file_write((u8*)&fpdata, 1, sizeof(u32), fp);
    if(ret != sizeof(u32))
    {
        nv_file_close(fp);
        printf("[%s]: write file fail, ret = 0x%x\n",__func__, ret);
        return BSP_ERR_NV_WRITE_FILE_FAIL;
    }
    nv_file_close(fp);
    return NV_OK;
}

/*测试从工作分区恢复NV nv_resume_ddr_from_img*/
u32 nv_test_resume_ddr_from_img_00(void)
{
    u32 ret = 0;
    ret = nv_resume_ddr_from_img();
    if(ret)
    {
        printf("[%s]: ret = 0x%x\n",__func__,ret);
        return ret;
    }
    return NV_OK;
}

/*从工作分区加载NV*/
u32 nv_test_resume_ddr_from_img_01(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = 0;
    u32 data = 0;
    void* off = 0;
    if(!NV_CRC_CHECK_YES)
    {
        printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    }
    off = (void *)(NV_GLOBAL_CTRL_INFO_ADDR + ctrl_info->ctrl_size + 0x30);

    data = *(u32 *)off;
    data+=2;
    *(u32 *)off = data;

    /*NV数据 CRC校验*/
    ret = nv_check_ddr_crc();
    if(!ret)
    {
        *(u32 *)off = data - 2;
        printf("[%s]: nv_check_ddr_crc ret = 0x%x\n",__func__,ret);
        return ret;
    }

    /*工作区恢复NV数据*/
    ret = nv_resume_ddr_from_img();
    if(ret)
    {
        printf("[%s]: ret = 0x%x\n",__func__,ret);
        return ret;
    }
    if(!NV_CRC_CHECK_YES)
    {
        printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    }
     /*恢复后的NV数据CRC校验*/
    ret = nv_check_ddr_crc();
    if(ret)
    {
        printf("[%s]: nv_check_ddr_crc ret = 0x%x\n",__func__,ret);
        return ret;
    }
    return NV_OK;
}

/**从备份分区加载NV
**注:有两次CRC校验错误的打印,一次是主动校验,一次是从工作分区加载数据时出现CRC校验错误
*/
u32 nv_test_resume_ddr_from_bak_00(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = 0;
    u32 bak_data = 0;
    void * off = 0;

    if(!NV_CRC_CHECK_YES)
    {
        printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    }

    off = (void *)(NV_GLOBAL_CTRL_INFO_ADDR + ctrl_info->ctrl_size + 0x30);
    /*破坏工作分区NV数据*/
    ret = nv_test_destory_img_data((s8 *)NV_IMG_PATH, (s8 *)(NV_IMG_FLAG_PATH), ctrl_info->ctrl_size);
    if(ret)
    {
        printf("[%s]: nv_test_destory_img_data ret = 0x%x\n",__func__,ret);
        return ret;
    }
    bak_data = *(u32 *)off;
    bak_data+=2;
    *(u32 *)off = bak_data;

    /*NV数据 CRC校验*/
    ret = nv_check_ddr_crc();
    if(!ret)
    {
        *(u32 *)off = bak_data -2;
        printf("[%s]: nv_check_ddr_crc ret = 0x%x\n",__func__,ret);
        return NV_ERROR;
    }
     /*恢复NV数据*/
    ret = nv_resume_ddr_from_img();
    if(ret)
    {
        *(u32 *)off = bak_data -2;
        printf("[%s]: ret = 0x%x\n",__func__,ret);
        return ret;
    }
    if(!NV_CRC_CHECK_YES)
    {
        printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    }
    /*恢复后的NV数据CRC校验*/
    ret = nv_check_ddr_crc();
    if(ret)
    {
        printf("[%s]: nv_check_ddr_crc ret = 0x%x\n",__func__,ret);
        return ret;
    }
    return NV_OK;
}
/*
** 从备份分区加载NV, 破坏CRC校验码
**注:有两次CRC校验错误的打印,一次是主动校验,一次是从工作分区加载数据时出现CRC校验错误
*/
u32 nv_test_resume_ddr_from_bak_01(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = 0;
    u32 bak_data = 0;
    void * off = 0;
    if(!NV_CRC_CHECK_YES)
    {
        printf("CRC mark = 0x%x\n",NV_CRC_CHECK_YES);
    }
    /*破坏工作区NV数据,破坏CRC校验码*/
    ret = nv_test_destory_img_data((s8 *)NV_IMG_PATH, (s8 *)(NV_IMG_FLAG_PATH), ddr_info->file_len + 10 *sizeof(u32));
    if(ret)
    {
        printf("[%s]: nv_test_destory_img_data ret = 0x%x\n",__func__,ret);
        return ret;
    }
    off = (void *)(NV_GLOBAL_CTRL_INFO_ADDR + ctrl_info->ctrl_size + 0x30);
    bak_data = *(u32 *)off;
    bak_data+=2;
    *(u32 *)off = bak_data;

    /*NV数据 CRC校验*/
    ret = nv_check_ddr_crc();
    if(!ret)
    {
        *(u32 *)off = bak_data - 2;
        printf("[%s]: nv_check_ddr_crc ret = 0x%x\n",__func__,ret);
        return NV_ERROR;
    }
    /*恢复NV数据*/
    ret = nv_resume_ddr_from_img();
    if(ret)
    {
        *(u32 *)off = bak_data - 2;
        printf("[%s]: ret = 0x%x\n",__func__,ret);
        return ret;
    }
    if(!NV_CRC_CHECK_YES)
    {
        printf("*****************CRC mark = 0x%x******************\n",NV_CRC_CHECK_YES);
    }
    /*恢复后的NV数据CRC校验*/
    ret = nv_check_ddr_crc();
    if(ret)
    {
        printf("[%s]: nv_check_ddr_crc ret = 0x%x\n",__func__,ret);
        return ret;
    }
    return NV_OK;
}
/*
**测试CRC校验码在写入前后是否一致
**同时测试写入的数据是否正确的写入
*/
u32 nv_crc_write_test00(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ref_data_info_stru ref_info = {};
    struct nv_file_list_info_stru file_info = {};
    u32 ret = 0;
    u32 nvid = 0xD007;
    u32 old_data = 0;    
    u32 new_data = 0;    
    u32 skip_crc_count = 0;
    u32 off = 0;
    u32 old_crc = 0;
    u32 new_crc = 0;
    u32 *pCrcCode = 0;

    if(!NV_CRC_CHECK_YES)
    {
        printf("CRC mark = 0x%x\n",NV_CRC_CHECK_YES);
        return NV_ERROR;
    }
    ret = nv_search_byid(nvid, (u8 *)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
    if(ret)
    {
        printf("11111 ret = 0x%x\n", ret);
        return ret;
    }
    off = ref_info.nv_off + ddr_info->file_info[ref_info.file_id -1].offset;
    skip_crc_count = (off - ctrl_info->ctrl_size)/NV_CRC32_CHECK_SIZE;
    pCrcCode = NV_DDR_CRC_CODE_OFFSET;
    old_crc = pCrcCode[skip_crc_count];
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);
    ret = bsp_nvm_read(nvid, (u8 *)&old_data, sizeof(u32));
    if(ret )
    {
        printf("2222 ret = 0x%x\n", ret);
        return ret;
    }
    old_data += 3;

    ret = bsp_nvm_write(nvid, (u8*)&old_data, sizeof(u32));
    if(ret)
    {
        printf("3333 ret = 0x%x\n", ret);
        return ret;
    }

    new_crc = *(u32 *)(NV_DDR_CRC_CODE_OFFSET + skip_crc_count);

    ret = bsp_nvm_read(nvid, (u8 *)&new_data, sizeof(u32));
    if(ret)
    {
        printf("5555 ret = 0x%x\n", ret);
        return ret;
    }

    if((old_crc == new_crc)||(new_data != old_data))
    {
        printf("4444 ret = 0x%x\n", ret);
        return NV_ERROR;
    }
    return NV_OK;
}
/*
**破坏0xD007的数据，后写入NV 0xD007
**注:串口有一次CRC校验不过的打印
**   串口有从工作分区中加载数据的log打印则认为通过 
*/
u32 nv_crc_write_test01(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ref_data_info_stru ref_info = {};
    struct nv_file_list_info_stru file_info = {};
    u32 ret = 0;
    u32 nvid = 0xD007;
    u32 data = 0;
    u32 new_data = 0;

    if(!NV_CRC_CHECK_YES)
    {
        printf("CRC mark = 0x%x\n",NV_CRC_CHECK_YES);
    }
    ret = nv_search_byid(nvid, (u8 *)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
    if(ret)
    {
        printf("11111 ret = 0x%x\n", ret);
        return ret;
    }
    data = *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off);
    data += 2;
    *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off) = data;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    ret = bsp_nvm_read(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("5555 ret = 0x%x\n", ret);
        return ret;
    }
    data+=2;
    ret = bsp_nvm_write(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("3333 ret = 0x%x\n", ret);
        return ret;
    }
    ret = bsp_nvm_read(nvid, (u8 *)&new_data, sizeof(u32));
    if(ret)
    {
        printf("6666 ret = 0x%x\n", ret);
        return ret;
    }
    if(new_data != data)
    {
        printf("7777 new_data = 0x%x, data = 0x%x ret = 0x%x\n", new_data, data,ret);
        return ret;
    }
    return NV_OK;
}
/*
**破坏DDR中的NV 0xD007 数据，破坏工作分区中的数据,后写入NV 0xD007
**注:串口有两次CRC校验不过的打印
**   串口有从工作分区中加载数据的log打印则认为通过 
*/
u32 nv_crc_write_test02(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ref_data_info_stru ref_info = {};
    struct nv_file_list_info_stru file_info = {};
    u32 ret = 0;
    u32 nvid = 0xD007;
    u32 data = 0;
    u32 new_data = 0;

    if(!NV_CRC_CHECK_YES)
    {
        printf("CRC mark = 0x%x\n",NV_CRC_CHECK_YES);
    }
    ret = nv_search_byid(nvid, (u8 *)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
    if(ret)
    {
        printf("11111 ret = 0x%x\n", ret);
        return ret;
    }

    ret = bsp_nvm_read(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("2222 ret = 0x%x\n", ret);
        return ret;
    }
    data+=2;
    *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off) = data;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

    ret = nv_test_destory_img_data((s8 *)NV_IMG_PATH, (s8 *)(NV_IMG_FLAG_PATH), ctrl_info->ctrl_size);
    if(ret)
    {
        printf("[%s]: 3333 ret = 0x%x\n",__func__,ret);
        return ret;
    }
    /*NV数据 CRC校验*/
    ret = nv_check_ddr_crc();
    if(!ret)
    {
        printf("[%s]: 4444 ret = 0x%x\n",__func__,ret);
        return NV_ERROR;
    }

    ret = bsp_nvm_write(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("3333 ret = 0x%x\n", ret);
        return ret;
    }
    ret = bsp_nvm_read(nvid, (u8 *)&new_data, sizeof(u32));
    if(ret)
    {
        printf("6666 ret = 0x%x\n", ret);
        return ret;
    }
    if(new_data != data)
    {
        printf("7777 new_data = 0x%x, data = 0x%x ret = 0x%x\n", new_data, data,ret);
        return ret;
    }
    return NV_OK;
}
/*
**破坏DDR中的除NV 0xD007 外数据，后写入NV 0xD007
**注:写入正常
** 
*/
u32 nv_crc_write_test03(void)
{
    struct nv_global_ddr_info_stru* ddr_info = (struct nv_global_ddr_info_stru*)NV_GLOBAL_INFO_ADDR;
    struct nv_ref_data_info_stru ref_info = {};
    struct nv_file_list_info_stru file_info = {};
    u32 ret = 0;
    u32 nvid = 0xD007;
    u32 data = 0;
    u32 new_data = 0;
    u32 bak_data = 0;
    if(!NV_CRC_CHECK_YES)
    {
        printf("CRC mark = 0x%x\n",NV_CRC_CHECK_YES);
    }
    ret = nv_search_byid(nvid, (u8 *)NV_GLOBAL_CTRL_INFO_ADDR, &ref_info, &file_info);
    if(ret)
    {
        printf("11111 ret = 0x%x\n", ret);
        return ret;
    }

    bak_data = *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off + 4*sizeof(u32));
    bak_data+=2;
    *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off) = bak_data;
    nv_flush_cache((void*)NV_GLOBAL_INFO_ADDR, (u32)NV_GLOBAL_INFO_SIZE);

     /*NV数据 CRC校验*/
    ret = nv_check_ddr_crc();
    if(!ret)
    {
        *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off) = bak_data -2;
        printf("[%s]: 2222 ret = 0x%x\n",__func__,ret);
        return NV_ERROR;
    }
    ret = bsp_nvm_read(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        *(u32 *)(NV_GLOBAL_CTRL_INFO_ADDR + ddr_info->file_info[ref_info.file_id - 1].offset + ref_info.nv_off) = bak_data -2;
        printf("3333 ret = 0x%x\n", ret);
        return ret;
    }
    data+=2;
    ret = bsp_nvm_write(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("4444 ret = 0x%x\n", ret);
        return ret;
    }
    ret = bsp_nvm_read(nvid, (u8 *)&new_data, sizeof(u32));
    if(ret)
    {
        printf("5555 ret = 0x%x\n", ret);
        return ret;
    }
    if(new_data != data)
    {
        printf("7777 new_data = 0x%x, data = 0x%x ret = 0x%x\n", new_data, data,ret);
        return ret;
    }


    return NV_OK;
}
/*找到位于NV文件边缘的NV*/
u32 nv_test_find_edge_nv(struct nv_ref_data_info_stru nvArray[10])
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    struct nv_file_list_info_stru* file_info = (struct nv_file_list_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE);
    struct nv_ref_data_info_stru* ref_info   = (struct nv_ref_data_info_stru*)(NV_GLOBAL_CTRL_INFO_ADDR+NV_GLOBAL_CTRL_INFO_SIZE\
        +NV_GLOBAL_FILE_ELEMENT_SIZE*ctrl_info->file_num);
    struct nv_ref_data_info_stru* temp_ref_info = nvArray;
    u32 i = 0;
    u32 count = 0;

    /*check ref list id sort */
    for(i = 0;i<ctrl_info->ref_count-1;i++)
    {
        if((ref_info->nv_len + ref_info->nv_off)>= file_info[ref_info->file_id - 1].file_size)
        {
            memcpy(&(temp_ref_info[count]), ref_info, sizeof(struct nv_ref_data_info_stru));
            count++;
        }
        ref_info ++;
    }
    return count;
}
u32 nv_crc_write_test04(void)
{
    struct nv_ref_data_info_stru nvArray[10] = {};
    u32 count = 0;
    u32 i = 0;
    u32 ret = 0;
    u8 *pOldNvData = NULL;
    u8 *pNewNvData = NULL;

    pOldNvData = (u8 *)nv_malloc(NV_MAX_UNIT_SIZE);
    pNewNvData = (u8 *)nv_malloc(NV_MAX_UNIT_SIZE);
    if((pOldNvData == NULL)||(pNewNvData == NULL))
    {
        nv_printf("malloc error 1111\n");
        return NV_ERROR;
    }
    count = nv_test_find_edge_nv(nvArray);
    for(i = 0; i < count; i++)
    {
        ret = bsp_nvm_read(nvArray[i].itemid, (u8 *)pOldNvData, nvArray[i].nv_len);
        if(ret)
        {
            nv_printf("read error 222, nvid = 0x%x\n", nvArray[i].itemid);
            return ret;
        }
        pOldNvData[0]+=2;
        ret = bsp_nvm_write(nvArray[i].itemid, (u8 *)pOldNvData, nvArray[i].nv_len);
        if(ret)
        {
            nv_printf("read error 3333, nvid = 0x%x\n", nvArray[i].itemid);
            return ret;
        }
        ret = bsp_nvm_read(nvArray[i].itemid, (u8 *)pNewNvData, nvArray[i].nv_len);
        if((ret)||(pNewNvData[0] != pOldNvData[0]))
        {
            nv_printf("read error 4444, nvid = 0x%x\n", nvArray[i].itemid);
            return ret;
        }
        ret = nv_check_ddr_crc();
        if(ret)
        {
            nv_printf("read error 5555, nvid = 0x%x\n", nvArray[i].itemid);
            return ret;
        }
    }
    nv_free(pOldNvData);
    nv_free(pNewNvData);
    return NV_OK;
}
void DelayMs(u32 delay_ms, u32 flag)
{
    u32 oldtime = 0;
    u32 newtime = 0;

    oldtime = bsp_get_slice_value();
    newtime = bsp_get_slice_value();
    while((oldtime + delay_ms*32) >  newtime)
    {
        newtime = bsp_get_slice_value();
    }
}
u32 g_crc_delay_ctrl = 1;
/*循环写入一个NV，测试双核的互斥是否有效*/
u32 nv_crc_write_test05(void)
{
    u32 ret = 0;
    u32 nvid = 0xD007;
    u32 data = 0;    
    u32 i = 0;
    for(i = 0; i < 1000; i++)
    {
        printf("\n****************第%d次测试开始**************************\n", i);

        ret = bsp_nvm_read(nvid, (u8 *)&data, sizeof(u32));
        if(ret)
        {
            nv_printf("read fail ,ret = 0x%x\n", ret);
            return ret;
        }
        data++;
        printf("\n****************第%d次测试开始 11111**************************\n", i);
        DelayMs(g_crc_delay_ctrl, 0);
        ret = bsp_nvm_write(nvid, (u8 *)&data, sizeof(u32));   
        if(ret)
        {
            nv_printf("write fail ,ret = 0x%x\n", ret);
            return ret;
        }
    }
    printf("\n****************第%d次测试开始 2222**************************\n", i);
    data = 20;
    ret = bsp_nvm_write(nvid, (u8 *)&data, sizeof(u32));   
    if(ret)
    {
        nv_printf("write fail 22222,ret = 0x%x\n", ret);
        return ret;
    }
    return NV_OK;
}
/*
**破坏工作分区中的数据，然后进行NV加载,加载时NV应该是从备份分区中进行的, 
**从打印串口看本用例是否成功
*/
extern u32 bsp_nvm_reload(void);

u32 nv_reload_data_from_bak_test(void)
{
    struct nv_ctrl_file_info_stru* ctrl_info = (struct nv_ctrl_file_info_stru*)NV_GLOBAL_CTRL_INFO_ADDR;
    u32 ret = 0;

    ret = nv_test_destory_img_data((s8 *)NV_IMG_PATH, (s8 *)(NV_IMG_FLAG_PATH), ctrl_info->ctrl_size + 0x30);
    if(ret)
    {
        printf("0000 ret = 0x%x\n", ret);
        return ret;
    }
    ret = bsp_nvm_reload();
    if(ret)
    {
        printf("1111 ret = 0x%x\n", ret);
    }

    return NV_OK;;
}

/*
**测试关机写功能, 需要重新出版本将NV 0xD007的优先级设置为1~6
*/
u32 nv_flush_test_00(void)
{
    u32 ret = 0;
    u32 nvid = 0xD007;
    u32 data = 0;    
    u32 new_data = 0;

    ret = bsp_nvm_read(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("1111 ret = 0x%x\n", ret);
        return ret;
    }

    data+=2;
    ret = bsp_nvm_write(nvid, (u8 *)&data, sizeof(u32));
    if(ret)
    {
        printf("2222 ret = 0x%x\n", ret);
        return ret;
    }
   /*关机写接口*/
    ret = bsp_nvm_flush();
    if(ret)
    {
        printf("3333 ret = 0x%x\n",__func__,ret);
        return ret;
    }
    ret = nv_resume_ddr_from_img();
    if(ret)
    {
        printf("4444 ret = 0x%x\n", ret);
        return ret;
    }

    ret = bsp_nvm_read(nvid, (u8 *)&new_data, sizeof(u32));
    if(ret)
    {
        printf("5555 ret = 0x%x\n", ret);
        return ret;
    }
    if(new_data != data)
    {
        printf("6666 new_data = 0x%x, data = 0x%x\n", new_data, data);
    }
    return NV_OK;
}

osl_sem_id nv_test_sem;
u32 nv_sem_ctrl = 0;

u32 nv_test_sem_func(void)
{
    u32 start = 0;
    u32 end = 0;
    if(0 == nv_sem_ctrl)
    {
        osl_sem_init(0,&nv_test_sem);
        nv_sem_ctrl = 1;
    }

    start = bsp_get_slice_value();
    if(osl_sem_downtimeout(&nv_test_sem, 10))
    {
        end = bsp_get_slice_value();
    }
    nv_printf("slice: 0x%x\n", end - start);
    return 0;
}

/*CRC test end*/

/* added by yangzhi for muti-carrier, Begin:*/
/* added by yangzhi for muti-carrier, End! */

 

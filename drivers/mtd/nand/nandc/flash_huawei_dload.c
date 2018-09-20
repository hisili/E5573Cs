

/*---------------------------------------------------------------------------
  Include file define region
---------------------------------------------------------------------------*/
#include <linux/mtd/flash_huawei_dload.h>
#if (FEATURE_ON == MBB_DLOAD)
#include <linux/mtd/mtd.h>
#include <linux/err.h>
#include "ptable_com.h"
#include "ptable_inc.h"
#include "nandc_balong.h"
/*---------------------------------------------------------------------------
  Macro define region
---------------------------------------------------------------------------*/
#define    MBB_DLOAD_OEMINFO_PARTI_NAME     "oeminfo"
#define    HUAWEI_DLOAD_XML_MAGIC_NUM        0x454D5045
#define    HUAWEI_RESTORE_XML_MAGIC_NUM    0x45504D45

#define    TRUE     1
#define    FALSE   0
/*---------------------------------------------------------------------------
  Cust type define region
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
  Global Variable define region
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
  Local Function decleare region
---------------------------------------------------------------------------*/

oper_region_struct_t   operation_region[REGION_MAX];
uint8 iso_header_buffer[MAX_PAGE_SIZE];

/*---------------------------------------------------------------------------
  Global table define region
---------------------------------------------------------------------------*/
child_region_t  child_region[] =
{
    /* SHARE子分区参数 */
    {MBB_DLOAD_OEMINFO_PARTI_NAME, MAGIC_DEFAULT,MAGIC_DEFAULT, \
     0, 4, NULL, 0},  /*0：起始block  4 总block 数*/

};

struct mtd_info * g_mtd = NULL;
/*===========================================================================

                   FUNCTION DECLARATIONS

===========================================================================*/

/*===========================================================================
 
FUNCTION  flash_get_share_region_info
 
DESCRIPTION
    Get Correspoding field info
Author: ChenFeng 2010-3-20

RETURN VALUE
    TRUE if Op Succeed
    FALSE if Op Failure
 
SIDE EFFECTS
  None
=========================================================================== */

boolean flash_get_share_region_info(rgn_pos_e_type region_type,void* data,uint32 data_len)
{
    unsigned int     page_index       = 0;
    unsigned int     blk_size             = 0;
    unsigned int     blk_count          = 0;
    unsigned int     blk_index          = INVALID_BLOCK_ID;
    rgn_hd_type* rgn_hd;
    unsigned int     offset                 = 0;
    unsigned int     start_offset       = 0;
    unsigned int     page_size          = 0;
    uint8*     read_buf          = NULL;
    int           retlength            = 0;
    
    if (NULL == flash_nand_oper_region_init(SHARE_REGION))
    {
        printk(KERN_ERR "flash_get_share_region_info init fail  \n");
        return FALSE;
    }
    
    /*Get Partition and Device Info*/
    blk_count      = child_region[SHARE_REGION].length;
    blk_size        = operation_region[SHARE_REGION].block_size;
    start_offset  = operation_region[SHARE_REGION].start_addr;
    page_size    = operation_region[SHARE_REGION].page_size;
    read_buf      = operation_region[SHARE_REGION].buffer;
    
    for (blk_index = 0; blk_index < blk_count; blk_index++)
    {
        offset = start_offset + blk_size * blk_index;
        /*If the blk is bad blk or it is erased*/
        if (mtd_block_isbad(g_mtd, offset))
        {
            printk(KERN_DEBUG "flash_get_share_region_info Block ID = 0x%x is bad block \n",
                   (offset / blk_size));
            continue;
        }

        /*If current blk is not erased, then read the first page and check its magic*/
        /*lint -e64*/
        if (!mtd_read(g_mtd, offset, page_size, &retlength, (unsigned char*)read_buf))
        /*lint +e64*/
        {
           /*Firstly we need to check the Share Region Info,If it was a valid block
            then read corresponding page*/
            rgn_hd = (rgn_hd_type*)read_buf;
            if(SHARE_RGN_MAGIC == rgn_hd->magic)
            {
                /*Read Spec Region Flag Info*/
                page_index += region_type;
                offset = offset + page_index * page_size;

                memset((void *)read_buf, NAND_FILL_CHAR_APP, page_size);
                /*lint -e64*/
                if(!mtd_read(g_mtd, offset, page_size, &retlength,
                   (unsigned char*)read_buf))
                /*lint +e64*/
                {
                    memcpy(data,read_buf,data_len);
                    flash_nand_oper_region_close(SHARE_REGION);
                    return TRUE;
                }  
                printk(KERN_DEBUG "flash_get_share_region_info fail  \n");
            }
        }
    }
    printk(KERN_ERR "flash_get_share_region_info error  \n");

    flash_nand_oper_region_close(SHARE_REGION);
    return FALSE;
}

/*===========================================================================
 
FUNCTION  FLASH_UPDATE_SHARE_REGION_INFO
 
DESCRIPTION
    update user flag,nv_mbn,and iso header info in the share region.
    This function use page to store these info, 

    Page 0: Share region Info:
           Magic:
           Sub region len:
    Page 1: Nv restore flag:
           In Used Flag Magic: 4 Byte
           Auto Restore Magic: 4Byte
           Auto Restore Flag: 
    Page 2:
          In User Flag Magic:
          MBN Magic:
          MBN Info Num:
          MBN: Version:
          MBN nv data
    Page 3:
          ISO Header Info;
          
    We use two block to implement this feature, when the first time to update this feature directly write it to the 
first available block,and then we update this info then firstly read the page info to cache, update the cache value
and then write it to another block, erase lasted block.

Author: ChenFeng 2010-3-20

RETURN VALUE
    TRUE if Op Succeed
    FALSE if Op Failure
 
SIDE EFFECTS
  None
 
===========================================================================*/
#if (FEATURE_OFF == MBB_DLOAD_OEM_PART_IM)
boolean flash_update_share_region_info(rgn_pos_e_type region_type, void* data, unsigned int data_len)
{
    unsigned int    block_index        = 0;
    unsigned int    start_index         = 0;
    unsigned int    blk_size              = 0;
    unsigned int    blk_count            = 0;
    unsigned int    max_rgn_num    = 0;
    unsigned int    dest_block          = 0;
    unsigned int    current_block     = INVALID_BLOCK_ID;
    unsigned int    current_page      = INVALID_PAGE_ID;
    unsigned int    dest_page           = INVALID_PAGE_ID;
    rgn_hd_type* rgn_hd       = NULL;
    unsigned int   offset                    = 0;
    unsigned int   page_size             = 0;
    uint8 *   cache_buf            = NULL;
    unsigned int   start_offset           = 0;
    struct   erase_info instr;
    int retlength = 0;
    rgn_hd_type rgn_hd_1 = {0};

    /*入参条件检测*/
    if (region_type >= RGN_MAX_NUM || NULL == data)
    {
        printk(KERN_ERR "Input parm error \n");
        return FALSE;
    }

    if(NULL == flash_nand_oper_region_init(SHARE_REGION))
    {
        printk(KERN_ERR "flash_update_share_region_info init error \n");
        return FALSE;
    }

    /*Get Partition and Device Info*/
    blk_count    = child_region[SHARE_REGION].length;
    blk_size       = operation_region[SHARE_REGION].block_size;
    start_offset = operation_region[SHARE_REGION].start_addr;
    page_size   = operation_region[SHARE_REGION].page_size;
    cache_buf    = operation_region[SHARE_REGION].buffer;
    
    instr.mtd = g_mtd;
    instr.len = g_mtd->erasesize;
    instr.callback = NULL;
    instr.priv = 0;

    if (data_len > page_size)
    {
        printk(KERN_ERR "Warning: input data truncated.\n");
        data_len = page_size;
    }

    /*instr.addr*/
    for (block_index = 0; block_index < blk_count; block_index++)
    {
        offset = start_offset + blk_size * block_index;
        
        /*If the blk is bad blk or it is erase */
        if (mtd_block_isbad(g_mtd, offset))
        {
            printk(KERN_DEBUG "Find Current Block ID = 0x%x is bad block \n",(offset / blk_size));
            continue;
        }

        /*lint -e64*/
        if (!mtd_read(g_mtd, offset, page_size, &retlength, (unsigned char*)cache_buf))
        /*lint +e64*/
        {
            /*Firstly we need to check the Share Region Info,If it was a valid block
            then read corresponding page*/
            rgn_hd = (rgn_hd_type*)cache_buf;
            
            if (SHARE_RGN_MAGIC == rgn_hd->magic)
            {
                current_page = block_index * blk_size;
                max_rgn_num = rgn_hd->sub_rgn_num;
                current_block = block_index;
                printk(KERN_DEBUG "Print entery current page = 0x%x  max_rgn_num = 0x%x  \n",
                       current_page, max_rgn_num);
                break;
            }  
            else
            {
                instr.addr = offset;
                if (mtd_erase(g_mtd, &instr))
                {
                    printk(KERN_DEBUG "Current Erase error ,Mark bad block \n");
                    /* mark bad */
                    mtd_block_markbad(g_mtd, offset);
                }
                printk(KERN_DEBUG "Print nand_erase offset = 0x%x  \n", offset);
                continue;
            }
        }
        else
        {
            printk(KERN_ERR "Nand_read  error  !\n");
            return FALSE;
        }
    }

    printk(KERN_DEBUG "Print first current page = %u  \n", current_page);

    if(block_index != blk_count)
    {
        /*从 当前block 的 下一个block 分区开始寻找下一个 没使用的BLOCK*/
        dest_block = (block_index + 1) % blk_count;
    }

    /*Find the first unused block*/
    for (block_index  = 0; block_index  < blk_count; block_index++)
    {
        offset = start_offset + dest_block * blk_size;
        if (mtd_block_isbad(g_mtd, offset))
        {
                printk(KERN_DEBUG "Find Dest Block ID = 0x%x is bad block \n", (offset / blk_size));
                continue;
        }
        else
        {
            if (current_block != dest_block)
            {
                instr.addr = offset;
                if (mtd_erase(g_mtd, &instr))
                {
                    /* mark bad */
                    mtd_block_markbad(g_mtd, offset);
                    
                    dest_block = (dest_block + 1) % blk_count;
                    printk(KERN_DEBUG "Dest Erase error ,Mark bad block \n");
                    continue;
                }
                dest_page = dest_block * blk_size;
                break;
            }
        }
        dest_block = (dest_block + 1) % blk_count;
    }

    /*将当前页的数据和更新的数据分别取出写入目标BLOCK*/
    if (INVALID_PAGE_ID != dest_page)
    {   
        start_index = 0;
        /*第一次升级SHARE RGN分区  或者RGN_MAX_NUM有变化*/
        if ((unsigned int)region_type >= max_rgn_num)
        {
            offset = start_offset;
            memset(cache_buf,0xFF, page_size);  /*flash 操作buf 清为0XFF*/
            rgn_hd_1.magic = SHARE_RGN_MAGIC;
            rgn_hd_1.sub_rgn_num = RGN_MAX_NUM;
            memcpy(cache_buf,(void *)&rgn_hd_1,sizeof(rgn_hd_type));
            printk(KERN_DEBUG "Print First Dest page = %u  \n",dest_page);

            /*擦除当前BLOCK*/
            if ((INVALID_BLOCK_ID == current_block) || (0 < max_rgn_num))
            {
                offset = offset + dest_page;
            }
            else
            {
                offset = offset + current_page;
            }

            /*lint -e64*/
            if (mtd_write(g_mtd, offset, page_size, &retlength,
                (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write rgn_hd error  \n");
                goto FalseQuit;
            }

            memset(cache_buf, 0xFF, page_size); /*flash 操作buf 清为0XFF*/
            memcpy(cache_buf, data, data_len);
            /*lint -e64*/
            if (mtd_write(g_mtd, (offset  + region_type * page_size),
                page_size, &retlength, (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write region page error  \n");
                goto FalseQuit;
            }
            start_index = 1;

        }

        if (INVALID_PAGE_ID != current_page && dest_page != current_page)
        {  
            offset = start_offset;
            printk(KERN_DEBUG "Print not First Dest page = %u  \n",dest_page);
            printk(KERN_DEBUG "Print last current page = %u  \n",current_page);
            for (; start_index < max_rgn_num; start_index++)            
            {
                if (start_index != region_type)
                {
                    memset(cache_buf,0xFF, page_size); /*flash 操作buf 清为0XFF*/
                    /*lint -e64*/
                    if (!mtd_read(g_mtd,(offset + current_page + start_index * page_size),
                    page_size, &retlength,(unsigned char*)cache_buf))
                    /*lint +e64*/
                    {
                        if (0 == start_index)
                        {
                            rgn_hd_type *temp_hd = NULL;
                            temp_hd = (rgn_hd_type *)cache_buf;
                            printk(KERN_DEBUG "magic = 0x%x   max_num = 0x%x \n", (unsigned int)temp_hd->magic,
                                    (unsigned int)temp_hd->sub_rgn_num);  
                        }
                        /*lint -e64*/
                        if (mtd_write(g_mtd, (offset + dest_page 
                            + start_index * page_size),
                            page_size, &retlength, (unsigned char*)cache_buf))
                        /*lint -e64*/
                        {
                            printk(KERN_DEBUG "nand_write start_index = %u  offset = 0x%x\n",
                                   start_index, (offset + dest_page
                                   + region_type * page_size));
                            continue;
                        }
                    }
                }
                else
                {
                    memset(cache_buf, 0xFF, page_size); /*flash 操作buf 清为0XFF*/
                    memcpy(cache_buf, data, data_len);
                    /*lint -e64*/
                    if (mtd_write(g_mtd, (offset + dest_page + region_type * page_size),
                        page_size, &retlength, (unsigned char*)cache_buf))
                    /*lint -e64*/
                    {
                        printk(KERN_ERR "nand_write error \n");
                        goto FalseQuit;
                    }
                    printk(KERN_DEBUG "Write offset = 0x%x ! \n",
                           (offset + dest_page + region_type * page_size)); 
                }
            }

            instr.addr = offset + current_page;
            if (!mtd_erase(g_mtd, &instr))
            {
                printk(KERN_DEBUG "nand_erase offset = 0x%x ! \n",(offset + current_page)); 
            }
            else
            {
                printk(KERN_DEBUG "Last Erase current error ,Mark bad block \n");
                /* mark bad */
                mtd_block_markbad(g_mtd, offset);
                goto FalseQuit;
            } 
        }
        else if (dest_page == current_page)
        {
            printk(KERN_DEBUG "Error dest_page == current_page offset = 0x%x  \n", current_page);
            goto FalseQuit;
        }
        
        flash_nand_oper_region_close(SHARE_REGION);
        return TRUE;
    }
FalseQuit:
    printk(KERN_ERR "flash_update_share_region_info error \n");
    flash_nand_oper_region_close(SHARE_REGION);
    return FALSE;
}
#else
boolean flash_update_share_region_info(rgn_pos_e_type region_type, void* data, unsigned int data_len)
{
    unsigned int    block_index        = 0;
    unsigned int    start_index        = 0;
    unsigned int    blk_size           = 0;
    unsigned int    blk_count          = 0;
    unsigned int    max_rgn_num        = 0;
    unsigned int    dest_block         = 0;
    unsigned int    current_block      = INVALID_BLOCK_ID;
    unsigned int    current_page       = INVALID_PAGE_ID;
    unsigned int    dest_page          = INVALID_PAGE_ID;
    rgn_hd_type     *rgn_hd            = NULL;
    unsigned int    offset             = 0;
    unsigned int    page_size          = 0;
    uint8           *cache_buf         = NULL;
    unsigned int   start_offset        = 0;
    struct  erase_info  instr;
    int retlength = 0;
    rgn_hd_type rgn_hd_1 = {0};
    uint8 *flash_block_buf = NULL;
    /* 当前数据页所在块和目的页所在块查找指示变量*/
    boolean  find_curr_blk = FALSE;
    boolean  find_dest_blk = FALSE;

    /*入参条件检测*/
    if (region_type >= RGN_MAX_NUM || NULL == data)
    {
        printk(KERN_ERR "Input parm error \n");
        return FALSE;
    }

    if (NULL == flash_nand_oper_region_init(SHARE_REGION))
    {
        printk(KERN_ERR "flash_update_share_region_info init fail\n");
        return FALSE;
    }

    /*Get Partition and Device Info*/
    blk_count    = child_region[SHARE_REGION].length;
    blk_size     = operation_region[SHARE_REGION].block_size;
    start_offset = operation_region[SHARE_REGION].start_addr;
    page_size    = operation_region[SHARE_REGION].page_size;
    cache_buf    = operation_region[SHARE_REGION].buffer;

    instr.mtd = g_mtd;
    instr.len = g_mtd->erasesize;
    instr.callback = NULL;
    instr.priv = 0;

    if (data_len > page_size)
    {
        printk(KERN_ERR "Warning: input data truncated.\n");
        data_len = page_size;
    }

    if ((0 == blk_size) || (0 == blk_count))
    {
        printk(KERN_ERR "blk_size or blk_count is zero, exit.\n");
        goto FalseQuit;
    }
    /*instr.addr*/
    for (block_index = 0; block_index < blk_count; block_index++)
    {
        offset = start_offset + blk_size * block_index;

        /*If the blk is bad blk or it is erase */
        if (0 != mtd_block_isbad(g_mtd, offset))
        {
            printk(KERN_DEBUG "Find Current Block ID = 0x%x is bad block \n",(offset / blk_size));
            continue;
        }

        /*lint -e64*/
        if (0 == mtd_read(g_mtd, offset, page_size, &retlength, (unsigned char*)cache_buf))
        /*lint +e64*/
        {
            /*Firstly we need to check the Share Region Info,If it was a valid block
            then read corresponding page*/
            rgn_hd = (rgn_hd_type*)cache_buf;
            if (SHARE_RGN_MAGIC == rgn_hd->magic)
            {
                current_page = block_index * blk_size;
                max_rgn_num = rgn_hd->sub_rgn_num;
                current_block = block_index;
                /* 找到当前块 */
                find_curr_blk = TRUE;
                printk(KERN_DEBUG "Print entery current page = 0x%x  max_rgn_num = 0x%x  \n",
                       current_page, max_rgn_num);
                break;
            }
            else
            {
                instr.addr = offset;
                if (0 != mtd_erase(g_mtd, &instr))
                {
                    printk(KERN_DEBUG "Current Erase error ,Mark bad block \n");
                    /* mark bad */
                    (void)mtd_block_markbad(g_mtd, offset);
                }
                printk(KERN_DEBUG "Print nand_erase offset = 0x%x  \n", offset);
                continue;
            }
        }
        else
        {
            printk(KERN_ERR "Nand_read  error  !\n");
            goto FalseQuit;
        }
    }

    printk(KERN_DEBUG "Print first current page = %u  \n", current_page);

    if (block_index != blk_count)
    {
        /*从 当前block 的 下一个block 分区开始寻找下一个 没使用的BLOCK*/
        dest_block = (block_index + 1) % blk_count;
    }

    /*Find the first unused block*/
    for (block_index  = 0; block_index  < blk_count; block_index++)
    {
        offset = start_offset + dest_block * blk_size;

        /* 是坏块则跳过 */
        if (0 != mtd_block_isbad(g_mtd, offset))
        {
                printk(KERN_DEBUG "Find Dest Block ID = 0x%x is bad block \n", (offset / blk_size));
                dest_block = (dest_block + 1) % blk_count;
                continue;
        }
        else
        {
            /* 若不是当前数据块，则擦除 */
            if (current_block != dest_block)
            {
                instr.addr = offset;
                if (0 != mtd_erase(g_mtd, &instr))
                {
                    /* mark bad */
                    (void)mtd_block_markbad(g_mtd, offset);
                    dest_block = (dest_block + 1) % blk_count;
                    printk(KERN_DEBUG "Dest Erase error, Mark bad block \n");
                    continue;
                }
            }

            dest_page = dest_block * blk_size;
            /* 找到目的好块 */
            find_dest_blk = TRUE;
            break;
        }
    }

    /*将当前页的数据和更新的数据分别取出写入目标BLOCK*/
    if (TRUE == find_dest_blk)
    {
        /*第一次升级SHARE RGN分区  或者RGN_MAX_NUM有变化*/
        if (FALSE == find_curr_blk)
        {
            offset = start_offset;

            memset(cache_buf, NAND_FILL_CHAR_APP, page_size);  /*flash 操作buf 清为0XFF*/

            rgn_hd_1.magic = SHARE_RGN_MAGIC;
            rgn_hd_1.sub_rgn_num = RGN_MAX_NUM;
            memcpy(cache_buf, (void *)&rgn_hd_1, sizeof(rgn_hd_type));
            printk(KERN_DEBUG "Print First Dest page = %u  \n",dest_page);

            /*擦除当前BLOCK*/
            if ((INVALID_BLOCK_ID == current_block) || (0 < max_rgn_num))
            {
                offset = offset + dest_page;
            }
            else
            {
                offset = offset + current_page;
            }

            /*lint -e64*/
            if (0 != mtd_write(g_mtd, offset, page_size, &retlength,
                (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write rgn_hd error  \n");
                goto FalseQuit;
            }

            memset(cache_buf, NAND_FILL_CHAR_APP, page_size); /*flash 操作buf 清为0XFF*/
            memcpy(cache_buf, data, data_len);
            /*lint -e64*/
            if (0 != mtd_write(g_mtd, (offset  + region_type * page_size),
                page_size, &retlength, (unsigned char*)cache_buf))
            /*lint +e64*/
            {
                printk(KERN_ERR "nand_write region page error  \n");
                goto FalseQuit;
            }

        }
        else if ((TRUE == find_curr_blk) && (dest_page != current_page))
        {
            offset = start_offset;
            printk(KERN_DEBUG "Print not First Dest page = %u  \n", dest_page);
            printk(KERN_DEBUG "Print last current page = %u  \n", current_page);
            for (start_index = 0; start_index < max_rgn_num; start_index++)
            {
                if (start_index != region_type)
                {
                    memset(cache_buf, NAND_FILL_CHAR_APP, page_size); /*flash 操作buf 清为0XFF*/
                    /*lint -e64*/
                    if (0 == mtd_read(g_mtd, (offset + current_page + start_index * page_size),
                    page_size, &retlength, (unsigned char*)cache_buf))
                    /*lint +e64*/
                    {
                        if (0 == start_index)
                        {
                            rgn_hd_type *temp_hd = NULL;
                            temp_hd = (rgn_hd_type *)cache_buf;
                            printk(KERN_DEBUG "magic = 0x%x   max_num = 0x%x \n",
                                    (unsigned int)temp_hd->magic,
                                    (unsigned int)temp_hd->sub_rgn_num);
                        }
                        /*lint -e64*/
                        if (0 != mtd_write(g_mtd,
                                    (offset + dest_page + start_index * page_size),
                                    page_size,
                                    &retlength,
                                    (unsigned char*)cache_buf))
                        /*lint -e64*/
                        {
                            printk(KERN_DEBUG "nand_write start_index = %u  offset = 0x%x\n",
                                   start_index, (offset + dest_page
                                   + region_type * page_size));
                            continue;
                        }
                    }
                }
                else
                {
                    memset(cache_buf, NAND_FILL_CHAR_APP, page_size); /*flash 操作buf 清为0XFF*/
                    memcpy(cache_buf, data, data_len);
                    /*lint -e64*/
                    if (0 != mtd_write(g_mtd, (offset + dest_page + region_type * page_size),
                        page_size, &retlength, (unsigned char*)cache_buf))
                    /*lint -e64*/
                    {
                        printk(KERN_ERR "nand_write error \n");
                        goto FalseQuit;
                    }
                    printk(KERN_DEBUG "Write offset = 0x%x ! \n",
                           (offset + dest_page + region_type * page_size));
                }
            }

            /* 当前数据擦除 */
            instr.addr = offset + current_page;
            if (0 == mtd_erase(g_mtd, &instr))
            {
                printk(KERN_DEBUG "nand_erase offset = 0x%x ! \n",(offset + current_page));
            }
            else
            {
                printk(KERN_DEBUG "Last Erase current error ,Mark bad block \n");
                /* mark bad */
                (void)mtd_block_markbad(g_mtd, offset);
                goto FalseQuit;
            }
        }
        else if ((TRUE == find_curr_blk) && (dest_page == current_page))
        {
            /* 仅剩余一个好块 */
            offset = start_offset;

            /* 申请buffer */
            flash_block_buf = (uint8*)kmalloc(max_rgn_num * page_size, GFP_KERNEL);
            if (NULL == flash_block_buf)
            {
                printk(KERN_DEBUG, "malloc fail.");
                goto FalseQuit;
            }

            (void)memset(flash_block_buf, NAND_FILL_CHAR_APP, (max_rgn_num * page_size));

            for (start_index = 0; start_index < max_rgn_num; start_index++)
            {
                if(start_index == region_type)
                {
                    /* 待更新页不读取 */
                    continue;
                }

                /*lint -e64*/
                if (0 != mtd_read(g_mtd, (offset + current_page + start_index * page_size), page_size,
                            &retlength, (unsigned char*)(flash_block_buf + start_index * page_size)))
                /*lint +e64*/
                {
                    printk(KERN_DEBUG, "read error, start_index = %d offset = 0x%x.", start_index,
                            (offset + current_page + start_index * page_size));
                    (void)kfree(flash_block_buf);
                    flash_block_buf = NULL;
                    goto FalseQuit;
                }
            }

            /* 写入数据复制到对应位置 */
            (void)memcpy((void *)(flash_block_buf + region_type * page_size), (void *)data, page_size);

            instr.addr = offset + current_page;
            if (0 == mtd_erase(g_mtd, &instr))
            {
                printk(KERN_DEBUG, "Block ID = %d is erased.", (offset + current_page) / blk_size);
            }
            else
            {
                /* mark bad */
                (void)mtd_block_markbad(g_mtd, offset + current_page);
                (void)kfree(flash_block_buf);
                flash_block_buf = NULL;
                printk(KERN_DEBUG, "Block ID = %d is marked bad.", (offset + current_page) / blk_size);
                goto FalseQuit;
            }

            /* 按页写入 */
            for (start_index = 0; start_index < max_rgn_num; start_index++)
            {
                /*lint -e64*/
                if (0 != mtd_write(g_mtd, (offset + current_page + start_index * page_size), page_size,
                            &retlength, (unsigned char*)(flash_block_buf + start_index * page_size)))
                /*lint -e64*/
                {
                    printk(KERN_DEBUG, "write error, start_index = %d offset = 0x%x", start_index,
                            (offset + current_page + start_index * page_size));
                    (void)kfree(flash_block_buf);
                    flash_block_buf = NULL;
                    goto FalseQuit;
                }
            }

            (void)kfree(flash_block_buf);
            flash_block_buf = NULL;

        }
        else
        {
            goto FalseQuit;
        }

        flash_nand_oper_region_close(SHARE_REGION);
        return TRUE;
    }
FalseQuit:
    printk(KERN_ERR "flash_update_share_region_info error \n");
    flash_nand_oper_region_close(SHARE_REGION);
    return FALSE;
}
#endif


/*===========================================================================
 
FUNCTION  FLASH_NAND_OPER_REGION_INIT
 
DESCRIPTION
    Initilize user partition info,and get the handle of nand flash
RETURN VALUE
    TRUE if Op Succeed
    FALSE if Op Failure
 
SIDE EFFECTS
  None
 
===========================================================================*/
oper_region_struct_t *flash_nand_oper_region_init(  oper_region_type  oper_region_idx)
{
    uint8                idx              = (uint8)oper_region_idx;
    char                 *parent_name_ptr = NULL;
    unsigned int               offset           = 0;
    
    /* 范围保护 */
    if (idx >= REGION_MAX)
    {
        printk(KERN_ERR "Parm error: idx = %d max_ids = %d\n",idx,REGION_MAX);
        return NULL;
    }
 
    /*lint -e662*/
    parent_name_ptr = (char *)child_region[idx].parent_name;
    printk(KERN_DEBUG "parent_name_ptr =%s \n",parent_name_ptr);
    /*lint +e662*/
    /*lint -e661*/
    offset          = child_region[idx].offset;
    /*lint +e661*/

    /* get mtd device */
    g_mtd = get_mtd_device_nm(parent_name_ptr);
    if (NULL == g_mtd)
    {
        printk(KERN_ERR "get_mtd_device_nm error\n");
        return NULL;
    }

    printk(" info :mtd->erasesize = %d ,mtd->writesize = %d \n",
           g_mtd->erasesize,g_mtd->writesize);
    
    /* get the flash address */
    operation_region[idx].block_size = g_mtd->erasesize;
    operation_region[idx].page_size = g_mtd->writesize;
    operation_region[idx].start_addr = (offset * operation_region[idx].block_size);
    operation_region[idx].buffer = himalloc(operation_region[idx].page_size); 

    printk(KERN_DEBUG "operation_region[%d] = %u \n", idx, (unsigned int)operation_region[idx].start_addr);
    
    if (NULL == operation_region[idx].buffer)
    {
        put_mtd_device(g_mtd);
        return NULL;
    }

    /* 初始化读写flash时使用的缓冲区 */
    memset((void *)operation_region[idx].buffer, NAND_FILL_CHAR_APP,
           operation_region[idx].page_size);
    
    return &operation_region[idx];
}

/*===========================================================================
FUNCTION FLASH_NAND_OPER_REGION_CLOSE
 
DESCRIPTION
  close flash device and release flash handle
 
DEPENDENCIES
  None.
 
RETURN VALUE
  TRUE:  valid
  FALSE: invalid
 
SIDE EFFECTS
  None.
 
===========================================================================*/
void flash_nand_oper_region_close(oper_region_type  oper_region_idx)
{
   (void) kfree(operation_region[oper_region_idx].buffer);
    operation_region[oper_region_idx].buffer = NULL;
    put_mtd_device(g_mtd);
    return;
}

#if (FEATURE_ON == MBB_DLOAD_DOUBLE_PART)
/*===========================================================================
FUNCTION flash_set_recovery_flag
DESCRIPTION
  写备份镜像的标志到flash
DEPENDENCIES
  None.
RETURN VALUE
  TRUE:  valid
  FALSE: invalid
SIDE EFFECTS
  None.
===========================================================================*/
int flash_set_recovery_flag(unsigned int recovery_flag)
{
    char *part_name = "m3boot"; /*标志存放在m3boot分区*/
    struct mtd_info *mtd = NULL;
    struct erase_info instr = {0};
    unsigned char *write_buf = NULL;
    unsigned int flag_off = 0;  /*标志偏移量*/
    unsigned int retlen = 0;

    /*参数判断*/
    if((SMEM_RECA_FLAG_NUM != recovery_flag) && (SMEM_RECB_FLAG_NUM != recovery_flag))
    {
        printk(KERN_ERR "flash_set_recovery_flag: input para error, 0x%x.\n", recovery_flag);
        return -1;
    }

    /*获取mtd设备,m3boot分区*/
    mtd = get_mtd_device_nm(part_name);
    if (IS_ERR(mtd))
    {
        printk(KERN_ERR"flash_set_recovery_flag: get_mtd_device_nm error.\n");
        return -1;
    }

    /*按页写,申请写buffer为一个整页*/
    write_buf = (unsigned char *)himalloc(mtd->writesize);
    if(NULL == write_buf)
    {
        printk(KERN_ERR "flash_set_recovery_flag: malloc write buffer failed, len = 0x%x.\n", mtd->writesize);
        return -1;
    }
    memset((void *)write_buf, 0xFF, mtd->writesize);
    memcpy((void *)write_buf, &recovery_flag, sizeof(unsigned int));

    /*标志存放在分区第二个block的前4个字节*/
    flag_off = mtd->erasesize;
    /*检测是否为坏块*/
    if(mtd_block_isbad(mtd, flag_off))
    {
        printk(KERN_ERR "flash_set_recovery_flag: bad block 0x%x.\n", flag_off);
        goto FAIL;
    }
    /*整体擦除block*/
    instr.mtd = mtd;
    instr.addr = flag_off;
    instr.len = mtd->erasesize;
    instr.callback = NULL;
    instr.priv = 0;
    if(mtd_erase(mtd, &instr))
    {
        /*擦除失败不标记坏块*/
        printk(KERN_ERR "flash_set_recovery_flag: erase block failed.\n");
        goto FAIL;
    }
    /*写数据*/
    if (mtd_write(mtd, flag_off, mtd->writesize, &retlen, write_buf))
    {
        printk(KERN_ERR "flash_set_recovery_flag: write page failed.\n");
        goto FAIL;
    }
    /*读出校验,真正上库前可删除*/
    memset(write_buf, 0, mtd->writesize);
    if(mtd_read(mtd, flag_off, mtd->writesize, &retlen, (unsigned char*)write_buf))
    {
        printk(KERN_ERR "flash_set_recovery_flag: read page failed.\n");
        goto FAIL;
    }
    unsigned int read_flag = *((unsigned int *)write_buf);
    if(read_flag != recovery_flag)
    {
        printk(KERN_ERR "flash_set_recovery_flag: read flag 0x%x not equal with write flag 0x%x.\n", read_flag, recovery_flag);
        goto FAIL;
    }

    hifree(write_buf);
    /*释放mtd设备*/
    put_mtd_device(mtd);

    return 0;
FAIL:
    if(NULL != write_buf)
    {
        hifree(write_buf);
    }
    /*释放mtd设备*/
    put_mtd_device(mtd);

    return -1;
}

int test_recovery_flag(unsigned int flag)
{
    int ret = -1;
    unsigned recovery_flag = SMEM_RECA_FLAG_NUM;
    if(flag)
    {
        recovery_flag = SMEM_RECB_FLAG_NUM;
    }

    ret = flash_set_recovery_flag(recovery_flag);
    if(ret)
    {
        printk(KERN_ERR "TEST WRITE RECOVERY FLAG: write 0x%x failed.\n", recovery_flag);
        return -1;
    }
    printk(KERN_ERR "TEST WRITE RECOVERY FLAG: write 0x%x successed.\n", recovery_flag);

    return 0;
}
#endif

#endif


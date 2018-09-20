
/*lint --e{537} */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <hi_dsp.h>
#include <hi_onoff.h>
#include <ptable_com.h>

#include <bsp_om.h>
#include <bsp_dsp.h>
#include <bsp_ipc.h>
#include <bsp_icc.h>
#include <bsp_sec.h>
#include <bsp_sram.h>
#include <bsp_nandc.h>
#include <bsp_shared_ddr.h>
#include <drv_mailbox.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>

#include <product_config.h>
#include "bsp_sram.h"

#include "drv_chg.h"




int bsp_dsp_is_hifi_exist(void)
{
    int ret = 0;
    DRV_MODULE_SUPPORT_STRU   stSupportNv = {0};

    ret = (int)bsp_nvm_read(NV_ID_DRV_MODULE_SUPPORT, (u8*)&stSupportNv, sizeof(DRV_MODULE_SUPPORT_STRU));
    if (ret)
        ret = 0;
    else
        ret = (int)stSupportNv.hifi;

    return ret;
}


static int bsp_dsp_load_image(char* part_name)
{
    int ret = 0;
    u32 offset = 0;
    u32 skip_len = 0;

    void *bbe_ddr_addr = NULL;
    void *tds_data_addr = NULL;

    /*coverity[var_decl] */
    struct image_head head;


    /* clean ok flag */
    writel(0, (void*)SHM_MEM_DSP_FLAG_ADDR);
	/* 指向一块DDR   空间用于存放镜像和配置数据 */
    bbe_ddr_addr = (void*)ioremap_nocache(DDR_TLPHY_IMAGE_ADDR, DDR_TLPHY_IMAGE_SIZE);
	
    tds_data_addr = (void*)ioremap_nocache(DDR_LPHY_SDR_ADDR + 0x1C0000, 0x40000);
    if ((NULL == bbe_ddr_addr) || (NULL == tds_data_addr))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "fail to io remap, %d \r\n", __LINE__);
        ret = -ENOMEM;
        goto err_unmap;
    }
	/* 获得在nand   中的bbe   镜像头 */
    if (NAND_OK != bsp_nand_read(part_name,  0, (char*)&head, sizeof(struct image_head), &skip_len))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "fail to load dsp image head\r\n");
        ret = NAND_ERROR;
        goto err_unmap;
    }

    /*coverity[uninit_use_in_call] */
	/* 判断是否找到dsp   镜像 */
    if (memcmp(head.image_name, DSP_IMAGE_NAME, sizeof(DSP_IMAGE_NAME)))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "dsp image not found\r\n");
        goto err_unmap;
    }

    offset += LPHY_BBE16_MUTI_IMAGE_OFFSET + sizeof(struct image_head) + skip_len;
	/* 将镜像从nand   读入申请的ddr   中 */
    if (NAND_OK == bsp_nand_read(part_name, offset, (char*)bbe_ddr_addr, LPHY_BBE16_MUTI_IMAGE_SIZE, &skip_len))
    {
        printk(KERN_INFO"succeed to load dsp image, address: 0x%x, size: 0x%x\r\n",
            DDR_TLPHY_IMAGE_ADDR, DDR_TLPHY_IMAGE_SIZE);
    }
    else
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "fail to load dsp image\r\n");
        ret = NAND_ERROR;
        goto err_unmap;
    }

    offset += LPHY_BBE16_MUTI_IMAGE_SIZE + skip_len;
	/* 将td   配置信息从nand   读入申请的ddr   中 */
    if (NAND_OK == bsp_nand_read(part_name, offset, (char*)tds_data_addr, TPHY_BBE16_CFG_DATA_SIZE, &skip_len))
    {
        printk(KERN_INFO"succeed to load TD config data, address: 0x%x, size: 0x%x\n",
            DDR_LPHY_SDR_ADDR + 0x1C0000, 0x40000);
    }
    else
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "fail to load TD config data\n");
        ret = NAND_ERROR;
        goto err_unmap;
    }


    /* set the ok flag of dsp image */
    writel(DSP_IMAGE_STATE_OK, (void*)SHM_MEM_DSP_FLAG_ADDR);

err_unmap:
    if (NULL != bbe_ddr_addr)
        iounmap(bbe_ddr_addr);
    if (NULL != tds_data_addr)
        iounmap(tds_data_addr);

    return ret;
}


int __init bsp_dsp_probe(struct platform_device *pdev)
{
    int ret = 0;

    struct ST_PART_TBL* dsp_part = NULL;
	/* 通过模块名来查找相应模块的镜像 */
    dsp_part = find_partition_by_name(PTABLE_DSP_NM);
    if(NULL == dsp_part)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "load ccore image succeed\r\n");
        ret = -EAGAIN;
        goto err_no_part;
    }
	/* 通过镜像名来加载dsp  镜像 */
    ret = bsp_dsp_load_image(dsp_part->name);

err_no_part:


    return ret;
}

static struct platform_device bsp_dsp_device = {
    .name = "bsp_dsp",
    .id = 0,
    .dev = {
    .init_name = "bsp_dsp",
    },
};

static struct platform_driver bsp_dsp_drv = {
    .probe      = bsp_dsp_probe,
    .driver     = {
        .name     = "bsp_dsp",
        .owner    = THIS_MODULE,
    },
};

static int bsp_dsp_acore_init(void);
static void bsp_dsp_acore_exit(void);

static int __init bsp_dsp_acore_init(void)
{
    int ret = 0;
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        printk(KERN_ERR "nandc_mtd_dload_proc_deal:get smem_data error\n");
        ret = - EINVAL;
        return ret;
    }
    /*升级模式下，不启动加载DSP*/
    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        return ret;
    }

    ret = platform_device_register(&bsp_dsp_device);
    if(ret)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "register his_modem device failed\r\n");
        return ret;
    }

    ret = platform_driver_register(&bsp_dsp_drv);
    if(ret)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_DSP, "register his_modem driver failed\r\n");
        platform_device_unregister(&bsp_dsp_device);
    }

    return ret;
}

static void __exit bsp_dsp_acore_exit(void)
{
    platform_driver_unregister(&bsp_dsp_drv);
    platform_device_unregister(&bsp_dsp_device);
}

module_init(bsp_dsp_acore_init);
module_exit(bsp_dsp_acore_exit);

MODULE_AUTHOR("z00227143@huawei.com");
MODULE_DESCRIPTION("HIS Balong V7R2 DSP load");
MODULE_LICENSE("GPL");



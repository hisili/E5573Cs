

/* 头文件包含 */
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <product_config.h>
#include <DrvInterface.h>
#include <asm/uaccess.h>
#include "vsim.h"

/*全局变量 节点可操作的数据结构*/
static struct proc_dir_entry *g_vsim_proc_file = NULL;

/*************************************************************************
* 函数名称   :  drv_vsim_proc_read
* 功能描述   :  读取共享内存vsim数据
* 输入参数   :  filp--文件指针，buffer--缓存区，
                                length--读取的长度，offset--读取偏移量
* 输出参数   :  buffer : 从内核将解锁状态传给用户态
* 返回值         :  0 | len : 操作成功
*                              -1: 操作失败
**************************************************************************/
static ssize_t drv_vsim_proc_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    void __user *buf_usr = (void __user *)buffer;

    int ret = -1,mode = 0;
    TAF_VSIM_DATA_AREA *p_area = (TAF_VSIM_DATA_AREA *)SHM_MEM_VSIM_DATA_ADDR;

    printk(KERN_ERR "vsim ddr data:\n");
    printk(KERN_ERR "p_area->magic_start = 0x%x\n", p_area->magic_start);
    printk(KERN_ERR "p_area->EventType = 0x%x\n", p_area->EventType);
    printk(KERN_ERR "sizeof(p_area) = %d\n", sizeof(TAF_VSIM_DATA_AREA));


    ret = copy_to_user(buf_usr, (void *)p_area, sizeof(TAF_VSIM_DATA_AREA));

    if(0 != ret)
    {
        printk(KERN_ERR "\r\n [drv_vsim_proc_read]copy_to_user fail!\r\n");
    }

    memset(p_area, 0x0, sizeof(TAF_VSIM_DATA_AREA));

    return ret;
}

/* vsim共享内存节点的操作函数数据结构*/
static struct file_operations drv_vsim_proc_ops = 
{
    .read  = drv_vsim_proc_read,
};

/*************************************************************************
* 函数名         :  create_vsim_proc_file
* 功能描述   :  创建节点
* 输入参数   :  void
* 输出参数   :  void
* 返回值         :  void
**************************************************************************/
static void create_vsim_proc_file(void)
{
    /* 设置权限 */
    g_vsim_proc_file = create_proc_entry(DRV_VSIM_PROC_FILE, 0444, NULL);
        
    if(g_vsim_proc_file)
    {
        g_vsim_proc_file->proc_fops = &drv_vsim_proc_ops;
    }
    else
    {
        pr_warning("%s: create proc entry for vsim failed\n", __FUNCTION__);
    }
}

/*************************************************************************
* 函数名         :  remove_vsim_proc_file
* 功能描述   :  删除节点
* 输入参数   :  void
* 输出参数   :  void
* 返回值        :  void
**************************************************************************/
static void remove_vsim_proc_file(void)
{
    remove_proc_entry(DRV_VSIM_PROC_FILE, NULL);
}

static int __init vsim_drv_init(void)
{
    create_vsim_proc_file();
    return 0;
}

static void __exit vsim_drv_exit(void)
{
    remove_vsim_proc_file();
}

module_init(vsim_drv_init);

module_exit(vsim_drv_exit);

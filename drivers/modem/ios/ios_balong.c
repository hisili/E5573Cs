/*
 * ios_balong.c - hisilicon balong ios driver
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*lint -save -e537*/
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include "product_config.h"
#include "bsp_ipc.h"
#include "bsp_version.h"
#include "bsp_om.h"
#include "bsp_reg_def.h"
#include "ios_list.h"
#include "soc_memmap.h"


/*lint -restore*/

#define  ios_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_GPIO, "[ios]: <%s> <%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  ios_print_info(fmt, ...)     (bsp_trace(BSP_LOG_LEVEL_ERROR,  BSP_MODU_GPIO, "[ios]: "fmt, ##__VA_ARGS__))


static void mmc0_to_gpio_save(void)
{

}

void mmc0_to_gpio_mux(void)
{
	mmc0_to_gpio_save();


}

void gpio_to_mmc0_mux(void)
{

}

MODULE_AUTHOR("l00225826@huawei.com");
MODULE_DESCRIPTION("HIS Balong V7R2 IO PAD");
MODULE_LICENSE("GPL");




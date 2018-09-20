/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/hardware/arm_timer.h>
#include <linux/module.h>
#include <linux/debugfs.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/of_address.h>
#include <bsp_icc.h>
#include <bsp_socp.h>
#include <product_config.h>



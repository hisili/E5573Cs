/******************************************************************************

  Copyright (C), 2001-2013, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : kerneldev.h
  Version       : Initial Draft
  Created       :
  Description   : led_ctl Init
  Function List :

  History        :
  1.Date         : 2013/11/26
    Author       : 王丽00249966
    Modification : Created

******************************************************************************/
#ifndef _KERNELDEV_H
#define _KERNELDEV_H

#include <linux/kthread.h>
#include <linux/sched.h>

#include "platform_api_cpe.h"

#ifdef __cplusplus
extern "C" {
#endif


/* LED 灯当前状态: 亮或者灭*/
#define LED_ON   (1)
#define LED_OFF (0)

int kernel_led_write(LED_IOCTL_ARG stCtrlParms, int next_state);
int kernel_led_blink(LED_IOCTL_ARG stCtrlParms);


#ifdef __cplusplus
}
#endif

#endif /* _KERNELDEV_H */
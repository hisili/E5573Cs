

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
/*lint -save -e537*/
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/completion.h>
#include <linux/wakelock.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
/*lint -restore*/
//#include "excDrv.h"
//#include "BSP.h"
#include "drv_mailbox.h"
#include "drv_mailbox_cfg.h"
#include "drv_mailbox_debug.h"
//#include "drv_mailbox_test_linux.h"
#include "drv_timer.h"
#include "drv_ipc.h"





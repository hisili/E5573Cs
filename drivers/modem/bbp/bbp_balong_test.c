

/*lint --e{537}*/
#include <linux/delay.h>
#include <osl_thread.h>
#include <osl_spinlock.h>

#include "soc_memmap.h"
#include "hi_base.h"
#include "hi_syscrg.h"
#include "hi_syssc.h"

#include "bsp_memmap.h"
#include <bsp_hardtimer.h>

#include "bsp_bbp.h"
#include "bbp_balong.h"

/*压力测试代码*/
static void bbp_msleep(int msecs)
{
    msleep((u32)msecs);
}
/*****************************************************************************
* 函数  : bsp_bbp_getcurtime
* 功能  : 被PS调用，用来获取系统精确时间
* 输入  : void
* 输出  : u64 *pcurtime
* 返回  : u32
*****************************************************************************/
u32 bsp_bbp_getcurtime_old(unsigned long long *pcurtime)
{
	/*lint -save -e958*/
    unsigned long long  timervalue[4];
	/*lint -restore*/

    timervalue[0] = get_hi_bbp_systime_abs_timer_l_abs_timer_l();
    timervalue[1] = get_hi_bbp_systime_abs_timer_h_abs_timer_h();
    timervalue[2] = get_hi_bbp_systime_abs_timer_l_abs_timer_l();
    timervalue[3] = get_hi_bbp_systime_abs_timer_h_abs_timer_h();

    if(timervalue[2] < timervalue[0])
    {
        (*pcurtime) = ((timervalue[3] - 1) << 32) | timervalue[0];
    }
    else
    {
        (*pcurtime) = (timervalue[1] << 32) | timervalue[0];
    }

    return 0;
}

u32 bbp_test_flag = 1;
static s64 bbp_get_timer_slice_delta(u64 begin,u64 end)
{
    u64 full_time = ~(u64)0x0;
    return ((end>=begin)?(end-begin):(full_time-begin+end));
}
void bbp_timer_test(void)
{
    u64 time_new = 0;
    u64 time_old = 0;
    u64 time_delta = 0;
    spinlock_t bbp_lock;
    unsigned long bbp_flag;

    while(bbp_test_flag)
    {
        spin_lock_irqsave(&bbp_lock,bbp_flag);
        bsp_bbp_getcurtime(&time_new);
        bsp_bbp_getcurtime_old(&time_old);
        spin_unlock_irqrestore(&bbp_lock,bbp_flag);
        bbp_msleep(10);
        time_delta = bbp_get_timer_slice_delta(time_new,time_old);
        if(time_delta > 5)
            bbp_print_error("error,time_delta is %lu",time_delta);
    }
}
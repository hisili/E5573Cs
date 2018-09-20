
#ifndef __WAKELOCK_DEBUG_H
#define __WAKELOCK_DEBUG_H

#include <linux/wakelock.h>
#include <bsp_om.h>
#include <bsp_hardtimer.h>
#include <product_config.h>

#define  vote_printf(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_WAKELOCK, "[wakelock]: <%s> "fmt"", __FUNCTION__, ##__VA_ARGS__))


#if (FEATURE_ON == MBB_COMMON)
/*A core record lock num*/
#define  MAX_LOCK_NUM  36
#else
#define  MAX_LOCK_NUM  32
#endif /*(FEATURE_ON == MBB_COMMON)*/

struct wakelock_lookup{
    int lock_id;
    struct wake_lock *lock;
};

struct suspend_info{
    unsigned int enter_suspend_count;
    unsigned int enter_suspend_time_stamp;    
    unsigned int quit_suspend_count;
    unsigned int quit_suspend_time_stamp;    
    unsigned int enter_early_suspend_count;
    unsigned int enter_early_suspend_time_stamp;    
    unsigned int quit_early_suspend_count;    
    unsigned int quit_early_suspend_time_stamp;    
    unsigned int enter_late_resume_count;
    unsigned int enter_late_resume_time_stamp;    
    unsigned int quit_late_resume_count;    
    unsigned int quit_late_resume_time_stamp;
};
extern struct suspend_info suspend_info_record;
extern struct wakelock_lookup hi6930_wakelock[MAX_LOCK_NUM];
extern int wakelock_count;

/************************************************************************
*wakelock模块调试接口
************************************************************************/
void debug_wake_lock(int id);
void debug_wake_unlock(int id);
void debug_wakelock(void);
void debug_wake_lock_destroy(int id);

/************************************************************************
*wakelock模块自动化测试函数
************************************************************************/
int wake_lock_init_case0(void);
int wake_lock_case0(void);
int wake_unlock_case0(void);

/************************************************************************
*wakelock模块MNTN功能测试函数
************************************************************************/
void test_wakelock(int num);
void test_wakelock32(int num);

#endif


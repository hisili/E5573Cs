#ifndef REB_TASK_H
#define REB_TASK_H


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
typedef enum
{
    FASTONOFF_MODE_CLR = 0,       /*正常开机状态*/
    FASTONOFF_MODE_SET = 1,       /*假关机状态  */
    FASTONOFF_MODE_MAX = 2,       /*非法值*/
}FASTONOFF_MODE;
/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
int reb_nv_kernel_read(void);
void hard_timer_creat(unsigned int time_in_ms, unsigned int input_para );
void hard_timer_reset( unsigned int time_in_ms );
void hard_timer_delete(void);
FASTONOFF_MODE fastOnOffGetFastOnOffMode(void);
void reb_for_power_off(void);
int reb_is_factory_mode(void);
unsigned long  reb_get_current_systime(void);
#endif
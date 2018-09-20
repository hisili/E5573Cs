#ifndef __CHG_CHARGE_STM_H__
#define __CHG_CHARGE_STM_H__






/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define CHG_CHGR_TYPE_CHECK_MAX_RETRY_TIMES       (4)

#define CHG_CHGR_TYPE_CHECK_INTERVAL_IN_MS        (3500)

/*Fast charge protection timer, in second. */
#define MS_IN_SECOND                            (1000)
#define SECOND_IN_HOUR                          (60 * 60)
#ifdef BSP_CONFIG_BOARD_E5_E5578
#define CHG_FAST_CHG_TIMER_VALUE                (6 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (6  * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (12 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (12  * SECOND_IN_HOUR)
#elif defined(BSP_CONFIG_BOARD_E5_E5770s) 
#define CHG_FAST_CHG_TIMER_VALUE                (12 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (12  * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (18 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (18  * SECOND_IN_HOUR)
#elif defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#define CHG_FAST_CHG_TIMER_VALUE                (12 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (15 * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (30 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (46  * SECOND_IN_HOUR)
#else
#define CHG_FAST_CHG_TIMER_VALUE                (5 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_CHG_TIMER_VALUE       (5  * SECOND_IN_HOUR)

#define CHG_FAST_USB_TIMER_VALUE                (10 * SECOND_IN_HOUR)
#define CHG_POWEROFF_FAST_USB_TIMER_VALUE       (10  * SECOND_IN_HOUR)
#endif

/*Time Interval for toggle CEN.*/
#define CHG_TOGGLE_CEN_INTVAL_IN_MS             (100)

/*Time Interval for switch to SLOW POLLING while battery only.*/
#define CHG_SWITCH_TO_SLOW_POLL_INTERVAL_IN_SEC (60)

/*Indicate battery charging start/stop flag.*/
#define CHG_UI_START_CHARGING                   (1)
#define CHG_UI_STOP_CHARGING                    (0)

#define EXTCHG_DELAY_COUNTER_SIZE               (500)        /* 延时500ms */

/* 若定义补电宏 */
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
#define CHG_DELAY_COUNTER_SIZE                  (10)        /* 延时10ms */
#ifdef BSP_CONFIG_BOARD_E5_E5578  
#define TBAT_SUPPLY_VOLT                        (3780)      /*电池补电阈值40%电量*/
#define TBAT_DISCHG_VOLT                        (4085)      /*电池放电阈值80%电量*/

#define TBAT_SUPPLY_STOP_VOLT                   (3825)      /*电池充电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (4050)      /*电池放电截止电压 */
#elif defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
#define TBAT_SUPPLY_VOLT                        (3460)      /*电池补电阈值25%电量*/
#define TBAT_DISCHG_VOLT                        (3970)      /*电池放电阈值80%电量*/

#define TBAT_SUPPLY_STOP_VOLT                   (3540)      /*电池充电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (3860)      /*电池放电截止电压 */
#else
#define TBAT_SUPPLY_VOLT                        (3696)      /*电池补电阈值40%电量*/
#define TBAT_DISCHG_VOLT                        (3970)      /*电池放电阈值80%电量*/

#define TBAT_SUPPLY_STOP_VOLT                   (3750)      /*电池充电截止电压 */
#define TBAT_DISCHG_STOP_VOLT                   (3940)      /*电池放电截止电压 */
#endif

#define TBAT_SUPPLY_CURR_SUCCESS                (0x0)       /*补电成功*/
#define TBAT_NO_NEED_SUPPLY_CURR                (0x1)       /*不需要补电*/

#define TBAT_STOP_DELAY_COUNTER                 (100)       /* 补电停止时间 */
#define TBAT_SUPLY_DELAY_COUNTER                (2300)      /* 补电启动时间 */
#endif /*HUAWEI_CHG_COMPENSATE */

/*关机充电关机检测次数**/
#define CHARGE_REMOVE_CHECK_MAX                 (1)


/*非标配电池充电电流设置: 同时会设置20pct标记*/
#define CHG_DEF_BATT_CUR_LEVEL                  (512)
/*非标配电池充电电压设置*/
#define CHG_DEF_BATT_VREG_LEVEL                 (4150)

#define CHG_DEF_BATT_TEMP_OVER_THRES        (45)
#define CHG_DEF_BATT_TEMP_OVER_RUSUM_THRES        (42)

#if (FEATURE_ON == MBB_CHG_BATT_EXPAND_PROTECT)
/*电池膨胀保护方案参数*/
/*参数说明*/
/*
    区分应用场景，只要充电器在位且电池温度超过45°时就不让电池电压超过4.1V：
    1、充电器在位and电池温度>=45°and电池电压>=4.1V 三个条件同时满足，则USB进入Suspend/前段限流100MA模式；
    2、电池电压<4.05V（需要有滞回区间）或者电池温度<42°，两个条件有一个满足则USB退出Suspend/前段限流100MA模式；
    3、高温复充门限统一由4.05V修改至4.0V（为了避免出现刚取消充电芯片输入限流100mA则出现复充问题）
*/
/*电池保护温度门限*/
#define CHG_BATTERY_PROTECT_TEMP            (45)
/*退出电池保护温度门限*/
#define CHG_BATTERY_PROTECT_RESUME_TEMP     (42)
/*电池电压限制门限*/
#define CHG_BATTERY_PROTECT_VOLTAGE         (4100)
/*退出电池保护电压门限*/
#define CHG_BATTERY_PROTECT_RESUME_VOLTAGE  (4050)
/*满电停充且充电器在位进入suspend/前段限流100MA的时间门限*/
#define CHG_BATTERY_PROTECT_CHGER_TIME_THRESHOLD_IN_SECONDS (16 * 60 * 60)
/*满电停充且充电器长时间在位状态常温复充门限*/
#define BATT_NORMAL_TEMP_RECHARGE_THR_LONG_TIME_NO_CHARGE    (4000)
/*高压电池电池膨胀高温复充门限统一改成了4.0V*/
#define BATT_EXP_HIGH_TEMP_RECHARGE_THR          (4000)
#endif

/*电池ID信息索引*/
typedef enum
{
    CHG_BATT_ID_DEF,
    CHG_BATT_ID_XINGWANGDA,
    CHG_BATT_ID_FEIMAOTUI, 
    CHG_BATT_ID_LISHEN, 
    CHG_BATT_ID_XINGWANGDA_3000,
    CHG_BATT_ID_FEIMAOTUI_3000, 
    CHG_BATT_ID_LISHEN_3000, 
    CHG_BATT_ID_MAX
}CHG_BATT_ID;

/*----------------------------------------------*
 * 结构定义                                       *
 *----------------------------------------------*/
typedef struct
{
    uint32_t pwr_supply_current_limit_in_mA;
    uint32_t chg_current_limit_in_mA;
    uint32_t chg_CV_volt_setting_in_mV;
    uint32_t chg_taper_current_in_mA;
    boolean  chg_is_enabled; /*FALSE: Charge disable; TRUE: Charge enable.*/
}chg_hw_param_t;

typedef  void (*chg_stm_func_type )(void);
typedef struct
{
  chg_stm_func_type        chg_stm_entry_func;
  chg_stm_func_type        chg_stm_period_func;
  chg_stm_func_type        chg_stm_exit_func;
}chg_stm_type;

/*高温关机温度参数NV50016结构定义*/
typedef struct
{
    uint32_t      ulIsEnable;             //高温关机使能开关
    int32_t       lCloseAdcThreshold;     //高温关机温度门限
    uint32_t      ulTempOverCount;        //高温关机温度检测次数
}CHG_BATTERY_OVER_TEMP_PROTECT_NV;

/*高温关机温度参数NV52005结构定义*/
typedef struct
{
    uint32_t      ulIsEnable;
    int32_t       lCloseAdcThreshold;
    uint32_t      ulTempLowCount;
}CHG_BATTERY_LOW_TEMP_PROTECT_NV;


/*充电过程中温度参数NV 50385结构定义*/
typedef struct
{
    uint32_t    ulChargeIsEnable;                   //充电温保护使能
    int32_t     overTempchgStopThreshold;           //充电高温保护门限
    int32_t     subTempChgLimitCurrentThreshold;    //高温充电进入门限
    int32_t     lowTempChgStopThreshold;            //充电低温保护门限
    int32_t     overTempChgResumeThreshold;         //充电高温恢复温度门限
    int32_t     lowTempChgResumeThreshold;          //充电低温恢复温度门限
    uint32_t    chgTempProtectCheckTimes;           //充电停充轮询次数
    uint32_t    chgTempResumeCheckTimes;            //充电复充轮询次数
    int32_t     exitWarmChgToNormalChgThreshold;    //由高温充电恢复到常温充电温度门限
    int32_t     reserved2;                           //预留
}CHG_SHUTOFF_TEMP_PROTECT_NV_TYPE;

/*充电过程中温度参数NV50386结构定义*/
typedef struct
{
    int32_t         battVoltPowerOnThreshold;           //开机电压门限
    int32_t         battVoltPowerOffThreshold;          //关机电压门限
    int32_t         battOverVoltProtectThreshold;       //平滑充电过压保护门限(平滑值)
    int32_t         battOverVoltProtectOneThreshold;    //单次充电过压保护门限(单次值)
    int32_t         battChgTempMaintThreshold;          //区分高温停充和正常停充的判断门限
    int32_t         battChgRechargeThreshold;           //高温区间二次复充门限
    int32_t         VbatLevelLow_MAX;                   //低电上限门限
    int32_t         VbatLevel0_MAX;                     //0格电压上限门限
    int32_t         VbatLevel1_MAX;                     //1格电压上限门限
    int32_t         VbatLevel2_MAX;                     //2格电压上限门限
    int32_t         VbatLevel3_MAX;                     //3格电压上限门限
    int32_t         battChgFirstMaintThreshold;         //首次判断是否满电
    int32_t         battNormalTempChgRechargeThreshold; //常温区间复充门限
}CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE;

/* NV50364电池参数数据结构**/
typedef struct
{
    /*非工厂模式下电池部在位开机使能标志*/
    uint8_t no_battery_powerup_enable;
    /*异常关机后，下次插入电源进入开机模式使能标志*/
    uint8_t exception_poweroff_poweron_enable;
    /*低电关机禁止标志*/
    uint8_t low_battery_poweroff_disable;
    /*保留*/
    uint8_t reserved;
}POWERUP_MODE_TYPE;

struct chg_batt_data {
    unsigned int      id_voltage_min;
    unsigned int      id_voltage_max;
    CHG_SHUTOFF_VOLT_PROTECT_NV_TYPE    chg_batt_volt_paras;
    unsigned int      batt_id;
};

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
 
/*set chg para*/ 
void chg_set_hardware_parameter(const chg_hw_param_t* ptr_hw_param);
/*******************************************************************
Function:      chg_poll_bat_level
Description:   电池电压值加工处理的流程控制函数。
Calls:         chg_get_parameter_level-------------采集电压值值接口函数；
               chg_calc_average_volt_value--------电压平滑算法处理函数；
               huawei_set_temp------------------------电压补偿函数
Data Accessed: 无
Data Updated:  全局变量的温度值
Input:         无
Output:        无
Return:        无
*******************************************************************/
void chg_poll_bat_level(void);

#if (MBB_CHG_COULOMETER == FEATURE_ON || MBB_CHG_BQ27510 == FEATURE_ON)
/*******************************************************************
Function:      chg_poll_batt_soc
Description:   update the battery soc,if changed,report to app
Calls:         chg main task
Input:         none
Output:        none
Return:        none
*******************************************************************/
void chg_poll_batt_soc(void);
/*******************************************************************
Function:      chg_poll_batt_charging_state_for_coul
Description:   update the battery charging state,if changed,notify coul
Calls:         chg main task
Input:         none
Output:        none
Return:        none
*******************************************************************/
void chg_poll_batt_charging_state_for_coul(void);
/**********************************************************************
函 数 名  :chg_is_batt_in_state_of_emergency
功能描述  : 查询电池是否在紧急需要关机状态
输入参数  : 无。
输出参数  : 无。
返 回 值  : TRUE:yes，FALSE:no
注意事项  : 无。
***********************************************************************/
boolean chg_is_batt_in_state_of_emergency();
/**********************************************************************
函 数 名: chg_low_battery_event_handler
功能描述: 收到库仑计低电事件后的处理函数
输入参数: None
输出参数: None
返 回 值: 无。
注意事项: Linux系统中，调用该函数时须判断是否还有事件没有处理完成
***********************************************************************/
void chg_low_battery_event_handler(void);
#endif
/*******************************************************************
Function:      chg_poll_batt_temp
Description:   电池温度值加工处理的流程控制函数。
Calls:          chg_get_parameter_level-----------------采集温度值接口函数；
                chg_calc_average_temp_value-------------平滑算法处理函数；
                chg_huawei_set_temp----------------------温度补偿函数
                chg_temp_is_too_hot_or_too_cold_for_chg ----------充电温保护的温度检测函数
                chg_temp_is_too_hot_or_too_code_for_shutoff------温度关机条件检测函数
Data Accessed: 无
Data Updated:  全局变量的温度值
Input:          无
Output:         无
Return:         无
*******************************************************************/
void chg_poll_batt_temp(void);
/*******************************************************************
Function:      chg_batt_volt_init
Description:   电池电压门限值初始化函数，读取NV项，将读取的值存到全局变量中
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_batt_volt_init(void);
/*******************************************************************
Function:      chg_batt_temp_init
Description:   电池温度门限值初始化函数，读取NV项，将读取的值存到全局变量中
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_batt_temp_init(void);

/*****************************************************************************
 函 数 名  : chg_stm_get_cur_state
 功能描述  : Get current battery charge state: Fast charge, Transitition state, etc.
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
extern chg_stm_state_type chg_stm_get_cur_state(void);

/*****************************************************************************
 函 数 名  : chg_set_cur_chg_mode
 功能描述  : set the current  charge mode
 输入参数  : new_state  The new state we gonna update to.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  :
 CALL-WHOM : None.
 NOTICE    : Helper function.
*****************************************************************************/
extern void chg_set_cur_chg_mode(CHG_MODE_ENUM chg_mode);

/*****************************************************************************
 函 数 名  : chg_stm_get_chgr_type
 功能描述  : Get current external charger type:
             CHG_CHGR_UNKNOWN: Chgr type has not been got from USB module.
             CHG_WALL_CHGR   : Wall standard charger, which D+/D- was short.
             CHG_USB_HOST_PC : USB HOST PC or laptop or pad, etc.
             CHG_NONSTD_CHGR : D+/D- wasn't short and USB enumeration failed.
             CHG_CHGR_INVALID: External Charger invalid or absent.
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current external charger type.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
             The real chgr type checking would be done by USB module.
*****************************************************************************/
extern chg_chgr_type_t chg_stm_get_chgr_type(void);

/*****************************************************************************
 函 数 名  : chg_stm_get_cur_state
 功能描述  : Get current battery charge state: Fast charge, Transitition state, etc.
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
extern CHG_MODE_ENUM chg_get_cur_chg_mode(void);


/*****************************************************************************
 函 数 名  : chg_check_and_update_hw_param_per_chgr_type
 功能描述  : When chgr_type_checking timer expired, this function would be called
             via charge main task.
             As its name suggests, it will check the chgr type from USB and if got,
             update the charge hardware parameter, or re-fire the timer utils max
             retry time reached.
 输入参数  : VOID.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : Charge Main Task.
 CALL-WHOM : None.
 NOTICE    : If max retry time reached, the chgr type would be set to CHG_NONSTD_CHGR.
             Need exported.
*****************************************************************************/
void chg_check_and_update_hw_param_per_chgr_type(void);

/**********************************************************************
函 数 名  : chg_get_batt_id_valid
功能描述  : 检测当前的电池是否在位，判断电池自身的ntc温度
输入参数  : 无。
输出参数  : 无。
返 回 值  : TRUE: 电池在位
            FALSE: 电池不在位
注意事项  : 无。
***********************************************************************/
extern boolean chg_get_batt_id_valid(void);

/**********************************************************************
函 数 名  :chg_get_batt_level
功能描述  : 获取当前电池电量格数的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量格数
注意事项  : 无。
***********************************************************************/
extern BATT_LEVEL_ENUM chg_get_batt_level(void);


/**********************************************************************
函 数 名  :chg_get_batt_level
功能描述  : 获取当前电池电量百分比的接口函数
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量百分比
注意事项  : 无。
***********************************************************************/
extern int32_t chg_get_sys_batt_capacity(void);

/**********************************************************************
函 数 名  :chg_set_sys_batt_capacity
功能描述  : 设置当前电池电量百分比的接口函数
输入参数  : capacity :电池电量百分比
输出参数  : 无。
返 回 值  : 电池电量百分比
注意事项  : 无。
***********************************************************************/
extern void chg_set_sys_batt_capacity(int32_t capacity);

/**********************************************************************
函 数 名  :chg_set_batt_time_to_full
功能描述  : 设置预测的剩余充电时间
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量距充满还有多长时间
注意事项  : 无。
***********************************************************************/
extern void chg_set_batt_time_to_full(int32_t time_to_full);

/**********************************************************************
函 数 名  :chg_get_batt_time_to_full
功能描述  : 获取电池电量距充满还有多长时间
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池电量距充满还有多长时间
注意事项  : 无。
***********************************************************************/
extern int32_t chg_get_batt_time_to_full(void);

/**********************************************************************
函 数 名  : chg_is_batt_full
功能描述  : 判断电池是否满电
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池满电

            0:电池非满电
注意事项  : 无。
***********************************************************************/
extern boolean chg_is_batt_full(void);

/**********************************************************************
函 数 名  : chg_is_batt_full_for_start
功能描述  : 判断电池电压是否满足开始充电条件
输入参数  : 无。
输出参数  : 无。
返 回 值  : 1:电池满电，不需要启动充电
            0:电池非满电，需要启动充电
注意事项  : 无。
***********************************************************************/
extern boolean chg_is_batt_full_for_start(void);


extern int32_t chg_get_bat_status(void);


extern int32_t chg_get_bat_health(void);


extern int32_t chg_get_extchg_status(void);

/**********************************************************************
函 数 名  : chg_get_batt_temp_state
功能描述  : 获取当前电池温度状态
输入参数  : 无。
输出参数  : 无。
返 回 值  : 电池温度状态
注意事项  : 无。
***********************************************************************/
TEMP_EVENT chg_get_batt_temp_state(void);

/*****************************************************************************
 函 数 名  : chg_stm_switch_state
 功能描述  : Charge State Machine Switch State function. State Machine or other
             Sub-Module call this function change the battery charge state.
 输入参数  : new_state  The new state we gonna switch to.
 输出参数  : None
 返 回 值  : VOID
 WHO-CALL  : All states' periodic function.
             Polling timer and charger insertion/removal event(BH)
             from charge task.
 CALL-WHOM : The exit function of current state.
             The entry function of new state.
 NOTICE    : If the new_state is Transition State, its periodic would be called.
*****************************************************************************/
extern void chg_stm_switch_state(chg_stm_state_type new_state);

/*****************************************************************************
 函 数 名  : chg_stm_periodic_checking_func
 功能描述  : Periodic checking function of charge state machine. Charge main
             task would call it periodically once polling timer triggered.
 输出参数  : None
 返 回 值  : VOID
 WHO-CALL  : chg main task.
 CALL-WHOM : All states' period function.
 NOTICE    : The periodic function of transition state should NOT be called
             here.
             It's better NOT to call this function in timer/interrupt context.
             Need Exported.
*****************************************************************************/
void chg_stm_periodic_checking_func(void);

/*****************************************************************************
 函 数 名  : chg_stm_init
 功能描述  : Charge State Machine Sub-Moudle initialize procedure.
 输入参数  : VOID
 输出参数  : VOID
 返 回 值  : CHG_STM_SUCCESS for initialize successful
             CHG_STM_FAILED  for initialize failed
 WHO-CALL  :
 CALL-WHOM : chg_stm_set_chgr_type
             chg_stm_set_cur_state
             chg_stm_switch_state
*****************************************************************************/
int32_t chg_stm_init(void);


/*****************************************************************************
 函 数 名  : chg_get_charging_status
 功能描述  :获取当前是否正在充电
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
boolean chg_get_charging_status(void);

/**********************************************************************
函 数 名  : chg_is_ftm_mode
功能描述  : 判断目前是否为ftm模式
输入参数  : 无。
输出参数  : 无。
返 回 值  : 目前是否为工厂模式
           TRUE : 工厂模式
           FALSE: 非工厂模式
注意事项  : 无。
***********************************************************************/
extern boolean chg_is_ftm_mode(void);

/*****************************************************************************
 函 数 名  : chg_stm_set_chgr_type
 功能描述  : Update the current type of external charger.
 输入参数  : chgr_type  The charger type we gonna update to.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : chg_check_and_update_hw_param_per_chgr_type.
 CALL-WHOM : None.
 NOTICE    : Helper function.
*****************************************************************************/
extern void chg_stm_set_chgr_type(chg_chgr_type_t chgr_type);
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)

extern chg_chgr_type_t extchg_charger_type_detect(void);
#endif
/*****************************************************************************
 函 数 名  : chg_start_chgr_type_checking
 功能描述  : Callback function of charger type checking timer.
             This timer is not autoload, we may re-start it in
             chg_check_and_update_hw_param_per_chgr_type.
 输入参数  : VOID.
 输出参数  : None
 返 回 值  : VOID.
 WHO-CALL  : OS timer interrupt or task.
 CALL-WHOM : None.
 NOTICE    : This function run in timer context.
*****************************************************************************/
extern void chg_start_chgr_type_checking(void);


#if (MBB_CHG_WARM_CHARGE == FEATURE_ON)

void chg_stm_set_pre_state(chg_stm_state_type pre_state);


chg_stm_state_type chg_stm_get_pre_state(void);


boolean is_batttemp_in_warm_chg_area( void );
#endif /* MBB_CHG_WARM_CHARGE */

/* 若定义补电宏 */
#if (MBB_CHG_COMPENSATE == FEATURE_ON)
/**********************************************************************
函 数 名  :  chg_tbat_status_get
功能描述  :  TBAT AT^TCHRENABLE?是否需要补电

输入参数  : 无
输出参数  : 无
返 回 值      : 1:需要补电
			    0:不需要补电
注意事项  : 无
***********************************************************************/
int32_t chg_tbat_status_get(void);

/**********************************************************************
函 数 名  :  chg_is_sply_finish
功能描述  :  判断补电是否完成

输入参数  : 无
输出参数  : 无
返 回 值      : 1:完成补电
                0:补电未完成
注意事项  : 无
***********************************************************************/
boolean chg_is_sply_finish(void);

/**********************************************************************
函 数 名  : chg_batt_suply_proc
功能描述  :  电池补电任务

输入参数  : 无
输出参数  : 无
返 回 值      : 1:不需要补电
                  0:补电成功
注意事项  : 无
***********************************************************************/
int32_t chg_batt_supply_proc(void *task_data);

/**********************************************************************
函 数 名  :  chg_tbat_chg_sply
功能描述  :  TBAT AT^TCHRENABLE=4补电实现
                        补电成功LCD显示成功图标
输入参数  : 无
输出参数  : 无
返 回 值      :
注意事项  : 无
***********************************************************************/
int32_t chg_tbat_chg_sply(void);
#endif /* MBB_CHG_COMPENSATE */

/*****************************************************************************
 函 数 名  : chg_get_charging_status
 功能描述  :获取当前是否正在充电
 输入参数  : None.
 输出参数  : None
 返 回 值  : Current battery charge state.
 WHO-CALL  : DFT and other sub-modules.
 CALL-WHOM : None.
 NOTICE    : Need exported.
*****************************************************************************/
extern boolean chg_get_charging_status(void);
/**********************************************************************
函 数 名  : boolean chg_is_exception_poweroff_poweron_mode(void)
功能描述  :  判断目前是否为异常关机后，下次插入电源后进入开机模式使能开机使能模式
输入参数  : 无。
输出参数  : 无。
返 回 值  : 是否为异常关机后，下次插入电源后进入开机模式使能
           TRUE : 是
           FALSE: 否
注意事项  : 无。
***********************************************************************/
extern boolean chg_is_exception_poweroff_poweron_mode(void);

/**********************************************************************
函 数 名  : chg_get_cur_batt_temp(void)
功能描述  :  获取当前电池电压
输入参数  : 无。
输出参数  : 无。
返 回 值  : 是否为异常关机后，下次插入电源后进入开机模式使能
           TRUE : 是
           FALSE: 否
注意事项  : 无。
***********************************************************************/
extern int32_t chg_get_cur_batt_temp(void);

/**********************************************************************
函 数 名  : chg_get_sys_batt_temp(void)
功能描述  :  获取当前电池温度平滑值
输入参数  : 无。
输出参数  : 无。
返 回 值  : 是否为异常关机后，下次插入电源后进入开机模式使能
           TRUE : 是
           FALSE: 否
注意事项  : 无。
***********************************************************************/
extern int32_t chg_get_sys_batt_temp(void);

/**********************************************************************
函 数 名  : chg_get_sys_batt_volt(void)
功能描述  :  获取当前电池电压平滑值
输入参数  : 无。
输出参数  : 无。
返 回 值  : 是否为异常关机后，下次插入电源后进入开机模式使能
           TRUE : 是
           FALSE: 否
注意事项  : 无。
***********************************************************************/
extern int32_t chg_get_sys_batt_volt(void);

/**********************************************************************
函 数 名      : void load_on_off_mode_parameter(void)
功能描述  :  读取硬件测试开机模式NV
输入参数  :none
输出参数  : 无。
返 回 值      : 无。
注意事项  : 在预充电启动前需要调用
***********************************************************************/
void load_on_off_mode_parameter(void);
/**********************************************************************
函 数 名  : void load_factory_mode_flag_init(void)
功能描述  : 读取工厂模式标志NV参数
输入参数  : none
输出参数  : 无。
返 回 值  : 无。
注意事项  : 无。
***********************************************************************/
void load_ftm_mode_init(void);

/*******************************************************************
Function:      chg_detect_batt_chg_for_shutoff
Description:   关机充电情况下外电源移除，进行关机处理。
Data Accessed: 无
Data Updated:  无
Input:         无
Return:        无
*******************************************************************/
void chg_detect_batt_chg_for_shutoff(void);


extern void chg_update_power_suply_info(void);


void chg_print_test_view_info(void);

/**********************************************************************
FUNCTION:    chg_charger_insert_proc
DESCRIPTION: usb remove process
INPUT:       chg_chgr_type_t chg_type
OUTPUT:      None.
RETURN:      None.
NOTE:        None
***********************************************************************/
void chg_charger_insert_proc(chg_chgr_type_t chg_type);
/**********************************************************************
FUNCTION:    chg_charger_remove_proc
DESCRIPTION: usb remove process
INPUT:       chg_chgr_type_t chg_type
OUTPUT:      None.
RETURN:      None.
NOTE:        None
***********************************************************************/
void chg_charger_remove_proc(chg_chgr_type_t chg_type);

/**********************************************************************
FUNCTION:    chg_get_batt_id_volt
DESCRIPTION: get batt id volt
INPUT:       None
OUTPUT:      None.
RETURN:      batt id volt
NOTE:        None
***********************************************************************/
int32_t chg_get_batt_id_volt(void);

#ifdef CONFIG_MBB_FAST_ON_OFF

extern void chg_get_system_suspend_status(ulong64_t suspend_status);
#endif/*CONFIG_MBB_FAST_ON_OFF*/

#if (MBB_CHG_WIRELESS == FEATURE_ON)
/*****************************************************************************
 函 数 名  : chg_stm_set_wireless_online_st
 功能描述  : get the wireless online status.
 输入参数  : ONLINE: 无线充电在位
             OFFLINE:无线充电不在位
 输出参数  : None
 返 回 值  : VOID.
*****************************************************************************/
extern void chg_stm_set_wireless_online_st(boolean online);

/*****************************************************************************
 函 数 名  : chg_stm_get_wireless_online_st
 功能描述  : get the wireless online status.
 输入参数  :
 输出参数  : ONLINE: 无线充电在位
             OFFLINE:无线充电不在位
 返 回 值  : VOID.
*****************************************************************************/
extern boolean chg_stm_get_wireless_online_st(void);

#endif/*MBB_CHG_WIRELESS*/

#if (MBB_CHG_EXTCHG == FEATURE_ON)
/*****************************************************************************
函 数 名  : chg_stm_set_extchg_online_st
功能描述  : get the extchg online status.
输入参数  : ONLINE: 无线充电在位
          OFFLINE:无线充电不在位
输出参数  : None
返 回 值  : VOID.
*****************************************************************************/
extern void chg_stm_set_extchg_online_st(boolean online);

/*****************************************************************************
函 数 名  : chg_stm_get_extchg_online_st
功能描述  : get the extchg online status.
输入参数  : ONLINE: 无线充电在位
          OFFLINE:无线充电不在位
输出参数  : None
返 回 值  : VOID.
*****************************************************************************/
boolean chg_stm_get_extchg_online_st(void);


void chg_extchg_config_data_init(void);


extern boolean chg_get_extchg_online_status(void);
#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)

extern int extchg_gpio_isr_init(void);

extern int extchg_set_charge_enable(boolean enable);
extern void extchg_set_cur_level(EXTCHG_ILIM curr);
extern int usb_direction_flag_set(boolean flag);

#endif


extern void chg_extchg_insert_proc(void);

extern void chg_extchg_remove_proc(void);

extern void chg_extchg_monitor_func(void);

#endif/*MBB_CHG_EXTCHG*/


boolean chg_get_usb_online_status(void);


void chg_set_usb_online_status(boolean online);


boolean chg_get_ac_online_status(void);


void chg_set_ac_online_status(boolean online);


extern boolean is_chg_charger_removed(void);


extern int32_t chg_hardware_paras_init(void);


extern struct chg_batt_data *chg_get_batt_data(unsigned int id_voltage);


extern int32_t chg_batt_volt_paras_init(void);

/*get batt brand*/
extern uint32_t chg_get_batt_id(void);

#if defined(BSP_CONFIG_BOARD_E5577S_321) || defined(BSP_CONFIG_BOARD_E5577S_932) \
   || defined(BSP_CONFIG_BOARD_E5577S_324)|| defined(BSP_CONFIG_BOARD_E5577BS_937)
/**********************************************************************
函 数 名      : chg_high_bat_paras_init
功能描述  :  开机初始化,3000mA电池参数配置
输入参数  : 无。
输出参数  : 无。
返 回 值      : 无。
注意事项  : 无。
***********************************************************************/
extern void chg_high_bat_paras_init(void);



extern boolean extchg_circult_short_detect(void);
#endif
#if defined(BSP_CONFIG_BOARD_E5771H_937)
boolean extchg_is_perph_circult_short(void);
#endif
#endif /*__CHG_CHARGE_STM_H__*/
#if ( FEATURE_ON == MBB_CHG_USB_TEMPPT_ILIMIT )

int32_t chg_get_usb_health(void);

int32_t chg_get_usb_cur_temp(void);

boolean chg_get_usb_temp_protect_stat(void);

void chg_test_set_usb_temp_limit_and_resume(int32_t limit,int32_t resume);
#endif

void chg_set_supply_limit_by_stm_stat(void);


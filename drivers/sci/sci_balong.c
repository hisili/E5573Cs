

#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include "product_config.h"
#include "osl_types.h"
#include "osl_bio.h"
#include "osl_irq.h"
#include "bsp_pmu.h"
#include "bsp_icc.h"
#include "sci_balong.h"
#if(FEATURE_ON == MBB_WPG_COMMON)
#include <linux/wakelock.h>
#endif

#define WAIT_TIME 5
#define MOV_BIT 16

/* sci init flag */
bsp_sci_st g_sci_stat = {0};
sci_debug_info_st g_sci_debug = {0};
struct work_struct	sim_detect_work;
struct work_struct	sim_hpd_work;


#if(FEATURE_ON == MBB_WPG_COMMON)
static struct wake_lock sci_sim_out_wakelock;
#endif

void sci_pmu_in_callback(u32 *pSciInOutEvent);
void sci_pmu_out_callback(u32 *pSciInOutEvent);
void sci_debug_cnt_print(void);
void sci_debug_status_print(void);

static irqreturn_t sim_detect_irq_cb(int irq, void *dev_id) ;
static void sim_detect_work_handler(struct work_struct *data);
static void sim_hpd_work_handler(struct work_struct *data);



s32 bsp_sci_init(void)
{ 

	u32 detect_level;
	u32 int_status;
	u32 int_num;

	if(TRUE == g_sci_stat.sci_init_flag)
	{
		return OK;
	}

	g_sci_debug.sci0_init_cnt ++ ;

	if(BSP_OK!=gpio_request(SIM0_GPIO_DETECT,"sim_detect"))
	{
		SCI_PRINT("%s gpio_request failed! \n",__FUNCTION__);
		g_sci_debug.sci0_init_request_cnt ++ ;
		
		return BSP_ERROR;
	}
	gpio_direction_input(SIM0_GPIO_DETECT); 
	gpio_int_mask_set(SIM0_GPIO_DETECT);
	
	/*  获取detect管脚电平高低*/
	detect_level = gpio_get_value(SIM0_GPIO_DETECT);
#ifdef BSP_CONFIG_BOARD_E5_E5578 
    if(SIM_CARD_DETECT_LOW == detect_level){
        g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_IN;
        g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;
        gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_HIGH);
    }
    else {
        g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_OUT;
        g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;
        gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_LOW);
    }
#else
	if(SIM_CARD_DETECT_LOW == detect_level)
	{
		g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_OUT;
		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;

		/* set int trigger */
		gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_HIGH);
	}
	else 
	{
		g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_IN;
		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;

		gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_LOW);
	}
#endif
	/* int solve */    
    gpio_set_function(SIM0_GPIO_DETECT,GPIO_INTERRUPT);

	int_num = gpio_to_irq(SIM0_GPIO_DETECT);

	if(request_irq(int_num, sim_detect_irq_cb,IRQF_SHARED, DETECT_NAME_SIM, "sim_detect"))
	{
		SCI_PRINT("%s request_irq failed! \n",__FUNCTION__);
		g_sci_debug.sci0_init_err_cnt ++ ;
		
		return BSP_ERROR;
    }

	gpio_int_state_clear(SIM0_GPIO_DETECT);
	/* unmask int after pmu callback register */
	gpio_int_unmask_set(SIM0_GPIO_DETECT);

#ifndef BSP_CONFIG_BOARD_E5_E5578
	/* 注册PMU预警中断处理函数, 只需要处理HPD 上跳中断*/
	int_status = INT_LVL_SIM0_PMU_IN;
	(BSP_VOID)bsp_pmu_irq_callback_register(INT_LVL_SIM0_PMU_IN, (pmufuncptr)sci_pmu_in_callback, &int_status);

	int_status = INT_LVL_SIM0_PMU_OUT;
	(BSP_VOID)bsp_pmu_irq_callback_register(INT_LVL_SIM0_PMU_OUT, (pmufuncptr)sci_pmu_out_callback, &int_status);

	/* set pmu hpd debounce time */
	(BSP_VOID)bsp_pmu_sim_debtime_set(PMU_HPD_DEBOUNCE_TIME);
#endif
    g_sci_stat.sci0_pmu_hpd = FALSE;
    
    INIT_WORK(&sim_detect_work, sim_detect_work_handler);

    INIT_WORK(&sim_hpd_work, sim_hpd_work_handler);
    
#if(FEATURE_ON == MBB_WPG_COMMON)
    wake_lock_init(&sci_sim_out_wakelock, WAKE_LOCK_SUSPEND, "sci_simout_lock");
#endif

	/* print func error */
	SCI_PRINT("Acore: sci init ok \n");

	g_sci_stat.sci_init_flag = TRUE;
	
	return OK;
}


void sci_pmu_in_callback(u32 *pSciInOutEvent)
{
    g_sci_debug.sci0_pmu_hpd_in_enter_cnt ++;

	if((SIM_CARD_DETECT_HIGH == g_sci_stat.sci0_detect_level)&&(SIM_CARD_STAUTS_IN != g_sci_stat.sci0_card_satus))
	{
		/*add print here */
		g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_IN;
		
		g_sci_debug.sci0_pmu_hpd_in_cnt ++;

        schedule_work(&sim_hpd_work);
	}
	else
	{	
		g_sci_debug.sci0_pmu_hpd_in_err_cnt ++;
		return ;
	}
	
    return ;
}


void sci_pmu_out_callback(u32 *pSciInOutEvent)
{
    
    g_sci_debug.sci0_pmu_hpd_out_enter_cnt ++;
    
	if(FALSE == g_sci_stat.sci0_pmu_hpd)
	{
		g_sci_stat.sci0_pmu_hpd = TRUE;
		g_sci_debug.sci0_pmu_hpd_out_cnt ++;
	}
	else
	{
		g_sci_debug.sci0_pmu_hpd_out_err_cnt ++;
		return ;
	}

    return ;
}


/*******************************************************************************
  Function:      sim_detect_irq_cd(int irq, void *dev_id)
  Description:   sd  detect interrpt handle
  Input:         irq: irq number
                 devid: device id
  Output:        irq handle result
  Return:        NA
  Others:        NA
*******************************************************************************/
static irqreturn_t sim_detect_irq_cb(int irq, void *dev_id)    
{  
    u32 int_status = 0; 

	g_sci_debug.sci0_detect_cnt++;
	
	/*判断是否为GPIO_0_19的中断*/
    int_status = gpio_int_state_get(SIM0_GPIO_DETECT);
    if (!int_status)
    {
    	g_sci_debug.sci0_detect_get_err_cnt++;
        return IRQ_NONE;
    }
	
    /*锁中断，清状态*/
    gpio_int_mask_set(SIM0_GPIO_DETECT);
    gpio_int_state_clear(SIM0_GPIO_DETECT);
	
    schedule_work(&sim_detect_work);

    return IRQ_HANDLED;

}

/*******************************************************************************
  Function:      sim_detect_work(struct work_struct *data)
  Description:   sd detect handle work
  Input:         data:work_struct
                 devid:
  Output:        NA
  Return:        NA
  Others:        NA
*******************************************************************************/
static void sim_detect_work_handler(struct work_struct *data)
{

	u32 detect_level = SIM_CARD_DETECT_LOW;
    s32 u32Lenth;
	u32 pSciInOut;
	u32 channel_id = 0;
    
	detect_level = gpio_get_value(SIM0_GPIO_DETECT);

#if(FEATURE_ON == MBB_WPG_COMMON)
    wake_lock_timeout(&sci_sim_out_wakelock, 
                (long)msecs_to_jiffies(SCI_SIMOUT_WAKELOCK_TIMEOUT_IN_MS));
#endif
#ifdef BSP_CONFIG_BOARD_E5_E5578 
    if (SIM_CARD_DETECT_LOW == detect_level){
        msleep(WAIT_TIME);
        detect_level = gpio_get_value(SIM0_GPIO_DETECT);
        if((SIM_CARD_DETECT_LOW == detect_level) && (SIM_CARD_DETECT_HIGH == g_sci_stat.sci0_detect_level) && 
            (SIM_CARD_STAUTS_OUT == g_sci_stat.sci0_card_satus)){
            g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;
            g_sci_stat.sci0_card_satus   = SIM_CARD_STAUTS_IN;
            g_sci_debug.sci0_detect_low_cnt++;
            gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_HIGH);
        }
        else{
            gpio_int_unmask_set(SIM0_GPIO_DETECT);
            g_sci_debug.sci0_detect_low_err_cnt++;
            return ;
        }
    }
    else if (SIM_CARD_DETECT_HIGH == detect_level){
        msleep(WAIT_TIME);
        detect_level = gpio_get_value(SIM0_GPIO_DETECT);
        if((SIM_CARD_DETECT_HIGH == detect_level) && (SIM_CARD_DETECT_LOW == g_sci_stat.sci0_detect_level) &&
            (SIM_CARD_STAUTS_IN == g_sci_stat.sci0_card_satus)){
            g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;
            g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_OUT;
            g_sci_debug.sci0_detect_high_cnt++;
            gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_LOW);
            pSciInOut = SIM_CARD_STAUTS_OUT;
            channel_id = (ICC_CHN_IFC << MOV_BIT | IFC_RECV_FUNC_SIM0);
            u32Lenth = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&pSciInOut, sizeof(u32));
            if(u32Lenth != sizeof(u32)){
                gpio_int_unmask_set(SIM0_GPIO_DETECT);
                g_sci_debug.sci0_icc_high_err_cnt++;
                return ;
            }
        }
        else{
            gpio_int_unmask_set(SIM0_GPIO_DETECT);
            g_sci_debug.sci0_detect_low_err_cnt++;
            return ;
        }
    } 
#elif defined(BSP_CONFIG_BOARD_E5771S_852) || defined(BSP_CONFIG_BOARD_E5771H_937)
/* E5771s-852支持热拔但是不是使用pmuhpd中断，单独处理 */
    if (SIM_CARD_DETECT_LOW == detect_level)
    {   /*  拔出 */
        msleep(WAIT_TIME);
        gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_HIGH);
        detect_level = gpio_get_value(SIM0_GPIO_DETECT);
        if((SIM_CARD_DETECT_LOW == detect_level) && (SIM_CARD_DETECT_HIGH == g_sci_stat.sci0_detect_level) && 
            (SIM_CARD_STAUTS_IN == g_sci_stat.sci0_card_satus))
        {
            g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;
            g_sci_stat.sci0_card_satus   = SIM_CARD_STAUTS_OUT;
            g_sci_debug.sci0_detect_low_cnt++;
            /* send c core sim out */
            pSciInOut = SIM_CARD_STAUTS_OUT;
            channel_id = (ICC_CHN_IFC << MOV_BIT | IFC_RECV_FUNC_SIM0);
            u32Lenth = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&pSciInOut, sizeof(u32));
            if(u32Lenth != sizeof(u32))
            {
                gpio_int_unmask_set(SIM0_GPIO_DETECT);
                g_sci_debug.sci0_icc_low_err_cnt++;
                return ;
            }
        }
        else
        {
            gpio_int_unmask_set(SIM0_GPIO_DETECT);
            g_sci_debug.sci0_detect_low_err_cnt++;
            return ;
        }
    }
    else if (SIM_CARD_DETECT_HIGH == detect_level)
    {
        msleep(WAIT_TIME);
        gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_LOW);
        detect_level = gpio_get_value(SIM0_GPIO_DETECT);
        if((SIM_CARD_DETECT_HIGH == detect_level) && (SIM_CARD_DETECT_LOW == g_sci_stat.sci0_detect_level) &&
            (SIM_CARD_STAUTS_OUT == g_sci_stat.sci0_card_satus))
        {
            g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;
            g_sci_stat.sci0_card_satus = SIM_CARD_STAUTS_IN;
            g_sci_debug.sci0_detect_high_cnt++;
        }
        else
        {
            gpio_int_unmask_set(SIM0_GPIO_DETECT);
            g_sci_debug.sci0_detect_high_err_cnt++;
            return ;
        }
    }
#else
    if (SIM_CARD_DETECT_HIGH == detect_level)   /* insert */
    {
        /*·à??′|àí*/
    	msleep(5);

        /* set int trigger */
        gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_LOW); 

        detect_level = gpio_get_value(SIM0_GPIO_DETECT);
        if((SIM_CARD_DETECT_HIGH == detect_level)&&(SIM_CARD_DETECT_LOW == g_sci_stat.sci0_detect_level)&&
		(SIM_CARD_STAUTS_OUT == g_sci_stat.sci0_card_satus))
    	{
    		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;
    		g_sci_debug.sci0_detect_high_cnt ++ ;

    	}
        else
        {
            /* unmask */
			gpio_int_unmask_set(SIM0_GPIO_DETECT);
            g_sci_debug.sci0_detect_high_err_cnt ++ ;
            return ;

        }
	}
    else if (SIM_CARD_DETECT_LOW == detect_level)
    {   
        /*·à??′|àí*/
    	msleep(5);
		
        /* set int trigger */
    	gpio_int_trigger_set(SIM0_GPIO_DETECT, IRQ_TYPE_LEVEL_HIGH);
        
        detect_level = gpio_get_value(SIM0_GPIO_DETECT);
        if((SIM_CARD_DETECT_LOW == detect_level)&&(SIM_CARD_DETECT_HIGH == g_sci_stat.sci0_detect_level)&&
    		(SIM_CARD_STAUTS_IN == g_sci_stat.sci0_card_satus) && (TRUE == g_sci_stat.sci0_pmu_hpd))
    	{
    		/*add code here */
    		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;
    		g_sci_stat.sci0_card_satus   = SIM_CARD_STAUTS_OUT;
    		g_sci_stat.sci0_pmu_hpd = FALSE;
    		g_sci_debug.sci0_detect_low_cnt ++ ;
           
            /* send to ccpu */
            pSciInOut = SIM_CARD_STAUTS_OUT;
        	channel_id = (ICC_CHN_IFC<< 16 | IFC_RECV_FUNC_SIM0);
        	u32Lenth = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&pSciInOut, sizeof(u32));
        	if(u32Lenth != sizeof(u32))
        	{
        		/*SCI_PRINT("App: status is %d,send to modem failed! \n",pSciInOut);*/
                gpio_int_unmask_set(SIM0_GPIO_DETECT);

                g_sci_debug.sci0_icc_low_err_cnt ++;
                
        		return  ;
        	}

            g_sci_debug.sci0_icc_low_ok_cnt ++;
    	}
        else
        {
            /* unmask */
			gpio_int_unmask_set(SIM0_GPIO_DETECT);
            g_sci_debug.sci0_detect_low_err_cnt ++ ;
            return ;
        }
    } 
#endif
	else
	{	
		g_sci_debug.sci0_detect_err_cnt ++;
        gpio_int_unmask_set(SIM0_GPIO_DETECT);
        return ; 
	}

	gpio_int_unmask_set(SIM0_GPIO_DETECT);
	
    return ;
}

/*******************************************************************************
  Function:      sim_hpd_work_handler(struct work_struct *data)
  Description:   sim hpd handle work
  Input:         data:work_struct
                 devid:
  Output:        NA
  Return:        NA
  Others:        NA
*******************************************************************************/
static void sim_hpd_work_handler(struct work_struct *data)
{
    s32 u32Lenth;
	u32 pSciInOut;
	u32 channel_id = 0;

    msleep(200);   
    if(SIM_CARD_STAUTS_IN == g_sci_stat.sci0_card_satus)
    {
        /* send to ccpu */
        pSciInOut = SIM_CARD_STAUTS_IN;
    	channel_id = (ICC_CHN_IFC<< 16 | IFC_RECV_FUNC_SIM0);
    	u32Lenth = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&pSciInOut, sizeof(u32));
    	if(u32Lenth != sizeof(u32))
    	{
    		/*SCI_PRINT("App: status is %d,send to modem failed! \n",pSciInOut);*/
            g_sci_debug.sci0_icc_high_err_cnt ++;
            
    		return ;
    	}
        g_sci_debug.sci0_icc_high_ok_cnt ++;
    }
    else
    {
        g_sci_debug.sci0_hpd_err_cnt ++;
    }
    return ;
}

/*******************************************************************************
  Function:      sci_debug_cnt_print
  Description:   sd  detect interrpt handle
  Input:         irq: irq number
                 devid: device id
  Output:        irq handle result
  Return:        NA
  Others:        NA
*******************************************************************************/
void sci_debug_cnt_print(void)
{
	/* cnt */
	SCI_PRINT("sci0_init_cnt is %d \n",g_sci_debug.sci0_init_cnt);
	SCI_PRINT("sci0_init_request_cnt is %d \n",g_sci_debug.sci0_init_request_cnt);
	SCI_PRINT("sci0_detect_cnt is %d \n",g_sci_debug.sci0_detect_cnt);
	SCI_PRINT("sci0_detect_low_cnt is %d \n",g_sci_debug.sci0_detect_low_cnt);
	SCI_PRINT("sci0_detect_high_cnt is %d \n",g_sci_debug.sci0_detect_high_cnt);
    SCI_PRINT("sci0_pmu_hpd_in_enter_cnt is %d \n",g_sci_debug.sci0_pmu_hpd_in_enter_cnt);
	SCI_PRINT("sci0_pmu_hpd_in_cnt is %d \n",g_sci_debug.sci0_pmu_hpd_in_cnt);
    SCI_PRINT("sci0_pmu_hpd_out_enter_cnt is %d \n",g_sci_debug.sci0_pmu_hpd_out_enter_cnt);
	SCI_PRINT("sci0_pmu_hpd_out_cnt is %d \n",g_sci_debug.sci0_pmu_hpd_out_cnt);
    SCI_PRINT("sci0_icc_high_ok_cnt is %d \n",g_sci_debug.sci0_icc_high_ok_cnt);
	SCI_PRINT("sci0_icc_low_ok_cnt is %d \n",g_sci_debug.sci0_icc_low_ok_cnt);
    SCI_PRINT("sci0_init_err_cnt is %d \n",g_sci_debug.sci0_init_err_cnt);
    SCI_PRINT("sci0_detect_get_err_cnt is %d \n",g_sci_debug.sci0_detect_get_err_cnt);
    SCI_PRINT("sci0_detect_high_err_cnt is %d \n",g_sci_debug.sci0_detect_high_err_cnt);
    SCI_PRINT("sci0_detect_low_err_cnt is %d \n",g_sci_debug.sci0_detect_low_err_cnt);
    SCI_PRINT("sci0_pmu_hpd_in_err_cnt is %d \n",g_sci_debug.sci0_pmu_hpd_in_err_cnt);
    SCI_PRINT("sci0_pmu_hpd_out_err_cnt is %d \n",g_sci_debug.sci0_pmu_hpd_out_err_cnt);
    SCI_PRINT("sci0_icc_high_err_cnt is %d \n",g_sci_debug.sci0_icc_high_err_cnt);
    SCI_PRINT("sci0_icc_low_err_cnt is %d \n",g_sci_debug.sci0_icc_low_err_cnt);
    SCI_PRINT("sci0_detect_err_cnt is %d \n",g_sci_debug.sci0_detect_err_cnt);
	SCI_PRINT("sci0_hpd_err_cnt is %d \n",g_sci_debug.sci0_hpd_err_cnt);
}

/*******************************************************************************
  Function:      sci_debug_status_print
  Description:   
  Input:         irq: irq number
                 devid: device id
  Output:        irq handle result
  Return:        NA
  Others:        NA
*******************************************************************************/
void sci_debug_status_print(void)
{
	/* status */
	SCI_PRINT("sci_init_flag is %d \n",g_sci_stat.sci_init_flag);
	SCI_PRINT("sci0_card_satus is %d \n",g_sci_stat.sci0_card_satus);
	SCI_PRINT("sci0_detect_level is %d \n",g_sci_stat.sci0_detect_level);
	SCI_PRINT("sci0_pmu_hpd is %d \n",g_sci_stat.sci0_pmu_hpd);
}


/* init func */
module_init(bsp_sci_init); 
/* EXPORT_SYMBOL(bsp_vic_enable); */





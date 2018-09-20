
/*lint -save -e429 -e529 -e534 -e550 -e650 -e661 -e715 -e537  -e737 -e539 -e574 -e239 -e438 -e701 -e740 -e958 -e451
-e64 -e732 -e740 -e530 -e830 -e713
*/



#include <linux/amba/bus.h>
#include <linux/io.h> 
#include <linux/gfp.h>
#include <linux/clkdev.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/mman.h>

#include <asm/system.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/pgtable.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/io.h>
#include <asm/system.h>

#include <bsp_clk.h>
#include <bsp_hardtimer.h>

#include <bsp_ipf.h>

IPF_UL_S g_stIpfUl = {0};
IPF_DL_S g_stIpfDl = {0};
/* 调试信息结构体 */
IPF_DEBUG_INFO_S* g_stIPFDebugInfo = NULL;
BSP_U32 g_IPFInit;




	
static int ipf_probe(struct platform_device *pdev);
static int ipf_remove(struct platform_device *pdev);


static int ipf_dl_dpm_prepare(struct device *pdev);
static void ipf_dl_dpm_complete(struct device *pdev);
static int ipf_dl_suspend(struct device *dev);
static int ipf_dl_resume(struct device *dev);


static const struct dev_pm_ops balong_ipf_dev_pm_ops ={
	.prepare = ipf_dl_dpm_prepare,
	.suspend_noirq = ipf_dl_suspend,
	.resume_noirq = ipf_dl_resume,
	.complete = ipf_dl_dpm_complete,
};
#define BALONG_DEV_PM_OPS (&balong_ipf_dev_pm_ops)



/*C核启动(数传允许)标示，该标志用于Acore决定是否进行上行数传*/
IPF_FORREST_CONTROL_E g_eCcoreResetFlag = IPF_FORRESET_CONTROL_ALLOW;

static struct platform_driver balong_driver_ipf = {
	.probe		= ipf_probe,
	.remove		= ipf_remove,

	.driver		= {
		.name	= "balong_ipf_v700r200",
		.owner	= THIS_MODULE,
		.pm		= BALONG_DEV_PM_OPS
	},
};

struct platform_device balong_device_ipf = {
	.name		  = "balong_ipf_v700r200",
	.id                	  =0,
};





/* IPF transfer time recorder start */
#define IPF_MAX_STAMP_ORDER             32
#define IPF_MAX_TIME_LIMIT              (19200000*10)
#define IPF_FLS_MASK                    (31)


typedef struct tagIPF_TIMESTAMP_INFO_S
{
    unsigned int diff_order_cnt[IPF_MAX_STAMP_ORDER];
    unsigned int cnt_sum;
    unsigned int diff_sum;
    unsigned int diff_max;
    unsigned int overflow;
}IPF_TIMESTAMP_INFO_S;

IPF_TIMESTAMP_INFO_S g_ipf_dl_timestamp_info;
unsigned int g_ipf_ul_start_enable;
#define IPF_GET_TIMESTAMP_INFO() (&g_ipf_dl_timestamp_info)
#define IPF_START_STAMP_ENABLE (g_ipf_ul_start_enable)


/*
 * On ARMv5 and above those functions can be implemented around
 * the clz instruction.
 * refer to kernel/arch/arm/include/asm/bitops.h
 * put the code here for both vxWorks and linux version.
 */
static inline int ipf_fls(int x)
{
    int ret;

    asm("clz\t%0, %1" : "=r" (ret) : "r" (x));
    ret = 32 - ret;
    return ret;
}

static inline void ipf_record_start_time_stamp(BSP_U32 en, BSP_U32* rec_point)
{
    if (!en) {
        return;
    }
    *rec_point = bsp_get_slice_value_hrt();
    return;
}

static inline void
ipf_record_end_time_stamp(BSP_U32 en, BSP_U32 beg_time)
{
    unsigned int diff_time;
    IPF_TIMESTAMP_INFO_S* stamp_info = IPF_GET_TIMESTAMP_INFO();
    int idx;

    if (!en) {
        return;
    }

    diff_time = bsp_get_slice_value_hrt() - beg_time;

    /* avoid to record the overflowed value */
    if (diff_time > IPF_MAX_TIME_LIMIT) {
        stamp_info->overflow++;
    }
    else {
        if (diff_time > stamp_info->diff_max)
            stamp_info->diff_max = diff_time;

        stamp_info->diff_sum += diff_time;
        stamp_info->cnt_sum++;

        /* find the first bit not zero */
        idx = ((ipf_fls(diff_time)-1) & IPF_FLS_MASK);
        stamp_info->diff_order_cnt[idx]++;
    }
}

static inline
unsigned int ipf_calc_percent(unsigned int value, unsigned int sum)
{
    if (0 == sum) {
        return 0;
    }
    return (value * 100 / sum);
}

void ipf_enable_ul_time_stamp(int en)
{
    g_stIPFDebugInfo->ipf_timestamp_ul_en = en;
    return;
}

void ipf_enable_dl_time_stamp(int en)
{
    g_stIPFDebugInfo->ipf_timestamp_dl_en = en;
    return;
}

void ipf_clear_time_stamp(void)
{
    IPF_TIMESTAMP_INFO_S* stamp_info = IPF_GET_TIMESTAMP_INFO();

    memset(stamp_info, 0, sizeof(IPF_TIMESTAMP_INFO_S));/*[false alarm]:fortify*/
    return;
}

void ipf_dump_time_stamp(void)
{
    IPF_TIMESTAMP_INFO_S* stamp_info = IPF_GET_TIMESTAMP_INFO();
    unsigned int tmp = 0;
    int i;

    IPF_PRINT(" max diff:%u(%uus)\n",
              stamp_info->diff_max, stamp_info->diff_max*10/192);
    IPF_PRINT(" sum diff:%u(%uus)\n",
              stamp_info->diff_sum, stamp_info->diff_max/19);

    if (stamp_info->cnt_sum) {
        tmp = stamp_info->diff_sum / stamp_info->cnt_sum;
    }

    IPF_PRINT(" avg diff:%u(%uus)\n", tmp, tmp*10/192);

    for (i = 0; i < IPF_MAX_STAMP_ORDER; i++) {
        tmp = ipf_calc_percent(stamp_info->diff_order_cnt[i], stamp_info->cnt_sum);
        IPF_PRINT(" diff time (%u~%u) (%uus~%uus) count:%u (%u %%)\n",
            (0x80000000 >> (31-i)),
            (0xFFFFFFFF >> (31-i)),
            (0x80000000 >> (31-i))/19,
            (0xFFFFFFFF >> (31-i))/19,
            stamp_info->diff_order_cnt[i], tmp);
    }
    return;
}


/* IPF transfer time recorder end */


void ipf_write_basic_filter(u32 filter_hw_id, IPF_MATCH_INFO_S* match_infos)
{
    u32 j;
    u32 match_info;
    u32* match_info_addr = (u32*)match_infos;
    ipf_writel(filter_hw_id, HI_IPF_REGBASE_ADDR_VIRT + HI_BFLT_INDEX_OFFSET);
    for(j=0; j<(sizeof(IPF_MATCH_INFO_S)/4); j++)
    {
        match_info = *(match_info_addr+j);
        ipf_writel((match_info), (HI_IPF_REGBASE_ADDR_VIRT+HI_FLT_LOCAL_ADDR0_OFFSET+j*4)); 
    }
}

/*****************************************************************************
* 函 数 名      : ipf_init
*
* 功能描述  : IPF初始化     内部使用，不作为接口函数
*
* 输入参数  : BSP_VOID
* 输出参数  : 无
* 返 回 值     : IPF_SUCCESS    初始化成功
*                           IPF_ERROR      初始化失败
*
* 修改记录  :2011年1月21日   鲁婷  创建
				 2013年4月30日    陈东岳修改，将寄存器配置分配到两核 
*****************************************************************************/
BSP_S32 __init ipf_init(BSP_VOID)
{
    BSP_U32 u32BDSize[IPF_CHANNEL_MAX] = {IPF_ULBD_DESC_SIZE, IPF_DLBD_DESC_SIZE};
    BSP_U32 u32RDSize[IPF_CHANNEL_MAX] = {IPF_ULRD_DESC_SIZE, IPF_DLRD_DESC_SIZE};
    BSP_U32 u32ADCtrl[IPF_CHANNEL_MAX] = {IPF_ADQ_DEFAULT_SEETING,IPF_ADQ_DEFAULT_SEETING};

    BSP_S32 s32Ret = 0;


    /*ipf enable clock*/
    struct clk *c_IpfClk; 

    c_IpfClk = clk_get(NULL, "ipf_clk");
    if(IS_ERR(c_IpfClk))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"ipf_init: ipf clk get failed.\n");
        return BSP_ERROR;
    }

    if(BSP_OK !=clk_enable(c_IpfClk))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"ipf_init: ipf clk open failed.\n");
        return BSP_ERROR;
    }


    memset((BSP_VOID*)IPF_DLBD_MEM_ADDR, 0x0, IPF_DLDESC_SIZE);
	
    g_stIpfUl.pstIpfBDQ = (IPF_BD_DESC_S*)IPF_ULBD_MEM_ADDR;
    g_stIpfUl.pstIpfRDQ = (IPF_RD_DESC_S*)IPF_ULRD_MEM_ADDR;
    g_stIpfUl.pstIpfADQ0 = (IPF_AD_DESC_S*)IPF_ULAD0_MEM_ADDR;
    g_stIpfUl.pstIpfADQ1 = (IPF_AD_DESC_S*)IPF_ULAD1_MEM_ADDR;
    g_stIpfUl.pu32IdleBd = (BSP_U32*)IPF_ULBD_IDLENUM_ADDR;

    g_stIpfDl.pstIpfBDQ = (IPF_BD_DESC_S*)IPF_DLBD_MEM_ADDR;
    g_stIpfDl.pstIpfRDQ = (IPF_RD_DESC_S*)IPF_DLRD_MEM_ADDR;
    g_stIpfDl.pstIpfADQ0 = (IPF_AD_DESC_S*)IPF_DLAD0_MEM_ADDR;
    g_stIpfDl.pstIpfADQ1 = (IPF_AD_DESC_S*)IPF_DLAD1_MEM_ADDR;
    g_stIpfDl.pstIpfCDQ = (IPF_CD_DESC_S*)IPF_DLCD_MEM_ADDR;

    g_stIpfDl.pstIpfPhyBDQ = (IPF_BD_DESC_S*)(IPF_IO_ADDRESS_PHY(IPF_DLBD_MEM_ADDR));
    g_stIpfDl.pstIpfPhyRDQ = (IPF_RD_DESC_S*)(IPF_IO_ADDRESS_PHY(IPF_DLRD_MEM_ADDR));
    g_stIpfDl.pstIpfPhyADQ0 = (IPF_AD_DESC_S*)(IPF_IO_ADDRESS_PHY(IPF_DLAD0_MEM_ADDR));
    g_stIpfDl.pstIpfPhyADQ1 = (IPF_AD_DESC_S*)(IPF_IO_ADDRESS_PHY(IPF_DLAD1_MEM_ADDR));
	
    g_stIpfDl.u32IpfCdRptr = (BSP_U32*) IPF_DLCDRPTR_MEM_ADDR;
    *(g_stIpfDl.u32IpfCdRptr) = 0;

	

    g_stIPFDebugInfo = (IPF_DEBUG_INFO_S*)IPF_DEBUG_INFO_ADDR;
	
    /* 配置下行通道的AD、BD和RD深度 */
    ipf_writel(u32BDSize[IPF_CHANNEL_DOWN]-1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_SIZE_OFFSET);
    ipf_writel(u32RDSize[IPF_CHANNEL_DOWN]-1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_SIZE_OFFSET);
    ipf_writel(u32ADCtrl[IPF_CHANNEL_DOWN], HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ_CTRL_OFFSET);

    /*下行通道的BD和RD起始地址*/    
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyBDQ, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_BADDR_OFFSET);
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyRDQ, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_BADDR_OFFSET);
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyADQ0, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_BASE_OFFSET);
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyADQ1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_BASE_OFFSET);

    IPF_Int_Connect();

    s32Ret = platform_device_register(&balong_device_ipf);
    if(s32Ret)
    {

        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,
                        "Platform device register failed! \n");

        return s32Ret;
    }
    s32Ret = platform_driver_register(&balong_driver_ipf);
    if(s32Ret)
    {

        platform_device_unregister(&balong_device_ipf);

        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,
                        "Platform driver register failed! \n");
        return s32Ret;
    }


    g_IPFInit = IPF_ACORE_INIT_SUCCESS;

    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"[IPF]  ipf init success\n");

    return s32Ret;
}

/*****************************************************************************
* 函 数 名     : IPF_Int_Connect
*
* 功能描述  : 挂IPF中断处理函数(两核都提供)
*
* 输入参数  : BSP_VOID
* 输出参数  : 无
* 返 回 值      : 无
*
* 修改记录  :2011年12月2日   鲁婷  创建
*****************************************************************************/
BSP_VOID IPF_Int_Connect(BSP_VOID)
{
	BSP_S32 s32Result;
	s32Result = request_irq(INT_LVL_IPF, (irq_handler_t)IPF_IntHandler, IRQF_NO_SUSPEND, "balong_ipf_v700r200", NULL);
	if(0 != s32Result)
	{
            bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF_Int_Connect Failed! \n",0,0,0,0,0,0);
	}
}

/*****************************************************************************
* 函 数 名  : IPF_IntHandler
*
* 功能描述  : IPF中断处理函数
*
* 输入参数  : 无
* 输出参数  : 无
* 返 回 值  : 无
*
* 修改记录  :2011年1月24日   鲁婷  创建


需要改动，增加ADQ空中段的处理函数。未完
*****************************************************************************/
irqreturn_t  IPF_IntHandler (int irq, void* dev)
{
    BSP_U32 u32IpfInt = 0;

    /* 读取中断状态 */

    u32IpfInt = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_INT1_OFFSET); 

    /* 下行结果上报和结果超时上报 */
    if(u32IpfInt&(IPF_DL_RPT_INT1|IPF_DL_TIMEOUT_INT1))
    {
        ipf_writel((IPF_DL_RPT_INT1|IPF_DL_TIMEOUT_INT1), HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_INT_STATE_OFFSET);

        g_stIPFDebugInfo->ipf_dlbd_done_count++;

        /* 唤醒ps下行任务 */
        if(g_stIpfDl.pFnDlIntCb != NULL)
        {
            (BSP_VOID)g_stIpfDl.pFnDlIntCb();  
        }
        else
        {
            bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF_IntHandler DLTASK NULL! \n",0,0,0,0,0,0);
	 }
    } 
     /* 下行ADQ0、ADQ1都空指示 */	
    if((IPF_DL_ADQ0_EPTY_INT1 | IPF_DL_ADQ1_EPTY_INT1) == (u32IpfInt & (IPF_DL_ADQ0_EPTY_INT1 | IPF_DL_ADQ1_EPTY_INT1)))
    {
        g_stIPFDebugInfo->u32DlAdq0Overflow++;
        g_stIPFDebugInfo->u32DlAdq1Overflow++;
        ipf_writel((IPF_DL_ADQ0_EPTY_INT1 | IPF_DL_ADQ1_EPTY_INT1), HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_INT_STATE_OFFSET);
        if(g_stIpfDl.pAdqEmptyDlCb != NULL)
        {
            (BSP_VOID)g_stIpfDl.pAdqEmptyDlCb(IPF_EMPTY_ADQ);  
        }
        else
        {
            bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF_IntHandler DLADQEMPTY NULL! \n",0,0,0,0,0,0);
        }
        return IRQ_HANDLED;
    }

    /* 下行ADQ0空指示 */
    if(u32IpfInt & IPF_DL_ADQ0_EPTY_INT1)
    {
        g_stIPFDebugInfo->u32DlAdq0Overflow++;
        ipf_writel(IPF_DL_ADQ0_EPTY_INT1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_INT_STATE_OFFSET);
        if(g_stIpfDl.pAdqEmptyDlCb != NULL)
        {
            (BSP_VOID)g_stIpfDl.pAdqEmptyDlCb(IPF_EMPTY_ADQ0);  
        }
        else
        {
            bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF_IntHandler DLADQEMPTY NULL! \n",0,0,0,0,0,0);
        }
    }
	
    /* 下行ADQ1空指示 */	
    if(u32IpfInt & IPF_DL_ADQ1_EPTY_INT1)
    {
        g_stIPFDebugInfo->u32DlAdq1Overflow++;
        ipf_writel(IPF_DL_ADQ1_EPTY_INT1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_INT_STATE_OFFSET);
        if(g_stIpfDl.pAdqEmptyDlCb != NULL)
        {
            (BSP_VOID)g_stIpfDl.pAdqEmptyDlCb(IPF_EMPTY_ADQ1);  
        }
        else
        {
            bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF_IntHandler DLADQEMPTY NULL! \n",0,0,0,0,0,0);
        }
    }
    return IRQ_HANDLED;
}/*lint !e550*/



/*****************************************************************************
* 函 数 名      : BSP_IPF_ConfigTimeout
*
* 功能描述  : 调试使用，配置超时时间接口
*
* 输入参数  : BSP_U32 u32Timeout 配置的超时时间
* 输出参数  : 无
* 返 回 值     : IPF_SUCCESS    成功
*                           BSP_ERR_IPF_INVALID_PARA      参数无效
*
* 说明              : 1代表256个时钟周期
*
* 修改记录   : 2011年11月30日   鲁婷  创建
*****************************************************************************/
BSP_S32 BSP_IPF_ConfigTimeout(BSP_U32 u32Timeout)
{
    if((u32Timeout == 0) || (u32Timeout > 0xFFFF))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigTimeout u32Timeout ERROR! \n");
        return BSP_ERR_IPF_INVALID_PARA;
    }

    u32Timeout |= TIME_OUT_ENABLE;
    ipf_writel(u32Timeout, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_TIME_OUT_OFFSET);
    
    return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名      : BSP_IPF_PktLen
*
* 功能描述  : 该接口用来配置过滤器的最大和最小包长
*
* 输入参数  : BSP_U32 MaxLen   最大包长
*                           BSP_U32 MinLen   最小包长
*
* 输出参数   : 无
* 返 回 值      : IPF_SUCCESS                成功
*                           BSP_ERR_IPF_INVALID_PARA   参数错误(最大包长比最小包长小)
*
* 修改记录  :2011年2月17日   鲁婷  创建
*****************************************************************************/
BSP_S32 BSP_IPF_SetPktLen(BSP_U32 u32MaxLen, BSP_U32 u32MinLen)
{
    BSP_U32 u32PktLen = 0;

    /* 参数检查 */
    if(u32MaxLen < u32MinLen)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF, "\r BSP_IPF_PktLen input error! \n");
        return BSP_ERR_IPF_INVALID_PARA;
    }
    
    u32PktLen = ((u32MaxLen&0x3FFF)<<16) | (u32MinLen&0x3FFF);
    
    ipf_writel(u32PktLen, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_PKT_LEN_OFFSET);
    return IPF_SUCCESS;
}/*lint !e550*/

/**********************************可维可测接口函数************************************/
BSP_VOID BSP_IPF_Help(BSP_VOID)
{
    IPF_PRINT("===============================================\n");
    IPF_PRINT("BSP_IPF_Info    参数1:通道类型  0为上行，1为下行\n");
    IPF_PRINT("BSP_IPF_Shared_DDR_Info \n");
    IPF_PRINT("BSP_IPF_BDInfo  参数1:通道类型  参数2:BD指针\n");
    IPF_PRINT("BSP_IPF_RDInfo  参数1:通道类型  参数2:RD指针\n"); 
    IPF_PRINT("BSP_IPF_ADInfo  参数1:通道类型  参数2:AD指针参数3:AD 队列类型0为短,1为长\n"); 
    IPF_PRINT("===============================================\n");
    IPF_PRINT("BSP_IPF_Dump_BDInfo  参数1:通道类型\n");
    IPF_PRINT("BSP_IPF_Dump_RDInfo  参数1:通道类型\n"); 
    IPF_PRINT("BSP_IPF_Dump_ADInfo  参数1:通道类型\n"); 
    IPF_PRINT("===============================================\n");
    IPF_PRINT("ipf_enable_ul_time_stamp  参数1:0-disable, 1-enable\n");
    IPF_PRINT("ipf_enable_dl_time_stamp  参数1:0-disable, 1-enable\n");
    IPF_PRINT("ipf_clear_time_stamp  清除实际戳记录\n");
    IPF_PRINT("ipf_dump_time_stamp  Linux:下行时间差, vxWorks:上行时间差\n");

    IPF_PRINT("===============================================\n");
    IPF_PRINT("上行配置BD,BD不够用次数(仅当调用GetDesc时有效):            %d\n",g_stIPFDebugInfo->u32UlBdNotEnough);
    IPF_PRINT("上行配置BD,AD0不够用次数(仅当调用GetDesc时有效):            %d\n",g_stIPFDebugInfo->u32UlAd0NotEnough);
    IPF_PRINT("上行配置BD,AD1不够用次数(仅当调用GetDesc时有效):            %d\n",g_stIPFDebugInfo->u32UlAd1NotEnough);
    IPF_PRINT("下行配置BD,BD不够用次数(仅当调用GetDesc时有效):            %d\n",g_stIPFDebugInfo->u32DlBdNotEnough);
    IPF_PRINT("下行配置BD,AD0不够用次数(仅当调用GetDesc时有效):            %d\n",g_stIPFDebugInfo->u32DlAd0NotEnough);
    IPF_PRINT("下行配置BD,AD1不够用次数(仅当调用GetDesc时有效):            %d\n",g_stIPFDebugInfo->u32DlAd1NotEnough);
    IPF_PRINT("下行配置CD,CD不够用次数:            %d\n",g_stIPFDebugInfo->u32DlCdNotEnough);
    IPF_PRINT("中断上报上行BD队列溢出次数:         %d\n",g_stIPFDebugInfo->u32UlBdqOverflow);
    IPF_PRINT("中断上报下行BD队列溢出次数:         %d\n",g_stIPFDebugInfo->u32DlBdqOverflow);
    IPF_PRINT("===============================================\n");
    IPF_PRINT("上行方向给BD长度配置为0次数:            %u\n",g_stIPFDebugInfo->ipf_ulbd_len_zero_count);
    IPF_PRINT("上行方向给AD0配置错误次数:            %u\n",g_stIPFDebugInfo->ipf_ulbd_len_zero_count);
    IPF_PRINT("上行方向给AD1配置错误次数:            %u\n",g_stIPFDebugInfo->ipf_ulbd_len_zero_count);
    IPF_PRINT("下行方向给BD长度配置为0次数:            %u\n",g_stIPFDebugInfo->ipf_dlbd_len_zero_count);
    IPF_PRINT("下行方向给AD0配置错误次数:            %u\n",g_stIPFDebugInfo->ipf_dlbd_len_zero_count);
    IPF_PRINT("下行方向给AD1配置错误次数:            %u\n",g_stIPFDebugInfo->ipf_dlbd_len_zero_count);
    IPF_PRINT("acore尝试在ccore复位时数传次数:            %u\n",g_stIPFDebugInfo->ipf_acore_not_init_count);
    IPF_PRINT("ccore尝试在ccore复位时数传次数:            %u\n",g_stIPFDebugInfo->ipf_ccore_not_init_count);
    IPF_PRINT("===============================================\n");
    IPF_PRINT("上行时间戳功能使能:                  %u\n",g_stIPFDebugInfo->ipf_timestamp_ul_en);
    IPF_PRINT("上行BD配置次数:                      %u\n",g_stIPFDebugInfo->ipf_cfg_ulbd_count);
    IPF_PRINT("上行BD完成中断次数:                  %u\n",g_stIPFDebugInfo->ipf_ulbd_done_count);
    IPF_PRINT("上行RD获取次数:                      %u\n",g_stIPFDebugInfo->ipf_get_ulrd_count);
    IPF_PRINT("上行AD0配置次数:                     %u\n",g_stIPFDebugInfo->ipf_cfg_ulad0_count);
    IPF_PRINT("上行AD1配置次数:                     %u\n",g_stIPFDebugInfo->ipf_cfg_ulad1_count);
    IPF_PRINT("CCore suspend次数:                   %u\n",g_stIPFDebugInfo->ipf_ccore_suspend_count);
    IPF_PRINT("CCore resume次数:                    %u\n",g_stIPFDebugInfo->ipf_ccore_resume_count);
    IPF_PRINT("===============================================\n");
    IPF_PRINT("下行时间戳功能使能:                  %u\n",g_stIPFDebugInfo->ipf_timestamp_dl_en);
    IPF_PRINT("下行BD配置次数:                      %u\n",g_stIPFDebugInfo->ipf_cfg_dlbd_count);
    IPF_PRINT("下行BD完成中断次数:                  %u\n",g_stIPFDebugInfo->ipf_dlbd_done_count);
    IPF_PRINT("下行RD获取次数:                      %u\n",g_stIPFDebugInfo->ipf_get_dlrd_count);
    IPF_PRINT("下行AD0配置次数:                     %u\n",g_stIPFDebugInfo->ipf_cfg_dlad0_count);
    IPF_PRINT("下行AD1配置次数:                     %u\n",g_stIPFDebugInfo->ipf_cfg_dlad1_count);
    IPF_PRINT("ACore suspend次数:                   %u\n",g_stIPFDebugInfo->ipf_acore_suspend_count);
    IPF_PRINT("ACore resume次数:                    %u\n",g_stIPFDebugInfo->ipf_acore_resume_count);
}

BSP_S32 BSP_IPF_Shared_DDR_Info(BSP_VOID)
{
/*	BSP_U32* pIPFInit = (BSP_U32*)IPF_INIT_ADDR;*/
	BSP_U32 ipf_Shared_ddr_start = SHM_MEM_IPF_ADDR;
	
	BSP_U32 ipf_Shared_ddr_ul_start = IPF_ULBD_MEM_ADDR;

	BSP_U32 ipf_Shared_ddr_filter_pwc_start = IPF_PWRCTL_BASIC_FILTER_ADDR;

	BSP_U32 ipf_Shared_ddr_pwc_info_start = IPF_PWRCTL_INFO_ADDR;

	BSP_U32 ipf_Shared_ddr_dlcdrptr = IPF_DLCDRPTR_MEM_ADDR;

	BSP_U32 ipf_Shared_ddr_debug_dlcd_start = IPF_DEBUG_DLCD_ADDR;
	BSP_U32 ipf_Shared_ddr_debug_dlcd_size = IPF_DEBUG_DLCD_SIZE;
	BSP_U32 ipf_Shared_ddr_end = IPF_DEBUG_INFO_END_ADDR;
	BSP_U32 ipf_Shared_ddr_len = IPF_DEBUG_INFO_END_ADDR - SHM_MEM_IPF_ADDR;
	
	IPF_PRINT("ipf_Shared_ddr_start                    value is 0x%x \n", ipf_Shared_ddr_start);
	IPF_PRINT("ipf_Shared_ddr_ul_start                value is 0x%x \n", ipf_Shared_ddr_ul_start);
	IPF_PRINT("ipf_Shared_ddr_filter_pwc_start     value is 0x%x \n", ipf_Shared_ddr_filter_pwc_start);
	IPF_PRINT("ipf_Shared_ddr_pwc_info_start      value is 0x%x \n", ipf_Shared_ddr_pwc_info_start);
	IPF_PRINT("ipf_Shared_ddr_dlcdrptr                value is 0x%x \n", ipf_Shared_ddr_dlcdrptr);
	IPF_PRINT("ipf_Shared_ddr_debug_dlcd_start   value is 0x%x \n", ipf_Shared_ddr_debug_dlcd_start);
	IPF_PRINT("ipf_Shared_ddr_debug_dlcd_size    value is 0x%x \n", ipf_Shared_ddr_debug_dlcd_size);
	IPF_PRINT("ipf_Shared_ddr_end                     value is 0x%x \n", ipf_Shared_ddr_end);
	IPF_PRINT("ipf_Shared_ddr_len                     value is 0x%x (Max len is 0x10000)\n", ipf_Shared_ddr_len);
/*	if(IPF_INIT_SUCCESS != (*pIPFInit))
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF Not Init! \n");
		IPF_PRINT("g_pIPFInit value is 0x%x \n", (*pIPFInit));
		return BSP_ERR_IPF_NOT_INIT;
	}
*/
	return IPF_SUCCESS;
}

BSP_S32 BSP_IPF_BDInfo(IPF_CHANNEL_TYPE_E eChnType, BSP_U32 u32BdqPtr)
{
      
    switch(eChnType)
    {
        case IPF_CHANNEL_UP:
                if(u32BdqPtr >= IPF_ULBD_DESC_SIZE)
                {
                    return IPF_ERROR;
                }
                IPF_PRINT("==========BD Info=========\n");
                IPF_PRINT("BD位置:         %d\n",u32BdqPtr);
                IPF_PRINT("u16Attribute:   %d\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u16Attribute);
                IPF_PRINT("u16PktLen:      %d\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u16PktLen);
                IPF_PRINT("u32InPtr:       0x%x\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u32InPtr);
                IPF_PRINT("u32OutPtr:      0x%x\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u32OutPtr);
                IPF_PRINT("u16Resv:        %d\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u16Resv);
                IPF_PRINT("u16UsrField1:   %d\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u16UsrField1);
                IPF_PRINT("u32UsrField2:   0x%x\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u32UsrField2);
                IPF_PRINT("u32UsrField3:   0x%x\n",g_stIpfUl.pstIpfBDQ[u32BdqPtr].u32UsrField3);
            break;
       case IPF_CHANNEL_DOWN:
                if(u32BdqPtr >= IPF_DLBD_DESC_SIZE)
                {
                    return IPF_ERROR;
                }
                IPF_PRINT("==========BD Info=========\n");
                IPF_PRINT("BD位置:         %d\n",u32BdqPtr);
                IPF_PRINT("u16Attribute:   %d\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u16Attribute);
                IPF_PRINT("u16PktLen:      %d\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u16PktLen);
                IPF_PRINT("u32InPtr:       0x%x\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u32InPtr);
                IPF_PRINT("u32OutPtr:      0x%x\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u32OutPtr);
                IPF_PRINT("u16Resv:        %d\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u16Resv);
                IPF_PRINT("u16UsrField1:   %d\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u16UsrField1);
                IPF_PRINT("u32UsrField2:   0x%x\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u32UsrField2);
                IPF_PRINT("u32UsrField3:   0x%x\n",g_stIpfDl.pstIpfBDQ[u32BdqPtr].u32UsrField3);
            break; 
        default:
            break;
    }
    IPF_PRINT("************************\n");
    return 0;
}

BSP_S32 BSP_IPF_Dump_BDInfo(IPF_CHANNEL_TYPE_E eChnType)
{
    BSP_U32 i;
    switch(eChnType)
    {
        case IPF_CHANNEL_UP:
			
            for(i = 0;i < IPF_ULBD_DESC_SIZE;i++)
            {
                BSP_IPF_BDInfo(IPF_CHANNEL_UP,i);
            }
        break;
		
        case IPF_CHANNEL_DOWN:

            for(i = 0;i < IPF_DLBD_DESC_SIZE;i++)
            {
                BSP_IPF_BDInfo(IPF_CHANNEL_DOWN,i);
            }
        break;
				
        default:
        IPF_PRINT("Input param invalid ! \n");
        break;

    }
    return 0;
}

BSP_S32 BSP_IPF_RDInfo(IPF_CHANNEL_TYPE_E eChnType, BSP_U32 u32RdqPtr)
{     
    switch(eChnType)
    {
        case IPF_CHANNEL_UP:
            if(u32RdqPtr >= IPF_ULRD_DESC_SIZE)
            {
                return IPF_ERROR;
            }
            IPF_PRINT("===========RD Info==========\n");
            IPF_PRINT("RD位置:             %d\n",u32RdqPtr);
            IPF_PRINT("u16Attribute:       %d\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u16Attribute);
            IPF_PRINT("u16PktLen:          %d\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u16PktLen);
            IPF_PRINT("u32InPtr:           0x%x\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u32InPtr);
            IPF_PRINT("u32OutPtr:          0x%x\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u32OutPtr);
            IPF_PRINT("u16Result:          0x%x\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u16Result);
            IPF_PRINT("u16UsrField1:       0x%x\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u16UsrField1);
            IPF_PRINT("u32UsrField2:       0x%x\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u32UsrField2);
            IPF_PRINT("u32UsrField3:       0x%x\n",g_stIpfUl.pstIpfRDQ[u32RdqPtr].u32UsrField3);
            break;
       case IPF_CHANNEL_DOWN:
            if(u32RdqPtr >= IPF_DLRD_DESC_SIZE)
            {
                return IPF_ERROR;
            }
            IPF_PRINT("============RD Info===========\n");
            IPF_PRINT("RD位置:             %d\n",u32RdqPtr);
            IPF_PRINT("u16Attribute:       %d\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u16Attribute);
            IPF_PRINT("u16PktLen:          %d\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u16PktLen);
            IPF_PRINT("u32InPtr:           0x%x\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u32InPtr);
            IPF_PRINT("u32OutPtr:          0x%x\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u32OutPtr);
            IPF_PRINT("u16Result:          0x%x\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u16Result);
            IPF_PRINT("u16UsrField1:       0x%x\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u16UsrField1);
            IPF_PRINT("u32UsrField2:       0x%x\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u32UsrField2);
            IPF_PRINT("u32UsrField3:       0x%x\n",g_stIpfDl.pstIpfRDQ[u32RdqPtr].u32UsrField3);
            break; 
        default:
            break;
    }
    IPF_PRINT("************************\n");
    return 0;
}


BSP_S32 BSP_IPF_Dump_RDInfo(IPF_CHANNEL_TYPE_E eChnType)
{
    int i;

    switch(eChnType)
    {
        case IPF_CHANNEL_UP:

            for(i = 0;i < IPF_ULBD_DESC_SIZE;i++)
            {
                BSP_IPF_RDInfo(IPF_CHANNEL_UP,i);
            }
        break;
		
        case IPF_CHANNEL_DOWN:

            for(i = 0;i < IPF_DLBD_DESC_SIZE;i++)
            {
                BSP_IPF_RDInfo(IPF_CHANNEL_DOWN,i);
            }
        break;
        default:
        IPF_PRINT("Input param invalid ! \n");
        break;
    }
    return 0;
}
BSP_S32 BSP_IPF_ADInfo(IPF_CHANNEL_TYPE_E eChnType, BSP_U32 u32AdqPtr, BSP_U32 u32AdType)
{

    switch(eChnType)
    {
        case IPF_CHANNEL_UP:
            if(u32AdqPtr >= IPF_ULAD0_DESC_SIZE)
            {
                return IPF_ERROR;
            }
            if(0 == u32AdType)
            {
                 IPF_PRINT("===========UL AD0 Info==========\n");
                 IPF_PRINT("AD位置:             %d\n",u32AdqPtr);
                 IPF_PRINT("u32OutPtr0(phy_addr, use by hardware):       0x%x\n",g_stIpfUl.pstIpfADQ0[u32AdqPtr].u32OutPtr0);
                 IPF_PRINT("u32OutPtr1(usrfield skb_addr default):          0x%x\n",g_stIpfUl.pstIpfADQ0[u32AdqPtr].u32OutPtr1);
            }
            else
            {
                 IPF_PRINT("===========UL AD1 Info==========\n");
                 IPF_PRINT("AD位置:             %d\n",u32AdqPtr);
                 IPF_PRINT("u32OutPtr0(phy_addr, use by hardware):       0x%x\n",g_stIpfUl.pstIpfADQ1[u32AdqPtr].u32OutPtr0);
                 IPF_PRINT("u32OutPtr1(usrfield skb_addr default):          0x%x\n",g_stIpfUl.pstIpfADQ1[u32AdqPtr].u32OutPtr1);
            }
            break;
       case IPF_CHANNEL_DOWN:
            if(u32AdqPtr >= IPF_ULAD1_DESC_SIZE)
            {
                return IPF_ERROR;
            }
            if(0 == u32AdType)
	      	{
                 IPF_PRINT("===========DL AD0 Info==========\n");
                 IPF_PRINT("AD位置:             %d\n",u32AdqPtr);
                 IPF_PRINT("u32OutPtr0(phy_addr, use by hardware):       0x%x\n",g_stIpfDl.pstIpfADQ0[u32AdqPtr].u32OutPtr0);
                 IPF_PRINT("u32OutPtr1(usrfield skb_addr default):          0x%x\n",g_stIpfDl.pstIpfADQ0[u32AdqPtr].u32OutPtr1);
            }
            else
            {
                 IPF_PRINT("===========DL AD1 Info==========\n");
                 IPF_PRINT("AD位置:             %d\n",u32AdqPtr);
                 IPF_PRINT("u32OutPtr0(phy_addr, use by hardware):       0x%x\n",g_stIpfDl.pstIpfADQ1[u32AdqPtr].u32OutPtr0);
                 IPF_PRINT("u32OutPtr1(usrfield skb_addr default):          0x%x\n",g_stIpfDl.pstIpfADQ1[u32AdqPtr].u32OutPtr1);
            }
            break;
        default:
            break;
    }
    IPF_PRINT("************************\n");
    return 0;
}


BSP_S32 BSP_IPF_Dump_ADInfo(IPF_CHANNEL_TYPE_E eChnType, BSP_U32 u32AdType)
{
    int i;

    switch(eChnType)
    {
        case IPF_CHANNEL_UP:
            for(i = 0;i < IPF_ULAD0_DESC_SIZE;i++)
            {
                BSP_IPF_ADInfo(IPF_CHANNEL_UP, i, u32AdType);
            }
        break;
		
        case IPF_CHANNEL_DOWN:

            for(i = 0;i < IPF_DLAD0_DESC_SIZE;i++)
            {
                BSP_IPF_ADInfo(IPF_CHANNEL_DOWN, i, u32AdType);
            }
        break;
		
        default:
        IPF_PRINT("Input param invalid ! \n");
        break;
    }
    return 0;
}

BSP_S32 BSP_IPF_Info(IPF_CHANNEL_TYPE_E eChnType)
{
    BSP_U32 u32BdqDepth = 0;
    BSP_U32 u32BdqWptr = 0;
    BSP_U32 u32BdqRptr = 0;
    BSP_U32 u32BdqWaddr = 0;
    BSP_U32 u32BdqRaddr = 0;
    BSP_U32 u32RdqDepth = 0;
    BSP_U32 u32RdqRptr = 0;
    BSP_U32 u32RdqWptr = 0;
    BSP_U32 u32RdqWaddr = 0;
    BSP_U32 u32RdqRaddr = 0;
    BSP_U32 u32Depth = 0;
    BSP_U32 u32status = 0;
	
    BSP_U32 u32Adq0Rptr = 0;
    BSP_U32 u32Adq0Wptr = 0;

    BSP_U32 u32Adq1Rptr = 0;
    BSP_U32 u32Adq1Wptr = 0;

    if(IPF_CHANNEL_UP == eChnType)
    {
        u32Depth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_DQ_DEPTH_OFFSET);
        u32RdqDepth = (u32Depth>>16)&IPF_DQ_DEPTH_MASK;
        u32BdqDepth = u32Depth&IPF_DQ_DEPTH_MASK;

        u32status = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_STATE_OFFSET);

        u32BdqWptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_BDQ_WPTR_OFFSET);
        u32BdqRptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_BDQ_RPTR_OFFSET); 
        u32BdqWaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_BDQ_WADDR_OFFSET); 
        u32BdqRaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_BDQ_RADDR_OFFSET); 
    
        u32RdqWptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_RDQ_WPTR_OFFSET);
        u32RdqRptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_RDQ_RPTR_OFFSET);
        u32RdqWaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_RDQ_WADDR_OFFSET); 
        u32RdqRaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_RDQ_RADDR_OFFSET); 
		
        u32Adq0Rptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ0_RPTR_OFFSET);
        u32Adq0Wptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ0_WPTR_OFFSET);

        u32Adq1Rptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ1_RPTR_OFFSET);
        u32Adq1Wptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ1_WPTR_OFFSET);

    }
    else if(IPF_CHANNEL_DOWN == eChnType)
    {
        u32Depth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_DQ_DEPTH_OFFSET);
        u32RdqDepth = (u32Depth>>16)&IPF_DQ_DEPTH_MASK;
        u32BdqDepth = u32Depth&IPF_DQ_DEPTH_MASK;

        u32status = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_STATE_OFFSET);
		
        u32BdqWptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_WPTR_OFFSET);
        u32BdqRptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_RPTR_OFFSET); 
        u32BdqWaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_WADDR_OFFSET); 
        u32BdqRaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_RADDR_OFFSET); 
    
        u32RdqWptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_WPTR_OFFSET);
        u32RdqRptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_RPTR_OFFSET);
        u32RdqWaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_WADDR_OFFSET); 
        u32RdqRaddr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_RADDR_OFFSET); 

        u32Adq0Rptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_RPTR_OFFSET);
        u32Adq0Wptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_WPTR_OFFSET);

        u32Adq1Rptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_RPTR_OFFSET);
        u32Adq1Wptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_WPTR_OFFSET);

    }
    else
    {
        return 1;
    }
    IPF_PRINT("============================\n");
    IPF_PRINT("通道 状态:            0x%x\n", u32status);
    IPF_PRINT("BD 深度:            %d\n", u32BdqDepth);
    IPF_PRINT("BD 写指针:          %d\n", u32BdqWptr);
    IPF_PRINT("BD 读指针:          %d\n", u32BdqRptr);
    IPF_PRINT("BD 写地址:          0x%x\n", u32BdqWaddr);
    IPF_PRINT("BD 读地址:          0x%x\n", u32BdqRaddr);
    IPF_PRINT("RD 深度:            %d\n", u32RdqDepth);
    IPF_PRINT("RD 读指针:          %d\n", u32RdqRptr);
    IPF_PRINT("RD 写指针:          %d\n", u32RdqWptr);
    IPF_PRINT("RD 读地址:          0x%x\n", u32RdqRaddr);
    IPF_PRINT("RD 写地址:          0x%x\n", u32RdqWaddr);

    IPF_PRINT("AD0 读指针:          %d\n", u32Adq0Rptr);
    IPF_PRINT("AD0 写指针:          %d\n", u32Adq0Wptr);
	
    IPF_PRINT("AD1 读指针:          %d\n", u32Adq1Rptr);
    IPF_PRINT("AD1 写指针:          %d\n", u32Adq1Wptr);
    IPF_PRINT("============================\n");   
    return 0;
}

BSP_VOID BSP_IPF_MEM(BSP_VOID)
{
	BSP_U32 ipf_Shared_ddr_start = SHM_MEM_IPF_ADDR;
	
	BSP_U32 ipf_Shared_ddr_ul_start = IPF_ULBD_MEM_ADDR;

	BSP_U32 ipf_Shared_ddr_filter_pwc_start = IPF_PWRCTL_BASIC_FILTER_ADDR;

	BSP_U32 ipf_Shared_ddr_pwc_info_start = IPF_PWRCTL_INFO_ADDR;

	BSP_U32 ipf_Shared_ddr_dlcdrptr = IPF_DLCDRPTR_MEM_ADDR;

	BSP_U32 ipf_Shared_ddr_debug_dlcd_start = IPF_DEBUG_DLCD_ADDR;
	BSP_U32 ipf_Shared_ddr_debug_dlcd_size = IPF_DEBUG_DLCD_SIZE;
	BSP_U32 ipf_Shared_ddr_end = IPF_DEBUG_INFO_END_ADDR;

	
	IPF_PRINT("ipf_Shared_ddr_start                    value is 0x%x \n", ipf_Shared_ddr_start);
	IPF_PRINT("ipf_Shared_ddr_ul_start                value is 0x%x \n", ipf_Shared_ddr_ul_start);
	IPF_PRINT("ipf_Shared_ddr_filter_pwc_start     value is 0x%x \n", ipf_Shared_ddr_filter_pwc_start);
	IPF_PRINT("ipf_Shared_ddr_pwc_info_start      value is 0x%x \n", ipf_Shared_ddr_pwc_info_start);
	IPF_PRINT("ipf_Shared_ddr_dlcdrptr                value is 0x%x \n", ipf_Shared_ddr_dlcdrptr);
	IPF_PRINT("ipf_Shared_ddr_debug_dlcd_start   value is 0x%x \n", ipf_Shared_ddr_debug_dlcd_start);
	IPF_PRINT("ipf_Shared_ddr_debug_dlcd_size    value is 0x%x \n", ipf_Shared_ddr_debug_dlcd_size);
	IPF_PRINT("ipf_Shared_ddr_end                     value is 0x%x \n", ipf_Shared_ddr_end);
	IPF_PRINT("ipf_Shared_ddr_total_size             value is 0x%x \n", (ipf_Shared_ddr_end-ipf_Shared_ddr_start));


    IPF_PRINT("=======================================\n");
    IPF_PRINT("   BSP_IPF_MEM          ADDR            SIZE\n");
    IPF_PRINT("IPF_ULBD_MEM_ADDR    0x%x 0x%x \n", IPF_ULBD_MEM_ADDR, IPF_ULBD_MEM_SIZE);
    IPF_PRINT("IPF_ULRD_MEM_ADDR    0x%x 0x%x \n", IPF_ULRD_MEM_ADDR, IPF_ULRD_MEM_SIZE);
    IPF_PRINT("IPF_DLBD_MEM_ADDR    0x%x 0x%x \n", IPF_DLBD_MEM_ADDR, IPF_DLBD_MEM_SIZE);
    IPF_PRINT("IPF_DLRD_MEM_ADDR    0x%x 0x%x \n", IPF_DLRD_MEM_ADDR, IPF_DLRD_MEM_SIZE);
    IPF_PRINT("IPF_DLCD_MEM_ADDR    0x%x 0x%x \n", IPF_DLCD_MEM_ADDR, IPF_DLCD_MEM_SIZE);
    IPF_PRINT("IPF_INIT_ADDR        0x%x 0x%x \n", IPF_INIT_ADDR, IPF_INIT_SIZE);
    IPF_PRINT("IPF_DEBUG_INFO_ADDR  0x%x 0x%x \n", IPF_DEBUG_INFO_ADDR, IPF_DEBUG_INFO_SIZE);
}




/*****************************************************************************
* 函 数 名     : BSP_IPF_DlRegReInit
*
* 功能描述  : Ccore复位时,IPF会随之复位,其寄存器信息会全部消失。
				  该函数在ccore复位、解复位并启动成功后,
				  在ADS回调处理中首先调用,用于重新配置IPF下行通道相关寄存器

*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 无
*
* 修改记录  :2013年9月1日   陈东岳创建
*****************************************************************************/
BSP_VOID BSP_IPF_DlRegReInit(BSP_VOID)
{
	/*配置IPF下行通道寄存器*/
    BSP_U32 u32BDSize[IPF_CHANNEL_MAX] = {IPF_ULBD_DESC_SIZE, IPF_DLBD_DESC_SIZE};
    BSP_U32 u32RDSize[IPF_CHANNEL_MAX] = {IPF_ULRD_DESC_SIZE, IPF_DLRD_DESC_SIZE};
    BSP_U32 u32ADCtrl[IPF_CHANNEL_MAX] = {IPF_ADQ_DEFAULT_SEETING,IPF_ADQ_DEFAULT_SEETING};
    memset((BSP_VOID*)IPF_DLBD_MEM_ADDR, 0x0, IPF_DLDESC_SIZE);
	
    g_stIpfUl.pstIpfBDQ = (IPF_BD_DESC_S*)IPF_ULBD_MEM_ADDR;
    g_stIpfUl.pstIpfRDQ = (IPF_RD_DESC_S*)IPF_ULRD_MEM_ADDR;
    g_stIpfUl.pstIpfADQ0 = (IPF_AD_DESC_S*)IPF_ULAD0_MEM_ADDR;
    g_stIpfUl.pstIpfADQ1 = (IPF_AD_DESC_S*)IPF_ULAD1_MEM_ADDR;
    g_stIpfUl.pu32IdleBd = (BSP_U32*)IPF_ULBD_IDLENUM_ADDR;

    g_stIpfDl.pstIpfBDQ = (IPF_BD_DESC_S*)IPF_DLBD_MEM_ADDR;
    g_stIpfDl.pstIpfRDQ = (IPF_RD_DESC_S*)IPF_DLRD_MEM_ADDR;
    g_stIpfDl.pstIpfADQ0 = (IPF_AD_DESC_S*)IPF_DLAD0_MEM_ADDR;
    g_stIpfDl.pstIpfADQ1 = (IPF_AD_DESC_S*)IPF_DLAD1_MEM_ADDR;
    g_stIpfDl.pstIpfCDQ = (IPF_CD_DESC_S*)IPF_DLCD_MEM_ADDR;

    g_stIpfDl.pstIpfPhyBDQ = (IPF_BD_DESC_S*)(IPF_DLBD_MEM_ADDR - DDR_SHARED_MEM_VIRT_ADDR + DDR_SHARED_MEM_ADDR);
    g_stIpfDl.pstIpfPhyRDQ = (IPF_RD_DESC_S*)(IPF_DLRD_MEM_ADDR - DDR_SHARED_MEM_VIRT_ADDR + DDR_SHARED_MEM_ADDR);
    g_stIpfDl.pstIpfPhyADQ0 = (IPF_AD_DESC_S*)(IPF_DLAD0_MEM_ADDR - DDR_SHARED_MEM_VIRT_ADDR + DDR_SHARED_MEM_ADDR);
    g_stIpfDl.pstIpfPhyADQ1 = (IPF_AD_DESC_S*)(IPF_DLAD1_MEM_ADDR - DDR_SHARED_MEM_VIRT_ADDR + DDR_SHARED_MEM_ADDR);
	
    g_stIpfDl.u32IpfCdRptr = (BSP_U32*) IPF_DLCDRPTR_MEM_ADDR;
    *(g_stIpfDl.u32IpfCdRptr) = 0;

    g_stIPFDebugInfo = (IPF_DEBUG_INFO_S*)IPF_DEBUG_INFO_ADDR;
	
    /* 配置下行通道的AD、BD和RD深度 */
    ipf_writel(u32BDSize[IPF_CHANNEL_DOWN]-1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_SIZE_OFFSET);
    ipf_writel(u32RDSize[IPF_CHANNEL_DOWN]-1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_SIZE_OFFSET);
    ipf_writel(u32ADCtrl[IPF_CHANNEL_DOWN], HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ_CTRL_OFFSET);

    /*下行通道的BD和RD起始地址*/    
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyBDQ, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_BDQ_BADDR_OFFSET);
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyRDQ, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_BADDR_OFFSET);
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyADQ0, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_BASE_OFFSET);
    ipf_writel((BSP_U32)g_stIpfDl.pstIpfPhyADQ1, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_BASE_OFFSET);

    g_IPFInit = IPF_ACORE_INIT_SUCCESS;

    bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"[IPF]  ipf dl register reinit success\n");

    return;

}

/*****************************************************************************
* 函 数 名     : BSP_IPF_GetUsedDlAd
*
* 功能描述  : 功能同V9R1的同名函数，用于获取配置给AD队列的，
				  且尚未被硬件使用的AD信息，调用者释放获取的AD
				  中对应的skb(IMM_Zc)，以防止Ccore reset时内存泄露
				  调用该函数期间，应禁止配置BD。
				  该函数运行时会关闭通道

* 输入参数  : eAdType: AD队列类型
*   
* 输出参数  : pu32AdNum: 需要释放的AD数目
				  pstAdDesc: 需要释放的AD数组头指针
*
* 返 回 值     : BSP_ERR_IPF_INVALID_PARA 入参非法
				  IPF_ERROR 失败
*				  IPF_SUCCESS 成功
* 修改记录  :2013年9月1日   陈东岳创建
*****************************************************************************/
BSP_S32 BSP_IPF_GetUsedDlAd(IPF_AD_TYPE_E eAdType, BSP_U32 * pu32AdNum, IPF_AD_DESC_S * pstAdDesc)
{
	BSP_U32 u32Timeout = 10;
	BSP_U32 u32DlStateValue;
	BSP_U32 u32ChanEnable;
	BSP_U32 u32AdStateValue;
	BSP_U32 u32FreeAdNum = 0;
	BSP_U32 u32ADQwptr;
	BSP_U32 u32ADQrptr;
	BSP_U32 u32ADCtrl;
	/*关闭下行AD配置接口*/
	g_IPFInit = 0;

	/*入参检测*/
	if((NULL == pu32AdNum)||(NULL == pstAdDesc))
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input para ERROR!NULL == pu32AdNum or NULL == pstAdDesc\n");
		return BSP_ERR_IPF_INVALID_PARA;
	}	
		
	/*等待通道idle ,200ms超时*/
	do
	{
		msleep(20);
		u32DlStateValue = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_STATE_OFFSET);

		if(u32DlStateValue == IPF_CHANNEL_STATE_IDLE)
		{
			break;
		}
		
	}while(--u32Timeout);
	if (!u32Timeout) 
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r After 20ms IPF dl channel still on, unable to free AD \n");
		return IPF_ERROR;
	}
	/*尝试关闭下行通道*/
	u32ChanEnable = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH_EN_OFFSET);
	u32ChanEnable &= 0xFFFFFFFF ^ (0x1<<IPF_CHANNEL_DOWN);
	ipf_writel(u32ChanEnable, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH_EN_OFFSET); 

	u32ADCtrl = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ_CTRL_OFFSET);
	u32ADCtrl &= IPF_ADQ_EN_MASK;
	u32ADCtrl |= (IPF_NO_ADQ);

	/*关闭AD，用于防止产生ADQ预取*/
	ipf_writel(u32ADCtrl, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ_CTRL_OFFSET);
	if(IPF_AD_0 == eAdType)
	{
		u32AdStateValue = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_STAT_OFFSET);/*[false alarm]:fortify*/
		/*回退AD读指针*/
		u32ADQwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_WPTR_OFFSET);
		u32ADQrptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_RPTR_OFFSET);
		while(u32ADQrptr != u32ADQwptr)
		{
			pstAdDesc->u32OutPtr1 = g_stIpfDl.pstIpfADQ0[u32ADQrptr].u32OutPtr1;
			pstAdDesc->u32OutPtr0 = g_stIpfDl.pstIpfADQ0[u32ADQrptr].u32OutPtr0;
//			printk("AD0[%u]OutPtr1 = 0x%x \n", u32ADQrptr, pstAdDesc->u32OutPtr1);
			u32ADQrptr = ((u32ADQrptr + 1) < IPF_DLAD0_DESC_SIZE)? (u32ADQrptr + 1) : 0;	
			pstAdDesc++;
			u32FreeAdNum++;
		}
	}
	else if(IPF_AD_1 == eAdType)
	{
		u32AdStateValue = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_STAT_OFFSET);/*[false alarm]:fortify*/
		/*回退AD读指针*/
		u32ADQwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_WPTR_OFFSET);
		u32ADQrptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_RPTR_OFFSET);

		while(u32ADQrptr != u32ADQwptr)
		{
			pstAdDesc->u32OutPtr1 = g_stIpfDl.pstIpfADQ1[u32ADQrptr].u32OutPtr1;
			pstAdDesc->u32OutPtr0 = g_stIpfDl.pstIpfADQ1[u32ADQrptr].u32OutPtr0;
//			printk("AD1[%u]OutPtr1 = 0x%x", u32ADQrptr, pstAdDesc->u32OutPtr1);
			u32ADQrptr = ((u32ADQrptr + 1) < IPF_DLAD1_DESC_SIZE)? (u32ADQrptr + 1) : 0;	
			pstAdDesc++;
			u32FreeAdNum++;
		}
	}
	else
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input para ERROR! u32AdType >= IPF_AD_MAX\n");
		return BSP_ERR_IPF_INVALID_PARA;
	}
	/*返回AD*/
	*pu32AdNum = u32FreeAdNum;
	return IPF_SUCCESS;
	
}



/*****************************************************************************
* 函 数 名     : BSP_IPF_SetControlFLagForCcoreReset
*
* 功能描述  : modem单独复位ipf适配函数，用于在复位时阻止下行数传
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 成功
*
* 修改记录  :	2013年4月19日   卢彦胜创建
					2013年6月16日   陈东岳适配到V7R2
*****************************************************************************/
BSP_VOID BSP_IPF_SetControlFLagForCcoreReset(IPF_FORREST_CONTROL_E eResetFlag)
{
    if(eResetFlag >= IPF_FORRESET_CONTROL_MAX)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_SetControlFLagForCcoreReset eResetFlag overflow! \n");
        return;
    }
    /*设置标志，终止上行数传*/
    g_eCcoreResetFlag = eResetFlag;
    cache_sync();			
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_GetControlFLagForCcoreReset
*
* 功能描述  : modem单独复位ipf适配函数，用于在复位时阻止下行数传
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 成功
*
* 修改记录  :2013年6月16日   陈东岳  创建
*****************************************************************************/
static IPF_FORREST_CONTROL_E BSP_IPF_GetControlFLagForCcoreReset(BSP_VOID)
{
    return g_eCcoreResetFlag;
}


/******************************************************************************
* 函 数 名     : BSP_IPF_GetUlBDNum
*
* 功能描述  : 该接口用于获取上行空闲BD 数目
*                            范围: 0~64
* 输入参数  : 无
*
* 输出参数  : 无
* 返 回 值      : 空闲BD数目
*  
* 修改记录  :2011年11月30日   鲁婷  创建
*****************************************************************************/
BSP_U32 BSP_IPF_GetUlBDNum(BSP_VOID)
{
    BSP_U32 u32UlBdDepth = 0;
    BSP_U32 u32IdleBd = 0;

    /* 计算空闲BD数量 */
    u32UlBdDepth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_DQ_DEPTH_OFFSET);

    u32IdleBd = IPF_ULBD_DESC_SIZE - (u32UlBdDepth & IPF_DQ_DEPTH_MASK);
    *(g_stIpfUl.pu32IdleBd) = u32IdleBd;
	
    if(0 == u32IdleBd)
	{
		g_stIPFDebugInfo->u32UlBdNotEnough++;
	}
    return u32IdleBd;
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_GetUlRdNum
*
* 功能描述  : 该接口用于读取上行RD数目
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 上行RD数目
*
* 修改记录  :2013年8月1日   chendongyue  创建
*****************************************************************************/
BSP_U32 BSP_IPF_GetUlRdNum(BSP_VOID)
{
    BSP_U32 u32RdqDepth = 0;
  
    /* 读取RD深度 */
    u32RdqDepth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_DQ_DEPTH_OFFSET);
    u32RdqDepth = (u32RdqDepth>>16)&IPF_DQ_DEPTH_MASK;
    return u32RdqDepth;
}


/*****************************************************************************
* 函 数 名     : BSP_IPF_GetUlDescNum
*
* 功能描述  : 该接口可读取上行可传输包数
			用于规避ipf硬件对头阻塞问题
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 上行可发送包数
*
* 修改记录  :2013年8月1日   chendongyue  创建
*****************************************************************************/

BSP_U32 BSP_IPF_GetUlDescNum(BSP_VOID)
{
	BSP_U32 u32UlAd0Num = 0;
	BSP_U32 u32UlAd1Num = 0;
	BSP_U32 u32UlBdNum = 0;
/*	BSP_U32 u32UlRdNum = 0;*/
	BSP_U32 u32UlAdwptr = 0;
	BSP_U32 u32UlAdrptr = 0;
	BSP_U32 u32UlBdDepth = 0;

	/* 计算空闲AD0数量 */
	u32UlBdDepth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_DQ_DEPTH_OFFSET);
	u32UlAdwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ0_WPTR_OFFSET);
	u32UlAdrptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ0_RPTR_OFFSET);
	u32UlBdDepth = u32UlBdDepth&IPF_DQ_DEPTH_MASK;
	
	if (u32UlAdwptr >= u32UlAdrptr)/*写指针在前，正常顺序*/
	{
		u32UlAd0Num = u32UlAdwptr - u32UlAdrptr;
	}
	else
	{
		u32UlAd0Num = IPF_ULAD0_DESC_SIZE - (u32UlAdrptr -u32UlAdwptr);
	}
	if(u32UlAd0Num > u32UlBdDepth)
	{
		u32UlAd0Num -= u32UlBdDepth;
	}
	else
	{
		u32UlAd0Num = 0;
		g_stIPFDebugInfo->u32UlAd0NotEnough++;
	}


	/* 计算空闲AD1数量 */
	u32UlBdDepth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_DQ_DEPTH_OFFSET);
	u32UlAdwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ1_WPTR_OFFSET);
	u32UlAdrptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_ADQ1_RPTR_OFFSET);
	u32UlBdDepth = u32UlBdDepth&IPF_DQ_DEPTH_MASK;
	
	if (u32UlAdwptr >= u32UlAdrptr)/*写指针在前，正常顺序*/
	{
		u32UlAd1Num = u32UlAdwptr - u32UlAdrptr;
	}
	else
	{
		u32UlAd1Num =  IPF_ULAD1_DESC_SIZE - (u32UlAdrptr -u32UlAdwptr);
	}
	
	if(u32UlAd1Num > u32UlBdDepth)
	{
		u32UlAd1Num -= u32UlBdDepth;
	}
	else
	{
		u32UlAd1Num = 0;
		g_stIPFDebugInfo->u32UlAd1NotEnough++;
	}


	u32UlBdNum = BSP_IPF_GetUlBDNum();

	if(u32UlBdNum > u32UlAd0Num)
	{
		u32UlBdNum = u32UlAd0Num;
	}

	if(u32UlBdNum > u32UlAd1Num)
	{
		u32UlBdNum = u32UlAd1Num;
	}
	/*
	u32UlRdNum = IPF_ULRD_DESC_SIZE - BSP_IPF_GetUlRdNum();
	if(u32UlRdNum > 1)
	{
		u32UlRdNum -= 1;
	}
	else
	{
		u32UlRdNum = 0;
	}


	if(u32UlBdNum > u32UlRdNum)
	{
		u32UlBdNum = u32UlRdNum;
	*/
       return u32UlBdNum;

}


/*****************************************************************************
* 函 数 名      : BSP_IPF_ConfigUpFilter
*
* 功能描述  : IPF上行BD配置函数 
*
* 输入参数  : BSP_U32 u32Num, IPF_CONFIG_ULPARAM_S* pstUlPara
* 输出参数  : 无
* 返 回 值      : IPF_SUCCESS    配置成功
*                           IPF_ERROR      配置失败
*                           BSP_ERR_IPF_NOT_INIT         模块未初始化
*                           BSP_ERR_IPF_INVALID_PARA     参数错误
*
* 修改记录  :2011年11月30日   鲁婷  创建
				2012年11月30日	陈东岳修改添加多过滤器链和
									动态业务模式配置的支持
*****************************************************************************/
BSP_S32 BSP_IPF_ConfigUpFilter(BSP_U32 u32Num, IPF_CONFIG_ULPARAM_S* pstUlPara)
{
    BSP_U32 u32BdqWptr = 0;
    IPF_CONFIG_ULPARAM_S* pstUlParam = pstUlPara;
    BSP_U32 u32BD = 0;
    BSP_U32 i = 0;
    BSP_U32 u32TimeStampEn;

    /* 参数检查 */
    if((NULL == pstUlPara)||(0 == u32Num))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,
                         "\r BSP_IPF_ConfigUpFilter pInfoNode NULL! \n",0,0,0,0,0,0);
        return BSP_ERR_IPF_INVALID_PARA;
    }

    /* 检查模块是否初始化 */
    if(IPF_ACORE_INIT_SUCCESS != g_IPFInit)
    {
        g_stIPFDebugInfo->ipf_acore_not_init_count++;
        return BSP_ERR_IPF_NOT_INIT;
    }

    /* 检查Ccore是否上电*/
    if(IPF_FORRESET_CONTROL_FORBID <= BSP_IPF_GetControlFLagForCcoreReset())
    {
        g_stIPFDebugInfo->ipf_acore_not_init_count++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }

    
    for(i = 0; i < u32Num; i++)
    {
        if(0 == pstUlParam[i].u16Len)
        {
            bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r INPUT BD_Len =0 !Drop pkt \n");
            g_stIPFDebugInfo->ipf_ulbd_len_zero_count++;
            return BSP_ERR_IPF_INVALID_PARA;
        }
    }
	
    u32TimeStampEn = g_stIPFDebugInfo->ipf_timestamp_ul_en;

    /* 读出BD写指针,将u32BdqWptr作为临时写指针使用 */
    u32BdqWptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_BDQ_WPTR_OFFSET); 
    u32BD = (u32BdqWptr&IPF_DQ_PTR_MASK);
    for(i = 0; i < u32Num; i++)
    {
        g_stIpfUl.pstIpfBDQ[u32BD].u16Attribute = pstUlParam[i].u16Attribute;
        g_stIpfUl.pstIpfBDQ[u32BD].u32InPtr = pstUlParam[i].u32Data;
        g_stIpfUl.pstIpfBDQ[u32BD].u16PktLen = pstUlParam[i].u16Len;
        g_stIpfUl.pstIpfBDQ[u32BD].u16UsrField1 = pstUlParam[i].u16UsrField1;
        g_stIpfUl.pstIpfBDQ[u32BD].u32UsrField2= pstUlParam[i].u32UsrField2;
        g_stIpfUl.pstIpfBDQ[u32BD].u32UsrField3 = pstUlParam[i].u32UsrField3;
        ipf_record_start_time_stamp(u32TimeStampEn, &g_stIpfUl.pstIpfBDQ[u32BD].u32UsrField3);

        u32BD = ((u32BD + 1) < IPF_ULBD_DESC_SIZE)? (u32BD + 1) : 0;
    }
    
    /* 检查Ccore是否上电*/
    if(IPF_FORRESET_CONTROL_FORBID <= BSP_IPF_GetControlFLagForCcoreReset())
    {
        g_stIPFDebugInfo->ipf_acore_not_init_count++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }
	
    g_stIPFDebugInfo->ipf_cfg_ulbd_count += u32Num;

    /* 更新BD写指针*/
    ipf_writel(u32BD, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH0_BDQ_WPTR_OFFSET);  
    return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_ConfigDlAd
*
* 功能描述  : 该接口仅在A核提供，用于移动ADQ写指针，
				给空闲的AD分配新的内存缓冲区，一次可以处理多个AD。
				数传前要调用这个函数分配缓冲区。
*                           
* 输入参数  : BSP_U32 u32ADNum0; 
				 BSP_U32 u32ADNum1; 
				 BSP_VOID* psk0; 
				 BSP_VOID* psk1
*
* 输出参数  : 无
* 返 回 值      : 无
* 修改记录  :2012年11月24日   陈东岳  创建
*****************************************************************************/
BSP_S32 BSP_IPF_ConfigDlAd(BSP_U32 u32AdType, BSP_U32  u32AdNum, IPF_AD_DESC_S * pstAdDesc)
{
	BSP_U32 u32ADQwptr = 0;
	struct tagIPF_AD_DESC_S * pstADDesc = pstAdDesc;
	BSP_U32 i;
	if(NULL == pstAdDesc)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input para ERROR! NULL == pstAdDesc\n");
		return BSP_ERR_IPF_INVALID_PARA;
	}	

	if(u32AdType >= IPF_AD_MAX)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input para ERROR! u32AdType >= IPF_AD_MAX\n");
		return BSP_ERR_IPF_INVALID_PARA;
	}

	/* 检查模块是否初始化 */
	if(IPF_ACORE_INIT_SUCCESS != g_IPFInit)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd IPF Not Init! \n");
		return BSP_ERR_IPF_NOT_INIT;
	}


	if(IPF_AD_0 == u32AdType)
	{
		if(u32AdNum >= IPF_DLAD0_DESC_SIZE)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input para ERROR! u32AdNum >=IPF_ULAD0_DESC_SIZE\n");
			return BSP_ERR_IPF_INVALID_PARA;
		}

		/*读出写指针*/
		u32ADQwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_WPTR_OFFSET);
		for(i=0; i < u32AdNum; i++)
		{
			if(NULL == (BSP_VOID*)(pstADDesc->u32OutPtr1))
			{
				bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input u32OutPtr1 NULL! \n");
				g_stIPFDebugInfo->ipf_dlad0_error_count++;
				return BSP_ERR_IPF_INVALID_PARA;
			}
			g_stIpfDl.pstIpfADQ0[u32ADQwptr].u32OutPtr1 = pstADDesc->u32OutPtr1;
			g_stIpfDl.pstIpfADQ0[u32ADQwptr].u32OutPtr0 = pstADDesc->u32OutPtr0;
			u32ADQwptr = ((u32ADQwptr + 1) < IPF_DLAD0_DESC_SIZE)? (u32ADQwptr + 1) : 0;		
			pstADDesc++;
		}
		g_stIPFDebugInfo->ipf_cfg_dlad0_count += u32AdNum;

		/* 更新AD0写指针*/
		ipf_writel(u32ADQwptr, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_WPTR_OFFSET);  
	}
	else if(IPF_AD_1 == u32AdType)
	{
		if(u32AdNum >= IPF_DLAD1_DESC_SIZE)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input para ERROR! u32AdNum >=IPF_ULAD1_DESC_SIZE \n");
			return BSP_ERR_IPF_INVALID_PARA;
		}

		/*读出写指针*/
		u32ADQwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_WPTR_OFFSET);
		for(i=0; i < u32AdNum; i++)
		{
			if(NULL == (BSP_VOID*)(pstADDesc->u32OutPtr1))
			{
				bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_ConfigDlAd input u32OutPtr1 NULL! \n");
				g_stIPFDebugInfo->ipf_dlad1_error_count++;
				return BSP_ERR_IPF_INVALID_PARA;
			}

			g_stIpfDl.pstIpfADQ1[u32ADQwptr].u32OutPtr1 = pstADDesc->u32OutPtr1;
			g_stIpfDl.pstIpfADQ1[u32ADQwptr].u32OutPtr0 = pstADDesc->u32OutPtr0;
			u32ADQwptr = ((u32ADQwptr + 1) < IPF_DLAD1_DESC_SIZE)? (u32ADQwptr + 1) : 0;		
			pstADDesc++;
		}
		g_stIPFDebugInfo->ipf_cfg_dlad1_count += u32AdNum;

		/* 更新AD1写指针*/
		ipf_writel(u32ADQwptr, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_WPTR_OFFSET);  
	}
	return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_RegisterWakeupDlCb
*
* 功能描述  : 该接口用于注册下行PS任务回调函数
*                           
* 输入参数  : BSP_IPF_WakeupDlkCb *pFnWakeupDl
*
* 输出参数  : 无
* 返 回 值      : 无
*  
* 修改记录  :2011年11月30日   鲁婷  创建
*****************************************************************************/
BSP_S32 BSP_IPF_RegisterWakeupDlCb(BSP_IPF_WakeupDlCb pFnWakeupDl)
{
    /* 参数检查 */
    if(NULL == pFnWakeupDl)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_RegisterWakeupDlCb inputPara ERROR! \n");
        return BSP_ERR_IPF_INVALID_PARA;
    }
    g_stIpfDl.pFnDlIntCb = pFnWakeupDl;
    return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_RegisterAdqEmptyDlCb
*
* 功能描述  : 此接口只在A核提供，用于注册唤醒上行PS的
                             ADQ队列空回调函数
*                           
* 输入参数  : BSP_IPF_AdqEmptyCb pAdqEmptyDl
*
* 输出参数  : 无
* 返 回 值      : IPF_SUCCESS 注册成功
*                            IPF_ERROR	注册失败
* 修改记录  :2012年11月24日   陈东岳  创建
*****************************************************************************/
BSP_S32 BSP_IPF_RegisterAdqEmptyDlCb(BSP_IPF_AdqEmptyDlCb pAdqEmptyDl)
{
    /* 参数检查 */
    if(NULL == pAdqEmptyDl)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_RegisterAdqEmptyDlCb inputPara ERROR! \n");
        return IPF_ERROR;
    }
    g_stIpfDl.pAdqEmptyDlCb = pAdqEmptyDl;
    return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_GetDlRd
*
* 功能描述  : 该接口用于读取下行BD, 支持一次读取多个BD
*
* 输入参数  : BSP_U32* pu32Num    
*                           IPF_RD_DESC_S *pstRd
*   
* 输出参数  : BSP_U32* pu32Num    实际读取的RD数目
*
* 返 回 值     : IPF_SUCCESS               操作成功
*                           IPF_ERROR                   操作失败
*
* 修改记录  :2011年11月30日   鲁婷  创建
*****************************************************************************/
BSP_VOID BSP_IPF_GetDlRd(BSP_U32* pu32Num, IPF_RD_DESC_S *pstRd)
{
    BSP_U32 u32RdqRptr = 0;
    BSP_U32 u32RdqDepth = 0;
    BSP_U32 u32Num = 0;
    BSP_U32 i = 0;
    BSP_U32 u32CdqRptr;
    BSP_U32 u32TimeStampEn;
	
    /* 读取RD深度 */
    u32RdqDepth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_DQ_DEPTH_OFFSET);
    u32RdqDepth = (u32RdqDepth>>16)&IPF_DQ_DEPTH_MASK;
	
    u32Num = (u32RdqDepth < *pu32Num)?u32RdqDepth:*pu32Num;
    if(0 == u32Num)
    {
        *pu32Num = 0;
        return;
    }
    u32TimeStampEn = g_stIPFDebugInfo->ipf_timestamp_dl_en;

    /* 读取RD读指针 */
    u32RdqRptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_RPTR_OFFSET);
    for(i = 0; i < u32Num; i++)
    {
        
        /* 获取RD */
        pstRd[i].u16Attribute = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u16Attribute;
        pstRd[i].u16PktLen = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u16PktLen;
        pstRd[i].u16Result = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u16Result;
        pstRd[i].u32InPtr = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u32InPtr;
        pstRd[i].u32OutPtr = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u32OutPtr;
        pstRd[i].u16UsrField1 = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u16UsrField1;
        pstRd[i].u32UsrField2 = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u32UsrField2;
        pstRd[i].u32UsrField3 = g_stIpfDl.pstIpfRDQ[u32RdqRptr].u32UsrField3;
        ipf_record_end_time_stamp(u32TimeStampEn, g_stIpfDl.pstIpfRDQ[u32RdqRptr].u32UsrField3);

       /* 更新CD读指针 */
        u32CdqRptr = (IPF_IO_ADDRESS(pstRd[i].u32InPtr) - (BSP_U32)g_stIpfDl.pstIpfCDQ)/sizeof(IPF_CD_DESC_S);

        while(g_stIpfDl.pstIpfCDQ[u32CdqRptr].u16Attribute != 1)
        {
            /* 将释放的CD  清0 */
            g_stIpfDl.pstIpfCDQ[u32CdqRptr].u16Attribute = 0;
            g_stIpfDl.pstIpfCDQ[u32CdqRptr].u16PktLen = 0;
            g_stIpfDl.pstIpfCDQ[u32CdqRptr].u32Ptr = 0;
            u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
        }
        g_stIpfDl.pstIpfCDQ[u32CdqRptr].u16Attribute = 0;
        g_stIpfDl.pstIpfCDQ[u32CdqRptr].u16PktLen = 0;
        g_stIpfDl.pstIpfCDQ[u32CdqRptr].u32Ptr = 0;
		
        u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
        *(g_stIpfDl.u32IpfCdRptr) = u32CdqRptr;
        /* 更新RD读指针 */
        u32RdqRptr = ((u32RdqRptr+1) < IPF_DLRD_DESC_SIZE)?(u32RdqRptr+1):0;        
    }
    ipf_writel(u32RdqRptr, HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_RDQ_RPTR_OFFSET);         
    *pu32Num = u32Num;
    g_stIPFDebugInfo->ipf_get_dlrd_count += u32Num;
}
/*****************************************************************************
* 函 数 名       : BSP_IPF_GetDlAdNum
*
* 功能描述  : 该接口只在A核提供，获取下行（A核）
					空闲(即填入该AD的缓冲区已经被占用)AD数目
*
* 输入参数  :BSP_OK：正常返回
                            BSP_ERROR：出错
*             
* 输出参数  : 无
* 返 回 值     : 无
* 修改记录  : 2011年11月24日   陈东岳  创建
*****************************************************************************/
BSP_S32 BSP_IPF_GetDlAdNum(BSP_U32* pu32AD0Num,BSP_U32* pu32AD1Num)
{
	BSP_U32 u32DlAdDepth = 0;
	BSP_U32 u32DlAdwptr = 0;
	BSP_U32 u32DlAdrptr = 0;

	/* 检查模块是否初始化 */
	if(IPF_ACORE_INIT_SUCCESS != g_IPFInit)
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_GetDlAdNum IPF Not Init! \n");
		return BSP_ERR_IPF_NOT_INIT;
	}

	/*入参检测*/
	if((NULL == pu32AD0Num)||(NULL == pu32AD1Num))
	{
		bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r BSP_IPF_GetDlAdNum pstCtrl NULL! \n");
		return BSP_ERR_IPF_INVALID_PARA;
	}
	
	/* 计算空闲AD0数量 */
	u32DlAdwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_WPTR_OFFSET);
	u32DlAdrptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ0_RPTR_OFFSET);
	if (u32DlAdwptr >= u32DlAdrptr)/*写指针在前，正常顺序，两指针相等的情况深度为0*/
	{
		u32DlAdDepth = IPF_DLAD0_DESC_SIZE - (u32DlAdwptr - u32DlAdrptr);
	}
	else
	{
		u32DlAdDepth = u32DlAdrptr -u32DlAdwptr;
	}

	/*扣除reserve ad，用于防止硬件将ad队列满识别成空和低功耗引发内存泄露*/
	if(u32DlAdDepth > IPF_ADQ_RESERVE_NUM)
	{
		*pu32AD0Num = u32DlAdDepth - IPF_ADQ_RESERVE_NUM;
	}
	else
	{
		*pu32AD0Num = 0;
	}
	
	/* 计算空闲AD1数量 */
	u32DlAdwptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_WPTR_OFFSET);
	u32DlAdrptr = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_ADQ1_RPTR_OFFSET);
	if (u32DlAdwptr >= u32DlAdrptr)/*写指针在前，正常顺序*/
	{
		u32DlAdDepth = IPF_DLAD1_DESC_SIZE - (u32DlAdwptr - u32DlAdrptr);
	}
	else
	{
		u32DlAdDepth =  u32DlAdrptr - u32DlAdwptr;
	}
	*pu32AD1Num = u32DlAdDepth;

	/*扣除reserve ad，用于防止硬件将ad队列满识别成空和低功耗引发内存泄露*/	
	if(u32DlAdDepth > IPF_ADQ_RESERVE_NUM)
	{
		*pu32AD1Num = u32DlAdDepth - IPF_ADQ_RESERVE_NUM;
	}
	else
	{
		*pu32AD1Num = 0;
	}

	return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : BSP_IPF_GetDlRdNum
*
* 功能描述  : 该接口用于读取下行RD数目
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 上行RD数目
*
* 修改记录  :2012年7月16日   鲁婷  创建
*****************************************************************************/
BSP_U32 BSP_IPF_GetDlRdNum(BSP_VOID)
{
    BSP_U32 u32RdqDepth = 0;
  
    /* 读取RD深度 */
    u32RdqDepth = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_DQ_DEPTH_OFFSET);
    u32RdqDepth = (u32RdqDepth>>16)&IPF_DQ_DEPTH_MASK;
    return u32RdqDepth;
}

/*****************************************************************************
* 函 数 名     : ipf_probe
*
* 功能描述  : 平台设备桩函数
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 成功
*
* 修改记录  :2013年6月16日   陈东岳  创建
*****************************************************************************/
static int ipf_probe(struct platform_device *pdev)
{

    return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : ipf_remove
*
* 功能描述  : 平台设备桩函数
*
* 输入参数  : 无
*   
* 输出参数  : 无
*
* 返 回 值     : 成功
*
* 修改记录  :2013年6月16日   陈东岳  创建
*****************************************************************************/
static int ipf_remove(struct platform_device *pdev)
{
    return IPF_SUCCESS;
}


/*****************************************************************************
* 函 数 名     : ipf_dl_dpm_prepare
*
* 功能描述  : dpm进入准备函数
*
* 输入参数  : 设备指针
*   
* 输出参数  : 无
*
* 返 回 值     : IPF_ERROR 失败
                            IPF_SUCCESS 成功
*
* 修改记录  :2013年6月16日   陈东岳  创建
*****************************************************************************/
static int ipf_dl_dpm_prepare(struct device *pdev)
{
    u32 u32_dl_state;
	


    /* 判断下行IPF是否空闲 */
    u32_dl_state = ipf_readl(HI_IPF_REGBASE_ADDR_VIRT + HI_IPF_CH1_STATE_OFFSET);

    if(u32_dl_state != IPF_CHANNEL_STATE_IDLE)
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r IPF CHANNEL NOT IDLE! \n");
        return IPF_ERROR;
    }
    return IPF_SUCCESS;
}

/*****************************************************************************
* 函 数 名     : ipf_dl_dpm_complete
*
* 功能描述  : dpm桩函数
*
* 输入参数  : 设备指针
*   
* 输出参数  : 无
*
* 返 回 值     : 无
*
* 修改记录  :2013年6月16日   陈东岳  创建
*****************************************************************************/
static void ipf_dl_dpm_complete(struct device *pdev)
{
    return ;
}

/*****************************************************************************
* 函 数 名     : ipf_dl_suspend
*
* 功能描述  : dpm桩函数
*
* 输入参数  : 设备指针
*   
* 输出参数  : 无
*
* 返 回 值     : 成功
*
* 修改记录  :2013年6月16日   陈东岳  创建
			2014年1月23日v1.01 陈东岳 修改 由于K3V3总线设计问题，
			无法在m3上进行低功耗恢复，移动到A9上进行。
*****************************************************************************/
static int ipf_dl_suspend(struct device *dev)
{
    g_stIPFDebugInfo->ipf_acore_suspend_count++;
    return IPF_SUCCESS;
}


/*****************************************************************************
* 函 数 名     : ipf_dl_resume
*
* 功能描述  : dpm桩函数
*
* 输入参数  : 设备指针
*   
* 输出参数  : 无
*
* 返 回 值     : 成功
*
* 修改记录  :2013年6月16日   陈东岳  创建
			2014年1月23日v1.01 陈东岳 修改 由于K3V3总线设计问题，
			无法在m3上进行低功耗恢复，移动到A9上进行。
*****************************************************************************/
static int ipf_dl_resume(struct device *dev)
{

    g_stIPFDebugInfo->ipf_acore_resume_count++;
    return IPF_SUCCESS;
}


module_init(ipf_init); /*lint !e528*/
EXPORT_SYMBOL(ipf_init);
EXPORT_SYMBOL(BSP_IPF_ConfigTimeout);
EXPORT_SYMBOL(BSP_IPF_ConfigUpFilter);
EXPORT_SYMBOL(BSP_IPF_GetUlBDNum);
EXPORT_SYMBOL(BSP_IPF_SetPktLen);
EXPORT_SYMBOL(BSP_IPF_RegisterWakeupDlCb);
EXPORT_SYMBOL(BSP_IPF_GetDlRd);
EXPORT_SYMBOL(BSP_IPF_GetDlRdNum);
EXPORT_SYMBOL(BSP_IPF_Help);
EXPORT_SYMBOL(BSP_IPF_BDInfo);
EXPORT_SYMBOL(BSP_IPF_RDInfo);
EXPORT_SYMBOL(BSP_IPF_Info);
EXPORT_SYMBOL(BSP_IPF_MEM);
EXPORT_SYMBOL(BSP_IPF_ConfigDlAd);
EXPORT_SYMBOL(BSP_IPF_RegisterAdqEmptyDlCb);
EXPORT_SYMBOL(BSP_IPF_GetDlAdNum);
EXPORT_SYMBOL(BSP_IPF_GetControlFLagForCcoreReset);
MODULE_LICENSE("GPL");



/*lint -restore*/


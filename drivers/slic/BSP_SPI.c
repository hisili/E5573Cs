
//#include <stdio.h>
//#include <taskLib.h>
//#include <intLib.h>
//#include <logLib.h>
//#include "BSP_GLOBAL.h"
//#include "arm_pbxa9.h"
//#include "product_config.h"
//#include "DrvInterface.h"
#include "DrvInterface.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include "BSP_DRV_SPI.h"
#include "data_types.h"
//#include "BSP_Report.h"

#define INTEGRATOR_SC_BASE 0x90000000

//add new interface

#include "slic_spi.h"
//#include "OnChipRom.h"
//#include "sys.h"
//#include "si3217x_intf.h"
//#include "si3217x_registers.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <asm/irq.h>
#include <mach/irqs.h>
//#include <linux/spi/spi.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/compat.h>
//#include <mach/gpio-tlmm-v1.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/spinlock.h>


#include "proslic.h"
#include "si3217x_registers.h"

#include "drv_sio.h"
#include "slic_log.h"

//end






#ifdef __cplusplus
extern "C" {
#endif


enum  EN_PERIPHERAL_TYPE
{
    EN_PERIPHERAL_USB = 0,
    EN_PERIPHERAL_HSIC,
    EN_PERIPHERAL_MAC,
    EN_PERIPHERAL_SIO,
};

BSP_U32 g_ulSioBaseAddr = 0;


    //继承M核寄存器接口命名
#define OUTREG32(reg, val) \
    BSP_REG_WRITE(reg, 0, val)

#define INREG32(reg) \
    BSP_REG(reg, 0)

#define SETBITVALUE32(addr, mask, value) \
    OUTREG32((addr), (INREG32(addr)&(~(mask))) | ((value)&(mask)))

    //end




#if 0
    /*描述SPI属性的结构体，包括SPI基地址等*/
    typedef struct tagSPI_ATTR
    {
        SPI_DEV_ATTR_S stSpiConf[SPI_DEV_CS_MAX];/*存储片选连接设备属性的结构体*/
        SEM_ID SpiSemId;                         /*每个SPI各自有一个信号量*/
        SPI_DEV_CS_E enSpiOwner;                 /*SPI按照哪个片选连接的设备进行配置*/
        BSP_U32 u32SpiBaseAddr;                  /*SPI基地址*/
        BSP_BOOL abFlag[SPI_DEV_CS_MAX];         /*标志SPI是否按照某片选连接的设备配置过*/
    }SPI_ATTR_S;
#endif

    static DEFINE_SPINLOCK(slic_lock);


    /*记录SPI是否被初始化的全局变量*/
    BSP_BOOL g_bSpiInit = BSP_FALSE;

    //SPI_ATTR_S stSpiDev[SPI_ID_MAX] = {0}; /*lint !e64*/

    /* 可维可测平台注册函数*/
    //extern BSPRegExEventFunc g_pBSPRegExEventFunc;

    /*SPI在中断中调用，需要锁中断*/
    BSP_U32 g_stSpiIntCallLock = 0;

    /*SPI 低功耗中断中调用*/
#if ((defined (CHIP_BB_6920ES) || defined (CHIP_BB_6920CS))\
    && (defined (BOARD_ASIC)||defined (BOARD_ASIC_BIGPACK)))
#define SPI_DRV_INT_CALL	1
#else
#define SPI_DRV_INT_CALL	0
#endif







    //add new interface
#define ECS_SPI0_BASE       (INTEGRATOR_SC_BASE + 0x00008000)
#define ECS_SPI0_SIZE		0x00001000

#define ECS_SPI1_BASE       (INTEGRATOR_SC_BASE + 0x00023000)
#define ECS_SPI1_SIZE		0x00001000

    /*const */UINT32 spiBase[2] = {ECS_SPI0_BASE, ECS_SPI1_BASE};
    UINT32 g_gpioBase = 0;
    INT32 g_lUserModeStatus = 0;

    //end







#if 0
    /*****************************************************************************
    * 函 数 名  : SPI_Init
    *
    * 功能描述  : SPI初始化
    *
    * 输入参数  : 无
    *
    * 输出参数  : 无
    *
    * 返 回 值  : 初始化成功或者失败
    *****************************************************************************/
    BSP_S32 SPI_Init()
    {
        BSP_U32 u32Number;

        /*如果SPI已经初始化，直接返回BSP_OK*/
        if(BSP_TRUE == g_bSpiInit)
        {
            return BSP_OK;
        }

        memset(stSpiDev, 0x0, sizeof(SPI_ATTR_S) * SPI_ID_MAX);

        /*为每个SPI的基地址赋值*/
        stSpiDev[SPI_ID0].u32SpiBaseAddr = SPI1_BASE_ADDR;
        stSpiDev[SPI_ID1].u32SpiBaseAddr = SPI2_REGBASE_ADDR;

#if (defined (CHIP_BB_6920ES) || defined(CHIP_BB_6920CS)\
    && (defined (BOARD_ASIC)||defined (BOARD_ASIC_BIGPACK)))
        /* 使能SPI复用 */
        BSP_REG_WRITEBITS(INTEGRATOR_SC_BASE, INTEGRATOR_SC_IOS_CTRL98, SSP0_CTRL);     // IO复用为SSP0
        BSP_REG_WRITEBITS(INTEGRATOR_SC_BASE, INTEGRATOR_SC_IOS_CTRL81, SSP0_WIRE_4 | SSP0_CS1_EN);     // 使能SSP0,CS0和CS1,否则CS1管脚默认为低，操作CS0的时候CS1也被选中
#ifdef FEATURE_BOARD_STUB_BJ_UDP
        BSP_REG_WRITEBITS(INTEGRATOR_SC_BASE, INTEGRATOR_SC_IOS_CTRL98, SSP1_CTRL); 	// IO复用为SSP1
        BSP_REG_WRITEBITS(INTEGRATOR_SC_BASE, INTEGRATOR_SC_IOS_CTRL81, SSP1_WIRE_4  | SSP1_CS1_EN);     // 使能SSP1
#endif
#endif

        for(u32Number = SPI_ID0; u32Number < SPI_ID_MAX; u32Number++)
        {
            stSpiDev[u32Number].enSpiOwner = NO_OWNER;
            /*为每个SPI创建一个信号量*/
            stSpiDev[u32Number].SpiSemId = (SEM_ID)semMCreate(SEM_Q_FIFO);
            /* 信号量创建失败，返回错误*/
            if(NULL == stSpiDev[u32Number].SpiSemId )
            {
                BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"NULL == stSpiDev[u32Number].SpiSemId.\n",0,0,0,0,0,0);
                return BSP_ERR_SPI_SEM_CREATE;
            }

            /*对每个SPI中的一些寄存器赋值为上电初始值*/
            BSP_REG_WRITE(stSpiDev[u32Number].u32SpiBaseAddr, SPI_CTRL0_OFFSET, 0x0);
            BSP_REG_WRITE(stSpiDev[u32Number].u32SpiBaseAddr, SPI_CTRL1_OFFSET, 0x0);
            BSP_REG_WRITE(stSpiDev[u32Number].u32SpiBaseAddr, SPI_EN_OFFSET, 0x0);
            BSP_REG_WRITE(stSpiDev[u32Number].u32SpiBaseAddr, SPI_MWCTRL_OFFSET, 0x0);
            BSP_REG_WRITE(stSpiDev[u32Number].u32SpiBaseAddr, SPI_SLAVE_EN_OFFSET, 0x0);
            BSP_REG_WRITE(stSpiDev[u32Number].u32SpiBaseAddr, SPI_BAUD_OFFSET, 0x0);
        }

        g_bSpiInit = BSP_TRUE;

        return BSP_OK;
    }


    /*****************************************************************************
    * 函 数 名  : SPI_Lock
    *
    * 功能描述  : 锁定SPI总线
    *
    * 输入参数  : enSpiId 需要锁定的SPI号
    *
    * 输出参数  : 无
    *
    * 返 回 值  : 锁定成功或者失败
    *****************************************************************************/
    BSP_S32  SPI_Lock(SPI_DEV_ID_E enSpiId)
    {
#if (SPI_DRV_INT_CALL == 1)
        g_stSpiIntCallLock = intLock();
        return BSP_OK;
#else
        return (semTake(stSpiDev[enSpiId].SpiSemId, WAIT_FOREVER));
#endif
    }

    /*****************************************************************************
    * 函 数 名  : SPI_UnLock
    *
    * 功能描述  : 释放SPI总线
    *
    * 输入参数  : enSpiId 需要释放的SPI号
    *
    * 输出参数  : 无
    *
    * 返 回 值  : 释放成功或者失败
    *****************************************************************************/
    BSP_S32 SPI_UnLock(SPI_DEV_ID_E enSpiId)
    {
#if (SPI_DRV_INT_CALL == 1)
        intUnlock(g_stSpiIntCallLock);
        return BSP_OK;
#else
        return (semGive(stSpiDev[enSpiId].SpiSemId));
#endif
    }

    /*****************************************************************************
    * 函 数 名  : SPI_Poll_Send
    *
    * 功能描述  : 轮询模式下的数据发送
    *
    * 输入参数  : pSpiId  进行数据发送的SPI号和片选号。
    pData   需要发送的数据缓冲区指针
    *             u32time 要发送数据的次数
    *
    * 输出参数  : 无
    *
    * 返 回 值  : BSP_OK     发送成功
    *             ERROR  发送失败
    *****************************************************************************/
    BSP_S32 SPI_POLL_Send(SPI_DEV_S *pSpiId, void *pData, BSP_U32 u32Count)
    {
        BSP_U32 i;
        BSP_U16 *pSh;
        SPI_DATA_LEN_E enTmpDataLen = stSpiDev[pSpiId->enSpiId].stSpiConf[pSpiId->enSpiCs].enDataLen;
        BSP_U32 u32TemBaseAddr = stSpiDev[pSpiId->enSpiId].u32SpiBaseAddr;

        if((NULL == pData) || (0 == u32Count) || (u32Count > SPI_FIFO_DEPTH))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"SPI_POLL_Send Paramater ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_INVALID_PARA;
        }

        /* 数据宽度为8Bit或者16Bit*/
        if((SPI_DATASIZE_8BITS == enTmpDataLen)||(SPI_DATASIZE_16BITS == enTmpDataLen))
        {
            pSh = (BSP_U16*)pData;
            /* 向数据寄存器中写入数据*/
            for(i = 0; i < u32Count; i++)
            {
                BSP_REG_WRITE(u32TemBaseAddr, SPI_DATA_OFFSET_BASE,(*pSh));
                pSh++;
            }
        }
        else
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"SPI_POLL_Send enTmpDataLen ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_DATASIZE_INVALID;
        }

        return BSP_OK;
    }

    /*****************************************************************************
    * 函 数 名  : SPI_Poll_Receive
    *
    * 功能描述  : 轮询模式下的数据接收
    *
    * 输入参数  : pSpiId  进行数据接收的SPI号和片选号。
    pData   存储接收数据的缓冲区指针
    *             u32Lens 待接收的数据长度
    *
    * 输出参数  : 无
    *
    * 返 回 值  : BSP_OK    接收成功
    *             ERROR 接收失败
    *****************************************************************************/
    BSP_S32 SPI_POLL_Receive(SPI_DEV_S *pSpiId,BSP_VOID *pData, BSP_U32 u32Lens)
    {
        BSP_U8 *pCh;
        BSP_U16 *pSh;
        BSP_U32 u32Number = 0;
        BSP_U32 u32Temp = 0;
        SPI_DATA_LEN_E enTmpDataLen = stSpiDev[pSpiId->enSpiId].stSpiConf[pSpiId->enSpiCs].enDataLen;
        BSP_U32 u32TemBaseAddr = stSpiDev[pSpiId->enSpiId].u32SpiBaseAddr;

        if((NULL == pData) || (0 == u32Lens))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"SPI_POLL_Receive Paramater ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_INVALID_PARA;
        }

        /* 数据宽度为8Bit*/
        if(SPI_DATASIZE_8BITS == enTmpDataLen)
        {
            pCh = (BSP_U8 *)pData;
            /* 等待接收FIFO非空,非空则读取数据*/
            while(SPI_STATUS & 0x8)
            {
                //            BSP_REG8_READ(u32TemBaseAddr, SPI_DATA_OFFSET_BASE,(*pCh));
                BSP_REG_READ(u32TemBaseAddr, SPI_DATA_OFFSET_BASE, u32Temp);
                (*pCh) = (BSP_U8)(u32Temp & 0xff);
                u32Number++;
                pCh++;
            }
        }
        /* 数据宽度为16Bit*/
        else if(SPI_DATASIZE_16BITS == enTmpDataLen)
        {
            pSh = (BSP_U16 *)pData;
            /* 等待接收FIFO非空,非空则读取数据*/
            while(SPI_STATUS & 0x8)
            {
                //            BSP_REG16_READ(u32TemBaseAddr, SPI_DATA_OFFSET_BASE,(*pSh));
                BSP_REG_READ(u32TemBaseAddr, SPI_DATA_OFFSET_BASE, u32Temp);
                (*pSh) = (BSP_U16)(u32Temp & 0xffff);
                u32Number++;
                pSh++;
            }
        }
        else
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"SPI_POLL_Receive enTmpDataLen ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_DATASIZE_INVALID;
        }

        /* 如果接收数据比预期数据多，返回错误*/
        if(u32Number > u32Lens)
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"SPI_POLL_Receive. The actual data don't equal expected 1. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_RECEIVE_POLL;
        }

        return BSP_OK;
    }

    /*****************************************************************************
    * 函 数 名  : BSP_SPI_SetAttr
    *
    * 功能描述  : 配置SPI的寄存器，设置控制命令字长度、数据帧长度等。
    *
    * 输入参数  : enSpiID        需要设置的SPI号，以及根据哪片片选进行配置的片选号。
    *             pstSpiDevAttr  记录SPI外接设备特性的结构体指针，结构体成员包括设备
    需要的命令字长度，数据帧长度，使用协议，波特率等。
    *
    * 输出参数  : 无
    *
    * 返 回 值  : BSP_OK    接收成功
    *             ERROR 接收失败
    *****************************************************************************/
    BSP_S32 BSP_SPI_SetAttr(SPI_DEV_S *pstSpiID,SPI_DEV_ATTR_S *pstSpiDevAttr)
    {
        BSP_U32 u32Ctrl0 = 0;
        BSP_U32 u32TemBaseAddr;
        SPI_DEV_ID_E enTemSpiId = pstSpiID->enSpiId;
        SPI_DEV_CS_E enTemSpiCs = pstSpiID->enSpiCs;

        /*参数判断*/
        if((enTemSpiId >= SPI_ID_MAX)||(enTemSpiId < SPI_ID0)
            || (enTemSpiCs >= SPI_DEV_CS_MAX) || (enTemSpiCs < SPI_DEV_CS0)
            || ((pstSpiDevAttr -> enCommandLen) >=  SPI_COMMANDSIZE_MAX)
            || ((pstSpiDevAttr -> enCommandLen) <  SPI_COMMANDSIZE_1BITS)
            || ((pstSpiDevAttr ->enDataLen) >= SPI_DATASIZE_MAX)
            || ((pstSpiDevAttr ->enDataLen) < SPI_DATASIZE_4BITS)
            || ((pstSpiDevAttr -> enSpiProt) >= SPI_PROT_MAX)
            || ((pstSpiDevAttr -> enSpiProt) < SPI_PROT_SPI))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_SetAttr Paramater ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_INVALID_PARA;
        }

        /*如果SPI没有初始化，不成配置SPI，返回错误*/
        if(BSP_FALSE == g_bSpiInit)
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_SetAttr. BSP_FALSE == g_bSpiInit. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_MODULE_NOT_INITED;
        }

        if (BSP_OK != SPI_Lock(enTemSpiId))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. BSP_OK != SPI_Lock. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_SEM_LOCK;
        }

        u32TemBaseAddr = stSpiDev[enTemSpiId].u32SpiBaseAddr;

        /* 将外接设备的属性保存到成员stSpiConf中 */
        memcpy(&(stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs]), pstSpiDevAttr, sizeof(SPI_DEV_ATTR_S));

        /* 成员enSpiOwner记录当前哪个片选连接的设备正在占有SPI */
        stSpiDev[enTemSpiId].enSpiOwner = enTemSpiCs;

        /* 数组abFlag表示片选连接的设备是否使用过SPI，如果使用过，设备属性会储存在
        stSpiDev中，再次使用该设备时，直接从stSpiDev中获取设备属性*/
        stSpiDev[enTemSpiId].abFlag[enTemSpiCs] = BSP_TRUE;

        /* 根据设备属性，组合控制寄存器Ctrl0要配置的值*/
        u32Ctrl0 = pstSpiDevAttr->enDataLen | (pstSpiDevAttr->enSpiProt << SPI_PROT_SHIFT_BITS)
            | pstSpiDevAttr->enCommandLen << SPI_COMM_SHIFT_BITS;


        /*禁止SPI数据传输*/
        BSP_REG_WRITE(u32TemBaseAddr, SPI_EN_OFFSET, 0x0);

        /*配置ctrl0寄存器，包括命令字长度，数据宽度*/
        BSP_REG_WRITE(u32TemBaseAddr, SPI_CTRL0_OFFSET,u32Ctrl0);

        /*配置SPI波特率*/
        BSP_REG_WRITE(u32TemBaseAddr, SPI_BAUD_OFFSET,pstSpiDevAttr->u16SpiBaud);

        if(BSP_OK != SPI_UnLock(enTemSpiId))
        {
            return BSP_ERR_SPI_SEM_UNLOCK;
        }

        return BSP_OK;

    }



    /*****************************************************************************
    * 函 数 名  : BSP_SPI_Write
    *
    * 功能描述  : 通过SPI向设备写入数据
    *
    * 输入参数  : pstWriteData 记录与SPI数据传输有关信息的结构体指针，成员包括
    要读写的SPI号，片选号，传输模式等
    *             pSendData    存储接收的数据缓冲区指针
    *             u32Length    待接收的数据长度
    *
    * 输出参数  : 无
    *
    * 返 回 值  : BSP_OK    接收成功
    *             ERROR 接收失败
    *****************************************************************************/
    BSP_S32 BSP_SPI_Write(SPI_DATA_HANDLE_S *pstWriteData,BSP_VOID *pSendData, BSP_U32 u32Length)
    {
        BSP_S32 s32State;
        BSP_U32 u32LoopNum = 0;
        BSP_U32 u32RegValue = 0;
        SPI_DEV_ID_E enTemSpiId = SPI_ID0;
        SPI_DEV_CS_E enTemSpiCs = SPI_DEV_CS0;
        SPI_SCPOL_E  enTemScpol = SPI_SCPOL_LOW;
        BSP_U32 u32TemBaseAddr = 0;
        SPI_DATA_LEN_E enDataLen;
        EX_REPORT_SPI_S stReportSpi;

        /*参数判断*/
        if((NULL == pSendData)||(u32Length < 1)||(NULL == pstWriteData)
            ||(pstWriteData->enSpiID >= SPI_ID_MAX) || (pstWriteData->enSpiID < SPI_ID0)
            ||(pstWriteData->enCsID >= SPI_DEV_CS_MAX) || (pstWriteData->enCsID < SPI_DEV_CS0)
            ||(pstWriteData->enMode >= SPI_SENDMOD_MAX)
            ||(pstWriteData->enMode < SPI_SENDMOD_POLLING))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write Paramater ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_INVALID_PARA;
        }

        enTemSpiId = pstWriteData->enSpiID;
        enTemSpiCs = pstWriteData->enCsID;
        enTemScpol = stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enSpiScpol;
        enDataLen  =  stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enDataLen;
        u32TemBaseAddr = stSpiDev[enTemSpiId].u32SpiBaseAddr;

        /*如果SPI没有初始化，返回错误*/
        if(BSP_FALSE == g_bSpiInit)
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. BSP_FALSE == g_bSpiInit. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_MODULE_NOT_INITED;
        }

        /*锁定SPI总线*/
        if (BSP_OK != SPI_Lock(enTemSpiId))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. BSP_OK != SPI_Lock. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_SEM_LOCK;
        }

        /*获取当前哪片片选连接的设备占有SPI总线,如果是不是要操作的设备，需要重新设置SPI*/
        s32State = SPI_SetSpiOwner((SPI_DEV_S *)pstWriteData);/*lint !e740*/
        if(BSP_OK != s32State)
        {
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            return s32State;
        }

        /*禁止SPI从设备*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_SLAVE_EN_OFFSET,0x0);

        /*禁止SPI数据传输*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_EN_OFFSET,0x0);

        /*配置寄存器，将数据传输模式设置为发送数据*/
        switch(stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enSpiProt)
        {
            /*使用NSM协议时，配置MWCR的mod位*/
        case SPI_PROT_NSM:
            BSP_REG_READ(u32TemBaseAddr,SPI_MWCTRL_OFFSET,u32RegValue);
            u32RegValue |= 0x2;
            BSP_REG_WRITE(u32TemBaseAddr,SPI_MWCTRL_OFFSET,u32RegValue);
            break;
            /*使用SPI协议时，修改CTRL0寄存器的Tmod位*/
        case SPI_PROT_SPI:
            BSP_REG_READ(u32TemBaseAddr,SPI_CTRL0_OFFSET,u32RegValue);
            u32RegValue &= (~0x3cf);
            u32RegValue = (u32RegValue | 0x100 | 0x40 | enDataLen | (enTemScpol << 0x7));
            BSP_REG_WRITE(u32TemBaseAddr,SPI_CTRL0_OFFSET,u32RegValue);
            break;
            /*当前代码实现不支持SSP协议*/
        case SPI_PROT_SSP:
        default:
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. Prot not support. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_PROT_INVALID;
        }

        /*使能Master，这样向数据寄存器写入数据时，数据会自己转移到FIFO中*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_EN_OFFSET,0x1);

        /*向数据寄存器中写入数据，数据自己转移到FIFO中，由于没有使能从设备，
        数据会暂时存放在FIFO中，而不会从FIFO中发送到从设备*/
        /* Polling方式写入数据*/
        if(SPI_SENDMOD_POLLING == pstWriteData->enMode)
        {
            s32State = SPI_POLL_Send((SPI_DEV_S *)pstWriteData,(BSP_VOID *)pSendData,u32Length);/*lint !e740*/
            if(BSP_OK != s32State)
            {
                if(BSP_OK != SPI_UnLock(enTemSpiId))
                {
                    return BSP_ERR_SPI_SEM_UNLOCK;
                }
                BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. SPI_POLL_Send ERROR. \n",0,0,0,0,0,0);
                return s32State;
            }
        }
        /*DMA方式写入数据不支持*/
        else
        {
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. Not Support Transmit mode. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_TMOD_INVALID;
        }

        /* 使能从设备，数据开始从FIFO传输到从设备*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_SLAVE_EN_OFFSET,0x1 << enTemSpiCs);

        /* 等待Transmit FIFO空*/
        while((0x4 != (SPI_STATUS & 0x4)) && (u32LoopNum < 20000))
        {
            u32LoopNum++;
        }

        if(20000 == u32LoopNum)
        {
            /* 异常状态上报给平台*/
            if (NULL != g_pBSPRegExEventFunc)
            {
                stReportSpi.u16FrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SYSFRM_ADDR);
                stReportSpi.u16SubFrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SUBFRM_ADDR);
                /* SPI写入时超时*/
                stReportSpi.enReportSPI = EX_SPI_WRITE_TIMEOUT;
                (BSP_VOID)g_pBSPRegExEventFunc(EX_MODU_SPI, sizeof(EX_REPORT_SPI_S), &stReportSpi);
            }

            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. Bus not Free. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_RETRY_TIMEOUT;
        }
        u32LoopNum = 0;

        /* 等待总线空闲，总线空闲时表示数据传输完毕*/
        while((0x0 != (SPI_STATUS & 0x1))&&(u32LoopNum < 20000))
        {
            u32LoopNum++;
        }

        if(20000 == u32LoopNum)
        {
            /* 异常状态上报给平台*/
            if (NULL != g_pBSPRegExEventFunc)
            {
                stReportSpi.u16FrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SYSFRM_ADDR);
                stReportSpi.u16SubFrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SUBFRM_ADDR);
                /* SPI写入时超时*/
                stReportSpi.enReportSPI = EX_SPI_WRITE_TIMEOUT;
                (BSP_VOID)g_pBSPRegExEventFunc(EX_MODU_SPI, sizeof(EX_REPORT_SPI_S), &stReportSpi);
            }

            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. Bus not Free. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_RETRY_TIMEOUT;
        }

        /* 释放SPI总线*/
        if(BSP_OK != SPI_UnLock(enTemSpiId))
        {
            return BSP_ERR_SPI_SEM_UNLOCK;
        }

        return BSP_OK;

    }

    /*****************************************************************************
    * 函 数 名  : BSP_SPI_Read
    *
    * 功能描述  : 通过SPI读取设备数据
    *
    * 输入参数  : pstReadData  记录与SPI数据传输有关信息的结构体指针，成员包括
    要读写的SPI号，片选号，传输模式等。
    *             u32Length    待接收的数据长度
    *
    * 输出参数  : pRecData     存储接收的数据缓冲区指针。
    *
    * 返 回 值  : BSP_OK    接收成功
    *             ERROR 接收失败
    *****************************************************************************/
    BSP_S32 BSP_SPI_Read(SPI_DATA_HANDLE_S *pstReadData,BSP_VOID *pRecData, BSP_U32 u32Length)
    {
        BSP_S32 s32State;
        BSP_U32 u32RegValue = 0;
        BSP_U32 u32LoopNum = 0;
        SPI_DEV_ID_E enTemSpiId = SPI_ID0;
        SPI_DEV_CS_E enTemSpiCs = SPI_DEV_CS0;
        SPI_SCPOL_E  enTemScpol = SPI_SCPOL_LOW;
        BSP_U32 u32TemBaseAddr = 0;
        SPI_DATA_LEN_E enDataLen;
        EX_REPORT_SPI_S stReportSpi;

        /*参数判断*/
        if((NULL == pRecData) || (0 == u32Length)
            || (pstReadData->enCsID >= SPI_DEV_CS_MAX) || (pstReadData->enCsID < SPI_DEV_CS0)
            || (pstReadData->enSpiID >= SPI_ID_MAX) || (pstReadData->enSpiID < SPI_ID0)
            || (pstReadData->enMode >= SPI_SENDMOD_MAX)
            || (pstReadData->enMode < SPI_SENDMOD_POLLING)
            || (NULL == pstReadData->pvCmdData) || (pstReadData->u32length == 0))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. Paramater ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_INVALID_PARA;
        }

        enTemSpiId = pstReadData->enSpiID;
        enTemSpiCs = pstReadData->enCsID;
        enTemScpol = stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enSpiScpol;
        enDataLen = stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enDataLen;
        u32TemBaseAddr = stSpiDev[enTemSpiId].u32SpiBaseAddr;

        /*如果SPI没有初始化，返回错误*/
        if(BSP_FALSE == g_bSpiInit)
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. BSP_FALSE == g_bSpiInit. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_MODULE_NOT_INITED;
        }

        /* SPI总线锁定 */
        if (BSP_OK != SPI_Lock(enTemSpiId))
        {
            return BSP_ERR_SPI_SEM_LOCK;
        }

        /*获取当前哪片片选连接的设备占有SPI总线,如果是不是要操作的设备，需要重新设置SPI*/

        s32State = SPI_SetSpiOwner((SPI_DEV_S *)pstReadData);/*lint !e740*/
        if(BSP_OK != s32State)
        {
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            return s32State;
        }

        /*禁止Slave*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_SLAVE_EN_OFFSET,0x0);

        /*禁止Master*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_EN_OFFSET,0x0);

        /* 设置接收数据的数目*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_CTRL1_OFFSET,(u32Length - 1));

        switch(stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enSpiProt)
        {
            /*使用NSM协议时，配置MWCR的mod位为接收模式*/
        case SPI_PROT_NSM:
            BSP_REG_READ(u32TemBaseAddr,SPI_MWCTRL_OFFSET,u32RegValue);
            u32RegValue &= (~0x2);
            BSP_REG_WRITE(u32TemBaseAddr,SPI_MWCTRL_OFFSET,u32RegValue);

            /*使能Master，这样向数据寄存器写入数据时，数据会自己转移到FIFO中*/
            BSP_REG_WRITE(u32TemBaseAddr,SPI_EN_OFFSET,0x1);

            /* NSM协议，接收数据前，需要向从设备发送命令字*/
            if(SPI_SENDMOD_POLLING == pstReadData->enMode)
            {
                /*lint -e740*/
                s32State = SPI_POLL_Send((SPI_DEV_S *)pstReadData,(BSP_VOID *)pstReadData->pvCmdData,\
                    pstReadData->u32length);
                if(BSP_OK != s32State)
                {
                    if(BSP_OK != SPI_UnLock(enTemSpiId))
                    {
                        return BSP_ERR_SPI_SEM_UNLOCK;
                    }
                    BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. SPI_POLL_Send ERROR. \n",0,0,0,0,0,0);
                    return s32State;
                }
            }
            else
            {
                if(BSP_OK != SPI_UnLock(enTemSpiId))
                {
                    return BSP_ERR_SPI_SEM_UNLOCK;
                }
                BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. DMA mode not Support. \n",0,0,0,0,0,0);
                return BSP_ERR_SPI_TMOD_INVALID;
            }

            break;

            /* 使用SPI协议*/
        case SPI_PROT_SPI:
            /* 修改CTRL0的TMOD位，将数据传输模式设置为接收数据*/
            BSP_REG_READ(u32TemBaseAddr,SPI_CTRL0_OFFSET,u32RegValue);

            u32RegValue &= (~0x3cf);
            u32RegValue = (u32RegValue | 0x40 | enDataLen | (enTemScpol << 0x7));
            BSP_REG_WRITE(u32TemBaseAddr,SPI_CTRL0_OFFSET,u32RegValue);

            /*使能Master*/
            BSP_REG_WRITE(u32TemBaseAddr,SPI_EN_OFFSET,0x1);

            if(SPI_SENDMOD_POLLING == pstReadData->enMode)
            {
                s32State = SPI_POLL_Send((SPI_DEV_S *)pstReadData,(BSP_VOID *)pstReadData->pvCmdData,\
                    pstReadData->u32length);
                if(BSP_OK != s32State)
                {
                    if(BSP_OK != SPI_UnLock(enTemSpiId))
                    {
                        return BSP_ERR_SPI_SEM_UNLOCK;
                    }
                    BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. SPI_POLL_Send ERROR. \n",0,0,0,0,0,0);
                    return s32State;
                }
            }
            else
            {
                if(BSP_OK != SPI_UnLock(enTemSpiId))
                {
                    return BSP_ERR_SPI_SEM_UNLOCK;
                }
                BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. DMA mode not Support. \n",0,0,0,0,0,0);
                return BSP_ERR_SPI_TMOD_INVALID;
            }

            break;
            /*lint +e740*/

            /* 暂不支持SSP协议*/
        case SPI_PROT_SSP:
        default:
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. Prot not support. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_PROT_INVALID;
        }

        /* 使能从设备，命令字传输到从设备，从设备解析命令字后，发送数据到SPI*/
        BSP_REG_WRITE(u32TemBaseAddr,SPI_SLAVE_EN_OFFSET,0x1<<enTemSpiCs);

        /* 等待Transmit FIFO空*/
        while((0x4 != (SPI_STATUS & 0x4)) && (u32LoopNum < 20000))
        {
            u32LoopNum++;
        }

        if(20000 == u32LoopNum)
        {
            /* 异常状态上报给平台*/
            if (NULL != g_pBSPRegExEventFunc)
            {
                stReportSpi.u16FrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SYSFRM_ADDR);
                stReportSpi.u16SubFrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SUBFRM_ADDR);
                /* SPI读取时超时*/
                stReportSpi.enReportSPI = EX_SPI_READ_TIMEOUT;
                (BSP_VOID)g_pBSPRegExEventFunc(EX_MODU_SPI, sizeof(EX_REPORT_SPI_S), &stReportSpi);
            }
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. Bus not Free. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_RETRY_TIMEOUT;
        }
        u32LoopNum = 0;

        /* 等待总线空闲，总线空闲时表示数据传输完毕*/
        while((0x0 != (SPI_STATUS & 0x1))&&(u32LoopNum < 20000))
        {
            u32LoopNum++;
        }

        if(20000 == u32LoopNum)
        {
            /* 异常状态上报给平台*/
            if (NULL != g_pBSPRegExEventFunc)
            {
                stReportSpi.u16FrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SYSFRM_ADDR);
                stReportSpi.u16SubFrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SUBFRM_ADDR);
                /* SPI读取时超时*/
                stReportSpi.enReportSPI = EX_SPI_READ_TIMEOUT;
                (BSP_VOID)g_pBSPRegExEventFunc(EX_MODU_SPI, sizeof(EX_REPORT_SPI_S), &stReportSpi);
            }

            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. Bus not Free. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_RETRY_TIMEOUT;
        }

        /* Polling方式接收数据*/
        if(SPI_SENDMOD_POLLING == pstReadData->enMode)
        {
            s32State = SPI_POLL_Receive((SPI_DEV_S *)pstReadData,(BSP_VOID *)pRecData, u32Length);/*lint !e740*/
            if(BSP_OK != s32State)
            {
                if(BSP_OK != SPI_UnLock(enTemSpiId))
                {
                    return BSP_ERR_SPI_SEM_UNLOCK;
                }
                BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. SPI_POLL_Receive ERROR. \n",0,0,0,0,0,0);
                return s32State;
            }
        }
        /* 当前代码不支持DMA方式接收数据*/
        else
        {
            if(BSP_OK != SPI_UnLock(enTemSpiId))
            {
                return BSP_ERR_SPI_SEM_UNLOCK;
            }
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. DMA mode not Support. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_TMOD_INVALID;
        }

        /* 释放SPI总线*/
        if(BSP_OK != SPI_UnLock(enTemSpiId))
        {
            return BSP_ERR_SPI_SEM_UNLOCK;
        }

        return BSP_OK;

    }

    /*****************************************************************************
    * 函 数 名  : BSP_SPI_GetAttr
    *
    * 功能描述  : 获取当前SPI设置的属性
    *
    * 输入参数  : enSpiId    要查询的SPI号。
    *
    * 输出参数  : pstDevAttr 存放SPI属性的结构体指针。
    *
    * 返 回 值  : BSP_OK    获取属性成功
    *             ERROR 获取属性失败
    *****************************************************************************/
    BSP_S32 BSP_SPI_GetAttr(SPI_DEV_ID_E enSpiId, SPI_DEV_ATTR_S *pstDevAttr)
    {
        BSP_U16 u16Ctrl0Val;
        BSP_U16 u16BautRate;
        BSP_U32 u32TemBaseAddr = stSpiDev[enSpiId].u32SpiBaseAddr;

        /*参数判断*/
        if((enSpiId < SPI_ID0)|| (enSpiId >= SPI_ID_MAX) || (NULL == pstDevAttr))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. Paramater ERROR. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_INVALID_PARA;
        }

        /*如果SPI没有初始化，返回错误*/
        if(BSP_FALSE == g_bSpiInit)
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Read. BSP_FALSE == g_bSpiInit. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_MODULE_NOT_INITED;
        }

        if (BSP_OK != SPI_Lock(enSpiId))
        {
            BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. BSP_OK != SPI_Lock. \n",0,0,0,0,0,0);
            return BSP_ERR_SPI_SEM_LOCK;
        }

        memset((BSP_VOID *)pstDevAttr, 0x0, sizeof(SPI_DEV_ATTR_S));

        /*获取SPI寄存器中的值*/
        BSP_REG16_READ(u32TemBaseAddr, SPI_CTRL0_OFFSET, u16Ctrl0Val);
        BSP_REG16_READ(u32TemBaseAddr, SPI_BAUD_OFFSET, u16BautRate);

        /*将SPI的属性存储在pstDevAttr中*/
        pstDevAttr->enDataLen = (SPI_DATA_LEN_E)(u16Ctrl0Val & 0xF);
        pstDevAttr->enCommandLen = (SPI_COMMAND_LEN_E)(u16Ctrl0Val >> SPI_COMM_SHIFT_BITS);
        pstDevAttr->enSpiProt = (SPI_PROT_E)((u16Ctrl0Val>>SPI_PROT_SHIFT_BITS) & 0x3);
        pstDevAttr->u16SpiBaud = u16BautRate;

        if(BSP_OK != SPI_UnLock(enSpiId))
        {
            return BSP_ERR_SPI_SEM_UNLOCK;
        }

        return BSP_OK;
    }

    /*****************************************************************************
    * 函 数 名  : SPI_SetSpiOwner
    *
    * 功能描述  : 设置当前SPI的属性
    *
    * 输入参数  : enSpiId 要查询的SPI号。
    *
    * 输出参数  : 无
    *
    * 返 回 值  : SPI当前使用的片选号
    *****************************************************************************/
    BSP_S32 SPI_SetSpiOwner(SPI_DEV_S *pstSpiID)
    {
        BSP_U32 u32Ctrl0 = 0;
        SPI_DEV_ID_E enTemSpiId = pstSpiID->enSpiId;
        SPI_DEV_CS_E enTemSpiCs = pstSpiID->enSpiCs;
        BSP_U32 u32TemBaseAddr = stSpiDev[enTemSpiId].u32SpiBaseAddr;

        if(enTemSpiCs != (stSpiDev[enTemSpiId].enSpiOwner))
        {
            /* 如果要写的片选之前没有使用过，无法获取设备属性，返回错误*/
            if(BSP_FALSE == stSpiDev[enTemSpiId].abFlag[enTemSpiCs])
            {
                if(BSP_OK != SPI_UnLock(enTemSpiId))
                {
                    return BSP_ERR_SPI_SEM_UNLOCK;
                }
                BSP_TRACE(BSP_LOG_LEVEL_ERROR,BSP_MODU_SPI,"BSP_SPI_Write. SPI Configed Wrong, please Configure. \n",0,0,0,0,0,0);
                return BSP_ERR_SPI_ATTR_NOTSET;
            }

            /* 如果要写的片选之前用过,首先判断要写的片选连接的设备和当前正使用的片选连接
            的设备属性是否一致，如果一致，不需要重新配置SPI,否则，重新配置SPI*/
            if(0 != memcmp(&(stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs]),
                &(stSpiDev[enTemSpiId].stSpiConf[stSpiDev[enTemSpiId].enSpiOwner]),
                sizeof(SPI_DEV_ATTR_S)))
            {
                u32Ctrl0 = stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enCommandLen << SPI_COMM_SHIFT_BITS\
                    | stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enSpiProt << SPI_PROT_SHIFT_BITS \
                    | stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].enDataLen;

                /*禁止SPI数据传输*/
                BSP_REG_WRITE(u32TemBaseAddr, SPI_EN_OFFSET, 0x0);
                /*配置ctrl0寄存器，包括命令字长度，数据宽度*/
                BSP_REG_WRITE(u32TemBaseAddr, SPI_CTRL0_OFFSET,u32Ctrl0);
                /*配置SPI波特率*/
                BSP_REG_WRITE(u32TemBaseAddr, SPI_BAUD_OFFSET,stSpiDev[enTemSpiId].stSpiConf[enTemSpiCs].u16SpiBaud);
            }

            /*重新设置SPI使用的片选号*/
            stSpiDev[enTemSpiId].enSpiOwner = enTemSpiCs;
        }

        return BSP_OK;
    }

    BSP_VOID BSP_ReportTest()
    {

        EX_REPORT_SPI_S stReportSpi;

        if (NULL != g_pBSPRegExEventFunc)
        {
            stReportSpi.u16FrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SYSFRM_ADDR);
            stReportSpi.u16SubFrameNo = (BSP_U16)(*(volatile BSP_U32 *)BSP_SDA_PHY_SUBFRM_ADDR);
            /* SPI读取时超时*/
            stReportSpi.enReportSPI = EX_SPI_READ_TIMEOUT;
            logMsg("\r report spi read timeout!\n",0,0,0,0,0,0);
            (BSP_VOID)g_pBSPRegExEventFunc(EX_MODU_SPI, sizeof(EX_REPORT_SPI_S), &stReportSpi);
        }
    }

    BSP_VOID SPI_GetGlobalVariable()
    {
        BSP_S32 i = 0;
        BSP_S32 j = 0;

        printf(" ============================= g_bSpiInit =============================\n");
        printf("\t g_bSpiInit:        0x%08x\n", (int)g_bSpiInit);

        for(i = 0; i < SPI_ID_MAX; i++)
        {
            printf("\n ============================= stSpiDev[%d]=============================\n",i);

            printf("\t stSpiDev[%d].u32SpiBaseAddr:       0x%08x\n", i,(int)stSpiDev[i].u32SpiBaseAddr);
            printf("\t stSpiDev[%d].enSpiOwner:           0x%08x\n", i,(int)stSpiDev[i].enSpiOwner);
            printf("\t stSpiDev[%d].SpiSemId:             0x%08x\n", i,(int)stSpiDev[i].SpiSemId);

            for(j=0; j<SPI_DEV_CS_MAX; j++)
            {
                printf("\t stSpiDev[%d].abFlag[%d]:            0x%08x\n", i,j,(int)stSpiDev[i].abFlag[j]);
            }

            for(j=0; j<SPI_DEV_CS_MAX; j++)
            {
                if(stSpiDev[i].abFlag[j])
                {
                    printf("\n ====================== stSpiDev[%d].stSpiConf[%d] ======================\n",i,j);
                    printf("\t stSpiDev[%d].stSpiConf[%d].enCommandLen:       0x%08x\n",\
                        i,j,(int)stSpiDev[i].stSpiConf[j].enCommandLen);
                    printf("\t stSpiDev[%d].stSpiConf[%d].enDataLen:          0x%08x\n",\
                        i,j,(int)stSpiDev[i].stSpiConf[j].enDataLen);
                    printf("\t stSpiDev[%d].stSpiConf[%d].enSpiProt:          0x%08x\n",\
                        i,j,(int)stSpiDev[i].stSpiConf[j].enSpiProt);
                    printf("\t stSpiDev[%d].stSpiConf[%d].u16SpiBaud:         0x%08x\n",\
                        i,j,(int)stSpiDev[i].stSpiConf[j].u16SpiBaud);
                }
            }
        }
        printf("\n ======================================================================\n");
        return ;
    }

    BSP_VOID SPI_GetRegistValue()
    {
        BSP_S32 i = 0;

        printf(" ================== Register Value =====================\n");

        for(i = 0; i < SPI_ID_MAX; i++)
        {
            printf("\t SPI[%d] control0  register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_CTRL0_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_CTRL0_OFFSET));
            printf("\t SPI[%d] control1  register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_CTRL1_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_CTRL1_OFFSET));
            printf("\t SPI[%d] Enable    register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_EN_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_EN_OFFSET));
            printf("\t SPI[%d] Microwire register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_MWCTRL_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_MWCTRL_OFFSET));
            printf("\t SPI[%d] slaveEn   register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_SLAVE_EN_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_SLAVE_EN_OFFSET));
            printf("\t SPI[%d] pautslect register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_BAUD_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_BAUD_OFFSET));
            printf("\t SPI[%d] Tx ftlr   register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_TXFTL_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_TXFTL_OFFSET));
            printf("\t SPI[%d] Rx ftlr   register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_RXFTL_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_RXFTL_OFFSET));
            printf("\t SPI[%d] Tx fifo   register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_TXFL_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_TXFL_OFFSET));
            printf("\t SPI[%d] Rx fifo   register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_RXFL_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_RXFL_OFFSET));
            printf("\t SPI[%d] state     register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_STATUS_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_STATUS_OFFSET));
            printf("\t SPI[%d] mask int  register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_INT_MASK_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_INT_MASK_OFFSET));
            printf("\t SPI[%d] int stat  register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_INT_STATUS_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_INT_STATUS_OFFSET));
            printf("\t SPI[%d] rawintstu register:        0x%x = 0x%x\n", i, (int)(stSpiDev[i].u32SpiBaseAddr + SPI_RAW_INT_STATUS_OFFSET),
                BSP_REG(stSpiDev[i].u32SpiBaseAddr, SPI_RAW_INT_STATUS_OFFSET));
            printf("\n");

        }

        printf(" =====================================================\n\n");

        return ;
    }
#endif




    //启动或禁止SLIC中断
    void EnableSlicIrq( int bEnable )
    {
        if( bEnable )
        {
            SLIC_GPIO_REG_SETBIT(g_gpioBase, HI_GPIO_INTEN, GPIO_23);
        }
        else
        {
            SLIC_GPIO_REG_CLRBIT(g_gpioBase, HI_GPIO_INTEN, GPIO_23);
        }
    }

    //add new interface

    INT32 HiSpiInit(UINT32 spiNo, UINT8 dataWidth, UINT8 clk, UINT8 baud)
    {
        UINT8 ucDataWidth = 0;
        UINT8 ucSCPOL = 0;
        UINT8 ucSCPH = 0;

        ucDataWidth = dataWidth - 1;
        ucSCPOL = (clk & 0x02) >> 1;
        ucSCPH = clk & 0x01;

        if ((0 != spiNo) && (1 != spiNo))
        {
            return ERROR;
        }
        #if 0

        /* PMU使用SPI0 CS0 */
        if(0 == spiNo)
        {
            BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x988, SSP0_CTRL, 1, 1);     // IO复用为SSP0
            BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x944, SSP0_WIRE_4, 2, 3);  // 使能SSP0
        }
        else
        {
            /* 使用SPI1_CTRL */
            BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x988, SSP1_CTRL, 1, 1);     // IO复用为SSP1
            BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x944, SSP1_WIRE_4, 2, 3);  // 使能SSP1
        }
        #endif

        spiBase[0] = (unsigned long)ioremap(spiBase[0], ECS_SPI0_SIZE);
        spiBase[1] = (unsigned long)ioremap(spiBase[1], ECS_SPI1_SIZE);

        /* 禁止SPI Slave*/
        OUTREG32(SPI_SLAVE_EN(spiNo), 0);

        /* 禁止SPI Master*/
        OUTREG32(SPI_EN(spiNo), 0);

        /* 配置ctrl0寄存器，命令字长度为1，数据宽度为8,上升沿触发,低电平有效 */
        OUTREG32(SPI_CTRL0(spiNo), (ucDataWidth<<SPI_CTRL0_DFS_BITPOS)    \
            | (ucSCPH<<SPI_CTRL0_SCPH_BITPOS)    \
            | (ucSCPOL<<SPI_CTRL0_SCPOL_BITPOS)   \
            | (0 << SPI_CTRL0_FRF_BITPOS)   \
            | (0x00<<SPI_CFS_BITPOS));

        /* 配置SPI波特率为SSI CLK的1/24，即48/24=2MHz */
        OUTREG32(SPI_BAUD(spiNo), baud);

        /*禁止所有中断信号*/
        OUTREG32(SPI_IMR(spiNo), 0);

        /*屏蔽DMA传输*/
        OUTREG32(SPI_DMAC(spiNo), 0);

        return OK;
    }

#define SPI_MAX_DELAY_TIMES 0x10000
#define SPI_CS_DEV          (1<<1)     /* CS1 for EEPROM/SFlash */
#define SPI_CS_PMU          (1<<0)     /* CS0,for eMMC/MMC/SD */

    INT32 HiSpiRecv (UINT32 spiNo, UINT32 cs, UINT8* prevData, UINT32 recvSize,UINT8* psendData,UINT32 sendSize )
    {
        UINT8 *pRh;
        UINT8 *pSh;
        UINT32  i;
        UINT32 ulLoop = SPI_MAX_DELAY_TIMES;

        if (((0 != spiNo) && (1 != spiNo))
            || (SPI_CS_PMU != cs && SPI_CS_DEV != cs)
            || (NULL == psendData) || (NULL == prevData) || (0 == recvSize) || (0 == sendSize))
        {
            return ERROR;
        }

        pRh = prevData;
        pSh = psendData;

        /* 禁止SPI Slave*/
        OUTREG32(SPI_SLAVE_EN(spiNo), 0);

        /* 禁止SPI Master*/
        OUTREG32(SPI_EN(spiNo), 0);

        /* 设置成EEPROM读模式 */
        SETBITVALUE32(SPI_CTRL0(spiNo), SPI_CTRL0_TMOD_BITMASK, SPI_CTRL0_TMOD_SEND_RECV);

        /* 设置接收数据的数目*/
        OUTREG32(SPI_CTRL1(spiNo),(recvSize-1));

        /*使能SPI Master*/
        OUTREG32(SPI_EN(spiNo), 1);

        /*使能SPI Slave*/
        OUTREG32(SPI_SLAVE_EN(spiNo), cs);

        /* 发送命令 */
        for(i = 0; i < sendSize; i++)
        {
            /* 等待发送FIFO非满 */
            while(!(INREG32(SLIC_SPI_STATUS(spiNo)) & SPI_STATUS_TXNOTFULL)
                && (0 != --ulLoop))
            {
            }

            if(0 == ulLoop)
            {
                return ERROR;
            }

            OUTREG32(SPI_DR(spiNo), *pSh++);
        }

        /*将发送FIFO中的数据全部发出*/
        while(!(INREG32(SLIC_SPI_STATUS(spiNo)) & SPI_STATUS_TXEMPTY)
            && (0 != --ulLoop))
        {
        }

        if(0 == ulLoop)
        {
            return ERROR;
        }

        /* 接收数据 */
        for(i = 0; i < recvSize; i++)
        {
            ulLoop = SPI_MAX_DELAY_TIMES;
            /* 等待读取到数据 */
            while(!(INREG32(SLIC_SPI_STATUS(spiNo)) & SPI_STATUS_RXNOTEMPTY)
                && (0 != --ulLoop))
            {
            }

            if(0 == ulLoop)
            {
                return ERROR;
            }

            *pRh++ = (UINT8)INREG32(SPI_DR(spiNo));
        }

        return OK;
    }


    INT32 HiSpiSend (UINT32 spiNo, UINT32 cs, UINT8* pData, UINT32 ulLen)
    {
        UINT8 *pSh;
        UINT32  i;
        UINT32 ulLoop = SPI_MAX_DELAY_TIMES;
        UINT32 ulVal;

        if (((0 != spiNo) && (1 != spiNo))
            || (SPI_CS_PMU != cs && SPI_CS_DEV != cs)
            || (NULL == pData) || (0 == ulLen))
        {
            return ERROR;
        }

        pSh = (UINT8*)pData;

        /* 禁止SPI Slave*/
        OUTREG32(SPI_SLAVE_EN(spiNo), 0);

        /* 禁止SPI Master*/
        OUTREG32(SPI_EN(spiNo), 0);

        /* 设置成发送模式 */
        SETBITVALUE32(SPI_CTRL0(spiNo), SPI_CTRL0_TMOD_BITMASK, SPI_CTRL0_TMOD_SEND);

        /*使能SPI Master*/
        OUTREG32(SPI_EN(spiNo), 1);

        /*使能SPI Slave*/
        OUTREG32(SPI_SLAVE_EN(spiNo), cs);

        /* 发送命令 */
        for(i = 0; i < ulLen; i++)
        {
            /* 等待发送FIFO非满 */
            while(!(INREG32(SLIC_SPI_STATUS(spiNo)) & SPI_STATUS_TXNOTFULL)
                && (0 != --ulLoop))
            {
            }

            if(0 == ulLoop)
            {
                return ERROR;
            }
            OUTREG32(SPI_DR(spiNo), *pSh++);
        }

        /*将发送FIFO中的数据全部发出,且不BUSY*/
        ulLoop = SPI_MAX_DELAY_TIMES;
        ulVal = INREG32(SLIC_SPI_STATUS(spiNo));
        while(((!(ulVal & SPI_STATUS_TXEMPTY)) || (ulVal & SPI_STATUS_BUSY))
            && (0 != --ulLoop))
        {
            ulVal = INREG32(SLIC_SPI_STATUS(spiNo));
        }

        if(0 == ulLoop)
        {
            return ERROR;
        }

        return OK;
    }

    void SlicPclkFsyncInit(void)
    {
        printk("enter SlicPclkFsyncInit\n");

        //PCM管脚复用
        //BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x988, 7, 2, 0x1);     // [8:7] = 01b
        //BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x988, 12, 2, 0x0);     // [13:12] = 00b

        //打开SIO工作时钟门控
        BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0c, 0x10);  //CRG_CTRL3[4]时钟使能

        //关闭SIO_SOC的位流时钟XCLK
        BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x28, 0x2000);  //CRG_CTRL10[13]时钟关闭

        //配置SIO为主模式
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x30, 4, 3, 0x6);     // [6:5:4] = 110b

        //时钟分频 122880k/240/64=8k bit0:15=240 bit16:27=64
        //FSYNC配置成8K PCLK为512K
        BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x44, 0x004000F0);

        //设置PCM接口管脚控制为master模式
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0940, 20, 1, 1);
    }

    void SlicPcmSlaveModeEnable(void)
    {
        //设置PCM为Slave模式
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0940, 20, 1, 0);
    }

    void SlicPcmMasterModeEnable(void)
    {
        //设置PCM为Master模式
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0940, 20, 1, 1);
    }

    void ClosePeripheralClock(BSP_U8 ucPeripheralType)
    {
        switch (ucPeripheralType)
        {
            case EN_PERIPHERAL_USB:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x10, 27, 1, 1);       
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x10, 13, 1, 1);
                break;
            }
            case EN_PERIPHERAL_HSIC:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x04, 13, 1, 1);            
                break;
            }
            case EN_PERIPHERAL_MAC:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x04, 10, 1, 1);
                break;
            }
            case EN_PERIPHERAL_SIO:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x10, 4, 1, 1);
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x28, 13, 1, 1);
                break;
            }
            default:
            {
                printk("invalid para %d\n", ucPeripheralType);
                break;
            }
        }
    }

    void OpenPeripheralClock(BSP_U8 ucPeripheralType)
    {
        switch (ucPeripheralType)
        {
            case EN_PERIPHERAL_USB:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0C, 27, 1, 1);       
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0C, 13, 1, 1);
                break;
            }
            case EN_PERIPHERAL_HSIC:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x00, 13, 1, 1);            
                break;
            }
            case EN_PERIPHERAL_MAC:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x00, 10, 1, 1);
                break;
            }
            case EN_PERIPHERAL_SIO:
            {
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0C, 4, 1, 1);
                BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x24, 13, 1, 1);
                break;
            }
            default:
            {
                printk("invalid para %d\n", ucPeripheralType);
                break;
            }
        }
    }

    /*调试接口*/
    void DebugWriteHiReg(BSP_U32 base, BSP_U32 reg, BSP_U32 data)
    {
        BSP_REG_WRITE(IO_ADDRESS(base), reg, data);
    }

    void DebugSetHiRegBits(BSP_U32 base, BSP_U32 reg, BSP_U32 pos, BSP_U32 bits, BSP_U32 val)
    {
        BSP_REG_SETBITS(IO_ADDRESS(base), reg, pos, bits, val);
    }

    BSP_U32 DebugReadHiReg(BSP_U32 base, BSP_U32 reg, BSP_U32 result)
    {
        return BSP_REG_READ(IO_ADDRESS(base), reg, result);
    }
    /*end*/    

    UINT32 DebugReadSlicPclkFsync(void)
    {
        UINT32 ulReg = 0;
        return BSP_REG_READ(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x44, ulReg);
    }

    void SlicSioInit(void)
    {
        BSP_U32 ulRegVal;
        BSP_U32 ulBitVal = 0;

        printk("enter SlicSioInit\n");

        g_ulSioBaseAddr = (unsigned long)ioremap(SIO_BASE_ADDR_PHY, SIO_SIZE);

        printk("g_ulSioBaseAddr=0x%x\n", g_ulSioBaseAddr);

        BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x230, 0x0);
        BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x234, 0x1319ec0);
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x238, 0, 1, 1);
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x238, 0, 1, 0);

        ulBitVal = BSP_REG_GETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x238, 31, 1);
        while(1 != ulBitVal)
        {
            ulBitVal = BSP_REG_GETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x238, 31, 1);
        }

        BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0xffff);

        BSP_REG_WRITE(SIO_INTR_CLR, 0, 0x3f);

        BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x8000);

        msleep(1);

        //打开SIO工作时钟门控
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x18, 29, 1, 1);

        //配置SIO为主模式
        BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x40c, 3, 1, 1);

        //时钟分频 122880k/60/256=8k bit0:15=60 bit16:27=256
        //BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x100, 0x0100003C);

        //时钟分频 120832k/59/256=8k bit0:15=59 bit16:27=256
        BSP_REG_WRITE(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x100, 0x0100003B);

        //设置PCM接口管脚控制为master模式
        //BSP_REG_SETBITS(IO_ADDRESS(INTEGRATOR_SC_BASE), 0x0940, 20, 1, 1);

        /*写SIO_INTR_CLR 0xffff复位SIO、禁止传输、禁止中断*/
        BSP_REG_WRITE(SIO_CTRL_CLR, 0, 0xffff);

        msleep(1);

        printk("After reset FIFO:%d\n", BSP_REG_READ(SIO_CTRL_RX_STA, 0, ulRegVal));

        /*屏蔽所有SIO中断*/
        BSP_REG_WRITE(SIO_INTR_MASK, 0, 0xffffffff);

        /*清除所有SIO中断*/
        BSP_REG_WRITE(SIO_INTR_CLR, 0, 0x3f);

        /*解复位、设置水线Tx-8(0.5)、RX-8(0.5)、使能中断*/
        BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x8088);

        /*配置为PCM模式*/
        BSP_REG_WRITE(SIO_MODE, 0, 0x1);

        /*配置为发送、接收16bit位宽*/
        BSP_REG_WRITE(SIO_DATA_WIDTH_SET, 0, 0x9);

        /*配置符号扩展禁止*/
        BSP_REG_WRITE(SIO_SIGNED_EXT, 0, 0);

        /*若中断回调函数非空，则打开中断屏蔽，挂接中断回调函数*/
        if(1/*BSP_NULL != pfIntHandleFunc*/)
        {
            /*对enIntMask取反，打开中断屏蔽*/
            BSP_REG_WRITE(SIO_INTR_MASK, 0, ~(0x14UL));
        }

        /*使能SIO接收、发送*/
        BSP_REG_WRITE(SIO_CTRL_SET, 0, 0x3000);

        msleep(1);

        printk("After en rx_tx FIFO:%d\n", BSP_REG_READ(SIO_CTRL_RX_STA, 0, ulRegVal));
    }

    INT32 SlicSpiInit(void)
    {
        INT32 lRet = 0;
        unsigned long flags;

        if(BSP_TRUE == g_bSpiInit)
        {
            return OK;
        }

        SlicPclkFsyncInit();
        msleep(100);

        spin_lock_irqsave(&slic_lock, flags);
        //lRet = HiSpiInit(1, 8, 0x03, 48);
        //SPI主频修改为256K
        lRet = HiSpiInit(0, 8, 0x03, 6);
        spin_unlock_irqrestore(&slic_lock, flags);

        if(OK == lRet)
        {
            g_bSpiInit = BSP_TRUE;
        }

        return lRet;
    }

    UINT8 SlicSpiRecv(void)
    {
        UINT8 rcvdata = 0;
        UINT8 snddata = 0;

        HiSpiRecv(0, 0x01, &rcvdata, 1, &snddata, 1);
        return rcvdata;
    }

    void SlicSpiSend(UINT8 ucSndByte)
    {
        HiSpiSend(0, 0x01, &ucSndByte, 1);
    }


    UINT8 SlicReadReg(UINT8 ucReg)
    {
        UINT8 ucRegVal = 0;
        unsigned long flags;

        if(BSP_FALSE == g_bSpiInit)
        {
            printk("slic not init\n");
            return ERROR;
        }

        spin_lock_irqsave(&slic_lock, flags);

        SlicSpiSend(0x60);
        SlicSpiSend(ucReg);
        ucRegVal = SlicSpiRecv();

        spin_unlock_irqrestore(&slic_lock, flags);

        return ucRegVal;
    }


    void SlicWriteReg(UINT8 ucReg, UINT8 ucVal)
    {
        unsigned long flags;

        if(BSP_FALSE == g_bSpiInit)
        {
            printk("slic not init\n");
            return;
        }

        spin_lock_irqsave(&slic_lock, flags);

        SlicSpiSend(0x20);
        SlicSpiSend(ucReg);
        SlicSpiSend(ucVal);

        spin_unlock_irqrestore(&slic_lock, flags);
    }

#define MAX_RAM_R_WAIT_TIMES 1000
    UINT32 SlicReadRam(UINT16 usRam)
    {
        UINT8 ucHighAddr = 0;
        UINT8 ucLowAddr = 0;
        UINT8 ucData0 = 0;
        UINT8 ucData1 = 0;
        UINT8 ucData2 = 0;
        UINT8 ucData3 = 0;
        UINT32 ulData = 0;
        UINT32 i = 0;
        unsigned long flags;

        if(BSP_FALSE == g_bSpiInit)
        {
            printk("slic not init\n");
            return ERROR;
        }

        spin_lock_irqsave(&slic_lock, flags);

        ucHighAddr = (usRam & 0x700) >> 3;
        ucLowAddr = usRam & 0xFF;

        SlicWriteReg(RAM_ADDR_HI, ucHighAddr);
        SlicWriteReg(RAM_ADDR_LO, ucLowAddr);

        for(i = 0; i < MAX_RAM_R_WAIT_TIMES; i++)
        {
            if(0 == (0x1 & SlicReadReg(RAMSTAT)))
            {
                break;
            }
        }

        if(MAX_RAM_R_WAIT_TIMES == i)
        {
            printk("SlicReadRam time out error.\n");
            spin_unlock_irqrestore(&slic_lock, flags);
            return -1;
        }

        ucData0 = SlicReadReg(RAM_DATA_B0);
        ucData1 = SlicReadReg(RAM_DATA_B1);
        ucData2 = SlicReadReg(RAM_DATA_B2);
        ucData3 = SlicReadReg(RAM_DATA_B3);

        ulData = (((UINT32)ucData0 >> 3) & 0x1F)
            | (((UINT32)ucData1 << 5) & 0x1FE0)
            | (((UINT32)ucData2 << 13) & 0x1FE000)
            | (((UINT32)ucData3 << 21) & 0x1FE00000);

        spin_unlock_irqrestore(&slic_lock, flags);

        return ulData;
    }

#define MAX_RAM_W_WAIT_TIMES 1000
    void SlicWriteRam(UINT16 usRam, UINT32 ulData)
    {
        UINT8 ucHighAddr = 0;
        UINT8 ucLowAddr = 0;
        UINT8 ucData0 = 0;
        UINT8 ucData1 = 0;
        UINT8 ucData2 = 0;
        UINT8 ucData3 = 0;
        UINT32 i = 0;
        unsigned long flags;

        if(BSP_FALSE == g_bSpiInit)
        {
            printk("slic not init\n");
            return;
        }

        spin_lock_irqsave(&slic_lock, flags);

        ucHighAddr = (usRam & 0x700) >> 3;
        ucLowAddr = usRam & 0xFF;

        ucData0 = (ulData << 3) & 0xF8;
        ucData1 = (ulData >> 5) & 0xFF;
        ucData2 = (ulData >> 13) & 0xFF;
        ucData3 = (ulData >> 21) & 0xFF;

        SlicWriteReg(RAM_ADDR_HI, ucHighAddr);

        SlicWriteReg(RAM_DATA_B0, ucData0);
        SlicWriteReg(RAM_DATA_B1, ucData1);
        SlicWriteReg(RAM_DATA_B2, ucData2);
        SlicWriteReg(RAM_DATA_B3, ucData3);

        SlicWriteReg(RAM_ADDR_LO, ucLowAddr);

        for(i = 0; i < MAX_RAM_W_WAIT_TIMES; i++)
        {
            if(0 == (0x1 & SlicReadReg(RAMSTAT)))
            {
                spin_unlock_irqrestore(&slic_lock, flags);
                return;
            }
        }

        if(MAX_RAM_W_WAIT_TIMES == i)
        {
            printk("SlicWriteRam time out error.\n");
        }

        spin_unlock_irqrestore(&slic_lock, flags);
        return;
    }


    INT32 SlicSetUserMode(BOOL on)
    {
        UINT8 data = 0;
        unsigned long flags;

        if(BSP_FALSE == g_bSpiInit)
        {
            printk("slic not init\n");
            return ERROR;
        }

        spin_lock_irqsave(&slic_lock, flags);

        if (on == TRUE)
        {
            if(g_lUserModeStatus < 2)
            {
                g_lUserModeStatus++;
            }
        }
        else
        {
            if (g_lUserModeStatus > 0)
            {
                g_lUserModeStatus--;
            }

            if (g_lUserModeStatus != 0)
            {
                printk("SlicSetUserMode not match error.\n");
                spin_unlock_irqrestore(&slic_lock, flags);
                return -1;
            }
        }

        data = SlicReadReg(USERMODE_ENABLE);
        if (((data & 1) != 0) == on)
        {
            spin_unlock_irqrestore(&slic_lock, flags);
            return 0;
        }

        SlicWriteReg(USERMODE_ENABLE, 2);
        SlicWriteReg(USERMODE_ENABLE, 8);
        SlicWriteReg(USERMODE_ENABLE, 0xe);
        SlicWriteReg(USERMODE_ENABLE, 0);

        spin_unlock_irqrestore(&slic_lock, flags);

        return 0;
    }

    //end

#ifdef E5172_SLIC_WAKEUP_DEBUG
/* GPIO管脚复用配置寄存器 */
#define CS_BASE_ADDR        0x90000000
#define CS_SIZE             0x1000
#define IOSHARE4_OFF_ADDR   0x0908

/* GPIO管脚复用控制位 */
#define GPIO_CTRL0_BIT      11
#endif    

INT32 HiInitIrq(void)
{
    if( 0==g_gpioBase )
    {
        g_gpioBase = (unsigned long)ioremap(0x90006000,  0x1000);
    }

    SLIC_GPIO_REG_CLRBIT(g_gpioBase, HI_GPIO_SWPORT_DDR, 22);     //GPIO_SWPORT_DDR 设置GPIO为输入
    SLIC_GPIO_REG_SETBIT(g_gpioBase, HI_GPIO_INTEN, 22);     // HI_GPIO_INTEN 设置中断使能
    SLIC_GPIO_REG_CLRBIT(g_gpioBase, HI_GPIO_INTMASK, 22);     // HI_GPIO_INTMASK 设置中断允许
    SLIC_GPIO_REG_SETBIT(g_gpioBase, HI_GPIO_INTTYPE_LEVEL, 22);     // HI_GPIO_INTTYPE_LEVEL 设置为电平触发
    SLIC_GPIO_REG_CLRBIT(g_gpioBase, HI_GPIO_INT_PLOARITY, 22);     // HI_GPIO_INT_PLOARITY设置为低电平

    return 0;
}

void EnSlicInt(void)
{
    SLIC_GPIO_REG_CLRBIT(g_gpioBase, HI_GPIO_INTMASK, GPIO_23);     // HI_GPIO_INTMASK 设置中断允许
}

void DisSlicInt(void)
{
    SLIC_GPIO_REG_SETBIT(g_gpioBase, HI_GPIO_INTMASK, GPIO_23);     // HI_GPIO_INTMASK 设置中断屏蔽
}


#ifdef __cplusplus
}
#endif


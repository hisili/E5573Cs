

#ifndef __DRV_PTABLE_H__
#define __DRV_PTABLE_H__

#include "drv_comm.h"


/*************************PTABLE START************************************/

typedef enum tagBSP_MEMORY_FLASH_OWNER_E
{
    BSP_MEMORY_FLASH_OWNER_BOOTLOAD = 0 ,
    BSP_MEMORY_FLASH_OWNER_NV_LTE       ,
    BSP_MEMORY_FLASH_OWNER_NV_GU        ,
    BSP_MEMORY_FLASH_OWNER_BOOTROM_A    ,
    BSP_MEMORY_FLASH_OWNER_BOOTROM_B    ,
    BSP_MEMORY_FLASH_OWNER_VXWORKS_MCORE,
    BSP_MEMORY_FLASH_OWNER_VXWORKS_ACORE,
    BSP_MEMORY_FLASH_OWNER_YAFFS_MCORE  ,
    BSP_MEMORY_FLASH_OWNER_YAFFS_ACORE  ,
    BSP_MEMORY_FLASH_OWNER_MAX
}BSP_MEMORY_FLASH_OWNER_E;

/*****************************************************************************
* 函 数 名  : BSP_OM_GetFlashDistribution
*
* 功能描述  : 获取系统FLASH空间划分
*
* 输入参数  : BSP_MEMORY_FLASH_OWNER_E eFlashOwner  : FLASH所有者
*
* 输出参数  : BSP_U32 *pu32StartAddr                : 起始地址
*             BSP_U32 *pu32EndAddr                  : 结束地址
*
* 返 回 值  : BSP_ERROR:获取失败
*             BSP_OK:获取成功
*
* 其它说明  : 无
*
*****************************************************************************/
static INLINE BSP_S32 BSP_OM_GetFlashDistribution(BSP_MEMORY_FLASH_OWNER_E eFlashOwner, BSP_U32 *pu32StartAddr, BSP_U32 *pu32EndAddr)
{
	return 0;
}

static INLINE unsigned long DRV_GET_LOCAL_FLASH_SUPPORT(void)
{
    return 0;
}






/*************************PTABLE END************************************/

#endif


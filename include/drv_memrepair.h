

#ifndef __DRV_MEM_REPAIR_H__
#define __DRV_MEM_REPAIR_H__

#include "drv_comm.h"

extern int bsp_get_memrepair_time(void);

#define DRV_GET_MEM_REPAIR_TIME() bsp_get_memrepair_time()

#endif


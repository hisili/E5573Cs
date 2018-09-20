#include <bsp_memmap.h>
#include <osl_bio.h>
#include <bsp_wdt.h>
#include <drv_wdt.h>
#include <hi_wdt.h>
#include <hi_syssc_interface.h>
BSP_S32 BSP_WDT_Enable(BSP_U8 wdtId)
{
	return bsp_wdt_start();
}

BSP_S32 BSP_WDT_Disable(BSP_U8 wdtId)
{
	return bsp_wdt_stop();
}

BSP_S32 BSP_WDT_HardwareFeed(BSP_U8 wdtId)
{
	return bsp_wdt_keepalive();
}



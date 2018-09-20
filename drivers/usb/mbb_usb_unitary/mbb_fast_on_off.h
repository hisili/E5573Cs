


#ifndef _FAST_ON_OFF_H
#define _FAST_ON_OFF_H

#include "usb_platform_comm.h"
#include <drv_chg.h>
#include <drv_onoff.h>

#ifdef USB_FAST_ON_OFF
USB_INT usb_fast_on_off_stat(USB_VOID);
USB_VOID fast_on_off_init(USB_VOID);
USB_VOID fast_on_off_exit(USB_VOID);
#else
static inline USB_INT usb_fast_on_off_stat(USB_VOID)
{
    return 0;
}

static inline USB_VOID fast_on_off_init(USB_VOID)
{
    return ;
}

static inline USB_VOID fast_on_off_exit(USB_VOID)
{
    return ;
}

#endif

#endif
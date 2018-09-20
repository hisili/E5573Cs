
#include <product_config.h>

#include "rtc_hi6559/rtc_hi6559.h"

unsigned int DRV_GET_RTC_VALUE (void)
{
    return hi6559_get_rtc_value();
}


/* arch/arm/mach-balong/include/mach/gpio.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * BALONGV7R2 - GPIO lib support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef	__BALONG_GPIO_H
#define	__BALONG_GPIO_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/types.h>
#include "bsp_memmap.h"
#include "hi_gpio.h"


#define gpio_get_value	__gpio_get_value
#define gpio_set_value	__gpio_set_value
#define gpio_cansleep	__gpio_cansleep
#define gpio_to_irq		__gpio_to_irq

#define GPIO_OK        0
#define GPIO_ERROR     -1

#define ARCH_NR_GPIOS     (GPIO_TOTAL_PINS_NUM)

/* GPIO TLMM: Function -- GPIO specific */

/* GPIO TLMM: Direction */
enum {
	GPIO_INPUT = 0,
	GPIO_OUTPUT = 1,
};

/* GPIO TLMM: Pullup/Pulldown */
enum {
	GPIO_NO_PULL,
	GPIO_PULL_DOWN,
	GPIO_KEEPER,
	GPIO_PULL_UP,
};

/*GPIO function*/
enum{
	GPIO_NORMAL= 0,
	GPIO_INTERRUPT = 1,
};

/*interrupt mask*/
enum {
	GPIO_INT_ENABLE = 0,
	GPIO_INT_DISABLE = 1,
};

/*interrupt level trigger*/
enum{
	GPIO_INT_TYPE_LEVEVL = 0,
	GPIO_INT_TYPE_EDGE = 1,
};

/*interrupt polarity*/
enum{
	GPIO_INT_POLARITY_FAL_LOW = 0,
	GPIO_INT_POLARITY_RIS_HIGH = 1,
};

/*************************************************
  Function:       gpio_direction_get
  Description:    Get GPIO Direction value,
  				  include GPIO_INPUT and GPIO_OUTPUT
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio, such as GPIO_*_**

  Return:         GPIO_INPUT, GPIO_OUTPUT
  Others:
************************************************/
extern int gpio_direction_get(unsigned gpio);

/*************************************************
  Function:       gpio_set_function
  Description:    Set GPIO function
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio, int mode, include
  	              GPIO_NORMAL,
				  GPIO_INTERRUPT,

  Return:         NONE
  Others:
************************************************/
extern void gpio_set_function(unsigned gpio, int mode);

/*************************************************
  Function:       gpio_int_mask_set
  Description:    Mask GPIO interrupt
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio, include GPIO_INT_ENABLE
  				  and GPIO_INT_DISABLE
  Return:         NONE
  Others:mask bit refer to GPIO_INT_ENABLE, GPIO_INT_DISABLE
************************************************/
extern void gpio_int_mask_set(unsigned gpio);

/*************************************************
  Function:       gpio_int_unmask_set
  Description:    unMask GPIO interrupt
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio

  Return:        NONE
  Others:mask bit refer to GPIO_INT_ENABLE, GPIO_INT_DISABLE
************************************************/
extern void gpio_int_unmask_set(unsigned gpio);

/*************************************************
  Function:       gpio_int_state_clear
  Description:    get GPIO interrupt status
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio

  Return:         NONE
  Others:
************************************************/
extern void gpio_int_state_clear(unsigned gpio);

/*************************************************
  Function:       gpio_int_state_get
  Description:    get GPIO interrupt  status
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio

  Return:         NONE
  Others:
************************************************/
extern int gpio_int_state_get(unsigned gpio);

/*************************************************
  Function:       gpio_raw_int_state_get
  Description:    Clear GPIO raw interrupt
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio

  Return:         NONE
  Others:clear refer to
************************************************/
extern int gpio_raw_int_state_get(unsigned gpio);

/*************************************************
  Function:       gpio_int_trigger_set
  Description:    set GPIO Interrupt Triger style
  Calls:
  Called By:
  Table Accessed: NONE
  Table Updated:  NONE
  Input:          unsigned gpio, int trigger_type
				  IRQ_TYPE_EDGE_RISING,
				  IRQ_TYPE_EDGE_FALLING,
				  IRQ_TYPE_LEVEL_HIGH,
				  IRQ_TYPE_LEVEL_LOW
  Output:         NONE
  Return:         OSAL_OK : successfully
                  OSAL_ERROR: fail
  Others:
************************************************/
extern void gpio_int_trigger_set(unsigned gpio, int trigger_type);

#include <asm-generic/gpio.h>
#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif /* __cplusplus */

#endif

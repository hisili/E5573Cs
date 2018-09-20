/*
 * LEDs driver for GPIOs
 *
 * Copyright (C) 2007 8D Technologies inc.
 * Raphael Assenat <raph@8d.com>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <product_config.h>
#include <bsp_wlan.h>

#include <linux/wakelock.h>
#include "bsp_regulator.h"
#include <drv_version.h>
static struct wake_lock huawei_led_wake_lock; /*lint !e86*/
#define LED_DELAY_TIME  (20 * HZ)    /*应对led节点初始化提前，改为20秒*/
extern struct workqueue_struct *led_workqueue_brightness;    /*亮灭控制队列*/
static struct regulator *g_led_vcc = NULL; /*LDO11 给led供电控制 */
/* 保存LED 特性归一化之后gpio led设置 */
struct gpio_led gpio_exp_leds_config_uniform[GPIO_LED_NUMBER] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};

struct gpio_led_data {
    struct led_classdev cdev;
    unsigned gpio;
    struct work_struct work;
    u8 new_level;
    u8 can_sleep;
    u8 active_low;
    u8 blinking;
    int (*platform_gpio_blink_set)(unsigned gpio, int state,
            unsigned long *delay_on, unsigned long *delay_off);
};

static void gpio_led_work(struct work_struct *work)
{
    struct gpio_led_data	*led_dat =
        container_of(work, struct gpio_led_data, work);
    if (GPIO_NULL != led_dat->gpio)                  /*lint !e10*/
    {
        if (led_dat->blinking) {
            led_dat->platform_gpio_blink_set(led_dat->gpio,
                             led_dat->new_level,
                             NULL, NULL);
            led_dat->blinking = 0;
        } else
        gpio_set_value(led_dat->gpio,  led_dat->new_level);
    }
    else
    {
        wlan_set_led_flag(led_dat->new_level);     /*lint !e628*/
    }
}

static void gpio_led_set(struct led_classdev *led_cdev,
    enum led_brightness value)
{
    struct gpio_led_data *led_dat =
    container_of(led_cdev, struct gpio_led_data, cdev);
    int level;

    if (value == LED_OFF)
        level = 0;
    else
        level = 1;

    if (led_dat->active_low)
        level = !level;

    /* Setting GPIOs with I2C/etc requires a task context, and we don't
     * seem to have a reliable way to know if we're already in one; so
     * let's just assume the worst.
     */
    if (GPIO_NULL != led_dat->gpio)             /*lint !e10*/
    {
        if (1) {
        led_dat->new_level = level;
        queue_work(led_workqueue_brightness, &(led_dat->work));
        } else {
            if (led_dat->blinking) {
                led_dat->platform_gpio_blink_set(led_dat->gpio, level,
                                 NULL, NULL);
                led_dat->blinking = 0;
            } else
                gpio_set_value(led_dat->gpio, level);
        }
    }
    else
    { 
        led_dat->new_level = level;
        schedule_work(&led_dat->work);            /*lint !e534*/
    }
        
}

static int gpio_blink_set(struct led_classdev *led_cdev,
    unsigned long *delay_on, unsigned long *delay_off)
{
    struct gpio_led_data *led_dat =
		container_of(led_cdev, struct gpio_led_data, cdev);

    led_dat->blinking = 1;
    return led_dat->platform_gpio_blink_set(led_dat->gpio, GPIO_LED_BLINK,
                        delay_on, delay_off);
}

static int __devinit create_gpio_led(const struct gpio_led *template,
    struct gpio_led_data *led_dat, struct device *parent,
    int (*blink_set)(unsigned, int, unsigned long *, unsigned long *))
{
    int ret, state;

    led_dat->gpio = -1;
    if (GPIO_NULL != template->gpio)         /*lint !e10*/
    {
        /* skip leds that aren't available */
        if (!gpio_is_valid(template->gpio)) {
		    printk(KERN_INFO "Skipping unavailable LED gpio %d (%s)\n",
				    template->gpio, template->name);
            return 0;
        }

            ret = gpio_request(template->gpio, template->name);
            if (ret < 0)
                return ret;
    }
    led_dat->cdev.name = template->name;
    led_dat->cdev.default_trigger = template->default_trigger;
    led_dat->gpio = template->gpio;
    if (GPIO_NULL != template->gpio)         /*lint !e10*/
    {
        led_dat->can_sleep = gpio_cansleep(template->gpio);
    }
    led_dat->active_low = template->active_low;
    led_dat->blinking = 0;
    if (blink_set) {
        led_dat->platform_gpio_blink_set = blink_set;
        led_dat->cdev.blink_set = gpio_blink_set;
    }
    led_dat->cdev.brightness_set = gpio_led_set;
    if (template->default_state == LEDS_GPIO_DEFSTATE_KEEP)
        state = !!gpio_get_value_cansleep(led_dat->gpio) ^ led_dat->active_low;
    else
        state = (template->default_state == LEDS_GPIO_DEFSTATE_ON);
    led_dat->cdev.brightness = state ? LED_FULL : LED_OFF;
    if (!template->retain_state_suspended)
        led_dat->cdev.flags |= LED_CORE_SUSPENDRESUME;
    if (GPIO_NULL != template->gpio)           /*lint !e10*/
    {
        ret = gpio_direction_output(led_dat->gpio, led_dat->active_low ^ state);
        if (ret < 0)
            goto err;
    }
    INIT_WORK(&led_dat->work, gpio_led_work);
    ret = led_classdev_register(parent, &led_dat->cdev);
    if (ret < 0)
        goto err;
    
    return 0;
err:
    gpio_free(led_dat->gpio);
    return ret;
}

static void delete_gpio_led(struct gpio_led_data *led)
{
    if (!gpio_is_valid(led->gpio))
        return;
    led_classdev_unregister(&led->cdev);
    cancel_work_sync(&led->work);
    gpio_free(led->gpio);
}

struct gpio_leds_priv {
    int num_leds;
    struct gpio_led_data leds[];
};

static inline int sizeof_gpio_leds_priv(int num_leds)
{
    return sizeof(struct gpio_leds_priv) +
        (sizeof(struct gpio_led_data) * num_leds);
}

/* Code to create from OpenFirmware platform devices */
static struct gpio_leds_priv * __devinit gpio_leds_create_of(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node, *child;
    struct gpio_leds_priv *priv;
    int count = 0, ret;

    /* count LEDs in this device, so we know how much to allocate */
    for_each_child_of_node(np, child)
        count++;
    if (!count)
        return NULL;

    priv = kzalloc(sizeof_gpio_leds_priv(count), GFP_KERNEL);
    if (!priv)
        return NULL;

    for_each_child_of_node(np, child) {
        struct gpio_led led = {};
        enum of_gpio_flags flags;
        const char *state;

    led.gpio = of_get_gpio_flags(child, 0, &flags);
    led.active_low = flags & OF_GPIO_ACTIVE_LOW;
    led.name = of_get_property(child, "label", NULL) ? : child->name;
    led.default_trigger =
        of_get_property(child, "linux,default-trigger", NULL);
    state = of_get_property(child, "default-state", NULL);
    if (state) {
    if (!strcmp(state, "keep"))
        led.default_state = LEDS_GPIO_DEFSTATE_KEEP;
    else if (!strcmp(state, "on"))
        led.default_state = LEDS_GPIO_DEFSTATE_ON;
    else
        led.default_state = LEDS_GPIO_DEFSTATE_OFF;
    }

    ret = create_gpio_led(&led, &priv->leds[priv->num_leds++],
                 &pdev->dev, NULL);
    if (ret < 0) {
        of_node_put(child);
        goto err;
        }
    }

    return priv;

err:
    for (count = priv->num_leds - 2; count >= 0; count--)
        delete_gpio_led(&priv->leds[count]);
    kfree(priv);
    return NULL;
}

static const struct of_device_id of_gpio_leds_match[] = {
    { .compatible = "gpio-leds", },
    {},
};


/*lint -e54 -e119 -e30 -e84 -e514 -e18*/
static struct gpio_led_platform_data gpio_leds_pdata = {
    .num_leds = ARRAY_SIZE(gpio_exp_leds_config_uniform),
    .leds = gpio_exp_leds_config_uniform,
};
/*lint +e54 +e119 +e30 +e84 +e514 +e18*/
static struct platform_device gpio_leds = {
    .name          = "leds_gpio",
    .id            = -1,
    .dev           = {
        .platform_data = &gpio_leds_pdata,
    },
};



bool support_uniform_product(u32 product_type, int *match_id)
{
    int i = 0;
    int number = 0;

    number = (int)(ARRAY_SIZE(uniform_led_config));
    /* 遍历uniform_led_config，查找该产品是否支持LED形态版本归一方案 */
    for (i = 0; i < number; i++)
    {
        if (product_type == uniform_led_config[i].board_id)
        {
            *match_id = i;
            return TRUE;
        }
    }

    *match_id = -1;
    return FALSE;
}



static void gpio_led_adapt(struct gpio_led_platform_data *pdata)
{
    unsigned int board_id = 0xffffffff;
    int i = 0;
    int match_id = 0;
    int gpio_led_number = 0;

    if(NULL == pdata)
    {
        return;
    }

    board_id = (unsigned int)bsp_version_get_boardid();  /* 获取硬件ID */

    /* 如果该产品支持LED归一，则使用uniform_led_config中的LED初始化配置， 后续新项目都支持归一 */
    if ( support_uniform_product(board_id, &match_id) )
    {
        /* gpio led个数初始化 */
        pdata->num_leds = uniform_led_config[match_id].total_gpio_led_number;
        for (i = 0; i < uniform_led_config[match_id].total_gpio_led_number; i++)
        {
            /* 判断GPIO LED初始化配置中led name及default_trigger是否为空, 如果为空，则返回 */
            if ( (NULL == uniform_led_config[match_id].gpio_led[i].name) || (NULL == uniform_led_config[match_id].gpio_led[i].default_trigger) )
            {
                printk(KERN_ERR "[%s]invalid gpio led name, match_id=%d, i=%d\n", __func__, match_id, i);
                return ;
            }

            /* 申请全局数组gpio_exp_leds_config_uniform[]的led name空间并初始化为0 */
            gpio_exp_leds_config_uniform[i].name = (char *)kzalloc(strlen(uniform_led_config[match_id].gpio_led[i].name) + 1, GFP_KERNEL);
            if (NULL == gpio_exp_leds_config_uniform[i].name)
            {
                printk(KERN_ERR "[%s]name malloc failed\n", __func__);
                return ;
            }

            /* 申请全局数组gpio_exp_leds_config_uniform[]的led default_trigger空间并初始化为0 */
            gpio_exp_leds_config_uniform[i].default_trigger = (char*)kzalloc(strlen(uniform_led_config[match_id].gpio_led[i].default_trigger) + 1, GFP_KERNEL);
            if (NULL == gpio_exp_leds_config_uniform[i].default_trigger)
            {
                kfree(gpio_exp_leds_config_uniform[i].name);
                gpio_exp_leds_config_uniform[i].name = NULL;
                printk(KERN_ERR "[%s]default_trigger malloc failed\n", __func__);
                return ;
            }

            /* 拷贝uniform_led_config中硬件ID匹配的GPIO灯的name配置到GPIO LED全局数组gpio_exp_leds_config_uniform[]的name配置中，后续操作都使用该全局数组 */
            memcpy(gpio_exp_leds_config_uniform[i].name, uniform_led_config[match_id].gpio_led[i].name, strlen(uniform_led_config[match_id].gpio_led[i].name));
            /* 拷贝uniform_led_config中default_trigger配置到GPIO LED全局数组gpio_exp_leds_config_uniform的default_trigger配置中 */
            memcpy(gpio_exp_leds_config_uniform[i].default_trigger, uniform_led_config[match_id].gpio_led[i].default_trigger, strlen(uniform_led_config[match_id].gpio_led[i].default_trigger));
            /* GPIO LED管脚初始化 */
            gpio_exp_leds_config_uniform[i].gpio = uniform_led_config[match_id].gpio_led[i].gpio;
            /* active_low初始化 */
            gpio_exp_leds_config_uniform[i].active_low = uniform_led_config[match_id].gpio_led[i].active_low;
            gpio_exp_leds_config_uniform[i].retain_state_suspended = uniform_led_config[match_id].gpio_led[i].retain_state_suspended;
            gpio_exp_leds_config_uniform[i].default_state = uniform_led_config[match_id].gpio_led[i].default_state;
        }
    }
    else    /* 如果该产品是老款产品，则使用gpio_exp_leds_config中的LED配置，此处为兼容老项目 */
    {
        gpio_led_number = (int)(ARRAY_SIZE(gpio_exp_leds_config));
        pdata->num_leds = gpio_led_number;
        for (i = 0; i < gpio_led_number; i++)
        {
            /* 判断GPIO LED初始化配置中led name及default_trigger是否为空, 如果为空，则返回 */
            if ( (NULL == gpio_exp_leds_config[i].name) || (NULL == gpio_exp_leds_config[i].default_trigger) )
            {
                printk(KERN_ERR "[%s]invalid gpio led name\n", __func__);
                return ;
            }

            /* 申请全局数组gpio_exp_leds_config_uniform[]的led name空间并初始化为0 */
            gpio_exp_leds_config_uniform[i].name = (char *)kzalloc(strlen(gpio_exp_leds_config[i].name) + 1, GFP_KERNEL);
            if (NULL == gpio_exp_leds_config_uniform[i].name)
            {
                printk(KERN_ERR "[%s]gpio name malloc failed\n", __func__);
                return ;
            }

            /* 申请全局数组gpio_exp_leds_config_uniform[]的led default_trigger空间并初始化为0 */
            gpio_exp_leds_config_uniform[i].default_trigger = (char *)kzalloc(strlen(gpio_exp_leds_config[i].default_trigger) + 1, GFP_KERNEL);
            if (NULL == gpio_exp_leds_config_uniform[i].default_trigger)
            {
                kfree(gpio_exp_leds_config_uniform[i].name);
                gpio_exp_leds_config_uniform[i].name = NULL;
                printk(KERN_ERR "[%s]default_trigger malloc failed\n", __func__);
                return ;
            }

            /* 拷贝gpio_exp_leds_config中name配置到GPIO LED全局数组gpio_exp_leds_config_uniform[]的name配置中，后续操作都使用该全局数组 */
            memcpy(gpio_exp_leds_config_uniform[i].name, gpio_exp_leds_config[i].name, strlen(gpio_exp_leds_config[i].name));
            /* 拷贝老产品的GPIO灯的default_trigger配置到GPIO LED全局数组gpio_exp_leds_config_uniform的default_trigger配置中 */
            memcpy(gpio_exp_leds_config_uniform[i].default_trigger, gpio_exp_leds_config[i].default_trigger, strlen(gpio_exp_leds_config[i].default_trigger));
            /* GPIO LED管脚初始化 */
            gpio_exp_leds_config_uniform[i].gpio = gpio_exp_leds_config[i].gpio;
            /* active_low初始化 */
            gpio_exp_leds_config_uniform[i].active_low = gpio_exp_leds_config[i].active_low;
            gpio_exp_leds_config_uniform[i].retain_state_suspended = gpio_exp_leds_config[i].retain_state_suspended;
            gpio_exp_leds_config_uniform[i].default_state = gpio_exp_leds_config[i].default_state;
        }
    }
}

static int __devinit gpio_led_probe(struct platform_device *pdev)
{
    struct gpio_led_platform_data *pdata = pdev->dev.platform_data;
    struct gpio_leds_priv *priv;
    int i, ret = 0;

    /* 根据硬件ID，动态适配GPIO LED配置，实现LED归一 */
    gpio_led_adapt(pdata);

    if (pdata && pdata->num_leds) {
        priv = kzalloc(sizeof_gpio_leds_priv(pdata->num_leds),
                GFP_KERNEL);
        if (!priv)
            return -ENOMEM;

    priv->num_leds = pdata->num_leds;
    for (i = 0; i < priv->num_leds; i++) {
        ret = create_gpio_led(&pdata->leds[i],
                      &priv->leds[i],
                      &pdev->dev, pdata->gpio_blink_set);
        if (ret < 0) {
            /* On failure: unwind the led creations */
            for (i = i - 1; i >= 0; i--)
                delete_gpio_led(&priv->leds[i]);
            kfree(priv);
            return ret;
        }
    }
    } else {
        return 0;
    }
    wake_lock_init(&huawei_led_wake_lock, WAKE_LOCK_SUSPEND,"huawei_led_wakelock");
    wake_lock_timeout(&huawei_led_wake_lock, LED_DELAY_TIME);

    platform_set_drvdata(pdev, priv);

    return 0;
}

static int __devexit gpio_led_remove(struct platform_device *pdev)
{
    struct gpio_leds_priv *priv = dev_get_drvdata(&pdev->dev);
    int i;

    for (i = 0; i < priv->num_leds; i++)
        delete_gpio_led(&priv->leds[i]);
    dev_set_drvdata(&pdev->dev, NULL);
    kfree(priv);

    wake_lock_destroy(&huawei_led_wake_lock);

    return 0;
}




static struct platform_driver gpio_led_driver = {
    .probe      = gpio_led_probe,
    .remove     = __devexit_p(gpio_led_remove),
    .driver     = {
        .name   = "leds_gpio",
        .owner  = THIS_MODULE,
        .of_match_table = of_gpio_leds_match,
   
    },
};

/*lint -e629*/
static int __init gpio_led_init(void)
{   
    int result;

    result = platform_driver_register(&gpio_led_driver);
    if (result < 0)
    {
        return result;
    }

    result = platform_device_register(&gpio_leds);
    if (result < 0)
    {
        platform_driver_unregister(&gpio_led_driver);
        return result;
    }

    return result;
}

static void __exit gpio_led_exit(void)
{
    platform_driver_unregister(&gpio_led_driver);
    platform_device_unregister(&gpio_leds);
}
/*lint +e629*/
/*lint -e529*/
module_init(gpio_led_init); 
module_exit(gpio_led_exit); 
/*lint +e529*/
MODULE_AUTHOR("Raphael Assenat <raph@8d.com>, Trent Piepho <tpiepho@freescale.com>");
MODULE_DESCRIPTION("GPIO LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-gpio");

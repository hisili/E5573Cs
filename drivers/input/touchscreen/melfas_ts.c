/*********************************************************************
 *
 * Melfas MCS6000 Touchscreen Controller Driver
 *
 *********************************************************************/

/*********************************************************************
 * drivers/input/touchscreen/melfas_ts.c
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation,and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *********************************************************************/
 

#include <linux/module.h>
#include <linux/delay.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "melfas_ts.h"



#define MELFAS_MAX_TOUCH      2
#define FW_VERSION            0x00 //Check Version
#define HW_VERSION            0x00 //Check Version

#define TS_MAX_X_COORD        240 //Check resolution
#define TS_MAX_Y_COORD        320 //Check resolution

#define TS_MAX_Z_TOUCH        255
#define TS_MAX_W_TOUCH        30

#define TS_READ_EVENT_PACKET_SIZE     0x0F
#define TS_READ_START_ADDR            0x10
#define TS_READ_VERSION_ADDR          0xF0 //Start read H/W, S/W Version
#define TS_READ_REGS_LEN              66
#define TS_SINGAL_POINT_LEN           6

#define TOUCH_TYPE_NONE         0
#define TOUCH_TYPE_SCREEN       1
#define TOUCH_TYPE_KEY          2

#define I2C_RETRY_CNT           10

#define SET_DOWNLOAD_BY_GPIO    1
#define ESD_DETECTED            0

#define PRESS_KEY               1
#define RELEASE_KEY             0
#define DEBUG_PRINT             1

#define TS_READ_ONCE            1
/*I2C read or write markers*/
#define I2C_MSG_NUM    2
#define I2C_READ    1
#define I2C_WRITE    0

static struct workqueue_struct *melfas_ts_wq = NULL;

static int melfas_ts_debug_mask = 0;
#define TP_DEBUG(args...) do { \
if(melfas_ts_debug_mask) { \
        printk( args); \
    } \
}while(0)

int melfas_ts_debug_set(int mask)
{
    melfas_ts_debug_mask = mask;
    return 0;
}

static struct melfas_touch_platform_data touch_platformdata = {
    .gpio_en = MELFAS_LDO_EN,
    .gpio_int = MELFAS_INT,
    .gpio_reset = MELFAS_RESET,
};

static struct i2c_board_info bus_i2c_devices[] = {
    {
        I2C_BOARD_INFO(MELFAS_TS_NAME, MELFAS_TOUCH_ADDR),
        .platform_data = &touch_platformdata,
    },
};

#if SET_DOWNLOAD_BY_GPIO
#include <melfas_download.h>
#endif // SET_DOWNLOAD_BY_GPIO

//该结构体用于保存TP触摸坐标信息
struct muti_touch_info
{
    int action;
    int fingerX;
    int fingerY;
    int width;
    int strength;
};

struct melfas_ts_data
{
    uint16_t addr;
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct work_struct  work;
    uint32_t flags;
    int (*power)(int on);
    #ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
    #endif
    struct melfas_touch_platform_data *platform_data;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];


static int melfas_init_panel(struct melfas_ts_data *ts)
{
    int buf = 0x00;
    int ret = 0;

    ret = i2c_master_send(ts->client, &buf, 1);
    return ret;
}

int melfas_data_receive(const struct i2c_client *client, const char *buf, int count, int offset )
{
    int ret = 0;
    struct i2c_msg msg[I2C_MSG_NUM] = {0};
    uint8_t buf0[I2C_MSG_NUM] = {0};

    buf0[0] = offset;    /*the slave register offset*/
    /*config the sending message*/
    msg[0].addr = client->addr;
    msg[0].flags = I2C_WRITE;    /*operation type */
    msg[0].len = 1;             /*the first msg is only include register offset*/
    msg[0].buf = buf0;          /*register offset*/
    msg[1].addr = client->addr;
    msg[1].flags = I2C_READ;    /*operation type */
    msg[1].len = count;         /*read byte number*/
    msg[1].buf = (char *)buf;   /*read data buf */

    /*read data*/
    ret = i2c_transfer(client->adapter, msg, I2C_MSG_NUM);
    if (ret < 0) 
    {
        printk(KERN_ERR "TP:i2c_transfer failed\n");
        return ret;
    }   
    return ret;
}

static void melfas_ts_work_func(struct work_struct *work)
{
    struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
    int ret = 0, i;
    uint8_t buf[TS_SINGAL_POINT_LEN + 1] = {0};
    uint8_t read_num = 0;
    uint8_t touchAction = 0, touchType = 0, fingerID = 0;


#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_work_func\n");
#endif 



    /******************************************************
    Simple send transaction:
    S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
    Simple recv transaction:
    S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
    *******************************************************/
#if 0
    buf[0] = TS_READ_EVENT_PACKET_SIZE;
    ret = i2c_master_send(ts->client, buf, 1);
#endif

#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif

    if (ret >= 0)
    {
#if 0
#if TS_READ_ONCE
        ret = i2c_master_recv(ts->client, buf, 13);
#else
        ret = i2c_master_recv(ts->client, buf, 1);
#endif
#endif
        /*only process one point data，1+6 byte */
        ret = melfas_data_receive(ts->client, buf, TS_SINGAL_POINT_LEN + 1, TS_READ_EVENT_PACKET_SIZE);
#if DEBUG_PRINT
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv packet_size [%d]\n", buf[0]);

#if TS_READ_ONCE
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv InputEvent_information [%d]\n", buf[1]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv xy_coordi [%d]\n", buf[2]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv x_coordi [%d]\n", buf[3]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv y_coordi [%d]\n", buf[4]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv width [%d]\n", buf[5]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv strength [%d]\n", buf[6]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv 2_InputEvent_information [%d]\n", buf[7]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv 2_xy_coordi [%d]\n", buf[8]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv 2_x_coordi [%d]\n", buf[9]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv 2_y_coordi [%d]\n", buf[10]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv 2_width [%d]\n", buf[11]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv 2_strength [%d]\n", buf[12]);
#endif
#endif
        if (ret >= 0)
        {
            read_num = buf[0];

#if ESD_DETECTED    
            if((read_num & 0x80))
            {
#if DEBUG_PRINT
                printk(KERN_ERR "***** ESD Detected status ****\n", ret);
#endif            
                //Need CE or VDD Control for TSP reset.
                return;
            }
#endif //ESD_DETECTED           
        }
        /*only process one point data，six byte */
        if(read_num > TS_SINGAL_POINT_LEN)
        {
            read_num = TS_SINGAL_POINT_LEN;
        }
    }
    else // ret < 0
    {
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
#endif
        //enable_irq(ts->client->irq);
        //lint -e628
        gpio_int_unmask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534 */
        return ;
    }
  
#if !(TS_READ_ONCE)
    buf[0] = TS_READ_START_ADDR;
    ret = i2c_master_send(ts->client, buf, 1);
#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif
    if (ret >= 0)
    {
        ret = i2c_master_recv(ts->client, buf, read_num);
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);
#endif
        if (ret >= 0)
            break; // i2c success
    }

    if(ret < 0) 
    {
        printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
        //enable_irq(ts->client->irq);
        gpio_int_unmask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/
        return ;
    }
    else // MIP (Melfas Interface Protocol)
#endif
    {
#if TS_READ_ONCE
        for(i = 1; i < read_num + 1; i = i + 6)
#else
        for(i = 0; i < read_num; i = i + 6)
#endif
        {
            touchAction = ((buf[i] & 0x80) == 0x80);
            //touchAction = ((buf[i] & 0x10) == 0x10);//RevB
            touchType = (buf[i] & 0x60) >> 5;
            fingerID = (buf[i] & 0x0F) - 1;

#if DEBUG_PRINT
            TP_DEBUG(KERN_ERR "melfas_ts_work_func: touchAction : %d, touchType: %d, fingerID: %d\n", touchAction, touchType, fingerID);
#endif

            if(touchType == TOUCH_TYPE_NONE)
            {
            }
            else if(touchType == TOUCH_TYPE_SCREEN)
            {
                g_Mtouch_info[fingerID].action = touchAction;
                g_Mtouch_info[fingerID].fingerX = (buf[i + 1] & 0x0F) << 8 | buf[i + 2];
                g_Mtouch_info[fingerID].fingerY = (buf[i + 1] & 0xF0) << 4 | buf[i + 3];
                g_Mtouch_info[fingerID].width = buf[i + 4];
                g_Mtouch_info[fingerID].strength = buf[i + 5];

                input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, fingerID);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[fingerID].fingerX);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[fingerID].fingerY);
                input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR,  g_Mtouch_info[fingerID].strength);
                input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR,g_Mtouch_info[fingerID].width);
                input_mt_sync(ts->input_dev);
                input_sync(ts->input_dev);
#if DEBUG_PRINT
                TP_DEBUG(KERN_ERR "melfas_ts_work_func: Touch ID: %d, x: %d, y: %d, z: %d w: %d\n",
                        i, g_Mtouch_info[fingerID].fingerX, g_Mtouch_info[fingerID].fingerY, g_Mtouch_info[fingerID].strength, g_Mtouch_info[fingerID].width);
#endif
            }
            else if(touchType == TOUCH_TYPE_KEY)
            {
                if (fingerID == 0x1)
                    input_report_key(ts->input_dev, KEY_MENU, touchAction ? PRESS_KEY : RELEASE_KEY);
                if (fingerID == 0x2)
                    input_report_key(ts->input_dev, KEY_HOME, touchAction ? PRESS_KEY : RELEASE_KEY);
                if (fingerID == 0x3)
                    input_report_key(ts->input_dev, KEY_BACK, touchAction ? PRESS_KEY : RELEASE_KEY);
                if (fingerID == 0x4)
                    input_report_key(ts->input_dev, KEY_SEARCH, touchAction ? PRESS_KEY : RELEASE_KEY);


                input_sync(ts->input_dev);
#if DEBUG_PRINT
                TP_DEBUG(KERN_ERR "melfas_ts_work_func: keyID : %d, keyState: %d\n", fingerID, touchAction);
#endif
            }
        }
    }

    //enable_irq(ts->client->irq);
    gpio_int_unmask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e144 !e534*/
}

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
    struct melfas_ts_data *ts = (struct melfas_ts_data *)handle;
    unsigned int ucdata = 0;
#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_irq_handler\n");
#endif

    //disable_irq_nosync(ts->client->irq);
    ucdata = gpio_int_state_get((unsigned int)(ts->platform_data->gpio_int));
    if (0 == ucdata)
    {
        printk(KERN_ERR "%s not gpio%d interrupt.\n", 
            __func__,ts->platform_data->gpio_int);
        return IRQ_NONE;/*lint !e82 !e110 !e533*/
    }

    gpio_int_mask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534 */
    gpio_int_state_clear((unsigned int)(ts->platform_data->gpio_int));/*lint !e534 */

    //schedule_work(&ts->work);
    queue_work(melfas_ts_wq, &ts->work);/*lint !e534*/

    return IRQ_HANDLED;
}


static int __devinit melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct melfas_ts_data *ts;
    int ret = 0, i;
    uint8_t buf[6];//dong.hu
    struct melfas_touch_platform_data *ts_platformdata = client->dev.platform_data;

#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe now ...\n");
#endif

    if(NULL == ts_platformdata)
    {
        printk(KERN_ERR "Could not find platform data!!\n");
        return ( - EINVAL);
    }
 
    //申请并配置LDO_EN使用的GPIO
    ret = gpio_request((unsigned int)(ts_platformdata->gpio_en), "melfas_en");
    if (ret)
    {
        printk(KERN_ERR "%s: Failed to get LDO_EN gpio %d. Code: %d.",
            __func__, ts_platformdata->gpio_en, ret);
        return ret;
    }
    gpio_direction_output((unsigned int)(ts_platformdata->gpio_en),1);/*lint !e534*/
    gpio_set_function((unsigned int)(ts_platformdata->gpio_en),GPIO_NORMAL);/*lint !e534*/
    gpio_set_value((unsigned int)(ts_platformdata->gpio_en), 1);
    msleep(MELFAS_POWER_UP_TIME);//wait the touch IC power up

    //申请并配置RESET使用的GPIO
    ret = gpio_request((unsigned int)(ts_platformdata->gpio_reset), "melfas_reset");
    if (ret)
    {
        printk(KERN_ERR "%s: Failed to get RESET gpio %d. Code: %d.",
            __func__, ts_platformdata->gpio_reset, ret);
        return ret;
    }
    gpio_direction_output((unsigned int)(ts_platformdata->gpio_reset),1);/*lint !e534*/
    gpio_set_function((unsigned int)(ts_platformdata->gpio_reset),GPIO_NORMAL);/*lint !e534 */ 

    msleep(MELFAS_TP_UNRESET_TIME);//wait the touch IC power up

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
        ret = -ENODEV;
        goto err_check_functionality_failed;
    }

    ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
    if (NULL == ts)
    {
        printk(KERN_ERR "melfas_ts_probe: failed to create a state of melfas-ts\n");
        ret = -ENOMEM;
        goto err_alloc_data_failed;
    }

    ts->platform_data = client->dev.platform_data;
    melfas_ts_wq = create_singlethread_workqueue("melfas_ts_wq");
    if (NULL == melfas_ts_wq)
    {
        printk(KERN_ERR "Could not create work queue melfas_ts_wq: no memory!!\n");
        ret = - ENOMEM;
        goto  error_wq_creat_failed; 
    }

    INIT_WORK(&ts->work, melfas_ts_work_func);

    ts->client = client;
    i2c_set_clientdata(client, ts);

    ret = i2c_master_send(ts->client, &buf, 1);

#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif

#if SET_DOWNLOAD_BY_GPIO
    buf[0] = TS_READ_VERSION_ADDR;
    for (i = 0; i < I2C_RETRY_CNT; i++)
    {
        ret = i2c_master_send(ts->client, buf, 1);
        if (ret >= 0)
        {
            ret = i2c_master_recv(ts->client, buf, 6);//dong.hu

            if (ret >= 0)
            {
                break; // i2c success
            }
        }
    }

    if(i == I2C_RETRY_CNT) //VERSION READ Fail
    {
        //ret = mcsdl_download_binary_file();
        mcsdl_download_binary_data();
    }
    else
    {
        if (buf[0] == HW_VERSION && buf[1] < FW_VERSION)
        {
            //ret = mcsdl_download_binary_file();
            mcsdl_download_binary_data();
        }
    }
#endif // SET_DOWNLOAD_BY_GPIO

    ts->input_dev = input_allocate_device();
    if (!ts->input_dev)
    {
        printk(KERN_ERR "melfas_ts_probe: Not enough memory\n");
        ret = -ENOMEM;
        goto err_input_dev_alloc_failed;
    }

    ts->input_dev->name = "melfas-ts" ;
    ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY) | BIT_MASK(EV_SYN);
    ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
    ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
    ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
    ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);

//__set_bit(BTN_TOUCH, ts->input_dev->keybit);
//__set_bit(EV_ABS,  ts->input_dev->evbit);
//ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH - 1, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);
//__set_bit(EV_SYN, ts->input_dev->evbit);
//__set_bit(EV_KEY, ts->input_dev->evbit);

    ret = input_register_device(ts->input_dev);
    if (ret)
    {
        printk(KERN_ERR "melfas_ts_probe: Failed to register device\n");
        ret = -ENOMEM;
        goto err_input_register_device_failed;
    }

    //申请中断
    ts->client->irq = gpio_to_irq(ts->platform_data->gpio_int);
    ret = gpio_request((unsigned int)(ts->platform_data->gpio_int), "melfas_int");
    if (ret) 
    {
        printk(KERN_ERR "%s: Failed to get int gpio %d. Code: %d.",
            __func__, ts->platform_data->gpio_int, ret);
        return ret;
    }
    gpio_int_mask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/
    gpio_int_state_clear((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/
    gpio_direction_input((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/ 
    gpio_set_function((unsigned int)(ts->platform_data->gpio_int),GPIO_INTERRUPT);/*lint !e534*/
    gpio_int_trigger_set((unsigned int)(ts->platform_data->gpio_int),IRQF_TRIGGER_LOW);/*lint !e534*/

    if (ts->client->irq)
    {
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_probe: trying to request irq: %s-%d\n", ts->client->name, ts->client->irq);
#endif
        ret = request_irq(client->irq, melfas_ts_irq_handler, IRQF_NO_SUSPEND | IRQF_SHARED, ts->client->name, ts);
        if (ret > 0)
        {
            printk(KERN_ERR "melfas_ts_probe: Can't allocate irq %d, ret %d\n", ts->client->irq, ret);
            ret = -EBUSY;
            goto err_request_irq;
        }
    }

    gpio_int_state_clear((unsigned int)ts->platform_data->gpio_int);/*lint !e534*/
    gpio_int_unmask_set((unsigned int)ts->platform_data->gpio_int);/*lint !e534*/
    //schedule_work(&ts->work);
    queue_work(melfas_ts_wq, &ts->work);/*lint !e534*/

    for (i = 0; i < MELFAS_MAX_TOUCH; i++)  /* _SUPPORT_MULTITOUCH_ */
    {   
        g_Mtouch_info[i].strength = -1;
    }

#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe: succeed to register input device\n");
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
    ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1;
    ts->early_suspend.suspend = melfas_ts_early_suspend;
    ts->early_suspend.resume = melfas_ts_late_resume;
    register_early_suspend(&ts->early_suspend);
#endif

#if DEBUG_PRINT
    printk(KERN_INFO "melfas_ts_probe: Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
#endif
    return 0;

err_request_irq:
    printk(KERN_ERR "melfas-ts: err_request_irq failed\n");
    free_irq(client->irq, ts);
err_input_register_device_failed:
    printk(KERN_ERR "melfas-ts: err_input_register_device failed\n");
    input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
    printk(KERN_ERR "melfas-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
    printk(KERN_ERR "melfas-ts: err_alloc_data failed\n");
error_wq_creat_failed:
    printk(KERN_ERR "melfas-ts: error_wq_creat failed\n");
err_detect_failed:
    printk(KERN_ERR "melfas-ts: err_detect failed\n");
    kfree(ts);
err_check_functionality_failed:
    printk(KERN_ERR "melfas-ts: err_check_functionality failed\n");

    return ret;
}

static int __devexit melfas_ts_remove(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    if(NULL == ts)
    {
        return -ENODEV;
    }
    #ifdef CONFIG_HAS_EARLYSUSPEND    
    unregister_early_suspend(&ts->early_suspend);
    #endif
    free_irq(client->irq, ts);
    input_unregister_device(ts->input_dev);
    kfree(ts);
    return 0;
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret,i;  
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    for (i = 0; i < MELFAS_MAX_TOUCH ; i++)
    {
        g_Mtouch_info[i].strength = -1;
        g_Mtouch_info[i].fingerX = 0;
        g_Mtouch_info[i].fingerY = 0;
        g_Mtouch_info[i].width = 0;
    }
    
    if(NULL == ts)
    {
        return -ENODEV;
    }
    //disable_irq(client->irq);
    gpio_int_mask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/
    
    ret = cancel_work_sync(&ts->work);
    if (ret) /* if work was pending disable-count is now 2 */
    {
        //enable_irq(client->irq);
        gpio_int_unmask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/
    }

    gpio_set_value((unsigned int)(ts->platform_data->gpio_reset), 0);

    //ret = i2c_smbus_write_byte_data(client, 0x01, 0x00); /* deep sleep */
    //if (ret < 0)
    //printk(KERN_ERR "melfas_ts_suspend: i2c_smbus_write_byte_data failed\n");
    printk(KERN_ERR "melfas_ts_suspend: success!!\n");
    return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    
    if(NULL == ts)
    {
        return -ENODEV;
    }
    gpio_set_value((unsigned int)(ts->platform_data->gpio_reset), 1);
    msleep(MELFAS_TP_UNRESET_TIME);
    
    melfas_init_panel(ts);
    cancel_work_sync(&ts->work);

    //schedule_work(&ts->work);
    queue_work(melfas_ts_wq, &ts->work);/*lint !e534*/
    //enable_irq(client->irq);
    gpio_int_unmask_set((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/
    printk(KERN_ERR "melfas_ts_resume: success!!\n");
    return 0;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
    struct melfas_ts_data *ts;
    int ret = 0;
    ts = container_of(h, struct melfas_ts_data, early_suspend);
    ret = melfas_ts_suspend(ts->client, PMSG_SUSPEND);
    if(ret)
    {
        printk(KERN_ERR "melfas_ts_suspend failed!!!\n");
    }
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
    struct melfas_ts_data *ts;
    int ret = 0;
    ts = container_of(h, struct melfas_ts_data, early_suspend);

    ret = melfas_ts_resume(ts->client);
    if(ret)
    {
        printk(KERN_ERR "melfas_ts_resume failed!!!\n");
    }
}
#endif

static const struct i2c_device_id melfas_ts_id[] =
{
    { MELFAS_TS_NAME, 0 },
    { }
};

static struct i2c_driver melfas_ts_driver =
{
    .driver = {
    .name = MELFAS_TS_NAME,
    .owner = THIS_MODULE,
    },
    .id_table = melfas_ts_id,
    .probe = melfas_ts_probe,
    .remove = __devexit_p(melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend = melfas_ts_suspend,
    .resume = melfas_ts_resume,
#endif
};

static int __init melfas_ts_device_init(void)
{/*lint !e629*/
    int ret = 0;
    
    if (ARRAY_SIZE(bus_i2c_devices)) /*lint !e119 !e30 !e84 !e514*/
    {
        ret = i2c_register_board_info(0, bus_i2c_devices, ARRAY_SIZE(bus_i2c_devices));/*lint !e119 !e30 !e84 !e514*/
        if(ret)/*lint !e514*/
        {
            printk(KERN_ERR "i2c_register_board_info failed!!!\n");
            return ret;
        }
    }
    printk(KERN_ERR "i2c_register_board_info succeed!!!\n");

    return ret;   
}

static int __init melfas_ts_init(void)
{
    return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
    i2c_del_driver(&melfas_ts_driver);
    if (NULL != melfas_ts_wq)
    {
        destroy_workqueue(melfas_ts_wq);
    }
}

MODULE_DESCRIPTION("Driver for Melfas MIP Touchscreen Controller");
MODULE_LICENSE("GPL");

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
postcore_initcall(melfas_ts_device_init);/*lint !e529*/


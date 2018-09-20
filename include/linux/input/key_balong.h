
#ifndef __KEY_BALONG_H__
#define __KEY_BALONG_H__

#include <product_config.h>

#if (FEATURE_ON == MBB_DLOAD_SDUP)
#define KEY_UP   0  //按键抬起
#define KEY_DOWN 1  //按键按下

#define KEY_DETECT_TIME_2000MS   (2000)  /*SD卡升级组合键按下时间长度*/
#define KEY_DETECT_TIME_500MS    (500)   /*SD卡升级触发时双击键检测时间*/
#define DOUBLE_CLK_KEY  (KEY_F24)       /*SD卡升级触发时需要双击的键*/
#endif

typedef enum
{
    INVALID_KEY,
    POWER_KEY,
    MENU_KEY,
    RESET_KEY,
    WIFI_KEY,
    BUTT_KEY
}KEY_ENUM;


void key_int_disable(KEY_ENUM key);


void key_int_enable(KEY_ENUM key);

#endif
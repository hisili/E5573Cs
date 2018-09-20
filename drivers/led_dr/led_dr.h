
#ifndef _BREATH_LED_H
#define _BREATH_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#define PUBLIC 
#define PRIVATE static
#define BR_OK    1
#define BR_ERROR 0

/* 網柲腑袨怓諷秶 */
typedef enum
{
    BREATH_LED_STATE_LIGHT   = 0, /* 網柲腑都謠 */
    BREATH_LED_STATE_SLEEP   = 1, /* 網柲腑都鏢 */
    BREATH_LED_STATE_SPARK   = 2, /* 網柲腑網柲 */
    BREATH_LED_STATE_ENABLE  = 3, /* 網柲腑妏夔 */
    BREATH_LED_STATE_DISABLE = 4, /* 網柲腑壽敕 */
    BREATH_LED_STATE_MAX
}breathled_state;


PUBLIC int breathled_controller(breathled_state led_state);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW)
void bat_yellow_led_set(int flag);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BREATH_LED_H */
#ifndef _AS_GPIO_PIN_H_
#define _AS_GPIO_PIN_H_

#define PIN_VER_4

#if defined(PIN_VER_4)
    /* ver 4 */
    #define GPIO_PIN_SENSOR         26

    #define GPIO_PIN_LED_A0         9
    #define GPIO_PIN_LED_RESET      10

    #define GPIO_PIN_BAT_CHARGE     0
    #define GPIO_PIN_BAT_LOW        1
    #define GPIO_PIN_BAT_FULL       25

    #define GPIO_PIN_BACK_BUTTON    7
    #define GPIO_PIN_FRONT_BUTTON   2
#elif defined(PIN_VER_5)
    /* ver 5 */
    #define GPIO_PIN_SENSOR         26

    #define GPIO_PIN_LED_A0         9
    #define GPIO_PIN_LED_RESET      10

    #define GPIO_PIN_BAT_CHARGE     0
    #define GPIO_PIN_BAT_LOW        1
    #define GPIO_PIN_BAT_FULL       7

    #define GPIO_PIN_BACK_BUTTON    GPIO_PIN_BAT_LOW
    #define GPIO_PIN_FRONT_BUTTON   2
    
    #define GPIO_PIN_TEMP           25
#endif


#endif /* _AS_GPIO_PIN_H_ */
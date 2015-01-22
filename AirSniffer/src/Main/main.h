#ifndef _MAIN_H_
#define _MAIN_H_

//#define USE_WATCHDOG

#ifdef USE_WATCHDOG
    #define WATCHDOG_DEV "/dev/watchdog"
#endif

#define PING_INTERVAL 120

#define ID_FILE "/etc/config/device.id"
#define CONFIG_FILE "/etc/config/device.conf"

#define AP_DEV "wlan1"
#define HOSTAPD_CONFIG_FILE "/etc/config/hostapd.conf"

#define STA_DEV "wlan0"
#define WPA_CTRL_PATH "/var/run/wpa_supplicant/wlan0"
#define WPA_CONFIG "/etc/config/wpa_supplicant.conf"

#define DHCPD_CONFIG "/etc/config/dhcpd.conf"

#define WIFI_SETUP_TIMEOUT 300
#define SIGNAL_FIFO "/tmp/assigfifo"

#define SENSOR_READ_INTERVAL_US 500

#define SENSOR_CALC_INTERVAL 60000

#define SPINNER_FRAME_LEN   (SENSOR_CALC_INTERVAL/SPINNER_FRAME_COUNT)

#define DATA_AVE_NUMBER 10
#define DATA_SEND_INTERVAL 10

#define BATTERY_STATE_CHARGE    (1<<0)
#define BATTERY_STATE_LOW       (1<<1)
#define BATTERY_STATE_FULL      (1<<2)

#define DISPLAY_TYPE_PLEASE_WAIT    1
#define DISPLAY_TYPE_UNIT           2
#define DISPLAY_TYPE_DATA           3
#define DISPLAY_TYPE_PLEASE_SETUP   4
#define DISPLAY_TYPE_SETUP_SUCC     5
#define DISPLAY_TYPE_SETUP_FAIL     6
#define DISPLAY_TYPE_BATTERY        7
#define DISPLAY_TYPE_WIFI_CONN      8
#define DISPLAY_TYPE_TEMP           9
#define DISPLAY_TYPE_SPINNER        10

#define TEMP_NO_DEVICE      -128

#define UNIT_TYPE_PCS   0
#define UNIT_TYPE_UG    1

#define TIMERS_SIZE      10

#endif /* _MAIN_H_ */
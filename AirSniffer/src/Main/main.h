#ifndef _MAIN_H_
#define _MAIN_H_

/*---------------------------------------------------------------------------*
 * Configs
 *---------------------------------------------------------------------------*/
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

#define DATA_AVE_NUMBER 10
#define DATA_SEND_INTERVAL 10

#define BATTERY_STATE_CHARGE    (1<<0)
#define BATTERY_STATE_LOW       (1<<1)
#define BATTERY_STATE_FULL      (1<<2)

#define DISPLAY_TYPE_PLEASE_WAIT    1
#define DISPLAY_TYPE_DATA_BG        2
#define DISPLAY_TYPE_DATA           3
#define DISPLAY_TYPE_PLEASE_SETUP   4
#define DISPLAY_TYPE_SETUP_SUCC     5
#define DISPLAY_TYPE_SETUP_FAIL     6
#define DISPLAY_TYPE_BATTERY        7
#define DISPLAY_TYPE_WIFI_CONN      8
#define DISPLAY_TYPE_TEMP           9
#define DISPLAY_TYPE_TEMP_BG        10

#define UPLOAD_XIVELY   0
#define UPLOAD_ALIYUN   1
#define UPLOAD          UPLOAD_XIVELY

#endif /* _MAIN_H_ */
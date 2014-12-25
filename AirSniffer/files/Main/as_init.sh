#!/bin/sh /etc/rc.common

START=99

APP=AirSniffer

PHY_DEV=phy0
STA_DEV=wlan0
#AP_DEV=wlan1

DEV_CONFIG=/etc/config/device.conf
WPA_CONFIG=/etc/config/wpa_supplicant.conf

TEMP_PIN=25

start() {
    exec 1>/dev/console
    exec 2>/dev/console
    
    echo "[AirSniffer]Starting"
    
    ifconfig $STA_DEV up
    #iw phy $phy interfacce add $AP_DEV type managed
    
    if [ ! -f $WPA_CONFIG ]; then
        cat > $WPA_CONFIG << EOF
ctrl_interface=/var/run/wpa_supplicant
update_config=1

EOF
    fi
    
    wpa_supplicant -B -i $STA_DEV -c $WPA_CONFIG
    udhcpc -b -i $STA_DEV -s /etc/udhcpc.script
    
    insmod w1-gpio-custom bus0=0,$TEMP_PIN,0
    
    if [ -f $DEV_CONFIG ]; then
        $APP &
    else
        echo "========================================================================="
        echo "Please wait till init complete then press ENTER to start console"
        echo "After console started, please run \"./dev_setup.sh\""
        echo "and setup device configuration"
        echo "========================================================================="
    fi
    
    ifconfig br-lan 10.10.10.254
}

stop() {
    killall $APP
}
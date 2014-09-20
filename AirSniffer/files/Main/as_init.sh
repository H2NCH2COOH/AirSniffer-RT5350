#!/bin/sh /etc/rc.common

START=99

APP=AirSniffer

PHY_DEV=phy0
STA_DEV=wlan0
#AP_DEV=wlan1

WPA_CONFIG=/etc/config/wpa_supplicant.conf

start() {
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
    udhcpc -i $STA_DEV -s /etc/udhcpc.script
    #$APP &
}

stop() {
    killall $APP
}
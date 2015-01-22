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
    
    if [ ! -f $DEV_CONFIG ]; then
        cat > $DEV_CONFIG << EOF
dev_id=Null
upload=none
telnet=y
EOF
    fi
    
    if [ ! x$(cat /etc/TZ) == "xBEJ-8" ]; then
        echo "BEJ-8" > /tmp/TZ
    fi
    
    source $DEV_CONFIG
    
    if [ x$telnet != "xy" ]; then
        killall telnetd
    fi
    
    ifconfig br-lan 10.10.10.254
    killall uhttpd
    uhttpd -p 80 -h /www
    wpa_supplicant -B -i $STA_DEV -c $WPA_CONFIG
    sid=$(echo $dev_id | cut -c -8)
    udhcpc -b -i $STA_DEV -s /etc/udhcpc.script -x hostname:AirS_$sid
    
    insmod w1-gpio-custom bus0=0,$TEMP_PIN,0
    $APP &
}

stop() {
    killall $APP
}
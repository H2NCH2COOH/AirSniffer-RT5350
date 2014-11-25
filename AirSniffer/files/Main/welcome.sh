#!/bin/sh /etc/rc.common

START=1

APP=Welcome

start() {
    exec 1>/dev/console
    exec 2>/dev/console
    
    modprobe as-spi-gpio
    modprobe as-spi-gpio-dev
    
    $APP
}

stop() {
    killall $APP
}
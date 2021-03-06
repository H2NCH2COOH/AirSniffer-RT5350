#!/bin/sh

COUNT=$(cat /sys/devices/w1_bus_master1/w1_master_slave_count)

if [ $COUNT -eq 1 ]; then
    ID=$(cat /sys/devices/w1_bus_master1/w1_master_slaves)
    DEV=/sys/devices/w1_bus_master1/$ID/w1_slave
    READ=$(cat $DEV | sed -n '2s/.*t=//p')
    echo $READ
else
    echo -n "No device"
fi

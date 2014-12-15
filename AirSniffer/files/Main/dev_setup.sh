#!/bin/sh

APP=AirSniffer

DEV_CONFIG=/etc/config/device.conf

echo "========================================================================="
echo "Setup device with console"

read -p "Device id: " dev_id
read -p "Feed id: " feed_id
read -p "Api key: " api_key
read -p "MAC addr(12:34:56:ab:cd:ef): " mac_addr

cat > $DEV_CONFIG << EOF
dev_id=$dev_id
feed_id=$feed_id
api_key=$api_key

EOF

cat /dev/mtd2 > /hexd

old_IFS=$IFS
IFS=':'
array=$mac_addr
fs=""
for i in $array; do
    fs="$fs\x$i"
done
IFS=$old_IFS

printf "$fs" | dd of=/hexd bs=1 seek=4 count=6 conv=notrunc

mtd erase factory
mtd write /hexd factory
rm /hexd

echo "Setup complete, reboot in 5 seconds"
sleep 5
reboot
#!/bin/sh

read -p "Change MAC to $1 ?" opt

if [ "$opt" != "y" ]; then
    exit 1
fi

cd /tmp

cat /dev/mtd2 > hexd

old_IFS=$IFS
IFS=':'
array=$1
fs=""
for i in $array; do
    fs="$fs\x$i"
done
IFS=$old_IFS

printf "$fs" | dd of=hexd bs=1 seek=4 count=6 conv=notrunc

mtd erase factory
mtd write hexd factory
rm hexd

echo "=====Change Complete====="
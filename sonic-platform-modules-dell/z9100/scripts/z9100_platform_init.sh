#!/bin/bash

#platform init script for Dell Z9100
found=0
for devnum in 0 1; do
    devname=`cat /sys/bus/i2c/devices/i2c-${devnum}/name`
    # iSMT adapter can be at either dffd0000 or dfff0000
    if [[ $devname == 'SMBus iSMT adapter at '* ]]; then
        found=1
        break
    fi
done

[ $found -eq 0 ] && echo "cannot find iSMT" && exit 1

# Attach Ox70 IOM mux's
echo pca9547 0x70 > /sys/bus/i2c/devices/i2c-${devnum}/new_device
sleep 2
# Attach 0x71 for iom  cpld's
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-4/new_device
sleep 2

# Attach syseeprom
echo 24c02 0x50 > /sys/bus/i2c/devices/i2c-2/new_device

#Attach cpld devices to drivers for each iom
for ((i=14;i<=17;i++));
do
echo  dell_z9100_iom_cpld 0x3e > /sys/bus/i2c/devices/i2c-$i/new_device 
sleep 2
done

for ((i=9;i>=6;i--));
do
# 0x71 mux on the IOM 1
echo "Attaching PCA9548 for IOM-$i"
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-$i/new_device
sleep 2
done

# Attach the SFP modules on PCA9548_2
echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-11/new_device
echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-12/new_device

# Attach 32 instances of EEPROM driver QSFP ports on IO module 1
#eeprom can dump data using below command
for ((i=18;i<=49;i++));
do
echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-$i/new_device 
sleep 2
done


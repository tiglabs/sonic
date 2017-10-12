#!/bin/bash

found=0
for devnum in 0 1; do
    devname=`cat /sys/bus/i2c/devices/i2c-${devnum}/name`
    if [[ $devname == 'SMBus iSMT adapter at dffd0000' ]]; then
        found=1
        break
    fi
done

[ $found -eq 0 ] && echo "cannot found iSMT" && exit 1

# Attach Ox70 IOM mux's
echo pca9547 0x70 > /sys/bus/i2c/devices/i2c-${devnum}/new_device
# Attach 0x71 for iom cpld's
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-4/new_device

# Attach syseeprom
echo 24c02 0x50 > /sys/bus/i2c/devices/i2c-2/new_device

#Attach cpld devices to drivers for each iom
for ((i=14;i<=17;i++)); do
    echo  dell_s6100_iom_cpld 0x3e > /sys/bus/i2c/devices/i2c-$i/new_device 
done

# 0x71,0x72 mux on the IOM 1
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-6/new_device
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-6/new_device
# 0x71,0x72 mux on the IOM 2
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-7/new_device
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-7/new_device
# 0x71,0x72 mux on the IOM 3
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-8/new_device
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-8/new_device
# 0x71,0x72 mux on the IOM 4
echo pca9548 0x71 > /sys/bus/i2c/devices/i2c-9/new_device
echo pca9548 0x72 > /sys/bus/i2c/devices/i2c-9/new_device

# Attach EEPROM driver QSFP ports on IO module 1
#eeprom can dump data using below command
for ((i=18;i<=33;i++)); do
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-$i/new_device 
done

# Attach EEPROM driver QSFP ports on IO module 2
for ((i=34;i<=49;i++)); do
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-$i/new_device 
done

# Attach EEPROM driver QSFP ports on IO module 3 
for ((i=50;i<=65;i++)); do
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-$i/new_device 
done

# Attach EEPROM driver QSFP ports on IO module 4 
for ((i=66;i<=81;i++)); do
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-$i/new_device 
done

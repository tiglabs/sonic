Arista SONiC Support
=====================

Copyright (C) 2016 Arista Networks, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Contents of this package

 - src/
 > Contains source for a kernels modules that add support for SONic platforms
 - initscripts/
 > Initialisation scripts for platforms
 - patches/
 > Required kernel patches
 - utils/
 > Scripts useful during platform bringup

## License

All Linux kernel code is licensed under the GPLv2. All other code is
licensed under the GPLv3. Please see the LICENSE file for copies of
both licenses.

All of the code for interacting with Arista hardware is contained in
the scd kernel module. It has been built and tested against the Linux
3.18 kernel but should be compatible with the Linux 3.16 kernel as
well. The initialization script will modprobe the needed modules, navigate
to the module's device directory in sysfs, and write configuration
data to the kernel module. Once the init\_trigger file is written with
a 1, the sonic-support kernel module will finish initializing the kernel
interfaces for all devices.


## Drivers

This section describes the kernel modules and their dependancies. The
init scripts load the modules in the correct order.

### Scd

The scd module on the DCS-7050QX-32S can be found in
`/sys/bus/pci/drivers/scd/0000:04:00.0`
or
`/sys/bus/pci/drivers/scd/0000:02:00.0`
on the DCS-7050QX-32S, DCS-7060CX2-32S and DCS-7060CX-32S platforms

The scd module must be loaded first on all platforms

### Sonic-support-driver

The sonic-support-driver on all platforms can be found in
`/sonic-support-driver`
within the directory where the scd is loaded

The sonic-support-driver depends on the scd driver which must
be loaded first.

### Fan drivers

The DCS-7050QX-32 platform requires the raven-fan-driver to be loaded.

The DCS-7050QX-32S and DCS-7060CX-32S platforms require the
crow-fan-driver to be loaded.

## Components

This section describes interacting with various components. All the
components are initialized in the init script. The following describes
manual initialization as well as interaction. The examples below
are for a given platform and may differ across platforms.

### LEDs

LED controls can be found in `/sys/class/leds`. The sysfs interface
for LEDs is not very expressive, so the brightness field is used
to toggle between off and different colors. The brightness to LED
color mappings are as follows (0 maps to off for all LEDs):

```
status, fan_status, psu1, psu2:
  0 => off
  1 => green
  2 => red

beacon:
  1+ => blue

qsfp:
  1 => green
  2 => yellow

fan:
  1 => green
  2 => red
  3 => yellow
```

## Fan controls

Fan controls can be found in `/sys/kernel/fan_driver`. Each fan has an
`fan<N>_input` and a `pwm<N>` file for reading and setting the fan speed.

## Temperature sensors

Temperature sensors are controlled by the lm73 and lm90 kernel
modules. If these were compiled into the kernel then after
initializing the kernel module they should already be visible in
`/sys/bus/i2c/drivers/lm73/3-0048` and
`/sys/bus/i2c/drivers/lm90/2-004c`. If they were compiled as modules,
then you will need to modprobe lm73 and modprobe lm90 for their sysfs
entries to show up.

## Power supplies

Power supplies and power controllers can be controlled by the kernel's
generic pmbus module. Assuming the pmbus module was compiled into the
kernel, the sysfs entries can be created as follows:

    echo pmbus 0x4e > /sys/bus/i2c/devices/i2c-3/new_device
    echo pmbus 0x58 > /sys/bus/i2c/devices/i2c-5/new_device
    echo pmbus 0x58 > /sys/bus/i2c/devices/i2c-6/new_device
    echo pmbus 0x4e > /sys/bus/i2c/devices/i2c-7/new_device

### EEPROM

The EEPOM containing the SKU, serial number, and other information can
be accessed with the eeprom kernel module. After using modprobe eeprom
to detect the eeprom, it will show up in sysfs at
`/sys/bus/i2c/drivers/eeprom/1-0052`.

The included prefdl utility can be used to decode the raw output of
the EEPROM. For example,

    bash# cat /sys/bus/i2c/drivers/eeprom/1-0052/eeprom | prefdl -
    SerialNumber: JAS13011012
    SKU: DCS-7050QX-32

### QSFPs

QSFP+ modules are managed by the sff8436 kernel driver. These can be
instantiated by the init script but can manually be instatiated as:

    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-10/new_device
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-11/new_device
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-12/new_device
    ...
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-40/new_device
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-41/new_device

After instantiation, the EEPROM information can be read like so:

```
-bash-4.1# cat /sys/bus/i2c/devices/19-0050/eeprom | hexdump -C
00000000  0d 00 02 f0 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 19 5c  00 00 7f 9c 00 00 00 00  |.......\........|
00000020  00 00 1f cd 20 2e 26 b8  22 94 00 00 00 00 00 00  |.... .&.".......|
00000030  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000070  00 00 00 00 00 04 02 00  00 00 00 00 00 00 00 00  |................|
00000080  0d 00 0c 04 00 00 00 00  00 00 00 05 67 00 00 32  |............g..2|
00000090  00 00 00 00 41 72 69 73  74 61 20 4e 65 74 77 6f  |....Arista Netwo|
000000a0  72 6b 73 20 00 00 1c 73  51 53 46 50 2d 34 30 47  |rks ...sQSFP-40G|
000000b0  2d 53 52 34 20 20 20 20  30 33 42 68 07 d0 46 0d  |-SR4    03Bh..F.|
000000c0  00 00 0f de 58 4d 44 31  34 30 34 30 30 35 56 52  |....XMD1404005VR|
000000d0  20 20 20 20 31 34 30 31  32 36 20 20 08 00 00 d2  |    140126  ....|
000000e0  10 03 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000000f0  00 00 00 00 00 00 02 f8  00 00 00 00 98 44 64 d1  |.............Dd.|
00000100  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000180  40 6b a1 2e 3c f4 2b eb  cd c1 e9 51 50 93 bb fe  |@k..<.+....QP...|
00000190  05 aa 32 3f 1c 4c 00 00  00 00 00 00 00 00 00 00  |..2?.L..........|
000001a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000200  4b 00 fb 00 46 00 00 00  00 00 00 00 00 00 00 00  |K...F...........|
00000210  8d cc 74 04 87 5a 7a 75  00 00 00 00 00 00 00 00  |..t..Zzu........|
00000220  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000230  55 76 01 be 43 e2 04 62  13 88 00 fa 12 8e 01 f4  |Uv..C..b........|
00000240  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000260  00 00 00 00 00 00 00 00  00 00 00 00 33 33 77 77  |............33ww|
00000270  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000280
```

Before being read, the QSFP+ modules must be taken out of reset and
have their module select signals asserted. This can be done through
the GPIO interface. GPIOs are exposed in the sysfs directory for the
scd (they may also be accessed through `/sys/class/gpio`, but named
aliases are created in the scd directory for convenient access).

For example, after initialization, you could run the following
commands to ensure QSFP10 is out of reset and has modsel asserted
before reading it:

    echo out > qsfp10_reset/direction
    echo 0 > qsfp10_reset/value
    echo out > qsfp10_modsel
    echo 1 > qsfp10_value

### SFPs

Initializing the SFP+ is done in the same manner.

    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-42/new_device
    ...
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-44/new_device
    echo sff8436 0x50 > /sys/bus/i2c/devices/i2c-45/new_device

### QSFP - SFP multiplexing

In the Arista DCS-7050QX-32S model, the first 4 QSFP and 4 SFP ports are
multiplexed. To choose between the two use the interface located at
`pathtoscd/mux_sfp_qsfp`

### GPIOs

All GPIOs have their active\_low values correctly set by the
initialization script, so for all GPIOs they may be asserted by
writing a 1 and de-asserted by writing a 0. Read-only GPIOs will not
have a direction file.

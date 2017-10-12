#!/usr/bin/python

# On S6100, the Platform Management Controller runs the
# thermal algorithm. It provides a mailbox for the Host
# to query relevant thermals. The dell_mailbox module
# provides the sysfs support for the following objects:
#   * onboard temperature sensors
#   * FAN trays
#   * PSU

import os
import sys
import logging

S6100_MAX_FAN_TRAYS = 4
S6100_MAX_PSUS = 2
S6100_MAX_IOMS = 4

MAILBOX_DIR="/sys/devices/platform/dell_s6100_lpc"

iom_status_list = [ ]

#Get a mailbox register
def get_pmc_register(reg_name):
    retval = 'ERR'
    mb_reg_file = MAILBOX_DIR+'/'+reg_name
    
    if ( not os.path.isfile(mb_reg_file)):
        print mb_reg_file,  'not found !'
        return retval

    try:
        with open(mb_reg_file, 'r') as fd:
            retval = fd.read()
    except Exception as error:
        logging.error("Unable to open ", mb_reg_file , "file !")

    retval = retval.rstrip('\r\n')
    return retval;

logging.basicConfig(level=logging.DEBUG)

if (os.path.isdir(MAILBOX_DIR)):
    print 'dell-s6100-lpc'
    print 'Adapter: S6100 Platform Management Controller'
else:
    logging.error('S6100 Platform Management Controller module not loaded !')
    sys.exit(0)

#Print the information for temperature sensors
def print_temperature_sensors():
    print("Onboard Temperature Sensors:")
    print '  CPU:                            ' , get_pmc_register('temp_sensor_1') , 'C'
    print '  BCM56960 (PSU side):            ' , get_pmc_register('temp_sensor_2') , 'C'
    print '  System Outlet 1 (switch board): ' , get_pmc_register('temp_sensor_3') , 'C'
    print '  BCM56960 (IO side):             ' , get_pmc_register('temp_sensor_4') , 'C'
    print '  System Outlet 2 (CPU board):    ' , get_pmc_register('temp_sensor_9') , 'C'
    print '  System Inlet Left (IO side):    ' , get_pmc_register('temp_sensor_10') , 'C'
    print '  System Inlet Right (IO side):   ' , get_pmc_register('temp_sensor_11') , 'C'

    iom_status = get_pmc_register('iom_status')
    iom_status = int(iom_status, 16)

    iom_presence = get_pmc_register('iom_presence')

    if (iom_presence != 'ERR'):
        iom_presence = int(iom_presence, 16)

        # IOM presence : 0 => IOM present
        # Temperature sensors 5..8 correspond to IOM 1..4
        for iom in range(0,S6100_MAX_IOMS):
            if (~iom_presence & (1<<iom)):
                iom_sensor_indx = iom + 5
                print '  IOM '+ str(iom + 1) +':\t\t\t  ' , get_pmc_register('temp_sensor_'+str(iom_sensor_indx)) , 'C'

                # Save the IOM Status for later use
                if (~iom_status & (1<<iom)):
                    iom_status_list.append('ON')
                else:
                    iom_status_list.append('OFF')
            else:
                iom_status_list.append('Not present')
                print '  IOM '+ str(iom + 1) +':\t\t\t  ', 'Not present'
    else:
        logging.error('Unable to check IOM presence')

print_temperature_sensors()

#Print the information for 1 Fan Tray
def print_fan_tray(tray):

    Fan_Status = ['Normal', 'Abnormal']
    Airflow_Direction = ['B2F', 'F2B']

    print '  Fan Tray ' + str(tray) + ':'
    print '    Speed:    ' , get_pmc_register('fan_tray_'+str(tray)+'_speed') , 'RPM'

    # Each fan tray has 2 fans. Each region register is 8 bits and has
    # information for 4 trays. The region_b has information about the first
    # 4 trays.
    if ( tray <= 4 ):
        fan_status_region = int(get_pmc_register('fan_status_region_b'), 16)
    else:
        fan_status_region = int(get_pmc_register('fan_status_region_a'), 16)

    fan1_shift = (tray-1)*2
    fan1_status = ( fan_status_region & (1<<fan1_shift) ) >> fan1_shift
    fan2_shift = ((tray-1)*2) + 1
    fan2_status = ( fan_status_region & (1<<fan2_shift) ) >> fan2_shift

    print '    Fan 1:    ', Fan_Status[fan1_status]
    print '    Fan 2:    ', Fan_Status[fan2_status]

    air_flow_reg = int(get_pmc_register('fan_tray_airflow'), 16)
    air_flow_status = air_flow_reg & (1<<(tray-1)) >> (tray-1)
    print '    Air Flow: ', Airflow_Direction[air_flow_status]


print('\nFan Trays:')
fan_tray_presence = get_pmc_register('fan_tray_presence')

if (fan_tray_presence != 'ERR'):
    fan_tray_presence = int(fan_tray_presence, 16)

    for tray in range(0,S6100_MAX_FAN_TRAYS):
        if (fan_tray_presence & (1<<tray)):
            print_fan_tray(tray + 1)
        else:
            print '\n  Fan Tray ' + str(tray + 1) + ':  Not present'
else:
    logging.error('Unable to read FAN presence')

#Print the information for 1 PSU
def print_psu(psu):
    Psu_Type = ['Normal', 'Mismatch']
    Psu_Input_Type = ['AC', 'DC']
    PSU_STATUS_TYPE_BIT = 4
    PSU_STATUS_INPUT_TYPE_BIT = 1
    PSU_FAN_PRESENT_BIT=2
    PSU_FAN_STATUS_BIT=1
    PSU_FAN_AIR_FLOW_BIT=0
    Psu_Fan_Presence = ['Present', 'Absent']
    Psu_Fan_Status = ['Normal', 'Abnormal']
    Psu_Fan_Airflow = ['B2F', 'F2B']

    print '  PSU ' + str(psu) + ':'
    psu_status  = int(get_pmc_register('psu_'+str(psu)+'_status'), 16)

    psu_type = (psu_status & (1<<PSU_STATUS_TYPE_BIT)) >> PSU_STATUS_TYPE_BIT
    psu_input_type = (psu_status & (1<<PSU_STATUS_INPUT_TYPE_BIT)) >> PSU_STATUS_INPUT_TYPE_BIT

    print '    Input:          ' , Psu_Input_Type[psu_input_type]
    print '    Type:           ' , Psu_Type[psu_type]

    # PSU FAN details
    print '    FAN Speed:      ' , get_pmc_register('psu_'+str(psu)+'_fan_speed') , 'RPM'
    psu_fan_status_reg = int(get_pmc_register('psu_'+str(psu)+'_fan_status'))
    psu_fan_present = (psu_fan_status_reg & (1<<PSU_FAN_PRESENT_BIT)) >> PSU_FAN_PRESENT_BIT 
    psu_fan_status = (psu_fan_status_reg & (1<<PSU_FAN_STATUS_BIT)) >> PSU_FAN_STATUS_BIT 
    psu_fan_airflow = (psu_fan_status_reg & (1<<PSU_FAN_AIR_FLOW_BIT)) >> PSU_FAN_AIR_FLOW_BIT 
    print '    FAN:            ' , Psu_Fan_Presence[psu_fan_present]
    print '    FAN Status:     ' , Psu_Fan_Status[psu_fan_status]
    print '    FAN AIRFLOW:    ' , Psu_Fan_Airflow[psu_fan_airflow]

    # PSU input & output monitors
    input_voltage = float(get_pmc_register('psu_'+str(psu)+'_input_voltage')) / 100
    print '    Input Voltage:   %6.2f' % (input_voltage) , 'V'

    output_voltage = float(get_pmc_register('psu_'+str(psu)+'_output_voltage')) / 100
    print '    Output Voltage:  %6.2f' % (output_voltage) , 'V'

    input_current = float(get_pmc_register('psu_'+str(psu)+'_input_current')) / 100
    print '    Input Current:   %6.2f' % (input_current) , 'A'

    output_current = float(get_pmc_register('psu_'+str(psu)+'_output_current')) / 100
    print '    Output Current:  %6.2f' % (output_current) , 'A'

    input_power = float(get_pmc_register('psu_'+str(psu)+'_input_power')) / 10
    print '    Input Power:     %6.2f' % (input_power) , 'W'

    output_power = float(get_pmc_register('psu_'+str(psu)+'_output_power')) / 10
    print '    Output Power:    %6.2f' % (output_power) , 'W'

    # PSU firmware gives spurious temperature reading without input power
    if ( input_power != 0 ):
        print '    Temperature:    ' , get_pmc_register('psu_'+str(psu)+'_temperature') , 'C'
    else:
        print '    Temperature:    ' , 'NA'

print('\nPSUs:')
for psu in range(1,S6100_MAX_PSUS+1):
    psu_status = get_pmc_register('psu_'+str(psu)+'_status')

    if ( psu_status != 'ERR' ):
        psu_status = int(psu_status, 16)

        # Check for PSU presence
        if (~psu_status & 0b1):
            print_psu(psu)
        else:
            print '\n  PSU ' , psu, 'Not present'
    else:
        logging.error('Unable to check PSU presence')

print '\n  Total Power:      ', get_pmc_register('psu_total_power'), 'W'

print('\nIO Modules:')

for iom in range(1,S6100_MAX_IOMS+1):
    print '  IOM '+ str(iom) + ': ' + iom_status_list[iom-1]

print '\n'


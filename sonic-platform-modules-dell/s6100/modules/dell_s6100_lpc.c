
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
//for platform drivers....
#include <linux/platform_device.h>

#define DRIVER_NAME "dell_s6100_lpc"

/* These Addresses defined by the Hardware Designer... 
   Simply added to demostrate.*/

#define RESOURCE1_START_ADDRESS 0x200
#define RESOURCE1_END_ADDRESS   0x260

/* SMF Registers */

#define SMF_REG_VERSION   0x200
#define SMF_MB_ADDR_HI    0x210
#define SMF_MB_ADDR_LO    0x211
#define SMF_MB_READ_DATA  0x212
#define SMF_MB_WRITE_DATA  0x213

/* System LED Registers */

#define SYS_STATUS_LED       0x130

/* IOM presence */

#define IO_MODULE_STATUS   0x0310
#define IO_MODULE_PRESENCE 0x0311

/* SMF temp sensors */

#define S6100_MAX_TEMP_SENSORS 11

#define MAX_NUM_TEMP_SENSORS 0x0013

#define TEMP_1_SENS  0x0014
#define TEMP_2_SENS  0x0016
#define TEMP_3_SENS  0x0018
#define TEMP_4_SENS  0x001A
#define TEMP_5_SENS  0x001C
#define TEMP_6_SENS  0x001E
#define TEMP_7_SENS  0x0020
#define TEMP_8_SENS  0x0022
#define TEMP_9_SENS  0x0024
#define TEMP_10_SENS  0x0026
#define TEMP_11_SENS  0x0028

/* FAN Tray */

#define S6100_MAX_NUM_FAN_TRAYS 4

#define MAX_NUM_FAN_TRAYS     0x00F0
#define MAX_NUM_FANS_PER_TRAY 0x00F1
#define FAN_TRAY_PRESENCE     0x0113
#define FAN_STATUS_GROUP_A    0x0114
#define FAN_STATUS_GROUP_B    0x0115
#define FAN_TRAY_AIRFLOW      0x0116

#define FAN_TRAY_1_SPEED  0x00F3
#define FAN_TRAY_2_SPEED  0x00F7
#define FAN_TRAY_3_SPEED  0x00FB
#define FAN_TRAY_4_SPEED  0x00FF

/* PSUs */

#define S6100_MAX_NUM_PSUS 2

#define MAX_NUM_PSUS           0x0231
#define CURRENT_TOTAL_POWER    0x0232

// PSU1
#define PSU_1_MAX_POWER        0x0234
#define PSU_1_FUNCTION_SUPPORT 0x0236
#define PSU_1_STATUS           0x0237
#define PSU_1_TEMPERATURE      0x0239
#define PSU_1_FAN_SPEED        0x023B
#define PSU_1_FAN_STATUS       0x023D
#define PSU_1_INPUT_VOLTAGE    0x023E
#define PSU_1_OUTPUT_VOLTAGE   0x0240
#define PSU_1_INPUT_CURRENT    0x0242
#define PSU_1_OUTPUT_CURRENT   0x0244
#define PSU_1_INPUT_POWER      0x0246
#define PSU_1_OUTPUT_POWER     0x0248

// PSU2
#define PSU_2_MAX_POWER        0x026D
#define PSU_2_FUNCTION_SUPPORT 0x026F
#define PSU_2_STATUS           0x0270
#define PSU_2_TEMPERATURE      0x0272
#define PSU_2_FAN_SPEED        0x0274
#define PSU_2_FAN_STATUS       0x0276
#define PSU_2_INPUT_VOLTAGE    0x0277
#define PSU_2_OUTPUT_VOLTAGE   0x0279
#define PSU_2_INPUT_CURRENT    0x027B
#define PSU_2_OUTPUT_CURRENT   0x027D
#define PSU_2_INPUT_POWER      0x027F
#define PSU_2_OUTPUT_POWER     0x0281

/* Optics Table */
#define MAX_NUM_OPTICS 0x410

unsigned long  *mmio;

spinlock_t smf_slock;

/* SMF read function */

static uint8_t sfm_read_byte(int addr)
{
    u8 retval;

    spin_lock(&smf_slock);
    outb(((addr >> 8) & 0xff),SMF_MB_ADDR_HI);
    outb((addr & 0xff),SMF_MB_ADDR_LO);
    retval = inb(SMF_MB_READ_DATA);
    spin_unlock(&smf_slock);

    return retval;
}

/* change ver to info */

static int sfm_read_two_byte(u16 offset)
{

    u8 low_byte=0,high_byte=0;
    high_byte = sfm_read_byte(offset);
    low_byte =  sfm_read_byte(offset+1);

    return ((high_byte <<8) | ( low_byte));

}


/* Specifying my resources information */
static struct resource dell_s6100_lpc_resources[] = {
    {
        .start          = RESOURCE1_START_ADDRESS,
        .end            = RESOURCE1_END_ADDRESS,
        .flags          = IORESOURCE_IO,
    },
};

static void dell_s6100_lpc_dev_release( struct device * dev)
{
    return ;
}

static struct platform_device dell_s6100_lpc_dev = {
    .name           = DRIVER_NAME,
    .id             = -1,
    .num_resources  = ARRAY_SIZE(dell_s6100_lpc_resources),
    .resource       = dell_s6100_lpc_resources,
    .dev = {
        .release = dell_s6100_lpc_dev_release,

    }
};

/* change ver to info */
static ssize_t get_smfver(struct device *dev, struct device_attribute *devattr, char *buf)
{
    printk( KERN_ALERT " Dev attributed fetched:%s\n", devattr->attr.name);
    return sprintf(buf, "%x\n",inb(SMF_REG_VERSION));
}

static ssize_t get_sys_led(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", inb(SYS_STATUS_LED));
}

static ssize_t get_max_num_optics(struct device *dev, struct device_attribute *devattr,
		char *buf)
{
    return sprintf(buf,"%d\n",sfm_read_byte(MAX_NUM_OPTICS));
}

static ssize_t set_sys_led(struct device *dev, struct device_attribute *devattr, const char *buf, 
        size_t count)
{
    unsigned long devdata;
    int err;

    err = kstrtoul(buf,16, &devdata);

    if (err)
        return err;

    outb(devdata,SYS_STATUS_LED);
    return count;
}

static ssize_t get_fan_tray_presence(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%02hhx\n", ~sfm_read_byte(FAN_TRAY_PRESENCE));
}

static ssize_t get_max_num_temp_sensors(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%hhd\n", sfm_read_byte(MAX_NUM_TEMP_SENSORS));
}

static ssize_t get_max_num_fan_trays(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%hhx\n", sfm_read_byte(MAX_NUM_FAN_TRAYS));
}

static ssize_t get_max_num_fans_per_tray(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(MAX_NUM_FANS_PER_TRAY));
}

static ssize_t get_fan_status_region_a(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(FAN_STATUS_GROUP_A));
}

static ssize_t get_fan_status_region_b(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(FAN_STATUS_GROUP_B));
}

static ssize_t get_fan_tray_airflow(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(FAN_TRAY_AIRFLOW));
}

static ssize_t get_max_num_psus(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(MAX_NUM_PSUS));
}

static ssize_t get_psu_total_power(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%d\n", (sfm_read_two_byte(CURRENT_TOTAL_POWER)/10));
}

static ssize_t get_iom_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(IO_MODULE_STATUS));
}


static ssize_t get_iom_presence(struct device *dev, struct device_attribute *devattr, char *buf)
{
    return sprintf(buf,"%x\n", sfm_read_byte(IO_MODULE_PRESENCE));
}


/* Temp Sensors */

#define GET_TEMP_SENSOR(index) \
static ssize_t get_temp_##index##_sensor(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(TEMP_##index##_SENS)/10)); \
}

GET_TEMP_SENSOR(1)
GET_TEMP_SENSOR(2)
GET_TEMP_SENSOR(3)
GET_TEMP_SENSOR(4)
GET_TEMP_SENSOR(5)
GET_TEMP_SENSOR(6)
GET_TEMP_SENSOR(7)
GET_TEMP_SENSOR(8)
GET_TEMP_SENSOR(9)
GET_TEMP_SENSOR(10)
GET_TEMP_SENSOR(11)

/* FAN Trays */

#define GET_FAN_TRAY_SPEED(index) \
static ssize_t get_fan_tray_##index##_speed(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", sfm_read_two_byte(FAN_TRAY_##index##_SPEED)); \
}

GET_FAN_TRAY_SPEED(1)
GET_FAN_TRAY_SPEED(2)
GET_FAN_TRAY_SPEED(3)
GET_FAN_TRAY_SPEED(4)

/* PSUs */

#define GET_PSU_MAX_POWER(index) \
static ssize_t get_psu_##index##_max_power(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_MAX_POWER)/10)); \
}

#define GET_PSU_FUNCTION_SUPPORT(index) \
static ssize_t get_psu_##index##_function_support(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_byte(PSU_##index##_FUNCTION_SUPPORT))); \
}

#define GET_PSU_STATUS(index) \
static ssize_t get_psu_##index##_status(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_byte(PSU_##index##_STATUS))); \
}

#define GET_PSU_TEMPERATURE(index) \
static ssize_t get_psu_##index##_temperature(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_TEMPERATURE)/10)); \
}

#define GET_PSU_FAN_SPEED(index) \
static ssize_t get_psu_##index##_fan_speed(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_FAN_SPEED))); \
}

#define GET_PSU_FAN_STATUS(index) \
static ssize_t get_psu_##index##_fan_status(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_byte(PSU_##index##_FAN_STATUS))); \
}

#define GET_PSU_INPUT_VOLTAGE(index) \
static ssize_t get_psu_##index##_input_voltage(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_INPUT_VOLTAGE))); \
}

#define GET_PSU_OUTPUT_VOLTAGE(index) \
static ssize_t get_psu_##index##_output_voltage(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_OUTPUT_VOLTAGE))); \
}

#define GET_PSU_INPUT_CURRENT(index) \
static ssize_t get_psu_##index##_input_current(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_INPUT_CURRENT))); \
}

#define GET_PSU_OUTPUT_CURRENT(index) \
static ssize_t get_psu_##index##_output_current(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_OUTPUT_CURRENT))); \
}

#define GET_PSU_INPUT_POWER(index) \
static ssize_t get_psu_##index##_input_power(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_INPUT_POWER))); \
}

#define GET_PSU_OUTPUT_POWER(index) \
static ssize_t get_psu_##index##_output_power(struct device *dev, struct device_attribute *devattr, char *buf) \
{ \
    return sprintf(buf,"%d\n", (sfm_read_two_byte(PSU_##index##_OUTPUT_POWER))); \
}

GET_PSU_MAX_POWER(1)
GET_PSU_FUNCTION_SUPPORT(1)
GET_PSU_STATUS(1)
GET_PSU_TEMPERATURE(1)
GET_PSU_FAN_SPEED(1)
GET_PSU_FAN_STATUS(1)
GET_PSU_INPUT_VOLTAGE(1)
GET_PSU_OUTPUT_VOLTAGE(1)
GET_PSU_INPUT_CURRENT(1)
GET_PSU_OUTPUT_CURRENT(1)
GET_PSU_INPUT_POWER(1)
GET_PSU_OUTPUT_POWER(1)

GET_PSU_MAX_POWER(2)
GET_PSU_FUNCTION_SUPPORT(2)
GET_PSU_STATUS(2)
GET_PSU_TEMPERATURE(2)
GET_PSU_FAN_SPEED(2)
GET_PSU_FAN_STATUS(2)
GET_PSU_INPUT_VOLTAGE(2)
GET_PSU_OUTPUT_VOLTAGE(2)
GET_PSU_INPUT_CURRENT(2)
GET_PSU_OUTPUT_CURRENT(2)
GET_PSU_INPUT_POWER(2)
GET_PSU_OUTPUT_POWER(2)

// Attribute declarations

static DEVICE_ATTR(smf_ver,S_IRUGO,get_smfver, NULL);
static DEVICE_ATTR(sys_led,S_IRUGO | S_IWUSR,get_sys_led,set_sys_led);
static DEVICE_ATTR(iom_status,S_IRUGO | S_IWUSR,get_iom_status,NULL);
static DEVICE_ATTR(iom_presence,S_IRUGO | S_IWUSR,get_iom_presence,NULL);

static DEVICE_ATTR(max_num_temp_sensors,S_IRUGO,get_max_num_temp_sensors, NULL);
static DEVICE_ATTR(temp_sensor_1,S_IRUGO,get_temp_1_sensor, NULL);
static DEVICE_ATTR(temp_sensor_2,S_IRUGO,get_temp_2_sensor, NULL);
static DEVICE_ATTR(temp_sensor_3,S_IRUGO,get_temp_3_sensor, NULL);
static DEVICE_ATTR(temp_sensor_4,S_IRUGO,get_temp_4_sensor, NULL);
static DEVICE_ATTR(temp_sensor_5,S_IRUGO,get_temp_5_sensor, NULL);
static DEVICE_ATTR(temp_sensor_6,S_IRUGO,get_temp_6_sensor, NULL);
static DEVICE_ATTR(temp_sensor_7,S_IRUGO,get_temp_7_sensor, NULL);
static DEVICE_ATTR(temp_sensor_8,S_IRUGO,get_temp_8_sensor, NULL);
static DEVICE_ATTR(temp_sensor_9,S_IRUGO,get_temp_9_sensor, NULL);
static DEVICE_ATTR(temp_sensor_10,S_IRUGO,get_temp_10_sensor, NULL);
static DEVICE_ATTR(temp_sensor_11,S_IRUGO,get_temp_11_sensor, NULL);

static DEVICE_ATTR(max_num_fan_trays,S_IRUGO,get_max_num_fan_trays, NULL);
static DEVICE_ATTR(max_num_fans_per_tray,S_IRUGO,get_max_num_fans_per_tray, NULL);
static DEVICE_ATTR(fan_tray_presence,S_IRUGO,get_fan_tray_presence, NULL);
static DEVICE_ATTR(fan_status_region_a,S_IRUGO,get_fan_status_region_a, NULL);
static DEVICE_ATTR(fan_status_region_b,S_IRUGO,get_fan_status_region_b, NULL);
static DEVICE_ATTR(fan_tray_airflow,S_IRUGO,get_fan_tray_airflow, NULL);

static DEVICE_ATTR(fan_tray_1_speed,S_IRUGO,get_fan_tray_1_speed, NULL);
static DEVICE_ATTR(fan_tray_2_speed,S_IRUGO,get_fan_tray_2_speed, NULL);
static DEVICE_ATTR(fan_tray_3_speed,S_IRUGO,get_fan_tray_3_speed, NULL);
static DEVICE_ATTR(fan_tray_4_speed,S_IRUGO,get_fan_tray_4_speed, NULL);

static DEVICE_ATTR(max_num_psus,S_IRUGO,get_max_num_psus, NULL);
static DEVICE_ATTR(psu_total_power,S_IRUGO,get_psu_total_power, NULL);

static DEVICE_ATTR(psu_1_max_power,S_IRUGO,get_psu_1_max_power, NULL);
static DEVICE_ATTR(psu_1_function_support,S_IRUGO,get_psu_1_function_support, NULL);
static DEVICE_ATTR(psu_1_status,S_IRUGO,get_psu_1_status, NULL);
static DEVICE_ATTR(psu_1_temperature,S_IRUGO,get_psu_1_temperature, NULL);
static DEVICE_ATTR(psu_1_fan_speed,S_IRUGO,get_psu_1_fan_speed, NULL);
static DEVICE_ATTR(psu_1_fan_status,S_IRUGO,get_psu_1_fan_status, NULL);
static DEVICE_ATTR(psu_1_input_voltage,S_IRUGO,get_psu_1_input_voltage, NULL);
static DEVICE_ATTR(psu_1_output_voltage,S_IRUGO,get_psu_1_output_voltage, NULL);
static DEVICE_ATTR(psu_1_input_current,S_IRUGO,get_psu_1_input_current, NULL);
static DEVICE_ATTR(psu_1_output_current,S_IRUGO,get_psu_1_output_current, NULL);
static DEVICE_ATTR(psu_1_input_power,S_IRUGO,get_psu_1_input_power, NULL);
static DEVICE_ATTR(psu_1_output_power,S_IRUGO,get_psu_1_output_power, NULL);

static DEVICE_ATTR(psu_2_max_power,S_IRUGO,get_psu_2_max_power, NULL);
static DEVICE_ATTR(psu_2_function_support,S_IRUGO,get_psu_2_function_support, NULL);
static DEVICE_ATTR(psu_2_status,S_IRUGO,get_psu_2_status, NULL);
static DEVICE_ATTR(psu_2_temperature,S_IRUGO,get_psu_2_temperature, NULL);
static DEVICE_ATTR(psu_2_fan_speed,S_IRUGO,get_psu_2_fan_speed, NULL);
static DEVICE_ATTR(psu_2_fan_status,S_IRUGO,get_psu_2_fan_status, NULL);
static DEVICE_ATTR(psu_2_input_voltage,S_IRUGO,get_psu_2_input_voltage, NULL);
static DEVICE_ATTR(psu_2_output_voltage,S_IRUGO,get_psu_2_output_voltage, NULL);
static DEVICE_ATTR(psu_2_input_current,S_IRUGO,get_psu_2_input_current, NULL);
static DEVICE_ATTR(psu_2_output_current,S_IRUGO,get_psu_2_output_current, NULL);
static DEVICE_ATTR(psu_2_input_power,S_IRUGO,get_psu_2_input_power, NULL);
static DEVICE_ATTR(psu_2_output_power,S_IRUGO,get_psu_2_output_power, NULL);

static DEVICE_ATTR(max_num_optics,S_IRUGO | S_IWUSR,get_max_num_optics,NULL);

static struct attribute *s6100_lpc_attrs[] = {
    &dev_attr_smf_ver.attr,
    &dev_attr_max_num_temp_sensors.attr,
    &dev_attr_max_num_fan_trays.attr,
    &dev_attr_max_num_fans_per_tray.attr,
    &dev_attr_fan_tray_presence.attr,
    &dev_attr_fan_status_region_a.attr,
    &dev_attr_fan_status_region_b.attr,
    &dev_attr_fan_tray_airflow.attr,
    &dev_attr_max_num_psus.attr,
    &dev_attr_sys_led.attr,
    &dev_attr_max_num_optics.attr,
    &dev_attr_iom_status.attr,
    &dev_attr_iom_presence.attr,
    NULL,
};

static struct attribute *s6100_temperature_attrs[] = {
    &dev_attr_temp_sensor_1.attr,
    &dev_attr_temp_sensor_2.attr,
    &dev_attr_temp_sensor_3.attr,
    &dev_attr_temp_sensor_4.attr,
    &dev_attr_temp_sensor_5.attr,
    &dev_attr_temp_sensor_6.attr,
    &dev_attr_temp_sensor_7.attr,
    &dev_attr_temp_sensor_8.attr,
    &dev_attr_temp_sensor_9.attr,
    &dev_attr_temp_sensor_10.attr,
    &dev_attr_temp_sensor_11.attr,
    NULL,
};

static struct attribute *s6100_fantray_attrs[] = {
    &dev_attr_fan_tray_1_speed.attr,
    &dev_attr_fan_tray_2_speed.attr,
    &dev_attr_fan_tray_3_speed.attr,
    &dev_attr_fan_tray_4_speed.attr,
    NULL,
};

static struct attribute *s6100_psu_attrs[] = {
    &dev_attr_psu_total_power.attr,
    &dev_attr_psu_1_max_power.attr,
    &dev_attr_psu_1_function_support.attr,
    &dev_attr_psu_1_status.attr,
    &dev_attr_psu_1_temperature.attr,
    &dev_attr_psu_1_fan_speed.attr,
    &dev_attr_psu_1_fan_status.attr,
    &dev_attr_psu_1_input_voltage.attr,
    &dev_attr_psu_1_output_voltage.attr,
    &dev_attr_psu_1_input_current.attr,
    &dev_attr_psu_1_output_current.attr,
    &dev_attr_psu_1_input_power.attr,
    &dev_attr_psu_1_output_power.attr,
    &dev_attr_psu_2_max_power.attr,
    &dev_attr_psu_2_function_support.attr,
    &dev_attr_psu_2_status.attr,
    &dev_attr_psu_2_temperature.attr,
    &dev_attr_psu_2_fan_speed.attr,
    &dev_attr_psu_2_fan_status.attr,
    &dev_attr_psu_2_input_voltage.attr,
    &dev_attr_psu_2_output_voltage.attr,
    &dev_attr_psu_2_input_current.attr,
    &dev_attr_psu_2_output_current.attr,
    &dev_attr_psu_2_input_power.attr,
    &dev_attr_psu_2_output_power.attr,
    NULL,
};

static struct attribute_group s6100_lpc_attr_grp = {
    .attrs = s6100_lpc_attrs,
};
static struct attribute_group s6100_temperature_attr_grp = {
    .attrs = s6100_temperature_attrs,
};

static struct attribute_group s6100_fantray_attr_grp = {
    .attrs = s6100_fantray_attrs,
};

static struct attribute_group s6100_psu_attr_grp = {
    .attrs = s6100_psu_attrs,
};


static int dell_s6100_lpc_drv_probe(struct platform_device *pdev)
{
    struct resource *res;
    int ret =0;

    res = platform_get_resource(pdev, IORESOURCE_IO,0);
    if (unlikely(!res)) {
        printk(KERN_ERR " Specified Resource Not Available... 1\n");
        return -1;
    }

    printk(KERN_INFO "Start:%lx,  End:%lx Size:%ld\n", (unsigned long)res->start, 
            (unsigned long)res->end, (unsigned long)resource_size(res));

    //Converting the physical Address to virtual for using in driver 
    mmio = ioremap(res->start, resource_size(res));
    if (unlikely(!mmio)) {
        printk(KERN_ERR " (cannot map IO)\n");
        return -1;
    }

    printk(KERN_INFO "\n Registering Sysfs \n");
    /* Register sysfs hooks */
    ret = sysfs_create_group(&pdev->dev.kobj, &s6100_lpc_attr_grp);
    if (ret) {
        printk(KERN_ERR "Cannot create sysfs\n");
    }

    printk(KERN_ERR "Attemting to create temperature attributes");
    ret = sysfs_create_group(&pdev->dev.kobj, &s6100_temperature_attr_grp);
    if (ret) {
        printk(KERN_ERR "Cannot create temperature sysfs\n");
    }

    printk(KERN_ERR "Attemting to create Fantray attributes");
    ret = sysfs_create_group(&pdev->dev.kobj, &s6100_fantray_attr_grp);
    if (ret) {
        printk(KERN_ERR "Cannot create fantray sysfs\n");
    }

    printk(KERN_ERR "Attemting to create PSU attributes");
    ret = sysfs_create_group(&pdev->dev.kobj, &s6100_psu_attr_grp);
    if (ret) {
        printk(KERN_ERR "Cannot create psu sysfs\n");
    }
    return 0;  
}

static int dell_s6100_lpc_drv_remove(struct platform_device *pdev)
{

    sysfs_remove_group(&pdev->dev.kobj, &s6100_lpc_attr_grp);

    return 0;
}

static struct platform_driver dell_s6100_lpc_drv = {
    .probe          = dell_s6100_lpc_drv_probe,
    .remove         = __exit_p(dell_s6100_lpc_drv_remove),
    .driver = {
        .name  = DRIVER_NAME,
    },
};

int dell_s6100_lpc_init(void)
{
    spin_lock_init(&smf_slock);

    platform_device_register(&dell_s6100_lpc_dev);
    platform_driver_register(&dell_s6100_lpc_drv);
    return 0;
}

void dell_s6100_lpc_exit(void)
{
    platform_device_unregister(&dell_s6100_lpc_dev);
    platform_driver_unregister(&dell_s6100_lpc_drv);
    return;
}

module_init(dell_s6100_lpc_init);
module_exit(dell_s6100_lpc_exit);

MODULE_AUTHOR("Srideep Devireddy  <srideep_devireddy@dell.com>");
MODULE_DESCRIPTION("dell_s6100_lpc driver");
MODULE_LICENSE("GPL");


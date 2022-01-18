// QE IP Example
// QE Driver (qe_driver.c)
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// QE 0 and 1:
//   GPIO_1[29-28] are used for QE 0 inputs B and A
//   GPIO_1[31-30] are used for QE 1 inputs B and A
// HPS interface:
//   Mapped to offset of 0x1000 in light-weight MM interface aperature

// Load kernel module with insmod qe_driver.ko [param=___]

//-----------------------------------------------------------------------------

#include <linux/kernel.h>     // kstrtouint
#include <linux/module.h>     // MODULE_ macros
#include <linux/init.h>       // __init
#include <linux/kobject.h>    // kobject, kobject_atribute,
                              // kobject_create_and_add, kobject_put
#include <asm/io.h>           // iowrite, ioread, ioremap_nocache (platform specific)
#include "address_map.h"      // overall memory map
#include "qe_regs.h"          // register offsets in QE IP

//-----------------------------------------------------------------------------
// Kernel module information
//-----------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason Losh");
MODULE_DESCRIPTION("QE IP Driver");

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

static unsigned int *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void enableChannel(uint8_t channel)
{
    unsigned int value = ioread32(base + OFS_CONTROL);
    iowrite32(value | (1 << channel), base + OFS_CONTROL);
}

void disableChannel(uint8_t channel)
{
    unsigned int value = ioread32(base + OFS_CONTROL);
    iowrite32(value & ~(1 << channel), base + OFS_CONTROL);
}

bool isChannelEnabled(uint8_t channel)
{
    return (ioread32(base + OFS_CONTROL) >> channel) & 1;
}

void enableChannelSwap(uint8_t channel)
{
    unsigned int value = ioread32(base + OFS_CONTROL);
    iowrite32(value | (4 << channel), base + OFS_CONTROL);
}

void disableChannelSwap(uint8_t channel)
{
    unsigned int value = ioread32(base + OFS_CONTROL);
    iowrite32(value & ~(4 << channel), base + OFS_CONTROL);
}

bool isChannelSwapEnabled(uint8_t channel)
{
    return (ioread32(base + OFS_CONTROL) >> (channel+2)) & 1;
}

void setPosition(uint8_t channel, int32_t position)
{
    iowrite32(position, base + (OFS_POSITION0 + channel * 2));
}

int32_t getPosition(uint8_t channel)
{
    return ioread32(base + (OFS_POSITION0 + channel * 2));
}

void setVelocityTimebase(unsigned int period)
{
    iowrite32(period, base + OFS_PERIOD);
}

int32_t getVelocity(uint8_t channel)
{
    return ioread32(base + (OFS_VELOCITY0 + (channel * 2)));
}

//-----------------------------------------------------------------------------
// Kernel Objects
//-----------------------------------------------------------------------------

// Enable 0
static bool enable0 = 0;
module_param(enable0, bool, S_IRUGO);
MODULE_PARM_DESC(enable0, " Enable encoder 0");

static ssize_t enable0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        enableChannel(0);
        enable0 = true;
    }
    else
        if (strncmp(buffer, "false", count-1) == 0)
        {
            disableChannel(0);
            enable0 = false;
        }
    return count;
}

static ssize_t enable0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    enable0 = isChannelEnabled(0);
    if (enable0)
        strcpy(buffer, "true\n");
    else
        strcpy(buffer, "false\n");
    return strlen(buffer);
}

static struct kobj_attribute enable0Attr = __ATTR(enable0, 0664, enable0Show, enable0Store);

// Swap 0
static bool swap0 = 0;
module_param(swap0, bool, S_IRUGO);
MODULE_PARM_DESC(swap0, " Swap encoder 0 inputs");

static ssize_t swap0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        enableChannelSwap(0);
        swap0 = true;
    }
    else
        if (strncmp(buffer, "false", count-1) == 0)
        {
            disableChannelSwap(0);
            swap0 = false;
        }
    return count;
}

static ssize_t swap0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    swap0 = isChannelSwapEnabled(0);
    if (swap0)
        strcpy(buffer, "true\n");
    else
        strcpy(buffer, "false\n");
    return strlen(buffer);
}

static struct kobj_attribute swap0Attr = __ATTR(swap0, 0664, swap0Show, swap0Store);

// Position 0
static int position0 = 0;
module_param(position0, int, S_IRUGO);
MODULE_PARM_DESC(position0, " Position of encoder 0");

static ssize_t position0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &position0);
    if (result == 0)
        setPosition(0, position0);
    return count;
}

static ssize_t position0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    position0 = getPosition(0);
    return sprintf(buffer, "%d\n", position0);
}

static struct kobj_attribute position0Attr = __ATTR(position0, 0664, position0Show, position0Store);

// Velocity 0
static int velocity0 = 0;
module_param(velocity0, int, S_IRUGO);
MODULE_PARM_DESC(velocity0, " Velocity of encoder 0");

static ssize_t velocity0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    velocity0 = getVelocity(0);
    return sprintf(buffer, "%d\n", velocity0);
}

static struct kobj_attribute velocity0Attr = __ATTR(velocity0, 0444, velocity0Show, NULL);

// Enable 1
static int enable1 = 0;
module_param(enable1, int, S_IRUGO);
MODULE_PARM_DESC(enable1, " Enable encoder 1");

static ssize_t enable1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        enableChannel(1);
        enable1 = true;
    }
    else
        if (strncmp(buffer, "false", count-1) == 0)
        {
            disableChannel(1);
            enable1 = false;
        }
    return count;
}

static ssize_t enable1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    enable1 = isChannelEnabled(1);
    if (enable1)
        strcpy(buffer, "true\n");
    else
        strcpy(buffer, "false\n");
    return strlen(buffer);
}

static struct kobj_attribute enable1Attr = __ATTR(enable1, 0664, enable1Show, enable1Store);

// Swap 1
static int swap1 = 0;
module_param(swap1, int, S_IRUGO);
MODULE_PARM_DESC(swap1, " Swap encoder 1 inputs");

static ssize_t swap1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        enableChannelSwap(1);
        swap1 = true;
    }
    else
        if (strncmp(buffer, "false", count-1) == 0)
        {
            disableChannelSwap(1);
            swap1 = false;
        }
    return count;
}

static ssize_t swap1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    swap1 = isChannelSwapEnabled(1);
    if (swap1)
        strcpy(buffer, "true\n");
    else
        strcpy(buffer, "false\n");
    return strlen(buffer);
}

static struct kobj_attribute swap1Attr = __ATTR(swap1, 0664, swap1Show, swap1Store);

// Position 1
static int position1 = 0;
module_param(position1, int, S_IRUGO);
MODULE_PARM_DESC(position1, " Position of encoder 1");

static ssize_t position1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &position1);
    if (result == 0)
        setPosition(1, position1);
    return count;
}

static ssize_t position1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    position1 = getPosition(1);
    return sprintf(buffer, "%d\n", position1);
}

static struct kobj_attribute position1Attr = __ATTR(position1, 0664, position1Show, position1Store);

// Velocity 1
static int velocity1 = 0;
module_param(velocity1, int, S_IRUGO);
MODULE_PARM_DESC(velocity1, " Velocity of encoder 1");

static ssize_t velocity1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    velocity1 = getVelocity(1);
    return sprintf(buffer, "%d\n", velocity1);
}

static struct kobj_attribute velocity1Attr = __ATTR(velocity1, 0444, velocity1Show, NULL);

// Timebase of velocity measurement
static unsigned int period = 5000000;
module_param(period, uint, S_IRUGO);
MODULE_PARM_DESC(period, " Timebase of velocity measurement (in system clocks)");

static ssize_t periodStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &period);
    if (result == 0)
        setVelocityTimebase(period);
    return count;
}

static ssize_t periodShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%u\n", period);
}

static struct kobj_attribute periodAttr = __ATTR(period, 0664, periodShow, periodStore);

// Attributes

static struct attribute *attrs0[] = {&enable0Attr.attr, &swap0Attr.attr, &position0Attr.attr, &velocity0Attr.attr, &periodAttr.attr, NULL};
static struct attribute *attrs1[] = {&enable1Attr.attr, &swap1Attr.attr, &position1Attr.attr, &velocity1Attr.attr, &periodAttr.attr, NULL};

static struct attribute_group group0 =
{
    .name = "qe0",
    .attrs = attrs0
};

static struct attribute_group group1 =
{
    .name = "qe1",
    .attrs = attrs1
};

static struct kobject *kobj;

//-----------------------------------------------------------------------------
// Initialization and Exit
//-----------------------------------------------------------------------------

static int __init initialize_module(void)
{
    int result;

    printk(KERN_INFO "QE driver: starting\n");

    // Create qe directory under /sys/kernel
    kobj = kobject_create_and_add("qe", kernel_kobj);
    if (!kobj)
    {
        printk(KERN_ALERT "QE driver: failed to create and add kobj\n");
        return -ENOENT;
    }

    // Create qe0 and qe1 groups
    result = sysfs_create_group(kobj, &group0);
    if (result !=0)
        return result;
    result = sysfs_create_group(kobj, &group1);
    if (result !=0)
        return result;

    // Physical to virtual memory map to access gpio registers
    base = (unsigned int*)ioremap_nocache(LW_BRIDGE_BASE + QE_BASE_OFFSET,
                                          QE_SPAN_IN_BYTES);
    if (base == NULL)
        return -ENODEV;

    printk(KERN_INFO "QE driver: initialized\n");

    return 0;
}


static void __exit exit_module(void)
{
    kobject_put(kobj);
    printk(KERN_INFO "QE driver: exit\n");
}

module_init(initialize_module);
module_exit(exit_module);


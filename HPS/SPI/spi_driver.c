// SPI IP
// SPI IP Driver
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board

// Hardware configuration:
// SPI Port:
//   GPIO_0[7,9,11,13,15,17,19] are used as a SPI interface
// HPS interface:
//   Mapped to offset of 8000 in light-weight MM interface aperature

// Load kernel module with insmod spi_driver.ko [param=___]

//=============================================================================

#include <linux/kernel.h>     // kstrtouint
#include <linux/module.h>     // MODULE_ macros
#include <linux/init.h>       // __init
#include <linux/kobject.h>    // kobject, kobject_atribute,
                              // kobject_create_and_add, kobject_put
#include <linux/delay.h>      // delay
#include <asm/io.h>           // iowrite, ioread, ioremap_nocache (platform specific)
#include "../address_map.h"   // overall memory map
#include "spi_regs.h"         // register offsets in SPI IP

//=============================================================================
// Kernel module information
//=============================================================================

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Deborah Jahaj and Nathan Fusselman");
MODULE_DESCRIPTION("SPI IP Driver");

//=============================================================================
// Global variables
//=============================================================================

#define SYSTEM_CLOCK 50000000

static unsigned int *base = NULL;

//=============================================================================
// Subroutines
//=============================================================================

int power(int x, int y)
{
    int temp;
    if(y == 0)
       return 1;
    temp = power(x, y/2);
    if (y%2 == 0)
        return temp*temp;
    else
    {
        if(y > 0)
            return x*temp*temp;
        else
            return (temp*temp)/x;
    }
}
//-----------------------------------------------------------------------------------------------------------------
bool getBRD(uint *brd)
{
    uint raw_brd = ioread32(base + OFS_BRD);
    uint divisor = raw_brd >> 6;
    uint i = 0;
    for (i = 0; i < 6; i++) {
        if (raw_brd & (1 << (5 - i))) {
            divisor += power(2,-i);
        }
    }
    *brd = SYSTEM_CLOCK / divisor;
    return true;
}

bool setBRD(uint brd)
{
    uint brd_value = (SYSTEM_CLOCK / brd) << 6;
    iowrite32(brd_value, base + OFS_BRD);
    return true;
}

//-----------------------------------------------------------------------------------------------------------------
bool getWordSize(uint *size)
{
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    *size = control_reg & 0x1F;
    *size = *size + 1;
    return true;
}

bool setWordSize(uint size)
{
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (size > 32) return false;
    control_reg &= ~0x1F;
    control_reg |= (size - 1) & 0x1F;
    iowrite32(control_reg, base + OFS_CONTROL);
    return true;
}
//-----------------------------------------------------------------------------------------------------------------
bool getDevice(uint *dev)
{
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (*dev > 3) return false;
    *dev = (control_reg >> 13) & 0x3;
    return true;
}

bool setDevice(uint dev)
{
    uint newDev = 0;
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    control_reg &= ~(0x3 << 13);
    control_reg |= ((dev & 0x3) << 13);
    iowrite32(control_reg, base + OFS_CONTROL);
    getDevice(&newDev);
    return dev == newDev;
}
//-----------------------------------------------------------------------------------------------------------------
bool getModeForDevice(uint dev, bool *spo, bool *sph)
{
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    *spo = (control_reg >> (16 + (dev * 2))) & 0x1;
    *sph = (control_reg >> (17 + (dev * 2))) & 0x1;
    iowrite32(control_reg, base + OFS_CONTROL);
    return true;
}

bool setModeForDevice(uint dev, bool spo, bool sph)
{
    bool newSPO, newSPH;
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    if (spo) {
        control_reg |= (1 << (16 + (dev * 2)));
    } else {
        control_reg &= ~(1 << (16 + (dev * 2)));
    }
    if (sph) {
        control_reg |= (1 << (17 + (dev * 2)));
    } else {
        control_reg &= ~(1 << (17 + (dev * 2)));
    }
    iowrite32(control_reg, base + OFS_CONTROL);
    getModeForDevice(dev, &newSPO, &newSPH);
    return spo == newSPO && sph == newSPH;
}
//-----------------------------------------------------------------------------------------------------------------
bool getCSAutoForDevice(uint dev, bool *enable)
{
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    *enable = (control_reg >> (5 + dev)) & 0x1;
    return true;
}

bool setCSAutoForDevice(uint dev, bool enable)
{
    bool newEnable;
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    if (enable) {
        control_reg |= (1 << (5 + dev));
    } else {
        control_reg &= ~(1 << (5 + dev));
    }
    iowrite32(control_reg, base + OFS_CONTROL);
    getCSAutoForDevice(dev, &newEnable);
    return enable == newEnable;
}
//-----------------------------------------------------------------------------------------------------------------
bool getCSEnableForDevice(uint dev, bool *enable)
{
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    *enable = (control_reg >> (9 + dev)) & 0x1;
    return true;
}

bool setCSEnableForDevice(uint dev, bool enable)
{
    bool newEnable;
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    if (enable) {
        control_reg |= (1 << (9 + dev));
    } else {
        control_reg &= ~(1 << (9 + dev));
    }
    iowrite32(control_reg, base + OFS_CONTROL);
    getCSEnableForDevice(dev, &newEnable);
    return enable == newEnable;
}
//-----------------------------------------------------------------------------------------------------------------
bool getRxStatus(bool *empty, bool *full, bool *ovr)
{
    uint32_t status_reg = ioread32(base + OFS_STATUS);
    *ovr = status_reg & ((1 << 0) << (3 * 0));
    *full = status_reg & ((1 << 1) << (3 * 0));
    *empty = status_reg & ((1 << 2) << (3 * 0));
    return true;
}

bool getTxStatus(bool *empty, bool *full, bool *ovr)
{
    uint32_t status_reg = ioread32(base + OFS_STATUS);
    *ovr = status_reg & ((1 << 0) << (3 * 1));
    *full = status_reg & ((1 << 1) << (3 * 1));
    *empty = status_reg & ((1 << 2) << (3 * 1));
    return true;
}
//-----------------------------------------------------------------------------------------------------------------
bool TXdata(uint32_t data)
{
    bool empty, full, ovr;
    uint32_t data_reg = data;
    getTxStatus(&empty, &full, &ovr);
    if (full) return false;
    iowrite32(data_reg, base + OFS_DATA);
    udelay(1);
    return true;
}
//-----------------------------------------------------------------------------------------------------------------
bool RXdata(uint32_t *data)
{
    bool empty, full, ovr;
    uint32_t data_reg = 0;
    getRxStatus(&empty, &full, &ovr);
    data_reg = ioread32(base + OFS_DATA);
    if (empty) return false;
    *data = data_reg;
    udelay(1);
    return true;
}

//=============================================================================
// Kernel Objects Devices0-3
//=============================================================================

// Baud Rate
static unsigned int baud_rate = 0;
module_param(baud_rate, uint, S_IRUGO);
MODULE_PARM_DESC(baud_rate, " Baud Rate");

static ssize_t baud_rateStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &baud_rate);
    if (result == 0 && baud_rate > 0 && baud_rate <= 25000000)
        setBRD(baud_rate);
    return count;
}

static ssize_t baud_rateShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getBRD(&baud_rate);
    return sprintf(buffer, "%d\n", baud_rate);
}

static struct kobj_attribute baud_rateAttr = __ATTR(baud_rate, 0664, baud_rateShow, baud_rateStore);

//-----------------------------------------------------------------------------------------------------------------

// Word Size
static unsigned int word_size = 0;
module_param(word_size, uint, S_IRUGO);
MODULE_PARM_DESC(word_size, " Word Size");

static ssize_t word_sizeStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0 && temp <= 32 )
    {
        word_size = temp;
        setWordSize(word_size);
    }
    return count;
}

static ssize_t word_sizeShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getWordSize(&word_size);
    return sprintf(buffer, "%d\n", word_size);
}

static struct kobj_attribute word_sizeAttr = __ATTR(word_size, 0664, word_sizeShow, word_sizeStore);

//-----------------------------------------------------------------------------------------------------------------

// CS Select
static unsigned int cs_select = 0;
module_param(cs_select, uint, S_IRUGO);
MODULE_PARM_DESC(cs_select, " CS Select");

static ssize_t cs_selectStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0 && temp < 4)
    {
        cs_select = temp;
        setDevice(cs_select);
    }
    return count;
}

static ssize_t cs_selectShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getDevice(&cs_select);
    return sprintf(buffer, "%d\n", cs_select);
}

static struct kobj_attribute cs_selectAttr = __ATTR(cs_select, 0664, cs_selectShow, cs_selectStore);

//================================================================================================================

// MODE0
static unsigned int mode0 = 0;
module_param(mode0, uint, S_IRUGO);
MODULE_PARM_DESC(mode0, " mode0");

static ssize_t mode0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0 && temp < 4)
    {
        mode0 = temp;
        setModeForDevice(0, (bool)(mode0 / 2), (bool)(mode0 % 2));
    }
    return count;
}

static ssize_t mode0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    bool spo, sph;
    getModeForDevice(0, &spo, &sph);
    mode0 = (spo << 1) | sph;
    return sprintf(buffer, "%d\n", mode0);
}

static struct kobj_attribute mode0Attr = __ATTR(mode0, 0664, mode0Show, mode0Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_AUTO0
static bool cs_auto0 = 0;
module_param(cs_auto0, bool, S_IRUGO);
MODULE_PARM_DESC(cs_auto0, " cs_auto0");

static ssize_t cs_auto0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_auto0 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_auto0 = false;
    }
    setCSAutoForDevice(0, cs_auto0);
    return count;
}

static ssize_t cs_auto0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSAutoForDevice(0, &cs_auto0);
    return sprintf(buffer, "%s\n", cs_auto0 ? "true" : "false");
}

static struct kobj_attribute cs_auto0Attr = __ATTR(cs_auto0, 0664, cs_auto0Show, cs_auto0Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_ENABLE0
static bool cs_enable0 = 0;
module_param(cs_enable0, bool, S_IRUGO);
MODULE_PARM_DESC(cs_enable0, " cs_enable0");

static ssize_t cs_enable0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_enable0 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_enable0 = false;
    }
    setCSEnableForDevice(0, cs_enable0);
    return count;
}

static ssize_t cs_enable0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSEnableForDevice(0, &cs_enable0);
    return sprintf(buffer, "%s\n", cs_enable0 ? "true" : "false");
}

static struct kobj_attribute cs_enable0Attr = __ATTR(cs_enable0, 0664, cs_enable0Show, cs_enable0Store);

//================================================================================================================

// MODE1
static unsigned int mode1 = 0;
module_param(mode1, uint, S_IRUGO);
MODULE_PARM_DESC(mode1, " mode1");

static ssize_t mode1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0 && temp < 4)
    {
        mode1 = temp;
        setModeForDevice(1, (bool)(mode1 / 2), (bool)(mode1 % 2));
    }
    return count;
}

static ssize_t mode1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    bool spo, sph;
    getModeForDevice(1, &spo, &sph);
    mode1 = (spo << 1) | sph;
    return sprintf(buffer, "%d\n", mode1);
}

static struct kobj_attribute mode1Attr = __ATTR(mode1, 0664, mode1Show, mode1Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_AUTO1
static bool cs_auto1 = 0;
module_param(cs_auto1, bool, S_IRUGO);
MODULE_PARM_DESC(cs_auto1, " cs_auto1");

static ssize_t cs_auto1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_auto1 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_auto1 = false;
    }
    setCSAutoForDevice(1, cs_auto1);
    return count;
}

static ssize_t cs_auto1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSAutoForDevice(1, &cs_auto1);
    return sprintf(buffer, "%s\n", cs_auto1 ? "true" : "false");
}

static struct kobj_attribute cs_auto1Attr = __ATTR(cs_auto1, 0664, cs_auto1Show, cs_auto1Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_ENABLE1
static bool cs_enable1 = 0;
module_param(cs_enable1, bool, S_IRUGO);
MODULE_PARM_DESC(cs_enable1, " cs_enable1");

static ssize_t cs_enable1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_enable1 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_enable1 = false;
    }
    setCSEnableForDevice(1, cs_enable1);
    return count;
}

static ssize_t cs_enable1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSEnableForDevice(1, &cs_enable1);
    return sprintf(buffer, "%s\n", cs_enable1 ? "true" : "false");
}

static struct kobj_attribute cs_enable1Attr = __ATTR(cs_enable1, 0664, cs_enable1Show, cs_enable1Store);

//================================================================================================================

// MODE2
static unsigned int mode2 = 0;
module_param(mode2, uint, S_IRUGO);
MODULE_PARM_DESC(mode2, " mode2");

static ssize_t mode2Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0 && temp < 4)
    {
        mode0 = temp;
        setModeForDevice(2, (bool)(mode2 / 2), (bool)(mode2 % 2));
    }
    return count;
}

static ssize_t mode2Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    bool spo, sph;
    getModeForDevice(2, &spo, &sph);
    mode2 = (spo << 1) | sph;
    return sprintf(buffer, "%d\n", mode2);
}

static struct kobj_attribute mode2Attr = __ATTR(mode2, 0664, mode2Show, mode2Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_AUTO2
static bool cs_auto2 = 0;
module_param(cs_auto2, bool, S_IRUGO);
MODULE_PARM_DESC(cs_auto2, " cs_auto2");

static ssize_t cs_auto2Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_auto2 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_auto2 = false;
    }
    setCSAutoForDevice(2, cs_auto2);
    return count;
}

static ssize_t cs_auto2Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSAutoForDevice(2, &cs_auto2);
    return sprintf(buffer, "%s\n", cs_auto2 ? "true" : "false");
}

static struct kobj_attribute cs_auto2Attr = __ATTR(cs_auto2, 0664, cs_auto2Show, cs_auto2Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_ENABLE2
static bool cs_enable2 = 0;
module_param(cs_enable2, bool, S_IRUGO);
MODULE_PARM_DESC(cs_enable2, " cs_enable2");

static ssize_t cs_enable2Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_enable2 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_enable2 = false;
    }
    setCSEnableForDevice(2, cs_enable2);
    return count;
}

static ssize_t cs_enable2Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSEnableForDevice(2, &cs_enable2);
    return sprintf(buffer, "%s\n", cs_enable2 ? "true" : "false");
}

static struct kobj_attribute cs_enable2Attr = __ATTR(cs_enable2, 0664, cs_enable2Show, cs_enable2Store);

//================================================================================================================

// MODE3
static unsigned int mode3 = 0;
module_param(mode3, uint, S_IRUGO);
MODULE_PARM_DESC(mode3, " mode3");

static ssize_t mode3Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0 && temp < 4)
    {
        mode3 = temp;
        setModeForDevice(3, (bool)(mode3 / 2), (bool)(mode3 % 2));
    }
    return count;
}

static ssize_t mode3Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    bool spo, sph;
    getModeForDevice(3, &spo, &sph);
    mode3 = (spo << 1) | sph;
    return sprintf(buffer, "%d\n", mode3);
}

static struct kobj_attribute mode3Attr = __ATTR(mode3, 0664, mode3Show, mode3Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_AUTO3
static bool cs_auto3 = 0;
module_param(cs_auto3, bool, S_IRUGO);
MODULE_PARM_DESC(cs_auto3, " cs_auto3");

static ssize_t cs_auto3Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_auto3 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_auto3 = false;
    }
    setCSAutoForDevice(3, cs_auto3);
    return count;
}

static ssize_t cs_auto3Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSAutoForDevice(3, &cs_auto3);
    return sprintf(buffer, "%s\n", cs_auto3 ? "true" : "false");
}

static struct kobj_attribute cs_auto3Attr = __ATTR(cs_auto3, 0664, cs_auto3Show, cs_auto3Store);

//-----------------------------------------------------------------------------------------------------------------

// CS_ENABLE3
static bool cs_enable3 = 0;
module_param(cs_enable3, bool, S_IRUGO);
MODULE_PARM_DESC(cs_enable3, " cs_enable3");

static ssize_t cs_enable3Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "true", count-1) == 0)
    {
        cs_enable3 = true;
    }
    else
    if (strncmp(buffer, "false", count-1) == 0)
    {
        cs_enable3 = false;
    }
    setCSEnableForDevice(3, cs_enable3);
    return count;
}

static ssize_t cs_enable3Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getCSEnableForDevice(3, &cs_enable3);
    return sprintf(buffer, "%s\n", cs_enable3 ? "true" : "false");
}

static struct kobj_attribute cs_enable3Attr = __ATTR(cs_enable3, 0664, cs_enable3Show, cs_enable3Store);

//================================================================================================================

// TX Data
static unsigned int tx_data = 0;
module_param(tx_data, uint, S_IRUGO);
MODULE_PARM_DESC(tx_data, " TX Data");

static ssize_t tx_dataStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    unsigned int temp;
    int result = kstrtouint(buffer, 0, &temp);
    if (result == 0)
    {
        tx_data = temp;
        TXdata(tx_data);
    }
    return count;
}

static struct kobj_attribute tx_dataAttr = __ATTR(tx_data, 0220, NULL, tx_dataStore);

//-----------------------------------------------------------------------------------------------------------------

// RX Data
static unsigned int rx_data = 0;
module_param(rx_data, uint, S_IRUGO);
MODULE_PARM_DESC(rx_data, " RX Data");

static ssize_t rx_dataShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    bool result = RXdata(&rx_data);
    if (!result)
        rx_data = -1;
    return sprintf(buffer, "0x%08X\n", rx_data);
}

static struct kobj_attribute rx_dataAttr = __ATTR(rx_data, 0444, rx_dataShow, NULL);

//================================================================================================================

// Attributes for devices0-3
static struct attribute *attrs0[] = {&mode0Attr.attr, &cs_auto0Attr.attr, &cs_enable0Attr.attr, NULL};
static struct attribute *attrs1[] = {&mode1Attr.attr, &cs_auto1Attr.attr, &cs_enable1Attr.attr, NULL};
static struct attribute *attrs2[] = {&mode2Attr.attr, &cs_auto2Attr.attr, &cs_enable2Attr.attr, NULL};
static struct attribute *attrs3[] = {&mode3Attr.attr, &cs_auto3Attr.attr, &cs_enable3Attr.attr, NULL};

static struct attribute_group device0 =
{
    .name = "device0",
    .attrs = attrs0
};

static struct attribute_group device1 =
{
    .name = "device1",
    .attrs = attrs1
};

static struct attribute_group device2 =
{
    .name = "device2",
    .attrs = attrs2
};

static struct attribute_group device3 =
{
    .name = "device3",
    .attrs = attrs3
};

static struct kobject *kobj;

//=============================================================================
// Initialization and Exit
//=============================================================================

static int __init initialize_module(void)
{
    int result;

    printk(KERN_INFO "SPI driver: starting\n");

    // Create qe directory under /sys/kernel
    kobj = kobject_create_and_add("spi", kernel_kobj);
    if (!kobj)
    {
        printk(KERN_ALERT "SPI driver: failed to create and add kobj\n");
        return -ENOENT;
    }

    result = sysfs_create_file(kobj, &baud_rateAttr.attr);
    if (result !=0)
        return result;
    result = sysfs_create_file(kobj, &word_sizeAttr.attr);
    if (result !=0)
        return result;
    result = sysfs_create_file(kobj, &cs_selectAttr.attr);
    if (result !=0)
        return result;
    
    // Create device0-3 groups
    result = sysfs_create_group(kobj, &device0);
    if (result !=0)
        return result;
    result = sysfs_create_group(kobj, &device1);
    if (result !=0)
        return result;
    result = sysfs_create_group(kobj, &device2);
    if (result !=0)
        return result;
    result = sysfs_create_group(kobj, &device3);
    if (result !=0)
        return result;

    result = sysfs_create_file(kobj, &tx_dataAttr.attr);
    if (result !=0)
        return result;
    result = sysfs_create_file(kobj, &rx_dataAttr.attr);
    if (result !=0)
        return result;


    // Physical to virtual memory map to access gpio registers
    base = (unsigned int*)ioremap_nocache(LW_BRIDGE_BASE + SPI_BASE_OFFSET,
                                          SPAN_IN_BYTES);
    if (base == NULL)
        return -ENODEV;

    printk(KERN_INFO "SPI driver: initialized\n");

    return 0;
}


static void __exit exit_module(void)
{
    kobject_put(kobj);
    printk(KERN_INFO "SPI driver: exit\n");
}

module_init(initialize_module);
module_exit(exit_module);


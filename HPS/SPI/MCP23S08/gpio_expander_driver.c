// SPI GPIO Expander 
// SPI GPIO Expander Driver
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// HPS interface:
//   Mapped to offset of 0 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

// Load kernel module with insmod qe_driver.ko [param=___]

//=============================================================================

#include <linux/kernel.h>           // kstrtouint
#include <linux/module.h>           // MODULE_ macros
#include <linux/init.h>             // __init
#include <linux/kobject.h>          // kobject, kobject_atribute,
                                    // kobject_create_and_add, kobject_put
#include <linux/delay.h>            // delay
#include <asm/io.h>                 // iowrite, ioread, ioremap_nocache (platform specific)
#include "../../address_map.h"      // overall memory map
#include "gpio_expander_regs.h"     // register offsets
#include "../spi_regs.h"            // register offsets

//=============================================================================
// Kernel module information
//=============================================================================

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Deborah Jahaj and Nathan Fusselman");
MODULE_DESCRIPTION("Gpio Expander Driver");

//=============================================================================
// Global variables
//=============================================================================

#define SYSTEM_CLOCK 50000000
#define BAUD_RATE 5000000
#define WORD_SIZE 24
#define DEVICE 0
#define MODE_SPO false
#define MODE_SPH false
#define CS_AUTO true

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
    *spo = (control_reg >> (17 + (dev * 2))) & 0x1;
    *sph = (control_reg >> (16 + (dev * 2))) & 0x1;
    iowrite32(control_reg, base + OFS_CONTROL);
    return true;
}

bool setModeForDevice(uint dev, bool spo, bool sph)
{
    bool newSPO, newSPH;
    uint32_t control_reg = ioread32(base + OFS_CONTROL);
    if (dev > 3) return false;
    if (spo) {
        control_reg |= (1 << (17 + (dev * 2)));
    } else {
        control_reg &= ~(1 << (17 + (dev * 2)));
    }
    if (sph) {
        control_reg |= (1 << (16 + (dev * 2)));
    } else {
        control_reg &= ~(1 << (16 + (dev * 2)));
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
    udelay(10);
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
    udelay(10);
    return true;
}

//================================================================================================================

void setPinDir(uint pin, bool input)
{
    uint data = 0;
    if (pin >= 8) return;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    TXdata(READ | IODIR | BLANK);
    RXdata(&data);                           // Read Current Value
    if(input)
        data |= (1 << pin);
    else
        data &= ~(1 << pin);
    TXdata(WRITE | IODIR | (data & 0xFF));    // Set New Value
    RXdata(&data);
}

void getPinDir(uint pin, bool *state)
{
    uint data = 0;
    if (pin >= 8) return;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    TXdata(READ | IODIR | BLANK);
    RXdata(&data);                           // Read Current Value
    *state = (data >> pin) & 0x1;
}

//-----------------------------------------------------------------------------------------------------------------

void setPinPullup(uint pin, bool enable)
{
    uint data = 0;
    if (pin >= 8) return;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    TXdata(READ | GPPU | BLANK);
    RXdata(&data);                           // Read Current Value
    if(enable)
        data |= (1 << pin);
    else
        data &= ~(1 << pin);
    TXdata(WRITE | GPPU | (data & 0xFF));    // Set New Value
    RXdata(&data);
}

void getPinPullup(uint pin, bool *state)
{
    uint data = 0;
    if (pin >= 8) return;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    TXdata(READ | GPPU | BLANK);
    RXdata(&data);                           // Read Current Value
    *state = (data >> pin) & 0x1;
}

//-----------------------------------------------------------------------------------------------------------------

void setPinData(uint pin, bool value)
{
    uint data = 0;
    if (pin >= 8) return;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    TXdata(READ | GPIO | BLANK);
    RXdata(&data);                           // Read Current Value
    printk(KERN_INFO "Current Data: 0x%08X\n", data);
    if(value)
        data |= (1 << pin);
    else
        data &= ~(1 << pin);
    printk(KERN_INFO "Current Data: 0x%08X\n", data);
    TXdata(WRITE | GPIO | (data & 0xFF));    // Set New Value
    RXdata(&data);
}

void getPinData(uint pin, bool *state)
{
    uint data = 0;
    if (pin >= 8) return;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    TXdata(READ | GPIO | BLANK);
    RXdata(&data);                           // Read Current Value
    *state = (data >> pin) & 0x1;
}

//=============================================================================
// Kernel Objects
//=============================================================================

// PIN0
// Dir 0
static bool dir0 = 0;
module_param(dir0, bool, S_IRUGO);
MODULE_PARM_DESC(dir0, "Direction of Port Pin0");

static ssize_t dir0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir0 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir0 = false;
    }
    setPinDir(0, dir0);
    return count;
}

static ssize_t dir0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(0, &dir0);
    return sprintf(buffer, "%s\n", dir0 ? "input" : "output");
}

static struct kobj_attribute dir0Attr = __ATTR(dir0, 0664, dir0Show, dir0Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 0
static bool pullup0 = 0;
module_param(pullup0, bool, S_IRUGO);
MODULE_PARM_DESC(pullup0, " Pull Up Pin0");

static ssize_t pullup0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup0 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup0 = false;
    }
    setPinPullup(0, pullup0);
    return count;
}

static ssize_t pullup0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(0, &pullup0);
    return sprintf(buffer, "%s\n", pullup0 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup0Attr = __ATTR(pullup0, 0664, pullup0Show, pullup0Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 0
static bool data0 = 0;
module_param(data0, bool, S_IRUGO);
MODULE_PARM_DESC(data0, " PData on Pin0");

static ssize_t data0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data0 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data0 = false;
    }
    setPinData(0, data0);
    return count;
}

static ssize_t data0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(0, &data0);
    return sprintf(buffer, "%s\n", data0 ? "on" : "off");
}

static struct kobj_attribute data0Attr = __ATTR(data0, 0664, data0Show, data0Store);

//================================================================================================================
// PIN1
// Dir 1
static bool dir1 = 0;
module_param(dir1, bool, S_IRUGO);
MODULE_PARM_DESC(dir1, " Direction of Port Pin1");

static ssize_t dir1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir1 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir1 = false;
    }
    setPinDir(1, dir1);
    return count;
}

static ssize_t dir1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(1, &dir1);
    return sprintf(buffer, "%s\n", dir1 ? "input" : "output");
}

static struct kobj_attribute dir1Attr = __ATTR(dir1, 0664, dir1Show, dir1Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 1
static bool pullup1 = 0;
module_param(pullup1, bool, S_IRUGO);
MODULE_PARM_DESC(pullup1, " Pull Up Pin1");

static ssize_t pullup1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup1 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup1 = false;
    }
    setPinPullup(1, pullup1);
    return count;
}

static ssize_t pullup1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(1, &pullup1);
    return sprintf(buffer, "%s\n", pullup1 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup1Attr = __ATTR(pullup1, 0664, pullup1Show, pullup1Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 1
static bool data1 = 0;
module_param(data1, bool, S_IRUGO);
MODULE_PARM_DESC(data1, " Data on Pin1");

static ssize_t data1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data1 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data1 = false;
    }
    setPinData(1, data1);
    return count;
}

static ssize_t data1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(1, &data1);
    return sprintf(buffer, "%s\n", data1 ? "on" : "off");
}

static struct kobj_attribute data1Attr = __ATTR(data1, 0664, data1Show, data1Store);

//================================================================================================================
// PIN2
// Dir 2
static bool dir2 = 0;
module_param(dir2, bool, S_IRUGO);
MODULE_PARM_DESC(dir2, " Direction of Port Pin2");

static ssize_t dir2Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir2 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir2 = false;
    }
    setPinDir(2, dir2);
    return count;
}

static ssize_t dir2Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(2, &dir2);
    return sprintf(buffer, "%s\n", dir2 ? "input" : "output");
}

static struct kobj_attribute dir2Attr = __ATTR(dir2, 0664, dir2Show, dir2Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 2
static bool pullup2 = 0;
module_param(pullup2, bool, S_IRUGO);
MODULE_PARM_DESC(pullup2, " Pull Up Pin2");

static ssize_t pullup2Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup2 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup2 = false;
    }
    setPinPullup(2, pullup2);
    return count;
}

static ssize_t pullup2Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(2, &pullup2);
    return sprintf(buffer, "%s\n", pullup2 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup2Attr = __ATTR(pullup2, 0664, pullup2Show, pullup2Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 2
static bool data2 = 0;
module_param(data2, bool, S_IRUGO);
MODULE_PARM_DESC(data2, " Data on Pin2");

static ssize_t data2Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data2 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data2 = false;
    }
    setPinData(2, data2);
    return count;
}

static ssize_t data2Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(2, &data2);
    return sprintf(buffer, "%s\n", data2 ? "on" : "off");
}

static struct kobj_attribute data2Attr = __ATTR(data2, 0664, data2Show, data2Store);

//================================================================================================================
// PIN3
// Dir 3
static bool dir3 = 0;
module_param(dir3, bool, S_IRUGO);
MODULE_PARM_DESC(dir3, " Direction of Port Pin3");

static ssize_t dir3Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir3 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir3 = false;
    }
    setPinDir(3, dir3);
    return count;
}

static ssize_t dir3Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(3, &dir3);
    return sprintf(buffer, "%s\n", dir3 ? "input" : "output");
}

static struct kobj_attribute dir3Attr = __ATTR(dir3, 0664, dir3Show, dir3Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 3
static bool pullup3 = 0;
module_param(pullup3, bool, S_IRUGO);
MODULE_PARM_DESC(pullup3, " Pull Up Pin0");

static ssize_t pullup3Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup3 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup3 = false;
    }
    setPinPullup(3, pullup3);
    return count;
}

static ssize_t pullup3Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(3, &pullup3);
    return sprintf(buffer, "%s\n", pullup3 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup3Attr = __ATTR(pullup3, 0664, pullup3Show, pullup3Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 3
static bool data3 = 0;
module_param(data3, bool, S_IRUGO);
MODULE_PARM_DESC(data3, " Data on Pin3");

static ssize_t data3Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data3 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data3 = false;
    }
    setPinData(3, data3);
    return count;
}

static ssize_t data3Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(3, &data3);
    return sprintf(buffer, "%s\n", data3 ? "on" : "off");
}

static struct kobj_attribute data3Attr = __ATTR(data3, 0664, data3Show, data3Store);

//================================================================================================================
// PIN4
// Dir 4
static bool dir4 = 0;
module_param(dir4, bool, S_IRUGO);
MODULE_PARM_DESC(dir4, " Direction of Port Pin4");

static ssize_t dir4Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir4 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir4 = false;
    }
    setPinDir(4, dir4);
    return count;
}

static ssize_t dir4Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(4, &dir4);
    return sprintf(buffer, "%s\n", dir4 ? "input" : "output");
}

static struct kobj_attribute dir4Attr = __ATTR(dir4, 0664, dir4Show, dir4Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 4
static bool pullup4 = 0;
module_param(pullup4, bool, S_IRUGO);
MODULE_PARM_DESC(pullup4, " Pull Up Pin4");

static ssize_t pullup4Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup4 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup4 = false;
    }
    setPinPullup(4, pullup4);
    return count;
}

static ssize_t pullup4Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(4, &pullup4);
    return sprintf(buffer, "%s\n", pullup4 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup4Attr = __ATTR(pullup4, 0664, pullup4Show, pullup4Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 4
static bool data4 = 0;
module_param(data4, bool, S_IRUGO);
MODULE_PARM_DESC(data4, "Data on Pin4");

static ssize_t data4Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data4 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data4 = false;
    }
    setPinData(4, data4);
    return count;
}

static ssize_t data4Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(4, &data4);
    return sprintf(buffer, "%s\n", data4 ? "on" : "off");
}

static struct kobj_attribute data4Attr = __ATTR(data4, 0664, data4Show, data4Store);

//================================================================================================================
// PIN5
// Dir 5
static bool dir5 = 0;
module_param(dir5, bool, S_IRUGO);
MODULE_PARM_DESC(dir5, "Direction of Port Pin5");

static ssize_t dir5Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir5 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir5 = false;
    }
    setPinDir(5, dir5);
    return count;
}

static ssize_t dir5Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(5, &dir5);
    return sprintf(buffer, "%s\n", dir5 ? "input" : "output");
}

static struct kobj_attribute dir5Attr = __ATTR(dir5, 0664, dir5Show, dir5Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 5
static bool pullup5 = 0;
module_param(pullup5, bool, S_IRUGO);
MODULE_PARM_DESC(pullup5, "Pull Up Pin5");

static ssize_t pullup5Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup5 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup5 = false;
    }
    setPinPullup(5, pullup5);
    return count;
}

static ssize_t pullup5Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(5, &pullup5);
    return sprintf(buffer, "%s\n", pullup5 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup5Attr = __ATTR(pullup5, 0664, pullup5Show, pullup5Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 5
static bool data5 = 0;
module_param(data5, bool, S_IRUGO);
MODULE_PARM_DESC(data5, "Data on Pin5");

static ssize_t data5Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data5 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data5 = false;
    }
    setPinData(5, data5);
    return count;
}

static ssize_t data5Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(5, &data5);
    return sprintf(buffer, "%s\n", data5 ? "on" : "off");
}

static struct kobj_attribute data5Attr = __ATTR(data5, 0664, data5Show, data5Store);

//================================================================================================================
// PIN6
// Dir 6
static bool dir6 = 0;
module_param(dir6, bool, S_IRUGO);
MODULE_PARM_DESC(dir6, "Direction of Port Pin6");

static ssize_t dir6Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir6 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir6 = false;
    }
    setPinDir(6, dir6);
    return count;
}

static ssize_t dir6Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(6, &dir6);
    return sprintf(buffer, "%s\n", dir6 ? "input" : "output");
}

static struct kobj_attribute dir6Attr = __ATTR(dir6, 0664, dir6Show, dir6Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 6
static bool pullup6 = 0;
module_param(pullup6, bool, S_IRUGO);
MODULE_PARM_DESC(pullup6, "Pull Up Pin0");

static ssize_t pullup6Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup6 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup6 = false;
    }
    setPinPullup(6, pullup6);
    return count;
}

static ssize_t pullup6Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(6, &pullup6);
    return sprintf(buffer, "%s\n", pullup6 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup6Attr = __ATTR(pullup6, 0664, pullup6Show, pullup6Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 6
static bool data6 = 0;
module_param(data6, bool, S_IRUGO);
MODULE_PARM_DESC(data6, "Data on Pin6");

static ssize_t data6Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data6 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data6 = false;
    }
    setPinData(6, data6);
    return count;
}

static ssize_t data6Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(6, &data6);
    return sprintf(buffer, "%s\n", data6 ? "on" : "off");
}

static struct kobj_attribute data6Attr = __ATTR(data6, 0664, data6Show, data6Store);

//================================================================================================================
// PIN7
// Dir 7
static bool dir7 = 0;
module_param(dir7, bool, S_IRUGO);
MODULE_PARM_DESC(dir7, "Direction of Port Pin7");

static ssize_t dir7Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "input", count-1) == 0)
    {
        dir7 = true;
    }
    else
    if (strncmp(buffer, "output", count-1) == 0)
    {
        dir7 = false;
    }
    setPinDir(7, dir7);
    return count;
}

static ssize_t dir7Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinDir(7, &dir7);
    return sprintf(buffer, "%s\n", dir7 ? "input" : "output");
}

static struct kobj_attribute dir7Attr = __ATTR(dir7, 0664, dir7Show, dir7Store);
//-----------------------------------------------------------------------------------------------------------------
// PullUp 7
static bool pullup7 = 0;
module_param(pullup7, bool, S_IRUGO);
MODULE_PARM_DESC(pullup7, "Pull Up Pin7");

static ssize_t pullup7Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "enable", count-1) == 0)
    {
        pullup7 = true;
    }
    else
    if (strncmp(buffer, "disable", count-1) == 0)
    {
        pullup7 = false;
    }
    setPinPullup(7, pullup7);
    return count;
}

static ssize_t pullup7Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinPullup(7, &pullup7);
    return sprintf(buffer, "%s\n", pullup7 ? "enabled" : "disabled");
}

static struct kobj_attribute pullup7Attr = __ATTR(pullup7, 0664, pullup7Show, pullup7Store);
//-----------------------------------------------------------------------------------------------------------------
// Data 7
static bool data7 = 0;
module_param(data7, bool, S_IRUGO);
MODULE_PARM_DESC(data7, " PData on Pin0");

static ssize_t data7Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", count-1) == 0)
    {
        data7 = true;
    }
    else
    if (strncmp(buffer, "off", count-1) == 0)
    {
        data7 = false;
    }
    setPinData(7, data7);
    return count;
}

static ssize_t data7Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    getPinData(7, &data7);
    return sprintf(buffer, "%s\n", data7 ? "on" : "off");
}

static struct kobj_attribute data7Attr = __ATTR(data7, 0664, data7Show, data7Store);

//================================================================================================================

// Attributes
static struct attribute *attrs0[] = {&dir0Attr.attr, &pullup0Attr.attr, &data0Attr.attr, NULL};
static struct attribute *attrs1[] = {&dir1Attr.attr, &pullup1Attr.attr, &data1Attr.attr, NULL};
static struct attribute *attrs2[] = {&dir2Attr.attr, &pullup2Attr.attr, &data2Attr.attr, NULL};
static struct attribute *attrs3[] = {&dir3Attr.attr, &pullup3Attr.attr, &data3Attr.attr, NULL};
static struct attribute *attrs4[] = {&dir4Attr.attr, &pullup4Attr.attr, &data4Attr.attr, NULL};
static struct attribute *attrs5[] = {&dir5Attr.attr, &pullup5Attr.attr, &data5Attr.attr, NULL};
static struct attribute *attrs6[] = {&dir6Attr.attr, &pullup6Attr.attr, &data6Attr.attr, NULL};
static struct attribute *attrs7[] = {&dir7Attr.attr, &pullup7Attr.attr, &data7Attr.attr, NULL};

static struct attribute_group group0 =
{
    .name = "pin0",
    .attrs = attrs0
};
static struct attribute_group group1 =
{
    .name = "pin1",
    .attrs = attrs1
};
static struct attribute_group group2 =
{
    .name = "pin2",
    .attrs = attrs2
};
static struct attribute_group group3 =
{
    .name = "pin3",
    .attrs = attrs3
};
static struct attribute_group group4 =
{
    .name = "pin4",
    .attrs = attrs4
};
static struct attribute_group group5 =
{
    .name = "pin5",
    .attrs = attrs5
};
static struct attribute_group group6 =
{
    .name = "pin6",
    .attrs = attrs6
};
static struct attribute_group group7 =
{
    .name = "pin7",
    .attrs = attrs7
};

static struct kobject *kobj;

//=============================================================================
// Initialization and Exit
//=============================================================================

static int __init initialize_module(void)
{
    int result;



    printk(KERN_INFO "SPI Expander driver: starting\n");

    // Create qe directory under /sys/kernel
    kobj = kobject_create_and_add("spi_expander", kernel_kobj);
    if (!kobj)
    {
        printk(KERN_ALERT "SPI Expaander driver: failed to create and add kobj\n");
        return -ENOENT;
    }

    // Create pin0-7 groups
    result = sysfs_create_group(kobj, &group0);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group1);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group2);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group3);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group4);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group5);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group6);
    if (result !=0)
        return result;

    result = sysfs_create_group(kobj, &group7);
    if (result !=0)
        return result;

        // Physical to virtual memory map to access gpio registers
    base = (unsigned int*)ioremap_nocache(LW_BRIDGE_BASE + SPI_BASE_OFFSET, SPAN_IN_BYTES);
    if (base == NULL)
        return -ENODEV;

    setBRD(BAUD_RATE);
    setWordSize(WORD_SIZE);
    setDevice(DEVICE);
    setModeForDevice(DEVICE, MODE_SPO, MODE_SPH);
    setCSAutoForDevice(DEVICE, CS_AUTO);

    printk(KERN_INFO "SPI Expander driver: initialized\n");

    return 0;
}


static void __exit exit_module(void)
{
    kobject_put(kobj);
    printk(KERN_INFO "SPI Expander driver: exit\n");
}

module_init(initialize_module);
module_exit(exit_module);


// GPIO IP Example
// GPIO IP ISR Default Handler (gpio_isr.c)
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// HPS interface:
//   Mapped to offset of 0 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

//-----------------------------------------------------------------------------

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <asm/io.h>           // iowrite, ioread (platform specific)
#include "address_map.h"
#include "gpio_regs.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint32_t *base = NULL;

//-----------------------------------------------------------------------------
// Kernel module information
//-----------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason Losh");
MODULE_DESCRIPTION("GPIO IP Interrupt Handler");

//-----------------------------------------------------------------------------
// ISR
//-----------------------------------------------------------------------------

irq_handler_t isr(int irq, void *dev_id, struct pt_regs *regs)
{
    uint32_t value;
    printk(KERN_INFO "GPIO IP IRQ (IRQ80) occurred\n");

    // Disable all interrupts
    iowrite32(0x00000000, base + OFS_INT_ENABLE);

    // Display active interrupts
    value = ioread32(base + OFS_INT_STATUS_CLEAR);
    printk("flags: %08x\n", value);

    // Clear interrupt flags
    iowrite32(0xFFFFFFFF, base + OFS_INT_STATUS_CLEAR);

    // Display active interrupts
    value = ioread32(base + OFS_INT_STATUS_CLEAR);
    printk("flags: %08x\n", value);

    return (irq_handler_t)IRQ_HANDLED;
}

//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------

static int __init initialize_module(void)
{
    int result;

    // Physical to virtual memory map to access gpio registers
    base = (uint32_t*)ioremap_nocache(LW_BRIDGE_BASE + GPIO_BASE_OFFSET,
                                      SPAN_IN_BYTES);

    // Register ISR
    result = request_irq(GPIO_IRQ, (irq_handler_t)isr, IRQF_SHARED,
                         "GPIO IP", (irq_handler_t)isr);
    return result;
}

static void __exit exit_module(void)
{
    free_irq(GPIO_IRQ, (irq_handler_t)isr);
}

module_init(initialize_module);
module_exit(exit_module);

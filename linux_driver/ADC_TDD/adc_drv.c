/*
* AST 2500 ADC Driver
* Portions Copyright WangKai .
*
*  https://blog.csdn.net/kao2406
*  
*  用TDD的思路重构ADC驱动.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public License,
* version 2, as published by the Free Software Foundation.
*
* This program is distributed in the hope it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "inc/ioaccess.h"
#include "inc/adc_define.h"

#define ENABLE_TDD           1

static void *__iomem adc_reg_base = NULL;

// product codes
#define read_register(address)	readl(address)

static int adc_get_chan_status(int chan_no)
{
    return 0;
}

static int adc_get_clock_divisor(void)
{
    return 0x40;
}

static void adc_set_clock(int divisor)
{
#if 0
    u32 regv = readl(adc_reg_base + ADC_CLOCK_CONTROL_REGISTER);

    regv &= ~ADC_DIVISOR_OF_ADC_CLOCK; // clear [9:0]
    regv |= divisor;
    
    writel(regv, adc_reg_base + ADC_CLOCK_CONTROL_REGISTER);
#else
    writel(divisor, adc_reg_base + ADC_CLOCK_CONTROL_REGISTER);
#endif
}

static int adc_ioctl (struct file *filp,
                            unsigned int cmd,
                            unsigned long arg) 
{
    switch (cmd)
    {
        case IOCTL_SET_ADC_CLOCK:
        break;
        
    }
    return 0;
}

// tdd test codes
#if ENABLE_TDD > 0
#include "test_template/test_template.h"

void test_setup(void)
{
    adc_reg_base = kmalloc(4096, GFP_KERNEL);
}

void test_teardown(void)
{
}

void test_adc_mod_init__all_chan_disable(void)
{
    TEST_ASSERT_EQUAL_INT(0, adc_get_chan_status(0));
    TEST_ASSERT_EQUAL_INT(0, adc_get_chan_status(1));
}

void test_adc_mod_init__clock_divisor_0x40(void)
{
    TEST_ASSERT_EQUAL_INT(0x40, adc_get_clock_divisor());
}

void test_adc_mod_init__reg_chardev(void)
{
    // nothing do to
}

void test_adc_mod_init__ioctl_set_clock(void)
{
    const int set_divisor = 0xa5;
    int result_divisor = 0;

    IO_ACCESS_DATA io_data;
    io_data.Data = set_divisor;

    adc_ioctl(NULL, IOCTL_SET_ADC_CLOCK, &io_data);

    result_divisor = adc_get_clock_divisor();

    TEST_ASSERT_EQUAL_INT(result_divisor, set_divisor);
}
#endif

int __init adc_mod_init(void)
{
    
#if ENABLE_TDD > 0
    printk(KERN_EMERG "\n*********************\n");
    test_setup();
    test_adc_mod_init__all_chan_disable();
    test_adc_mod_init__reg_chardev();
    test_adc_mod_init__ioctl_set_clock();
    printk(KERN_EMERG "\n*********************\n");
#endif
    return 0;
}

void __exit adc_mod_cleanup (void)
{

}

MODULE_LICENSE ("GPL");
MODULE_AUTHOR("WangKai -- https://blog.csdn.net/kao2406");
MODULE_DESCRIPTION("ADC driver");

module_init (adc_mod_init);
module_exit (adc_mod_cleanup);




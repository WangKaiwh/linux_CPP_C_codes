/*
* AST 2500 ADC Driver
* Portions Copyright WangKai .
*
* https://blog.csdn.net/kao2406
*  
* 用TDD的思路重构ADC驱动.
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
static int adc_get_chan_status(int chan_no)
{
    u32 regv = readl(adc_reg_base + ADC_ENGINE_CONTROL_REGISTER);

    return ( regv & (1 << chan_no) ) >> chan_no;
}

static int adc_get_clock_divisor(void)
{
    u32 regv = readl(adc_reg_base + ADC_CLOCK_CONTROL_REGISTER);

    return regv & ADC_DIVISOR_OF_ADC_CLOCK;
}

static void adc_set_clock(int divisor)
{
    u32 regv = readl(adc_reg_base + ADC_CLOCK_CONTROL_REGISTER);

    regv &= ~ADC_DIVISOR_OF_ADC_CLOCK; // clear [9:0]
    regv |= divisor;
    
    writel(regv, adc_reg_base + ADC_CLOCK_CONTROL_REGISTER);
}

// chan_no, which to control
// enable,  enabled or disabled
static void adc_enable_chan(int chan_no, int enable)
{
    u32 regv = readl(adc_reg_base + ADC_ENGINE_CONTROL_REGISTER);

    regv &= ~(1 << chan_no);

    if (1 == enable)
        regv |= 1 << chan_no;

    writel(regv, adc_reg_base + ADC_ENGINE_CONTROL_REGISTER);
}

static u32 adc_measure(int chan_no)
{
    void *__iomem database = adc_reg_base + ADC_DATA_REGISTER;

    u8 offset = 4 * (chan_no / 2);

    u32 regv = readl(database + offset);

    if (0 == chan_no % 2)
        return (regv >> 16) & 0x3ff;
    else 
        return regv & 0x3ff;
}

static int adc_ioctl (struct file *filp,
                            unsigned int cmd,
                            unsigned long arg)
{
    IO_ACCESS_DATA *io_access = (IO_ACCESS_DATA *)arg;

    switch (cmd)
    {
        case IOCTL_SET_ADC_CLOCK:
        {
            adc_set_clock(io_access->Data);
            return 0;
        }
        
        case IOCTL_ADC_MEASURE:
        {
            u32 rawdata = 0;
            rawdata = adc_measure(io_access->Address);
            io_access->Data = rawdata;
            return 0;
        }
        
        case IOCTL_ENABLE_ADC:
        {
            adc_enable_chan(io_access->Address, io_access->Data);
            return 0;
        }
        default:
            return -1;
    }
}

static void adc_init(void)
{
    adc_set_clock(0x40);
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

int test_adc_mod_init__all_chan_disable(void)
{
    TEST_ASSERT_EQUAL_INT(0, adc_get_chan_status(0));
    TEST_ASSERT_EQUAL_INT(0, adc_get_chan_status(1));
    return true;
}

int test_adc_mod_init__clock_divisor_0x40(void)
{
    TEST_ASSERT_EQUAL_INT(0x40, adc_get_clock_divisor());
    return true;
}

int test_adc_mod_init__reg_chardev(void)
{
    // nothing do to
    return true;
}

int test_adc_mod_init__ioctl_set_clock(void)
{
    const int set_divisor = 0xa5;
    int result_divisor = 0;

    IO_ACCESS_DATA io_data;

    memset(&io_data, 0, sizeof(io_data));
    io_data.Data = set_divisor;

    adc_ioctl(NULL, IOCTL_SET_ADC_CLOCK, (unsigned long)&io_data);

    result_divisor = adc_get_clock_divisor();

    TEST_ASSERT_EQUAL_INT(result_divisor, set_divisor);
    
    return true;
}

int test_adc_measure__before_enable(void)
{
    int ret = 0;
    IO_ACCESS_DATA io_data;

    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 1;
    ret = adc_ioctl(NULL, IOCTL_ADC_MEASURE, (unsigned long)&io_data);

    TEST_ASSERT_TRUE(0 != ret);

    return true;
}

int test_adc_get_chan_status__after_enable_status_on(void)
{
    int ret = -1;
    IO_ACCESS_DATA io_data;

    // disable firstly    
    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 2;
    io_data.Data = 0;
    adc_ioctl(NULL, IOCTL_ENABLE_ADC, (unsigned long)&io_data);

    // enable secondly
    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 2;
    io_data.Data = 1;
    adc_ioctl(NULL, IOCTL_ENABLE_ADC, (unsigned long)&io_data);

    // check status
    ret = adc_get_chan_status(2);
    TEST_ASSERT_EQUAL_INT(1, ret);

    return true;
}

int test_adc_get_chan_status__after_disable_status_off(void)
{
    int ret = -1;
    IO_ACCESS_DATA io_data;

    // enable firstly
    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 3;
    io_data.Data = 1;
    adc_ioctl(NULL, IOCTL_ENABLE_ADC, (unsigned long)&io_data);

    // disable secondly    
    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 3;
    io_data.Data = 0;
    adc_ioctl(NULL, IOCTL_ENABLE_ADC, (unsigned long)&io_data);

    ret = adc_get_chan_status(3);

    TEST_ASSERT_EQUAL_INT(0, ret);

    return true;
}

int test_adc_measure__when_chan_enable(void)
{
    IO_ACCESS_DATA io_data;

    // first, enable one chan
    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 4;
    io_data.Data = 1;
    adc_ioctl(NULL, IOCTL_ENABLE_ADC, (unsigned long)&io_data);

    // second, measure
    memset(&io_data, 0, sizeof(io_data));
    io_data.Address = 4;
    adc_ioctl(NULL, IOCTL_ADC_MEASURE, (unsigned long)&io_data);
    TEST_ASSERT_TRUE(0 != io_data.Data);

    return true;
}
#endif

int __init adc_mod_init(void)
{

#if ENABLE_TDD > 0
    int __unity_cnt = 0;
    TEST_BEGIN(__unity_cnt);

    adc_init(); // temp
    
    RUN_TEST(test_adc_mod_init__all_chan_disable(), __unity_cnt);
    RUN_TEST(test_adc_mod_init__clock_divisor_0x40(), __unity_cnt);
    RUN_TEST(test_adc_mod_init__reg_chardev(), __unity_cnt);
    RUN_TEST(test_adc_mod_init__ioctl_set_clock(), __unity_cnt);
    RUN_TEST(test_adc_measure__before_enable(), __unity_cnt);
    RUN_TEST(test_adc_get_chan_status__after_enable_status_on(), __unity_cnt);
    RUN_TEST(test_adc_get_chan_status__after_disable_status_off(), __unity_cnt);
    RUN_TEST(test_adc_measure__when_chan_enable(), __unity_cnt);
    

    TEST_END(__unity_cnt);
#endif
    return 0;
}

void __exit adc_mod_cleanup (void)
{

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WangKai -- https://blog.csdn.net/kao2406");
MODULE_DESCRIPTION("ADC driver");

module_init (adc_mod_init);
module_exit (adc_mod_cleanup);




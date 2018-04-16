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

#define ENABLE_TDD      1

// product codes


// tdd codes
#if ENABLE_TDD > 0
#include "test_template/test_template.h"

#endif

int __init adc_mod_init(void)
{
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




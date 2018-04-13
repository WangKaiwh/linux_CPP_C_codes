/*
* AST 2500 ADC Driver
* Portions Copyright (C) 2015 Insyde Software Corp.
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
#include <mach/hardware.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "inc/ioaccess.h"
#include "inc/adc_define.h"

#include <linux/dbgklogd.h>

#if defined (__GNUC__)
#define DKLOG(lvl, fmt, ...)            if (gAdcDebugFlag) { printk(lvl "%s: " fmt, __FUNCTION__, ##__VA_ARGS__); }
#elif defined (__STDC__)
#define DKLOG(lvl, fmt, ...)            if (gAdcDebugFlag) { printk(lvl "%s: " fmt, __func__, ##__VA_ARGS__); }
#else
#define DKLOG(lvl, fmt, ...)            if (gAdcDebugFlag) { printk(lvl fmt, ##__VA_ARGS__); }
#endif

#define read_register(address)	*(volatile unsigned int *)(IO_ADDRESS((unsigned int)(address)))
#define write_register(address, data)	*(volatile unsigned int *)(IO_ADDRESS((unsigned int)(address))) = (unsigned int)(data)
#define write_register_or(address, data)	*(volatile unsigned int *)(IO_ADDRESS((unsigned int)(address))) = (unsigned int)(read_register(address) | data)
#define write_register_and(address, anddata, data)	*(volatile unsigned int *)(IO_ADDRESS((unsigned int)(address))) = (unsigned int)((read_register(address) & (~anddata)) | data)

int ioaccess_major = 0;
int gAdcDebugFlag = 0;

unsigned char set_adc_clock(unsigned char, unsigned char);
void adc_init(void);
int adc_measure(unsigned char);
int enable_adc_channel(unsigned char);
static long CV_Data = 0;
static int ioaccess_ioctl(struct file *filp, unsigned cmd, unsigned long arg);

struct file_operations ioaccess_fops = {
    .unlocked_ioctl  = ioaccess_ioctl,
};


static int ioaccess_ioctl (struct file *filp,
                            unsigned int cmd,
                            unsigned long arg) {
  int ret = 0;
  long readdata = 0;

  IO_ACCESS_DATA Kernel_IO_Data;

  memset (&Kernel_IO_Data, 0, sizeof(IO_ACCESS_DATA));

  Kernel_IO_Data = *(IO_ACCESS_DATA *) arg;

  switch (cmd) {
  case IOCTL_IO_READ:
    Kernel_IO_Data.Data = *(unsigned int *) (IO_ADDRESS (Kernel_IO_Data.Address));
    DKLOG_INFO("IOCTL_IO_READ (value = %ld)\n", Kernel_IO_Data.Data);
    *(IO_ACCESS_DATA *) arg = Kernel_IO_Data;
    ret = 0;
    break;

  case IOCTL_IO_WRITE:
    DKLOG_INFO("IOCTL_IO_WRITE (value = %ld)\n", Kernel_IO_Data.Data);
    *(unsigned int *) (IO_ADDRESS (Kernel_IO_Data.Address)) = Kernel_IO_Data.Data;
    ret = 0;
    break;

  case IOCTL_SET_ADC_CLOCK:
    DKLOG_INFO("IOCTL_SET_ADC_CLOCK (value = %ld)\n", Kernel_IO_Data.Data);
    ret = set_adc_clock ((unsigned char) Kernel_IO_Data.Address, (unsigned char) Kernel_IO_Data.Data);
    break;

  case IOCTL_ADC_MEASURE:
    readdata = adc_measure ((unsigned char) Kernel_IO_Data.Address);
    readdata = readdata + CV_Data;
    if( (readdata < 0) || (readdata == CV_Data) ) {
        Kernel_IO_Data.Data = 0;
    } else {
        Kernel_IO_Data.Data = (unsigned long)readdata;
    }
    DKLOG_INFO("IOCTL_ADC_MEASURE (value = %ld)\n", Kernel_IO_Data.Data);
    *(IO_ACCESS_DATA *) arg = Kernel_IO_Data;
    ret = (int) Kernel_IO_Data.Data;
    break;

  case IOCTL_ENABLE_ADC:
    DKLOG_INFO("IOCTL_ENABLE_ADC (value = %ld)\n", Kernel_IO_Data.Address);
    ret = enable_adc_channel ((unsigned char) Kernel_IO_Data.Address);
    break;

  default:
    ret = 3;
  }
  return (ret);
}

unsigned char set_adc_clock (unsigned char adc_clock_divisor, unsigned char inverse) {
  DKLOG_DEBUG("IOCTL_SET_ADC_CLOCK (RegValue = 0x%X)\n", (0x01 << ADC_CLOCK_INVERSE) + ADC_DIVISOR_OF_ADC_CLOCK, adc_clock_divisor | (inverse << ADC_CLOCK_INVERSE));
  write_register_and (ADC_CLOCK_CONTROL_REGISTER,
                      (0x01 << ADC_CLOCK_INVERSE) + ADC_DIVISOR_OF_ADC_CLOCK,
                      adc_clock_divisor | (inverse << ADC_CLOCK_INVERSE));
  return (1);
}

int adc_measure (unsigned char adc_channel_selection) {
  volatile unsigned long raw_data;

  raw_data = read_register(ADC_DATA_REGISTER + 4 * (adc_channel_selection / 2));
  if ((adc_channel_selection % 2)) {
    raw_data = (raw_data >> 16) & 0x3ff;
  } else {
    raw_data &= 0x3ff;
  }

  DKLOG_DEBUG("Measured ADC Channel[%d] Raw Data: %lu", adc_channel_selection, raw_data);
  return (raw_data);
}

int enable_adc_channel (unsigned char adc_channel_selection) {
  write_register_or (ADC_ENGINE_CONTROL_REGISTER,
                     0x01 << (ADC_CHANNEL_ENABLE + adc_channel_selection));
  return (1);
}

int disable_adc_channel (unsigned char adc_channel_selection) {
  write_register_and (ADC_ENGINE_CONTROL_REGISTER,
                      0x01 << (ADC_CHANNEL_ENABLE + adc_channel_selection),
                      0x00);
  return (1);
}

/*
//1. The recommend ADC divisor of clock is from 120Khz ~ 6Mhz in voltage and 120Khz ~ 1Mhz in temperature
//2. ADC measure will use 12T
//3. We will enable ADC channel0, Normal operation mode and divisor of clock to 0x40 as default in this sample (it's around 200Khz)
*/
void adc_init () {
  //SCU Password
  write_register(SCU_PASSWORD_REGISTER, SCU_PROTECTION_KEY);

  //SCU ADC Reset
  write_register_or(SCU_RESET_CONTROL_REGISTER, ADC_RESET_BIT);
  barrier ();
  write_register_and(SCU_RESET_CONTROL_REGISTER, ADC_RESET_BIT, 0);
  barrier ();

  //Engine Clock
  write_register(ADC_CLOCK_CONTROL_REGISTER, 0x40);

  // Initial Sequence Check
  write_register(ADC_ENGINE_CONTROL_REGISTER, 0x0000000f);
  while ((read_register(ADC_ENGINE_CONTROL_REGISTER) & (1<<8)) == 0);

  //Compensating Sensing
  write_register(ADC_ENGINE_CONTROL_REGISTER, 0x0000001f);
  write_register(ADC_ENGINE_CONTROL_REGISTER, 0x0001001f);

  mdelay (1000);

  CV_Data = 0x200 - adc_measure (0);

  DKLOG_INFO("adc_init() CV_Data = %ld\n", CV_Data);

  //Compensating Sensing End
  //Enable Engine, and Normal Operation Mode
  write_register(ADC_ENGINE_CONTROL_REGISTER, 0x0000000f);
  write_register(SCU_PASSWORD_REGISTER, 0xaa);
}

int adc_mod_init (void) {
  int result;
  DKLOG_ALERT ("ADC Module Insert, Build Time %s\n", __TIME__);

  result = register_chrdev (ADC_MAJOR, ADC_MODULE_NAME, &ioaccess_fops);
  if (result < 0) {
    DKLOG_ERR("Can't get major number.\n");
    return (result);
  }
  if (ioaccess_major == 0) {
    ioaccess_major = result;
    DKLOG(KERN_ALERT, "Auto assigned ioaccess_major = %d \n", ioaccess_major);
  }
  adc_init ();
  DKLOG_ALERT("ADC Module Insert done\n");
  return (0);
}

void adc_mod_cleanup (void) {
  unregister_chrdev (ADC_MAJOR, ADC_MODULE_NAME);
  DKLOG_ALERT ("ADC Module cleanup\n");
  return;
}

MODULE_LICENSE ("GPL");
MODULE_AUTHOR("Insyde Software Corp.");
MODULE_DESCRIPTION("ADC driver");
module_param_named(debug, gAdcDebugFlag, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(gAdcDebugFlag, "Debug flag for adc driver");
module_init (adc_mod_init);
module_exit (adc_mod_cleanup);

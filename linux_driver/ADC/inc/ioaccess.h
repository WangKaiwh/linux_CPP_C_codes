/*
* IO Control Numbers for ADC Driver
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

#include <linux/socket.h>
#include <linux/tcp.h>

#define  IOCTL_IO_READ                  0x1103
#define  IOCTL_IO_WRITE                 0x1104
#define  IOCTL_REMAP                    0x1105
#define  IOCTL_REAL_IO_READ             0x1106
#define  IOCTL_REAL_IO_WRITE            0x1107
#define  IOCTL_BIT_STREAM_BASE          0x1108
#define  IOCTL_TX_BIT_STREAM            0x1109
#define  IOCTL_GET_SOCKET               0x1110
#define  IOCTL_AUTOMODE_TRIGGER         0x1111
#define  IOCTL_PASS3_TRIGGER            0x1112
#define  IOCTL_I2C_READ                 0x1113
#define  IOCTL_I2C_WRITE                0x1114
#define  IOCTL_DRAM_TEST                0x1115
#define  IOCTL_VUART_TEST               0x1116
#define  IOCTL_SET_PWM                  0x1117
#define  IOCTL_TACHO_MEASURE            0x1118
#define  IOCTL_ENABLE_PWM               0x1119
#define  IOCTL_ENABLE_TACHO             0x111A
#define  IOCTL_PECI_FIRE                0x111B
#define  IOCTL_ENABLE_ADC               0x111C
#define  IOCTL_ADC_MEASURE              0x111D
#define  IOCTL_SET_ADC_CLOCK            0x111E

typedef struct _IO_ACCESS_DATA {
    unsigned char Type;
    unsigned long Address;
    unsigned long Data;
    unsigned long Value;
    unsigned long I2CValue;
    int      kernel_socket;
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

// Commands for PWN FAN I/O control 
#define PWMFAN_IOCTL_IOREAD          0
#define PWMFAN_IOCTL_IOWRITE         1
#define PWMFAN_IOCTL_SET_PWM         2
#define PWMFAN_IOCTL_READ_TACHO      3
#define PWMFAN_IOCTL_ENABLE_PWM      4
#define PWMFAN_IOCTL_ENABLE_TACHO    5

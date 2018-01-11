/******************************************************************************
*
*                  INTEL CORPORATION PROPRIETARY INFORMATION
*      This software is supplied under the terms of a license agreement or
*      nondisclosure agreement with Intel Corporation and may not be copied
*      or disclosed except in accordance with the terms of that agreement.
*          Copyright (c) 2014 Intel Corporation. All Rights Reserved.
*
*  Abstract:  This header file defines the API for a simple i2c single master mode only
*             polled driver for the ROM bootloader.
*
*  Contents:
*
******************************************************************************/

#ifndef _I2C_HW_H_
#define _I2C_HW_H_ 1

#define AST_I2C_REG_BASE            0x1E78A000      /* I2C controller registers base address */

/* I2C device registers offset */
#define I2C_DISR    AST_I2C_REG_BASE        /* I2C controller device interrupt status register */
#define I2C_FCR     (i2c_bus_base_addr + 0x00)      /* function control register */
#define I2C_CATCR1  (i2c_bus_base_addr + 0x04)      /* clock and AT timming control register 1 */
#define I2C_CATCR2  (i2c_bus_base_addr + 0x08)      /* clock and AT timming control register 2 */
#define I2C_ICR     (i2c_bus_base_addr + 0x0C)      /* interrupt control register */
#define I2C_ISR     (i2c_bus_base_addr + 0x10)      /* interrupt status register */
#define I2C_CSR     (i2c_bus_base_addr + 0x14)      /* command/status register */
#define I2C_DR      (i2c_bus_base_addr + 0x18)      /* device address register */
#define I2C_BCR     (i2c_bus_base_addr + 0x1C)      /* buffer control register */
#define I2C_TRBB    (i2c_bus_base_addr + 0x20)      /* transmit/receive byte buffer */
#define I2C_DMCR    (i2c_bus_base_addr + 0x24)      /* DMA mode control register */
#define I2C_DMSR    (i2c_bus_base_addr + 0x28)      /* DMA mode status register */

#endif /* _I2C_HW_H_ */

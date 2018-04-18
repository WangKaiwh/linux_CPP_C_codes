#ifndef _ADC_DEFINE_H_
#define _ADC_DEFINE_H_

#define CONFIG_CPU_ASPEED_2500

#if defined(CONFIG_CPU_ASPEED_2300) || defined(CONFIG_CPU_ASPEED_2400) || defined(CONFIG_CPU_ASPEED_2500)
#define ADC_MAJOR                         255
#define ADC_MODULE_NAME                   "Analog to Digital Converter"

//SCU Controller Register
#define SCU_PASSWORD_REGISTER             0x1E6E2000
#define SCU_RESET_CONTROL_REGISTER        0x1E6E2004
#define SCU_PROTECTION_KEY                0x1688A8A8
#define ADC_RESET_BIT                     (1 << 23)

//ADC Engine Register
#define PCI_AHB_OFFSET                    0x10000
#define ADC_ENGINE_ADDRESS                0x1E6E9000

// OFFSET
#define ADC_ENGINE_CONTROL_REGISTER       0x0
#define ADC_INTERRUPT_CONTROL_REGISTER    0x4
#define ADC_CLOCK_CONTROL_REGISTER        0xC
#define ADC_DATA_REGISTER                 0x10
#define ADC_DATA_0_1                      0x10
#define ADC_DATA_2_3                      0x14
#define ADC_DATA_4_5                      0x18
#define ADC_DATA_6_7                      0x1C
#define ADC_DATA_8_9                      0x20
#define ADC_DATA_10_11                    0x24
#define ADC_DATA_12                       0x28
#define ADC_DATA_13                       0x2C
#define ADC_UPPER_LOWER_BOUND_0           0x30
#define ADC_UPPER_LOWER_BOUND_1           0x34
#define ADC_UPPER_LOWER_BOUND_2           0x38
#define ADC_UPPER_LOWER_BOUND_3           0x3C
#define ADC_UPPER_LOWER_BOUND_4           0x40
#define ADC_UPPER_LOWER_BOUND_5           0x44
#define ADC_UPPER_LOWER_BOUND_6           0x48
#define ADC_UPPER_LOWER_BOUND_7           0x4C
#define ADC_UPPER_LOWER_BOUND_8           0x50
#define ADC_UPPER_LOWER_BOUND_9           0x54
#define ADC_UPPER_LOWER_BOUND_10          0x58
#define ADC_UPPER_LOWER_BOUND_11          0x5C
#define ADC_UPPER_LOWER_BOUND_12          0x60
#define ADC_UPPER_LOWER_BOUND_13          0x64


//ADC_ENGINE_CONTROL_REGISTER
#define ENGINE_ENABLE                     (1 << 0)
#define ADC_OPERATION_MODE_SELECTION      (0xf << 1)
#define ADC_TEMPERATURE_SENSE_SELECTION   (1 << 5)
#define ADC_RESENSE_OR_BIAS_VALUE_12      (1 << 6)
#define ADC_RESENSE_OR_BIAS_VALUE_13      (1 << 7)
#define ADC_CHANNEL_ENABLE                16

//ADC_INTERRUPT_CONTROL_REGISTER
#define ADC_INTERRUPT_STATUS              (1 << 0)
#define ADC_INTERRUPT_ENABLE              (1 << 16)

//ADC_CLOCK_CONTROL_REGISTER
#define ADC_DIVISOR_OF_ADC_CLOCK          0x3ff
#define ADC_CLOCK_INVERSE                 16

//ADC_DATA_0_1
#define ADC_DATA_CHANNEL_LOW              0x3ff
#define ADC_DATA_CHANNEL_HIGH             (0x3ff << 16)

#endif // CONFIG_CPU_ASPEED_2300/2400/2500

#endif // _ADC_DEFINE_H_

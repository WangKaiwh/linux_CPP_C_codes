#include <iostream>
#include <cstdlib>

typedef unsigned short UINT16;
typedef unsigned char  UINT8;


typedef struct
    {
    // Bytes 0-1
    UINT16 u16ClkDivisor;  // I2C Clock Divisor - unused with ESB2

    // Byte 2
    UINT8 u8PrivateBusId;  // private bus id used by M W/R I2C

    // Byte 3-4
    UINT16 u16ModeFlags;

    // Byte 5
    UINT8 u8BusTimeout;  // Bus Timeout Vaue in units of 10 ms

    // Bytes 6-7
    UINT16 u16BusSpeedInKHz;  // Bus speed in units of KHz

} __attribute__((packed)) SPIAPhysicalI2CBus;


int main()
{
    SPIAPhysicalI2CBus obj = {
    .u16ClkDivisor = 0x1234,
    .u8PrivateBusId = 0x56,
    .u16ModeFlags = 0x7890,
    .u8BusTimeout = 0xaa,
    .u16BusSpeedInKHz = 0xbbcc
    };

    std::system("uname -a");

    std::cout << "total size: " << sizeof(SPIAPhysicalI2CBus) << "(Bytes)" << std::endl; 

    for (int i=0; i<sizeof(SPIAPhysicalI2CBus); i++)
    {   
        UINT8 *p = ((UINT8 *)&obj) + i;  
        std::cout << " " << *p; 
    }
    std::cout << std::endl;

    return 0;
}

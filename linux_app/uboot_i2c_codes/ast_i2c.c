#include <common.h>
#include "i2c.h"
#include "i2c-hw.h"
//#include <platform.h>

//#include "autoconfig.h"


/*----------------------------------------------------------------------------------------------*/
/*------------------------------------  Common stuff for All SOC -------------------------------*/
/*----------------------------------------------------------------------------------------------*/

static void real_i2c_init(int speed, int slaveadd, int mode);
static int i2c_init_pending = 1;

static int i2c_bus_base_addr = 0;
static int i2c_bus_num =  - 1; //0 base


static int i2c_bus_slaveaddr = 0x10;    /* Default BMC Address */
static int i2c_bus_speed = 100000;      /* Default 100Khz */

#if 0
unsigned int i2c_get_bus_speed(void)
{
    return i2c_bus_speed;
}

int i2c_set_bus_speed(unsigned int speed)
{
    i2c_bus_speed = speed;
    real_i2c_init(i2c_bus_speed, i2c_bus_slaveaddr);
    return 0;
}

unsigned int i2c_get_bus_num(void)
{
    return i2c_bus_num;
}

int i2c_set_bus_num(unsigned int bus)
{
    i2c_bus_num = bus;
    real_i2c_init(i2c_bus_speed, i2c_bus_slaveaddr);
    return 0;
}
#endif

#if 0                           /* Already defined in i2c.h */
uchar i2c_reg_read(uchar i2c_addr, uchar reg)
{
    unsigned char buf;
    i2c_read(i2c_addr, reg, 1, &buf, 1);
    return buf;
}

void i2c_reg_write(uchar i2c_addr, uchar reg, uchar val)
{
    i2c_write(i2c_addr, reg, 1, &val, 1);
    return;
}
#endif

/*----------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------*/
/* ---------------------  i2c interface functions (SOC Specific --------------------------------*/
/*----------------------------------------------------------------------------------------------*/

static int i2c_start(unsigned char chip, unsigned char read_write);
static void i2c_stop(void);
static unsigned long i2c_interrupt_status(void);
static int i2c_send_byte(unsigned char byte, int mode);
static int i2c_receive_byte(unsigned char *byte, int last, int mode);

/* I2C operations */
#define I2C_WRITE 0
#define I2C_READ 1

#if 0
void i2c_init(int speed, int slaveadd)
{
    /* This is called before relocation and hence we cannot access global variables */
    return;
}
#endif

enum {
    slavem,
    masterm
};

static void real_i2c_init(int speed, int slaveadd, int mode)
{
    volatile unsigned long scu_reg;

    // printf( "**************** in real_i2c_init with i2c_bus_num = %d (decimal) ********************\n", i2c_bus_num );

    /* Calculate bus address based on bus number */
    if(i2c_bus_num >= 7)
        i2c_bus_base_addr =
            (AST_I2C_REG_BASE + 0x100 + (0x040 * (i2c_bus_num + 1)));
    else
        i2c_bus_base_addr =
            (AST_I2C_REG_BASE + (0x040 * (i2c_bus_num + 1)));

    printf( "and i2c_bus_base_addr = 0x%08x \n", i2c_bus_base_addr );

    *((volatile ulong *) AST_SCU_BASE) = 0x1688A8A8;    /* unlock SCU, SCU00: Protection Key Register */

    if(i2c_bus_num >= 2) {
        scu_reg = *((volatile ulong *) (AST_SCU_BASE + 0x90));
        if(9 <= i2c_bus_num && i2c_bus_num <= 12)
            scu_reg &= ~0x01;   // Disable SD1 pin functions to make available for i2c use
        *((volatile ulong *) (AST_SCU_BASE + 0x90)) = scu_reg | (0x4000 << i2c_bus_num);        /*enable multi function pin */
        // printf( "SCU Reg %08x written with %08lx\n", AST_SCU_VA_BASE+0x90, scu_reg | (0x4000 << i2c_bus_num));
    }

    /* stop I2C controller reset */
    scu_reg = *((volatile ulong *) (AST_SCU_BASE + 0x04));
    *((volatile ulong *) (AST_SCU_BASE + 0x04)) = scu_reg & ~(0x04);

//      *((volatile ulong *) AST_SCU_BASE) = 0; /* lock SCU */

    /* Set Clock and AC timing Control Register 1 and 2 */
    /* the divisor is 256 */
    *((volatile unsigned long *) I2C_CATCR1) = 0x77777344;
    *((volatile unsigned long *) I2C_CATCR2) = 0x00000000;

    /* Set Slave address */
    *((volatile unsigned long *) I2C_DR) = slaveadd;

    /* Clear Interrupt Status register */
    *((volatile unsigned long *) I2C_ISR) = 0xFFFFFFFF; /* write 1 to clear bits */


    /* Set Master or Slave mode */
    *((volatile unsigned long *) I2C_FCR) = (1 << 0) | (1<<1) | (1<<2);       /* slave master enable */


    /* Set Interrupt Control register */
    *((volatile unsigned long *) I2C_ICR) = 0x0000002F;

    i2c_init_pending = 0;
}

int i2c_probe(uchar chip, int mode)
{
    int ret = 0;

    real_i2c_init(i2c_bus_speed, i2c_bus_slaveaddr, mode);

    if(i2c_start(chip, I2C_WRITE))      /* Do not use I2C_READ or the bus will be blocked by salve */
        ret = 1;

    i2c_stop();

    return ret;
}

int i2c_read(uchar chip, uint addr, int alen, uchar * buffer, int len, int mode)
{
    int i = 0, last = 0, ret = 0;

    printf( "In i2c_read with chip = %02x, addr = %04x, alen = %d, and len = %d\n", chip, addr, alen, len );

    if(i2c_init_pending){
        real_i2c_init(i2c_bus_speed, i2c_bus_slaveaddr, mode);
    }

    if(i2c_start(chip, I2C_WRITE)){
        return 1;
    }
    printf( "i2c_start(chip, I2C_WRITE) succeeded\n");

    for (i = 0; i < alen; i++) {
        if(i2c_send_byte((addr >> ((alen - 1 - i) * 8)) & 0xFF, mode)) {
            return 1;
        }
    }

    if(i2c_start(chip, I2C_READ))
        return 1;
    printf( "i2c_start(chip, I2C_READ) succeeded\n");

    for (i = 0; i < len; i++) {
        last = (i < (len - 1)) ? 0 : 1;
        if(i2c_receive_byte(&(buffer[i]), last, mode)) {
            printf("%s %d read error!\n", __func__, (int)__LINE__);
            ret = 1;
        }
    }

    i2c_stop();

    return ret;
}

int i2c_write(uchar chip, uint addr, int alen, uchar * buffer, int len, int mode)
{
    int i = 0, ret = 0;

    if(i2c_init_pending)
        real_i2c_init(i2c_bus_speed, i2c_bus_slaveaddr, mode);

    if(i2c_start(chip, I2C_WRITE))
        return 1;

    for (i = 0; i < alen; i++) {
        if(i2c_send_byte((addr >> ((alen - 1 - i) * 8)) & 0xFF, mode)) {
            return 1;
        }
    }

    for (i = 0; i < len; i++) {
        if(i2c_send_byte(buffer[i], mode)) {
            ret = 1;
        }
    }

    i2c_stop();

    return ret;
}

/*----------------------------------------------------------------------------------------------*/
/* ---------------------  i2c internal fuctions for SOC         --------------------------------*/
/*----------------------------------------------------------------------------------------------*/
static int i2c_send_byte(unsigned char byte, int mode)
{
    printf("%s writereg=%#x\n", __func__, byte & 0x000000FF);

    *((volatile unsigned long *) I2C_TRBB) = byte & 0x000000FF;
    if (slavem == mode)
    {
        *((volatile unsigned long *) I2C_CSR) = 1 << 2;       /* slave transmit command */
    }
    else
    {
        *((volatile unsigned long *) I2C_CSR) = 1 << 1;       /* master transmit command */
    }

    if(!(i2c_interrupt_status() & 0x03)) {      /* transmit done with ACK returned */
        return 1;
    }

    return 0;
}

static int i2c_receive_byte(unsigned char *byte, int last, int mode)
{
    if (masterm == mode)
        *((volatile unsigned long *) I2C_CSR) =  0x08 | (last << 4); /* master receive command, master/slave receive command last */
    else
        *((volatile unsigned long *) I2C_CSR) =  /*0x08 |*/ (last << 4);

    if(!(i2c_interrupt_status() & 0x04)) {      /* receive done interrupt */
        return 1;
    }

    *byte = (unsigned char) ((*((volatile unsigned long *) I2C_TRBB) & 0x0000FF00) >> 8);

    return 0;
}

static int i2c_start(unsigned char chip, unsigned char read_write)
{
    *((volatile unsigned long *) I2C_TRBB) = (chip << 1) | (read_write & 0x1);  /* load address to buffer and specify read/write */
    *((volatile unsigned long *) I2C_CSR) = 0x03;       /* master start command, master transmit command */

    if(i2c_interrupt_status() & 0x01) { /* transmit done with ACK returned interrupt */
        return 0;
    }

    return 1;
}

static void i2c_stop(void)
{
    *((volatile unsigned long *) I2C_CSR) = 0x0030;     /* master stop command */
}

static unsigned long i2c_interrupt_status(void)
{
    unsigned long isr;          /* device interrupt status register */
    unsigned long device_bit;
    unsigned long interrup_status;
    long i;

    interrup_status = 0;
    // printf( "Address of I2C_DISR is %08x\n", I2C_DISR );

    for (i = 0; i < 100000; i++) {      /* poll device interrupt status */
        isr = *((volatile unsigned long *) I2C_DISR);

        device_bit = 0x01 << i2c_bus_num;
        if((isr & device_bit) != 0) {   /* interrupt occurs */
            interrup_status = *((volatile unsigned long *) I2C_ISR);
            break;
        }
    }

#if 0
    if(i == 100000)
        printf("i2c time out! DISR = 0x%lX\n", *((volatile unsigned long *) I2C_DISR));
#endif

    *((volatile unsigned long *) I2C_ISR) = 0xFFFFFFFF; /* clear bits */

    return interrup_status;
}

static int do_slaverw(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{

    unsigned char byte = 0;
	if (2 != argc)
		return -1;

	i2c_bus_num = simple_strtoul(argv[1], NULL, 16);

	real_i2c_init(i2c_bus_speed, i2c_bus_slaveaddr, slavem);

	while (1) {
	    int ret = -1;
		if (ctrlc())
			break;

		if (0 == i2c_receive_byte(&byte, (0x21 == byte ? 1 : 0), slavem))
			printf("[r]%#x\n", byte);

        if (0x21 == byte)
        {
            byte = 0;

            char wbuf[] = {0x0};
            ret = i2c_send_byte(wbuf[0], slavem);
            if (0 != ret)
                printf("write %#x error\n", wbuf[0]);
            else
                printf("write %#x ok\n", wbuf[0]);

            wbuf[0] = 0x55;
            ret = i2c_send_byte(wbuf[0], slavem);
            if (0 != ret)
                printf("write %#x error\n", wbuf[0]);
            else
                printf("write %#x ok\n", wbuf[0]);

            //*((volatile unsigned long *) I2C_CSR) = /* 0x08 | */(1 << 4);

        }
	}
	return 0;
}


U_BOOT_CMD(
	slave,	5,	0,	do_slaverw,
	"i2c slave cmd",
	"[bus]"
);


static int do_masterrw(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	if (2 != argc)
		return -1;

	i2c_bus_num = simple_strtoul(argv[1], NULL, 16);

    //i2c_probe(0x10, masterm);
    char reg = 0;
    printf("master\n");

    reg = i2c_reg_read(0x10, 0x30, masterm);
    printf("reg %#x\n", reg);

	return 0;
}


U_BOOT_CMD(
	master,	5,	0,	do_masterrw,
	"i2c master cmd",
	"[bus]"
);



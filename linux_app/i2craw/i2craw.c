
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <assert.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include "i2c-dev.h"
#include "i2c.h"

#define I2C_SLAVE_RDWR      0x0709
#define I2C_S_EN		    0x0080	/* slave enalbe */

#define STATUS int
typedef unsigned char UINT8;
typedef unsigned int UINT32;

static int i2c_open(int bus)
{
    int fd = -1;
    char name[32] = {0};

    snprintf(name, sizeof(name), "/dev/i2c%d", bus);
    printf("open '%s'\n", name);

    fd = open(name, O_RDWR);

    return fd;
}

static STATUS i2c_slave_enable(int fd, int addr)
{
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msg_data;
    int ret;

    //don't need to use semphore as this is a async event

    /* Issue the actual slave read command */
    msg_data.addr = addr;
    msg_data.buf = 0;
    msg_data.flags = I2C_S_EN;
    //slave driver will omit ths count para as "stop" is end of frame
    msg_data.len = 0;

    //set ioctl data block
    ioctl_data.nmsgs = 1;
    ioctl_data.msgs = &msg_data;
    //Grantly let I2C_SLAVEREAD ioctl returns data length just when nmsgs = 1 is right!
    //or needs to define other ways to return data length
    ret = ioctl(fd, I2C_SLAVE_RDWR, &ioctl_data);

    if (ret < 0) {
        printf("ret %d.\n", ret);
        return -1;
    } else
        return ret;
}

static STATUS HW_I2CRawSlaveRead(int fd, UINT8 *data, size_t count)
{
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msg_data;
    int ret;

    //don't need to use semphore as this is a async event

    /* Issue the actual slave read command */
    msg_data.addr = 0;
    msg_data.buf = data;
    msg_data.flags = 0;
    //slave driver will omit ths count para as "stop" is end of frame
    msg_data.len = count;

    //set ioctl data block
    ioctl_data.nmsgs = 1;
    ioctl_data.msgs = &msg_data;
    //Grantly let I2C_SLAVEREAD ioctl returns data length just when nmsgs = 1 is right!
    //or needs to define other ways to return data length
    ret = ioctl(fd, I2C_SLAVE_RDWR, &ioctl_data);

    if (ret < 0) {
        printf("ret %d.\n", ret);
        return -1;
    } else
        return ret;
}

static STATUS HW_I2CRawSlaveWrite(int fd, UINT8 *data, size_t count)
{
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msg_data;
    int ret;

    //don't need to use semphore as this is a async event

    /* Issue the actual slave read command */
    msg_data.addr = 0;
    msg_data.buf = data;
    msg_data.flags = I2C_M_RD;
    //slave driver will omit ths count para as "stop" is end of frame
    msg_data.len = count;

    //set ioctl data block
    ioctl_data.nmsgs = 1;
    ioctl_data.msgs = &msg_data;
    //Grantly let I2C_SLAVEREAD ioctl returns data length just when nmsgs = 1 is right!
    //or needs to define other ways to return data length
    ret = ioctl(fd, I2C_SLAVE_RDWR, &ioctl_data);

    if (ret < 0) {
        printf( "ret %d.\n", ret);
        return -1;
    } else
        return ret;
}

STATUS HW_I2CRawWriteReadBytes(
    int fd,
    UINT32 u32SlaveAddr,         // Target address
    UINT8 u8NumberOfWriteBytes,  // Number of write bytes
    UINT8 *psOutBuffer,          // Ptr to outgoing data buffer
    UINT8 u8NumberOfReadBytes,   // Number of read bytes
    UINT8 *psInBuffer)           // Ptr to incoming data buffer
{
    STATUS Status = -1;
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg ioctl_msgs[2];
    int ret;

    if ((psOutBuffer == NULL) && (psInBuffer == NULL))
        return -1;

    if ((psOutBuffer != NULL && u8NumberOfWriteBytes) && (psInBuffer != NULL && u8NumberOfReadBytes)) {
        //write message
        ioctl_msgs[0].addr = u32SlaveAddr;
        ioctl_msgs[0].len = u8NumberOfWriteBytes;
        ioctl_msgs[0].buf = psOutBuffer;
        ioctl_msgs[0].flags = 0;

        //read message
        ioctl_msgs[1].addr = u32SlaveAddr;
        ioctl_msgs[1].len = u8NumberOfReadBytes;
        ioctl_msgs[1].buf = psInBuffer;
        ioctl_msgs[1].flags = I2C_M_RD;

        ioctl_data.nmsgs = 2;
    } else if (psOutBuffer != NULL && u8NumberOfWriteBytes) {
        //write message
        ioctl_msgs[0].addr = u32SlaveAddr;
        ioctl_msgs[0].len = u8NumberOfWriteBytes;
        ioctl_msgs[0].buf = psOutBuffer;
        ioctl_msgs[0].flags = 0;

        ioctl_data.nmsgs = 1;
    } else if (psInBuffer != NULL && u8NumberOfReadBytes) {
        //read message
        ioctl_msgs[0].addr = u32SlaveAddr;
        ioctl_msgs[0].len = u8NumberOfReadBytes;
        ioctl_msgs[0].buf = psInBuffer;
        ioctl_msgs[0].flags = I2C_M_RD;

        ioctl_data.nmsgs = 1;
    } else {
        /*
        ** when requesting zero byte read and zero byte write(eg, ping, master write read
        ** command with no write and no read), as I2C kernel driver cannot handle zero
        ** read nor zero write case in i2c-dev layer, as a workaound, treat it as a 1 byte
        ** read to check device's ACK response, but don't return this read byte to applications.
        */
        UINT8 buf = 0;
        ioctl_msgs[0].addr = u32SlaveAddr;
        ioctl_msgs[0].len = 1;
        ioctl_msgs[0].buf = &buf;
        ioctl_msgs[0].flags = I2C_M_RD;
        ioctl_data.nmsgs = 1;
    }

    ioctl_data.msgs = ioctl_msgs;
    //ioctl
    ret = ioctl(fd, I2C_RDWR, &ioctl_data);
    if (ret == ioctl_data.nmsgs) {
        Status = 0;
    } else {
        printf(" slave address %x, ret %d.\n", (unsigned int)u32SlaveAddr, ret);
        Status = -1;
    }

    return Status;
}

static void buf_init(char *buf, int size)
{
    int i;
    for (i=0; i<size; i++)
        buf[i] = i;
}

static void new_usage(const char *pro)
{
    printf("Usage: %s bus [opt] [reg]\n", pro);
    printf("      opt - r|w\n");
}


#define MAX_SLAVE_READ_LEN 256

#define SLAVE_ADDR 0x10

static char sg_reg = -1;

static void* i_slave_read_task(void *input_para)
{
    UINT8 u8ReadBuffer[MAX_SLAVE_READ_LEN];
    int ret = -1;
    int i;
    int fd = *(int *)input_para;

    printf("slave fd = %d\n", fd);

    //capture data and sends out
    while (1)
    {
        int sg_read = 0;
        memset(u8ReadBuffer, 0, MAX_SLAVE_READ_LEN);

        ret = HW_I2CRawSlaveRead(*(int *)input_para, u8ReadBuffer, MAX_SLAVE_READ_LEN);

        if (ret > 0)
        {
            for (i=0; i<ret ;i++)
            {
                printf("%#x \n", u8ReadBuffer[i]);
                if (u8ReadBuffer[i] == sg_reg)
                {
                    sg_read = 1;
                }

                if ( ( u8ReadBuffer[i] == ((SLAVE_ADDR << 1) | 1) )
                        && sg_read)
                {
                    char reg[] = {(SLAVE_ADDR<<1)|1, 0x55, 0xaa};

                    sleep(0.1);

                    ret = HW_I2CRawWriteReadBytes(fd, SLAVE_ADDR, sizeof(reg), reg, 0, NULL);
                    if (ret == 0)
                        printf("send reg val: %#x %#x ok\n", reg[0], reg[1]);
                    else
                        printf("send reg val: %#x %#x error\n", reg[0], reg[1]);
                }
            }
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (4 != argc) {
        new_usage(argv[0]);
        return -1;
    }

    int bus = atoi(argv[1]);
    char *opt = argv[2];
    int fd = i2c_open(bus);
    int ret = -1;
    sg_reg = atoi(argv[3]);

    printf("%s\n", __DATE__);

    // fd > 0
    assert(fd > 0);

    pthread_t ntid;
    ret = pthread_create(&ntid, NULL, i_slave_read_task, &fd);
    if (ret != 0)
        printf("can't create thread: %s\n", strerror(ret));

    sleep(1);

    if (!strncmp(opt, "r", strlen(opt)))
    {
        char cmd[2];
        cmd[0] = sg_reg;
        cmd[1] = (SLAVE_ADDR << 1) | 1;
        ret = HW_I2CRawWriteReadBytes(fd, SLAVE_ADDR, sizeof(cmd), cmd, 0, NULL);
        if (ret == 0)
            printf("send write ok\n");
        else
            printf("send write error\n");
    }
    else if (!strncmp(opt, "w", strlen(opt)))
    {
        // nothing to do
    }
    else
    {
        close(fd);
        new_usage(argv[0]);
        return -1;
    }

    while (1)
    {
        sleep(100);
    }

    pthread_join(ntid, NULL);
    close(fd);

    return 0;
}


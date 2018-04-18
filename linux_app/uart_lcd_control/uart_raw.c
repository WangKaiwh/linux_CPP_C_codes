#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <errno.h>
#include <sys/ioctl.h>

#define LCD_UART_NO			  4 // UART3

static int open_uart_dev(int b_uart_num)
{
    int fd = 0;

    switch(b_uart_num)
    {
        case 3:
            fd = open("/dev/ttyS1", O_RDWR);    //UART3
            break;
        case 4:
            fd = open("/dev/ttyS2", O_RDWR);    //UART4
            break;
        case 5:
            fd = open("/dev/ttyS0", O_RDWR);    //UART5 = debug console
            break;
        default:
            fd = -1;
            break;
    }

    return fd;
}

static void uart_speed_set(int fd)
{
    struct termios options;

    /*
     * Get the current options for the port...
     */
    tcgetattr(fd, &options);
    /*
     * Set the baud rates to 19200...
     */
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    /*
     * Enable the receiver and set local mode...
     */
    options.c_cflag |= (CLOCAL | CREAD);

    /*
     * Set the new options for the port...
     */
    tcsetattr(fd, TCSANOW, &options);
}

int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop)  
{  
    struct termios newtio,oldtio;  
    if(tcgetattr(fd,&oldtio)!=0)  
    {  
        perror("error:SetupSerial 3\n");  
        return -1;  
    }  
    bzero(&newtio,sizeof(newtio));  
    //使能串口接收  
    newtio.c_cflag |= CLOCAL | CREAD;  
    newtio.c_cflag &= ~CSIZE;  
      
    newtio.c_lflag &=~ICANON;//原始模式    
                               
    //newtio.c_lflag |=ICANON; //标准模式   
      
    //设置串口数据位  
    switch(nBits)  
    {  
        case 7:  
            newtio.c_cflag |= CS7;  
            break;  
        case 8:  
            newtio.c_cflag |=CS8;  
            break;  
    }  
    //设置奇偶校验位  
    switch(nEvent)  
      
    {  
        case 'O':  
            newtio.c_cflag |= PARENB;  
            newtio.c_cflag |= PARODD;  
            newtio.c_iflag |= (INPCK | ISTRIP);  
            break;  
        case 'E':  
            newtio.c_iflag |= (INPCK | ISTRIP);  
            newtio.c_cflag |= PARENB;  
            newtio.c_cflag &= ~PARODD;  
            break;  
        case 'N':  
            newtio.c_cflag &=~PARENB;  
            break;  
    }  
    //设置串口波特率  
    switch(nSpeed)  
    {  
        case 2400:  
            cfsetispeed(&newtio,B2400);  
            cfsetospeed(&newtio,B2400);  
            break;  
        case 4800:  
            cfsetispeed(&newtio,B4800);  
            cfsetospeed(&newtio,B4800);  
            break;  
        case 9600:  
            cfsetispeed(&newtio,B9600);  
            cfsetospeed(&newtio,B9600);  
            break;  
        case 115200:  
            cfsetispeed(&newtio,B115200);  
            cfsetospeed(&newtio,B115200);  
            break;  
        case 460800:  
            cfsetispeed(&newtio,B460800);  
            cfsetospeed(&newtio,B460800);  
            break;  
        default:  
            cfsetispeed(&newtio,B9600);  
            cfsetospeed(&newtio,B9600);  
            break;  
    }  
    //设置停止位  
    if(nStop == 1)  
        newtio.c_cflag &= ~CSTOPB;  
    else if(nStop == 2)  
        newtio.c_cflag |= CSTOPB;  
    newtio.c_cc[VTIME] = 5;  
    newtio.c_cc[VMIN] = 0;  
    tcflush(fd,TCIFLUSH);  
      
    if(tcsetattr(fd,TCSANOW,&newtio)!=0)  
    {  
        perror("com set error\n");  
        return -1;  
    }  
    return 0;  
}  

static inline void usage(const char *msg)
{
    printf("usage: %s [options]\n", msg);
    printf("         options- key\n");
    printf("         options- main\n");
    printf("         options- recv\n");
}

int main(int argc, char **argv)
{
    int fd = open_uart_dev(LCD_UART_NO);
    char key[] = "page key";
    char main[] = "page main";
    char over[] = {0xff, 0xff, 0xff};
    int size = 0;
    char *opt = NULL;

    if (argc != 2) {
        usage(argv[0]);
        exit(-1);
    }

    if (fd < 0) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    
	/** baudrate: 9600 */
    //uart_speed_set(fd);
    set_opt(fd, 9600, 8, 'N', 1);

    opt = argv[1];
   
    /** 刷新按键界面 */
    if (!strncmp(opt, "key", strlen("key"))) {
        size = write(fd, key, strlen(key));
        if (strlen(key) != size)
            printf("write: '%s' error!\n", key);
        else
            printf("write %s ok\n", key);
    } else if (!strncmp(opt, "main", strlen("main"))) { /* 主界面 */
        size = write(fd, main, strlen(main));
        if (strlen(main) != size)
            printf("write: '%s' error!\n", main);
        else
            printf("write %s ok\n", main);
    } else if (!strncmp(opt, "recv", strlen("recv"))) {
        char rev[128] = {0};
        printf("recv...\n");
        while (1) {
            memset(rev, 0, sizeof(rev));
            size = read(fd, rev, sizeof(rev));
            if (size > 0) { // only printf string
                printf("rev: %s\n", rev);
            }
        }
    } else {
        usage(argv[0]);
        close(fd);
        exit(-1);
    }

    size = write(fd, over, sizeof(over));
    if (sizeof(over) != size)
        printf("write over char error!\n");
    else
        printf("write over char ok\n");

    close(fd);

    return 0;
}




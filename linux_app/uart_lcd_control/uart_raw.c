#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <errno.h>

int open_uart_dev(int b_uart_num)
{
    int fd = 0, b_LogiChannel = 0;

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

    // printf("[%s,%d]******fd = %d*****\n", __FILE__, __LINE__, fd);
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

static inline void usage(const char *msg)
{
    printf("usage: %s [options]\n", msg);
    printf("         options- key\n");
    printf("         options- main\n");
}

int main(int argc, char **argv)
{
    int fd = open_uart_dev(3);
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

    uart_speed_set(fd);

    opt = argv[1];

    /* 刷新按键界面 */
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




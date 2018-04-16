#ifndef __C_TEST_TEMPLATE_H__
#define __C_TEST_TEMPLATE_H__

#include <stdio.h>

#ifdef __cplusplus  
extern "C"
#endif

#ifndef false 
#define false   0
#endif

#ifndef true
#define true    1 
#endif

#if linux_driver
    // ...  ##__VA_ARGS__, 在windows和linux下面均可以的方法
    #define OUTPUT_MSG(fmt, ...) do {\
        printk(KERN_EMERG "%s, %s, %d: ", __FILE__, __func__,  __LINE__);\
        printk(KERN_EMERG fmt, ##__VA_ARGS__);\
    } while (0)
#else
    // ...  ##__VA_ARGS__, 在windows和linux下面均可以的方法
    #define OUTPUT_MSG(fmt, ...) do {\
        printf(fmt, ##__VA_ARGS__); \
    } while (0)
#endif

static inline int TEST_ASSERT_EQUAL_INT(
        const char *file,
        const char *func,
        int line,
        int expected, 
        int actual)
{
    if (expected != actual)
    {
        OUTPUT_MSG("%s, %s, %d, FAILED! expected: %d, actual: %d\n", 
                file, func, line, expected, actual);
        return false;
    }
    OUTPUT_MSG("%s, %s, %d, OK\n", file, func, line);
    return true;
}

#define RUN_TEST_EQUAL_INT(expected, actual) do{\
    if (false == TEST_ASSERT_EQUAL_INT(__FILE__, __func__, __LINE__, expected, actual))\
            return -1;\
    } while (0)

#endif

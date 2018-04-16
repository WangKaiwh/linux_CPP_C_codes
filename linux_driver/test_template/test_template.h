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
        printf("%s, %d: ", __FILE__, __LINE__); \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)
#endif

static inline int TEST_ASSERT_EQUAL_INT(int expected, int actual)
{
    if (expected != actual)
    {
        OUTPUT_MSG("FAILED! expected: %d, actual: %d\n", expected, actual);
        return false;
    }
    OUTPUT_MSG("OK\n");
    return true;
}

#define RUN_TEST(f) do{\
        if ( false == (f) )\
            return -1;\
    } while (0)

#endif

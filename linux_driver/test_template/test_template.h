#ifndef __C_TEST_TEMPLATE_H__
#define __C_TEST_TEMPLATE_H__

#ifdef __cplusplus  
    extern "C"
#endif

#ifndef false 
#define false   0
#endif

#ifndef true
#define true    1 
#endif

#if __KERNEL__
    // ...  ##__VA_ARGS__, windows and linux compatible
    #define OUTPUT_MSG(fmt, ...) do {\
        printk(KERN_EMERG fmt, ##__VA_ARGS__);\
    } while (0)
#else
    // ...  ##__VA_ARGS__, windows and linux compatible
    #define OUTPUT_MSG(fmt, ...) do {\
        printf(fmt, ##__VA_ARGS__); \
    } while (0)
#endif

static inline int __ASSERT_EQUAL_INT(
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

#define TEST_ASSERT_EQUAL_INT(expected, actual) do{\
    if ( false == __ASSERT_EQUAL_INT(__FILE__, __func__, __LINE__, expected, actual) )\
            return;\
    } while (0)

#endif

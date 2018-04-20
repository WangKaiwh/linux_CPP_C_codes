/*
*  描述: C语言单元测试模板
*  
*  作者: 王凯
*
*  日期: 2018.04.18
*
*  博客: https://blog.csdn.net/kao2406
* 
*  GitHub: https://github.com/WangKaiwh
*
*/
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
        OUTPUT_MSG("%s, %s, %d, FAILED! expected: %d(%#x), actual: %d(%#x)\n", 
            file, func, line, expected, expected, actual, actual);
        return false;
    }
    OUTPUT_MSG("%s, %s, %d, OK\n", file, func, line);
    return true;
}

#define MIN(x, y)  ((x) > (y) ? (y) : (x))

static inline int __ASSERT_EQUAL_STRING(
        const char *file,
        const char *func,
        int line,
        const char *left, 
        int l_bufsize,
        const char *right,
        int r_bufsize)
{
    if (NULL == left || NULL == right)
    {
        OUTPUT_MSG("null pointer error!!!\n");
        return false;
    }

    int realsize = MIN(l_bufsize, r_bufsize);

    if (0 != strncmp(left, right, realsize))
    {
        OUTPUT_MSG("%s, %s, %d, FAILED! left: %s, right: %s\n", 
            file, func, line, left, right);
        return false;
    }
    return true;
}

static inline int __ASSERT_TRUE(
        const char *file,
        const char *func,
        int line,
        int condition)
{
    if (true != condition)
    {
        OUTPUT_MSG("assert true, but actual false\n");
        return false;
    }
    return true;
}

/////////////////////////////////////////////// 
/// 对外接口(external interface) //////////////////////////////////

#define TEST_ASSERT_EQUAL_INT(expected, actual) do{\
    if ( false == __ASSERT_EQUAL_INT(__FILE__, __func__, __LINE__, expected, actual) )\
            return false;\
    } while (0)

#define TEST_ASSERT_EQUAL_STRING(left, l_bufsize, right, r_bufsize) do {\
    if ( false == __ASSERT_EQUAL_STRING(__FILE__, __func__, __LINE__, \
        (left), (l_bufsize), (right), (r_bufsize)) ) \
        return false; \
    } while (0)

#define TEST_ASSERT_TRUE(condition) do {\
    if ( false == __ASSERT_TRUE(__FILE__, __func__, __LINE__, (condition)) )\
        return false;\
    } while (0)

#define TEST_BEGIN(__cnt_macro_val) do {\
        (__cnt_macro_val) = 0;\
        OUTPUT_MSG( "\n*********************\n");\
        test_setup(); \
    } while (0)

#define RUN_TEST(__f, __cnt_macro_val) do {\
        if (false == (__f))\
            (__cnt_macro_val)++; \
    } while (0)

#define TEST_END(__cnt_macro_val) do {\
        if (__cnt_macro_val > 0)\
            OUTPUT_MSG("FAILED! %d test failed!\n", __cnt_macro_val);\
        else\
            OUTPUT_MSG("OK\n");\
        OUTPUT_MSG("*********************\n");\
        test_teardown();\
    } while (0)

#endif

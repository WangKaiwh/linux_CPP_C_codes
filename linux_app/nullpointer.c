#include <stdio.h>

struct str{
    int len;
    char s[0];
};

struct foo {
    struct str *a;
};

int main(int argc, char** argv) {
    struct foo f = {0};

    if (f.a->s) {
        printf("%d\n", f.a->s);
        *f.a->s = 8;
    }


    return 0;
}

/**
我本地的结果为:
-----------------------------
4
Segmentation fault (core dumped)
**/

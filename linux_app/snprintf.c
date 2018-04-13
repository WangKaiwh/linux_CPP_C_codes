#include <stdio.h>

int main()
{
    char version[32] = {0};

    snprintf(version, 32, "%02X%02X%02X%02X", 1,2,3,4);

    printf("%s\n", version);

    return 0;
}

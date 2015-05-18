#include <stdio.h>
#include <stdlib.h>
#include "fail.h"

void fail(unsigned condition, const char reason[])
{
    if(!condition)
        return;

    printf("Fail: %s!\n", reason);
    exit(0);
}

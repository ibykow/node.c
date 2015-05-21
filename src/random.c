#include "common.h"

void init_random() {
    struct timeval t;
    gettimeofday(&t, NULL);
    srand(t.tv_usec * t.tv_sec);
}

unsigned urand(unsigned min, unsigned max)
{
    if(max <= min)
        return min;

    return (unsigned) (rand() / (RAND_MAX / (max - min))) + min;
}

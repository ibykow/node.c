#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "fail.h"

#include "node.h"
#include "str.h"

#define pfunc() printf("%s\n", __func__)

#ifndef MIN
#define MIN(a, b) (a > b ? b : a)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#endif

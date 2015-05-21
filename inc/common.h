#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "fail.h"

#include "node.h"
#include "str.h"
#include "int.h"
#include "stack.h"
#include "test.h"

#define pfunc() printf("%s\n", __func__)

#ifndef MIN
#define MIN(a, b) (a > b ? b : a)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifdef PR_DEBUG
#define pr_dbg(fmt, ...) printf("%s " fmt "\n", __func__, ##__VA_ARGS__)
#else
#define pr_dbg(fmt, ...)
#endif

#define nop() ((void) 0)

#endif

#ifndef COMMON_H
#define COMMON_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define MIN(a, b) (a > b ? b : a)

#define SET_FLAG(reg, n, v) if (v) reg |= n; else reg &= ~n

#define GET_FLAG(reg, n) ((reg & n) ? 1 : 0)

typedef uint8_t byte;

void ignoreLog(const char *fmt, ...);

#if 0
    #define DOLOG printf
    #define ISLOGGING 1

#else
    #define DOLOG ignoreLog
#endif

#endif

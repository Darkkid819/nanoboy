#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define KNRM "\033[0m"
#define KRED "\033[1;31m"
#define KMAG "\033[1;35m"

#define NL "\n"

#ifdef DEBUG
#define debug(S, ...)                                                          \
    do {                                                                       \
        fprintf(stderr, KMAG "DEBUG: %s:%s:%d " KNRM S NL,                    \
                __FILE__, __extension__ __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#define debug(S, ...)
#endif

#ifdef DEBUG
#define error(S, ...)                                                          \
    do {                                                                       \
        fprintf(stderr, KRED "ERROR: %s:%s:%d " KNRM S NL,                    \
                __FILE__, __extension__ __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#define error(S, ...)
#endif

#endif

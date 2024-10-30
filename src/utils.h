#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define KNRM "\033[0m"
#define KRED "\033[1;31m"
#define KGRN "\033[1;32m"
#define KBLU "\033[1;34m"
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

#define p_instr(S, ...)                                                           \
  do {                                                                         \
    fprintf(stderr, KBLU "INSTR: " KNRM S NL,##__VA_ARGS__); \
  } while (0)

#define error(S, ...)                                                          \
    do {                                                                       \
        fprintf(stderr, KRED "ERROR: " KNRM S NL, ##__VA_ARGS__); \
    } while (0)

#define success(S, ...)                                                        \
  do {                                                                         \
    fprintf(stderr, KGRN "SUCCESS: " KNRM S NL, ##__VA_ARGS__);                \
  } while (0)

#endif
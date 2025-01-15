
#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>
#include <stdlib.h>

#define EXIT_MEM_ERROR() \
    do { \
        fprintf(stderr, "Memory allocation failed: %s [%s:%d]\n", __func__, __FILE__, __LINE__); \
        exit(255); \
    } while(0)


#define EXIT_WITH_ERROR(msg, code) \
    do { \
        fprintf(stderr, "Error: %s: %s [%s:%d]\n", \
                msg, \
                __func__, __FILE__, __LINE__); \
        exit((code)); \
    } while (0)

#endif // ERROR_HANDLING_H
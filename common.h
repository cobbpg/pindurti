#ifndef _common
#define _common

#include <stdint.h>
#define UNUSED_PARAMETER(p) (void)p

typedef uint16_t WORD;
typedef uint8_t BYTE;

extern char error_msg[256];
extern FILE *logfp;

#endif


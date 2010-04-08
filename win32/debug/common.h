#include "../../common.h"

typedef struct {
	WORD read_address;
	WORD read_number;
	WORD read_base;
} DEBUG_COMMON;

extern DEBUG_COMMON dcom;

int debug_read_address(HWND hwnd);
int debug_read_number(HWND hwnd);

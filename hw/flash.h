#include "emucore.h"

#define FLASH_TYPE_83P   0x00
#define FLASH_TYPE_83PSE 0x01
#define FLASH_TYPE_84P   0x02
#define FLASH_TYPE_84PSE 0x03

#define FLASH_RESET   0x00
#define FLASH_CONSUME 0x03
#define FLASH_PROGRAM 0x13
#define FLASH_ERASEC  0x20
#define FLASH_ERASE   0x22

typedef struct {
	int flags;
	int start;
	int size;
} FLASH_PAGE;

typedef struct {
	int phase;
	BYTE *mem;
	int mask;
	BYTE pnum;
	FLASH_PAGE *pages;
} FLASH_ROM;

extern FLASH_ROM *flash;

void flash_reset(int type, BYTE *mem);
void flash_write(int adr, BYTE val);


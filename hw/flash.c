// no erase suspend/resume
// no word mode
// no status info
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flash.h"

// 83+ layout

FLASH_PAGE flash_83p[11] = {
	{ 0, 0x00000, 0x10000 },
	{ 0, 0x10000, 0x10000 },
	{ 0, 0x20000, 0x10000 },
	{ 0, 0x30000, 0x10000 },
	{ 0, 0x40000, 0x10000 },
	{ 0, 0x50000, 0x10000 },
	{ 0, 0x60000, 0x10000 },
	{ 0, 0x70000, 0x08000 },
	{ 0, 0x78000, 0x02000 },
	{ 0, 0x7a000, 0x02000 },
	{ 1, 0x7c000, 0x04000 }
};

FLASH_ROM *flash;

void flash_reset(int type, BYTE *mem) {
	flash->phase = FLASH_RESET;
	flash->mem = mem;
	switch (type) {
		case FLASH_TYPE_83P:
			flash->pages = flash_83p;
			flash->pnum = 11;
			flash->mask = 0x7ffff;
		break;
	}
}

void flash_write(int adr, BYTE val) {
	adr &= flash->mask;
	switch (flash->phase & 0x03) {
		// Unlock cycle 1
		case 0:
			if ((val == 0xaa) && ((adr & 0xfff) == 0xaaa))
				flash->phase++;
			else
				flash->phase = FLASH_RESET;
		break;
		// Unlock cycle 2
		case 1:
			if ((val == 0x55) && ((adr & 0xfff) == 0x555))
				flash->phase++;
			else
				flash->phase = FLASH_RESET;
		break;
		// Command
		case 2:
			switch (val) {
				// Chip Erase
				case 0x10:
					if ((flash->phase == FLASH_ERASE) && ((adr & 0xfff) == 0xaaa)) {
						int i;
						for (i = 0; i < flash->pnum; i++)
							if (!flash->pages[i].flags)
								memset(flash->mem + flash->pages[i].start, 0xff, flash->pages[i].size);
					}
					flash->phase = FLASH_RESET;
				break;
				// Sector Erase
				case 0x30:
					if (flash->phase == FLASH_ERASE) {
						int i;
						for (i = 0; i < flash->pnum; i++)
							if (adr < (flash->pages[i].start + flash->pages[i].size)) {
								if (!flash->pages[i].flags)
									memset(flash->mem + flash->pages[i].start, 0xff, flash->pages[i].size);
								break;
							}
					}
					flash->phase = FLASH_RESET;
				break;
				// Erase
				case 0x80:
					flash->phase = FLASH_ERASEC;
				break;
				// Read Silicon ID / Sector Protect Verify (not implemented)
				case 0x90:
					flash->phase = FLASH_CONSUME;
				break;
				// Program (reset data bits)
				case 0xa0:
					flash->phase = FLASH_PROGRAM;
				break;
			}
		break;
		// Data
		case 3:
			// Write zero bits
			if (flash->phase == FLASH_PROGRAM)
				flash->mem[adr] &= val;
			// Reset anyway
			flash->phase = FLASH_RESET;
		break;
	}
}


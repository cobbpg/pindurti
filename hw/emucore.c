#include <stdio.h>
#include <string.h>
#include "emucore.h"

EMU_CORE *emu;

// Macros for identical serialisation in both directions
#define lEMUC(x) emu->x = fgetc(fp)
#define lEMUI(x) \
	emu->x = fgetc(fp) << 24; \
	emu->x += fgetc(fp) << 16; \
	emu->x += fgetc(fp) << 8; \
	emu->x += fgetc(fp);

#define sEMUC(x) fputc(emu->x, fp)
#define sEMUI(x) \
	fputc(emu->x >> 24, fp); \
	fputc(emu->x >> 16, fp); \
	fputc(emu->x >> 8, fp); \
	fputc(emu->x, fp)

#define SEREMU(ls) \
	ls ## EMUI(stop_cnt); ls ## EMUI(stop_period); ls ## EMUI(it_num); \
	ls ## EMUC(dbus); ls ## EMUC(link_state);

// Save the current emucore state
int emu_save(FILE *fp) {
	SEREMU(s);
	return 0;
}

// Load the current emucore state (pointers must be restored elsewhere!)
int emu_load(FILE *fp) {
	SEREMU(l);
	return 0;
}


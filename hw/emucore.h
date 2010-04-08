#ifndef _emucore
#define _emucore

#include "../common.h"

typedef struct {
	int stop_cnt; // GUI display refresh counter
	int stop_period; // GUI display refresh period
	int it_num; // Number of IT counters to update
	int *it_cnt; // Pointer to current interrupt counters (needed by CPU)
	BYTE dbus; // Byte on the databus
	BYTE link_state; // Link socket state 0x00-0x03
	BYTE *partner_link; // Partner link state 0x00-0x03
} EMU_CORE;

extern EMU_CORE *emu; // Pointer to running emulator trunk

int emu_save(FILE *fp); // Save the current emucore state
int emu_load(FILE *fp); // Load the current emucore state

#endif


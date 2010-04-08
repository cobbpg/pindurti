#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "../hw/hwcore.h"
#include "debug.h"
#include "dz80.h"
#include "dtrap.h"

char *debug_values[DEBUG_VALUES];
char debug_input[DEBUG_INPUT_LEN];
int dbg_dec, dbg_hex;
int debug_active, debug_trapped;
char debug_code_bp[0x10000];

void debug_init() {
	int i;

	memset(debug_code_bp, 0, 0x10000);
	for (i = 0; i < DEBUG_VALUES; i++) {
		debug_values[i] = malloc(sizeof(char[200]));
		debug_values[i][0] = 0;
	}
	debug_trap_init();
}

void debug_deinit() {
	int i;

	for (i = 0; i < DEBUG_VALUES; i++)
		free(debug_values[i]);
}

DEBUG_INFO no_debug_info() {
	DEBUG_INFO tmp = { 0, NULL, NULL };
	return tmp;
}

void memory_page_string(char *dest, BYTE *pg, BYTE *rom, BYTE romp, BYTE *ram, BYTE ramp) {
	if ((((pg - rom) >> 14) < romp) && (pg >= rom)) sprintf(dest, "rom%02x", (pg - rom) >> 14);
	else if ((((pg - ram) >> 14) < ramp) && (pg >= ram)) sprintf(dest, "ram%02x", (pg - ram) >> 14);
	else sprintf(dest, "?");
}

int debug_read_input() {
	char *err;
	dbg_hex = strtol(debug_input, &err, 16);
	if (!debug_input[0] || err[0]) return 1;
	dbg_dec = strtol(debug_input, &err, 10);
	return 0;
}

int debug_step_instruction(int slot) {
	if (activate_calculator(slot)) return -1;
	BYTE ccodes[18] = {
		0xcd, 0xc4, 0xcc, 0xd4, 0xdc, 0xe4, 0xec, 0xf4, 0xfc, // CALL
		0xc7, 0xcf, 0xd7, 0xdf, 0xe7, 0xef, 0xf7, 0xff, // RST
		0x10 // DJNZ
	};
	BYTE rcodes[8] = {
		0xb0, 0xb1, 0xb2, 0xb3, 0xb8, 0xb9, 0xba, 0xbb // repeats
	};
	int i;
	BYTE oc = z80_acc(R_PC);
	for (i = 0; i < 18; i++) if (oc == ccodes[i]) return 1;
	if (oc == 0xdd || oc == 0xfd) {
		oc = z80_acc(R_PC + 1);
		for (i = 0; i < 18; i++) if (oc == ccodes[i]) return 1;
	} else if (oc == 0xed) {
		oc = z80_acc(R_PC + 1);
		for (i = 0; i < 8; i++) if (oc == rcodes[i]) return 1;
	}
	return 0;
}

void calculator_step(int slot) {
	if (activate_calculator(slot)) return;
	int rtime = 1;
	int da = debug_active;
	if (z80->hlt)
		switch (calc[slot].rom_ver & FILE_MODEL_MASK) {
			case FILE_MODEL_82:
			case FILE_MODEL_82b: rtime = ti_82->it_next - ti_82->it_cnt + 1; break;
			case FILE_MODEL_83: rtime = ti_83->it_next - ti_83->it_cnt + 1; break;
			case FILE_MODEL_83P: rtime = ti_83p->it_next - ti_83p->it_cnt + 1; break;
		}
	debug_active = 0;
	calculator_run_timed(rtime);
	debug_active = da;
}

void calculator_step_over(int slot) {
	switch (debug_step_instruction(slot)) {
	case 0:
		calculator_step(slot);
		break;
	case 1: {
		int da = debug_active;
		char dcbp[5];
		WORD adr = R_PC + opcode_name(R_PC);
		int i;
		debug_active |= DEBUG_ACTIVE_CODEBP;
		for (i = 0; i < 5; i++) {
			dcbp[i] = debug_code_bp[(adr + i) & 0xffff];
			debug_code_bp[(adr + i) & 0xffff] |= 1;
		}
		calculator_run_timed(10000000);
		debug_trapped = 0;
		debug_active = da;
		for (i = 0; i < 5; i++)
			debug_code_bp[(adr + i) & 0xffff] = dcbp[i];
		break;
	}
	}
}

void debug_toggle_code_breakpoint(WORD adr) {
	int i, a2;
	a2 = adr + opcode_name(adr);
	for (i = adr; i < a2; i++)
		debug_code_bp[i & 0xffff] ^= 1;
	debug_active &= ~DEBUG_ACTIVE_CODEBP;
	for (i = 0; i < 0x10000; i++)
		if (debug_code_bp[i]) {
			debug_active |= DEBUG_ACTIVE_CODEBP;
			break;
		}
}

void debug_set_code_breakpoint(WORD adr, int state) {
	int i;
	debug_code_bp[adr] = state;
	debug_active &= ~DEBUG_ACTIVE_CODEBP;
	for (i = 0; i < 0x10000; i++)
		if (debug_code_bp[i]) {
			debug_active |= DEBUG_ACTIVE_CODEBP;
			break;
		}
}

void debug_clear_code_breakpoints() {
	memset(debug_code_bp, 0, sizeof(debug_code_bp));
	debug_active &= ~DEBUG_ACTIVE_CODEBP;
}

void debug_check() {
	if ((debug_active & DEBUG_ACTIVE_CODEBP) && debug_code_bp[R_PC])
		debug_trapped = 1;
}


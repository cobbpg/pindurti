#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../hw/z80.h"
#include "dtrap.h"

DEBUG_LOG debug_log;

void trap_allocate_next_begin(int l) {
	if (debug_log.list[debug_log.next] != NULL) free(debug_log.list[debug_log.next]);
	debug_log.list[debug_log.next] = malloc(l);
}

void trap_allocate_next_end() {
	debug_log.llen[debug_log.next] = strlen(debug_log.list[debug_log.next]);
	debug_log.next = (debug_log.next + 1) % DEBUG_LOG_LIST_LENGTH;
}

void trap_dump_register_contents() {
	static const char *dformat = "\
pc=%04x sp=%04x af=%04x  bc=%04x  de=%04x  hl=%04x\r\n\
ix=%04x iy=%04x af'=%04x bc'=%04x de'=%04x hl'=%04x\r\n\
r=%02x i=%02x im=%d halt=%d\r\n";

	trap_allocate_next_begin(strlen(dformat));
	sprintf(debug_log.list[debug_log.next], dformat,
		R_PC, R_SP, R_AF, R_BC, R_DE, R_HL, R_IX, R_IY, R_AFS, R_BCS, R_DES, R_HLS,
		R_R, R_I, z80->im, z80->hlt);
	trap_allocate_next_end();
}

void trap_printf() {
#define BUFS 2000
	char buf[BUFS + 1];
	int i;
	WORD ptr = R_HL;
	BYTE c;

	for (i = 0; i < BUFS; i++) {
		c = z80_acc(ptr++);
		if (c != '\\') {
			buf[i] = c;
			if (!c) break;
			continue;
		}
		c = z80_acc(ptr++);
		switch (c) {
		case '\\': buf[i] = '\\'; break;
		case '!': if (i < BUFS - 2) { buf[i++] = '\r'; buf[i] = '\n'; } break;
		case 'a': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_A); i++; } break;
		case 'f': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_F); i++; } break;
		case 'b': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_B); i++; } break;
		case 'c': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_C); i++; } break;
		case 'd': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_D); i++; } break;
		case 'e': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_E); i++; } break;
		case 'h': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_H); i++; } break;
		case 'l': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_L); i++; } break;
		case 'i': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_I); i++; } break;
		case 'r': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_R); i++; } break;
		case 'x': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_IXL); i++; } break;
		case 'y': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_IXH); i++; } break;
		case 'w': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_IYL); i++; } break;
		case 'z': if (i < BUFS - 2) { sprintf(buf + i, "%02x", R_IYH); i++; } break;
		case 'A': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_AF); i += 3; } break;
		case 'B': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_BC); i += 3; } break;
		case 'D': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_DE); i += 3; } break;
		case 'H': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_HL); i += 3; } break;
		case 'X': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_IX); i += 3; } break;
		case 'Y': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_IY); i += 3; } break;
		case 'P': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_PC); i += 3; } break;
		case 'S': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_SP); i += 3; } break;
		case 'F': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_AFS); i += 3; } break;
		case 'C': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_BCS); i += 3; } break;
		case 'E': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_DES); i += 3; } break;
		case 'L': if (i < BUFS - 4) { sprintf(buf + i, "%04x", R_HLS); i += 3; } break;
		case 'm': if (i < BUFS - 2) { sprintf(buf + i, "%02x", z80_acc(R_BC)); i++; } break;
		case 'n': if (i < BUFS - 2) { sprintf(buf + i, "%02x", z80_acc(R_DE)); i++; } break;
		case 'o': if (i < BUFS - 2) { sprintf(buf + i, "%02x", z80_acc(R_IX)); i++; } break;
		case 'q': if (i < BUFS - 2) { sprintf(buf + i, "%02x", z80_acc(R_IY)); i++; } break;
		}
	}
	buf[i] = 0;
	trap_allocate_next_begin(strlen(buf));
	strcpy(debug_log.list[debug_log.next], buf);
	trap_allocate_next_end();
}

void debug_trap_init() {
	int i;

	for (i = 0; i < DEBUG_LOG_LIST_LENGTH; i++) {
		debug_log.list[i] = NULL;
		debug_log.llen[i] = 0;
	}
	debug_log.next = 0;
}

void execute_trap(BYTE id) {
	switch (id) {
	case 0x00: trap_dump_register_contents(); break;
	case 0x01: trap_printf(); break;
	}
}

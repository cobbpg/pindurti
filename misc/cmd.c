#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MINGW32__
#include <fcntl.h>
#endif
#include "cmd.h"
#include "../common.h"
#include "../hw/hwcore.h"
#include "../debug/dz80.h"
#include "../debug/dlcd.h"
#include "../debug/dti82.h"
#include "../debug/dti83.h"
#include "../debug/dti83p.h"

#define PRINT_OK printf("OK\n")
#define PRINT_FU printf("FU\n")
#define PRINT_ERROR(fs, ...) printf("Error: " fs "\n", ##__VA_ARGS__)
#define PRINT_INFO(fs, ...) printf("Info: " fs "\n", ##__VA_ARGS__)
#define ERR_INACTIVE "inactive slot"
#define ERR_CREATE "must create slot first"
#define ERR_INVALID_SLOT "invalid slot number (must be between 0 and %d)"
#define ERR_INVALID_KEY "invalid key name"
#define ERR_INVALID_DUMP "invalid property name"
#define ERR_INVALID_ADDRESS "invalid address"
#define INF_BREAKPOINT_ENCOUNTERED "breakpoint encountered"

const CMD_CALL cmds[] = {
	{ "send-file", cmd_send_file },
	{ "activate-slot", cmd_activate_slot },
	{ "run", cmd_run },
	{ "step", cmd_step },
	{ "draw-screen-bw", cmd_screen_bw },
	{ "draw-screen-gs", cmd_screen_gs },
	{ "key-down", cmd_key_down },
	{ "key-up", cmd_key_up },
	{ "reset-calc", cmd_reset_calc },
	{ "dump-state", cmd_dump_state },
	{ "set-breakpoint code", cmd_set_code_bp },
	{ "remove-breakpoint code", cmd_remove_code_bp },
	{ "", NULL }
};

// Running a textual command
int run_command(char *cmd) {
	int i, ret = 0;

	for (i = 0; cmds[i].cmd != NULL; i++) {
		int len = strlen(cmds[i].name);
		if (!strncmp(cmds[i].name, cmd, len)) {
			ret = cmds[i].cmd(cmd + len);
			break;
		}
	}

	if (cmds[i].cmd == NULL) PRINT_FU;
	fflush(stdout);
	return ret;
}

// Interpreting a keyboard command
int cmd_key_set(char *key, int state) {
	static struct {
		const char* name;
		BYTE id;
	} kmap[] = {
		{ "down",   0x00 },
		{ "left",   0x01 },
		{ "right",  0x02 },
		{ "up",     0x03 },
		{ "enter",  0x10 },
		{ "+",      0x11 },
		{ "-",      0x12 },
		{ "*",      0x13 },
		{ "/",      0x14 },
		{ "^",      0x15 },
		{ "clear",  0x16 },
		{ "(-)",    0x20 },
		{ "3",      0x21 },
		{ "6",      0x22 },
		{ "9",      0x23 },
		{ ")",      0x24 },
		{ "tan",    0x25 },
		{ "vars",   0x26 },
		{ ".",      0x30 },
		{ "2",      0x31 },
		{ "5",      0x32 },
		{ "8",      0x33 },
		{ "(",      0x34 },
		{ "cos",    0x35 },
		{ "prgm",   0x36 },
		{ "stat",   0x37 },
		{ "0",      0x40 },
		{ "1",      0x41 },
		{ "4",      0x42 },
		{ "7",      0x43 },
		{ ",",      0x44 },
		{ "sin",    0x45 },
		{ "matrx",  0x46 },
		{ "apps",   0x46 },
		{ "x",      0x47 },
		{ "sto",    0x51 },
		{ "ln",     0x52 },
		{ "log",    0x53 },
		{ "x^2",    0x54 },
		{ "x^-1",   0x55 },
		{ "math",   0x56 },
		{ "alpha",  0x57 },
		{ "graph",  0x60 },
		{ "trace",  0x61 },
		{ "zoom",   0x62 },
		{ "window", 0x63 },
		{ "y=",     0x64 },
		{ "2nd",    0x65 },
		{ "mode",   0x66 },
		{ "del",    0x67 },
		{ "on",     0x80 },
		{ "",       0xff }
	};
	int i;
	size_t l = strlen(key) + 1;

	for (i = 0; kmap[i].name[0]; i++) {
		if (!strncmp(kmap[i].name, key, l)) {
			handle_key(kmap[i].id, (state & 0x01) | 0x02);
			return 0;
		}
	}

	return 1;
}

// Skipping whitespace
void cmdarg_ws(char **ptr) {
	while (**ptr && **ptr <= ' ') (*ptr)++;
}

// Reading a decimal number
int cmdarg_dec(char **ptr) {
	cmdarg_ws(ptr);
	if (!**ptr) return 0;
	char *err;
	int val = strtol(*ptr, &err, 0);
	*ptr = err;
	return val;
}

// Sending a file to a given slot
CMD_FN(send_file) {
	char *ccmd = cmd;
	int slot = cmdarg_dec(&ccmd);
	if ((slot < 0) || (slot >= MAX_CALC)) {
		PRINT_ERROR(ERR_INVALID_SLOT, MAX_CALC - 1);
		return 1;
	}
	cmdarg_ws(&ccmd);
	if (process_file(ccmd, slot)) {
		printf("Error: %s\n", error_msg);
		return 1;
	}
	PRINT_OK;
	return 0;
}

// Activating a slot
CMD_FN(activate_slot) {
	char *ccmd = cmd;
	int slot = cmdarg_dec(&ccmd);
	if ((slot < 0) || (slot >= MAX_CALC)) {
		PRINT_ERROR(ERR_INVALID_SLOT, MAX_CALC - 1);
		return 1;
	}
	if (activate_calculator(slot)) {
		PRINT_ERROR(ERR_CREATE);
		return 1;
	}
	PRINT_OK;
	return 0;
}

// Running for a given time
CMD_FN(run) {
	char *ccmd = cmd;
	int cc = cmdarg_dec(&ccmd);
	if (running_calculator != -1) {
		calculator_run_timed(cc);
		if (debug_trapped) {
			debug_trapped = 0;
			PRINT_INFO(INF_BREAKPOINT_ENCOUNTERED);
		}
		PRINT_OK;
		return 0;
	} else {
		PRINT_ERROR(ERR_INACTIVE);
		return 1;
	}
}

// Stepping a given number of instructions
CMD_FN(step) {
	char *ccmd = cmd;
	int ni = cmdarg_dec(&ccmd);
	if (running_calculator != -1) {
		int i;
		if (ni == 0) ni = 1;
		for (i = 0; i < ni; i++)
			calculator_step(running_calculator);
		PRINT_OK;
		return 0;
	} else {
		PRINT_ERROR(ERR_INACTIVE);
		return 1;
	}
}

// Output black and white screen data (LCD memory contents)
CMD_FN(screen_bw) {
	UNUSED_PARAMETER(cmd);
	int i, j;

	if (running_calculator != -1) {
		PRINT_OK;
		printf("%d%d\n", calculator_powered(), lcd->on);
		for (i = 0; i < 64; i++)
			for (j = 0; j < 96; j++)
				putchar(lcd->dat[i * 120 + j] * 255);
		return 0;
	} else {
		PRINT_ERROR(ERR_INACTIVE);
		return 1;
	}
}

// Output greyscale screen data (only updated 25 times a second)
CMD_FN(screen_gs) {
	UNUSED_PARAMETER(cmd);
	int i, j;

	if (running_calculator != -1) {
		PRINT_OK;
		printf("%d%d\n", calculator_powered(), lcd->on);
		#ifdef __MINGW32__
		fflush(stdout);
		_setmode(_fileno(stdout), _O_BINARY);
		#endif
		for (i = 0; i < 64; i++)
			for (j = 0; j < 96; j++)
				putchar(lcd->scr[i * 120 + j] >> 8);
		#ifdef __MINGW32__
		fflush(stdout);
		_setmode(_fileno(stdout), _O_TEXT);
		#endif
		return 0;
	} else {
		PRINT_ERROR(ERR_INACTIVE);
		return 1;
	}
}

// Press a key
CMD_FN(key_down) {
	char *ccmd = cmd;
	cmdarg_ws(&ccmd);
	if (running_calculator == -1) {
		PRINT_ERROR(ERR_INACTIVE);
		return 1;
	}
	if (cmd_key_set(ccmd, 1)) {
		PRINT_ERROR(ERR_INVALID_KEY);
		return 1;
	} else {
		PRINT_OK;
		return 0;
	}
}

// Release a key
CMD_FN(key_up) {
	char *ccmd = cmd;
	cmdarg_ws(&ccmd);
	if (running_calculator == -1) {
		PRINT_ERROR(ERR_INACTIVE);
		return 1;
	}
	if (cmd_key_set(ccmd, 0)) {
		PRINT_ERROR(ERR_INVALID_KEY);
		return 1;
	} else {
		PRINT_OK;
		return 0;
	}
}

// Resetting the calc in the current slot
CMD_FN(reset_calc) {
	UNUSED_PARAMETER(cmd);
	switch (calc[running_calculator].rom_ver & FILE_PROT_MASK) {
		case FILE_PROT_82: ti_82_reset(); break;
		case FILE_PROT_83: ti_83_reset(); break;
		case FILE_PROT_83P: ti_83p_reset(); break;
		default:
			PRINT_ERROR(ERR_INACTIVE);
			return 1;
	}
	PRINT_OK;
	return 0;
}

// Running a key-value command
CMD_FN(dump_state) {
	static struct {
		const char* name;
		DEBUG_INFO_FN fn82, fn83, fn83p;
	} dmap[] = {
		{ "lcd physics",  lcd_debug_physical,    lcd_debug_physical,    lcd_debug_physical },
		{ "lcd software", lcd_debug_software,    lcd_debug_software,    lcd_debug_software },
		{ "model",        ti_82_debug_info,      ti_83_debug_info,      ti_83p_debug_info },
		{ "time",         ti_82_debug_time,      ti_83_debug_time,      ti_83p_debug_time },
		{ "interrupt",    ti_82_debug_interrupt, ti_83_debug_interrupt, ti_83p_debug_interrupt },
		{ "pager",        ti_82_debug_memory,    ti_83_debug_memory,    ti_83p_debug_memory },
		{ "keyboard",     ti_82_debug_keys,      ti_83_debug_keys,      ti_83p_debug_keys },
		{ "link",         ti_82_debug_link,      ti_83_debug_link,      ti_83p_debug_link },
		{ "",             NULL, NULL, NULL }
	};

	char *ccmd = cmd;
	cmdarg_ws(&ccmd);

	int i, j;
	size_t l = strlen(ccmd) + 1;

	for (i = 0; dmap[i].name[0]; i++) {
		if (!strncmp(dmap[i].name, ccmd, l)) {
			DEBUG_INFO di;
			switch (calc[running_calculator].rom_ver & FILE_PROT_MASK) {
				case FILE_PROT_82: di = dmap[i].fn82(); break;
				case FILE_PROT_83: di = dmap[i].fn83(); break;
				case FILE_PROT_83P: di = dmap[i].fn83p(); break;
				default:
					PRINT_ERROR(ERR_INACTIVE);
					return 1;
			}
			PRINT_OK;
			printf("%d\n", di.count);
			for (j = 0; j < di.count; j++) printf("%s: %s\n", di.key[j], di.value[j]);
			return 0;
		}
	}

	if (!strncmp("cpu", ccmd, l)) {
		PRINT_OK;
		printf("af=%04x\nbc=%04x\nde=%04x\nhl=%04x\nix=%04x\niy=%04x\npc=%04x\nsp=%04x\n\
af'=%04x\nbc'=%04x\nde'=%04x\nhl'=%04x\nr=%02x\ni=%02x\nim=%d\nhalt=%d\n",
			R_AF, R_BC, R_DE, R_HL, R_IX, R_IY, R_PC, R_SP,
			R_AFS, R_BCS, R_DES, R_HLS, R_R, R_I, z80->im, z80->hlt);
		return 0;
	}

	if (!strncmp("memory", ccmd, l)) {
		PRINT_OK;
		#ifdef __MINGW32__
		fflush(stdout);
		_setmode(_fileno(stdout), _O_BINARY);
		#endif
		for (i = 0; i < 0x10000; i++)
			putchar(z80_acc(i));
		#ifdef __MINGW32__
		fflush(stdout);
		_setmode(_fileno(stdout), _O_TEXT);
		#endif
		return 0;
	}

	PRINT_ERROR(ERR_INVALID_DUMP);
	return 1;
}

CMD_FN(set_code_bp) {
	char *ccmd = cmd;
	int adr = cmdarg_dec(&ccmd);
	if ((adr < 0) || (adr > 0xffff)) {
		PRINT_ERROR(ERR_INVALID_ADDRESS);
		return 1;
	}
	debug_set_code_breakpoint(adr, 1);
	PRINT_OK;
	return 0;
}

CMD_FN(remove_code_bp) {
	char *ccmd = cmd;
	int adr = cmdarg_dec(&ccmd);
	if ((adr < -1) || (adr > 0xffff)) {
		PRINT_ERROR(ERR_INVALID_ADDRESS);
		return 1;
	}
	if (adr == -1)
		debug_clear_code_breakpoints();
	else
		debug_set_code_breakpoint(adr, 0);
	PRINT_OK;
	return 0;
}

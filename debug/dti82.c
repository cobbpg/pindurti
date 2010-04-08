#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../hw/hwcore.h"
#include "debug.h"
#include "dti82.h"

#define ITSPEED00 (6000000 * (3 + 0) / 1620)
#define ITSPEED02 (6000000 * (3 + 4) / 1620)
#define ITSPEED04 (6000000 * (3 + 8) / 1620)
#define ITSPEED06 (6000000 * (3 + 12) / 1620)
#define ITSPEED10 (int)(ITSPEED00 * 0.9)
#define ITSPEED12 (int)(ITSPEED02 * 0.9)
#define ITSPEED14 (int)(ITSPEED04 * 0.9)
#define ITSPEED16 (int)(ITSPEED06 * 0.9)

DEBUG_QUERY debug_ti_82[] = {
	{ "Model information", ti_82_debug_info },
	{ "Emulation clock", ti_82_debug_time },
	{ "Interrupt state", ti_82_debug_interrupt },
	{ "Memory", ti_82_debug_memory },
	{ "Keyboard state", ti_82_debug_keys },
	{ "Link state", ti_82_debug_link }
};

char *debug_ti_82_info[] = {
	"Model"
};

char *debug_ti_82_time[] = {
	"Clock"
};

char *debug_ti_82_interrupt[] = {
	"Enabled", "Frequency", "Mask", "Active", "Next"
};

char *debug_ti_82_keys[] = {
	"Group 0", "Group 1", "Group 2", "Group 3",
	"Group 4", "Group 5", "Group 6", "On key"
};

char *debug_ti_82_memory[] = {
	"Mapping", "Pager", "Port 2", "Page 0", "Page 1", "Page 2", "Page 3"
};

char *debug_ti_82_link[] = {
	"Link state", "Partner link", "Direction"
};

DEBUG_INFO ti_82_debug_info() {
	DEBUG_INFO tmp = { 1, debug_ti_82_info, debug_values };
	char *v = rom_version_name(calc[running_calculator].rom_ver);
	sprintf(debug_values[0], "%s (%s)", v ? v : "?", ti_82->model ? "new" : "old");
	return tmp;
}

DEBUG_INFO ti_82_debug_time() {
	DEBUG_INFO tmp = { 1, debug_ti_82_time, debug_values };
	sprintf(debug_values[0], "%d", emu->stop_cnt);
	return tmp;
}

DEBUG_INFO ti_82_debug_interrupt() {
	DEBUG_INFO tmp = { 5, debug_ti_82_interrupt, debug_values };
	char nextit[4] = "";
	int nexttime = 0;
	sprintf(debug_values[0], "%s", (z80->iff1) ? ((z80->ie) ? "not yet" : "yes") : "no");
	switch (ti_82->it_times[3]) {
		case ITSPEED00: nexttime = 0x00; break;
		case ITSPEED02: nexttime = 0x02; break;
		case ITSPEED04: nexttime = 0x04; break;
		case ITSPEED06: nexttime = 0x06; break;
		case ITSPEED10: nexttime = 0x10; break;
		case ITSPEED12: nexttime = 0x12; break;
		case ITSPEED14: nexttime = 0x14; break;
		case ITSPEED16: nexttime = 0x16; break;
	}
	sprintf(debug_values[1], "%5.2f x %d Hz (%02x)", 6000000.0 / ti_82->it_times[3], (ti_82->it_mask >> 1) & 0x03, nexttime);
	sprintf(debug_values[2], "on=%c t1=%c t2=%c power=%c",
		(ti_82->it_mask & 0x01) + '0',
		((ti_82->it_mask >> 1) & 0x01) + '0',
		((ti_82->it_mask >> 2) & 0x01) + '0',
		((ti_82->it_mask >> 3)) + '0');
	sprintf(debug_values[3], "on=%c t1=%c t2=%c",
		(ti_82->it_active & 0x01) + '0',
		((ti_82->it_active >> 1) & 0x01) + '0',
		((ti_82->it_active >> 2) & 0x01) + '0');
	if (!(ti_82->it_mask & 0x06)) {
		sprintf(debug_values[4], "none");
	} else {
		switch (ti_82->it_mask & 0x06) {
			case 2:
				sprintf(nextit, "t1");
				nexttime = ti_82->it_times[2] - ti_82->it_cnt;
			break;
			case 4:
				sprintf(nextit, "t2%c", (ti_82->it_state == 1) ? 'b' : 'a');
				nexttime = ti_82->it_times[(ti_82->it_state == 1)] - ti_82->it_cnt;
			break;
			case 6:
				switch (ti_82->it_state) {
					case 0: case 3: sprintf(nextit, "t2a"); break;
					case 1: sprintf(nextit, "t2b"); break;
					case 2: sprintf(nextit, "t1"); break;
				}
				nexttime = ti_82->it_times[ti_82->it_state % 3] - ti_82->it_cnt;
			break;
		}
		if (nexttime < 0) nexttime += ti_82->it_times[3];
		sprintf(debug_values[4], "%s in %d cc", nextit, nexttime);
	}
	return tmp;
}

DEBUG_INFO ti_82_debug_memory() {
	DEBUG_INFO tmp = { 7, debug_ti_82_memory, debug_values };
	int i;
	sprintf(debug_values[0], "%d", ti_82->mmap);
	sprintf(debug_values[1], "%02x", ti_82->rom_page);
	sprintf(debug_values[2], "%02x", ti_82->port_02);
	for (i = 0; i < 4; i++)
		memory_page_string(debug_values[i + 3], ti_82->page[i], ti_82->rom, 0x08, ti_82->ram, 0x02);
	return tmp;
}

DEBUG_INFO ti_82_debug_keys() {
	DEBUG_INFO tmp = { 8, debug_ti_82_keys, debug_values };
	int i, j;
	for (i = 0; i < 7; i++) {
		for (j = 0; j < 8; j++)
			debug_values[i][j] = ((ti_82->key_state[i] >> (7 - j)) & 1) + '0';
		debug_values[i][j] = 0;
	}
	sprintf(debug_values[7], "%s", ti_82->on_key ? "down" : "up");
	return tmp;
}

DEBUG_INFO ti_82_debug_link() {
	DEBUG_INFO tmp = { 3, debug_ti_82_link, debug_values };
	sprintf(debug_values[0], "%d%d", ti_82->link_state >> 1, ti_82->link_state & 1);
	sprintf(debug_values[1], "%d%d", *ti_82->partner_link >> 1, *ti_82->partner_link & 1);
	sprintf(debug_values[2], "%02x", ti_82->link_mask);
	return tmp;
}

void ti_82_enter_it_ie() {
	z80->iff1 ^= 1;
	z80->iff2 = z80->iff1;
}

void ti_82_enter_it_freq() {
	switch (ti_82->it_times[3]) {
		case ITSPEED00: ti_82->it_times[3] = ITSPEED10; break;
		case ITSPEED02: ti_82->it_times[3] = ITSPEED12; break;
		case ITSPEED04: ti_82->it_times[3] = ITSPEED14; break;
		case ITSPEED06: ti_82->it_times[3] = ITSPEED16; break;
		case ITSPEED10: ti_82->it_times[3] = ITSPEED06; break;
		case ITSPEED12: ti_82->it_times[3] = ITSPEED00; break;
		case ITSPEED14: ti_82->it_times[3] = ITSPEED02; break;
		case ITSPEED16: ti_82->it_times[3] = ITSPEED04; break;
	}
	ti_82->it_times[0] = ti_82->it_times[3] >> 1;
	ti_82->it_times[1] = ti_82->it_times[0] + 1600;
	ti_82->it_times[2] = ti_82->it_times[1] + 1200;
	ti_82->it_next = ti_82->it_times[ti_82->it_state];
}

void ti_82_enter_it_mask_on() { ti_82->it_mask ^= 0x01; }
void ti_82_enter_it_mask_t1() { ti_82->it_mask ^= 0x02; }
void ti_82_enter_it_mask_t2() { ti_82->it_mask ^= 0x04; }
void ti_82_enter_it_mask_power() { ti_82->it_mask ^= 0x08; }

void ti_82_enter_it_active_on() { ti_82->it_active ^= 0x01; }
void ti_82_enter_it_active_t1() { ti_82->it_active ^= 0x02; }
void ti_82_enter_it_active_t2() { ti_82->it_active ^= 0x04; }

void ti_82_enter_mem_map() { ti_82->mmap ^= 1; ti_82_swap_rom_page(ti_82->rom_page); }
void ti_82_enter_rom_page() {
	if (debug_read_input()) return;
	ti_82_out(0x02, (ti_82->port_02 & 0xb8) | ((dbg_hex & 0x10) << 2) | (dbg_hex & 0x07));
}
void ti_82_enter_port_02() {
	if (debug_read_input()) return;
	ti_82_out(0x02, dbg_hex);
}

void ti_82_enter_on_key() { if (ti_82->on_key ^= 1) ti_82->it_active |= ti_82->it_mask & 0x01; }

void ti_82_enter_key_00() { ti_82->key_state[0] ^= 0x01; }
void ti_82_enter_key_01() { ti_82->key_state[0] ^= 0x02; }
void ti_82_enter_key_02() { ti_82->key_state[0] ^= 0x04; }
void ti_82_enter_key_03() { ti_82->key_state[0] ^= 0x08; }
void ti_82_enter_key_04() { ti_82->key_state[0] ^= 0x10; }
void ti_82_enter_key_05() { ti_82->key_state[0] ^= 0x20; }
void ti_82_enter_key_06() { ti_82->key_state[0] ^= 0x40; }
void ti_82_enter_key_07() { ti_82->key_state[0] ^= 0x80; }

void ti_82_enter_key_10() { ti_82->key_state[1] ^= 0x01; }
void ti_82_enter_key_11() { ti_82->key_state[1] ^= 0x02; }
void ti_82_enter_key_12() { ti_82->key_state[1] ^= 0x04; }
void ti_82_enter_key_13() { ti_82->key_state[1] ^= 0x08; }
void ti_82_enter_key_14() { ti_82->key_state[1] ^= 0x10; }
void ti_82_enter_key_15() { ti_82->key_state[1] ^= 0x20; }
void ti_82_enter_key_16() { ti_82->key_state[1] ^= 0x40; }
void ti_82_enter_key_17() { ti_82->key_state[1] ^= 0x80; }

void ti_82_enter_key_20() { ti_82->key_state[2] ^= 0x01; }
void ti_82_enter_key_21() { ti_82->key_state[2] ^= 0x02; }
void ti_82_enter_key_22() { ti_82->key_state[2] ^= 0x04; }
void ti_82_enter_key_23() { ti_82->key_state[2] ^= 0x08; }
void ti_82_enter_key_24() { ti_82->key_state[2] ^= 0x10; }
void ti_82_enter_key_25() { ti_82->key_state[2] ^= 0x20; }
void ti_82_enter_key_26() { ti_82->key_state[2] ^= 0x40; }
void ti_82_enter_key_27() { ti_82->key_state[2] ^= 0x80; }

void ti_82_enter_key_30() { ti_82->key_state[3] ^= 0x01; }
void ti_82_enter_key_31() { ti_82->key_state[3] ^= 0x02; }
void ti_82_enter_key_32() { ti_82->key_state[3] ^= 0x04; }
void ti_82_enter_key_33() { ti_82->key_state[3] ^= 0x08; }
void ti_82_enter_key_34() { ti_82->key_state[3] ^= 0x10; }
void ti_82_enter_key_35() { ti_82->key_state[3] ^= 0x20; }
void ti_82_enter_key_36() { ti_82->key_state[3] ^= 0x40; }
void ti_82_enter_key_37() { ti_82->key_state[3] ^= 0x80; }

void ti_82_enter_key_40() { ti_82->key_state[4] ^= 0x01; }
void ti_82_enter_key_41() { ti_82->key_state[4] ^= 0x02; }
void ti_82_enter_key_42() { ti_82->key_state[4] ^= 0x04; }
void ti_82_enter_key_43() { ti_82->key_state[4] ^= 0x08; }
void ti_82_enter_key_44() { ti_82->key_state[4] ^= 0x10; }
void ti_82_enter_key_45() { ti_82->key_state[4] ^= 0x20; }
void ti_82_enter_key_46() { ti_82->key_state[4] ^= 0x40; }
void ti_82_enter_key_47() { ti_82->key_state[4] ^= 0x80; }

void ti_82_enter_key_50() { ti_82->key_state[5] ^= 0x01; }
void ti_82_enter_key_51() { ti_82->key_state[5] ^= 0x02; }
void ti_82_enter_key_52() { ti_82->key_state[5] ^= 0x04; }
void ti_82_enter_key_53() { ti_82->key_state[5] ^= 0x08; }
void ti_82_enter_key_54() { ti_82->key_state[5] ^= 0x10; }
void ti_82_enter_key_55() { ti_82->key_state[5] ^= 0x20; }
void ti_82_enter_key_56() { ti_82->key_state[5] ^= 0x40; }
void ti_82_enter_key_57() { ti_82->key_state[5] ^= 0x80; }

void ti_82_enter_key_60() { ti_82->key_state[6] ^= 0x01; }
void ti_82_enter_key_61() { ti_82->key_state[6] ^= 0x02; }
void ti_82_enter_key_62() { ti_82->key_state[6] ^= 0x04; }
void ti_82_enter_key_63() { ti_82->key_state[6] ^= 0x08; }
void ti_82_enter_key_64() { ti_82->key_state[6] ^= 0x10; }
void ti_82_enter_key_65() { ti_82->key_state[6] ^= 0x20; }
void ti_82_enter_key_66() { ti_82->key_state[6] ^= 0x40; }
void ti_82_enter_key_67() { ti_82->key_state[6] ^= 0x80; }


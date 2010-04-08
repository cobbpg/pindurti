#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../hw/hwcore.h"
#include "debug.h"
#include "dti83p.h"

#define ITSPEED00 (6000000 * (3 + 0) / 1620)
#define ITSPEED02 (6000000 * (3 + 4) / 1620)
#define ITSPEED04 (6000000 * (3 + 8) / 1620)
#define ITSPEED06 (6000000 * (3 + 12) / 1620)

DEBUG_QUERY debug_ti_83p[] = {
	{ "Model information", ti_83p_debug_info },
	{ "Emulation clock", ti_83p_debug_time },
	{ "Interrupt state", ti_83p_debug_interrupt },
	{ "Memory", ti_83p_debug_memory },
	{ "Keyboard state", ti_83p_debug_keys },
	{ "Link state", ti_83p_debug_link }
};

char *debug_ti_83p_info[] = {
	"Model"
};

char *debug_ti_83p_time[] = {
	"Clock"
};

char *debug_ti_83p_interrupt[] = {
	"Enabled", "Frequency", "Mask", "Active", "Next"
};

char *debug_ti_83p_keys[] = {
	"Group 0", "Group 1", "Group 2", "Group 3",
	"Group 4", "Group 5", "Group 6", "On key"
};

char *debug_ti_83p_memory[] = {
	"Mapping", "Bank A", "Bank B", "Page 0", "Page 1", "Page 2", "Page 3"
};

char *debug_ti_83p_link[] = {
	"Link state", "Partner link"
};

DEBUG_INFO ti_83p_debug_info() {
	DEBUG_INFO tmp = { 1, debug_ti_83p_info, debug_values };
	char *v = rom_version_name(calc[running_calculator].rom_ver);
	sprintf(debug_values[0], "%s", v ? v : "?");
	return tmp;
}

DEBUG_INFO ti_83p_debug_time() {
	DEBUG_INFO tmp = { 1, debug_ti_83p_time, debug_values };
	sprintf(debug_values[0], "%d", emu->stop_cnt);
	return tmp;
}

DEBUG_INFO ti_83p_debug_interrupt() {
	DEBUG_INFO tmp = { 5, debug_ti_83p_interrupt, debug_values };
	char nextit[4] = "";
	int nexttime = 0;
	sprintf(debug_values[0], "%s", (z80->iff1) ? ((z80->ie) ? "not yet" : "yes") : "no");
	switch (ti_83p->it_times[3]) {
		case ITSPEED00: nexttime = 0x00; break;
		case ITSPEED02: nexttime = 0x02; break;
		case ITSPEED04: nexttime = 0x04; break;
		case ITSPEED06: nexttime = 0x06; break;
	}
	sprintf(debug_values[1], "%5.2f x %d Hz (%02x)", 6000000.0 / ti_83p->it_times[3], (ti_83p->it_mask >> 1) & 0x03, nexttime);
	sprintf(debug_values[2], "on=%c t1=%c t2=%c power=%c",
		(ti_83p->it_mask & 0x01) + '0',
		((ti_83p->it_mask >> 1) & 0x01) + '0',
		((ti_83p->it_mask >> 2) & 0x01) + '0',
		((ti_83p->it_mask >> 3)) + '0');
	sprintf(debug_values[3], "on=%c t1=%c t2=%c",
		(ti_83p->it_active & 0x01) + '0',
		((ti_83p->it_active >> 1) & 0x01) + '0',
		((ti_83p->it_active >> 2) & 0x01) + '0');
	if (!(ti_83p->it_mask & 0x06)) {
		sprintf(debug_values[4], "none");
	} else {
		switch (ti_83p->it_mask & 0x06) {
			case 2:
				sprintf(nextit, "t1");
				nexttime = ti_83p->it_times[2] - ti_83p->it_cnt;
			break;
			case 4:
				sprintf(nextit, "t2%c", (ti_83p->it_state == 1) ? 'b' : 'a');
				nexttime = ti_83p->it_times[(ti_83p->it_state == 1)] - ti_83p->it_cnt;
			break;
			case 6:
				switch (ti_83p->it_state) {
					case 0: case 3: sprintf(nextit, "t2a"); break;
					case 1: sprintf(nextit, "t2b"); break;
					case 2: sprintf(nextit, "t1"); break;
				}
				nexttime = ti_83p->it_times[ti_83p->it_state % 3] - ti_83p->it_cnt;
			break;
		}
		if (nexttime < 0) nexttime += ti_83p->it_times[3];
		sprintf(debug_values[4], "%s in %d cc", nextit, nexttime);
	}
	return tmp;
}

DEBUG_INFO ti_83p_debug_memory() {
	DEBUG_INFO tmp = { 7, debug_ti_83p_memory, debug_values };
	int i;
	sprintf(debug_values[0], "%d", ti_83p->mmap);
	sprintf(debug_values[1], "%02x", ti_83p->bank_a);
	sprintf(debug_values[2], "%02x", ti_83p->bank_b);
	for (i = 0; i < 4; i++)
		memory_page_string(debug_values[i + 3], ti_83p->page[i], ti_83p->rom, 0x20, ti_83p->ram, 0x02);
	return tmp;
}

DEBUG_INFO ti_83p_debug_keys() {
	DEBUG_INFO tmp = { 8, debug_ti_83p_keys, debug_values };
	int i, j;
	for (i = 0; i < 7; i++) {
		for (j = 0; j < 8; j++)
			debug_values[i][j] = ((ti_83p->key_state[i] >> (7 - j)) & 1) + '0';
		debug_values[i][j] = 0;
	}
	sprintf(debug_values[7], "%s", ti_83p->on_key ? "down" : "up");
	return tmp;
}

DEBUG_INFO ti_83p_debug_link() {
	DEBUG_INFO tmp = { 2, debug_ti_83p_link, debug_values };
	sprintf(debug_values[0], "%d%d", ti_83p->link_state >> 1, ti_83p->link_state & 1);
	sprintf(debug_values[1], "%d%d", *ti_83p->partner_link >> 1, *ti_83p->partner_link & 1);
	return tmp;
}

void ti_83p_enter_it_ie() {
	z80->iff1 ^= 1;
	z80->iff2 = z80->iff1;
}

void ti_83p_enter_it_freq() {
	switch (ti_83p->it_times[3]) {
		case ITSPEED00: ti_83p->it_times[3] = ITSPEED06; break;
		case ITSPEED02: ti_83p->it_times[3] = ITSPEED00; break;
		case ITSPEED04: ti_83p->it_times[3] = ITSPEED02; break;
		case ITSPEED06: ti_83p->it_times[3] = ITSPEED04; break;
	}
	ti_83p->it_times[0] = ti_83p->it_times[3] >> 1;
	ti_83p->it_times[1] = ti_83p->it_times[0] + 1600;
	ti_83p->it_times[2] = ti_83p->it_times[1] + 1200;
	ti_83p->it_next = ti_83p->it_times[ti_83p->it_state];
}

void ti_83p_enter_it_mask_on() { ti_83p->it_mask ^= 0x01; }
void ti_83p_enter_it_mask_t1() { ti_83p->it_mask ^= 0x02; }
void ti_83p_enter_it_mask_t2() { ti_83p->it_mask ^= 0x04; }
void ti_83p_enter_it_mask_power() { ti_83p->it_mask ^= 0x08; }

void ti_83p_enter_it_active_on() { ti_83p->it_active ^= 0x01; }
void ti_83p_enter_it_active_t1() { ti_83p->it_active ^= 0x02; }
void ti_83p_enter_it_active_t2() { ti_83p->it_active ^= 0x04; }

void ti_83p_enter_mem_map() {
	ti_83p->mmap ^= 1;
	ti_83p_swap_rom_page(ti_83p->bank_a, ti_83p->bank_b);
}
void ti_83p_enter_bank_a() {
	if (debug_read_input()) return;
	ti_83p_swap_rom_page(dbg_hex & 0x7f, ti_83p->bank_b);
}
void ti_83p_enter_bank_b() {
	if (debug_read_input()) return;
	ti_83p_swap_rom_page(ti_83p->bank_a, dbg_hex & 0x7f);
}

void ti_83p_enter_on_key() { if (ti_83p->on_key ^= 1) ti_83p->it_active |= ti_83p->it_mask & 0x01; }

void ti_83p_enter_key_00() { ti_83p->key_state[0] ^= 0x01; }
void ti_83p_enter_key_01() { ti_83p->key_state[0] ^= 0x02; }
void ti_83p_enter_key_02() { ti_83p->key_state[0] ^= 0x04; }
void ti_83p_enter_key_03() { ti_83p->key_state[0] ^= 0x08; }
void ti_83p_enter_key_04() { ti_83p->key_state[0] ^= 0x10; }
void ti_83p_enter_key_05() { ti_83p->key_state[0] ^= 0x20; }
void ti_83p_enter_key_06() { ti_83p->key_state[0] ^= 0x40; }
void ti_83p_enter_key_07() { ti_83p->key_state[0] ^= 0x80; }

void ti_83p_enter_key_10() { ti_83p->key_state[1] ^= 0x01; }
void ti_83p_enter_key_11() { ti_83p->key_state[1] ^= 0x02; }
void ti_83p_enter_key_12() { ti_83p->key_state[1] ^= 0x04; }
void ti_83p_enter_key_13() { ti_83p->key_state[1] ^= 0x08; }
void ti_83p_enter_key_14() { ti_83p->key_state[1] ^= 0x10; }
void ti_83p_enter_key_15() { ti_83p->key_state[1] ^= 0x20; }
void ti_83p_enter_key_16() { ti_83p->key_state[1] ^= 0x40; }
void ti_83p_enter_key_17() { ti_83p->key_state[1] ^= 0x80; }

void ti_83p_enter_key_20() { ti_83p->key_state[2] ^= 0x01; }
void ti_83p_enter_key_21() { ti_83p->key_state[2] ^= 0x02; }
void ti_83p_enter_key_22() { ti_83p->key_state[2] ^= 0x04; }
void ti_83p_enter_key_23() { ti_83p->key_state[2] ^= 0x08; }
void ti_83p_enter_key_24() { ti_83p->key_state[2] ^= 0x10; }
void ti_83p_enter_key_25() { ti_83p->key_state[2] ^= 0x20; }
void ti_83p_enter_key_26() { ti_83p->key_state[2] ^= 0x40; }
void ti_83p_enter_key_27() { ti_83p->key_state[2] ^= 0x80; }

void ti_83p_enter_key_30() { ti_83p->key_state[3] ^= 0x01; }
void ti_83p_enter_key_31() { ti_83p->key_state[3] ^= 0x02; }
void ti_83p_enter_key_32() { ti_83p->key_state[3] ^= 0x04; }
void ti_83p_enter_key_33() { ti_83p->key_state[3] ^= 0x08; }
void ti_83p_enter_key_34() { ti_83p->key_state[3] ^= 0x10; }
void ti_83p_enter_key_35() { ti_83p->key_state[3] ^= 0x20; }
void ti_83p_enter_key_36() { ti_83p->key_state[3] ^= 0x40; }
void ti_83p_enter_key_37() { ti_83p->key_state[3] ^= 0x80; }

void ti_83p_enter_key_40() { ti_83p->key_state[4] ^= 0x01; }
void ti_83p_enter_key_41() { ti_83p->key_state[4] ^= 0x02; }
void ti_83p_enter_key_42() { ti_83p->key_state[4] ^= 0x04; }
void ti_83p_enter_key_43() { ti_83p->key_state[4] ^= 0x08; }
void ti_83p_enter_key_44() { ti_83p->key_state[4] ^= 0x10; }
void ti_83p_enter_key_45() { ti_83p->key_state[4] ^= 0x20; }
void ti_83p_enter_key_46() { ti_83p->key_state[4] ^= 0x40; }
void ti_83p_enter_key_47() { ti_83p->key_state[4] ^= 0x80; }

void ti_83p_enter_key_50() { ti_83p->key_state[5] ^= 0x01; }
void ti_83p_enter_key_51() { ti_83p->key_state[5] ^= 0x02; }
void ti_83p_enter_key_52() { ti_83p->key_state[5] ^= 0x04; }
void ti_83p_enter_key_53() { ti_83p->key_state[5] ^= 0x08; }
void ti_83p_enter_key_54() { ti_83p->key_state[5] ^= 0x10; }
void ti_83p_enter_key_55() { ti_83p->key_state[5] ^= 0x20; }
void ti_83p_enter_key_56() { ti_83p->key_state[5] ^= 0x40; }
void ti_83p_enter_key_57() { ti_83p->key_state[5] ^= 0x80; }

void ti_83p_enter_key_60() { ti_83p->key_state[6] ^= 0x01; }
void ti_83p_enter_key_61() { ti_83p->key_state[6] ^= 0x02; }
void ti_83p_enter_key_62() { ti_83p->key_state[6] ^= 0x04; }
void ti_83p_enter_key_63() { ti_83p->key_state[6] ^= 0x08; }
void ti_83p_enter_key_64() { ti_83p->key_state[6] ^= 0x10; }
void ti_83p_enter_key_65() { ti_83p->key_state[6] ^= 0x20; }
void ti_83p_enter_key_66() { ti_83p->key_state[6] ^= 0x40; }
void ti_83p_enter_key_67() { ti_83p->key_state[6] ^= 0x80; }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hwcore.h"

#define CPUSPEED 6000000
#define STOPPERIOD (CPUSPEED / 25)

MODEL_TI83 *ti_83;

/*
KeyPressed: ; bit:   7       6       5       4       3       2       1       0
 .byte 255  ;                                        up      right   left    down
 .byte 255  ;                clear   ^       /       *       -       +       enter
 .byte 255  ;                vars    tan     )       9       6       3       (-)
 .byte 255  ;        stat    prgm    cos     (       8       5       2        .
 .byte 255  ;        X       matrx   sin     ,       7       4       1        0
 .byte 255  ;        alpha   math    x^-1    x^2     log     ln      sto
 .byte 255  ;        del     mode    2nd     y=      window  zoom    trace    graph
*/

/*
	ti_83->key_map[1] = 0x66; // ESC -> mode
	ti_83->key_map[2] = 0x41; // 1
	ti_83->key_map[3] = 0x31; // 2
	ti_83->key_map[4] = 0x21; // 3
	ti_83->key_map[5] = 0x42; // 4
	ti_83->key_map[6] = 0x32; // 5
	ti_83->key_map[7] = 0x22; // 6
	ti_83->key_map[8] = 0x43; // 7
	ti_83->key_map[9] = 0x33; // 8
	ti_83->key_map[10] = 0x23; // 9
	ti_83->key_map[11] = 0x40; // 0
	ti_83->key_map[12] = 0x20; // - -> (-)
	ti_83->key_map[13] = 0x47; // = -> XT0n
	ti_83->key_map[16] = 0x23; // Q -> 9
	ti_83->key_map[17] = 0x12; // W -> -
	ti_83->key_map[18] = 0x45; // E -> sin
	ti_83->key_map[19] = 0x13; // R -> *
	ti_83->key_map[20] = 0x42; // T -> 4
	ti_83->key_map[21] = 0x41; // Y -> 1
	ti_83->key_map[22] = 0x32; // U -> 5
	ti_83->key_map[23] = 0x54; // I -> x2
	ti_83->key_map[24] = 0x43; // O -> 7
	ti_83->key_map[25] = 0x33; // P -> 8
	ti_83->key_map[26] = 0x34; // [ -> (
	ti_83->key_map[27] = 0x24; // ] -> )
	ti_83->key_map[28] = 0x10; // Enter
	ti_83->key_map[29] = 0x57; // L-Ctrl -> alpha
	ti_83->key_map[30] = 0x56; // A -> math
	ti_83->key_map[31] = 0x52; // S -> ln
	ti_83->key_map[32] = 0x55; // D -> x-1
	ti_83->key_map[33] = 0x35; // F -> cos
	ti_83->key_map[34] = 0x25; // G -> tan
	ti_83->key_map[35] = 0x15; // H -> ^
	ti_83->key_map[36] = 0x44; // J -> ,
	ti_83->key_map[37] = 0x34; // K -> (
	ti_83->key_map[38] = 0x24; // L -> )
	ti_83->key_map[42] = 0x65; // L-Shift -> 2nd
	ti_83->key_map[44] = 0x31; // Z -> 2
	ti_83->key_map[45] = 0x51; // X -> sto
	ti_83->key_map[46] = 0x36; // C -> prgm
	ti_83->key_map[47] = 0x22; // V -> 6
	ti_83->key_map[48] = 0x46; // B -> matrx
	ti_83->key_map[49] = 0x53; // N -> log
	ti_83->key_map[50] = 0x14; // M -> /
	ti_83->key_map[51] = 0x44; // , -> ,
	ti_83->key_map[52] = 0x30; // . -> .
	ti_83->key_map[53] = 0x14; // /
	ti_83->key_map[54] = 0x16; // R-Shift -> clear
	ti_83->key_map[55] = 0x13; // *
	ti_83->key_map[57] = 0x40; // spc -> 0
	ti_83->key_map[59] = 0x64; // F1 -> y=
	ti_83->key_map[60] = 0x63; // F2 -> window
	ti_83->key_map[61] = 0x62; // F3 -> zoom
	ti_83->key_map[62] = 0x61; // F4 -> trace
	ti_83->key_map[63] = 0x60; // F5 -> graph
	ti_83->key_map[71] = 0x56; // Home -> math
	ti_83->key_map[72] = 0x03; // Up
	ti_83->key_map[73] = 0x46; // PgUp -> matrx
	ti_83->key_map[74] = 0x12; // -
	ti_83->key_map[75] = 0x01; // Left
	ti_83->key_map[77] = 0x02; // Right
	ti_83->key_map[78] = 0x11; // +
	ti_83->key_map[79] = 0x37; // End -> stat
	ti_83->key_map[80] = 0x00; // Down
	ti_83->key_map[81] = 0x36; // PgDn -> prgm
	ti_83->key_map[82] = 0x26; // Ins -> vars
	ti_83->key_map[83] = 0x67; // Del -> del
*/

BYTE ti_83_key_map[0x80] = {
	0xff, 0x66, 0x41, 0x31, 0x21, 0x42, 0x32, 0x22,
	0x43, 0x33, 0x23, 0x40, 0x20, 0x47, 0xff, 0xff,
	0x23, 0x12, 0x45, 0x13, 0x42, 0x41, 0x32, 0x54,
	0x43, 0x33, 0x34, 0x24, 0x10, 0x57, 0x56, 0x52,
	0x55, 0x35, 0x25, 0x15, 0x44, 0x34, 0x24, 0xff,
	0xff, 0xff, 0x65, 0xff, 0x31, 0x51, 0x36, 0x22,
	0x46, 0x53, 0x14, 0x44, 0x30, 0x14, 0x16, 0x13,
	0xff, 0x40, 0xff, 0x64, 0x63, 0x62, 0x61, 0x60,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x56,
	0x03, 0x46, 0x12, 0x01, 0xff, 0x02, 0x11, 0x37,
	0x00, 0x36, 0x26, 0x67, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void ti_83_default_keymap() {
	memset(ti_83->key_state, 0xff, 7);
	memcpy(ti_83->key_map, ti_83_key_map, 0x80);
}

// Resetting the calc with the current ROM
void ti_83_reset() {
	int i;

	ti_83_default_keymap();
	z80_reset();
	lcd_reset();

	emu->stop_cnt = 0;
	emu->stop_period = STOPPERIOD;
	emu->dbus = 0xfe;
	emu->link_state = 0x03;

	// Memory pages
	ti_83->page[0] = ti_83->rom;
	ti_83->mut[0] = 0;
	ti_83->mmap = 0;
	ti_83->port_02 = 0;
	ti_83_swap_rom_page(0x00);

	// Interrupts
	ti_83->it_cnt = 0;
	ti_83->it_state = 0;
	ti_83->it_next = 100000;
	ti_83->it_active = 0;
	ti_83->it_mask = 0;
	for (i = 0; i < 4; i++)
		ti_83->it_times[i] = 100000 + i * 1000;

	// Other hardware
	ti_83->link_state = 0;
	ti_83->on_key = 0;
	ti_83->key_mask = 0xff;
	for (i = 0; i < 7; i++)
		ti_83->key_state[i] = 0xff;

}

// Returning power state
int ti_83_powered() {
	return (ti_83->it_mask & 0x08) || !z80->hlt;
}

// Setting key state
void ti_83_key(BYTE key, int state) {
	if (key == 0x80) {
		if (state & 1) {
			if (!ti_83->on_key) {
				ti_83->on_key = 1;
				ti_83->it_active |= ti_83->it_mask & 0x01;
			}
		} else ti_83->on_key = 0;
	} else {
		BYTE km = (state < 2) ? ti_83->key_map[key] : key;
		if (km < 0x70) {
			if (state & 1) ti_83->key_state[km >> 4] &= ~(1 << (km & 0x07));
			else ti_83->key_state[km >> 4] |= (1 << (km & 0x07));
		}
	}
}

// Telling whether the upcoming instruction affects the link port
int ti_83_link_instruction() {
	BYTE b1 = ti_83_read(R_PC);
	BYTE b2 = ti_83_read(R_PC + 1);
	return
		// out (n),a
		((b1 == 0xd3) && ((b2 & 0x17) == 0x00)) ||
		// in a,(n)
		((b1 == 0xdb) && (((b2 & 0x17) == 0x00) || ((b2 & 0x17) == 0x04) || ((b2 & 0x17) == 0x14))) ||
		((b1 == 0xed) && (
			// out (c),r
			(((b2 & 0xc7) == 0x41) && ((R_C & 0x17) == 0x00)) ||
			// in r,(c)
			(((b2 & 0xc7) == 0x40) && (((R_C & 0x17) == 0x00) || ((R_C & 0x17) == 0x04) || ((R_C & 0x17) == 0x14)))
		));
}

// Running between two screen refreshes
void ti_83_run() {
	for (;;) {
		if (debug_active) debug_check();
		while ((ti_83->it_cnt < ti_83->it_next) && !(ti_83->it_active) &&
			(emu->stop_cnt < emu->stop_period) && !z80->hlt && !debug_trapped) {
				z80_step();
				if (debug_active) debug_check();
			}
		// Return on debug event
		if (debug_trapped) return;
		// Cycle time elapsed, need to return
		if (emu->stop_cnt >= emu->stop_period) return;
		// Handling interrupts
		switch (z80->hlt + (ti_83->it_active != 0) * 4 + (ti_83->it_cnt >= ti_83->it_next) * 2) {
			// Normal instruction
			case 0:
				z80_step();
			break;
			// The CPU is halted, jumping straight to the next timer interrupt or exit
			case 1: {
				if ((ti_83->it_mask & 0x06) && (emu->stop_period - emu->stop_cnt > ti_83->it_next - ti_83->it_cnt)) {
					// Jumping to timer event
					emu->stop_cnt += ti_83->it_next - ti_83->it_cnt;
					ti_83->it_cnt = ti_83->it_next;
				} else {
					// Jumping to the end of the period
					ti_83->it_cnt += emu->stop_period - emu->stop_cnt;
					emu->stop_cnt = emu->stop_period;
					return;
				}
			}
			break;
			// An interrupt timer is firing; bringing down timer lines if power is on
			case 2: case 3: case 6: case 7:
				switch (ti_83->it_state) {
					// Timer 2
					case 0: case 1:
						ti_83->it_active |= ti_83->it_mask & ((((ti_83->it_mask & 0x08) || !z80->hlt)) ? 0x04 : 0x00);
					break;
					// Timer 1
					case 2:
						ti_83->it_active |= ti_83->it_mask & ((((ti_83->it_mask & 0x08) || !z80->hlt)) ? 0x02 : 0x00);
					break;
					// Resetting counter
					case 3:
						ti_83->it_cnt -= ti_83->it_next;
					break;
				}
				ti_83->it_state = (ti_83->it_state + 1) & 0x03;
				ti_83->it_next = ti_83->it_times[ti_83->it_state];
				if (!ti_83->it_active) break;
			// Some interrupt is active
			case 4: case 5:
				if (z80->ie) z80_step();
				if (z80->iff1) {
					emu->dbus = rand();
					z80_interrupt();
				}
				else if (!z80->hlt) z80_step();
				else {
					// Halted with interrupts disabled...
					emu->stop_cnt = emu->stop_period;
					return;
				}
			break;
		}
	}
}

// Running while connected
void ti_83_run_linked() {
	for (;;) {
		while ((ti_83->it_cnt < ti_83->it_next) && !(ti_83->it_active) &&
			(emu->stop_cnt < emu->stop_period) && !z80->hlt &&
			((emu->stop_cnt < linking.time) || !ti_83_link_instruction())) z80_step();
		// Link instruction encountered, but partner link state is old
		if ((emu->stop_cnt >= linking.time) && ti_83_link_instruction()) return;
		// Cycle time elapsed, need to return
		if (emu->stop_cnt >= emu->stop_period) return;
		// Handling interrupts
		switch (z80->hlt + (ti_83->it_active != 0) * 4 + (ti_83->it_cnt >= ti_83->it_next) * 2) {
			// Normal instruction
			case 0:
				z80_step();
			break;
			// The CPU is halted, jumping straight to the next timer interrupt or exit
			case 1: {
				if ((ti_83->it_mask & 0x06) && (emu->stop_period - emu->stop_cnt > ti_83->it_next - ti_83->it_cnt)) {
					// Jumping to timer event
					emu->stop_cnt += ti_83->it_next - ti_83->it_cnt;
					ti_83->it_cnt = ti_83->it_next;
				} else {
					// Jumping to the end of the period
					ti_83->it_cnt += emu->stop_period - emu->stop_cnt;
					emu->stop_cnt = emu->stop_period;
					return;
				}
			}
			break;
			// An interrupt timer is firing; bringing down timer lines if power is on
			case 2: case 3: case 6: case 7:
				switch (ti_83->it_state) {
					// Timer 2
					case 0: case 1:
						ti_83->it_active |= ti_83->it_mask & ((((ti_83->it_mask & 0x08) || !z80->hlt)) ? 0x04 : 0x00);
					break;
					// Timer 1
					case 2:
						ti_83->it_active |= ti_83->it_mask & ((((ti_83->it_mask & 0x08) || !z80->hlt)) ? 0x02 : 0x00);
					break;
					// Resetting counter
					case 3:
						ti_83->it_cnt -= ti_83->it_next;
					break;
				}
				ti_83->it_state = (ti_83->it_state + 1) & 0x03;
				ti_83->it_next = ti_83->it_times[ti_83->it_state];
				if (!ti_83->it_active) break;
			// Some interrupt is active
			case 4: case 5:
				if (z80->ie) z80_step();
				if (z80->iff1) {
					emu->dbus = rand();
					z80_interrupt();
				}
				else if (!z80->hlt) z80_step();
				else {
					// Halted with interrupts disabled...
					emu->stop_cnt = emu->stop_period;
					return;
				}
			break;
		}
	}
}

// Sending a file (which can be a group) from the computer through silent link
int ti_83_send_file(char *fname, BYTE **partner) {
	FILE *fp;
	int i, err = 0;
	WORD lgt, type, cs;
	BYTE dat[0x10000];
	BYTE *psave;

	// Plugging into the file transfer interface
	psave = *partner;
	*partner = &emu->link_state;

	fp = fopen(fname, "rb");
	fseek(fp, 0x39, SEEK_SET);

	// Sending a whole group
	while (!feof(fp)) {
		lgt = 0xffff;
		fread(&lgt, 2, 1, fp);
		if (lgt == 0xffff) break;
		type = 0; fread(&type, 1, 1, fp);

		// Silent link RTS
		dat[0] = 0x03; dat[1] = 0xc9; dat[2] = 0x0b; dat[3] = 0x00;
		dat[4] = lgt; dat[5] = lgt >> 8; dat[6] = type;
		fread(dat + 7, 10, 1, fp);
		for (i = 4, cs = 0; i < 15; i++) cs += dat[i];
		dat[15] = cs; dat[16] = cs >> 8;
		if (send_data(dat, 17)) { err = 1; break; }

		// ACK and CTS from calc
		if (recv_byte() != 0x83) { err = 1; break; }
		if (recv_byte() != 0x56) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }
		if (recv_byte() != 0x83) { err = 1; break; }
		if (recv_byte() != 0x09) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }

		// ACK and data block
		dat[1] = 0x56; dat[2] = 0x00; dat[4] = 0x03; dat[5] = 0x15;
		dat[6] = lgt; dat[7] = lgt >> 8;
		fread(dat + 8, lgt + 2, 1, fp);
		for (i = 0, cs = 0; i < lgt; i++) cs += dat[i + 8];
		dat[lgt + 8] = cs; dat[lgt + 9] = cs >> 8;
		calculator_run_timed(100);
		if (send_data(dat, lgt + 10)) { err = 1; break; }

		// ACK from calc
		if (recv_byte() != 0x83) { err = 1; break; }
		if (recv_byte() != 0x56) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }

		// EOT to calc
		dat[1] = 0x92;
		calculator_run_timed(100);
		if (send_data(dat, 4)) { err = 1; break; }
	}

	// Finished transfer
	fclose(fp);
	emu->link_state = 3;
	emu->stop_period = STOPPERIOD;

	// Plugging back into the original partner
	*partner = psave;

	return err;
}

// Replacing the ROM and resetting
void ti_83_load_rom(const char *fname) {
	FILE *fp;

	fp = fopen(fname, "rb");
	fread(ti_83->rom, 0x40000, 1, fp);
	fclose(fp);

	ti_83_reset();
}

// ROM page swapping
void ti_83_swap_rom_page(BYTE new_page) {
	BYTE *p1, *p2;
	BYTE m1, m2;

	ti_83->rom_page = new_page;

	// Page determined by port 2 bits 3 and 7
	switch (ti_83->port_02 & 0x88) {
		case 0x00: p1 = ti_83->rom + ((ti_83->rom_page & 0x08) << 14); m1 = 0; break;
		case 0x08: p1 = ti_83->rom + ((ti_83->rom_page & 0x08) << 14) + 0x4000; m1 = 0; break;
		case 0x80: p1 = ti_83->ram; m1 = 1; break;
		case 0x88: p1 = ti_83->ram + 0x4000; m1 = 1; break;
		default: p1 = ti_83->rom; m1 = 0;
	}

	// Swappable page
	if (new_page & 0x10) {
		// RAM page activated
		p2 = ti_83->ram + ((new_page & 1) << 14);
		m2 = 1;
	} else {
		// ROM page activated
		p2 = ti_83->rom + (new_page << 14);
		m2 = 0;
	}

	// Setting memory map
	switch (ti_83->mmap | (ti_83->port_02 & 0x40)) {
		case 0x00:
		case 0x40:
			ti_83->page[1] = p2;
			ti_83->page[2] = p1;
			ti_83->page[3] = ti_83->ram;
			ti_83->mut[1] = m2;
			ti_83->mut[2] = m1;
			ti_83->mut[3] = 1;
		break;
		case 0x01:
			ti_83->page[1] = ti_83->rom + ((ti_83->rom_page & 0x08) << 14);
			ti_83->page[2] = p2;
			ti_83->page[3] = p1;
			ti_83->mut[1] = 0;
			ti_83->mut[2] = m2;
			ti_83->mut[3] = m1;
		break;
		case 0x41:
			ti_83->page[1] = ti_83->ram;
			ti_83->page[2] = ti_83->ram + 0x4000;
			ti_83->page[3] = p1;
			ti_83->mut[1] = 1;
			ti_83->mut[2] = 1;
			ti_83->mut[3] = m1;
		break;
	}
}

// TI-83 memory write
void ti_83_write(WORD adr, BYTE val) {
	BYTE tmp;

	if (ti_83->mut[(tmp = adr >> 14)]) {
		ti_83->page[tmp][adr & 0x3fff] = val;
	}
}

// TI-83 memory read
BYTE ti_83_read(WORD adr) {
	return ti_83->page[adr >> 14][adr & 0x3fff];
}

// TI-83 output port
void ti_83_out(BYTE port, BYTE val) {
	// Note that bit 3 is masked, so everything is mirrored at x+8
	switch (port & 0x17) {
		// Linking + ROM page bit 3
		case 0x00: // case 0x08:
			ti_83_swap_rom_page((ti_83->rom_page & 0x17) | ((val & 0x10) >> 1));
			ti_83->link_state = (~val) & 0x03;
		break;
		// Keyboard mask
		case 0x01: // case 0x09:
			if (val == 0xff)
				ti_83->key_mask = 0xff;
			else
				ti_83->key_mask &= val;
		break;
		// ROM page state (lower bits and RAM swap)
		case 0x02: // case 0x0A:
			ti_83->port_02 = val;
			// Change between RAM and ROM pages
			ti_83_swap_rom_page((ti_83->rom_page & 0x08) | (val & 0x07) | ((val & 0x40) >> 2));
		break;
		// IT mask
		case 0x03: // case 0x0B:
			// Resetting bit 3 shuts down the LCD and the timers on the next halt
			ti_83->it_mask = val & 0x0f;
			ti_83->it_active &= val;
		break;
		// IT frequency + paging mode
		case 0x04: // case 0x0C:
			// IT period
			ti_83->it_times[3] = CPUSPEED * (3 + ((val & 0x06) << 1)) / 1620;
			// First firing of timer 2
			ti_83->it_times[0] = ti_83->it_times[3] >> 1;
			// Second firing of timer 2
			ti_83->it_times[1] = ti_83->it_times[0] + 1600;
			// Firing of timer 1
			ti_83->it_times[2] = ti_83->it_times[1] + 1200;
			// Bit 4 controls a general multiplier
			if (val & 0x10) {
				ti_83->it_times[0] *= 0.9;
				ti_83->it_times[1] *= 0.9;
				ti_83->it_times[2] *= 0.9;
				ti_83->it_times[3] *= 0.9;
			}
			ti_83->it_next = ti_83->it_times[ti_83->it_state];
			// Bit 0 affects paging
			ti_83->mmap = val & 1;
			ti_83_swap_rom_page(ti_83->rom_page);
			// Bits 5-7 are related to power management, no emulation is necessary
			// (virtual batteries are always fresh :)
		break;
		// LCD command port
		case 0x10: case 0x12: // case 0x18: case 0x1A:
			lcd_command(val);
		break;
		// LCD data port
		case 0x11: case 0x13: // case 0x19: case 0x1B:
			lcd_write(val);
		break;
	}
}

// TI-83 input port
BYTE ti_83_in(BYTE port) {
	BYTE retval = 0;
	// Note that bit 3 is masked, so everything is mirrored at x+8
	switch (port & 0x17) {
		// Linking + ROM page bit 3
		case 0x00: case 0x04: case 0x14: // case 0x08: case 0x0C: case 0x1C:
			retval =
				ti_83->link_state |
				((ti_83->link_state & *(ti_83->partner_link)) << 2) |
				((ti_83->rom_page & 0x08) << 1);
			// Good batteries indicated at port 14
			if ((port & 0x17) == 0x14) retval |= 1;
		break;
		// Keyboard state
		case 0x01: case 0x05: case 0x15: { // case 0x09: case 0x0D: case 0x1D:
			int i;
			retval = 0xff;
			for (i = 0; i < 7; i++)
				if (!(ti_83->key_mask & (1 << i))) retval &= ti_83->key_state[i];
			// Clearing bit 0 for a mirror
			if ((port & 0x17) == 0x15) retval &= 0xfe;
		}
		break;
		// ROM page state
		case 0x02: case 0x06: case 0x16: // case 0x0A: case 0x0E: case 0x1E:
			retval = ti_83->port_02;
			// Clearing bit 0 for a mirror
			if ((port & 0x17) == 0x16) retval &= 0xfe;
		break;
		// IT status
		case 0x03: case 0x07: case 0x17: // case 0x0B: case 0x0F: case 0x1F:
			retval =
				ti_83->it_active | // ON key and timer interrupts
				((ti_83->on_key ^ 1) << 3); // ON key state
			// Clearing bit 0 for a mirror
			if ((port & 0x17) == 0x17) retval &= 0xfe;
		break;
		// LCD status port
		case 0x10: case 0x12: // case 0x18: case 0x1A:
			retval = lcd_status();
		break;
		// LCD data port
		case 0x11: case 0x13: // case 0x19: case 0x1B:
			retval = lcd_read();
		break;
	}

	return retval;
}

// Loading a VTI saved state with the current ROM
void ti_83_load_vti_sav(const char *fname) {
	FILE *fp;
	BYTE tmp[0x358];
	int i, j;

	fp = fopen(fname, "rb");

	// Z80 state
	fread(tmp, 0x94, 1, fp);
	z80->af.w = tmp[0x40] + (tmp[0x41] << 8);
	z80->bc.w = tmp[0x44] + (tmp[0x45] << 8);
	z80->de.w = tmp[0x48] + (tmp[0x49] << 8);
	z80->hl.w = tmp[0x4c] + (tmp[0x4d] << 8);
	z80->ix.w = tmp[0x50] + (tmp[0x51] << 8);
	z80->iy.w = tmp[0x54] + (tmp[0x55] << 8);
	z80->pc = tmp[0x58] + (tmp[0x59] << 8);
	z80->sp.w = tmp[0x5c] + (tmp[0x5d] << 8);
	z80->afs = tmp[0x60] + (tmp[0x61] << 8);
	z80->bcs = tmp[0x64] + (tmp[0x65] << 8);
	z80->des = tmp[0x68] + (tmp[0x69] << 8);
	z80->hls = tmp[0x6c] + (tmp[0x6d] << 8);
	z80->iff1 = tmp[0x70];
	z80->iff2 = tmp[0x74];
	z80->hlt = tmp[0x78] & 1;
	z80->im = tmp[0x7c];
	z80->i = tmp[0x80];
	z80->r = (tmp[0x84] & 0x7f) | (tmp[0x88] & 0x80);

	// RAM contents (note the reversed order of pages)
	fread(ti_83->ram + 0x4000, 0x4000, 1, fp);
	fread(ti_83->ram, 0x4000, 1, fp);

	// Miscellaneous pieces of hardware
	fread(tmp, 0x354, 1, fp);
	ti_83->key_mask = tmp[0x300];
	ti_83->mmap = 0;
	ti_83->port_02 = tmp[0x350];
	ti_83_swap_rom_page(tmp[0x308] & 0x0f);
	// VTI apparently doesn't implement timer masking, so it's always enabled
	ti_83->it_mask = 0x0e | (tmp[0x0340] & 1);

	// LCD state
	for (i = 0; i < 0x300; i++)
		for (j = 0; j < 8; j++) {
			lcd->dat[(i / 12) * 120 + (i % 12) * 8 + j] = (tmp[i] >> (7 - j)) & 1;
			lcd->scr[(i / 12) * 120 + (i % 12) * 8 + j] = 0;
		}
	lcd->y = tmp[0x0314];
	lcd->x = tmp[0x0318];
	lcd->up_dn = tmp[0x031c] & 1;
	lcd->cnt_sel = (tmp[0x031c] >> 1) & 1;
	lcd->dummy = tmp[0x0320] & 1;
	lcd->w_len = tmp[0x0324] & 1;
	lcd->on = (tmp[0x0348] & 1) ^ 1;
	lcd->ctr = tmp[0x034c] & 0x3f;
	lcd->amp = 3;
	lcd->amp2 = 3;

	fclose(fp);
}

// Macros for identical serialisation in both directions
#define l83C(x) ti_83->x = fgetc(lfp)
#define l83I(x) \
	ti_83->x = fgetc(lfp) << 24; \
	ti_83->x += fgetc(lfp) << 16; \
	ti_83->x += fgetc(lfp) << 8; \
	ti_83->x += fgetc(lfp);

#define s83C(x) fputc(ti_83->x, sfp)
#define s83I(x) \
	fputc(ti_83->x >> 24, sfp); \
	fputc(ti_83->x >> 16, sfp); \
	fputc(ti_83->x >> 8, sfp); \
	fputc(ti_83->x, sfp)

#define SER83(_ls,ls,rw) \
	emu_ ## _ls(ls ## fp); \
	z80_ ## _ls(ls ## fp); \
	lcd_ ## _ls(ls ## fp); \
	ls ## 83C(rom_page); ls ## 83C(link_state); ls ## 83C(on_key); \
	ls ## 83C(it_mask); ls ## 83C(it_active); ls ## 83I(it_cnt); ls ## 83I(it_next); ls ## 83C(it_state); \
	ls ## 83I(it_times[0]); ls ## 83I(it_times[1]); ls ## 83I(it_times[2]); ls ## 83I(it_times[3]); \
	ls ## 83C(port_02); ls ## 83C(key_mask); ls ## 83C(mmap); \
	f ## rw(ti_83->key_state, 7, 1, ls ## fp); \
	f ## rw(ti_83->key_map, 0x80, 1, ls ## fp); \
	f ## rw(ti_83->ram, 0x8000, 1, ls ## fp);

// Loading TI-83 state from <romfile>.pti
int ti_83_load_state(char *romfile) {
	FILE *lfp;
	char fn[MAX_PATH + 5];

	ti_83_load_rom(romfile);
	sprintf(fn, "%s.pti", romfile);
	if (!(lfp = fopen(fn, "rb"))) return 0;
	SER83(load, l, read);
	fclose(lfp);
	// Restoring things that couldn't be saved
	ti_83_swap_rom_page(ti_83->rom_page);

	return 0;
}

// Saving TI-83 state to <romfile>.pti
int ti_83_save_state(char *romfile) {
	FILE *sfp;
	char fn[MAX_PATH + 5];

	sprintf(fn, "%s.pti", romfile);
	sfp = fopen(fn, "wb");
	SER83(save, s, write);
	fclose(sfp);

	return 0;
}


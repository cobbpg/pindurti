#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hwcore.h"

#define CPUSPEED 6000000
#define STOPPERIOD (CPUSPEED / 25)

MODEL_TI83P *ti_83p;

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
	ti_83p->key_map[1] = 0x66; // ESC -> mode
	ti_83p->key_map[2] = 0x41; // 1
	ti_83p->key_map[3] = 0x31; // 2
	ti_83p->key_map[4] = 0x21; // 3
	ti_83p->key_map[5] = 0x42; // 4
	ti_83p->key_map[6] = 0x32; // 5
	ti_83p->key_map[7] = 0x22; // 6
	ti_83p->key_map[8] = 0x43; // 7
	ti_83p->key_map[9] = 0x33; // 8
	ti_83p->key_map[10] = 0x23; // 9
	ti_83p->key_map[11] = 0x40; // 0
	ti_83p->key_map[12] = 0x20; // - -> (-)
	ti_83p->key_map[13] = 0x47; // = -> XT0n
	ti_83p->key_map[16] = 0x23; // Q -> 9
	ti_83p->key_map[17] = 0x12; // W -> -
	ti_83p->key_map[18] = 0x45; // E -> sin
	ti_83p->key_map[19] = 0x13; // R -> *
	ti_83p->key_map[20] = 0x42; // T -> 4
	ti_83p->key_map[21] = 0x41; // Y -> 1
	ti_83p->key_map[22] = 0x32; // U -> 5
	ti_83p->key_map[23] = 0x54; // I -> x2
	ti_83p->key_map[24] = 0x43; // O -> 7
	ti_83p->key_map[25] = 0x33; // P -> 8
	ti_83p->key_map[26] = 0x34; // [ -> (
	ti_83p->key_map[27] = 0x24; // ] -> )
	ti_83p->key_map[28] = 0x10; // Enter
	ti_83p->key_map[29] = 0x57; // L-Ctrl -> alpha
	ti_83p->key_map[30] = 0x56; // A -> math
	ti_83p->key_map[31] = 0x52; // S -> ln
	ti_83p->key_map[32] = 0x55; // D -> x-1
	ti_83p->key_map[33] = 0x35; // F -> cos
	ti_83p->key_map[34] = 0x25; // G -> tan
	ti_83p->key_map[35] = 0x15; // H -> ^
	ti_83p->key_map[36] = 0x44; // J -> ,
	ti_83p->key_map[37] = 0x34; // K -> (
	ti_83p->key_map[38] = 0x24; // L -> )
	ti_83p->key_map[42] = 0x65; // L-Shift -> 2nd
	ti_83p->key_map[44] = 0x31; // Z -> 2
	ti_83p->key_map[45] = 0x51; // X -> sto
	ti_83p->key_map[46] = 0x36; // C -> prgm
	ti_83p->key_map[47] = 0x22; // V -> 6
	ti_83p->key_map[48] = 0x46; // B -> matrx
	ti_83p->key_map[49] = 0x53; // N -> log
	ti_83p->key_map[50] = 0x14; // M -> /
	ti_83p->key_map[51] = 0x44; // , -> ,
	ti_83p->key_map[52] = 0x30; // . -> .
	ti_83p->key_map[53] = 0x14; // /
	ti_83p->key_map[54] = 0x16; // R-Shift -> clear
	ti_83p->key_map[55] = 0x13; // *
	ti_83p->key_map[57] = 0x40; // spc -> 0
	ti_83p->key_map[59] = 0x64; // F1 -> y=
	ti_83p->key_map[60] = 0x63; // F2 -> window
	ti_83p->key_map[61] = 0x62; // F3 -> zoom
	ti_83p->key_map[62] = 0x61; // F4 -> trace
	ti_83p->key_map[63] = 0x60; // F5 -> graph
	ti_83p->key_map[71] = 0x56; // Home -> math
	ti_83p->key_map[72] = 0x03; // Up
	ti_83p->key_map[73] = 0x46; // PgUp -> matrx
	ti_83p->key_map[74] = 0x12; // -
	ti_83p->key_map[75] = 0x01; // Left
	ti_83p->key_map[77] = 0x02; // Right
	ti_83p->key_map[78] = 0x11; // +
	ti_83p->key_map[79] = 0x37; // End -> stat
	ti_83p->key_map[80] = 0x00; // Down
	ti_83p->key_map[81] = 0x36; // PgDn -> prgm
	ti_83p->key_map[82] = 0x26; // Ins -> vars
	ti_83p->key_map[83] = 0x67; // Del -> del
*/

BYTE ti_83p_key_map[0x80] = {
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

void ti_83p_default_keymap() {
	memset(ti_83p->key_state, 0xff, 7);
	memcpy(ti_83p->key_map, ti_83p_key_map, 0x80);
}

// Resetting the calc with the current ROM
void ti_83p_reset() {
	int i;

	ti_83p_default_keymap();
	z80_reset();
	R_PC = 0x8000;
	lcd_reset();
	lcd->cwht = -0.85 * 65535;
	lcd->cblk = 0.10 * 65535;
	flash_reset(FLASH_TYPE_83P, ti_83p->rom);

	emu->stop_cnt = 0;
	emu->stop_period = STOPPERIOD;
	emu->dbus = 0xfe;
	emu->link_state = 0x03;

	// Memory pages
	ti_83p->page[0] = ti_83p->rom;
	ti_83p->mut[0] = 0;
	ti_83p->exc[0] = 0;
	ti_83p->mmap = 0;
	ti_83p_swap_rom_page(0x1f, 0x1f);
	ti_83p->flash_lock = 1;
	memset(ti_83p->run_lock, 0, 0x22);

	// Interrupts
	ti_83p->it_cnt = 0;
	ti_83p->it_state = 0;
	ti_83p->it_next = 100000;
	ti_83p->it_active = 0;
	ti_83p->it_mask = 0;
	for (i = 0; i < 4; i++)
		ti_83p->it_times[i] = 100000 + i * 1000;

	// Other hardware
	ti_83p->prot_cnt = 0;
	ti_83p->link_state = 0;
	ti_83p->on_key = 0;
	ti_83p->key_mask = 0xff;
	memset(ti_83p->key_state, 0xff, 7);
	memset(ti_83p->prot_buffer, 0xff, 8);

	// Memory contents
//	memset(ti_83p->ram, 0x00, 0x8000);
	// Patching certificate
	ti_83p->rom[0x78000] = 0x00;
	ti_83p->rom[0x78001] = 0xff;
	ti_83p->rom[0x79fe0] = 0x00;
	ti_83p->rom[0x79fe1] = 0x00;
	ti_83p->rom[0x7a000] = 0xff;

}

// Returning power state
int ti_83p_powered() {
	return (ti_83p->it_mask & 0x08) || !z80->hlt;
}

// Setting key state
void ti_83p_key(BYTE key, int state) {
	if (key == 0x80) {
		if (state & 1) {
			if (!ti_83p->on_key) {
				ti_83p->on_key = 1;
				ti_83p->it_active |= ti_83p->it_mask & 0x01;
			}
		} else ti_83p->on_key = 0;
	} else {
		BYTE km = (state < 2) ? ti_83p->key_map[key] : key;
		if (km < 0x70) {
			if (state & 1) ti_83p->key_state[km >> 4] &= ~(1 << (km & 0x07));
			else ti_83p->key_state[km >> 4] |= (1 << (km & 0x07));
		}
	}
}

// Telling whether the upcoming instruction affects the link port
int ti_83p_link_instruction() {
	BYTE b1 = ti_83p_rmem(R_PC);
	BYTE b2 = ti_83p_rmem(R_PC + 1);
	return
		// out (n),a; in a,(n)
		(((b2 & 0x17) == 0x00) && ((b1 & 0xf7) == 0xd3)) ||
		// out (c),r; in r,(c)
		((b1 == 0xed) && ((R_C & 0x17) == 0x00) && ((b2 & 0xc6) == 0x40));
}

// Running between two screen refreshes
void ti_83p_run() {
	for (;;) {
		if (debug_active) debug_check();
		while ((ti_83p->it_cnt < ti_83p->it_next) && !(ti_83p->it_active) &&
			(emu->stop_cnt < emu->stop_period) && !z80->hlt && !debug_trapped) {
				z80_step();
				if (debug_active) debug_check();
			}
		// Return on debug event
		if (debug_trapped) return;
		// Forced reset (code executed on RAM0)
		if (z80->hlt == 2) ti_83p_reset();
		// Cycle time elapsed, need to return
		if (emu->stop_cnt >= emu->stop_period) return;
		// Handling interrupts
		switch (z80->hlt + (ti_83p->it_active != 0) * 4 + (ti_83p->it_cnt >= ti_83p->it_next) * 2) {
			// Normal instruction
			case 0:
				z80_step();
			break;
			// The CPU is halted, jumping straight to the next timer interrupt or exit
			case 1: {
				if ((ti_83p->it_mask & 0x06) && (emu->stop_period - emu->stop_cnt > ti_83p->it_next - ti_83p->it_cnt)) {
					// Jumping to timer event
					emu->stop_cnt += ti_83p->it_next - ti_83p->it_cnt;
					ti_83p->it_cnt = ti_83p->it_next;
				} else {
					// Jumping to the end of the period
					ti_83p->it_cnt += emu->stop_period - emu->stop_cnt;
					emu->stop_cnt = emu->stop_period;
					return;
				}
			}
			break;
			// An interrupt timer is firing; bringing down timer lines if power is on
			case 2: case 3: case 6: case 7:
				switch (ti_83p->it_state) {
					// Timer 2
					case 0: case 1:
						ti_83p->it_active |= ti_83p->it_mask & ((((ti_83p->it_mask & 0x08) || !z80->hlt)) ? 0x04 : 0x00);
					break;
					// Timer 1
					case 2:
						ti_83p->it_active |= ti_83p->it_mask & ((((ti_83p->it_mask & 0x08) || !z80->hlt)) ? 0x02 : 0x00);
					break;
					// Resetting counter
					case 3:
						ti_83p->it_cnt -= ti_83p->it_next;
					break;
				}
				ti_83p->it_state = (ti_83p->it_state + 1) & 0x03;
				ti_83p->it_next = ti_83p->it_times[ti_83p->it_state];
				if (!ti_83p->it_active) break;
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
void ti_83p_run_linked() {
	for (;;) {
		while ((ti_83p->it_cnt < ti_83p->it_next) && !(ti_83p->it_active) &&
			(emu->stop_cnt < emu->stop_period) && !z80->hlt &&
			((emu->stop_cnt < linking.time) || !ti_83p_link_instruction())) z80_step();
		// Forced reset (code executed on RAM0)
		if (z80->hlt == 2) ti_83p_reset();
		// Link instruction encountered, but partner link state is old
		if ((emu->stop_cnt >= linking.time) && ti_83p_link_instruction()) return;
		// Cycle time elapsed, need to return
		if (emu->stop_cnt >= emu->stop_period) return;
		// Handling interrupts
		switch (z80->hlt + (ti_83p->it_active != 0) * 4 + (ti_83p->it_cnt >= ti_83p->it_next) * 2) {
			// Normal instruction
			case 0:
				z80_step();
			break;
			// The CPU is halted, jumping straight to the next timer interrupt or exit
			case 1: {
				if ((ti_83p->it_mask & 0x06) && (emu->stop_period - emu->stop_cnt > ti_83p->it_next - ti_83p->it_cnt)) {
					// Jumping to timer event
					emu->stop_cnt += ti_83p->it_next - ti_83p->it_cnt;
					ti_83p->it_cnt = ti_83p->it_next;
				} else {
					// Jumping to the end of the period
					ti_83p->it_cnt += emu->stop_period - emu->stop_cnt;
					emu->stop_cnt = emu->stop_period;
					return;
				}
			}
			break;
			// An interrupt timer is firing; bringing down timer lines if power is on
			case 2: case 3: case 6: case 7:
				switch (ti_83p->it_state) {
					// Timer 2
					case 0: case 1:
						ti_83p->it_active |= ti_83p->it_mask & ((((ti_83p->it_mask & 0x08) || !z80->hlt)) ? 0x04 : 0x00);
					break;
					// Timer 1
					case 2:
						ti_83p->it_active |= ti_83p->it_mask & ((((ti_83p->it_mask & 0x08) || !z80->hlt)) ? 0x02 : 0x00);
					break;
					// Resetting counter
					case 3:
						ti_83p->it_cnt -= ti_83p->it_next;
					break;
				}
				ti_83p->it_state = (ti_83p->it_state + 1) & 0x03;
				ti_83p->it_next = ti_83p->it_times[ti_83p->it_state];
				if (!ti_83p->it_active) break;
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
int ti_83p_send_file(char *fname, BYTE **partner, BYTE *it_active, BYTE it_mask) {
	FILE *fp;
	int i, err = 0;
	WORD lgt, type, cs;
	BYTE dat[0x10000];
	BYTE *psave;

	// Plugging into the file transfer interface
	psave = *partner;
	*partner = &emu->link_state;

	// Triggering link port interrupt
	*it_active |= it_mask & 0x10;

	fp = fopen(fname, "rb");
	fseek(fp, 0x39, SEEK_SET);

	// Sending a whole group
	while (!feof(fp)) {
		lgt = 0xffff;
		fread(&lgt, 2, 1, fp);
		if (lgt == 0xffff) break;
		type = 0; fread(&type, 1, 1, fp);

		// Silent link RTS
		dat[0] = 0x23; dat[1] = 0xc9; dat[2] = 0x0d; dat[3] = 0x00;
		dat[4] = lgt; dat[5] = lgt >> 8; dat[6] = type;
		fread(dat + 7, 12, 1, fp);
		for (i = 4, cs = 0; i < 17; i++) cs += dat[i];
		dat[17] = cs; dat[18] = cs >> 8;
		if (send_data(dat, 19)) { err = 1; break; }

		// ACK and CTS from calc
		if (recv_byte() != 0x73) { err = 1; break; }
		if (recv_byte() != 0x56) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }

		if (recv_byte() != 0x73) { err = 1; break; }
		if (recv_byte() != 0x09) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }
		if (recv_byte() == -1) { err = 1; break; }

		// ACK and data block
		dat[1] = 0x56; dat[2] = 0x00; dat[4] = 0x23; dat[5] = 0x15;
		dat[6] = lgt; dat[7] = lgt >> 8;
		fread(dat + 8, lgt + 2, 1, fp);
		for (i = 0, cs = 0; i < lgt; i++) cs += dat[i + 8];
		dat[lgt + 8] = cs; dat[lgt + 9] = cs >> 8;
		calculator_run_timed(100);
		if (send_data(dat, lgt + 10)) { err = 1; break; }

		// ACK from calc
		if (recv_byte() != 0x73) { err = 1; break; }
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

	// Plugging back into the original partner
	*partner = psave;

	return err;
}

// Sending an application (forced loading)
int ti_83p_send_app(char *fname) {
	FILE *fp;
	int i, pnum, adr, ofs, err = 0;
	WORD lgt;
	BYTE line[0x100], app[0x80000], dat[0x100], hex[0x100], *blk;

	// Setting up hex LUT
	memset(hex, 0x00, 0x100);
	for (i = 0; i < 10; i++) hex['0' + i] = i;
	for (i = 0; i < 6; i++) hex['a' + i] = hex['A' + i] = i + 10;

	fp = fopen(fname, "rb");
	fread(dat, 1, 1, fp);
	if (dat[0] == '*') fseek(fp, 0x3f, SEEK_SET);

	// Loading the whole app before sending
	memset(app, 0xff, 0x80000);
	blk = app;
	pnum = 0;
	while (!feof(fp)) {
		for (i = 0; ; i++) {
			line[i] = fgetc(fp);
			if ((line[i] == ':') || feof(fp)) {
				line[i] = 0;
				break;
			}
		}
		// Chunk formats:
		// :llaaaatt....cc - ll - length, aaaa - address, tt - type, cc - checksum
		// tt = 0 -> data, ll <= 0x20, aaaa = target, ll bytes of data
		// tt = 1 -> end, ll = 0x00, aaaa = 0x0000
		// tt = 2 -> new page, ll = 0x02, aaaa = 0x0000, data[1] = relative page
		if (line[2] != '0') {
			lgt = hex[line[0]] * 16 + hex[line[1]];
			for (i = 0; i < lgt; i++) *(blk++) = hex[line[i * 2 + 8]] * 16 + hex[line[i * 2 + 9]];
		} else {
			blk = app + (pnum << 14);
			if (line[1] == '0') break;
			pnum++;
		}
	}
	fclose(fp);

	// No pages could be extracted from the file
	if (!pnum) { err = 1; goto leave; }

	// Forced placement - starting from the top (0x54000 -> 0x30000),
	// defragmenting done by the OS!
	ofs = 0x2c000 + (pnum << 14);
	for (adr = 0x54000; (ti_83p->rom[adr] != 0xff) && (adr >= ofs); adr -= 0x4000) {
		// App with the same name found
		if (!strncmp(ti_83p->rom + adr + 0x12, app + 0x12, 8)) break;
	}

	// Not enough free archive found
	if (adr < ofs) { err = 1; goto leave; }

	// Forced loading
	for (i = 0; i < pnum; i++) {
		memcpy(ti_83p->rom + adr, app + (i << 14), 0x4000);
		ti_83p->run_lock[adr >> 14] = 0;
		adr -= 0x4000;
	}
/*
	int page;
	WORD cs;

	// Triggering link port interrupt
	ti_83p->it_active |= ti_83p->it_mask & 0x10;

	// Data ready, sending
	memset(dat, 0x00, 0x100);

	// Application identifier (silent link)
	dat[0] = 0x23; dat[1] = 0x2d;
	if (send_data(dat, 4)) { err = 1; goto leave; }

	// ACK from calc
	if (recv_byte() != 0x73) { err = 1; goto leave; }
	if (recv_byte() != 0x56) { err = 1; goto leave; }
	if (recv_byte() != 0x00) { err = 1; goto leave; }
	if (recv_byte() != 0x00) { err = 1; goto leave; }

	// Continue
	dat[1] = 0x09;
	calculator_run_timed(100);
	if (send_data(dat, 4)) { err = 1; goto leave; }

	// ACK from calc
	if (recv_byte() != 0x73) { err = 1; goto leave; }
	if (recv_byte() != 0x56) { err = 1; goto leave; }
	if (recv_byte() != 0x00) { err = 1; goto leave; }
	if (recv_byte() != 0x00) { err = 1; goto leave; }

	for (i = 0; i < 17; i++) if (recv_byte() == -1) { err = 1; goto leave; }

	// Double ACK to calc
	dat[1] = 0x56; dat[5] = 0x68;
	calculator_run_timed(100);
	if (send_data(dat, 8)) { err = 1; goto leave; }

	// ACK from calc
	if (recv_byte() != 0x73) { err = 1; goto leave; }
	if (recv_byte() != 0x56) { err = 1; goto leave; }
	if (recv_byte() != 0x00) { err = 1; goto leave; }
	if (recv_byte() != 0x00) { err = 1; goto leave; }

	// Sending the pages one by one
	for (page = 0; page < pnum; page++) {
		blk = app + (page << 14);

		for (ofs = 0x4000; ofs < 0x8000; ofs += 0x80) {
			// Flash page header to calc
			dat[1] = 0x06; dat[2] = 0x0a; dat[3] = 0x00;
			dat[4] = 0x80; dat[5] = 0x00; dat[6] = 0x24;
			dat[7] = 0x00; dat[8] = 0x00; dat[9] = 0xff;
			dat[10] = ofs; dat[11] = ofs >> 8;
			dat[12] = page; dat[13] = page >> 8;
			for (i = 4, cs = 0; i < 14; i++) cs += dat[i];
			dat[14] = cs; dat[15] = cs >> 8;
			calculator_run_timed(100);
			if (send_data(dat, 16)) { err = 1; goto leave; }

			// ACK and CTS from calc
			if (recv_byte() != 0x73) { err = 1; goto leave; }
			if (recv_byte() != 0x56) { err = 1; goto leave; }
			if (recv_byte() == -1) { err = 1; goto leave; }
			if (recv_byte() == -1) { err = 1; goto leave; }
			if (recv_byte() != 0x73) { err = 1; goto leave; }
			if (recv_byte() != 0x09) { err = 1; goto leave; }
			if (recv_byte() == -1) { err = 1; goto leave; }
			if (recv_byte() == -1) { err = 1; goto leave; }

			// ACK and data to calc
			dat[1] = 0x56; dat[2] = 0x00; dat[4] = 0x23; dat[5] = 0x15;
			dat[6] = 0x80; dat[7] = 0x00;
			memcpy(dat + 8, blk, 0x80);
			blk += 0x80;
			for (i = 8, cs = 0; i < 136; i++) cs += dat[i];
			dat[136] = cs; dat[137] = cs >> 8;
			calculator_run_timed(100);
			if (send_data(dat, 138)) { err = 1; goto leave; }

			cs = ti_83p->key_state[3];

			if (ofs == 0x4000) {
				// Pressing 2 to garbage collect if asked
				if (!page) ti_83p->key_state[3] &= ~0x02;

				// A long wait might be necessary here
				for (i = 0; (i < 100000000) && (ti_83p->link_state == 3); i += 1000) {
					calculator_run_timed(1000);
				}
			}

			// ACK from calc
			if (recv_byte() != 0x73) { err = 1; goto leave; }
			ti_83p->key_state[3] = cs;
			if (recv_byte() != 0x56) { err = 1; goto leave; }
			if (recv_byte() == -1) { err = 1; goto leave; }
			if (recv_byte() == -1) { err = 1; goto leave; }
		}
	}

	// EOT to calc
	dat[1] = 0x92;
	calculator_run_timed(100);
	if (send_data(dat, 4)) { err = 1; goto leave; }
*/
	leave:

	// Finished transfer
	emu->link_state = 3;

	return err;
}

// Replacing the ROM and resetting
void ti_83p_load_rom(const char *fname) {
	FILE *fp;

	fp = fopen(fname, "rb");
	fread(ti_83p->rom, 0x80000, 1, fp);
	fclose(fp);

	ti_83p_reset();
}

// ROM page swapping
void ti_83p_swap_rom_page(BYTE bank_a, BYTE bank_b) {
	BYTE *p1, *p2;
	BYTE m1, m2, e1, e2;

	ti_83p->bank_a = bank_a;
	ti_83p->bank_b = bank_b;

	// Bank A
	if (bank_a & 0x40) {
		// RAM page activated
		if ((bank_a & 0x1f) > 1) {
			p1 = invalid_page;
			m1 = 0;
		} else {
			p1 = ti_83p->ram + ((bank_a & 0x01) << 14);
			m1 = 1;
		}
		e1 = ti_83p->run_lock[(bank_a & 0x01) | 0x20];
	} else {
		// ROM page activated
		p1 = ti_83p->rom + ((bank_a & 0x1f) << 14);
		m1 = 0;
		e1 = ti_83p->run_lock[bank_a & 0x1f];
	}

	// Bank B
	if (bank_b & 0x40) {
		// RAM page activated
		if ((bank_b & 0x1f) > 1) {
			p2 = invalid_page;
			m2 = 0;
		} else {
			p2 = ti_83p->ram + ((bank_b & 0x01) << 14);
			m2 = 1;
		}
		e2 = ti_83p->run_lock[(bank_b & 0x01) | 0x20];
	} else {
		// ROM page activated
		p2 = ti_83p->rom + ((bank_b & 0x1f) << 14);
		m2 = 0;
		e2 = ti_83p->run_lock[bank_b & 0x1f];
	}

	// Setting memory map
	if (ti_83p->mmap) {
		ti_83p->page[1] = ti_83p->ram;
		ti_83p->page[2] = p1;
		ti_83p->page[3] = p2;
		ti_83p->mut[1] = 1;
		ti_83p->mut[2] = m1;
		ti_83p->mut[3] = m2;
		ti_83p->exc[1] = ti_83p->run_lock[0x20];
		ti_83p->exc[2] = e1;
		ti_83p->exc[3] = e2;
	} else {
		ti_83p->page[1] = p1;
		ti_83p->page[2] = p2;
		ti_83p->page[3] = ti_83p->ram;
		ti_83p->mut[1] = m1;
		ti_83p->mut[2] = m2;
		ti_83p->mut[3] = 1;
		ti_83p->exc[1] = e1;
		ti_83p->exc[2] = e2;
		ti_83p->exc[3] = ti_83p->run_lock[0x20];
	}
}

// TI-83 plus memory write
void ti_83p_write(WORD adr, BYTE val) {
	BYTE tmp;

	if (ti_83p->mut[(tmp = adr >> 14)]) {
/*		extern int do_log;
		if (do_log && (ti_83p->page[tmp] - ti_83p->ram >= 0x4000))
			fprintf(logfp, "%08x %02x \"%c\"\n", ti_83p->page[tmp] - ti_83p->ram + (adr & 0x3fff), val, val);*/
		ti_83p->page[tmp][adr & 0x3fff] = val;
	}
	else {
/*		if ((adr != 0x6aaa) && (adr != 0x5555)) {
			fprintf(logfp, "%d %02X %02X %04X flash %04X %02X\n", ti_83p->mmap, ti_83p->bank_a, ti_83p->bank_b, R_PC, adr, val);
			fflush(logfp);
		}*/
		if (ti_83p->flash_lock & 1) {
/*			extern int do_log;
			if (do_log && (ti_83p->page[tmp] - ti_83p->rom > 0x10000))
				fprintf(logfp, "%08x %02x \"%c\"\n", ti_83p->page[tmp] - ti_83p->rom + (adr & 0x3fff), val, val);*/
			flash_write(ti_83p->page[tmp] - ti_83p->rom + (adr & 0x3fff), val);
		}
	}
}

// TI-83 plus memory read
BYTE ti_83p_read(WORD adr) {
	BYTE tmp;
	BYTE retval = ti_83p->page[tmp = adr >> 14][adr & 0x3fff];
	int pa = ti_83p->page[tmp] - ti_83p->rom;
/*	extern int do_log;
	if (do_log && (pa >= 0x20000) && (pa < 0x60000))
		fprintf(logfp, "%08x %02x \"%c\"\n", pa + (adr & 0x3fff), retval, retval);*/
	// If code is executed on a locked page, the calculator must be reset
	if ((adr == R_PC) && ti_83p->exc[tmp]) z80->hlt = 2;
	// The protection buffer is filled with valid values only from pages 0x1c-0x1f
	ti_83p->prot_buffer[ti_83p->prot_cnt++] = ((pa >= 0x70000) && (pa < 0x80000)) ? retval : 0xff;
	ti_83p->prot_cnt &= 0x07;
	return retval;
}

// TI-83 plus memory read without side effects
BYTE ti_83p_rmem(WORD adr) {
	return ti_83p->page[adr >> 14][adr & 0x3fff];
}

/*

Ports:

00: link: 0-1: common; 5-6: outgoing; 2: unknown
01: keyboard
02: R - status: 0: battery; 1: lcd ready (sampled); 5: set for 84*; 7: reset for 83+
    W - nothing?
03: it mask: 0-3: like 83, but mask, not state; 4: link port
04: R - it & on key state: 0-4: evident; 5-7: others only
    W - paging and timer freq: see 83; normal map: ROM0 BNKA BNKB RAM0 (PRT5 on non-83+)
05: not on the 83+: ram paging: values: 0-7
06: bank A mapping: ROM: 00-1f; RAM: 40-41 (others otherwise)
07: bank B mapping: same
10-11: LCD
14: not on the 83+?: flash lock
*/

// TI-83 plus output port
void ti_83p_out(BYTE port, BYTE val) {
/*	if ((port != 0x02) && (port != 0x06) && (port != 0x10) && (port != 0x11)) {
		fprintf(logfp, "%d %02X %02X %04X out %02X %02X\n", ti_83p->mmap, ti_83p->bank_a, ti_83p->bank_b, R_PC, port, val);
		fflush(logfp);
	}*/
	// Note that bit 3 is masked, so everything is mirrored at x+8
	switch (port & 0x17) {
		// Linking
		case 0x00: // case 0x08:
			ti_83p->link_state = (~val) & 0x03;
		break;
		// Keyboard mask
		case 0x01: // case 0x09:
			if (val == 0xff)
				ti_83p->key_mask = 0xff;
			else
				ti_83p->key_mask &= val;
		break;
		// Nothing?
		case 0x02: // case 0x0A:
		break;
		// IT mask
		case 0x03: // case 0x0B:
			// Resetting bit 3 shuts down the LCD and the timers on the next halt
			ti_83p->it_mask = val & 0x1f;
			ti_83p->it_active &= val;
		break;
		// IT frequency + paging mode
		case 0x04: // case 0x0C:
			// IT period
			ti_83p->it_times[3] = CPUSPEED * (3 + ((val & 0x06) << 1)) / 1620;
			// First firing of timer 2
			ti_83p->it_times[0] = ti_83p->it_times[3] >> 1;
			// Second firing of timer 2
			ti_83p->it_times[1] = ti_83p->it_times[0] + 1600;
			// Firing of timer 1
			ti_83p->it_times[2] = ti_83p->it_times[1] + 1200;
			// Bit 4 does not affect the frequency any more!
			ti_83p->it_next = ti_83p->it_times[ti_83p->it_state];
			// Bit 0 affects paging
			ti_83p->mmap = val & 1;
			ti_83p_swap_rom_page(ti_83p->bank_a, ti_83p->bank_b);
			// Bits 5-7 are related to power management, no emulation is necessary
			// (virtual batteries are always fresh :)
		break;
		// Execution protection selector
		case 0x05: // case 0x0D:
			ti_83p->flash_lock = (ti_83p->flash_lock & 0x01) | ((val & 0x07) << 1);
		break;
		// Mapping memory bank A
		case 0x06: // case 0x0E:
			ti_83p_swap_rom_page(val & 0x7f, ti_83p->bank_b);
		break;
		// Mapping memory bank B
		case 0x07: // case 0x0F:
			ti_83p_swap_rom_page(ti_83p->bank_a, val & 0x7f);
		break;
		// LCD command port
		case 0x10: case 0x12: // case 0x18: case 0x1A:
			lcd_command(val);
		break;
		// LCD data port
		case 0x11: case 0x13: // case 0x19: case 0x1B:
			lcd_write(val);
		break;
		// Flash lock
		case 0x14: { // case 0x1C:
			if (ti_83p_protection(0x14))
				ti_83p->flash_lock = (ti_83p->flash_lock & 0xfe) | (val & 0x01);
		}
		break;
		// Execution protection
		case 0x16: // case 0x1E:
			if ((ti_83p->flash_lock & 0x01) && ti_83p_protection(0x16)) {
				int i;
				switch (ti_83p->flash_lock & 0x0e) {
					case 0x00:
						// Locking ROM pages 0x08-0x0f
						for (i = 0; i < 8; i++) ti_83p->run_lock[0x08 + i] = (val >> i) & 0x01;
					break;
					case 0x02:
						// Locking ROM pages 0x10-0x17
						for (i = 0; i < 8; i++) ti_83p->run_lock[0x10 + i] = (val >> i) & 0x01;
					break;
					case 0x04:
						// Locking ROM pages 0x18-0x1b
						for (i = 0; i < 4; i++) ti_83p->run_lock[0x18 + i] = (val >> i) & 0x01;
					break;
					case 0x0e:
						// Locking RAM pages
						for (i = 0; i < 2; i++) ti_83p->run_lock[0x20 + i] = (val >> (i * 5)) & 0x01;
					break;
				}
				ti_83p_swap_rom_page(ti_83p->bank_a, ti_83p->bank_b);
			}
		break;
	}
}

// TI-83 plus input port
BYTE ti_83p_in(BYTE port) {
	BYTE retval = 0;
	// Note that bit 3 is masked, so everything is mirrored at x+8
	switch (port & 0x17) {
		// Linking
		case 0x00: // case 0x08:
			retval =
				(ti_83p->link_state & *(ti_83p->partner_link)) |
				((ti_83p->link_state ^ 0x03) << 4);
		break;
		// Keyboard state
		case 0x01: { // case 0x09:
			int i;
			retval = 0xff;
			for (i = 0; i < 7; i++)
				if (!(ti_83p->key_mask & (1 << i))) retval &= ti_83p->key_state[i];
		}
		break;
		// Status
		case 0x02: // case 0x0A:
			retval =
				0x03 | // Good batteries and bit 2 set
				((ti_83p->flash_lock << 2) & 0x3c); // Flash lock state
		break;
		// IT mask
		case 0x03: // case 0x0B:
			retval = ti_83p->it_mask; // Current IT mask
		break;
		// IT status
		case 0x04: // case 0x0C:
			retval =
				ti_83p->it_active | // ON key, link port and timer interrupts
				((ti_83p->on_key ^ 1) << 3); // ON key state
		break;
		// Memory bank A
		case 0x06: // case 0x0E:
			retval = ti_83p->bank_a;
		break;
		// Memory bank B
		case 0x07: // case 0x0F:
			retval = ti_83p->bank_b;
		break;
		// LCD status port
		case 0x10: case 0x12: // case 0x18: case 0x1A:
			retval = lcd_status();
		break;
		// LCD data port
		case 0x11: case 0x13: // case 0x19: case 0x1B:
			retval = lcd_read();
		break;
		// Blank ports
		case 0x14: case 0x15: case 0x17: // case 0x1C: case 0x1D: case: 0x1F
			retval = 0xff;
		break;
		// Execution protection?
		case 0x16: { // case 0x1E:
			BYTE p1 = (ti_83p->page[((R_PC - 1) >> 14) & 0x03] - ti_83p->rom) >> 14;
			BYTE p2 = (ti_83p->page[((R_PC - 2) >> 14) & 0x03] - ti_83p->rom) >> 14;
			retval = ((p1 >= 0x1c) && (p1 <= 0x1f) && (p2 >= 0x1c) && (p2 <= 0x1f)) ? 0xfe : 0xff;
		}
		break;
	}

/*	if ((port != 0x02) && (port != 0x06) && (port != 0x10) && (port != 0x11)) {
		fprintf(logfp, "%d %02X %02X %04X in %02X %02X\n", ti_83p->mmap, ti_83p->bank_a, ti_83p->bank_b, R_PC, port, retval);
		fflush(logfp);
	}*/
	return retval;
}

// TI-83 plus port protection
int ti_83p_protection(BYTE port) {
	// This solution doesn't allow mirrors
	BYTE il[7] = { 0x00, 0x00, 0xed, 0x56, 0xf3, 0xd3, port };
	int i, ok;
	for (ok = 1, i = 0; i < 7; i++)
		ok &= (il[i] == ti_83p->prot_buffer[(ti_83p->prot_cnt + i + 1) & 0x07]);
	return ok;
}

// Loading a VTI saved state with the current ROM
void ti_83p_load_vti_sav(const char *fname) {
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
	fread(ti_83p->ram + 0x4000, 0x4000, 1, fp);
	fread(ti_83p->ram, 0x4000, 1, fp);

	// Miscellaneous pieces of hardware
	fread(tmp, 0x354, 1, fp);
	ti_83p->key_mask = tmp[0x300];
	ti_83p->mmap = 0;
//	ti_83p_swap_rom_page(tmp[0x308] & 0x0f);
	// VTI apparently doesn't implement timer masking, so it's always enabled
	ti_83p->it_mask = 0x0e | (tmp[0x0340] & 1);

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
#define l83PC(x) ti_83p->x = fgetc(lfp)
#define l83PI(x) \
	ti_83p->x = fgetc(lfp) << 24; \
	ti_83p->x += fgetc(lfp) << 16; \
	ti_83p->x += fgetc(lfp) << 8; \
	ti_83p->x += fgetc(lfp);

#define s83PC(x) fputc(ti_83p->x, sfp)
#define s83PI(x) \
	fputc(ti_83p->x >> 24, sfp); \
	fputc(ti_83p->x >> 16, sfp); \
	fputc(ti_83p->x >> 8, sfp); \
	fputc(ti_83p->x, sfp)

#define SER83P(_ls,ls,rw) \
	emu_ ## _ls(ls ## fp); \
	z80_ ## _ls(ls ## fp); \
	lcd_ ## _ls(ls ## fp); \
	ls ## 83PC(bank_a); ls ## 83PC(bank_b); ls ## 83PC(link_state); ls ## 83PC(on_key); \
	ls ## 83PC(it_mask); ls ## 83PC(it_active); ls ## 83PI(it_cnt); ls ## 83PI(it_next); ls ## 83PC(it_state); \
	ls ## 83PI(it_times[0]); ls ## 83PI(it_times[1]); ls ## 83PI(it_times[2]); ls ## 83PI(it_times[3]); \
	ls ## 83PC(key_mask); ls ## 83PC(mmap); ls ## 83PC(flash_lock); ls ## 83PI(flash.phase); ls ## 83PC(prot_cnt); \
	f ## rw(ti_83p->run_lock, 0x22, 1, ls ## fp); \
	f ## rw(ti_83p->key_state, 7, 1, ls ## fp); \
	f ## rw(ti_83p->key_map, 0x80, 1, ls ## fp); \
	f ## rw(ti_83p->ram, 0x8000, 1, ls ## fp); \
	f ## rw(ti_83p->prot_buffer, 8, 1, ls ## fp)

// Loading TI-83+ state from <romfile>.pti
int ti_83p_load_state(char *romfile) {
	FILE *lfp;
	char fn[MAX_PATH + 5];

	ti_83p_load_rom(romfile);
	sprintf(fn, "%s.pti", romfile);
	if (!(lfp = fopen(fn, "rb"))) return 0;
	SER83P(load, l, read);
	fclose(lfp);
	// Restoring things that couldn't be saved
	ti_83p_swap_rom_page(ti_83p->bank_a, ti_83p->bank_b);

	return 0;
}

// Saving TI-83+ state to <romfile>.pti
int ti_83p_save_state(char *romfile) {
	FILE *sfp;
	char fn[MAX_PATH + 5];

	sprintf(fn, "%s.pti", romfile);
	sfp = fopen(fn, "wb");
	SER83P(save, s, write);
	fclose(sfp);

	sfp = fopen(romfile, "wb");
	fwrite(ti_83p->rom, 0x80000, 1, sfp);
	fclose(sfp);

	return 0;
}


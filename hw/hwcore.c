#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hwcore.h"
#include "../debug/dz80.h"

#define LINKDELAY 1000000
#define EXISTING_MODELS 4
#define RETERROR(emsg) do { sprintf(error_msg, emsg); return 1; } while (0)

CALCULATOR_RUN_FN calculator_run;
CALCULATOR_RUN_FN calculator_run_linked;
CALCULATOR_ON_FN calculator_powered;
HANDLE_KEY_FN handle_key;
CALCULATOR calc[MAX_CALC];
CALCULATOR_STATE_FN calculator_load;
CALCULATOR_STATE_FN calculator_save;
BYTE invalid_page[0x4000];
int running_calculator;
LINKING linking;

CALCULATOR cmodels[EXISTING_MODELS] = {
	{ 0, FILE_MODEL_82, "", NULL, NULL, NULL, NULL, ti_82_run, ti_82_run_linked, ti_82_powered, ti_82_key,
		ti_82_write, ti_82_read, ti_82_out, ti_82_in, ti_82_read, ti_82_load_state, ti_82_save_state },
	{ 0, FILE_MODEL_82b, "", NULL, NULL, NULL, NULL, ti_82_run, ti_82_run_linked, ti_82_powered, ti_82_key,
		ti_82_write, ti_82_read, ti_82_out, ti_82_in, ti_82_read, ti_82_load_state, ti_82_save_state },
	{ 0, FILE_MODEL_83, "", NULL, NULL, NULL, NULL, ti_83_run, ti_83_run_linked, ti_83_powered, ti_83_key,
		ti_83_write, ti_83_read, ti_83_out, ti_83_in, ti_83_read, ti_83_load_state, ti_83_save_state },
	{ 0, FILE_MODEL_83P, "", NULL, NULL, NULL, NULL, ti_83p_run, ti_83p_run_linked, ti_83p_powered, ti_83p_key,
		ti_83p_write, ti_83p_read, ti_83p_out, ti_83p_in, ti_83p_rmem, ti_83p_load_state, ti_83p_save_state }
};

void hw_init(int flags) {
	int i;
	FILE *fp;
	char tmp[MAX_PATH + 3];

	z80_prepare();
	for (i = 0; i < MAX_CALC; i++) {
		calc[i].flags = 0;
		calc[i].rom_ver = -1;
	}
	running_calculator = -1;

	// Dummy memory page needed for some models
	memset(invalid_page, 0xff, 0x4000);

	// Linking history
	linking.recp = 0;
	memset(linking.rec, 0x03, MAX_CALC * LINK_HISTORY);

	if (!(flags & INIT_CONFIG) || !(fp = fopen("pti.conf", "r"))) return;
	while (!feof(fp)) {
		fgets(tmp, MAX_PATH + 2, fp);
		if (tmp[1] == ':') {
			for (i = 0; tmp[i]; i++) if (tmp[i] < ' ') tmp[i] = 0;
			process_file(tmp + 2, atoi(tmp));
		}
	}
	fclose(fp);
}

void hw_deinit(int flags) {
	int i;
	FILE *fp;

	if (!(flags & INIT_CONFIG)) return;
	fp = fopen("pti.conf", "w");
	for (i = 0; i < MAX_CALC; i++) {
		fprintf(fp, "%d:%s\n", i, calc[i].rom_path);
		free_calculator(i);
	}
	fclose(fp);
}

int create_calculator(int model, int slot) {
	int i;

	if ((slot < 0) || (slot >= MAX_CALC)) return 1;
	if (calc[slot].rom_ver != -1) free_calculator(slot);
	for (i = 0; i < EXISTING_MODELS; i++) if (cmodels[i].rom_ver == (model & FILE_MODEL_MASK)) break;
	if (i == EXISTING_MODELS) return 1;

	calc[slot] = cmodels[i];
	calc[slot].rom_ver = model;
	calc[slot].emu = malloc(sizeof(EMU_CORE));
	calc[slot].z80 = malloc(sizeof(Z80_CPU));
	calc[slot].lcd = malloc(sizeof(LCD));

	switch (model & FILE_MODEL_MASK) {
		case FILE_MODEL_82:
		case FILE_MODEL_82b: {
			calc[slot].model = malloc(sizeof(MODEL_TI82));
			calc[slot].emu->it_num = 1;
			calc[slot].emu->it_cnt = &((MODEL_TI82*)calc[slot].model)->it_cnt;
			((MODEL_TI82*)calc[slot].model)->partner_link = &calc[slot].emu->link_state;
			calc[slot].emu->partner_link = &((MODEL_TI82*)calc[slot].model)->link_state;
		}
		break;
		case FILE_MODEL_83: {
			calc[slot].model = malloc(sizeof(MODEL_TI83));
			calc[slot].emu->it_num = 1;
			calc[slot].emu->it_cnt = &((MODEL_TI83*)calc[slot].model)->it_cnt;
			((MODEL_TI83*)calc[slot].model)->partner_link = &calc[slot].emu->link_state;
			calc[slot].emu->partner_link = &((MODEL_TI83*)calc[slot].model)->link_state;
		}
		break;
		case FILE_MODEL_83P: {
			calc[slot].model = malloc(sizeof(MODEL_TI83P));
			calc[slot].emu->it_num = 1;
			calc[slot].emu->it_cnt = &((MODEL_TI83P*)calc[slot].model)->it_cnt;
			((MODEL_TI83P*)calc[slot].model)->partner_link = &calc[slot].emu->link_state;
			calc[slot].emu->partner_link = &((MODEL_TI83P*)calc[slot].model)->link_state;
		}
		break;
	}

	return 0;
}

int activate_calculator(int slot) {
	if ((slot < 0) || (slot >= MAX_CALC) || (calc[slot].rom_ver == -1)) return 1;

	emu = calc[slot].emu;
	z80 = calc[slot].z80;
	lcd = calc[slot].lcd;
	calculator_run = calc[slot].calc_run;
	calculator_run_linked = calc[slot].calc_run_linked;
	calculator_powered = calc[slot].calc_powered;
	handle_key = calc[slot].handle_key;
	z80_write = calc[slot].z80_write;
	z80_read = calc[slot].z80_read;
	z80_out = calc[slot].z80_out;
	z80_in = calc[slot].z80_in;
	z80_acc = calc[slot].z80_acc;
	calculator_load = calc[slot].calc_load;
	calculator_save = calc[slot].calc_save;
	switch (calc[slot].rom_ver & FILE_MODEL_MASK) {
		case FILE_MODEL_82:
		case FILE_MODEL_82b:
			ti_82 = calc[slot].model;
		break;
		case FILE_MODEL_83:
			ti_83 = calc[slot].model;
		break;
		case FILE_MODEL_83P:
			ti_83p = calc[slot].model;
			flash = &((MODEL_TI83P*)calc[slot].model)->flash;
		break;
	}

	running_calculator = slot;

	return 0;
}

void free_calculator(int slot) {
	if ((slot < 0) || (slot >= MAX_CALC) || (calc[slot].rom_ver == -1)) return;
	if (!activate_calculator(slot)) calculator_save(calc[slot].rom_path);
	calc[slot].flags = 0;
	calc[slot].rom_ver = -1;
	calc[slot].rom_path[0] = 0;
	free(calc[slot].emu);
	free(calc[slot].z80);
	free(calc[slot].lcd);
	free(calc[slot].model);
}

int process_file(char *fname, int slot) {
	int i, file_type = detect_file_type(fname);

	if ((slot < 0) || (slot >= MAX_CALC)) return 1;
	switch (file_type & FILE_TYPE_MASK) {
		case FILE_TYPE_ROM:
			for (i = 0; i < MAX_CALC; i++)
				if (!(strcmp(fname, calc[i].rom_path))) {
					if (i == slot) return 0; // Nothing to do (already running ROM image)
					else RETERROR("You must make a copy of the ROM image if you want\n\rto use more than one instance at the same time.");
				}
			create_calculator(file_type, slot);
			activate_calculator(slot);
			strcpy(calc[slot].rom_path, fname);
			switch (file_type & FILE_MODEL_MASK) {
				case FILE_MODEL_82: ti_82->model = 0; ti_82_load_state(fname); break;
				case FILE_MODEL_82b: ti_82->model = 1; ti_82_load_state(fname); break;
				case FILE_MODEL_83: ti_83_load_state(fname); break;
				case FILE_MODEL_83P: ti_83p_load_state(fname); break;
				default: RETERROR("Unsupported ROM image.");
			}
			break;
		case FILE_TYPE_CALC: {
			// TODO: extract data first, then use the native sender.
			BYTE **partner = NULL;
			BYTE *key_state = NULL;
			sprintf(error_msg, "Transmission error.");
			// Sometimes 83+ files are in 83 format, so it's
			// a special model mismatch that can be allowed
			if (((file_type ^ calc[slot].rom_ver) & FILE_PROT_MASK) &&
				(((file_type ^ FILE_PROT_83) | (calc[slot].rom_ver ^ FILE_PROT_83P)) & FILE_PROT_MASK))
				RETERROR("This file doesn't match the running model.");
			activate_calculator(slot);
			switch (calc[slot].rom_ver & FILE_MODEL_MASK) {
				case FILE_MODEL_82:
				case FILE_MODEL_82b:
					partner = &ti_82->partner_link;
					key_state = ti_82->key_state;
					break;
				case FILE_MODEL_83:
					partner = &ti_83->partner_link;
					key_state = ti_83->key_state;
					break;
				case FILE_MODEL_83P:
					partner = &ti_83p->partner_link;
					key_state = ti_83p->key_state;
					break;
			}
			switch (file_type & FILE_PROT_MASK) {
				case FILE_PROT_82:
					if (ti_82_send_file(fname, partner, key_state)) return 1;
					break;
				case FILE_PROT_83:
					if (ti_83_send_file(fname, partner)) return 1;
					break;
				case FILE_PROT_83P:
					if (ti_83p_send_file(fname, partner, &ti_83p->it_active, ti_83p->it_mask)) return 1;
					break;
				default: RETERROR("Unsupported calc file type.");
			}
			break;
		}
		case FILE_TYPE_APP:
			sprintf(error_msg, "Transmission error.");
			if ((file_type ^ calc[slot].rom_ver) & FILE_PROT_MASK)
				RETERROR("This file doesn't match the running model.");
			activate_calculator(slot);
			switch (file_type & FILE_PROT_MASK) {
				case FILE_PROT_83P: if (ti_83p_send_app(fname)) return 1; break;
				default: RETERROR("Unsupported calc application type.");
			}
			break;
		case FILE_TYPE_VTI_SAV:
			// It's impossible to tell whether a VTI save file is of
			// version 19.0 or 19.006 in the case of the 82, so it is
			// allowed to mix the two (the emulated calc will hang though).
			if (((file_type ^ calc[slot].rom_ver) & FILE_ROM_MASK) &&
				(((file_type ^ FILE_ROM_IMAGE_82_19_0) |
				(calc[slot].rom_ver ^ FILE_ROM_IMAGE_82_19_006)) & FILE_ROM_MASK))
				RETERROR("This save file doesn't match the running model.");
			activate_calculator(slot);
			switch (file_type & FILE_PROT_MASK) {
				case FILE_PROT_82: ti_82_load_vti_sav(fname); break;
				case FILE_PROT_83: ti_83_load_vti_sav(fname); break;
				default: RETERROR("Unsupported VTI save state.");
			}
			break;
		default: RETERROR("Unknown file type.");
	}

	return 0;
}

// Running for a certain time
void calculator_run_timed(int t) {
	int p = emu->stop_period;
	if (emu->stop_cnt + t >= p) {
		t -= p - emu->stop_cnt;
		calculator_run();
		if (debug_trapped) return;
		lcd_update();
		emu->stop_cnt -= p;
		while (emu->stop_cnt + t >= p) {
			calculator_run();
			if (debug_trapped) return;
			lcd_update();
			emu->stop_cnt -= p;
			t -= p;
		}
	}
	emu->stop_period = emu->stop_cnt + t;
	calculator_run();
	emu->stop_period = p;
}

// Advancing all the active slots by a frame
void run_all_slots(int *update_needed) {
	int s, linked, recpo;
	for (linked = 0, s = 0; s < 4; s++)
		linked += (calc[s].flags & CALC_FLAG_LINK) > 0;

	// Handling calcs linked to the hub
	if (linked > 0) {
		BYTE dummy = 0x03;
		int cmin, cmax, shub;
		BYTE *lstates[4];
		for (s = 0; s < 4; s++)
			if (calc[s].flags & CALC_FLAG_LINK) {
				switch (calc[s].rom_ver & FILE_MODEL_MASK) {
					case FILE_MODEL_82: case FILE_MODEL_82b:
						lstates[s] = &((MODEL_TI82*)calc[s].model)->link_state;
						break;
					case FILE_MODEL_83:
						lstates[s] = &((MODEL_TI83*)calc[s].model)->link_state;
						break;
					case FILE_MODEL_83P:
						lstates[s] = &((MODEL_TI83P*)calc[s].model)->link_state;
						break;
					default:
						lstates[s] = &dummy;
				}
			} else lstates[s] = &dummy;
		recpo = linking.recp > 0 ? linking.recp - 1 : LINK_HISTORY - 1;
		for (linking.time = 0, cmin = 0; cmin < emu->stop_period; linking.time = cmin + 1) {
			for (cmax = 0, cmin = emu->stop_period, s = 0; s < 4; s++)
				if ((calc[s].flags & CALC_FLAG_LINK) && !activate_calculator(s)) {
					for (shub = 0, linking.hub = 0x03; shub < 4; shub++)
						if (shub != s) linking.hub &= *lstates[shub];
					calculator_run_linked();
					if (emu->stop_cnt > cmax) cmax = emu->stop_cnt;
					if (emu->stop_cnt < cmin) cmin = emu->stop_cnt;
					linking.rec[s][linking.recp] = *lstates[s];
				} else linking.rec[s][linking.recp] = 3;
				for (shub = 0; shub < MAX_CALC; shub++) {
					if (linking.rec[shub][recpo] != linking.rec[shub][linking.recp]) {
						recpo = linking.recp;
						linking.recp++;
						if (linking.recp >= LINK_HISTORY) linking.recp = 0;
						break;
					}
				}
		}
		for (s = 0; s < 4; s++)
			if ((calc[s].flags & CALC_FLAG_LINK) && !activate_calculator(s)) {
				lcd_update();
				emu->stop_cnt -= emu->stop_period;
			}
	}

	// Handling calcs that aren't linked
	for (s = 0; s < 4; s++)
		if (((*update_needed & 1) || (calc[s].flags & CALC_FLAG_WARP)) &&
			!(calc[s].flags & CALC_FLAG_PAUSE) &&
			((linked < 2) || !(calc[s].flags & CALC_FLAG_LINK)) &&
			!activate_calculator(s)) {
			calculator_run();
			if (debug_trapped) return;
			lcd_update();
			emu->stop_cnt -= emu->stop_period;
		}
}

// Toggle mode flags and adjust additional data where needed
void calculator_toggle_flags(int slot, int flags) {
	calc[slot].flags ^= flags;

	BYTE *partner = (calc[slot].flags & CALC_FLAG_LINK) ? &linking.hub : &calc[slot].emu->link_state;
	switch (calc[slot].rom_ver & FILE_MODEL_MASK) {
		case FILE_MODEL_82: case FILE_MODEL_82b:
			((MODEL_TI82*)calc[slot].model)->partner_link = partner;
			break;
		case FILE_MODEL_83:
			((MODEL_TI83*)calc[slot].model)->partner_link = partner;
			break;
		case FILE_MODEL_83P:
			((MODEL_TI83P*)calc[slot].model)->partner_link = partner;
			break;
	}
}

// Render magnified pixel image of the current slot
void render_lcd_bitmap(BYTE *scr, int scrx, int mulfac, int x, int y, int act) {

#define SCRXSIZE 96
#define SCRYSIZE 64

	int i, j, k, c, c2, cp = act ? 128 : 0;
	int lcdx = SCRXSIZE * mulfac;
	BYTE *scp, *scpo;
	if (lcd->on && calculator_powered()) {
		scpo = scr + x + y * scrx;
		if (mulfac == 1) {
			for (i = 0; i < SCRYSIZE; i++) {
				scp = scpo;
				for (j = 0; j < SCRXSIZE; j++)
					*(scp++) = (lcd->scr[i * 120 + j] >> 9) + cp;
				scpo += scrx;
			}
		} else {
			for (i = 0; i < SCRYSIZE; i++) {
				scp = scpo;
				for (j = 0; j < SCRXSIZE; j++) {
					c = lcd->scr[i * 120 + j] >> 9;
					c2 = c * 4 / (mulfac + 3) + cp;
					c += cp;
					for (k = 0; k < mulfac - 1; k++) *(scp++) = c;
					*(scp++) = c2;
				}
				scpo += scrx;
				for (j = 1; j < mulfac - 1; j++) {
					memcpy(scpo, scpo - scrx, lcdx);
					scpo += scrx;
				}
				scp = scpo;
				for (j = 0; j < SCRXSIZE; j++) {
					c = *(scp - scrx + mulfac - 1);
					for (k = 0; k < mulfac; k++) *(scp++) = c;
				}
				scpo += scrx;
			}
		}
	} else {
		cp += 32;
		scp = scr + y * scrx + x;
		for (i = 0; i < SCRYSIZE * mulfac; i++)
			memset(scp + i * scrx, cp, lcdx);
	}
}

int send_byte(BYTE val) {
	int i, cc;

//	fprintf(logfp, "-> %02X\n", val);
//	fflush(logfp);
	for (i = 0; i < 8; i++) {
		emu->link_state = 2 - (val & 1);
		for (cc = 0; (cc < LINKDELAY) && (*(emu->partner_link) == 3); cc += 50) calculator_run_timed(50);
		if (cc >= LINKDELAY) return i + 1;
		emu->link_state = 3;
		for (cc = 0; (cc < LINKDELAY) && (*(emu->partner_link) != 3); cc += 50) calculator_run_timed(50);
		if (cc >= LINKDELAY) return i + 11;
		val >>= 1;
	}

	return 0;
}

int recv_byte() {
	int i, cc;
	BYTE rb;

	rb = 0;
	for (i = 0; i < 8; i++) {
		emu->link_state = 3;
		for (cc = 0; (cc < LINKDELAY) && (*(emu->partner_link) == 3); cc += 50) calculator_run_timed(50);
		if (cc >= LINKDELAY) return -1;
		rb = (rb >> 1) | (*(emu->partner_link) << 7);
		emu->link_state = *(emu->partner_link) ^ 3;
		for (cc = 0; (cc < LINKDELAY) && (*(emu->partner_link) != 3); cc += 50) calculator_run_timed(50);
		if (cc >= LINKDELAY) return -1;
	}
	emu->link_state = 3;

//	fprintf(logfp, "<- %02X\n", rb);
//	fflush(logfp);
	return rb;
}

int send_data(BYTE *dat, int length) {
	int i;

	for (i = 0; i < length; i++) if (send_byte(dat[i])) return 1;

	return 0;
}


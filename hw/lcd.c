// TODO: BLOD (test mode)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"

#define LOS(x) ((BYTE*)&lcd->x - (BYTE*)lcd)
#define LOD(x) LOS(x) + 3, LOS(x) + 2, LOS(x) + 1, LOS(x)

LCD *lcd;

// Write LCD command word
void lcd_command(BYTE cmd) {
	// Checking if writing is possible
	if (emu->stop_cnt < lcd->next_w) return;
	// Word length
	if ((cmd & 0xfe) == 0x00) {
		lcd->w_len = cmd & 1;
	} else
	// Turn on/off
	if ((cmd & 0xfe) == 0x02) {
		lcd->on = cmd & 1;
	} else
	// Counter select and direction
	if ((cmd & 0xfc) == 0x04) {
		lcd->up_dn = cmd & 1;
		lcd->cnt_sel = (cmd >> 1) & 1;
	} else
	// Op-amp 1
	if ((cmd & 0xf8) == 0x10) {
		lcd->amp = cmd & 3;
	} else
	// Op-amp 2
	if ((cmd & 0xf8) == 0x08) {
		lcd->amp2 = cmd & 3;
	} else
	// Test mode
	if ((cmd & 0xf8) == 0x18) {
		lcd->test = (cmd >> 2) & 1;
		lcd->ctr = 0x3f;
	} else
	// Set y-address
	if ((cmd & 0xe0) == 0x20) {
		lcd->y = cmd & 0x1f;
		lcd->dummy = 1;
	} else
	// Set z-address
	if ((cmd & 0xc0) == 0x40) {
		lcd->z = cmd & 0x3f;
	} else
	// Set x-address
	if ((cmd & 0xc0) == 0x80) {
		lcd->x = cmd & 0x3f;
		lcd->dummy = 1;
	} else
	// Set contrast
	if ((cmd & 0xc0) == 0xc0) {
		lcd->ctr = cmd & 0x3f;
	}
	// Black and white shade values
	lcd->swht = lcd->cwht + lcd->cmul * (lcd->ctr - 32) - lcd->amul * (lcd->amp + lcd->amp2);
	lcd->sblk = lcd->cblk + lcd->cmul * (lcd->ctr - 32) - lcd->amul * (lcd->amp + lcd->amp2);
	if (lcd->tjit > 1)
		lcd->next_w = emu->stop_cnt + lcd->tmin + rand() % lcd->tjit;
	else
		lcd->next_w = emu->stop_cnt + lcd->tmin;
}

// Read LCD status
BYTE lcd_status() {
	return ((emu->stop_cnt < lcd->next_w) << 7) | (lcd->w_len << 6) |
		(lcd->on << 5) | (lcd->cnt_sel << 1) | (lcd->up_dn);
}

// Write LCD data
void lcd_write(BYTE data) {
	// Checking if writing is possible
	if (emu->stop_cnt < lcd->next_w) return;
	int wl = (lcd->w_len) ? 8 : 6;
	int cn = (lcd->w_len) ? 15 : 20;
	int adr = lcd->x * 120 + lcd->y * wl;
	int i;
	// Modifying 6 or 8 bits and their shade counters
	for (i = 0; i < wl; i++) {
		if (!lcd->dat[adr]) lcd->wcnt[adr] += emu->stop_cnt - lcd->lcnt[adr];
		lcd->lcnt[adr] = emu->stop_cnt;
		lcd->dat[adr] = (data >> (wl - i - 1)) & 1;
		adr++;
	}
	// Advancing the appropriate counter
	switch ((lcd->cnt_sel << 1) | lcd->up_dn) {
		case 0: lcd->x = (BYTE)(lcd->x - 1) % 64; break;
		case 1: lcd->x = (BYTE)(lcd->x + 1) % 64; break;
		case 2: lcd->y = (BYTE)(lcd->y - 1 + cn) % cn; break;
		case 3: lcd->y = (BYTE)(lcd->y + 1) % cn; break;
	}
	if (lcd->tjit > 1)
		lcd->next_w = emu->stop_cnt + lcd->tmin + rand() % lcd->tjit;
	else
		lcd->next_w = emu->stop_cnt + lcd->tmin;
}

// Read LCD data
BYTE lcd_read() {
	// Dummy read (now returns 0, although that's incorrect)
	if (lcd->dummy) { lcd->dummy = 0; return 0; }
	int wl = (lcd->w_len) ? 8 : 6;
	int cn = (lcd->w_len) ? 15 : 20;
	int adr = lcd->x * 120 + lcd->y * wl;
	int i, v;
	v = 0;
	// Recovering a 6 or 8-bit value
	for (i = 0; i < wl; i++) v |= lcd->dat[adr + i] << (wl - i - 1);
	// Advancing the appropriate counter
	switch ((lcd->cnt_sel << 1) | lcd->up_dn) {
		case 0: lcd->x = (BYTE)(lcd->x - 1) % 64; break;
		case 1: lcd->x = (BYTE)(lcd->x + 1) % 64; break;
		case 2: lcd->y = (BYTE)(lcd->y - 1 + cn) % cn; break;
		case 3: lcd->y = (BYTE)(lcd->y + 1) % cn; break;
	}
	return v;
}

// Reset LCD
void lcd_reset() {
	int i;
	// Hardware defaults
	lcd->x = lcd->y = lcd->z = lcd->on = lcd->test = lcd->amp = lcd->amp2 = lcd->next_w = 0;
	lcd->up_dn = lcd->cnt_sel = lcd->w_len = lcd->dummy = 1;
	for (i = 0; i < 120 * 64; i++) {
		lcd->dat[i] = lcd->wcnt[i] = lcd->lcnt[i] = 0;
		lcd->scr[i] = 0;
	}
	lcd->ctr = 18;
	// Emulated physical properties (not adjustable by the calc)
	lcd->tmin = 25;
	lcd->tjit = 22;
	lcd->sinc = 0.15 * 65535;
	lcd->sdec = 0.07 * 65535;
	lcd->cwht = 0.05 * 65535;
	lcd->cblk = 1.00 * 65535;
	lcd->cmul = 0.05 * 65535;
	lcd->amul = 0.15 * 65535;
	lcd->swht = lcd->cwht + lcd->cmul * (lcd->ctr - 32) - lcd->amul * (lcd->amp + lcd->amp2);
	lcd->sblk = lcd->cblk + lcd->cmul * (lcd->ctr - 32) - lcd->amul * (lcd->amp + lcd->amp2);
}

// Calculate target shades and move towards them (in each pixel independently)
void lcd_update() {
	int i, adr;
	int tmp;
	for (i = 0; i < 120 * 64; i++) {
		adr = (i + lcd->z * 120) % (120 * 64);
		if (!lcd->dat[adr]) lcd->wcnt[adr] += emu->stop_cnt - lcd->lcnt[adr];
		tmp = (lcd->wcnt[adr] << 8) / emu->stop_period;
		tmp = (tmp * lcd->swht + (256 - tmp) * lcd->sblk) >> 8;
		if (lcd->scr[i] > tmp) {
			lcd->scr[i] -= lcd->sdec;
			if (lcd->scr[i] < tmp) lcd->scr[i] = tmp;
		} else
		if (lcd->scr[i] < tmp) {
			lcd->scr[i] += lcd->sinc;
			if (lcd->scr[i] > tmp) lcd->scr[i] = tmp;
		}
		if (lcd->scr[i] < 0) lcd->scr[i] = 0;
		if (lcd->scr[i] > 65535) lcd->scr[i] = 65535;
		lcd->wcnt[adr] = lcd->lcnt[adr] = 0;
	}
	if (lcd->next_w > 0) lcd->next_w -= emu->stop_period;
}

// Macros for identical serialisation in both directions
#define lLCDC(x) lcd->x = fgetc(fp)
#define lLCDI(x) \
	lcd->x = fgetc(fp) << 24; \
	lcd->x += fgetc(fp) << 16; \
	lcd->x += fgetc(fp) << 8; \
	lcd->x += fgetc(fp);

#define sLCDC(x) fputc(lcd->x, fp)
#define sLCDI(x) \
	fputc(lcd->x >> 24, fp); \
	fputc(lcd->x >> 16, fp); \
	fputc(lcd->x >> 8, fp); \
	fputc(lcd->x, fp)

#define SERLCD(ls) \
	ls ## LCDC(x); ls ## LCDC(y); ls ## LCDC(z); ls ## LCDC(up_dn); ls ## LCDC(cnt_sel); ls ## LCDC(on); \
	ls ## LCDC(w_len); ls ## LCDC(amp); ls ## LCDC(amp2); ls ## LCDC(test); ls ## LCDC(ctr); ls ## LCDC(dummy); \
	ls ## LCDI(next_w); ls ## LCDI(tmin); ls ## LCDI(tjit); ls ## LCDI(sinc); ls ## LCDI(sdec); \
	ls ## LCDI(cwht); ls ## LCDI(cblk); ls ## LCDI(cmul); ls ## LCDI(amul); ls ## LCDI(swht); ls ## LCDI(sblk)

// Save the current LCD state
int lcd_save(FILE *fp) {
	BYTE tb;
	int i, j;

	SERLCD(s);
	for (i = 0; i < 15 * 64; i++) {
		tb = 0;
		for (j = 0; j < 8; j++) tb |= lcd->dat[(i << 3) + j] << (7 - j);
		fputc(tb, fp);
	}
	return 0;
}

// Load the current LCD state
int lcd_load(FILE *fp) {
	BYTE tb;
	int i, j;

	memset(lcd, 0x00, sizeof(LCD));
	SERLCD(l);
	for (i = 0; i < 15 * 64; i++) {
		tb = fgetc(fp);
		for (j = 0; j < 8; j++)
			lcd->dat[(i << 3) + j] = (tb >> (7 - j)) & 1;
	}
	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../hw/hwcore.h"
#include "debug.h"
#include "dlcd.h"

#define SET_PERCENTAGE(var, minp, maxp) \
	void lcd_enter_ ## var() {\
		if (debug_read_input()) return;\
		if (dbg_dec < minp) dbg_dec = minp;\
		if (dbg_dec > maxp) dbg_dec = maxp;\
		lcd->var = dbg_dec * 655.35;\
		lcd_debug_update_shades();\
	}

char *debug_lcd_physical[] = {
	"Busy time min.", "Busy time max.", "Remains busy",
	"Momentum up", "Momentum down",
	"Contrast factor", "Op-amp factor",
	"White base level", "Black base level",
	"Current white", "Current black"
};

char *debug_lcd_software[] = {
	"Power", "Contrast", "X-Address", "Y-Address", "Z-Address", "Counter",
	"Word length", "Op-amps", "Test mode", "Dummy read"
};

DEBUG_INFO lcd_debug_physical() {
	DEBUG_INFO tmp = { 11, debug_lcd_physical, debug_values };
	sprintf(debug_values[0], "%d cc", lcd->tmin);
	sprintf(debug_values[1], "%d cc", lcd->tmin + lcd->tjit);
	sprintf(debug_values[2], "%d cc", lcd->next_w > emu->stop_cnt ? lcd->next_w - emu->stop_cnt : 0);
	sprintf(debug_values[3], "%d%%", lcd->sinc / 655);
	sprintf(debug_values[4], "%d%%", lcd->sdec / 655);
	sprintf(debug_values[5], "%d%%", lcd->cmul / 655);
	sprintf(debug_values[6], "%d%%", lcd->amul / 655);
	sprintf(debug_values[7], "%d%%", lcd->cwht / 655);
	sprintf(debug_values[8], "%d%%", lcd->cblk / 655);
	sprintf(debug_values[9], "%d%%", lcd->swht / 655);
	sprintf(debug_values[10], "%d%%", lcd->sblk / 655);
	return tmp;
}

DEBUG_INFO lcd_debug_software() {
	DEBUG_INFO tmp = { 10, debug_lcd_software, debug_values };
	sprintf(debug_values[0], lcd->on ? "on" : "off");
	sprintf(debug_values[1], "%d", lcd->ctr);
	sprintf(debug_values[2], "%d", lcd->x);
	sprintf(debug_values[3], "%d", lcd->y);
	sprintf(debug_values[4], "%d", lcd->z);
	sprintf(debug_values[5], "%c, %s", lcd->cnt_sel ? 'Y' : 'X', lcd->up_dn ? "up" : "down");
	sprintf(debug_values[6], "%c", lcd->w_len ? '8' : '6');
	sprintf(debug_values[7], "amp1=%d amp2=%d", lcd->amp, lcd->amp2);
	sprintf(debug_values[8], lcd->test ? "on" : "off");
	sprintf(debug_values[9], lcd->dummy ? "yes" : "no");
	return tmp;
}

void lcd_debug_update_shades() {
	lcd->swht = lcd->cwht + lcd->cmul * (lcd->ctr - 32) - lcd->amul * (lcd->amp + lcd->amp2);
	lcd->sblk = lcd->cblk + lcd->cmul * (lcd->ctr - 32) - lcd->amul * (lcd->amp + lcd->amp2);
}

void lcd_enter_tmin() {
	if (debug_read_input()) return;
	lcd->tjit -= dbg_dec - lcd->tmin;
	lcd->tmin = dbg_dec;
	if (lcd->tjit < 0) lcd->tjit = 0;
}
void lcd_enter_tmax() {
	if (debug_read_input()) return;
	lcd->tjit = dbg_dec - lcd->tmin;
	if (lcd->tjit < 0) {
		lcd->tmin += lcd->tjit;
		lcd->tjit = 0;
	}
}
void lcd_enter_busy() {
	if (debug_read_input()) return;
	if (dbg_dec > 0)
		lcd->next_w = emu->stop_cnt + dbg_dec;
	else
		lcd->next_w = emu->stop_cnt;
}
SET_PERCENTAGE(sinc, 0, 100)
SET_PERCENTAGE(sdec, 0, 100)
SET_PERCENTAGE(cmul, 0, 100)
SET_PERCENTAGE(amul, 0, 100)
SET_PERCENTAGE(cwht, -1000, 1000)
SET_PERCENTAGE(cblk, -1000, 1000)

void lcd_enter_power() { lcd->on ^= 1; }
void lcd_enter_test() { lcd->test ^= 1; }
void lcd_enter_dummy() { lcd->dummy ^= 1; }
void lcd_enter_word_length() { lcd->w_len ^= 1; }
void lcd_enter_counter_sel() { lcd->cnt_sel ^= 1; }
void lcd_enter_counter_dir() { lcd->up_dn ^= 1; }
void lcd_enter_amp1() { lcd->amp++; lcd->amp &= 0x03; lcd_debug_update_shades(); }
void lcd_enter_amp2() { lcd->amp2++; lcd->amp2 &= 0x03; lcd_debug_update_shades(); }

void lcd_enter_x_address() { if (debug_read_input()) return; lcd->x = dbg_dec & 0x3f; }
void lcd_enter_y_address() { if (debug_read_input()) return; lcd->y = dbg_dec & 0x1f; }
void lcd_enter_z_address() { if (debug_read_input()) return; lcd->z = dbg_dec & 0x3f; }
void lcd_enter_contrast() { if (debug_read_input()) return; lcd->ctr = dbg_dec & 0x3f; lcd_debug_update_shades(); }


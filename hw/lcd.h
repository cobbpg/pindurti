#include "emucore.h"

typedef struct {
	BYTE x; // X-address (0-63)
	BYTE y; // Y-address (0-14/19)
	BYTE z; // Z-address (0-63)
	BYTE up_dn; // Counter up/down: 0-down; 1-up
	BYTE cnt_sel; // Counter select: 0-X; 1-Y
	BYTE on; // Status: 0-off; 1-on
	BYTE w_len; // Word length: 0-6 bits; 1-8 bits
	BYTE amp; // Op-amp
	BYTE amp2; // Op-amp
	BYTE test; // Test mode: 0-normal; 1-test
	BYTE ctr; // Contrast
	BYTE dummy; // Dummy read indicator
	BYTE dat[120 * 64]; // Actual data in the LCD driver (not affected by z-address)
	int wcnt[120 * 64]; // White time counter
	int lcnt[120 * 64]; // Time of last data modification
	int next_w; // Minimum time stamp of next output to be accepted
	int tmin; // Minimum busy time in cc
	int tjit; // Busy time jitter in cc
	int sinc; // intensity increment speed
	int sdec; // intensity decrement speed
	int cwht; // white contrast
	int cblk; // black contrast
	int cmul; // contrast multiplier
	int amul; // op-amp multiplier
	int swht; // white shade
	int sblk; // black shade
	int scr[120 * 64]; // Physical shade level (affected by z-address)
} LCD;

extern LCD *lcd; // Pointer to LCD driver of running calculator

void lcd_command(BYTE cmd); // Write LCD command word
BYTE lcd_status(); // Read LCD status
void lcd_write(BYTE data); // Write LCD data
BYTE lcd_read(); // Read LCD data
void lcd_update(); // Calculate target shades and move towards them
void lcd_reset(); // Reset LCD
int lcd_save(FILE *fp); // Save the current LCD state
int lcd_load(FILE *fp); // Load the current LCD state


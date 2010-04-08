#ifndef _hwcore
#define _hwcore

#include "emucore.h"
#include "lcd.h"
#include "z80.h"
#include "flash.h"
#include "files.h"
#include "ti82.h"
#include "ti83.h"
#include "ti83p.h"
#include "../debug/debug.h"

#define MAX_CALC 20
#define LINK_HISTORY 5000

#define INIT_CONFIG 0x01

#define CALC_FLAG_PAUSE 0x0001
#define CALC_FLAG_WARP  0x0002
#define CALC_FLAG_LINK  0x0004
#define CALC_FLAG_SHOT  0x0008

typedef void (*CALCULATOR_RUN_FN)(void);
typedef void (*HANDLE_KEY_FN)(BYTE, int);
typedef int (*CALCULATOR_ON_FN)(void);
typedef int (*CALCULATOR_STATE_FN)(char*);

typedef struct {
	int flags; // Miscellaneous settings
	int rom_ver; // ROM version identifier
	char rom_path[MAX_PATH]; // Path to ROM image used
	EMU_CORE *emu; // Emulator trunk
	Z80_CPU *z80; // CPU state
	LCD *lcd; // LCD state
	void *model; // Model specific data
	CALCULATOR_RUN_FN calc_run; // Emulation of a refresh period
	CALCULATOR_RUN_FN calc_run_linked; // Emulation of a virtually linked calc
	CALCULATOR_ON_FN calc_powered; // Calculator power state
	HANDLE_KEY_FN handle_key; // Keyboard handler
	Z80_WRITE_FN z80_write; // Z80 memory write slot
	Z80_READ_FN z80_read; // Z80 memory read slot
	Z80_OUT_FN z80_out; // Z80 output slot
	Z80_IN_FN z80_in; // Z80 input slot
	Z80_READ_FN z80_acc; // Z80 memory access slot (read without side effects)
	CALCULATOR_STATE_FN calc_load; // Load calculator state
	CALCULATOR_STATE_FN calc_save; // Save calculator state
} CALCULATOR;

typedef struct {
	int time;
	BYTE hub;
	int recp;
	BYTE rec[MAX_CALC][LINK_HISTORY];
} LINKING;

extern LINKING linking;

extern CALCULATOR_RUN_FN calculator_run; // Running between two screen refreshes
extern CALCULATOR_RUN_FN calculator_run_linked; // Running while virtually linked
extern CALCULATOR_ON_FN calculator_powered; // Calculator power state
extern HANDLE_KEY_FN handle_key; // Keypress handling interface
extern CALCULATOR_STATE_FN calculator_load; // Loading calculator state from <romfile>.pti
extern CALCULATOR_STATE_FN calculator_save; // Saving calculator state to <romfile>.pti
extern CALCULATOR calc[MAX_CALC];
extern BYTE invalid_page[0x4000];
extern int running_calculator;

void hw_init(int flags); // Resetting some essentials
void hw_deinit(int flags); // Freeing memory, saving state
int create_calculator(int model, int slot);
int activate_calculator(int slot);
void free_calculator(int slot);
void calculator_run_timed(int t); // Running for a certain time
void run_all_slots(int *update_needed); // Advancing all the active slots by a frame
void calculator_toggle_flags(int slot, int flags); // Toggle mode flags and adjust additional data where needed
void render_lcd_bitmap(BYTE *scr, int win_x_size, int mulfac, int x, int y, int act);

int process_file(char *fname, int slot);
int send_byte(BYTE val);
int recv_byte();
int send_data(BYTE *dat, int length);

#endif

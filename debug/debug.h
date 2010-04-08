#ifndef _debugheader
#define _debugheader

#define DEBUG_INPUT_LEN 1000
#define DEBUG_VALUES 100

#define DEBUG_ACTIVE_CODEBP 0x00000001

typedef struct {
	int count;
	char **key;
	char **value;
} DEBUG_INFO;

typedef DEBUG_INFO (*DEBUG_INFO_FN)(void);
typedef void (*ENTER_FN)(void);

typedef struct {
	char name[100];
	DEBUG_INFO_FN query;
} DEBUG_QUERY;

extern char *debug_values[DEBUG_VALUES];
extern char debug_input[DEBUG_INPUT_LEN];
extern int dbg_dec, dbg_hex;
extern int debug_active, debug_trapped;
extern char debug_code_bp[0x10000];

void debug_init();
void debug_deinit();
DEBUG_INFO no_debug_info();
void memory_page_string(char *dest, BYTE *page, BYTE *rom, BYTE romp, BYTE *ram, BYTE ramp);
int debug_read_input();
int debug_step_instruction(int slot);
void calculator_step(int slot);
void calculator_step_over(int slot);
void debug_toggle_code_breakpoint(WORD adr);
void debug_set_code_breakpoint(WORD adr, int state);
void debug_clear_code_breakpoints();
void debug_check();

#endif

typedef struct {
	BYTE bank_a; // Current page mapped in bank A
	BYTE bank_b; // Current page mapped in bank B
	BYTE link_state; // Outgoing link state 0x00-0x03
	BYTE *partner_link; // Incoming link state 0x00-0x03
	BYTE on_key; // On key being pressed
	BYTE it_mask; // Interrupt mask: bit 0 - on key, bit 1-2 - timers, bit 3 - power, bit 4 - link
	BYTE it_active; // Active interrupts (bits as in mask)
	int it_cnt; // IT timer clock counters
	int it_next; // Time of upcoming IT timer event
	BYTE it_state; // Next IT event identifier (0: t2a, 1: t2b, 2: t1, 3: reset)
	int it_times[4]; // Times of events in a period
	BYTE key_mask; // Keyboard reading mask
	BYTE key_state[7]; // Keyboard state
	BYTE key_map[0x80]; // Scancode to keystate mapping
	BYTE rom[0x80000]; // ROM image
	BYTE ram[0x8000]; // RAM contents
	BYTE *page[4]; // Pointers to memory pages
	BYTE mut[4]; // Mutability indicator: 0 - ROM, 1 - RAM
	BYTE mmap; // Memory map mode 0x00-0x01
	BYTE flash_lock; // Flash lock state: 0 - lock, 1 - unlock
	BYTE run_lock[0x22]; // Execution protection: 0 - execution enabled, 1 - execution disabled
	BYTE exc[4]; // Executability indicator for each page copied from run_lock
	BYTE prot_cnt; // Buffer counter for port protection
	BYTE prot_buffer[8]; // Memory read buffer for port protection
	FLASH_ROM flash; // Flash ROM state
} MODEL_TI83P;

extern MODEL_TI83P *ti_83p; // Pointer to currently running TI-83+

void ti_83p_reset(); // Resetting the calc with the current ROM
void ti_83p_key(BYTE key, int state); // Setting key state
void ti_83p_run(); // Running between two screen refreshes
void ti_83p_run_linked(); // Running while connected
int ti_83p_powered(); // Returning power state
int ti_83p_send_file(char *fname, BYTE **partner, BYTE *it_active, BYTE it_mask); // Sending files through silent link
int ti_83p_send_app(char *fname); // Force loading applications
void ti_83p_load_rom(const char *fname); // Replacing the ROM and resetting
void ti_83p_load_vti_sav(const char *fname); // Loading a VTI saved state with the current ROM
void ti_83p_swap_rom_page(BYTE bank_a, BYTE bank_b); // ROM page swapping
void ti_83p_write(WORD adr, BYTE val); // TI-83+ memory write
BYTE ti_83p_read(WORD adr); // TI-83+ memory read
BYTE ti_83p_rmem(WORD adr); // TI-83+ memory read without side effects
void ti_83p_out(BYTE port, BYTE val); // TI-83+ output port
BYTE ti_83p_in(BYTE port); // TI-83+ input port
int ti_83p_protection(BYTE port); // TI-83 plus port protection
int ti_83p_load_state(char *romfile); // Loading TI-83+ state from <romfile>.pti
int ti_83p_save_state(char *romfile); // Saving TI-83+ state to <romfile>.pti


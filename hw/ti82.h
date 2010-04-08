typedef struct {
	BYTE model; // Sub-model: 0 - old style, 1 - 19.006 (83 style)
	BYTE rom_page; // Current ROM page 0x00-0x1f
	BYTE link_state; // Outgoing link state 0x00-0x03
	BYTE *partner_link; // Incoming link state 0x00-0x03
	BYTE link_mask; // Link direction mask
	BYTE on_key; // On key being pressed
	BYTE it_mask; // Interrupt mask: bit 0 - on key, bit 1 - timer, bit 2 - timer, bit 3 - power
	BYTE it_active; // Active interrupts (bits as in mask)
	int it_cnt; // IT timer clock counters
	int it_next; // Time of upcoming IT timer event
	BYTE it_state; // Next IT event identifier (0: t2a, 1: t2b, 2: t1, 3: reset)
	int it_times[4]; // Times of events in a period
	BYTE port_02; // Value output to port 2
	BYTE key_mask; // Keyboard reading mask
	BYTE key_state[7]; // Keyboard state
	BYTE key_map[0x80]; // Scancode to keystate mapping
	BYTE rom[0x20000]; // ROM image
	BYTE ram[0x8000]; // RAM contents
	BYTE *page[4]; // Pointers to memory pages
	BYTE mut[4]; // Mutability indicator: 0 - ROM, 1 - RAM
	BYTE mmap; // Memory map mode 0x00-0x01
} MODEL_TI82;

extern MODEL_TI82 *ti_82; // Pointer to currently running TI-82

void ti_82_reset(); // Resetting the calc with the current ROM
void ti_82_key(BYTE key, int state); // Setting key state
void ti_82_run(); // Running between two screen refreshes
void ti_82_run_linked(); // Running while connected
int ti_82_powered(); // Returning power state
int ti_82_send_file(char *fname, BYTE **partner, BYTE *key_state); // Sending files through silent link
void ti_82_load_rom(const char *fname); // Replacing the ROM and resetting
void ti_82_load_vti_sav(const char *fname); // Loading a VTI saved state with the current ROM
void ti_82_swap_rom_page(BYTE new_page); // ROM page swapping
void ti_82_write(WORD adr, BYTE val); // TI-82 memory write
BYTE ti_82_read(WORD adr); // TI-82 memory read
void ti_82_out(BYTE port, BYTE val); // TI-82 output port
BYTE ti_82_in(BYTE port); // TI-82 input port
int ti_82_load_state(char *romfile); // Loading TI-82 state from <romfile>.pti
int ti_82_save_state(char *romfile); // Saving TI-82 state to <romfile>.pti


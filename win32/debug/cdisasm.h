typedef struct {
	int slot;
	int items;
	int adr;
	int caret;
	int focused;
} DBG_DISASM_DATA;

int disassembly_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


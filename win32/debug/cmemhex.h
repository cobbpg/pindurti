typedef struct {
	int slot;
	int adr;
	int x, y, xs, ys;
	int focused;
} DBG_MEMHEX_DATA;

int memhex_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

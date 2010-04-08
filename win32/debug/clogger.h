typedef struct {
	int slot;
	HWND lbox;
} DBG_LOGGER_DATA;

int logger_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


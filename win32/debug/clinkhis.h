typedef struct {
	int slot;
	BITMAPINFO *bmi;
} DBG_LINK_HISTORY_DATA;

int link_history_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


typedef struct {
	int slot;
	HWND lview;
} DBG_VAT_DATA;

int vat_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


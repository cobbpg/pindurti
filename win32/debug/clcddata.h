typedef struct {
	int slot;
	BITMAPINFO *bmi;
	char *idat;
} DBG_LCD_DATA;

int lcd_data_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


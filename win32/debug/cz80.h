typedef struct {
	int slot;
	int columns;
	int editing;
} DBG_CPU_DATA;

int cpu_state_calculate_columns(int w);
int cpu_state_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


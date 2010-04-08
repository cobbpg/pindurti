typedef struct {
	int y, x, xs;
	int editable;
	ENTER_FN enter;
} PARAM_ITEM;

typedef struct {
	char name[50];
	DEBUG_INFO_FN debug;
	PARAM_ITEM pos[];
} KEYVAL_DATA;

typedef struct {
	int slot;
	int type;
	DEBUG_INFO_FN debug;
	PARAM_ITEM *pos;
	int key_width;
	ENTER_FN enter;
} DBG_KEYVAL_DATA;

int keyval_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


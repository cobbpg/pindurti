#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include "theme.h"
#include "../misc/cmd.h"
#include "../hw/hwcore.h"
#include "../misc/gif.h"
#include "debug/dwin.h"

#define SCRXSIZE 96
#define SCRYSIZE 64
#define SCRYSIZEB 80

#define MUSER_MASK 0xff00
#define MUSER_REFRESH 0xa100

#define REFRESH_LENGTH 50

//#define LOGFILE

const BYTE status_icon_pause[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const BYTE status_icon_warp[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const BYTE status_icon_link[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const BYTE status_icon_shot[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0,
	0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char window_class_name[] = "PindurWnd";
LPBITMAPINFO scr_header;
HACCEL accel_handle;
BYTE *scr;
TIMECAPS ptc;
MMRESULT lcd_timer;
FILE *logfp;
int mulfac = 2;
int update_needed = 0;
int cwdx, cwdy;
int refresh_cnt = 0;
int win_x_size = SCRXSIZE * 2, win_y_size = SCRYSIZEB * 2;
int active_slot = 0, active_calcs;
char error_msg[256];
int dump_no = 1;
int accelerate_emu = 0;
DWORD reftimes[MAX_CALC][50];
int reftidx[MAX_CALC];
int do_log = 0;
int gif_base_delay_start;

typedef HTHEME (WINAPI *tOpenThemeData)(HWND, LPCWSTR);
typedef HRESULT (WINAPI *tCloseThemeData)(HTHEME);
typedef HRESULT (WINAPI *tDrawThemeBackground)(HTHEME, HDC, int, int, const RECT*, const RECT*);
typedef HRESULT (WINAPI *tDrawThemeText)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*);
typedef HRESULT (WINAPI *tGetThemeTextExtent)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, const RECT*, RECT*);
typedef BOOL (WINAPI *tIsThemeActive)();

HMODULE uxthemelib;
tOpenThemeData fn_open_theme_data;
tCloseThemeData fn_close_theme_data;
tDrawThemeBackground fn_draw_theme_background;
tDrawThemeText fn_draw_theme_text;
tGetThemeTextExtent fn_get_theme_text_extent;
tIsThemeActive fn_is_theme_active;

void draw_sunken_box(HDC dc, RECT *r) {
	if (is_theme_active()) {
		HTHEME th = open_theme_data(NULL, L"edit");
		draw_theme_background(th, dc, EP_EDITTEXT, ETS_NORMAL, r, NULL);
		close_theme_data(th);
	} else DrawEdge(dc, r, EDGE_SUNKEN, BF_RECT);
}

HTHEME open_theme_data(HWND hwnd, LPCWSTR cs) {
	if (uxthemelib) {
		return fn_open_theme_data(hwnd, cs);
	} else return NULL;
}

HRESULT close_theme_data(HTHEME th) {
	if (uxthemelib) {
		return fn_close_theme_data(th);
	} else return S_OK;
}

HRESULT draw_theme_background(HTHEME th, HDC hdc, int pid, int sid, const RECT* r, const RECT* cr) {
	if (uxthemelib) {
		return fn_draw_theme_background(th, hdc, pid, sid, r, cr);
	} else return S_OK;
}

HRESULT draw_theme_text(HTHEME th, HDC hdc, int pid, int sid, LPCWSTR str, int len, DWORD f1, DWORD f2, const RECT* r) {
	if (uxthemelib) {
		return fn_draw_theme_text(th, hdc, pid, sid, str, len, f1, f2, r);
	} else return S_OK;
}

HRESULT get_theme_text_extent(HTHEME th, HDC hdc, int pid, int sid, LPCWSTR str, int len, DWORD f, const RECT* r, RECT* ro) {
	if (uxthemelib) {
		return fn_get_theme_text_extent(th, hdc, pid, sid, str, len, f, r, ro);
	} else return S_OK;
}

BOOL is_theme_active() {
	if (uxthemelib) {
		return fn_is_theme_active();
	} return FALSE;
}

void load_theme_procs() {
	OSVERSIONINFO os;

	uxthemelib = NULL;
	ZeroMemory(&os, sizeof(os));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os);
	if ((os.dwMajorVersion << 16) + os.dwMinorVersion >= 0x50001) {
		uxthemelib = LoadLibrary("uxtheme.dll");
		if (uxthemelib != NULL) {
			fn_open_theme_data = (void*)GetProcAddress(uxthemelib, "OpenThemeData");
			if (!fn_open_theme_data) exit(0);
			fn_close_theme_data = (void*)GetProcAddress(uxthemelib, "CloseThemeData");
			if (!fn_close_theme_data) exit(0);
			fn_draw_theme_background = (void*)GetProcAddress(uxthemelib, "DrawThemeBackground");
			if (!fn_draw_theme_background) exit(0);
			fn_draw_theme_text = (void*)GetProcAddress(uxthemelib, "DrawThemeText");
			if (!fn_draw_theme_text) exit(0);
			fn_get_theme_text_extent = (void*)GetProcAddress(uxthemelib, "GetThemeTextExtent");
			if (!fn_get_theme_text_extent) exit(0);
			fn_is_theme_active = (void*)GetProcAddress(uxthemelib, "IsThemeActive");
			if (!fn_is_theme_active) exit(0);
		}
	}
}

void CALLBACK refresh_screen(UINT tid, UINT msg, DWORD hwnd, DWORD dw1, DWORD dw2) {
	UNUSED_PARAMETER(tid);
	UNUSED_PARAMETER(msg);
	UNUSED_PARAMETER(dw1);
	UNUSED_PARAMETER(dw2);
	PostMessage((HWND)hwnd, WM_USER, MUSER_REFRESH, 0);
}

void handle_screenshot() {
	static int gif_file_no = 1;
	int i, j, s, marked;

	for (marked = 0, s = 0; s < 4; s++)
		marked += (calc[s].flags & CALC_FLAG_SHOT) != 0;
	if ((gif_write_state != GIF_IDLE) && debug_active) gif_write_state = GIF_END;
	switch (gif_write_state) {
		case GIF_IDLE:
			gif_newframe = 0;
			break;
		case GIF_START:
			sprintf(gif_file_name, "ptiani%d.gif", gif_file_no++);
			gif_xs = marked ? SCRXSIZE * marked : SCRXSIZE;
			gif_ys = SCRYSIZE;
			gif_base_delay = gif_base_delay_start;
			gif_time = 0;
			gif_newframe = 1;
			if (marked) {
				int xpos;
				for (xpos = 0, s = 0; s < 4; s++)
					if (calc[s].flags & CALC_FLAG_SHOT) {
						if (!activate_calculator(s)) {
							for (i = 0; i < SCRYSIZE; i++)
								for (j = 0; j < SCRXSIZE; j++)
									gif_frame[i * gif_xs + j + xpos] = lcd->scr[i * 120 + j] >> 13;
						} else {
							for (i = 0; i < SCRYSIZE; i++)
								for (j = 0; j < SCRXSIZE; j++)
									gif_frame[i * gif_xs + j + xpos] = 0;
						}
						xpos += SCRXSIZE;
					}
			} else if (!activate_calculator(active_slot)) {
				for (i = 0; i < SCRYSIZE; i++)
					for (j = 0; j < SCRXSIZE; j++)
						gif_frame[i * gif_xs + j] = lcd->scr[i * 120 + j] >> 13;
			}
			break;
		case GIF_FRAME:
			gif_time += 4;
			if (gif_time >= gif_base_delay) {
				gif_time -= gif_base_delay;
				gif_newframe = 1;
				if (marked) {
					int xpos;
					for (xpos = 0, s = 0; s < 4; s++)
						if (calc[s].flags & CALC_FLAG_SHOT) {
							if (!activate_calculator(s)) {
								for (i = 0; i < SCRYSIZE; i++)
									for (j = 0; j < SCRXSIZE; j++)
										gif_frame[i * gif_xs + j + xpos] = lcd->scr[i * 120 + j] >> 13;
							} else {
								for (i = 0; i < SCRYSIZE; i++)
									for (j = 0; j < SCRXSIZE; j++)
										gif_frame[i * gif_xs + j + xpos] = 0;
							}
							xpos += SCRXSIZE;
						}
				} else if (!activate_calculator(active_slot)) {
					for (i = 0; i < SCRYSIZE; i++)
						for (j = 0; j < SCRXSIZE; j++)
							gif_frame[i * gif_xs + j] = lcd->scr[i * 120 + j] >> 13;
				}
			}
			break;
		case GIF_END:
			gif_newframe = 1;
			break;
	}
	if (gif_newframe) {
		gif_newframe = 0;
		gif_writer();
	}
}

void draw_status_icon(const BYTE *dat, int x, int y, int inv) {
	int i, j, k, c;
	BYTE *dst = scr + y * win_x_size * mulfac + x;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			c = (*(dat++) ^ inv) ? 255 : 128;
			for (k = 0; k < mulfac; k++) *(dst++) = c;
		}
		dst += win_x_size * mulfac - 16 * mulfac;
		for (j = 1; j < mulfac; j++) {
			memcpy(dst, dst - win_x_size * mulfac, 16 * mulfac);
			dst += win_x_size * mulfac;
		}
	}
}

void refresh_lcd_display(int slot) {
	if (!activate_calculator(slot)) {
		active_calcs++;
		SYSTEMTIME st;
		FILETIME ft;
		GetSystemTime(&st);
		if (SystemTimeToFileTime(&st, &ft)) {
			reftimes[slot][reftidx[slot]++] = ft.dwLowDateTime;
			reftidx[slot] %= REFRESH_LENGTH;
		}
		int x = (slot & 1) * SCRXSIZE * mulfac;
		int y = ((slot >> 1) * SCRYSIZEB + SCRYSIZEB - SCRYSIZE) * mulfac;
		render_lcd_bitmap(scr, win_x_size * mulfac, mulfac, x, y, slot == active_slot);
		y -= mulfac * (SCRYSIZEB - SCRYSIZE);
		draw_status_icon(status_icon_pause, x, y, (calc[slot].flags & CALC_FLAG_PAUSE) != 0);
		draw_status_icon(status_icon_warp, x + 16 * mulfac, y, (calc[slot].flags & CALC_FLAG_WARP) != 0);
		draw_status_icon(status_icon_link, x + 32 * mulfac, y, (calc[slot].flags & CALC_FLAG_LINK) != 0);
		draw_status_icon(status_icon_shot, x + 48 * mulfac, y, (calc[slot].flags & CALC_FLAG_SHOT) != 0);
	}
}

void refresh_hardware(HWND hwnd) {
	int s;
	if (debug_shown == 3) return;
	if (debug_shown == 1) {
		PostMessage(debug_window, WM_USER, DEBUG_REFRESH, 0);
		debug_shown = 3;
		return;
	}
	debug_shown = 0;
	active_calcs = 0;
	run_all_slots(&update_needed);
	if (debug_trapped) {
		debug_shown = 1;
		debug_trapped = 0;
		show_debug_window(debug_shown);
		return;
	}
	for (s = 0; s < 4; s++)
		if (((update_needed & 1) || (calc[s].flags & CALC_FLAG_WARP)) && !activate_calculator(s))
			refresh_lcd_display(s);
	update_needed = accelerate_emu ? 2 : 0;
	handle_screenshot();
	InvalidateRgn(hwnd, NULL, FALSE);
	if (debug_shown) PostMessage(debug_window, WM_USER, DEBUG_REFRESH, 0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CREATE: {
			int i, c;
			RECT wr, cr;
			ACCEL acc[] = {
				{FVIRTKEY, VK_F7, DEBUG_CMD_STEP},
				{FVIRTKEY, VK_F8, DEBUG_CMD_STEP_OVER},
				{FVIRTKEY, VK_F12, DEBUG_CMD_LAYOUT_EDITOR},
				{FVIRTKEY | FCONTROL, VK_ADD, DEBUG_CMD_FONT_BIG},
				{FVIRTKEY | FCONTROL, VK_SUBTRACT, DEBUG_CMD_FONT_SMALL}
			};
#ifdef LOGFILE
			logfp = fopen("logfile", "w");
			fprintf(logfp, "--start--\n");
#endif
			accel_handle = CreateAcceleratorTable(acc, 5);
			scr = malloc(win_x_size * win_y_size * mulfac * mulfac);
			memset(scr, 0x00, win_x_size * win_y_size * mulfac * mulfac);
			memset(reftimes, 0x00, sizeof(reftimes));
			memset(reftidx, 0x00, sizeof(reftidx));
			scr_header = malloc(sizeof(BITMAPINFOHEADER) + 1024);
			scr_header->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			scr_header->bmiHeader.biWidth = win_x_size * mulfac;
			scr_header->bmiHeader.biHeight = -win_y_size * mulfac;
			scr_header->bmiHeader.biPlanes = 1;
			scr_header->bmiHeader.biBitCount = 8;
			scr_header->bmiHeader.biCompression = BI_RGB;
			scr_header->bmiHeader.biSizeImage = 0;
			scr_header->bmiHeader.biXPelsPerMeter = 0;
			scr_header->bmiHeader.biYPelsPerMeter = 0;
			scr_header->bmiHeader.biClrUsed = 256;
			scr_header->bmiHeader.biClrImportant = 256;
			for(i = 0; i < 256; i++) {
				c = ((i & 0x7f) ^ 0x7f) << 1;
				scr_header->bmiColors[i].rgbRed = (i >= 128) ? c : c * 0.75;
				scr_header->bmiColors[i].rgbGreen = c;
				scr_header->bmiColors[i].rgbBlue = c;
			}
			GetWindowRect(hwnd, &wr);
			GetClientRect(hwnd, &cr);
			cwdx = wr.right - wr.left - cr.right;
			cwdy = wr.bottom - wr.top - cr.bottom;
			DragAcceptFiles(hwnd, TRUE);
			debug_init();
			hw_init(INIT_CONFIG);
		}
		break;
		case WM_DESTROY:
			if (gif_write_state != GIF_IDLE) {
				gif_write_state = GIF_END;
				gif_writer();
			}
			hw_deinit(INIT_CONFIG);
			debug_deinit();
			free(scr);
			free(scr_header);
			DestroyAcceleratorTable(accel_handle);
			timeKillEvent(lcd_timer);
			timeEndPeriod(ptc.wPeriodMin);
#ifdef LOGFILE
			fprintf(logfp, "--quit--\n");
			fclose(logfp);
#endif
			PostQuitMessage(0);
		break;
		case WM_RBUTTONDOWN:
			active_slot =
				GET_X_LPARAM(lParam) / SCRXSIZE / mulfac +
				(GET_Y_LPARAM(lParam) / SCRYSIZEB / mulfac) * 2;
		break;
		case WM_LBUTTONDOWN: {
			int ns =
				GET_X_LPARAM(lParam) / SCRXSIZE / mulfac +
				(GET_Y_LPARAM(lParam) / SCRYSIZEB / mulfac) * 2;
			if ((GET_Y_LPARAM(lParam) / mulfac) % SCRYSIZEB < SCRYSIZEB - SCRYSIZE) {
				switch (((GET_X_LPARAM(lParam) / mulfac) % SCRXSIZE) / 16) {
					case 0: {
						calculator_toggle_flags(ns, CALC_FLAG_PAUSE);
						int i;
						for (accelerate_emu = 0, i = 0; i < MAX_CALC; i++)
							accelerate_emu |=
								(calc[i].flags & CALC_FLAG_WARP) &&
								!(calc[i].flags & CALC_FLAG_PAUSE);
						}
						break;
					case 1: {
						calculator_toggle_flags(ns, CALC_FLAG_WARP);
						int i;
						for (accelerate_emu = 0, i = 0; i < MAX_CALC; i++)
							accelerate_emu |=
								(calc[i].flags & CALC_FLAG_WARP) &&
								!(calc[i].flags & CALC_FLAG_PAUSE);
						}
						break;
					case 2:
						calculator_toggle_flags(ns, CALC_FLAG_LINK);
						break;
					case 3:
						if (gif_write_state == GIF_IDLE)
							calculator_toggle_flags(ns, CALC_FLAG_SHOT);
						break;
				}
			} else {
				active_slot = ns;
				if (!activate_calculator(active_slot)) handle_key(0x80, 1);
			}
		}
		break;
		case WM_LBUTTONUP:
			if (!((GET_Y_LPARAM(lParam) / mulfac) % SCRYSIZEB < SCRYSIZEB - SCRYSIZE)) {
				active_slot =
					GET_X_LPARAM(lParam) / SCRXSIZE / mulfac +
					(GET_Y_LPARAM(lParam) / SCRYSIZEB / mulfac) * 2;
				if (!activate_calculator(active_slot)) handle_key(0x80, 0);
			}
		break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			// If TAB is pressed, control moving screenshot
			if ((wParam == VK_TAB) && !debug_active && !activate_calculator(active_slot)) {
				gif_base_delay_start = 11;
				gif_write_state++;
			}
			// Backspace: smooth screenshot (bigger file!)
			if ((wParam == VK_BACK) && !debug_active && !activate_calculator(active_slot)) {
				gif_base_delay_start = 4;
				gif_write_state++;
			}
			// F12: establish/break virtual link
//			if (wParam == VK_F12) {
//				calc[active_slot].flags ^= CALC_FLAG_LINK;
////				calculator_toggle_flags(active_slot, CALC_FLAG_LINK);
//				int i;
//				for (virtual_link = 0, i = 0; i < MAX_CALC; i++)
//					virtual_link |= calc[active_slot].flags & CALC_FLAG_LINK;
//				if (active_calcs == 2) {
//					virtual_link ^= 1;
//					if (virtual_link) MessageBox(NULL, "Virtual link established.", "Info", MB_OK);
//					else MessageBox(NULL, "Virtual link disconnected.", "Info", MB_OK);
//				}
//			}
			// F11: enable/disable warp mode
			if (wParam == VK_F11) {
				calculator_toggle_flags(active_slot, CALC_FLAG_WARP);
				int i;
				for (accelerate_emu = 0, i = 0; i < MAX_CALC; i++)
					accelerate_emu |=
						(calc[i].flags & CALC_FLAG_WARP) &&
						!(calc[i].flags & CALC_FLAG_PAUSE);
			}
			// F10: enable/disable debug window
			if (wParam == VK_F10) {
				debug_shown ^= 1;
				show_debug_window(debug_shown);
			}
			// F9: reset calc
			if (wParam == VK_F9) {
				if (!activate_calculator(active_slot)) {
					switch (calc[active_slot].rom_ver & FILE_PROT_MASK) {
						case FILE_PROT_82: ti_82_reset(); break;
						case FILE_PROT_83: ti_83_reset(); break;
						case FILE_PROT_83P: ti_83p_reset(); break;
					}
				}
			}
			// F8: log stuff
			if (wParam == VK_F8) do_log ^= 1;
		case WM_SYSKEYUP:
		case WM_KEYUP: {
			if (!activate_calculator(active_slot))
				handle_key((lParam >> 16) & 0x7f, ((lParam >> 16) & KF_UP) == 0);
//			fprintf(logfp, "key %c %d\n", (lParam & KF_UP) ? 'u' : 'd', (BYTE)(lParam >> 16));
		}
		break;
		case WM_USER: {
			switch (wParam & MUSER_MASK) {
				case MUSER_REFRESH: {
					refresh_cnt++;
					update_needed |= 1;
					if (refresh_cnt >= 5) {
						int spd;
						if (reftimes[active_slot][reftidx[active_slot]]) {
							int dif =
								(reftimes[active_slot][(reftidx[active_slot] + REFRESH_LENGTH - 1) % REFRESH_LENGTH] -
								reftimes[active_slot][reftidx[active_slot]]) / 10;
							spd = (4000000 * REFRESH_LENGTH - 2000000) / dif;
						} else {
							spd = 0;
						}
						char new_title[50];
						if (active_calcs > 0) {
							if (gif_write_state != GIF_IDLE)
								sprintf(new_title, "%d bytes - %s", gif_file_size, gif_file_name);
							else {
								char mstr[5];
								switch (calc[active_slot].rom_ver & FILE_MODEL_MASK) {
									case FILE_MODEL_82:
									case FILE_MODEL_82b: sprintf(mstr, "82"); break;
									case FILE_MODEL_83: sprintf(mstr, "83"); break;
									case FILE_MODEL_83P: sprintf(mstr, "83+"); break;
									default: mstr[0] = 0;
								}
								sprintf(new_title, "PindurTI%s - %d%%", mstr, spd);
							}
						} else sprintf(new_title, "PindurTI");
						SetWindowText(hwnd, new_title);
						refresh_cnt = 0;
					}
				}
				break;
			}
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_SIZING: {
			RECT *ns = (RECT*)lParam;
			switch (wParam) {
				case WMSZ_BOTTOM: case WMSZ_BOTTOMRIGHT:
					ns->right = ns->left + (ns->bottom - ns->top) / win_y_size * win_x_size + cwdx;
					ns->bottom = ns->top + (ns->bottom - ns->top) / win_y_size * win_y_size + cwdy;
				break;
				case WMSZ_RIGHT:
					ns->bottom = ns->top + (ns->right - ns->left) / win_x_size * win_y_size + cwdy;
					ns->right = ns->left + (ns->right - ns->left) / win_x_size * win_x_size + cwdx;
				break;
				case WMSZ_LEFT: case WMSZ_BOTTOMLEFT:
					ns->bottom = ns->top + (ns->right - ns->left) / win_x_size * win_y_size + cwdy;
					ns->left = ns->right - (ns->right - ns->left) / win_x_size * win_x_size - cwdx;
				break;
				case WMSZ_TOP: case WMSZ_TOPRIGHT:
					ns->right = ns->left + (ns->bottom - ns->top) / win_y_size * win_x_size + cwdx;
					ns->top = ns->bottom - (ns->bottom - ns->top) / win_y_size * win_y_size - cwdy;
				break;
				case WMSZ_TOPLEFT:
					ns->top = ns->bottom - (ns->right - ns->left) / win_x_size * win_y_size - cwdy;
					ns->left = ns->right - (ns->right - ns->left) / win_x_size * win_x_size - cwdx;
				break;
			}
			if (mulfac != (ns->bottom - ns->top - cwdy) / win_y_size)
				InvalidateRgn(hwnd, NULL, FALSE);
			mulfac = (ns->bottom - ns->top - cwdy) / win_y_size;
		}
		return TRUE;
		case WM_SIZE:
			free(scr);
			scr = malloc(win_x_size * win_y_size * mulfac * mulfac);
			memset(scr, 0x00, win_x_size * win_y_size * mulfac * mulfac);
			scr_header->bmiHeader.biWidth = win_x_size * mulfac;
			scr_header->bmiHeader.biHeight = -win_y_size * mulfac;
		break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			SetDIBitsToDevice(hdc, 0, 0, win_x_size * mulfac, win_y_size * mulfac, 0, 0,
				0, win_y_size * mulfac, scr, scr_header, DIB_RGB_COLORS);
			EndPaint(hwnd, &ps);
		}
		break;
		case WM_COPYDATA: {
			BYTE *dat = (BYTE*)((PCOPYDATASTRUCT)lParam)->lpData;
			if (dat[dat[1] + 2]) {
				MessageBox(NULL, "Erroneous message received.", "Error!", MB_ICONEXCLAMATION | MB_OK);
				return 1;
			}
			else if (process_file(dat + 2, (dat[0] == 0xff) ? active_slot : dat[0])) {
				MessageBox(NULL, error_msg, "Error!", MB_ICONEXCLAMATION | MB_OK);
				return 1;
			}
			else if ((detect_file_type(dat + 2) & FILE_TYPE_MASK) == FILE_TYPE_ROM)
				PostMessage(debug_window, WM_USER, DEBUG_RESLOT, 0);
		}
		break;
		case WM_DROPFILES: {
			HANDLE hDrop = (HANDLE)wParam;
			POINT mPos;
			RECT wr;
			GetWindowRect(hwnd, &wr);
			GetCursorPos(&mPos);
			int new_slot =
				(mPos.x - wr.left - cwdx) / SCRXSIZE / mulfac +
				((mPos.y - wr.top - cwdy) / SCRYSIZEB / mulfac) * 2;
			if ((new_slot >= 0) && (new_slot < 16)) active_slot = new_slot;
			int files = DragQueryFile(hDrop, -1, NULL, 0);
			int i;
			char name[256];
			for (i = 0; i < files; i++) {
				DragQueryFile(hDrop, i, name, 255);
				if (process_file(name, active_slot)) {
					MessageBox(NULL, error_msg, "Error!", MB_ICONEXCLAMATION | MB_OK);
				}
				else if ((detect_file_type(name) & FILE_TYPE_MASK) == FILE_TYPE_ROM)
					PostMessage(debug_window, WM_USER, DEBUG_RESLOT, 0);
			}
			DragFinish(hDrop);
		}
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int pindurti_window() {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;
	RECT wrect;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = window_class_name;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window registration failed.", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	wrect.left = 0; wrect.right = win_x_size * mulfac;
	wrect.top = 0; wrect.bottom = win_y_size * mulfac;
	if (!AdjustWindowRect(&wrect, WS_OVERLAPPEDWINDOW, FALSE)) {
		wrect.right = win_x_size * mulfac;
		wrect.bottom = win_y_size * mulfac;
	} else {
		wrect.right -= wrect.left;
		wrect.bottom -= wrect.top;
	}

	hwnd = CreateWindowEx(0, window_class_name, "PindurTI", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, wrect.right, wrect.bottom, NULL, NULL, GetModuleHandle(NULL), NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Window creation failed.", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	if (timeGetDevCaps(&ptc, sizeof(TIMECAPS)) != TIMERR_NOERROR) {
		MessageBox(NULL, "Multimedia timer resolution could not be obtained.", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	if (timeBeginPeriod(ptc.wPeriodMin) != TIMERR_NOERROR) {
		MessageBox(NULL, "Multimedia timer resolution could not be set.", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	lcd_timer = timeSetEvent(40, 10, refresh_screen, (DWORD)hwnd, TIME_PERIODIC);

	if (lcd_timer == 0) {
		MessageBox(NULL, "Timer creation failed.", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	load_theme_procs();
	InitCommonControls();

	register_debug_window_classes();
	init_debug_window();

	ShowWindow(hwnd, TRUE);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)	{
		if (!TranslateAccelerator(debug_window, accel_handle, &Msg)) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		if (update_needed && !GetQueueStatus(QS_ALLINPUT)) refresh_hardware(hwnd);
	}

	return Msg.wParam;
}

int pindurti_piped(int flags) {
	char line[0x10000];
	int i;
	FILE *fp = NULL;

	if (flags & 1) fp = fopen("pti.log", "w");
	debug_init();
	hw_init(0);

	while (!feof(stdin)) {
		fgets(line, 0x10000, stdin);
		for (i = 0; i < 0x10000; i++)
			if (line[i] < ' ') {
				line[i] = 0;
				break;
			}
		if (flags & 1) {
			int nf = !strncmp(line, "draw-", 5);
			nf |= !strncmp(line, "run ", 4);
			if (flags & 2 || !nf) {
				fprintf(fp, "%s\n", line);
				fflush(fp);
			}
		}
		run_command(line);
/*		if ((flags & 1) && !strncmp(line, "key-", 4)) {
			for (i = 0; i < 7; i++)
				fprintf(fp, "%02x\n", ti_83p->key_state[i]);
		}*/
		fflush(stdout);
	}

	hw_deinit(0);
	debug_deinit();
	if (flags & 1) fclose(fp);

	return 0;
}

int main(int argc, char* argv[]) {
	int i;
	int f_piped = 0, f_logged = 0, f_vlogged = 0;

	// Processing command line
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
				case 'p': f_piped = 1; break;
				case 'l': f_logged = 1; break;
				case 'v': f_vlogged = 1; break;
				case '-':
					f_piped = !strcmp(argv[i] + 2, "piped");
					f_logged = !strcmp(argv[i] + 2, "logged");
					f_vlogged = !strcmp(argv[i] + 2, "verbose-log");
				break;
			}
	}

	// Working in a non-interactive pipe
	if (f_piped) return pindurti_piped(f_logged + f_vlogged * 2);

	// Starting a GUI session
	return pindurti_window();
}

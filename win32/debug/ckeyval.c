#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include "dwin.h"

int keyval_id(PARAM_ITEM *pos, int x, int y) {
	int i;

	for (i = 0; pos[i].y != -1; i++)
		if ((pos[i].y == y) && (pos[i].x <= x) && (pos[i].x + pos[i].xs > x)) return i;

	return -1;
}

void keyval_set_width(DBG_KEYVAL_DATA *dat, HWND hwnd) {
	if (activate_calculator(dat->slot)) return;
	int i;
	RECT tr;
	DEBUG_INFO di = dat->debug();
	HDC dc = GetDC(hwnd);
	HFONT fnt = SelectObject(dc, font.sans);
	dat->key_width = 0;
	for (i = 0; i < di.count; i++) {
		char tmp[200];
		int tmpl = sprintf(tmp, "%s:", di.key[i]);
		tr.left = 14;
		tr.top = 20;
		tr.right = cdat->r.xs - 14;
		tr.bottom = cdat->r.ys - 14;
		DrawText(dc, tmp, tmpl, &tr, DT_CALCRECT);
		if (tr.right - tr.left > dat->key_width)
			dat->key_width = tr.right - tr.left;
	}
	SelectObject(dc, fnt);
	ReleaseDC(hwnd, dc);
	dat->key_width += 3;
}

void keyval_paint(DBG_KEYVAL_DATA *dat, HDC dc) {
	if (activate_calculator(dat->slot)) return;
	int i;
	RECT tr;
	char tk[10000], tv[10000], *tkp, *tvp;
	DEBUG_INFO di = dat->debug();
	HFONT fnt = SelectObject(dc, font.sans);
	SetBkMode(dc, TRANSPARENT);
	tkp = tk;
	tvp = tv;
	for (i = 0; (i < di.count) && (44 + i * 13 < cdat->r.ys); i++) {
		tkp += sprintf(tkp, "%s:\n", di.key[i]);
		tvp += sprintf(tvp, "%s\n", di.value[i]);
	}
	tr.left = 14;
	tr.top = 20;
	tr.right = (cdat->r.xs - 28 > dat->key_width) ? dat->key_width + 14 : cdat->r.xs - 14;
	tr.bottom = 20 + i * 13;
	if (tr.right >= dat->key_width + 14) {
		DrawText(dc, tk, tkp - tk, &tr, DT_NOCLIP);
		tr.left = dat->key_width + 14;
		tr.right = cdat->r.xs - 14;
		DrawText(dc, tv, tvp - tv, &tr, DT_WORD_ELLIPSIS);
	} else DrawText(dc, tk, tkp - tk, &tr, DT_WORD_ELLIPSIS);
	SelectObject(dc, fnt);
}

void keyval_stop_editing(DBG_KEYVAL_DATA *dat) {
	if (dat->enter) {
		SendMessage(edit_box, WM_GETTEXT, DEBUG_INPUT_LEN, (LPARAM)debug_input);
		dat->enter();
		dat->enter = NULL;
		SendMessage(debug_window, WM_USER, DEBUG_REFRESH, 0);
	}
	ShowWindow(edit_box, FALSE);
}

int keyval_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	DBG_KEYVAL_DATA *dat = cdat->state;

	switch (msg) {
		case WM_CREATE: {
			dat->slot = -1;
			dat->type = -1;
			dat->key_width = 0;
			dat->pos = NULL;
			dat->enter = NULL;
		}
		break;
		case WM_MOUSEMOVE: {
			int x = LOWORD(lparam), y = HIWORD(lparam);
			if ((x >= 14) && (x < cdat->r.xs - 14) && (y >= 20) && (y < cdat->r.ys - 14)) {
				if (keyval_id(dat->pos, x - dat->key_width - 14, (y - 20) / 13) != -1)
					SetCursor(cur_mod);
				else
					SetCursor(cur_arr);
			}
		}
		break;
		case WM_LBUTTONDOWN: {
			if (activate_calculator(dat->slot) || !dat->key_width) break;
			int x = LOWORD(lparam) - dat->key_width - 14, y = (HIWORD(lparam) - 20) / 13;
			int par = keyval_id(dat->pos, x, y);
			if (par != -1) {
				if (dat->pos[par].editable) {
					DEBUG_INFO di = dat->debug();
					char *ci;
					for (ci = di.value[y]; *ci; ci++)
						if ((*ci == ' ') || (*ci == '%')) {
							*ci = 0;
							break;
						}
					SetWindowPos(edit_box, HWND_TOP,
						cdat->r.x + dat->pos[par].x + dat->key_width + 14, cdat->r.y + y * 13 + 20,
						dat->pos[par].xs, 13, 0);
					SendMessage(edit_box, WM_SETTEXT, 0, (LPARAM)di.value[y]);
					SendMessage(edit_box, EM_SETLIMITTEXT, DEBUG_INPUT_LEN, 0);
					ShowWindow(edit_box, TRUE);
					SetFocus(edit_box);
					dat->enter = dat->pos[par].enter;
				} else {
					if (dat->pos[par].enter) {
						dat->pos[par].enter();
						SendMessage(debug_window, WM_USER, DEBUG_REFRESH, 0);
					}
					dat->enter = NULL;
				}
			} else {
				keyval_stop_editing(dat);
			}
			update_component();
		}
		break;
		case WM_PRINT:
			keyval_paint(dat, (HDC)wparam);
		break;
		case WM_USER:
			switch (wparam) {
				case DEBUG_SLOT: {
					dat->slot = LOWORD(lparam);
					if (HIWORD(lparam) > 0) dat->type = HIWORD(lparam) - 1;
					KEYVAL_DATA* kvd = keyval_data(dat->type, calc[dat->slot].rom_ver);
					dat->debug = kvd->debug;
					dat->pos = kvd->pos;
					strcpy(cdat->name, kvd->name);
					keyval_set_width(dat, hwnd);
				}
				break;
				case DEBUG_RESLOT: {
					KEYVAL_DATA* kvd = keyval_data(dat->type, calc[dat->slot].rom_ver);
					dat->debug = kvd->debug;
					dat->pos = kvd->pos;
					strcpy(cdat->name, kvd->name);
					keyval_set_width(dat, hwnd);
				}
				break;
				case DEBUG_EDITED:
					keyval_stop_editing(dat);
				break;
				case DEBUG_NOEDIT:
				case DEBUG_REFRESH:
				case DEBUG_STARTDRAG:
					dat->enter = NULL;
					keyval_stop_editing(dat);
				break;
			}
		break;
	}
	return 0;
}

#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0400

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include <wingdi.h>
#include <uxtheme.h>
#include <tmschema.h>
#include "dwin.h"
#include "../../debug/dtrap.h"
/*
void logger_append_content(DBG_LOGGER_DATA *dat, const char* s) {
	if (s == NULL) return;
	int len = SendMessage(dat->lbox, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(dat->lbox, EM_SETSEL, len, len);
	SendMessage(dat->lbox, EM_REPLACESEL, FALSE, (LPARAM)s);
}
*/
int logger_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	DBG_LOGGER_DATA *dat = cdat->state;

	switch (msg) {
	case WM_CREATE:
		dat->slot = -1;
		dat->lbox = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL |
			ES_MULTILINE | ES_NOHIDESEL | ES_READONLY | ES_AUTOVSCROLL,
			0, 0, 0, 0, hwnd, (HMENU)0, NULL, NULL);
		SendMessage(dat->lbox, WM_SETFONT, (WPARAM)font.mono, 0);
		break;
	case WM_DESTROY:
		DestroyWindow(dat->lbox);
		break;
	case WM_PRINT:
		break;
	case WM_SIZE:
		SetWindowPos(dat->lbox, HWND_TOP, 14, 20, cdat->r.xs - 28, cdat->r.ys - 34, 0);
		break;
	case WM_USER:
		switch (wparam) {
		case DEBUG_REFRESH: {
			int i, s;
			char *tmp, *nt;
			for (s = 0, i = 0; i < DEBUG_LOG_LIST_LENGTH; i++) s += debug_log.llen[i];
			tmp = malloc(s + 1);
			for (nt = tmp, i = debug_log.next; i < DEBUG_LOG_LIST_LENGTH; i++) {
				memcpy(nt, debug_log.list[i], debug_log.llen[i]);
				nt += debug_log.llen[i];
			}
			for (i = 0; i < debug_log.next; i++) {
				memcpy(nt, debug_log.list[i], debug_log.llen[i]);
				nt += debug_log.llen[i];
			}
			nt[0] = 0;
			SetWindowRedraw(dat->lbox, FALSE);
			SendMessage(dat->lbox, WM_SETTEXT, 0, (LPARAM)tmp);
			Edit_Scroll(dat->lbox, 10000, 0);
			SetWindowRedraw(dat->lbox, TRUE);
			free(tmp);
//			InvalidateRect(dat->lbox, NULL, FALSE);
//			UpdateWindow(dat->lbox);
			break;
		}
		}
		break;
	}
	return 0;
}

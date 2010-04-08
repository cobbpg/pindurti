#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include <wingdi.h>
#include "dwin.h"
#include "../theme.h"

int state_sizes[] = {
	sizeof(DBG_KEYVAL_DATA),
	sizeof(DBG_DISASM_DATA),
	sizeof(DBG_CPU_DATA),
	sizeof(DBG_LCD_DATA),
	sizeof(DBG_MEMHEX_DATA),
	sizeof(DBG_VAT_DATA),
	sizeof(DBG_LINK_HISTORY_DATA),
	sizeof(DBG_LOGGER_DATA)
};

void component_box_paint(HDC dc) {
	RECT box, tbox, cbox;
	HFONT fnt = SelectObject(dc, font.sans);
	SetBkMode(dc, TRANSPARENT);
	tbox.left = 12;
	tbox.top = 0;
	tbox.right = 40;
	tbox.bottom = 13;
	if (is_theme_active()) {
		box.left = dt_root->r.x - cdat->r.x;
		box.top = dt_root->r.y - cdat->r.y;
		box.right = box.left + dt_root->r.xs;
		box.bottom = box.top + dt_root->r.ys;
		HTHEME th = open_theme_data(NULL, L"tab");
		draw_theme_background(th, dc, TABP_BODY, 0, &box, NULL);
		close_theme_data(th);
		th = open_theme_data(NULL, L"button");
		int widelen = MultiByteToWideChar(CP_ACP, 0, cdat->name, strlen(cdat->name) + 1, NULL, 0);
		WCHAR *wname = malloc(sizeof(WCHAR[widelen + 1]));
		MultiByteToWideChar(CP_ACP, 0, cdat->name, strlen(cdat->name) + 1, wname, widelen);
		get_theme_text_extent(th, dc, BP_GROUPBOX, GBS_NORMAL, wname, -1, DT_CALCRECT, NULL, &tbox);
		OffsetRect(&tbox, 12, 0);
		if (tbox.right > cdat->r.xs - 12) tbox.right = cdat->r.xs - 12;
		draw_theme_text(th, dc, BP_GROUPBOX, GBS_NORMAL, wname, -1, DT_END_ELLIPSIS, 0, &tbox);
		free(wname);
		box.left = 5;
		box.top = 5;
		box.right = cdat->r.xs - 4;
		box.bottom = cdat->r.ys - 4;
		cbox.left = 0;
		cbox.top = 0;
		cbox.right = tbox.left;
		cbox.bottom = cdat->r.ys;
		draw_theme_background(th, dc, BP_GROUPBOX, GBS_NORMAL, &box, &cbox);
		cbox.left = tbox.right;
		cbox.right = cdat->r.xs;
		draw_theme_background(th, dc, BP_GROUPBOX, GBS_NORMAL, &box, &cbox);
		cbox.left = 0;
		cbox.top = cdat->r.ys - 12;
		draw_theme_background(th, dc, BP_GROUPBOX, GBS_NORMAL, &box, &cbox);
		close_theme_data(th);
	} else {
		box.left = 0;
		box.top = 0;
		box.right = cdat->r.xs;
		box.bottom = cdat->r.ys;
		FillRect(dc, &box, GetSysColorBrush(COLOR_BTNFACE));
		box.left = 6;
		box.top = 6;
		box.right = cdat->r.xs - 6;
		box.bottom = cdat->r.ys - 6;
		DrawEdge(dc, &box, EDGE_ETCHED, BF_BOTTOMLEFT | BF_RIGHT);
		cbox.left = cdat->r.x;
		cbox.top = cdat->r.y;
		cbox.right = cbox.left + cdat->r.xs;
		cbox.bottom = cbox.top + cdat->r.ys;
		DrawText(dc, cdat->name, strlen(cdat->name), &tbox, DT_CALCRECT);
		if (tbox.right > cdat->r.xs - 12) tbox.right = cdat->r.xs - 12;
		DrawText(dc, cdat->name, strlen(cdat->name), &tbox, DT_END_ELLIPSIS);
		box.left = 7;
		box.right = 11;
		DrawEdge(dc, &box, EDGE_ETCHED, BF_TOP);
		box.left = tbox.right + 1;
		box.right = cdat->r.xs - 6;
		DrawEdge(dc, &box, EDGE_ETCHED, BF_TOP);
	}
	SelectObject(dc, fnt);
}

LRESULT CALLBACK component_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	int dwp = 0;
	void *ctmp = cdat = get_window_data(hwnd);

	switch (msg) {
	case WM_NCCREATE: {
		cdat = ((CREATESTRUCT*)lparam)->lpCreateParams;
		cdat->state = malloc(state_sizes[cdat->type]);
		cdat->menu = CreatePopupMenu();
		AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_STEP, "Step\tF7");
		AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_STEP_OVER, "Step over\tF8");
		AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_LAYOUT_EDITOR, "Toggle layout editor\tF12");
		//AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_FONT_BIG, "Bigger font\tCtrl++");
		//AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_FONT_SMALL, "Smaller font\tCtrl+-");
		set_window_data(hwnd, cdat);
		return TRUE;
	}
	case WM_NCDESTROY:
		free(cdat->state);
		DestroyMenu(cdat->menu);
		break;
	case WM_CONTEXTMENU:
		TrackPopupMenu(cdat->menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam),
			0, hwnd, NULL);
		return 0;
	case WM_RBUTTONUP:
		dwp = 1;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		SendMessage(debug_window, msg, wparam, MAKELPARAM(LOWORD(lparam) + cdat->r.x, HIWORD(lparam) + cdat->r.y));
		break;
	case WM_COMMAND: {
		int id = LOWORD(wparam);
		if (HIWORD(wparam) == 0)
			switch (id) {
			case DEBUG_CMD_STEP:
			case DEBUG_CMD_STEP_OVER:
			case DEBUG_CMD_LAYOUT_EDITOR:
			case DEBUG_CMD_FONT_BIG:
			case DEBUG_CMD_FONT_SMALL:
				SendMessage(debug_window, WM_COMMAND, MAKEWPARAM(id, 1), 0);
				return 0;
			}
		break;
	}
	case WM_PAINT: {
		HDC dc = GetDC(hwnd);
		HDC cdc = CreateCompatibleDC(dc);
		HBITMAP bm = CreateCompatibleBitmap(dc, cdat->r.xs, cdat->r.ys);
		HBITMAP obm = SelectObject(cdc, bm);
		component_box_paint(cdc);
		cdat->win_proc(hwnd, WM_PRINT, (WPARAM)cdc, PRF_CLIENT | PRF_NONCLIENT);
		BitBlt(dc, 0, 0, cdat->r.xs, cdat->r.ys, cdc, 0, 0, SRCCOPY);
		SelectObject(cdc, obm);
		DeleteObject(bm);
		DeleteDC(cdc);
		ReleaseDC(hwnd, dc);
		ValidateRgn(hwnd, NULL);
		return 0;
	}
	case WM_PRINT:
		component_box_paint((HDC)wparam);
		cdat->win_proc(hwnd, WM_PRINT, wparam, lparam);
		return 0;
	default: dwp = 1;
	}

	cdat = ctmp;
	cdat->win_proc(hwnd, msg, wparam, lparam);
	return dwp ? DefWindowProc(hwnd, msg, wparam, lparam) : 0;
}

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include <wingdi.h>
#include "dwin.h"
#include "../theme.h"

#define ITEM_WIDTH 7
#define ITEM_HEIGHT 16

void memhex_paint(DBG_MEMHEX_DATA *dat, HDC dc) {
	if (activate_calculator(dat->slot)) return;
	HFONT of = SelectObject(dc, font.mono);
	RECT r;
	size_t nc;
	if ((cdat->r.xs < 32 + 7 * ITEM_WIDTH) || (cdat->r.ys < 38)) return;
	r.left = 14;
	r.top = 20;
	r.right = r.left + 6 + (7 + dat->xs * 4) * ITEM_WIDTH;
	r.bottom = r.top + dat->ys * ITEM_HEIGHT + 4;
	nc = (r.right - r.left) / ITEM_WIDTH;
	draw_sunken_box(dc, &r);
	r.left += 2;
	r.top += 2;
	r.right -= 2;
	r.bottom -= 2;
	SetBkMode(dc, TRANSPARENT);
	if ((r.right > r.left) && (r.bottom > r.top)) {
		WORD adr = dat->adr;
		char tmps[8 + 4 * dat->xs], *tmpe;
		int i, j, y;
		if (!is_theme_active()) {
			FillRect(dc, &r, bkg_col[0]);
			PatBlt(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, WHITENESS);
		}
		i = r.right;
		r.right = r.left + 5 * ITEM_WIDTH;
		FillRect(dc, &r, bkg_col[4]);
		r.left = r.right;
		r.right = r.left + (1 + dat->xs * 3) * ITEM_WIDTH;
		if (!is_theme_active()) FillRect(dc, &r, bkg_col[0]);
		r.left = r.right;
		r.right = i;
		FillRect(dc, &r, bkg_col[4]);
		for (i = 0, y = 22; i < dat->ys; i++, y += ITEM_HEIGHT) {
			tmpe = tmps + sprintf(tmps, "%04x ", adr);
			for (j = 0; j < dat->xs; j++, adr++) {
				BYTE br = z80_acc(adr);
				tmps[7 + dat->xs * 3 + j] = (br < 32) ? '.' : br;
				tmpe = tmpe + sprintf(tmpe, " %02x", br);
			}
			tmpe[0] = ' ';
			tmpe[1] = ' ';
			tmps[7 + dat->xs * 3 + j] = 0;
			TextOut(dc, 17, y, tmps, min(nc, strlen(tmps)));
		}
		if (dat->focused) {
			r.left = 16 + ITEM_WIDTH * 6 + dat->x * ITEM_WIDTH * 3;
			r.top = 22 + dat->y * ITEM_HEIGHT;
			r.right = r.left + ITEM_WIDTH * 2 + 4;
			r.bottom = r.top + ITEM_HEIGHT;
			DrawFocusRect(dc, &r);
		}
	}
	SelectObject(dc, of);
}

int memhex_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	DBG_MEMHEX_DATA *dat = cdat->state;
	static const void *cmds[] = {
		&&edit_value, &&go_to, &&go_to_pc, &&go_to_bc, &&go_to_de, &&go_to_hl, &&go_to_sp, &&go_to_ix, &&go_to_iy
	};

	switch (msg) {
		case WM_CREATE: {
			dat->slot = -1;
			dat->adr = 0;
			dat->x = 0;
			dat->y = 0;
			dat->xs = 0;
			dat->ys = 0;
			dat->focused = 0;
			AppendMenu(cdat->menu, MF_SEPARATOR, 0, NULL);
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 0, "Edit value...\tEnter");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 1, "Go to...\tG");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 2, "Go to PC\tP");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 3, "Go to BC\tB");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 4, "Go to DE\tD");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 5, "Go to HL\tH");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 6, "Go to SP\tS");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 7, "Go to IX\tX");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 8, "Go to IY\tY");
		}
		break;
		case WM_PRINT:
			memhex_paint(dat, (HDC)wparam);
		break;
		case WM_SIZE: {
			int w = (LOWORD(lparam) - 32 - 7 * ITEM_WIDTH) / ITEM_WIDTH / 4, h = (HIWORD(lparam) - 38) / ITEM_HEIGHT;
			if ((w != dat->xs) || (h != dat->ys)) {
				dat->xs = w;
				dat->ys = h;
				if (dat->x >= dat->xs) dat->x = dat->xs - 1;
				if (dat->y >= dat->ys) dat->y = dat->ys - 1;
				SendMessage(hwnd, WM_USER, DEBUG_REFRESH, 0);
			}
		}
		break;
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN: {
			int x = LOWORD(lparam), y = HIWORD(lparam);
			if ((x >= 16 + 6 * ITEM_WIDTH) && (x < 16 + 6 * ITEM_WIDTH + 3 * dat->xs * ITEM_WIDTH) &&
				(y >= 22) && (y < 22 + dat->ys * ITEM_HEIGHT)) {
				SetFocus(hwnd);
				dat->focused = 1;
				dat->x = (x - (16 + 6 * ITEM_WIDTH)) / ITEM_WIDTH / 3;
				dat->y = (y - 22) / ITEM_HEIGHT;
				update_component();
			}
		}
		break;
		case WM_SETFOCUS:
			dat->focused = 1;
			update_component();
		break;
		case WM_KILLFOCUS:
			dat->focused = 0;
			update_component();
		break;
		case WM_KEYDOWN:
			switch (wparam) {
				// Navigation
				case VK_UP:
					go_up:
					if (--dat->y == -1) {
						dat->adr -= dat->xs;
						dat->y = 0;
					}
				break;
				case VK_DOWN:
					go_down:
					if (++dat->y == dat->ys) {
						dat->adr += dat->xs;
						dat->y = dat->ys - 1;
					}
				break;
				case VK_LEFT:
					if (--dat->x == -1) {
						dat->x = dat->xs - 1;
						goto go_up;
					}
				break;
				case VK_RIGHT:
					if (++dat->x == dat->xs) {
						dat->x = 0;
						goto go_down;
					}
				break;
				case VK_PRIOR:
					if (dat->y > 0) dat->y = 0;
					else {
						dat->adr -= dat->xs * dat->ys;
						dat->y = 0;
					}
				break;
				case VK_NEXT:
					if (dat->y < dat->ys - 1) dat->y = dat->ys - 1;
					else {
						dat->adr += dat->xs * dat->ys;
						dat->y = dat->ys - 1;
					}
				break;
				case 'P': go_to_pc: dat->adr = R_PC; goto go_val;
				case 'B': go_to_bc: dat->adr = R_BC; goto go_val;
				case 'D': go_to_de: dat->adr = R_DE; goto go_val;
				case 'H': go_to_hl: dat->adr = R_HL; goto go_val;
				case 'S': go_to_sp: dat->adr = R_SP; goto go_val;
				case 'X': go_to_ix: dat->adr = R_IX; goto go_val;
				case 'Y': go_to_iy: dat->adr = R_IY; goto go_val;
				case 'G': go_to:
					if (debug_read_address(hwnd)) {
						dat->adr = dcom.read_address;
						go_val:
						dat->adr -= dat->xs * (dat->ys >> 1);
						dat->x = 0;
						dat->y = dat->ys >> 1;
					}
				break;
				// Updating byte or word at the current address
				case VK_RETURN: edit_value:
					dcom.read_base = 16;
					if (debug_read_number(hwnd)) {
						WORD a = dat->adr + dat->xs * dat->y + dat->x;
						z80_write(a, dcom.read_number);
						if (GetAsyncKeyState(VK_SHIFT))
							z80_write(a + 1, dcom.read_number >> 8);
					}
				break;
			}
			update_component();
		break;
		case WM_COMMAND: {
			int id = LOWORD(wparam);
			if (HIWORD(wparam) == 0 && id >= DEBUG_CMD_COMPONENT)
				goto *cmds[id - DEBUG_CMD_COMPONENT];
			break;
		}
		case WM_USER:
			switch (wparam) {
			case DEBUG_SLOT:
				dat->slot = lparam;
				break;
			case DEBUG_VARSELECT:
				dat->adr = lparam;
				goto go_val;
			}
		break;
	}
	return 0;
}

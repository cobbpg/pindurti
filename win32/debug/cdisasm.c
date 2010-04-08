#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include <wingdi.h>
#include "dwin.h"
#include "../pindurti.h"
#include "../theme.h"

#define ITEM_WIDTH 7
#define ITEM_HEIGHT 16

void disassembly_paint(DBG_DISASM_DATA *dat, HDC dc) {
	if (activate_calculator(dat->slot)) return;
	HFONT of = SelectObject(dc, font.mono);
	RECT r;
	size_t nc;
	if ((cdat->r.xs < 32) || (cdat->r.ys < 38)) return;
	r.left = 14;
	r.top = 20;
	r.right = cdat->r.xs - 14;
	r.bottom = cdat->r.ys - 14;
	r.right -= (r.right - 20) % ITEM_WIDTH;
	r.bottom -= (r.bottom - 24) % ITEM_HEIGHT;
	nc = (r.right - r.left) / ITEM_WIDTH;
	if ((r.right > r.left) && (r.bottom > r.top)) draw_sunken_box(dc, &r);
	r.left += 2;
	r.top += 2;
	r.right -= 2;
	r.bottom -= 2;
	SetBkMode(dc, TRANSPARENT);
	if ((r.right > r.left) && (r.bottom > r.top)) {
		WORD adr = dat->adr, oadr = 0;
		char tmps[100];
		int i, j, y;
		if (!is_theme_active()) FillRect(dc, &r, bkg_col[0]);
		r.top = 0;
		r.bottom = ITEM_HEIGHT;
		for (i = 0, y = 22; i < dat->items; i++, y += ITEM_HEIGHT) {
			int col = (adr == R_PC);
			oadr = adr;
			sprintf(tmps, "%04x ", adr);
			adr += opcode_name(adr);
			strcpy(tmps + 5, debug_ins);
			for (j = oadr; j < adr; j++)
				if (debug_code_bp[j]) col |= 2;
			if (col) {
				r.top = y;
				r.bottom = y + ITEM_HEIGHT;
				FillRect(dc, &r, bkg_col[col]);
			}
			if ((oadr < R_PC) && (R_PC < adr)) {
				r.top = y;
				r.bottom = y + ITEM_HEIGHT;
				FillRect(dc, &r, bkg_col[(col >> 1) | 4]);
				opcode_name(R_PC);
				sprintf(tmps + strlen(tmps), " [%04x: %s]", R_PC, debug_ins + 9);
			}
			TextOut(dc, 17, y, tmps, min(nc, strlen(tmps)));
			if (dat->focused && (i == dat->caret)) {
				r.top = y;
				r.bottom = y + ITEM_HEIGHT;
				DrawFocusRect(dc, &r);
			}
		}
	}
	SelectObject(dc, of);
}

int disassembly_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	DBG_DISASM_DATA *dat = cdat->state;
	static const void *cmds[] = {
		&&toggle_breakpoint, &&run_from_cursor, &&go_to,
		&&go_to_pc, &&go_to_bc, &&go_to_de, &&go_to_hl, &&go_to_sp, &&go_to_ix, &&go_to_iy
	};

	switch (msg) {
		case WM_CREATE: {
			dat->slot = -1;
			dat->items = 0;
			dat->adr = 0;
			dat->caret = 0;
			dat->focused = 0;
			AppendMenu(cdat->menu, MF_SEPARATOR, 0, NULL);
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 0, "Toggle breakpoint\tF2");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 1, "Set PC to cursor\tR");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 2, "Go to...\tG");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 3, "Go to PC\tP");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 4, "Go to BC\tB");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 5, "Go to DE\tD");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 6, "Go to HL\tH");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 7, "Go to SP\tS");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 8, "Go to IX\tX");
			AppendMenu(cdat->menu, MF_ENABLED | MF_STRING, DEBUG_CMD_COMPONENT + 9, "Go to IY\tY");
		}
		break;
		case WM_PRINT:
			disassembly_paint(dat, (HDC)wparam);
		break;
		case WM_SIZE: {
			int h = (HIWORD(lparam) - 38) / ITEM_HEIGHT;
			if (h != dat->items) {
				dat->items = h;
				SendMessage(hwnd, WM_USER, DEBUG_REFRESH, 0);
			}
		}
		break;
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN: {
			int x = LOWORD(lparam), y = HIWORD(lparam);
			if ((x >= 16) && (x < cdat->r.xs - 16) && (y >= 22) && (y < 22 + dat->items * ITEM_HEIGHT)) {
				SetFocus(hwnd);
				dat->focused = 1;
				dat->caret = (y - 22) / ITEM_HEIGHT;
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
		case WM_KEYDOWN: {
			int i;
			switch (wparam) {
				case VK_UP:
					if (--dat->caret == -1) {
						dat->adr = prev_address(dat->adr);
						dat->caret = 0;
					}
				break;
				case VK_DOWN:
					if (++dat->caret == dat->items) {
						dat->adr += opcode_name(dat->adr);
						dat->caret = dat->items - 1;
					}
				break;
				case VK_PRIOR:
					if (dat->caret > 0) dat->caret = 0;
					else {
						for (i = 0; i < dat->items - 1; i++) dat->adr = prev_address(dat->adr);
						dat->caret = 0;
					}
				break;
				case VK_NEXT:
					if (dat->caret < dat->items - 1) dat->caret = dat->items - 1;
					else {
						for (i = 0; i < dat->items - 1; i++) dat->adr += opcode_name(dat->adr);
						dat->caret = dat->items - 1;
					}
				break;
				case VK_F2: toggle_breakpoint: {
					int adr = dat->adr, i;
					for (i = 0; i < dat->caret; i++)
						adr += opcode_name(adr);
					debug_toggle_code_breakpoint(adr);
				}
				break;
				case 'R': run_from_cursor: {
					int adr = dat->adr, i;
					for (i = 0; i < dat->caret; i++)
						adr += opcode_name(adr);
					R_PC = adr;
					InvalidateRect(debug_window, NULL, FALSE);
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
						for (i = 0; i < ((dat->items - 1) >> 1); i++) dat->adr = prev_address(dat->adr);
						dat->caret = (dat->items - 1) >> 1;
					}
				break;
			}
			update_component();
		}
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
				case DEBUG_REFRESH: {
					int i;
					if (activate_calculator(dat->slot)) break;
					dat->adr = R_PC;
					for (i = 0; i < ((dat->items - 1) >> 1); i++) dat->adr = prev_address(dat->adr);
					dat->caret = (dat->items - 1) >> 1;
				}
				break;
			}
		break;
	}
	return 0;
}


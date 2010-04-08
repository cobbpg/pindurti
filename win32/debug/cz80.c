#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include "dwin.h"

#define NUM_REGS 16

#define COLSIZE1 84
#define COLSIZE2 140
#define COLSIZE3 189
#define COLSIZE4 245

enum cpu_state_reg {
	r_af, r_bc, r_de, r_hl, r_ix, r_iy, r_pc, r_sp,
	r_afs, r_bcs, r_des, r_hls, r_r, r_i, r_im, r_hlt
};

int cst_len[16] = {7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 4, 4, 4, 6};
int cst_str[16] = {3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 2, 2, 3, 5};

int cst_pos[4][NUM_REGS][2] = {
	{{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
	 {0, 8}, {0, 9}, {0,10}, {0,11}, {0,12}, {0,13}, {0,14}, {0,15}},
	{{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
	 {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}},
	{{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {8, 4}, {8, 5},
	 {8, 0}, {8, 1}, {8, 2}, {8, 3}, {17,0}, {17,1}, {17,2}, {17,3}},
	{{0, 0}, {0, 1}, {0, 2}, {0, 3}, {17,0}, {17,1}, {17,2}, {17,3},
	 {8, 0}, {8, 1}, {8, 2}, {8, 3}, {25,0}, {25,1}, {25,2}, {25,3}}
};

int cpu_reg_id(int c, int x, int y) {
	int i, *pp;
	for (pp = (int*)(cst_pos + ((c - 1) % 4)), i = 0; i < NUM_REGS; pp += 2, i++)
		if ((pp[1] == y) && (pp[0] <= x) && (pp[0] + cst_len[i] > x))
			return i;
	return -1;
}

int cpu_reg_value(int id) {
	switch (id) {
		case r_af: return R_AF;
		case r_bc: return R_BC;
		case r_de: return R_DE;
		case r_hl: return R_HL;
		case r_ix: return R_IX;
		case r_iy: return R_IY;
		case r_pc: return R_PC;
		case r_sp: return R_SP;
		case r_afs: return R_AFS;
		case r_bcs: return R_BCS;
		case r_des: return R_DES;
		case r_hls: return R_HLS;
		case r_r: return R_R;
		case r_i: return R_I;
		case r_im: return z80->im;
		case r_hlt: return z80->hlt;
		default: return -1;
	}
}

void cpu_reg_text(int id, char *txt) {
	int val = cpu_reg_value(id);
	if (val == -1) {
		txt[0] = 0;
		return;
	}
	switch (id) {
		case r_r: case r_i: sprintf(txt, "%02x", val); break;
		case r_im: case r_hlt: sprintf(txt, "%d", val); break;
		default: sprintf(txt, "%04x", val); break;
	}
}

void cpu_reg_set(int id, char *txt) {
	char *err;
	int val = strtol(txt, &err, 16);
	if (!txt[0] || err[0]) return;
	switch (id) {
		case r_af: R_AF = val; break;
		case r_bc: R_BC = val; break;
		case r_de: R_DE = val; break;
		case r_hl: R_HL = val; break;
		case r_ix: R_IX = val; break;
		case r_iy: R_IY = val; break;
		case r_pc: R_PC = val; break;
		case r_sp: R_SP = val; break;
		case r_afs: R_AFS = val; break;
		case r_bcs: R_BCS = val; break;
		case r_des: R_DES = val; break;
		case r_hls: R_HLS = val; break;
		case r_r: R_R = val; break;
		case r_i: R_I = val; break;
		case r_im: z80->im = (val & 3) % 3; break;
		case r_hlt: z80->hlt = (val & 3) % 3; break;
	}
}

int cpu_state_calculate_columns(int w) {
	int c = 1;
	if (w >= COLSIZE2) c = 2;
	if (w >= COLSIZE3) c = 3;
	if (w >= COLSIZE4) c = 4;
	return c;
}

void cpu_state_paint(DBG_CPU_DATA *dat, HDC dc) {
	char tmp[1024];
	int tmpl;
	RECT tr;
	if (!z80) tmpl = sprintf(tmp, "N/A");
	else
		switch (dat->columns) {
			case 1:
				tmpl = sprintf(tmp,
"af=%04x\nbc=%04x\nde=%04x\nhl=%04x\nix=%04x\niy=%04x\npc=%04x\nsp=%04x\n\
af'=%04x\nbc'=%04x\nde'=%04x\nhl'=%04x\nr=%02x\ni=%02x\nim=%d\nhalt=%d",
					R_AF, R_BC, R_DE, R_HL, R_IX, R_IY, R_PC, R_SP,
					R_AFS, R_BCS, R_DES, R_HLS, R_R, R_I, z80->im, z80->hlt);
			break;
			case 2:
				tmpl = sprintf(tmp,
"af=%04x af'=%04x\nbc=%04x bc'=%04x\nde=%04x de'=%04x\nhl=%04x hl'=%04x\n\
ix=%04x r=%02x\niy=%04x i=%02x\npc=%04x im=%d\nsp=%04x halt=%d",
					R_AF, R_AFS, R_BC, R_BCS, R_DE, R_DES, R_HL, R_HLS,
					R_IX, R_R, R_IY, R_I, R_PC, z80->im, R_SP, z80->hlt);
			break;
			case 3:
				tmpl = sprintf(tmp,
"af=%04x af'=%04x r=%02x\nbc=%04x bc'=%04x i=%02x\nde=%04x de'=%04x im=%d\nhl=%04x hl'=%04x halt=%d\n\
ix=%04x pc=%04x\niy=%04x sp=%04x",
					R_AF, R_AFS, R_R, R_BC, R_BCS, R_I, R_DE, R_DES, z80->im, R_HL, R_HLS, z80->hlt,
					R_IX, R_PC, R_IY, R_SP);
			break;
			case 4:
				tmpl = sprintf(tmp,
"af=%04x af'=%04x ix=%04x r=%02x\nbc=%04x bc'=%04x iy=%04x i=%02x\n\
de=%04x de'=%04x pc=%04x im=%d\nhl=%04x hl'=%04x sp=%04x halt=%d",
					R_AF, R_AFS, R_IX, R_R, R_BC, R_BCS, R_IY, R_I,
					R_DE, R_DES, R_PC, z80->im, R_HL, R_HLS, R_SP, z80->hlt);
			break;
			default:
				tmp[0] = 0;
				tmpl = 0;
		}
	tr.left = 14;
	tr.top = 20;
	tr.right = cdat->r.xs - 14;
	tr.bottom = cdat->r.ys - 14;
	HFONT fnt = SelectObject(dc, font.mono);
	SetBkMode(dc, TRANSPARENT);
	DrawText(dc, tmp, tmpl, &tr, DT_WORD_ELLIPSIS);
	SelectObject(dc, fnt);
}

void cpu_state_stop_editing(DBG_CPU_DATA *dat) {
	dat->editing = -1;
	SetWindowPos(edit_box, 0, 0, 0, 0, 0, SWP_NOZORDER);
	ShowWindow(edit_box, TRUE);
}

int cpu_state_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	UNUSED_PARAMETER(hwnd);
	DBG_CPU_DATA *dat = cdat->state;

	switch (msg) {
		case WM_CREATE: {
			dat->editing = -1;
		}
		break;
		case WM_LBUTTONDOWN: {
			if (activate_calculator(dat->slot)) break;
			int x = (LOWORD(lparam) - 14) / 7, y = (HIWORD(lparam) - 20) / 14;
			int reg = cpu_reg_id(dat->columns, x, y);
			if (reg == dat->editing) break;
			if (dat->editing != -1) {
				char tmp[8];
				SendMessage(edit_box, WM_GETTEXT, 8, (LPARAM)tmp);
				cpu_reg_set(dat->editing, tmp);
			}
			if (reg != -1) {
				char tmp[8];
				dat->editing = reg;
				cpu_reg_text(reg, tmp);
				SetWindowPos(edit_box, HWND_TOP,
					cdat->r.x + (cst_pos[(dat->columns - 1) % 4][reg][0] + cst_str[reg]) * 7 + 14,
					cdat->r.y + cst_pos[(dat->columns - 1) % 4][reg][1] * 14 + 20,
					(cst_len[reg] - cst_str[reg]) * 7, 14, SWP_HIDEWINDOW);
				SendMessage(edit_box, WM_SETTEXT, 0, (LPARAM)tmp);
				SendMessage(edit_box, EM_SETLIMITTEXT, cst_len[reg] - cst_str[reg], 0);
				ShowWindow(edit_box, TRUE);
				SetFocus(edit_box);
			} else cpu_state_stop_editing(dat);
			update_component();
		}
		break;
		case WM_PRINT:
			cpu_state_paint(dat, (HDC)wparam);
		break;
		case WM_SIZE: {
			int w = LOWORD(lparam);
			int reg = dat->editing;
			dat->columns = cpu_state_calculate_columns(w);
			if (reg != -1) {
				SetWindowPos(edit_box, 0,
					cdat->r.x + (cst_pos[(dat->columns - 1) % 4][reg][0] + cst_str[reg]) * 7 + 14,
					cdat->r.y + cst_pos[(dat->columns - 1) % 4][reg][1] * 14 + 20,
					(cst_len[reg] - cst_str[reg]) * 7, 14, SWP_NOZORDER);
			}
		}
		break;
		case WM_USER:
			switch (wparam) {
				case DEBUG_SLOT:
					dat->slot = lparam;
				break;
				case DEBUG_EDITED:
					if (dat->editing != -1) {
						char tmp[8];
						SendMessage(edit_box, WM_GETTEXT, 8, (LPARAM)tmp);
						cpu_reg_set(dat->editing, tmp);
						cpu_state_stop_editing(dat);
					}
				break;
				case DEBUG_NOEDIT:
				case DEBUG_REFRESH:
				case DEBUG_STARTDRAG:
					cpu_state_stop_editing(dat);
				break;
			}
		break;
	}
	return 0;
}


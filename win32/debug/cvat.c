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

int vat_entry_name(int adr, char *desc) {
	BYTE vtype, vpage, tok1, tok2;
	WORD vadr;
	int plus = (calc[running_calculator].rom_ver & FILE_MODEL_MASK) == FILE_MODEL_83P;
	if (plus) {
		vtype = z80_acc(adr);
		adr -= 3;
		vadr = z80_acc(adr) + (z80_acc(adr - 1) << 8);
		adr -= 2;
		vpage = z80_acc(adr--);
		desc += sprintf(desc, "%02x:%04x ", vpage, vadr);
	} else {
		vtype = z80_acc(adr--);
		vadr = z80_acc(adr) + (z80_acc(adr - 1) << 8);
		adr -= 2;
		desc += sprintf(desc, "%04x ", vadr);
	}
	switch (vtype & 0x1f) {
	case 0x00: // real number
		desc += sprintf(desc, "Real ");
		goto token;
	case 0x02: // matrix
		desc += sprintf(desc, "Matrix ");
		goto token;
	case 0x03: // equation
		desc += sprintf(desc, "EquVar ");
		goto token;
	case 0x04: // string
		desc += sprintf(desc, "String ");
		goto token;
	case 0x07: // picture
		desc += sprintf(desc, "Picture ");
		goto token;
	case 0x08: // GDB
		desc += sprintf(desc, "GDB ");
		goto token;
	case 0x0b: // new equation
		desc += sprintf(desc, "NewEquVar ");
		goto token;
	case 0x0c: // complex number
		desc += sprintf(desc, "Complex ");
		token:
		tok1 = z80_acc(adr--);
		tok2 = z80_acc(adr--);
		switch (tok1) {
		case 0x5b: // theta
			sprintf(desc, "Theta");
			break;
		case 0x5c: // matrix
			sprintf(desc, "[%c]", tok2 + 'A');
			break;
		case 0x5d: // list
			sprintf(desc, "L%d", (tok2 & 0x0f) + 1);
			break;
		case 0x5e: // equation
			if (tok2 & 0x10) sprintf(desc, "Y%d", ((tok2 & 0x0f) + 1) % 10);
			else if (tok2 & 0x20) sprintf(desc, "%c%dt",
				tok2 & 1 ? 'Y' : 'X', ((tok2 >> 1) & 0x07) + 1);
			else if (tok2 & 0x40) sprintf(desc, "r%d", (tok2 & 0x0f) + 1);
			else if (tok2 & 0x80) sprintf(desc, "%c", (tok2 & 0x0f) + 'u');
			break;
		case 0x5f: // program
			break;
		case 0x60: // picture
			sprintf(desc, "Pic%c", (tok2 + 1) % 10 + '0');
			break;
		case 0x61: // GDB
			sprintf(desc, "GDB%c", (tok2 + 1) % 10 + '0');
			break;
		case 0x62: // system output only
			sprintf(desc, "SysOut $%02x", tok2);
			break;
		case 0x63: // system input/output
			sprintf(desc, "SysIO $%02x", tok2);
			break;
		case 0x72: // ans
			sprintf(desc, "Ans");
			break;
		case 0xaa: // string
			sprintf(desc, "Str%c", (tok2 + 1) % 10 + '0');
			break;
		default:
			if ((tok1 >= 'A') && (tok1 <= 'Z')) sprintf(desc, "%c", tok1);
			else sprintf(desc, "%02x:%02x%02x", vtype, tok1, tok2);
		}
		adr--;
		break;
	case 0x01: // list
	case 0x0d: { // complex list
		BYTE nl = z80_acc(adr--);
		adr--;
		BYTE name[6];
		int i;
		if (nl > 6) return 0;
		for (i = 0; i < nl - 1; i++) {
			name[i] = z80_acc(adr--);
		}
		name[i] = 0;
		if (name[0] < 'A') sprintf(desc, "List L%d", (name[0] & 0x0f) + 1);
		else sprintf(desc, "List L.%s", name);
		if (plus) adr--;
		break;
	}
	case 0x05: // program
	case 0x06: // program (protected)
	case 0x15: // appvar
	case 0x16: { // temporary program
		BYTE nl = z80_acc(adr--);
		char name[9], ntype[10];
		int i;
		if (nl > 8) return 0;
		for (i = 0; i < nl; i++) {
			name[i] = z80_acc(adr--);
		}
		name[i] = 0;
		switch (vtype & 0x1f) {
			case 0x05: strcpy(ntype, "Program"); break;
			case 0x06: strcpy(ntype, "Program*"); break;
			case 0x15: strcpy(ntype, "AppVar"); break;
			case 0x16: strcpy(ntype, "Temp"); break;
		}
		sprintf(desc, "%s %s", ntype, name);
		break;
	}
	default: return 0;
	}
	return vadr ? adr : 0;
}

int vat_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	DBG_VAT_DATA *dat = cdat->state;

	switch (msg) {
		case WM_CREATE: {
			dat->slot = -1;
			dat->lview = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL,
				0, 0, 0, 0, hwnd, (HMENU)0, NULL, NULL);
#define LVS_EX_DOUBLEBUFFER 0x00010000 // Missing declaration from my commctrl.h!!! needed for new theming...
			ListView_SetExtendedListViewStyle(dat->lview, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
			LVCOLUMN lvc;
			lvc.fmt = LVCFMT_LEFT;
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.pszText = "Address";
			lvc.iSubItem = 0;
			lvc.cx = 60;
			ListView_InsertColumn(dat->lview, 0, &lvc);
			lvc.pszText = "Type";
			lvc.iSubItem = 1;
			lvc.cx = 60;
			ListView_InsertColumn(dat->lview, 1, &lvc);
			lvc.pszText = "Name";
			lvc.iSubItem = 1;
			lvc.cx = 100;
			ListView_InsertColumn(dat->lview, 2, &lvc);
		}
		break;
		case WM_DESTROY:
			DestroyWindow(dat->lview);
		break;
		case WM_PRINT:
		break;
		case WM_SIZE: {
			SetWindowPos(dat->lview, HWND_TOP, 14, 20, cdat->r.xs - 28, cdat->r.ys - 34, 0);
		}
		break;
		case WM_NOTIFY: {
			switch (((NMHDR*)lparam)->code) {
			case NM_DBLCLK: {
				char tmps[20];
				int tmpa;
				ListView_GetItemText(dat->lview, ((NMITEMACTIVATE*)lparam)->iItem, 0, tmps, 19);
				if (tmps[2] == ':')
					tmpa = strtol(tmps + 3, NULL, 16) + (strtol(tmps, NULL, 16) << 16);
				else
					tmpa = strtol(tmps, NULL, 16);
				SendMessage(debug_window, WM_USER, DEBUG_VARSELECT, tmpa);
				break;
			}}
		}
		break;
		case WM_USER:
			switch (wparam) {
				case DEBUG_SLOT:
					dat->slot = lparam;
				break;
				case DEBUG_REFRESH: {
					if (activate_calculator(dat->slot)) break;
					int adr, i;
					char tmps[1000], *tmpp;
					switch (calc[running_calculator].rom_ver & FILE_MODEL_MASK) {
						case FILE_MODEL_82:
						case FILE_MODEL_82b:
						case FILE_MODEL_83: adr = 0xfe6e; break;
						case FILE_MODEL_83P: adr = 0xfe66; break;
						default: adr = 0;
					}
					SetWindowRedraw(dat->lview, FALSE);
					ListView_DeleteAllItems(dat->lview);
					for (i = 0; adr; i++) {
						adr = vat_entry_name(adr, tmps);
						if (!adr) break;
						// TI-83+ app
						if ((tmps[2] == ':') && ((tmps[0] != '0') || (tmps[1] != '0'))) {
							i--;
							continue;
						}
						tmpp = tmps;
						LVITEM lvi;
						lvi.mask = LVIF_TEXT;
						lvi.pszText = tmpp;
						lvi.iSubItem = 0;
						lvi.iItem = i;
						int j;
						for (j = 0; tmpp[j]; j++) if (tmpp[j] == ' ') { tmpp[j] = 0; break; }
						ListView_InsertItem(dat->lview, &lvi);
						tmpp += strlen(tmpp) + 1;
						for (j = 0; tmpp[j]; j++) if (tmpp[j] == ' ') { tmpp[j] = 0; break; }
						ListView_SetItemText(dat->lview, i, 1, tmpp);
						tmpp += strlen(tmpp) + 1;
						ListView_SetItemText(dat->lview, i, 2, tmpp);
//						if (tmps[strlen(tmps) - 1] == '!') break;
					}
					SetWindowRedraw(dat->lview, TRUE);
					UpdateWindow(dat->lview);
					//dat->last = i - 1;
				}
				break;
			}
		break;
	}
	return 0;
}

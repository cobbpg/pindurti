#define _WIN32_IE 0x400
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include <uxtheme.h>
#include <tmschema.h>
#include "dwin.h"

#define ADD_COMP(p, c) dtree_add_child(p, c, -1)
#define ADD_NEW_COMP(p, ct, sl, s) dtree_add_child(p, dtree_new_component(DEBUG_COMP_ ## ct, sl, s), -1)

const struct {
	char *name;
	int type;
	LPARAM par;
} debug_comp_par[] = {
	{ "Disassembler", DEBUG_COMP_DISASM, 0 },
	{ "Memory viewer", DEBUG_COMP_MEMHEX, 0 },
	{ "TIOS VAT", DEBUG_COMP_VAT, 0 },
	{ "Z80 registers", DEBUG_COMP_Z80, 0 },
	{ "LCD data", DEBUG_COMP_LCDDATA, 0 },
	{ "LCD physics", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_LCD_P) },
	{ "LCD software", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_LCD_S) },
	{ "Model", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_INFO) },
	{ "Time", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_TIME) },
	{ "Interrupts", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_INTERRUPT) },
	{ "Memory mapping", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_MEMORY) },
	{ "Keyboard state", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_KEYBOARD) },
	{ "Link state", DEBUG_COMP_KEYVAL, MAKELPARAM(0, KEYVAL_LINK) },
	{ "Link history", DEBUG_COMP_LINKHISTORY, 0 },
	{ "Logger", DEBUG_COMP_LOGGER, 0 },
	{ NULL, 0, 0 }
};

DT_NODE *dt_root;

HWND debug_window, debug_parent, edit_box, layout_tree;
DT_COMPONENT *cdat;

int debug_shown, layout_shown;

DEBUG_FONT_DATA font;
HBRUSH bkg_col[6];
HCURSOR cur_arr, cur_horz, cur_vert, cur_mod;

struct {
	int dir;
	DT_NODE *comp;
	RECT area;
} drag;

struct {
/*	HDC dc, cdc;
	HBITMAP bm, obm;
	int cx, cy;
	int on;*/
	LONG msg_time;
} dbdat;

void update_component() {
//	RECT wr;
//	if ((cdat->r.xs <= 0) || (cdat->r.ys <= 0)) return;
/*	wr.left = cdat->r.x;
	wr.top = cdat->r.y;
	wr.right = wr.left + cdat->r.xs;
	wr.bottom = wr.top + cdat->r.ys;
	InvalidateRect(debug_window, &wr, TRUE);
	UpdateWindow(debug_window);*/
	InvalidateRgn(cdat->hwnd, NULL, FALSE);
	UpdateWindow(cdat->hwnd);
}

void debug_draw_child(HWND hwnd, RECT *rect) {
	RECT wc, nwc;
	if (rect != NULL) {
		GetWindowRect(hwnd, &wc);
		if (!IntersectRect(&nwc, &wc, rect)) return;
	}
	InvalidateRgn(hwnd, NULL, FALSE);
	UpdateWindow(hwnd);
/*	SetViewportOrgEx(dbdat.cdc, wc.left, wc.top, NULL);
	SendMessage(hwnd, WM_PRINT, (WPARAM)dbdat.cdc, PRF_CLIENT | PRF_NONCLIENT);
	SetViewportOrgEx(dbdat.cdc, 0, 0, NULL);*/
}

/*void debug_show_children(int show) {
	UNUSED_PARAMETER(show);
	dtree_show(dt_root, TRUE);
//	dtree_show(dt_root, show);
}*/

void debug_draw_window(RECT *rect) {
//	UNUSED_PARAMETER(rect);
//	FillRect(dbdat.cdc, rect, GetSysColorBrush(COLOR_BTNFACE));
//	dtree_draw_leaves(dt_root, rect);
/*	BitBlt(dbdat.dc, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
		dbdat.cdc, rect->left, rect->top, SRCCOPY);*/
	if (GetMessageTime() < dbdat.msg_time) return;
	dtree_draw_leaves(dt_root, rect);
	UpdateWindow(edit_box);
	dbdat.msg_time = GetTickCount();
}

HTREEITEM debug_layout_insert(DT_NODE *node, HTREEITEM par, HTREEITEM prev) {
	CHAR tmp[1000];
	TVINSERTSTRUCT is;
	is.hParent = par;
	is.hInsertAfter = prev;
	is.itemex.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	is.itemex.state = TVIS_EXPANDED;
	is.itemex.stateMask = TVIS_EXPANDED;
	switch (node->tag) {
	case dtt_node:
//		sprintf(tmp, "%s [%d]", node->dat.node.dir == dtd_horz ? "|||" : "==", dtree_num_children(node));
		sprintf(tmp, "%s", node->dat.node.dir == dtd_horz ? "|||" : "==");
		break;
	case dtt_leaf:
		// Slot hack!!!
		sprintf(tmp, "%s (%d)", node->dat.leaf->name, *((int*)node->dat.leaf->state));
		break;
	}
	is.itemex.pszText = tmp;
	is.itemex.cchTextMax = strlen(is.itemex.pszText);
	is.itemex.lParam = (LPARAM)node;
	return TreeView_InsertItem(layout_tree, &is);
}

void debug_layout_tree(DT_NODE *node, HTREEITEM par) {
	HTREEITEM cur = debug_layout_insert(node, par, TVI_LAST);
	if (node->tag == dtt_node)
		for (node = node->dat.node.child; node != NULL; node = node->sibling)
			debug_layout_tree(node, cur);
}

void debug_layout_update() {
	TreeView_DeleteAllItems(layout_tree);
	debug_layout_tree(dt_root, NULL);
}

WNDPROC default_edit_proc;

LRESULT CALLBACK debug_edit_box_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_KEYDOWN)
		switch (wparam) {
		case VK_RETURN: SendMessage(debug_window, WM_COMMAND, DEBUG_CMD_EDIT_OK, 0); break;
		case VK_ESCAPE: SendMessage(debug_window, WM_COMMAND, DEBUG_CMD_EDIT_CANCEL, 0); break;
		}
	return (LRESULT)CallWindowProc((WNDPROC)default_edit_proc, hwnd, msg, wparam, lparam);
}

WNDPROC default_list_proc;

struct {
	HTREEITEM ti;
	int pos;
} ntidat;

LRESULT CALLBACK debug_component_list_box_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_LBUTTONDBLCLK:
		goto addnode;
	case WM_KEYDOWN:
		switch (wparam) {
		case VK_RETURN: {
			TVITEMEX tvi;
			addnode:
			tvi.hItem = TreeView_GetParent(layout_tree, ntidat.ti);
			if (tvi.hItem != NULL) {
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_GetItem(layout_tree, &tvi);
				DT_NODE *pnode = (DT_NODE*)tvi.lParam;
				int idx = ListBox_GetCaretIndex(hwnd);
				ShowWindow(hwnd, FALSE);
				UpdateWindow(layout_tree);
				DT_NODE *nnode = dtree_new_component(debug_comp_par[idx].type,
					debug_comp_par[idx].par, 1.0 / (dtree_num_children(pnode) + 1));
				debug_layout_insert(nnode, tvi.hItem, ntidat.ti);
				dtree_add_child(pnode, nnode, ntidat.pos);
				dtree_normalise(dt_root);
				dtree_set_pos(dt_root, 0, 0, dt_root->r.xs, dt_root->r.ys);
//				InvalidateRgn(debug_window, NULL, FALSE);
//				UpdateWindow(debug_window);
			}
			DestroyWindow(hwnd);
			break;
		}
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		break;
	case WM_KILLFOCUS:
		DestroyWindow(hwnd);
		return 0;
	case WM_NCDESTROY:
		InvalidateRgn(layout_tree, NULL, FALSE);
		UpdateWindow(layout_tree);
		break;
	}
	return (LRESULT)CallWindowProc((WNDPROC)default_list_proc, hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK debug_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE: {
		int i;

		// GDI objects
		COLORREF rgb[6] = {
			RGB(255, 255, 255), RGB(215, 215, 255),
			RGB(215, 255, 215), RGB(205, 235, 235),
			RGB(235, 235, 235), RGB(215, 235, 215)};
//		NONCLIENTMETRICS ncms;
//		ncms.cbSize = sizeof(NONCLIENTMETRICS);
//		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncms, 0);
//		font.sans = CreateFontIndirect(&ncms.lfMenuFont);
		font.sans = GetStockObject(DEFAULT_GUI_FONT);
		font.mag = 1.0;
		font.mono = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
//		font.sans = CreateFont(-11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
		for (i = 0; i < 6; i++)
			bkg_col[i] = CreateSolidBrush(rgb[i]);
		cur_arr = LoadCursor(NULL, IDC_ARROW);
		cur_horz = LoadCursor(NULL, IDC_SIZEWE);
		cur_vert = LoadCursor(NULL, IDC_SIZENS);
		cur_mod = LoadCursor(NULL, IDC_HAND);

		// Double buffering
		//dbdat.on = 0;
		//dbdat.dc = GetDC(hwnd);
		//dbdat.cdc = CreateCompatibleDC(dbdat.dc);
		//dbdat.bm = CreateCompatibleBitmap(dbdat.dc, 2048, 1536);
		//dbdat.obm = SelectObject(dbdat.cdc, dbdat.bm);
		POINT pt = {0, 0};
		ScreenToClient(hwnd, &pt);
		//dbdat.cx = pt.x;
		//dbdat.cy = pt.y;
		dbdat.msg_time = GetTickCount();

		// Customised edit box
		edit_box = CreateWindowEx(0, "EDIT", "",
			WS_CHILD | ES_LOWERCASE, 0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
		default_edit_proc = (WNDPROC)GetWindowLong(edit_box, GWL_WNDPROC);
		SetWindowLong(edit_box, GWL_WNDPROC, (long)debug_edit_box_proc);
		SendMessage(edit_box, WM_SETFONT, (WPARAM)font.mono, 0);

		// Component tree display
		layout_tree = CreateWindowEx(0, WC_TREEVIEW, "Layout Editor",
			TVS_HASLINES | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP,
			CW_USEDEFAULT, CW_USEDEFAULT, 200, 400, hwnd, NULL, GetModuleHandle(NULL), NULL);
		layout_shown = 0;

		// Component tree
		debug_parent = hwnd;
		dt_root = dtree_new_frame(dtd_horz, 1.0);
		DT_NODE *dtn_left = dtree_new_frame(dtd_vert, 3.0);
		DT_NODE *dtn_right = dtree_new_frame(dtd_vert, 1.0);
		DT_NODE *dtn_lefttxt = dtree_new_frame(dtd_horz, 3.0);
		DT_NODE *dtn_leftmem = dtree_new_frame(dtd_horz, 1.0);
		DT_NODE *dtn_leftlcd = dtree_new_frame(dtd_horz, 1.0);
		ADD_COMP(dt_root, dtn_left);
		ADD_COMP(dt_root, dtn_right);
		ADD_NEW_COMP(dtn_lefttxt, DISASM, 0, 1.0);
		ADD_NEW_COMP(dtn_lefttxt, LOGGER, 0, 1.0);
		ADD_COMP(dtn_left, dtn_lefttxt);
		ADD_NEW_COMP(dtn_left, LINKHISTORY, 0, 0.5);
		ADD_COMP(dtn_left, dtn_leftmem);
		ADD_COMP(dtn_left, dtn_leftlcd);
		ADD_NEW_COMP(dtn_leftmem, MEMHEX, 0, 3.0);
		ADD_NEW_COMP(dtn_leftmem, VAT, 0, 1.0);
		ADD_NEW_COMP(dtn_leftlcd, KEYVAL, MAKELPARAM(0, KEYVAL_LCD_P), 3.0);
		ADD_NEW_COMP(dtn_leftlcd, KEYVAL, MAKELPARAM(0, KEYVAL_LCD_S), 3.0);
		ADD_NEW_COMP(dtn_leftlcd, LCDDATA, 0, 4.0);
		ADD_NEW_COMP(dtn_right, Z80, 0, 3.0);
		ADD_NEW_COMP(dtn_right, KEYVAL, MAKELPARAM(0, KEYVAL_INFO), 1.5);
		ADD_NEW_COMP(dtn_right, KEYVAL, MAKELPARAM(0, KEYVAL_TIME), 1.5);
		ADD_NEW_COMP(dtn_right, KEYVAL, MAKELPARAM(0, KEYVAL_INTERRUPT), 3.5);
		ADD_NEW_COMP(dtn_right, KEYVAL, MAKELPARAM(0, KEYVAL_MEMORY), 3.5);
		ADD_NEW_COMP(dtn_right, KEYVAL, MAKELPARAM(0, KEYVAL_KEYBOARD), 4.5);
		ADD_NEW_COMP(dtn_right, KEYVAL, MAKELPARAM(0, KEYVAL_LINK), 2.5);
		dtree_normalise(dt_root);

		debug_layout_update();

		// Other
		drag.dir = dtd_none;
		break;
	}
	case WM_DESTROY: {
		int i;
		//SelectObject(dbdat.cdc, dbdat.obm);
		//DeleteObject(dbdat.bm);
		//DeleteDC(dbdat.cdc);
		//ReleaseDC(hwnd, dbdat.dc);
		DeleteObject(font.mono);
//		DeleteObject(font.sans);
		for (i = 0; i < 6; i++)
			DeleteObject(bkg_col[i]);
		break;
	}
	case WM_CLOSE:
		debug_shown = 0;
		layout_shown = 0;
		ShowWindow(layout_tree, layout_shown);
		show_debug_window(0);
		break;
	case WM_PAINT: {
		RECT udr;
		GetUpdateRect(hwnd, &udr, FALSE);
		POINT pt = {0, 0};
		ScreenToClient(hwnd, &pt);
		OffsetRect(&udr, pt.x, pt.y);
		debug_draw_window(&udr);
		ValidateRgn(hwnd, NULL);
		break;
	}
	case WM_ENTERSIZEMOVE: {
		RECT cr;
		GetClientRect(hwnd, &cr);
//		debug_show_children(FALSE);
//		debug_draw_window(&cr);
		//dbdat.on = 1;
		break;
	}
	case WM_EXITSIZEMOVE:
//		debug_show_children(TRUE);
//		debug_draw_window(NULL);
//		UpdateWindow(hwnd);
		//dbdat.on = 0;
		break;
	case WM_MOVE: {
//		POINT pt = {0, 0};
//		RECT cr;
//		ScreenToClient(hwnd, &pt);
		//dbdat.cx = pt.x;
		//dbdat.cy = pt.y;
//		GetClientRect(hwnd, &cr);
//		debug_draw_window(&cr);
//		BitBlt(dbdat.dc, 0, 0, cr.right, cr.bottom, dbdat.cdc, 0, 0, SRCCOPY);
//		ValidateRgn(hwnd, NULL);
		break;
	}
	case WM_SIZE: {
		int xs = LOWORD(lparam), ys = HIWORD(lparam);
		//POINT pt = {0, 0};
		dtree_set_pos(dt_root, 0, 0, xs, ys);
		//ScreenToClient(hwnd, &pt);
		//dbdat.cx = pt.x;
		//dbdat.cy = pt.y;
		SendMessage(hwnd, WM_USER, DEBUG_REFRESH, 0);
		break;
	}
	case WM_LBUTTONDOWN: {
		DT_NODE *dnode;
		drag.dir = dtree_pointed(dt_root, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), &dnode);
		if (drag.dir != dtd_none) {
			drag.comp = dnode;
			drag.area.left = dnode->r.x;
			drag.area.top = dnode->r.y;
			drag.area.right = dnode->r.x + dnode->r.xs;
			drag.area.bottom = dnode->r.y + dnode->r.ys;
			switch (drag.dir) {
			case dtd_horz: drag.area.right += dnode->sibling->r.xs; break;
			case dtd_vert: drag.area.bottom += dnode->sibling->r.ys; break;
			}
//			debug_show_children(FALSE);
//			dtree_show(dnode, FALSE);
//			dtree_show(dnode->sibling, FALSE);
			SetCapture(hwnd);
			SendMessage(hwnd, WM_USER, DEBUG_STARTDRAG, 0);
			//dbdat.on = 1;
		}
		break;
	}
	case WM_LBUTTONUP:
		drag.dir = dtd_none;
		//dbdat.on = 0;
//		debug_show_children(TRUE);
		ReleaseCapture();
		break;
	case WM_CAPTURECHANGED:
		drag.dir = dtd_none;
		//dbdat.on = 0;
//		debug_show_children(TRUE);
		break;
	case WM_MOUSEMOVE: {
		DT_NODE *dnode;
		if (GetMessageTime() < dbdat.msg_time) break;
		short int x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);
		switch (drag.dir) {
		case dtd_none:
			switch (dtree_pointed(dt_root, x, y, &dnode)) {
			case dtd_horz: SetCursor(cur_horz); break;
			case dtd_vert: SetCursor(cur_vert); break;
			default: SetCursor(cur_arr);
			}
			break;
		case dtd_horz:
			dtree_drag_edge(drag.comp, dtd_horz, x);
//			dtree_draw_leaves(dnode, NULL);
//			dtree_draw_leaves(dnode->sibling, NULL);
			dbdat.msg_time = GetTickCount();
//			debug_draw_window(&drag.area);
			break;
		case dtd_vert:
			dtree_drag_edge(drag.comp, dtd_vert, y);
//			dtree_draw_leaves(dnode, NULL);
//			dtree_draw_leaves(dnode->sibling, NULL);
			dbdat.msg_time = GetTickCount();
//			debug_draw_window(&drag.area);
			break;
		}
		break;
	}
	case WM_USER: {
		switch (wparam) {
		case DEBUG_RESLOT:
			dtree_broadcast(dt_root, WM_USER, DEBUG_RESLOT, 0);
			dtree_broadcast(dt_root, WM_USER, DEBUG_REFRESH, 0);
			break;
		default: dtree_broadcast(dt_root, WM_USER, wparam, lparam);
		}
/*		RECT cr;
		cr.left = dt_root->r.x;
		cr.top = dt_root->r.y;
		cr.right = cr.left + dt_root->r.xs;
		cr.bottom = cr.top + dt_root->r.ys;
		debug_draw_window(&cr);*/
		debug_draw_window(NULL);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case DEBUG_CMD_STEP: {
			extern int active_slot;
			calculator_step(active_slot);
			SendMessage(hwnd, WM_USER, DEBUG_REFRESH, 0);
			break;
		}
		case DEBUG_CMD_STEP_OVER: {
			extern int active_slot;
			if (debug_step_instruction(active_slot)) {
//				debug_shown = 0;
				layout_shown = 0;
				ShowWindow(layout_tree, layout_shown);
//				show_debug_window(0);
			}
			calculator_step_over(active_slot);
			SendMessage(hwnd, WM_USER, DEBUG_REFRESH, 0);
			break;
		}
		case DEBUG_CMD_LAYOUT_EDITOR:
			if (debug_shown) {
				layout_shown ^= 1;
				ShowWindow(layout_tree, layout_shown);
				UpdateWindow(layout_tree);
			}
			break;
		case DEBUG_CMD_EDIT_OK: SendMessage(hwnd, WM_USER, DEBUG_EDITED, 0); break;
		case DEBUG_CMD_EDIT_CANCEL: SendMessage(hwnd, WM_USER, DEBUG_NOEDIT, 0); break;
		case DEBUG_CMD_CODEBP: {
			//
			break;
		}
		case DEBUG_CMD_FONT_BIG:
		// TODO
//			DeleteObject(font.mono);
//			DeleteObject(font.sans);

			break;
		case DEBUG_CMD_FONT_SMALL:
			break;
		}
//		if (((HWND)lparam == edit_box) && (HIWORD(wparam) == EN_UPDATE))
//			SendMessage(hwnd, WM_USER, DEBUG_EDITED, 0);
		break;
	case WM_NOTIFY: {
		UINT code = ((NMHDR*)lparam)->code;
		if (((NMHDR*)lparam)->hwndFrom == layout_tree)
			switch (code) {
			case TVN_KEYDOWN: {
				int key = ((TV_KEYDOWN*)lparam)->wVKey;
				HTREEITEM ti = TreeView_GetSelection(layout_tree);
				switch (key) {
				case VK_DELETE:
					if (ti != NULL) {
						TVITEMEX tvi;
						tvi.mask = TVIF_HANDLE | TVIF_PARAM;
						tvi.hItem = ti;
						TreeView_GetItem(layout_tree, &tvi);
						TreeView_DeleteItem(layout_tree, ti);
						dtree_delete_from(dt_root, (DT_NODE*)tvi.lParam);
						dtree_normalise(dt_root);
						dtree_set_pos(dt_root, 0, 0, dt_root->r.xs, dt_root->r.ys);
//						InvalidateRgn(debug_window, NULL, FALSE);
//						UpdateWindow(debug_window);
					}
					break;
				case VK_INSERT:
					if (ti != NULL) {
						TVITEMEX tvi;
						tvi.hItem = TreeView_GetParent(layout_tree, ti);
						if (tvi.hItem == NULL) break;

						int i;
						HTREEITEM tic;
						for (i = 0, tic = ti; tic != NULL; tic = TreeView_GetPrevSibling(layout_tree, tic), i++);
						RECT r;
						TreeView_GetItemRect(layout_tree, ti, &r, TRUE);

						ntidat.ti = ti;
						ntidat.pos = i;
						HWND clist = CreateWindowEx(0, "LISTBOX", "",
							WS_CHILD | WS_BORDER | WS_HSCROLL | WS_VSCROLL, r.left, r.top, 120, 140, layout_tree, NULL, GetModuleHandle(NULL), NULL);
						default_list_proc = (WNDPROC)GetWindowLong(clist, GWL_WNDPROC);
						SetWindowLong(clist, GWL_WNDPROC, (long)debug_component_list_box_proc);
						SendMessage(clist, WM_SETFONT, (WPARAM)font.sans, 0);
						for (i = 0; debug_comp_par[i].name; i++)
							ListBox_AddString(clist, debug_comp_par[i].name);
						SetFocus(clist);
						UpdateWindow(clist);
						ShowWindow(clist, TRUE);
					}
					break;
				default:
					if (ti != NULL && key >= '0' && key <= '3') {
						TVITEMEX tvi;
						tvi.mask = TVIF_HANDLE | TVIF_PARAM;
						tvi.hItem = ti;
						TreeView_GetItem(layout_tree, &tvi);
						dtree_broadcast((DT_NODE*)tvi.lParam, WM_USER, DEBUG_SLOT, key - '0');
						debug_layout_update();
//						InvalidateRgn(debug_window, NULL, FALSE);
//						UpdateWindow(debug_window);
					}
				}
				break;
			}
			}
		break;
	}
/*	case WM_KEYDOWN: {
//		char tmp[128];
//		GetKeyNameText(lparam, tmp, 128);
//		MessageBox(NULL, tmp, "Info", MB_OK);
	}*/
	default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

int register_window_class(const char *cname, WNDPROC win_proc) {
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = strcmp(cname, "PindurDebug") ? CS_CLASSDC | CS_PARENTDC : CS_OWNDC;
	wc.lpfnWndProc = win_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(void*);
	wc.hInstance = GetModuleHandle(0);
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = cname;
	wc.hIconSm = NULL;

	return RegisterClassEx(&wc);
}

void register_debug_window_classes() {
	register_window_class("PindurDebug", debug_window_proc);
	register_window_class("PindurDebugComponent", component_window_proc);
}

int init_debug_window() {
	debug_window = CreateWindowEx(0, "PindurDebug", "PindurTI Debugger", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandle(0), NULL);

	if (debug_window == NULL) {
		MessageBox(NULL, "Debugger window creation failed.", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	debug_shown = 0;

	return 0;
}

int show_debug_window(int state) {
	ShowWindow(debug_window, state);
	UpdateWindow(debug_window);
	return 0;
}

void *get_window_data(HWND hwnd) {
	return (void*)GetWindowLong(hwnd, 0);
}

void set_window_data(HWND hwnd, void *ptr) {
	SetWindowLong(hwnd, 0, (LONG)ptr);
}

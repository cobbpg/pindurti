#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include "dwin.h"

DT_COMPONENT_CLASS dt_comp[] = {
	{ "KeyValue", keyval_window_proc },
	{ "Disassembly", disassembly_window_proc },
	{ "Registers", cpu_state_window_proc },
	{ "LCD data", lcd_data_window_proc },
	{ "Memory view", memhex_window_proc },
	{ "Variables", vat_window_proc },
	{ "Link history", link_history_window_proc },
	{ "Logger", logger_window_proc },
};

DT_NODE *dtree_new_node(int type, double size) {
	DT_NODE *node = malloc(sizeof(DT_NODE));
	node->tag = type;
	node->size = size;
	node->sibling = NULL;

	switch (type) {
	case dtt_node:
		node->dat.node.child = NULL;
		node->dat.node.dir = dtd_none;
		break;
	case dtt_leaf:
		node->dat.leaf = NULL;
		break;
	}

	return node;
}

DT_NODE *dtree_new_frame(int dir, double size) {
	DT_NODE *node = dtree_new_node(dtt_node, size);
	dtree_set_dir(node, dir);
	return node;
}

DT_NODE *dtree_new_component(int type, int slot, double size) {
	DT_COMPONENT *comp = malloc(sizeof(DT_COMPONENT));
	strcpy(comp->name, dt_comp[type].class_name);
	comp->win_proc = dt_comp[type].win_proc;
	comp->type = type;
	comp->r.x = 0;
	comp->r.y = 0;
	comp->r.xs = 0;
	comp->r.ys = 0;
	comp->hwnd = CreateWindowEx(0, "PindurDebugComponent", "", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN,
		0, 0, 0, 0, debug_parent, NULL, GetModuleHandle(NULL), comp);
	SendMessage(comp->hwnd, WM_USER, DEBUG_SLOT, slot);
	DT_NODE *node = dtree_new_node(dtt_leaf, size);
	dtree_set_component(node, comp);
	return node;
}

void dtree_delete(DT_NODE *node) {
	switch (node->tag) {
	case dtt_node:
		for (node = node->dat.node.child; node != NULL;) {
			DT_NODE *tmp = node->sibling;
			dtree_delete(node);
			node = tmp;
		}
		break;
	case dtt_leaf:
		DestroyWindow(node->dat.leaf->hwnd);
		break;
	}
	free(node);
}

void dtree_delete_from(DT_NODE *par, DT_NODE *node) {
	if (par == node) {
		dtree_delete(node);
		return;
	}

	if (par->tag != dtt_node) return;

	if (par->dat.node.child == node) {
		par->dat.node.child = node->sibling;
		dtree_delete(node);
		return;
	}

	for (par = par->dat.node.child; par->sibling != NULL; par = par->sibling) {
		if (par->sibling == node) {
			par->sibling = node->sibling;
			dtree_delete(node);
			return;
		} else {
			dtree_delete_from(par, node);
		}
	}
	dtree_delete_from(par, node);
}

void dtree_add_child(DT_NODE *node, DT_NODE *child, int pos) {
	if (node->tag != dtt_node) return;

	if (node->dat.node.child == NULL) {
		child->sibling = NULL;
		node->dat.node.child = child;
		return;
	}

	if (pos == 0) {
		child->sibling = node->dat.node.child;
		node->dat.node.child = child;
		return;
	}

	int i;

	for (i = 1, node = node->dat.node.child;
		((i < pos) || (pos < 0)) && (node->sibling != NULL); node = node->sibling, i++);
	child->sibling = node->sibling;
	node->sibling = child;
}

void dtree_set_pos(DT_NODE *node, int x, int y, int xs, int ys) {
	node->r.x = x;
	node->r.y = y;
	node->r.xs = xs;
	node->r.ys = ys;

	if (node->tag == dtt_leaf) {
		node->dat.leaf->r.x = x;
		node->dat.leaf->r.y = y;
		node->dat.leaf->r.xs = xs;
		node->dat.leaf->r.ys = ys;
		SetWindowPos(node->dat.leaf->hwnd, HWND_TOP, x, y, xs, ys, 0);
		InvalidateRgn(node->dat.leaf->hwnd, NULL, FALSE);
		UpdateWindow(node->dat.leaf->hwnd);
		return;
	}

	double p, pn;

	switch (node->dat.node.dir) {
	case dtd_horz:
		for (p = x + 0.5, node = node->dat.node.child; node != NULL; node = node->sibling) {
			pn = p + xs * node->size;
			dtree_set_pos(node, (int)p, y, (int)pn - (int)p, ys);
			p = pn;
		}
		break;
	case dtd_vert:
		for (p = y + 0.5, node = node->dat.node.child; node != NULL; node = node->sibling) {
			pn = p + ys * node->size;
			dtree_set_pos(node, x, (int)p, xs, (int)pn - (int)p);
			p = pn;
		}
		break;
	}
}

void dtree_set_dir(DT_NODE *node, int dir) {
	if (node->tag != dtt_node) return;
	node->dat.node.dir = dir;
}

void dtree_set_component(DT_NODE *node, DT_COMPONENT *comp) {
	if (node->tag == dtt_leaf) node->dat.leaf = comp;
}

void dtree_drag_edge(DT_NODE *node, int dir, int pos) {
	if (node->sibling == NULL) return;

	double s = node->size + node->sibling->size;
	int d;

	switch (dir) {
	case dtd_horz:
		pos = max(pos, node->r.x + DEBUG_DRAG_WIDTH * 4);
		pos = min(pos, node->r.x + node->r.xs + node->sibling->r.xs - DEBUG_DRAG_WIDTH * 4);
		d = node->r.xs + node->sibling->r.xs;
		node->size = (pos - node->r.x) * s / d;
		node->sibling->size = s - node->size;
		dtree_set_pos(node, node->r.x, node->r.y, pos - node->r.x, node->r.ys);
		dtree_set_pos(node->sibling, pos, node->r.y, d - node->r.xs, node->r.ys);
		break;
	case dtd_vert:
		pos = max(pos, node->r.y + DEBUG_DRAG_WIDTH * 4);
		pos = min(pos, node->r.y + node->r.ys + node->sibling->r.ys - DEBUG_DRAG_WIDTH * 4);
		d = node->r.ys + node->sibling->r.ys;
		node->size = (pos - node->r.y) * s / d;
		node->sibling->size = s - node->size;
		dtree_set_pos(node, node->r.x, node->r.y, node->r.xs, pos - node->r.y);
		dtree_set_pos(node->sibling, node->r.x, pos, node->r.xs, d - node->r.ys);
		break;
	}
}

DT_COMPONENT *dtree_get_component(DT_NODE *node) {
	if (node->tag == dtt_leaf)
		return node->dat.leaf;
	else
		return NULL;
}

int dtree_num_children(DT_NODE *node) {
	if (node->tag != dtt_node) return 0;

	int i;

	for (i = 0, node = node->dat.node.child; node != NULL; node = node->sibling, i++);
	return i;
}

int dtree_pointed(DT_NODE *node, int x, int y, DT_NODE **fnode) {
	*fnode = node;
	if (node->tag == dtt_leaf || node->dat.node.child == NULL) return dtd_none;

	int oxs = node->r.xs;
	int oys = node->r.ys;
	double p, pn;

	switch (node->dat.node.dir) {
	case dtd_horz:
		for (p = node->r.x, node = node->dat.node.child;
			node != NULL; node = node->sibling) {
			pn = p + oxs * node->size;
			if (pn - DEBUG_DRAG_WIDTH <= x && x < pn + DEBUG_DRAG_WIDTH) {
				if (node->sibling != NULL) {
					*fnode = node;
					return dtd_horz;
				} else return dtd_none;
			}
			if (p + DEBUG_DRAG_WIDTH <= x && x < pn - DEBUG_DRAG_WIDTH)
				return dtree_pointed(node, x, y, fnode);
			p = pn;
		}
		break;
	case dtd_vert:
		for (p = node->r.y, node = node->dat.node.child;
			node != NULL; node = node->sibling) {
			pn = p + oys * node->size;
			if (pn - DEBUG_DRAG_WIDTH <= y && y < pn + DEBUG_DRAG_WIDTH) {
				if (node->sibling != NULL) {
					*fnode = node;
					return dtd_vert;
				} else return dtd_none;
			}
			if (p + DEBUG_DRAG_WIDTH <= y && y < pn - DEBUG_DRAG_WIDTH)
				return dtree_pointed(node, x, y, fnode);
			p = pn;
		}
		break;
	}

	return dtd_none;
}

void dtree_normalise(DT_NODE *node) {
	if (node->tag == dtt_leaf) return;

	double sum;
	DT_NODE *fch = node->dat.node.child;

	for (sum = 0.0, node = fch; node != NULL; node = node->sibling) sum += node->size;
	if (sum > 0.0) for (node = fch; node != NULL; node = node->sibling) node->size /= sum;
	for (node = fch; node != NULL; node = node->sibling) dtree_normalise(node);
}

void dtree_show(DT_NODE *node, int show) {
	if (node->tag == dtt_leaf) {
		ShowWindow(node->dat.leaf->hwnd, show);
		return;
	}

	for (node = node->dat.node.child; node != NULL; node = node->sibling)
		dtree_show(node, show);
}

void dtree_draw_leaves(DT_NODE *node, RECT *rect) {
	if (node->tag == dtt_leaf) {
		debug_draw_child(node->dat.leaf->hwnd, rect);
		return;
	}

	for (node = node->dat.node.child; node != NULL; node = node->sibling)
		dtree_draw_leaves(node, rect);
}

void dtree_broadcast(DT_NODE *node, UINT msg, WPARAM wp, LPARAM lp) {
	if (node->tag == dtt_leaf) {
		SendMessage(node->dat.leaf->hwnd, msg, wp, lp);
		return;
	}

	for (node = node->dat.node.child; node != NULL; node = node->sibling)
		dtree_broadcast(node, msg, wp, lp);
}

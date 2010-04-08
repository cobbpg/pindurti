#define DEBUG_DRAG_WIDTH 6

#define DEBUG_COMP_KEYVAL 0
#define DEBUG_COMP_DISASM 1
#define DEBUG_COMP_Z80 2
#define DEBUG_COMP_LCDDATA 3
#define DEBUG_COMP_MEMHEX 4
#define DEBUG_COMP_VAT 5
#define DEBUG_COMP_LINKHISTORY 6
#define DEBUG_COMP_LOGGER 7

typedef int (*WINDOW_PROC)(HWND, UINT, WPARAM, LPARAM);

// Debug tree node types
enum {
	dtt_node,
	dtt_leaf
};

// Debug frame directions
enum {
	dtd_none,
	dtd_horz,
	dtd_vert
};

typedef struct {
	int x, y, xs, ys;
} DT_RECT;

typedef struct {
	char class_name[100];
	WINDOW_PROC win_proc;
} DT_COMPONENT_CLASS;

typedef struct {
	char name[100];
	int type;
	HWND hwnd;
	HMENU menu;
	WINDOW_PROC win_proc;
	DT_RECT r;
	void *state;
} DT_COMPONENT;

typedef struct T_DT_NODE {
	int tag;
	double size;
	DT_RECT r;
	struct T_DT_NODE *sibling;
	union {
		struct {
			int dir;
			struct T_DT_NODE *child;
		} node;
		DT_COMPONENT *leaf;
	} dat;
} DT_NODE;

DT_NODE *dtree_new_node(int type, double size);
DT_NODE *dtree_new_frame(int dir, double size);
DT_NODE *dtree_new_component(int type, int slot, double size);
void dtree_delete(DT_NODE *node);
void dtree_delete_from(DT_NODE *par, DT_NODE *node);

void dtree_add_child(DT_NODE *node, DT_NODE *child, int pos);
void dtree_set_pos(DT_NODE *node, int x, int y, int xs, int ys);
void dtree_set_dir(DT_NODE *node, int dir);
void dtree_set_component(DT_NODE *node, DT_COMPONENT *comp);
void dtree_drag_edge(DT_NODE *node, int dir, int pos);

DT_COMPONENT *dtree_get_component(DT_NODE *node);
int dtree_num_children(DT_NODE *node);
int dtree_pointed(DT_NODE *node, int x, int y, DT_NODE **fnode);

void dtree_normalise(DT_NODE *node);
void dtree_show(DT_NODE *node, int show);
void dtree_draw_leaves(DT_NODE *node, RECT *rect);
void dtree_broadcast(DT_NODE *node, UINT msg, WPARAM wp, LPARAM lp);

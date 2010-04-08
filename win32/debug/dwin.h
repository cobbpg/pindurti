#include "../../hw/hwcore.h"
#include "../../hw/files.h"
#include "../../debug/debug.h"
#include "../../debug/dz80.h"
#include "../../debug/dlcd.h"
#include "../../debug/dti82.h"
#include "../../debug/dti83.h"
#include "../../debug/dti83p.h"
#include "common.h"
#include "cwin.h"
#include "ckeyval.h"
#include "cdisasm.h"
#include "cz80.h"
#include "clcddata.h"
#include "cmemhex.h"
#include "cvat.h"
#include "clinkhis.h"
#include "clogger.h"
#include "kvdata.h"
#include "dtree.h"

#define DEBUG_REFRESH 1
#define DEBUG_SLOT 2
#define DEBUG_RESLOT 3
#define DEBUG_STARTDRAG 4
#define DEBUG_EDITED 5
#define DEBUG_NOEDIT 6
#define DEBUG_VARSELECT 7

#define DEBUG_CMD_STEP 1000
#define DEBUG_CMD_EDIT_OK 1001
#define DEBUG_CMD_EDIT_CANCEL 1002
#define DEBUG_CMD_CODEBP 1003
#define DEBUG_CMD_LAYOUT_EDITOR 1004
#define DEBUG_CMD_STEP_OVER 1005
#define DEBUG_CMD_FONT_BIG 2000
#define DEBUG_CMD_FONT_SMALL 2001

#define DEBUG_CMD_COMPONENT 10000

typedef struct {
	HFONT sans, mono;
	int width, height;
	double mag;
} DEBUG_FONT_DATA;

extern DT_COMPONENT *cdat;
extern DT_NODE *dt_root;
extern HWND debug_window, debug_parent, edit_box, debug_blocker;
extern int debug_shown;
extern DEBUG_FONT_DATA font;
extern HBRUSH bkg_col[6];
extern HCURSOR cur_arr, cur_horz, cur_vert, cur_mod;

LRESULT CALLBACK debug_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void debug_draw_child(HWND hwnd, RECT *rect);

int register_window_class(const char *cname, WNDPROC win_proc);
void register_debug_window_classes();
void update_component();
int init_debug_window();
int show_debug_window(int state);
int run_debug_window();

void *get_window_data(HWND hwnd);
void set_window_data(HWND hwnd, void *ptr);


#include "../common.h"

#define GIF_FRAME_MAX (120 * 64 * 4)

#define GIF_IDLE 0
#define GIF_START 1
#define GIF_FRAME 2
#define GIF_END 3

extern int gif_write_state;
extern int gif_file_size;
extern char gif_file_name[256];
extern WORD gif_base_delay;
extern int gif_xs;
extern int gif_ys;
extern BYTE gif_frame[GIF_FRAME_MAX];
extern int gif_time;
extern int gif_newframe;

void gif_writer();


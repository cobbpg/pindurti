#include "emucore.h"

#define R_A z80->af.b.h
#define R_F z80->af.b.l
#define R_B z80->bc.b.h
#define R_C z80->bc.b.l
#define R_D z80->de.b.h
#define R_E z80->de.b.l
#define R_H z80->hl.b.h
#define R_L z80->hl.b.l
#define R_IXH z80->ix.b.h
#define R_IXL z80->ix.b.l
#define R_IYH z80->iy.b.h
#define R_IYL z80->iy.b.l
#define R_AF z80->af.w
#define R_BC z80->bc.w
#define R_DE z80->de.w
#define R_HL z80->hl.w
#define R_IX z80->ix.w
#define R_IY z80->iy.w
#define R_SP z80->sp.w
#define R_PC z80->pc
#define R_R z80->r
#define R_I z80->i
#define R_AFS z80->afs
#define R_BCS z80->bcs
#define R_DES z80->des
#define R_HLS z80->hls

#define F_S 0x80
#define F_Z 0x40
#define F_5 0x20
#define F_H 0x10
#define F_3 0x08
#define F_P 0x04
#define F_N 0x02
#define F_C 0x01
#define F_NS 0x7f
#define F_NZ 0xbf
#define F_N5 0xdf
#define F_NH 0xef
#define F_N3 0xf7
#define F_NP 0xfb
#define F_NN 0xfd
#define F_NC 0xfe

#ifdef __BIG_ENDIAN

typedef union {
	WORD w;
	struct {
		BYTE h;
		BYTE l;
	} b;
} Z80_WORD;

#else

typedef union {
	WORD w;
	struct {
		BYTE l;
		BYTE h;
	} b;
} Z80_WORD;

#endif

typedef struct {
	Z80_WORD af, bc, de, hl, ix, iy, sp;
	WORD pc, afs, bcs, des, hls, idx;
	BYTE r, i, im, hlt, iff1, iff2, pref, ie;
} Z80_CPU;

typedef void (*Z80_WRITE_FN)(WORD adr, BYTE val);
typedef BYTE (*Z80_READ_FN)(WORD adr);
typedef void (*Z80_OUT_FN)(BYTE port, BYTE val);
typedef BYTE (*Z80_IN_FN)(BYTE port);

extern Z80_CPU *z80; // Running CPU
extern Z80_WRITE_FN z80_write; // Z80 memory write slot
extern Z80_READ_FN z80_read; // Z80 memory read slot
extern Z80_OUT_FN z80_out; // Z80 output slot
extern Z80_IN_FN z80_in; // Z80 input slot
extern Z80_READ_FN z80_acc; // Z80 memory access slot (read without side effects)

void z80_step(); // Executing one instruction
void z80_interrupt(); // Forcing an interrupt
void z80_prepare(); // Calculating instruction LUTs
void z80_reset(); // Power on defaults
int z80_save(FILE *fp); // Save the current CPU state
int z80_load(FILE *fp); // Load the current CPU state


// no nmi support; no proper im 0 either
// 16-bit port addresses not implemented
// redundant luts -> better indexing recommended
// (not a big problem, since the total memory footprint is still under 3 megs)
// todo: z80->idx behaviour (zxdocs/memptr)

#include <stdio.h>
#include <stdlib.h>
#include "z80.h"
#include "../debug/dtrap.h"

#define IF_NZ if (!(R_F & F_Z))
#define IF_Z if (R_F & F_Z)
#define IF_NC if (!(R_F & F_C))
#define IF_C if (R_F & F_C)
#define IF_PO if (!(R_F & F_P))
#define IF_PE if (R_F & F_P)
#define IF_P if (!(R_F & F_S))
#define IF_M if (R_F & F_S)

#define S_NONE 0
#define S_DD 1
#define S_FD 2
#define S_ED 3
#define S_CB 4
#define S_DDCB 5
#define S_FDCB 6

#define READ_MEM16(adr, var) var = z80_read(adr) + (z80_read((adr) + 1) << 8)
#define WRITE_MEM8(adr, val) z80_write(adr, val)
#define WRITE_MEM16(adr, val) z80_write(adr, val); z80_write((adr) + 1, (val) >> 8)
#define FETCH_VAL8(var) var = z80_read(R_PC++)
#define FETCH_VAL16(var) var = z80_read(R_PC) + (z80_read(R_PC + 1) << 8); R_PC += 2
#define FETCH_IDX8(var) WORD idx = ((int8_t)z80_read(R_PC++)) + var
#define READ_IO8(var) var = z80_in(R_C); R_F = in_f[(var << 8) + R_F]
#define WRITE_IO8(var) z80_out(R_C, var)

#define POP_REG16(var) READ_MEM16(R_SP, var); R_SP += 2
#define PUSH_REG16(var) R_SP -= 2; WRITE_MEM16(R_SP, var)

#define INC_REG8(var) var++; R_F = inc_f[(var << 8) + R_F]
#define DEC_REG8(var) var--; R_F = dec_f[(var << 8) + R_F]
#define ADD_REG8(var) R_AF = adc_lut[(R_A << 8) + var]
#define ADC_REG8(var) R_AF = adc_lut[((R_F & F_C) << 16) + (R_A << 8) + var]
#define SUB_REG8(var) R_AF = sbc_lut[(R_A << 8) + var]
#define SBC_REG8(var) R_AF = sbc_lut[((R_F & F_C) << 16) + (R_A << 8) + var]
#define AND_REG8(var) R_A &= var; R_F = log_f[R_A] | F_H
#define OR_REG8(var) R_A |= var; R_F = log_f[R_A]
#define XOR_REG8(var) R_A ^= var; R_F = log_f[R_A]
#define CP_REG8(var) R_F = (sbc_lut[(R_A << 8) + var] & F_N5 & F_N3) | (var & (F_5 | F_3))

#define RLC_REG8(var) WORD tmp = rlc_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define RRC_REG8(var) WORD tmp = rrc_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define RL_REG8(var) WORD tmp = rl_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define RR_REG8(var) WORD tmp = rr_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define SLA_REG8(var) WORD tmp = sla_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define SRA_REG8(var) WORD tmp = sra_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define SLL_REG8(var) WORD tmp = sll_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8
#define SRL_REG8(var) WORD tmp = srl_lut[(var << 8) + R_F]; R_F = tmp; var = tmp >> 8

#define INC_MEM8(adr) BYTE tmp = z80_read(adr) + 1; R_F = inc_f[(tmp << 8) + R_F]; z80_write(adr, tmp)
#define DEC_MEM8(adr) BYTE tmp = z80_read(adr) - 1; R_F = dec_f[(tmp << 8) + R_F]; z80_write(adr, tmp)

#define ADD_REG16(pre1, pre2) \
	WORD tmp = adc_lut[(((pre1.b.l + pre2.b.l) & 0x100) << 8) + (pre1.b.h << 8) + pre2.b.h]; \
	pre1.w += pre2.w; R_F = (R_F & (F_S | F_Z | F_P)) | (tmp & (F_5 | F_H | F_3 | F_C))
#define ADC_REG16(pre1, pre2) \
	WORD tmp = adc_lut[(((pre1.b.l + pre2.b.l + (R_F & F_C)) & 0x100) << 8) + (pre1.b.h << 8) + pre2.b.h]; \
	pre1.w += pre2.w + (R_F & F_C); R_F = (tmp & F_NZ) | (F_Z * (pre1.w == 0))
#define SBC_REG16(pre1, pre2) \
	WORD tmp = sbc_lut[((pre1.b.l < pre2.b.l + (R_F & F_C)) << 16) + (pre1.b.h << 8) + pre2.b.h]; \
	pre1.w -= pre2.w + (R_F & F_C); R_F = (tmp & F_NZ) | (F_Z * (pre1.w == 0))

#define RLD \
	WORD tmp = (z80_read(R_HL) << 4) + (R_A & 0x0f); \
	WRITE_MEM8(R_HL, tmp); R_A = (R_A & 0xf0) | ((tmp >> 8) & 0x0f); \
	R_F = (log_f[R_A] & F_NC) | (R_F & F_C)
#define RRD \
	BYTE tdt = z80_read(R_HL); \
	WORD tmp = (tdt >> 4) + ((R_A & 0x0f) << 4) + (tdt << 8); \
	WRITE_MEM8(R_HL, tmp); R_A = (R_A & 0xf0) | ((tmp >> 8) & 0x0f); \
	R_F = (log_f[R_A] & F_NC) | (R_F & F_C)

#define LDI \
	BYTE val = z80_read(R_HL); WRITE_MEM8(R_DE, val); R_BC--; R_DE++; R_HL++; \
	val += R_A; R_F = (R_F & (F_S | F_Z | F_C)) | (F_P * (R_BC != 0)) | ((val & 2) << 4) | (val & F_3)
#define LDD \
	BYTE val = z80_read(R_HL); WRITE_MEM8(R_DE, val); R_BC--; R_DE--; R_HL--; \
	val += R_A; R_F = (R_F & (F_S | F_Z | F_C)) | (F_P * (R_BC != 0)) | ((val & 2) << 4) | (val & F_3)
#define CPI \
	R_BC--; \
	BYTE tmp = z80_read(R_HL); \
	R_F = (R_F & F_C) | (sbc_lut[(R_A << 8) + tmp] & (F_S | F_Z | F_H | F_N)) | (F_P * (R_BC != 0)); \
	tmp = R_A - tmp - ((R_F & F_H) >> 4); \
	R_F |= ((tmp & 2) << 4) | (tmp & F_3); R_HL++
#define CPD \
	R_BC--; \
	BYTE tmp = z80_read(R_HL); \
	R_F = (R_F & F_C) | (sbc_lut[(R_A << 8) + tmp] & (F_S | F_Z | F_H | F_N)) | (F_P * (R_BC != 0)); \
	tmp = R_A - tmp - ((R_F & F_H) >> 4); \
	R_F |= ((tmp & 2) << 4) | (tmp & F_3); R_HL--
#define INI \
	BYTE var = z80_in(R_C); R_B--; \
	R_F = (dec_f[(R_B << 8) + R_F] & (F_S | F_Z | F_5 | F_3)) | (F_N * (var >> 7)) | \
		((F_H | F_C) * (var + (BYTE)(R_C + 1) > 255)) | par[((var + (BYTE)(R_C + 1)) & 7) ^ R_B]; \
	WRITE_MEM8(R_HL, var); R_HL++
#define IND \
	BYTE var = z80_in(R_C); R_B--; \
	R_F = (dec_f[(R_B << 8) + R_F] & (F_S | F_Z | F_5 | F_3)) | (F_N * (var >> 7)) | \
		((F_H | F_C) * (var + (BYTE)(R_C - 1) > 255)) | par[((var + (BYTE)(R_C - 1)) & 7) ^ R_B]; \
	WRITE_MEM8(R_HL, var); R_HL--
#define OUTI \
	BYTE var = z80_read(R_HL); R_B--; R_HL++; \
	R_F = (dec_f[(R_B << 8) + R_F] & (F_S | F_Z | F_5 | F_3)) | (F_N * (var >> 7)) | \
		((F_H | F_C) * (var + R_L > 255)) | par[((var + R_L) & 7) ^ R_B]; \
	z80_out(R_C, var)
#define OUTD \
	BYTE var = z80_read(R_HL); R_B--; R_HL--; \
	R_F = (dec_f[(R_B << 8) + R_F] & (F_S | F_Z | F_5 | F_3)) | (F_N * (var >> 7)) | \
		((F_H | F_C) * (var + R_L > 255)) | par[((var + R_L) & 7) ^ R_B]; \
	z80_out(R_C, var)

#define BIT_OP8(var, msk) R_F = log_f[var & msk] | F_H | (R_F & F_C)
#define SET_OP8(var, msk) var |= msk
#define RES_OP8(var, msk) var &= msk
#define SET_MEM8(adr, msk) BYTE tmp = z80_read(adr) | msk; WRITE_MEM8(adr, tmp)
#define RES_MEM8(adr, msk) BYTE tmp = z80_read(adr) & msk; WRITE_MEM8(adr, tmp)
#define BITM_OP8(dummy, msk) R_F = (log_f[z80_read(R_HL) & msk] & F_N5 & F_N3) | F_H | (R_F & F_C) | ((z80->idx >> 8) & (F_5 | F_3))
#define BITI_OP8(dummy, msk) R_F = (log_f[z80_read(z80->idx) & msk] & F_N5 & F_N3) | F_H | (R_F & F_C) | ((z80->idx >> 8) & (F_5 | F_3))
#define SETI_OP8(var, msk) var = z80_read(z80->idx) | msk; WRITE_MEM8(z80->idx, var)
#define RESI_OP8(var, msk) var = z80_read(z80->idx) & msk; WRITE_MEM8(z80->idx, var)

#define SWAP_REG16(var1, var2) tmp = var1; var1 = var2; var2 = tmp

#define FCT_REG8_1(fname,c1) \
	static inline void fname ## _a() { c1 (R_A); } \
	static inline void fname ## _b() { c1 (R_B); } \
	static inline void fname ## _c() { c1 (R_C); } \
	static inline void fname ## _d() { c1 (R_D); } \
	static inline void fname ## _e() { c1 (R_E); } \
	static inline void fname ## _h() { c1 (R_H); } \
	static inline void fname ## _l() { c1 (R_L); } \
	static inline void fname ## _ixh() { c1 (R_IXH); } \
	static inline void fname ## _ixl() { c1 (R_IXL); } \
	static inline void fname ## _iyh() { c1 (R_IYH); } \
	static inline void fname ## _iyl() { c1 (R_IYL); }

#define FCT_OP8_1(fname,c1) \
	FCT_REG8_1(fname,c1) \
	static inline void fname ## _m() { c1 (z80_read(R_HL)); } \
	static inline void fname ## _x() { FETCH_IDX8(R_IX); c1 (z80_read(idx)); } \
	static inline void fname ## _y() { FETCH_IDX8(R_IY); c1 (z80_read(idx)); }

#define FCT_REG8_2(fbeg,fend,c1) \
	static inline void fbeg ## _a_ ## fend() { c1 (R_A); } \
	static inline void fbeg ## _b_ ## fend() { c1 (R_B); } \
	static inline void fbeg ## _c_ ## fend() { c1 (R_C); } \
	static inline void fbeg ## _d_ ## fend() { c1 (R_D); } \
	static inline void fbeg ## _e_ ## fend() { c1 (R_E); } \
	static inline void fbeg ## _h_ ## fend() { c1 (R_H); } \
	static inline void fbeg ## _l_ ## fend() { c1 (R_L); } \
	static inline void fbeg ## _ixh_ ## fend() { c1 (R_IXH); } \
	static inline void fbeg ## _ixl_ ## fend() { c1 (R_IXL); } \
	static inline void fbeg ## _iyh_ ## fend() { c1 (R_IYH); } \
	static inline void fbeg ## _iyl_ ## fend() { c1 (R_IYL); }

#define FCT_COND(fname,fct,ccinc,pcinc) \
	static inline void fname()        { fct       } \
	static inline void fname ## _nz() { IF_NZ { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _z()  { IF_Z  { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _nc() { IF_NC { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _c()  { IF_C  { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _po() { IF_PO { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _pe() { IF_PE { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _p()  { IF_P  { fct z80cctmp += ccinc; } else R_PC += pcinc; } \
	static inline void fname ## _m()  { IF_M  { fct z80cctmp += ccinc; } else R_PC += pcinc; } \

#define FCT_NUM_1(fbeg,fend,mac,par) \
	static inline void fbeg ## _0_ ## fend() { mac (par, 0x01); } \
	static inline void fbeg ## _1_ ## fend() { mac (par, 0x02); } \
	static inline void fbeg ## _2_ ## fend() { mac (par, 0x04); } \
	static inline void fbeg ## _3_ ## fend() { mac (par, 0x08); } \
	static inline void fbeg ## _4_ ## fend() { mac (par, 0x10); } \
	static inline void fbeg ## _5_ ## fend() { mac (par, 0x20); } \
	static inline void fbeg ## _6_ ## fend() { mac (par, 0x40); } \
	static inline void fbeg ## _7_ ## fend() { mac (par, 0x80); }

#define FCT_NUM_2(fbeg,fend,mac,par) \
	static inline void fbeg ## _0_ ## fend() { mac (par, 0xfe); } \
	static inline void fbeg ## _1_ ## fend() { mac (par, 0xfd); } \
	static inline void fbeg ## _2_ ## fend() { mac (par, 0xfb); } \
	static inline void fbeg ## _3_ ## fend() { mac (par, 0xf7); } \
	static inline void fbeg ## _4_ ## fend() { mac (par, 0xef); } \
	static inline void fbeg ## _5_ ## fend() { mac (par, 0xdf); } \
	static inline void fbeg ## _6_ ## fend() { mac (par, 0xbf); } \
	static inline void fbeg ## _7_ ## fend() { mac (par, 0x7f); }

#define FCT_XDCB(ins,xlet,xreg) \
	static inline void ins ## _ ## xlet ## _b() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_B = tmp >> 8; WRITE_MEM8(z80->idx, R_B); } \
	static inline void ins ## _ ## xlet ## _c() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_C = tmp >> 8; WRITE_MEM8(z80->idx, R_C); } \
	static inline void ins ## _ ## xlet ## _d() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_D = tmp >> 8; WRITE_MEM8(z80->idx, R_D); } \
	static inline void ins ## _ ## xlet ## _e() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_E = tmp >> 8; WRITE_MEM8(z80->idx, R_E); } \
	static inline void ins ## _ ## xlet ## _h() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_H = tmp >> 8; WRITE_MEM8(z80->idx, R_H); } \
	static inline void ins ## _ ## xlet ## _l() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_L = tmp >> 8; WRITE_MEM8(z80->idx, R_L); } \
	static inline void ins ## _ ## xlet ## _a() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		R_A = tmp >> 8; WRITE_MEM8(z80->idx, R_A); } \
	static inline void ins ## _ ## xlet() { \
		WORD tmp = ins ## _lut[(z80_read(z80->idx) << 8) + R_F]; R_F = tmp; \
		WRITE_MEM8(z80->idx, (tmp >> 8)); }

#define ITB(opcode, timing, inscall) case opcode: z80cctmp = timing; inscall(); break
#define ICNT(val) do { \
		int tmpitc; \
		for (tmpitc = 0; tmpitc < emu->it_num; tmpitc++) emu->it_cnt[tmpitc] += val; \
		emu->stop_cnt += val; \
	} while (0)

int z80cctmp; // Temporary clock cycle placeholder

Z80_CPU *z80;
Z80_WRITE_FN z80_write;
Z80_READ_FN z80_read;
Z80_OUT_FN z80_out;
Z80_IN_FN z80_in;
Z80_READ_FN z80_acc;

BYTE inc_f[0x10000];
BYTE dec_f[0x10000];
BYTE log_f[0x100];
BYTE inc_r[0x100];
BYTE in_f[0x10000];
BYTE par[0x100];
WORD rlca_lut[0x10000];
WORD rrca_lut[0x10000];
WORD rla_lut[0x10000];
WORD rra_lut[0x10000];
WORD daa_lut[0x10000];
WORD cpl_lut[0x10000];
WORD scf_lut[0x10000];
WORD ccf_lut[0x10000];
WORD adc_lut[0x20000];
WORD sbc_lut[0x20000];
WORD rlc_lut[0x10000];
WORD rrc_lut[0x10000];
WORD rl_lut[0x10000];
WORD rr_lut[0x10000];
WORD sla_lut[0x10000];
WORD sra_lut[0x10000];
WORD sll_lut[0x10000];
WORD srl_lut[0x10000];

// Prefix pseudo instructions
static inline void pref_cb() { z80->pref = S_CB; }
static inline void pref_dd() { z80->pref = S_DD; }
static inline void pref_ed() { z80->pref = S_ED; }
static inline void pref_fd() { z80->pref = S_FD; }
static inline void pref_ddcb() { z80->pref = S_DDCB; }
static inline void pref_fdcb() { z80->pref = S_FDCB; }

// Debug trap pseudo instructions
static inline void trap() { execute_trap(z80_acc(R_PC - 1)); }

FCT_REG8_1(inc, INC_REG8) // INC reg8; inc_*()
FCT_REG8_1(dec, DEC_REG8) // DEC reg8; dec_*()
FCT_REG8_2(ld, n, FETCH_VAL8) // LD reg8,n; ld_*_n()
FCT_OP8_1(ld_b, R_B =) // LD B,op8; ld_b_*()
FCT_OP8_1(ld_c, R_C =) // LD C,op8; ld_c_*()
FCT_OP8_1(ld_d, R_D =) // LD D,op8; ld_d_*()
FCT_OP8_1(ld_e, R_E =) // LD E,op8; ld_e_*()
FCT_OP8_1(ld_h, R_H =) // LD H,op8; ld_h_*()
FCT_OP8_1(ld_l, R_L =) // LD L,op8; ld_l_*()
FCT_OP8_1(ld_a, R_A =) // LD A,op8; ld_a_*()
FCT_OP8_1(ld_ixh, R_IXH =) // LD IXH,op8; ld_ixh_*()
FCT_OP8_1(ld_ixl, R_IXL =) // LD IXL,op8; ld_ixl_*()
FCT_OP8_1(ld_iyh, R_IYH =) // LD IYH,op8; ld_iyh_*()
FCT_OP8_1(ld_iyl, R_IYL =) // LD IYL,op8; ld_iyl_*()
FCT_OP8_1(add_a, ADD_REG8) // ADD A,op8; add_a_*()
FCT_OP8_1(adc_a, ADC_REG8) // ADC A,op8; adc_a_*()
FCT_OP8_1(sub, SUB_REG8) // SUB op8; sub_*()
FCT_OP8_1(sbc_a, SBC_REG8) // SBC A,op8; sbc_a_*()
FCT_OP8_1(and, AND_REG8) // AND op8; and_*()
FCT_OP8_1(xor, XOR_REG8) // XOR op8; xor_*()
FCT_OP8_1(or, OR_REG8) // OR op8; or_*()
FCT_OP8_1(cp, CP_REG8) // CP op8; cp_*()
FCT_COND(ret, POP_REG16(R_PC);, 6, 0) // RET aa; ret*()
FCT_COND(jp, READ_MEM16(R_PC, R_PC);, 0, 2) // JP aa; jp*()
FCT_COND(call, { PUSH_REG16(R_PC + 2); READ_MEM16(R_PC, R_PC); }, 7, 2) // CALL aa; call*()
FCT_REG8_1(rlc, RLC_REG8) // RLC reg8; rlc_*()
FCT_REG8_1(rrc, RRC_REG8) // RRC reg8; rrc_*()
FCT_REG8_1(rl, RL_REG8) // RL reg8; rl_*()
FCT_REG8_1(rr, RR_REG8) // RR reg8; rr_*()
FCT_REG8_1(sla, SLA_REG8) // SLA reg8; sla_*()
FCT_REG8_1(sra, SRA_REG8) // SRA reg8; sra_*()
FCT_REG8_1(sll, SLL_REG8) // SLL reg8; sll_*()
FCT_REG8_1(srl, SRL_REG8) // SRL reg8; srl_*()
FCT_NUM_1(bit,b,BIT_OP8,R_B) // BIT *,*; bit*()
FCT_NUM_1(bit,c,BIT_OP8,R_C)
FCT_NUM_1(bit,d,BIT_OP8,R_D)
FCT_NUM_1(bit,e,BIT_OP8,R_E)
FCT_NUM_1(bit,h,BIT_OP8,R_H)
FCT_NUM_1(bit,l,BIT_OP8,R_L)
FCT_NUM_1(bit,m,BITM_OP8,nothing)
FCT_NUM_1(bit,a,BIT_OP8,R_A)
FCT_NUM_1(bit,x,BITI_OP8,nothing)
FCT_NUM_1(bit,y,BITI_OP8,nothing)
FCT_NUM_1(set,b,SET_OP8,R_B) // SET *,*; set*()
FCT_NUM_1(set,c,SET_OP8,R_C)
FCT_NUM_1(set,d,SET_OP8,R_D)
FCT_NUM_1(set,e,SET_OP8,R_E)
FCT_NUM_1(set,h,SET_OP8,R_H)
FCT_NUM_1(set,l,SET_OP8,R_L)
FCT_NUM_1(set,m,SET_MEM8,R_HL)
FCT_NUM_1(set,a,SET_OP8,R_A)
FCT_NUM_1(set,x_b,SETI_OP8,R_B)
FCT_NUM_1(set,x_c,SETI_OP8,R_C)
FCT_NUM_1(set,x_d,SETI_OP8,R_D)
FCT_NUM_1(set,x_e,SETI_OP8,R_E)
FCT_NUM_1(set,x_h,SETI_OP8,R_H)
FCT_NUM_1(set,x_l,SETI_OP8,R_L)
FCT_NUM_1(set,x,SET_MEM8,z80->idx)
FCT_NUM_1(set,x_a,SETI_OP8,R_A)
FCT_NUM_1(set,y_b,SETI_OP8,R_B)
FCT_NUM_1(set,y_c,SETI_OP8,R_C)
FCT_NUM_1(set,y_d,SETI_OP8,R_D)
FCT_NUM_1(set,y_e,SETI_OP8,R_E)
FCT_NUM_1(set,y_h,SETI_OP8,R_H)
FCT_NUM_1(set,y_l,SETI_OP8,R_L)
FCT_NUM_1(set,y,SET_MEM8,z80->idx)
FCT_NUM_1(set,y_a,SETI_OP8,R_A)
FCT_NUM_2(res,b,RES_OP8,R_B) // RES *,*; res*()
FCT_NUM_2(res,c,RES_OP8,R_C)
FCT_NUM_2(res,d,RES_OP8,R_D)
FCT_NUM_2(res,e,RES_OP8,R_E)
FCT_NUM_2(res,h,RES_OP8,R_H)
FCT_NUM_2(res,l,RES_OP8,R_L)
FCT_NUM_2(res,m,RES_MEM8,R_HL)
FCT_NUM_2(res,a,RES_OP8,R_A)
FCT_NUM_2(res,x_b,RESI_OP8,R_B)
FCT_NUM_2(res,x_c,RESI_OP8,R_C)
FCT_NUM_2(res,x_d,RESI_OP8,R_D)
FCT_NUM_2(res,x_e,RESI_OP8,R_E)
FCT_NUM_2(res,x_h,RESI_OP8,R_H)
FCT_NUM_2(res,x_l,RESI_OP8,R_L)
FCT_NUM_2(res,x,RES_MEM8,z80->idx)
FCT_NUM_2(res,x_a,RESI_OP8,R_A)
FCT_NUM_2(res,y_b,RESI_OP8,R_B)
FCT_NUM_2(res,y_c,RESI_OP8,R_C)
FCT_NUM_2(res,y_d,RESI_OP8,R_D)
FCT_NUM_2(res,y_e,RESI_OP8,R_E)
FCT_NUM_2(res,y_h,RESI_OP8,R_H)
FCT_NUM_2(res,y_l,RESI_OP8,R_L)
FCT_NUM_2(res,y,RES_MEM8,z80->idx)
FCT_NUM_2(res,y_a,RESI_OP8,R_A)
FCT_XDCB(rlc,x,R_IX) // shift autocopy IX
FCT_XDCB(rrc,x,R_IX)
FCT_XDCB(rl,x,R_IX)
FCT_XDCB(rr,x,R_IX)
FCT_XDCB(sra,x,R_IX)
FCT_XDCB(sla,x,R_IX)
FCT_XDCB(sll,x,R_IX)
FCT_XDCB(srl,x,R_IX)
FCT_XDCB(rlc,y,R_IY) // shift autocopy IY
FCT_XDCB(rrc,y,R_IY)
FCT_XDCB(rl,y,R_IY)
FCT_XDCB(rr,y,R_IY)
FCT_XDCB(sra,y,R_IY)
FCT_XDCB(sla,y,R_IY)
FCT_XDCB(sll,y,R_IY)
FCT_XDCB(srl,y,R_IY)

// 00 - NOP
static inline void nop() {}
// 01 - LD BC,nn
static inline void ld_bc_nn() { FETCH_VAL16(R_BC); }
// 02 - LD (BC),A
static inline void ld_mb_a() { WRITE_MEM8(R_BC, R_A); }
// 03 - INC BC
static inline void inc_bc() { R_BC++; }
// 07 - RLCA
static inline void rlca() { R_AF = rlca_lut[R_AF]; }
// 08 - EX AF,AF'
static inline void ex_af_af() { WORD tmp; SWAP_REG16(R_AF, R_AFS); }
// 09 - ADD HL,BC -- --?-0*
static inline void add_hl_bc() { ADD_REG16(z80->hl, z80->bc); }
// 0A - LD A,(BC)
static inline void ld_a_mb() { R_A = z80_read(R_BC); }
// 0B - DEC BC
static inline void dec_bc() { R_BC--; }
// 0F - RRCA
static inline void rrca() { R_AF = rrca_lut[R_AF]; }
// 10 - DJNZ d
static inline void djnz() { int8_t tmp; FETCH_VAL8(tmp); R_B--; if (R_B != 0) { R_PC += tmp; z80cctmp += 5; } }
// 11 - LD DE,nn
static inline void ld_de_nn() { FETCH_VAL16(R_DE); }
// 12 - LD (DE),A
static inline void ld_md_a() { WRITE_MEM8(R_DE, R_A); }
// 13 - INC DE
static inline void inc_de() { R_DE++; }
// 17 - RLA
static inline void rla() { R_AF = rla_lut[R_AF]; }
// 18 - JR d
static inline void jr() { int8_t tmp; FETCH_VAL8(tmp); R_PC += tmp; }
// 19 - ADD HL,DE -- --?-0*
static inline void add_hl_de() { ADD_REG16(z80->hl, z80->de); }
// 1A - LD A,(DE)
static inline void ld_a_md() { R_A = z80_read(R_DE); }
// 1B - DEC DE
static inline void dec_de() { R_DE--; }
// 1F - RRA
static inline void rra() { R_AF = rra_lut[R_AF]; }
// 20 - JR NZ,d
static inline void jr_nz() { int8_t tmp; FETCH_VAL8(tmp); IF_NZ { R_PC += tmp; z80cctmp += 5; } }
// 21 - LD HL,nn
static inline void ld_hl_nn() { FETCH_VAL16(R_HL); }
// 22 - LD (aa),HL
static inline void ld_aa_hl() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM16(tmp, R_HL); }
// 23 - INC HL
static inline void inc_hl() { R_HL++; }
// 27 - DAA
static inline void daa() { R_AF = daa_lut[R_AF]; }
// 28 - JR Z,d
static inline void jr_z() { int8_t tmp; FETCH_VAL8(tmp); IF_Z { R_PC += tmp; z80cctmp += 5; } }
// 29 - ADD HL,HL -- --?-0*
static inline void add_hl_hl() { ADD_REG16(z80->hl, z80->hl); }
// 2A - LD HL,(aa)
static inline void ld_hl_aa() { WORD tmp; FETCH_VAL16(tmp); READ_MEM16(tmp, R_HL); }
// 2B - DEC HL
static inline void dec_hl() { R_HL--; }
// 2F - CPL
static inline void cpl() { R_AF = cpl_lut[R_AF]; }
// 30 - JR NC,d
static inline void jr_nc() { int8_t tmp; FETCH_VAL8(tmp); IF_NC { R_PC += tmp; z80cctmp += 5; } }
// 31 - LD SP,nn
static inline void ld_sp_nn() { FETCH_VAL16(R_SP); }
// 32 - LD (aa),A
static inline void ld_aa_a() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM8(tmp, R_A); }
// 33 - INC SP
static inline void inc_sp() { R_SP++; }
// 34 - INC (HL) -- ***V0-
static inline void inc_m() { INC_MEM8(R_HL); }
// 35 - DEC (HL) -- ***V1-
static inline void dec_m() { DEC_MEM8(R_HL); }
// 36 - LD (HL),n
static inline void ld_m_n() { BYTE tmp; FETCH_VAL8(tmp); WRITE_MEM8(R_HL, tmp); }
// 37 - SCF
static inline void scf() { R_AF = scf_lut[R_AF]; }
// 38 - JR C,d
static inline void jr_c() { int8_t tmp; FETCH_VAL8(tmp); IF_C { R_PC += tmp; z80cctmp += 5; } }
// 39 - ADD HL,SP -- --?-0*
static inline void add_hl_sp() { ADD_REG16(z80->hl, z80->sp); }
// 3A - LD A,(aa)
static inline void ld_a_aa() { WORD tmp; FETCH_VAL16(tmp); R_A = z80_read(tmp); }
// 3B - DEC SP
static inline void dec_sp() { R_SP--; }
// 3F - CCF
static inline void ccf() { R_AF = ccf_lut[R_AF]; }
// 70 - LD (HL),B
static inline void ld_m_b() { WRITE_MEM8(R_HL, R_B); }
// 71 - LD (HL),C
static inline void ld_m_c() { WRITE_MEM8(R_HL, R_C); }
// 72 - LD (HL),D
static inline void ld_m_d() { WRITE_MEM8(R_HL, R_D); }
// 73 - LD (HL),E
static inline void ld_m_e() { WRITE_MEM8(R_HL, R_E); }
// 74 - LD (HL),H
static inline void ld_m_h() { WRITE_MEM8(R_HL, R_H); }
// 75 - LD (HL),L
static inline void ld_m_l() { WRITE_MEM8(R_HL, R_L); }
// 76 - HALT
static inline void halt() { z80->hlt = 1; }
// 77 - LD (HL),A
static inline void ld_m_a() { WRITE_MEM8(R_HL, R_A); }
// C1 - POP BC
static inline void pop_bc() { POP_REG16(R_BC); }
// C5 - PUSH BC
static inline void push_bc() { PUSH_REG16(R_BC); }
// C6 - ADD A,n - ***V0*
static inline void add_a_n() { BYTE tmp; FETCH_VAL8(tmp); ADD_REG8(tmp); }
// C7 - RST $00
static inline void rst_00() { PUSH_REG16(R_PC); R_PC = 0x00; }
// CE - ADC A,n - ***V0*
static inline void adc_a_n() { BYTE tmp; FETCH_VAL8(tmp); ADC_REG8(tmp); }
// CF - RST $08
static inline void rst_08() { PUSH_REG16(R_PC); R_PC = 0x08; }
// D1 - POP DE
static inline void pop_de() { POP_REG16(R_DE); }
// D3 - OUT (n),A
static inline void out_n_a() { BYTE tmp; FETCH_VAL8(tmp); z80_out(tmp, R_A); }
// D5 - PUSH DE
static inline void push_de() { PUSH_REG16(R_DE); }
// D6 - SUB n - ***V1*
static inline void sub_n() { BYTE tmp; FETCH_VAL8(tmp); SUB_REG8(tmp); }
// D7 - RST $10
static inline void rst_10() { PUSH_REG16(R_PC); R_PC = 0x10; }
// D9 - EXX
static inline void exx() { WORD tmp; SWAP_REG16(R_BC, R_BCS); SWAP_REG16(R_DE, R_DES); SWAP_REG16(R_HL, R_HLS); }
// DB - IN A,(n)
static inline void in_a_n() { BYTE tmp; FETCH_VAL8(tmp); R_A = z80_in(tmp); }
// DE - SBC A,n - ***V1*
static inline void sbc_a_n() { BYTE tmp; FETCH_VAL8(tmp); SBC_REG8(tmp); }
// DF - RST $18
static inline void rst_18() { PUSH_REG16(R_PC); R_PC = 0x18; }
// E1 - POP HL
static inline void pop_hl() { POP_REG16(R_HL); }
// E3 - EX (SP),HL
static inline void ex_sp_hl() { WORD tmp; READ_MEM16(R_SP, tmp); WRITE_MEM16(R_SP, R_HL); R_HL = tmp; }
// E5 - PUSH HL
static inline void push_hl() { PUSH_REG16(R_HL); }
// E6 - AND n - **1P00
static inline void and_n() { BYTE tmp; FETCH_VAL8(tmp); AND_REG8(tmp); }
// E7 - RST $20
static inline void rst_20() { PUSH_REG16(R_PC); R_PC = 0x20; }
// E9 - JP (HL)
static inline void jp_hl() { R_PC = R_HL; }
// EB - EX DE,HL
static inline void ex_de_hl() { WORD tmp; SWAP_REG16(R_DE, R_HL); }
// EE - XOR n - **0P00
static inline void xor_n() { BYTE tmp; FETCH_VAL8(tmp); XOR_REG8(tmp); }
// EF - RST $28
static inline void rst_28() { PUSH_REG16(R_PC); R_PC = 0x28; }
// F1 - POP AF
static inline void pop_af() { POP_REG16(R_AF); }
// F3 - DI
static inline void di() { z80->iff1 = z80->iff2 = 0; }
// F5 - PUSH AF
static inline void push_af() { PUSH_REG16(R_AF); }
// F6 - OR n - **0P00
static inline void or_n() { BYTE tmp; FETCH_VAL8(tmp); OR_REG8(tmp); }
// F7 - RST $30
static inline void rst_30() { PUSH_REG16(R_PC); R_PC = 0x30; }
// F9 - LD SP,HL
static inline void ld_sp_hl() { R_SP = R_HL; }
// FB - EI
static inline void ei() { if (z80->iff1 == 0) z80->ie = 2; z80->iff1 = z80->iff2 = 1; }
// FE - CP n - ***V1*
static inline void cp_n() { BYTE tmp; FETCH_VAL8(tmp); CP_REG8(tmp); }
// FF - RST $38
static inline void rst_38() { PUSH_REG16(R_PC); R_PC = 0x38; }

// DD09 - ADD IX,BC -- --?-0*
static inline void add_ix_bc() { ADD_REG16(z80->ix, z80->bc); }
// DD19 - ADD IX,DE -- --?-0*
static inline void add_ix_de() { ADD_REG16(z80->ix, z80->de); }
// DD21 - LD IX,nn
static inline void ld_ix_nn() { FETCH_VAL16(R_IX); }
// DD22 - LD (aa),IX
static inline void ld_aa_ix() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM16(tmp, R_IX); }
// DD23 - INC IX
static inline void inc_ix() { R_IX++; }
// DD29 - ADD IX,IX -- --?-0*
static inline void add_ix_ix() { ADD_REG16(z80->ix, z80->ix); }
// DD2A - LD IX,(aa)
static inline void ld_ix_aa() { WORD tmp; FETCH_VAL16(tmp); READ_MEM16(tmp, R_IX); }
// DD2B - DEC IX
static inline void dec_ix() { R_IX--; }
// DD34 - INC (IX+d) -- ***V0-
static inline void inc_x() { FETCH_IDX8(R_IX); INC_MEM8(idx); }
// DD35 - DEC (IX+d) -- ***V1-
static inline void dec_x() { FETCH_IDX8(R_IX); DEC_MEM8(idx); }
// DD36 - LD (IX+d),n
static inline void ld_x_n() { BYTE tmp; FETCH_IDX8(R_IX); FETCH_VAL8(tmp);  WRITE_MEM8(idx, tmp); }
// DD39 - ADD IX,SP -- --?-0*
static inline void add_ix_sp() { ADD_REG16(z80->ix, z80->sp); }
// DD70 - LD (IX+d),B
static inline void ld_x_b() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_B); }
// DD71 - LD (IX+d),C
static inline void ld_x_c() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_C); }
// DD72 - LD (IX+d),D
static inline void ld_x_d() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_D); }
// DD73 - LD (IX+d),E
static inline void ld_x_e() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_E); }
// DD74 - LD (IX+d),H
static inline void ld_x_h() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_H); }
// DD75 - LD (IX+d),L
static inline void ld_x_l() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_L); }
// DD77 - LD (IX+d),A
static inline void ld_x_a() { FETCH_IDX8(R_IX); WRITE_MEM8(idx, R_A); }
// DDE1 - POP IX
static inline void pop_ix() { POP_REG16(R_IX); }
// DDE3 - EX (SP),IX
static inline void ex_sp_ix() { WORD tmp; READ_MEM16(R_SP, tmp); WRITE_MEM16(R_SP, R_IX); R_IX = tmp; }
// DDE5 - PUSH IX
static inline void push_ix() { PUSH_REG16(R_IX); }
// DDE9 - JP (IX)
static inline void jp_ix() { R_PC = R_IX; }
// DDF9 - LD SP,IX
static inline void ld_sp_ix() { R_SP = R_IX; }

// FD09 - ADD IY,BC -- --?-0*
static inline void add_iy_bc() { ADD_REG16(z80->iy, z80->bc); }
// FD19 - ADD IY,DE -- --?-0*
static inline void add_iy_de() { ADD_REG16(z80->iy, z80->de); }
// FD21 - LD IY,nn
static inline void ld_iy_nn() { FETCH_VAL16(R_IY); }
// FD22 - LD (aa),IY
static inline void ld_aa_iy() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM16(tmp, R_IY); }
// FD23 - INC IY
static inline void inc_iy() { R_IY++; }
// FD29 - ADD IY,IY -- --?-0*
static inline void add_iy_iy() { ADD_REG16(z80->iy, z80->iy); }
// FD2A - LD IY,(aa)
static inline void ld_iy_aa() { WORD tmp; FETCH_VAL16(tmp); READ_MEM16(tmp, R_IY); }
// FD2B - DEC IY
static inline void dec_iy() { R_IY--; }
// FD34 - INC (IY+d) -- ***V0-
static inline void inc_y() { FETCH_IDX8(R_IY); INC_MEM8(idx); }
// FD35 - DEC (IY+d) -- ***V1-
static inline void dec_y() { FETCH_IDX8(R_IY); DEC_MEM8(idx); }
// FD36 - LD (IY+d),n
static inline void ld_y_n() { BYTE tmp; FETCH_IDX8(R_IY); FETCH_VAL8(tmp); WRITE_MEM8(idx, tmp); }
// FD39 - ADD IY,SP -- --?-0*
static inline void add_iy_sp() { ADD_REG16(z80->iy, z80->sp); }
// FD70 - LD (IY+d),B
static inline void ld_y_b() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_B); }
// FD71 - LD (IY+d),C
static inline void ld_y_c() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_C); }
// FD72 - LD (IY+d),D
static inline void ld_y_d() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_D); }
// FD73 - LD (IY+d),E
static inline void ld_y_e() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_E); }
// FD74 - LD (IY+d),H
static inline void ld_y_h() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_H); }
// FD75 - LD (IY+d),L
static inline void ld_y_l() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_L); }
// FD77 - LD (IY+d),A
static inline void ld_y_a() { FETCH_IDX8(R_IY); WRITE_MEM8(idx, R_A); }
// FDE1 - POP IY
static inline void pop_iy() { POP_REG16(R_IY); }
// FDE3 - EX (SP),IY
static inline void ex_sp_iy() { WORD tmp; READ_MEM16(R_SP, tmp); WRITE_MEM16(R_SP, R_IY); R_IY = tmp; }
// FDE5 - PUSH IY
static inline void push_iy() { PUSH_REG16(R_IY); }
// FDE9 - JP (IY)
static inline void jp_iy() { R_PC = R_IY; }
// FDF9 - LD SP,IY
static inline void ld_sp_iy() { R_SP = R_IY; }

// ED40 - IN B,(C) - ***P0-
static inline void in_b_c() { READ_IO8(R_B); }
// ED41 - OUT (C),B
static inline void out_c_b() { WRITE_IO8(R_B); }
// ED42 - SBC HL,BC
static inline void sbc_hl_bc() { SBC_REG16(z80->hl, z80->bc); }
// ED43 - LD (aa),BC
static inline void ld_aa_bc() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM16(tmp, R_BC); }
// ED44 - NEG
static inline void neg() { R_AF = sbc_lut[R_A]; }
// ED45 - RETN
static inline void retn() { POP_REG16(R_PC); z80->iff1 = z80->iff2; }
// ED46 - IM 0
static inline void im_0() { z80->im = 0; }
// ED47 - LD I,A
static inline void ld_i_a() { R_I = R_A; }
// ED48 - IN C,(C) - ***P0-
static inline void in_c_c() { READ_IO8(R_C); }
// ED49 - OUT (C),C
static inline void out_c_c() { WRITE_IO8(R_C); }
// ED4A - ADC HL,BC
static inline void adc_hl_bc() { ADC_REG16(z80->hl, z80->bc); }
// ED4B - LD BC,(aa)
static inline void ld_bc_aa() { WORD tmp; FETCH_VAL16(tmp); READ_MEM16(tmp, R_BC); }
// ED4C - NEG
// ED4D - RETI
static inline void reti() { POP_REG16(R_PC); z80->iff1 = z80->iff2; }
// ED4E - IM 0
// ED4F - LD R,A
static inline void ld_r_a() { R_R = R_A; }
// ED50 - IN D,(C) - ***P0-
static inline void in_d_c() { READ_IO8(R_D); }
// ED51 - OUT (C),D
static inline void out_c_d() { WRITE_IO8(R_D); }
// ED52 - SBC HL,DE
static inline void sbc_hl_de() { SBC_REG16(z80->hl, z80->de); }
// ED53 - LD (aa),DE
static inline void ld_aa_de() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM16(tmp, R_DE); }
// ED54 - NEG
// ED55 - RETN
// ED56 - IM 1
static inline void im_1() { z80->im = 1; }
// ED57 - LD A,I
static inline void ld_a_i() { R_A = R_I; R_F = (R_A & (F_S | F_5 | F_3)) | (F_Z * (R_A == 0)) | (F_P * z80->iff2) | (R_F & F_C); }
// ED58 - IN E,(C) - ***P0-
static inline void in_e_c() { READ_IO8(R_E); }
// ED59 - OUT (C),E
static inline void out_c_e() { WRITE_IO8(R_E); }
// ED5A - ADC HL,DE
static inline void adc_hl_de() { ADC_REG16(z80->hl, z80->de); }
// ED5B - LD DE,(aa)
static inline void ld_de_aa() { WORD tmp; FETCH_VAL16(tmp); READ_MEM16(tmp, R_DE); }
// ED5C - NEG
// ED5D - RETI
// ED5E - IM 2
static inline void im_2() { z80->im = 2; }
// ED5F - LD A,R
static inline void ld_a_r() { R_A = R_R; R_F = (R_A & (F_S | F_5 | F_3)) | (F_Z * (R_A == 0)) | (F_P * z80->iff2) | (R_F & F_C); }
// ED60 - IN H,(C) - ***P0-
static inline void in_h_c() { READ_IO8(R_H); }
// ED61 - OUT (C),H
static inline void out_c_h() { WRITE_IO8(R_H); }
// ED62 - SBC HL,HL
static inline void sbc_hl_hl() { SBC_REG16(z80->hl, z80->hl); }
// ED63 - LD (aa),HL
// ED64 - NEG
// ED65 - RETN
// ED66 - IM 0
// ED67 - RRD
static inline void rrd() { RRD; }
// ED68 - IN L,(C) - ***P0-
static inline void in_l_c() { READ_IO8(R_L); }
// ED69 - OUT (C),L
static inline void out_c_l() { WRITE_IO8(R_L); }
// ED6A - ADC HL,HL
static inline void adc_hl_hl() { ADC_REG16(z80->hl, z80->hl); }
// ED6B - LD HL,(aa)
// ED6C - NEG
// ED6D - RETI
// ED6E - IM 0
// ED6F - RLD
static inline void rld() { RLD; }
// ED70 - IN F,(C) - ***P0-
static inline void in_f_c() { BYTE tmp; READ_IO8(tmp); }
// ED71 - OUT (C),0
static inline void out_c_0() { WRITE_IO8(0); }
// ED72 - SBC HL,SP
static inline void sbc_hl_sp() { SBC_REG16(z80->hl, z80->sp); }
// ED73 - LD (aa),SP
static inline void ld_aa_sp() { WORD tmp; FETCH_VAL16(tmp); WRITE_MEM16(tmp, R_SP); }
// ED74 - NEG
// ED75 - RETN
// ED76 - IM 1
// ED78 - IN A,(C) - ***P0-
static inline void in_a_c() { READ_IO8(R_A); }
// ED79 - OUT (C),A
static inline void out_c_a() { WRITE_IO8(R_A); }
// ED7A - ADC HL,SP
static inline void adc_hl_sp() { ADC_REG16(z80->hl, z80->sp); }
// ED7B - LD SP,(aa)
static inline void ld_sp_aa() { WORD tmp; FETCH_VAL16(tmp); READ_MEM16(tmp, R_SP); }
// ED7C - NEG
// ED7D - RETI
// ED7E - IM 2

// EDA0 - LDI
static inline void ldi() { LDI; }
// EDA8 - LDD
static inline void ldd() { LDD; }
// EDB0 - LDIR
static inline void ldir() { LDI; if (R_BC != 0) { R_PC -= 2; z80cctmp += 5; } }
// EDB8 - LDDR
static inline void lddr() { LDD; if (R_BC != 0) { R_PC -= 2; z80cctmp += 5; } }
// EDA1 - CPI
static inline void cpi() { CPI; }
// EDA9 - CPD
static inline void cpd() { CPD; }
// EDB1 - CPIR
static inline void cpir() { CPI; if ((R_BC != 0) & !(R_F & F_Z)) { R_PC -= 2; z80cctmp += 5; } }
// EDB9 - CPDR
static inline void cpdr() { CPD; if ((R_BC != 0) & !(R_F & F_Z)) { R_PC -= 2; z80cctmp += 5; } }
// EDA2 - INI
static inline void ini() { INI; }
// EDAA - IND
static inline void ind() { IND; }
// EDB2 - INIR
static inline void inir() { INI; if (R_B != 0) { R_PC -= 2; z80cctmp += 5; } }
// EDBA - INDR
static inline void indr() { IND; if (R_B != 0) { R_PC -= 2; z80cctmp += 5; } }
// EDA3 - OUTI
static inline void outi() { OUTI; }
// EDAB - OUTD
static inline void outd() { OUTD; }
// EDB3 - OTIR
static inline void otir() { OUTI; if (R_B != 0) { R_PC -= 2; z80cctmp += 5; } }
// EDBB - OTDR
static inline void otdr() { OUTD; if (R_B != 0) { R_PC -= 2; z80cctmp += 5; } }

// CB06 - RLC (HL)
static inline void rlc_m() { WORD tmp = rlc_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB0E - RRC (HL)
static inline void rrc_m() { WORD tmp = rrc_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB16 - RL (HL)
static inline void rl_m() { WORD tmp = rl_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB1E - RR (HL)
static inline void rr_m() { WORD tmp = rr_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB26 - SLA (HL)
static inline void sla_m() { WORD tmp = sla_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB2E - SRA (HL)
static inline void sra_m() { WORD tmp = sra_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB36 - SLL (HL)
static inline void sll_m() { WORD tmp = sll_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }
// CB3E - SRL (HL)
static inline void srl_m() { WORD tmp = srl_lut[(z80_read(R_HL) << 8) + R_F]; R_F = tmp; WRITE_MEM8(R_HL, (tmp >> 8)); }

// Executing one instruction
void z80_step() {
	BYTE nb, cpref;
	do {
		FETCH_VAL8(nb);
//		printf("%04X %02X\n", R_PC, nb);
		cpref = z80->pref;
		z80->pref = S_NONE;
		switch (cpref) {
			case S_NONE:
				R_R = inc_r[R_R];
				switch (nb) {
					ITB(0x00, 4,nop     ); ITB(0x01,10,ld_bc_nn ); ITB(0x02, 7,ld_mb_a ); ITB(0x03, 6,inc_bc  );
					ITB(0x04, 4,inc_b   ); ITB(0x05, 4,dec_b    ); ITB(0x06, 7,ld_b_n  ); ITB(0x07, 4,rlca    );
					ITB(0x08, 4,ex_af_af); ITB(0x09,11,add_hl_bc); ITB(0x0A, 7,ld_a_mb ); ITB(0x0B, 6,dec_bc  );
					ITB(0x0C, 4,inc_c   ); ITB(0x0D, 4,dec_c    ); ITB(0x0E, 7,ld_c_n  ); ITB(0x0F, 4,rrca    );
					ITB(0x10, 8,djnz    ); ITB(0x11,10,ld_de_nn ); ITB(0x12, 7,ld_md_a ); ITB(0x13, 6,inc_de  );
					ITB(0x14, 4,inc_d   ); ITB(0x15, 4,dec_d    ); ITB(0x16, 7,ld_d_n  ); ITB(0x17, 4,rla     );
					ITB(0x18,12,jr      ); ITB(0x19,11,add_hl_de); ITB(0x1A, 7,ld_a_md ); ITB(0x1B, 6,dec_de  );
					ITB(0x1C, 4,inc_e   ); ITB(0x1D, 4,dec_e    ); ITB(0x1E, 7,ld_e_n  ); ITB(0x1F, 4,rra     );
					ITB(0x20, 7,jr_nz   ); ITB(0x21,10,ld_hl_nn ); ITB(0x22,20,ld_aa_hl); ITB(0x23, 6,inc_hl  );
					ITB(0x24, 4,inc_h   ); ITB(0x25, 4,dec_h    ); ITB(0x26, 7,ld_h_n  ); ITB(0x27, 4,daa     );
					ITB(0x28, 7,jr_z    ); ITB(0x29,11,add_hl_hl); ITB(0x2A,20,ld_hl_aa); ITB(0x2B, 6,dec_hl  );
					ITB(0x2C, 4,inc_l   ); ITB(0x2D, 4,dec_l    ); ITB(0x2E, 7,ld_l_n  ); ITB(0x2F, 4,cpl     );
					ITB(0x30, 7,jr_nc   ); ITB(0x31,10,ld_sp_nn ); ITB(0x32,13,ld_aa_a ); ITB(0x33, 6,inc_sp  );
					ITB(0x34,11,inc_m   ); ITB(0x35,11,dec_m    ); ITB(0x36,10,ld_m_n  ); ITB(0x37, 4,scf     );
					ITB(0x38, 7,jr_c    ); ITB(0x39,11,add_hl_sp); ITB(0x3A,13,ld_a_aa ); ITB(0x3B, 6,dec_sp  );
					ITB(0x3C, 4,inc_a   ); ITB(0x3D, 4,dec_a    ); ITB(0x3E, 7,ld_a_n  ); ITB(0x3F, 4,ccf     );
					ITB(0x40, 4,ld_b_b  ); ITB(0x41, 4,ld_b_c   ); ITB(0x42, 4,ld_b_d  ); ITB(0x43, 4,ld_b_e  );
					ITB(0x44, 4,ld_b_h  ); ITB(0x45, 4,ld_b_l   ); ITB(0x46, 7,ld_b_m  ); ITB(0x47, 4,ld_b_a  );
					ITB(0x48, 4,ld_c_b  ); ITB(0x49, 4,ld_c_c   ); ITB(0x4A, 4,ld_c_d  ); ITB(0x4B, 4,ld_c_e  );
					ITB(0x4C, 4,ld_c_h  ); ITB(0x4D, 4,ld_c_l   ); ITB(0x4E, 7,ld_c_m  ); ITB(0x4F, 4,ld_c_a  );
					ITB(0x50, 4,ld_d_b  ); ITB(0x51, 4,ld_d_c   ); ITB(0x52, 4,ld_d_d  ); ITB(0x53, 4,ld_d_e  );
					ITB(0x54, 4,ld_d_h  ); ITB(0x55, 4,ld_d_l   ); ITB(0x56, 7,ld_d_m  ); ITB(0x57, 4,ld_d_a  );
					ITB(0x58, 4,ld_e_b  ); ITB(0x59, 4,ld_e_c   ); ITB(0x5A, 4,ld_e_d  ); ITB(0x5B, 4,ld_e_e  );
					ITB(0x5C, 4,ld_e_h  ); ITB(0x5D, 4,ld_e_l   ); ITB(0x5E, 7,ld_e_m  ); ITB(0x5F, 4,ld_e_a  );
					ITB(0x60, 4,ld_h_b  ); ITB(0x61, 4,ld_h_c   ); ITB(0x62, 4,ld_h_d  ); ITB(0x63, 4,ld_h_e  );
					ITB(0x64, 4,ld_h_h  ); ITB(0x65, 4,ld_h_l   ); ITB(0x66, 7,ld_h_m  ); ITB(0x67, 4,ld_h_a  );
					ITB(0x68, 4,ld_l_b  ); ITB(0x69, 4,ld_l_c   ); ITB(0x6A, 4,ld_l_d  ); ITB(0x6B, 4,ld_l_e  );
					ITB(0x6C, 4,ld_l_h  ); ITB(0x6D, 4,ld_l_l   ); ITB(0x6E, 7,ld_l_m  ); ITB(0x6F, 4,ld_l_a  );
					ITB(0x70, 7,ld_m_b  ); ITB(0x71, 7,ld_m_c   ); ITB(0x72, 7,ld_m_d  ); ITB(0x73, 7,ld_m_e  );
					ITB(0x74, 7,ld_m_h  ); ITB(0x75, 7,ld_m_l   ); ITB(0x76, 4,halt    ); ITB(0x77, 7,ld_m_a  );
					ITB(0x78, 4,ld_a_b  ); ITB(0x79, 4,ld_a_c   ); ITB(0x7A, 4,ld_a_d  ); ITB(0x7B, 4,ld_a_e  );
					ITB(0x7C, 4,ld_a_h  ); ITB(0x7D, 4,ld_a_l   ); ITB(0x7E, 7,ld_a_m  ); ITB(0x7F, 4,ld_a_a  );
					ITB(0x80, 4,add_a_b ); ITB(0x81, 4,add_a_c  ); ITB(0x82, 4,add_a_d ); ITB(0x83, 4,add_a_e );
					ITB(0x84, 4,add_a_h ); ITB(0x85, 4,add_a_l  ); ITB(0x86, 7,add_a_m ); ITB(0x87, 4,add_a_a );
					ITB(0x88, 4,adc_a_b ); ITB(0x89, 4,adc_a_c  ); ITB(0x8A, 4,adc_a_d ); ITB(0x8B, 4,adc_a_e );
					ITB(0x8C, 4,adc_a_h ); ITB(0x8D, 4,adc_a_l  ); ITB(0x8E, 7,adc_a_m ); ITB(0x8F, 4,adc_a_a );
					ITB(0x90, 4,sub_b   ); ITB(0x91, 4,sub_c    ); ITB(0x92, 4,sub_d   ); ITB(0x93, 4,sub_e   );
					ITB(0x94, 4,sub_h   ); ITB(0x95, 4,sub_l    ); ITB(0x96, 7,sub_m   ); ITB(0x97, 4,sub_a   );
					ITB(0x98, 4,sbc_a_b ); ITB(0x99, 4,sbc_a_c  ); ITB(0x9A, 4,sbc_a_d ); ITB(0x9B, 4,sbc_a_e );
					ITB(0x9C, 4,sbc_a_h ); ITB(0x9D, 4,sbc_a_l  ); ITB(0x9E, 7,sbc_a_m ); ITB(0x9F, 4,sbc_a_a );
					ITB(0xA0, 4,and_b   ); ITB(0xA1, 4,and_c    ); ITB(0xA2, 4,and_d   ); ITB(0xA3, 4,and_e   );
					ITB(0xA4, 4,and_h   ); ITB(0xA5, 4,and_l    ); ITB(0xA6, 7,and_m   ); ITB(0xA7, 4,and_a   );
					ITB(0xA8, 4,xor_b   ); ITB(0xA9, 4,xor_c    ); ITB(0xAA, 4,xor_d   ); ITB(0xAB, 4,xor_e   );
					ITB(0xAC, 4,xor_h   ); ITB(0xAD, 4,xor_l    ); ITB(0xAE, 7,xor_m   ); ITB(0xAF, 4,xor_a   );
					ITB(0xB0, 4,or_b    ); ITB(0xB1, 4,or_c     ); ITB(0xB2, 4,or_d    ); ITB(0xB3, 4,or_e    );
					ITB(0xB4, 4,or_h    ); ITB(0xB5, 4,or_l     ); ITB(0xB6, 7,or_m    ); ITB(0xB7, 4,or_a    );
					ITB(0xB8, 4,cp_b    ); ITB(0xB9, 4,cp_c     ); ITB(0xBA, 4,cp_d    ); ITB(0xBB, 4,cp_e    );
					ITB(0xBC, 4,cp_h    ); ITB(0xBD, 4,cp_l     ); ITB(0xBE, 7,cp_m    ); ITB(0xBF, 4,cp_a    );
					ITB(0xC0, 5,ret_nz  ); ITB(0xC1,10,pop_bc   ); ITB(0xC2,10,jp_nz   ); ITB(0xC3,10,jp      );
					ITB(0xC4,10,call_nz ); ITB(0xC5,11,push_bc  ); ITB(0xC6, 7,add_a_n ); ITB(0xC7,11,rst_00  );
					ITB(0xC8, 5,ret_z   ); ITB(0xC9,10,ret      ); ITB(0xCA,10,jp_z    ); ITB(0xCB, 4,pref_cb );
					ITB(0xCC,10,call_z  ); ITB(0xCD,17,call     ); ITB(0xCE, 7,adc_a_n ); ITB(0xCF,11,rst_08  );
					ITB(0xD0, 5,ret_nc  ); ITB(0xD1,10,pop_de   ); ITB(0xD2,10,jp_nc   ); ITB(0xD3,11,out_n_a );
					ITB(0xD4,10,call_nc ); ITB(0xD5,11,push_de  ); ITB(0xD6, 7,sub_n   ); ITB(0xD7,11,rst_10  );
					ITB(0xD8, 5,ret_c   ); ITB(0xD9, 4,exx      ); ITB(0xDA,10,jp_c    ); ITB(0xDB,11,in_a_n  );
					ITB(0xDC,10,call_c  ); ITB(0xDD, 4,pref_dd  ); ITB(0xDE, 7,sbc_a_n ); ITB(0xDF,11,rst_18  );
					ITB(0xE0, 5,ret_po  ); ITB(0xE1,10,pop_hl   ); ITB(0xE2,10,jp_po   ); ITB(0xE3,19,ex_sp_hl);
					ITB(0xE4,10,call_po ); ITB(0xE5,11,push_hl  ); ITB(0xE6, 7,and_n   ); ITB(0xE7,11,rst_20  );
					ITB(0xE8, 5,ret_pe  ); ITB(0xE9, 4,jp_hl    ); ITB(0xEA,10,jp_pe   ); ITB(0xEB, 4,ex_de_hl);
					ITB(0xEC,10,call_pe ); ITB(0xED, 4,pref_ed  ); ITB(0xEE, 7,xor_n   ); ITB(0xEF,11,rst_28  );
					ITB(0xF0, 5,ret_p   ); ITB(0xF1,10,pop_af   ); ITB(0xF2,10,jp_p    ); ITB(0xF3, 4,di      );
					ITB(0xF4,10,call_p  ); ITB(0xF5,11,push_af  ); ITB(0xF6, 7,or_n    ); ITB(0xF7,11,rst_30  );
					ITB(0xF8, 5,ret_m   ); ITB(0xF9, 6,ld_sp_hl ); ITB(0xFA,10,jp_m    ); ITB(0xFB, 4,ei      );
					ITB(0xFC,10,call_m  ); ITB(0xFD, 4,pref_fd  ); ITB(0xFE, 7,cp_n    ); ITB(0xFF,11,rst_38  );
				}
			break;
			case S_DD:
				R_R = inc_r[R_R];
				// DD (cc +4)
				switch (nb) {
					ITB(0x00, 4,nop       ); ITB(0x01,10,ld_bc_nn  ); ITB(0x02, 7,ld_mb_a ); ITB(0x03, 6,inc_bc   );
					ITB(0x04, 4,inc_b     ); ITB(0x05, 4,dec_b     ); ITB(0x06, 7,ld_b_n  ); ITB(0x07, 4,rlca     );
					ITB(0x08, 4,ex_af_af  ); ITB(0x09,11,add_ix_bc ); ITB(0x0A, 7,ld_a_mb ); ITB(0x0B, 6,dec_bc   );
					ITB(0x0C, 4,inc_c     ); ITB(0x0D, 4,dec_c     ); ITB(0x0E, 7,ld_c_n  ); ITB(0x0F, 4,rrca     );
					ITB(0x10, 8,djnz      ); ITB(0x11,10,ld_de_nn  ); ITB(0x12, 7,ld_md_a ); ITB(0x13, 6,inc_de   );
					ITB(0x14, 4,inc_d     ); ITB(0x15, 4,dec_d     ); ITB(0x16, 7,ld_d_n  ); ITB(0x17, 4,rla      );
					ITB(0x18, 7,jr        ); ITB(0x19,11,add_ix_de ); ITB(0x1A, 7,ld_a_md ); ITB(0x1B, 6,dec_de   );
					ITB(0x1C, 4,inc_e     ); ITB(0x1D, 4,dec_e     ); ITB(0x1E, 7,ld_e_n  ); ITB(0x1F, 4,rra      );
					ITB(0x20, 7,jr_nz     ); ITB(0x21,10,ld_ix_nn  ); ITB(0x22,16,ld_aa_ix); ITB(0x23, 6,inc_ix   );
					ITB(0x24, 4,inc_ixh   ); ITB(0x25, 4,dec_ixh   ); ITB(0x26, 4,ld_ixh_n); ITB(0x27, 4,daa      );
					ITB(0x28, 7,jr_z      ); ITB(0x29,11,add_ix_ix ); ITB(0x2A,16,ld_ix_aa); ITB(0x2B, 6,dec_ix   );
					ITB(0x2C, 4,inc_ixl   ); ITB(0x2D, 4,dec_ixl   ); ITB(0x2E, 4,ld_ixl_n); ITB(0x2F, 4,cpl      );
					ITB(0x30, 7,jr_nc     ); ITB(0x31,10,ld_sp_nn  ); ITB(0x32,13,ld_aa_a ); ITB(0x33, 6,inc_sp   );
					ITB(0x34,19,inc_x     ); ITB(0x35,19,dec_x     ); ITB(0x36,15,ld_x_n  ); ITB(0x37, 4,scf      );
					ITB(0x38, 7,jr_c      ); ITB(0x39,11,add_ix_sp ); ITB(0x3A,13,ld_a_aa ); ITB(0x3B, 6,dec_sp   );
					ITB(0x3C, 4,inc_a     ); ITB(0x3D, 4,dec_a     ); ITB(0x3E, 7,ld_a_n  ); ITB(0x3F, 4,ccf      );
					ITB(0x40, 4,ld_b_b    ); ITB(0x41, 4,ld_b_c    ); ITB(0x42, 4,ld_b_d  ); ITB(0x43, 4,ld_b_e   );
					ITB(0x44, 4,ld_b_ixh  ); ITB(0x45, 4,ld_b_ixl  ); ITB(0x46,15,ld_b_x  ); ITB(0x47, 4,ld_b_a   );
					ITB(0x48, 4,ld_c_b    ); ITB(0x49, 4,ld_c_c    ); ITB(0x4A, 4,ld_c_d  ); ITB(0x4B, 4,ld_c_e   );
					ITB(0x4C, 4,ld_c_ixh  ); ITB(0x4D, 4,ld_c_ixl  ); ITB(0x4E,15,ld_c_x  ); ITB(0x4F, 4,ld_c_a   );
					ITB(0x50, 4,ld_d_b    ); ITB(0x51, 4,ld_d_c    ); ITB(0x52, 4,ld_d_d  ); ITB(0x53, 4,ld_d_e   );
					ITB(0x54, 4,ld_d_ixh  ); ITB(0x55, 4,ld_d_ixl  ); ITB(0x56,15,ld_d_x  ); ITB(0x57, 4,ld_d_a   );
					ITB(0x58, 4,ld_e_b    ); ITB(0x59, 4,ld_e_c    ); ITB(0x5A, 4,ld_e_d  ); ITB(0x5B, 4,ld_e_e   );
					ITB(0x5C, 4,ld_e_ixh  ); ITB(0x5D, 4,ld_e_ixl  ); ITB(0x5E,15,ld_e_x  ); ITB(0x5F, 4,ld_e_a   );
					ITB(0x60, 4,ld_ixh_b  ); ITB(0x61, 4,ld_ixh_c  ); ITB(0x62, 4,ld_ixh_d); ITB(0x63, 4,ld_ixh_e );
					ITB(0x64, 4,ld_ixh_ixh); ITB(0x65, 4,ld_ixh_ixl); ITB(0x66,15,ld_h_x  ); ITB(0x67, 4,ld_ixh_a );
					ITB(0x68, 4,ld_ixl_b  ); ITB(0x69, 4,ld_ixl_c  ); ITB(0x6A, 4,ld_ixl_d); ITB(0x6B, 4,ld_ixl_e );
					ITB(0x6C, 4,ld_ixl_ixh); ITB(0x6D, 4,ld_ixl_ixl); ITB(0x6E,15,ld_l_x  ); ITB(0x6F, 4,ld_ixl_a );
					ITB(0x70,15,ld_x_b    ); ITB(0x71,15,ld_x_c    ); ITB(0x72,15,ld_x_d  ); ITB(0x73,15,ld_x_e   );
					ITB(0x74,15,ld_x_h    ); ITB(0x75,15,ld_x_l    ); ITB(0x76, 4,halt    ); ITB(0x77,15,ld_x_a   );
					ITB(0x78, 4,ld_a_b    ); ITB(0x79, 4,ld_a_c    ); ITB(0x7A, 4,ld_a_d  ); ITB(0x7B, 4,ld_a_e   );
					ITB(0x7C, 4,ld_a_ixh  ); ITB(0x7D, 4,ld_a_ixl  ); ITB(0x7E,15,ld_a_x  ); ITB(0x7F, 4,ld_a_a   );
					ITB(0x80, 4,add_a_b   ); ITB(0x81, 4,add_a_c   ); ITB(0x82, 4,add_a_d ); ITB(0x83, 4,add_a_e  );
					ITB(0x84, 4,add_a_ixh ); ITB(0x85, 4,add_a_ixl ); ITB(0x86,15,add_a_x ); ITB(0x87, 4,add_a_a  );
					ITB(0x88, 4,adc_a_b   ); ITB(0x89, 4,adc_a_c   ); ITB(0x8A, 4,adc_a_d ); ITB(0x8B, 4,adc_a_e  );
					ITB(0x8C, 4,adc_a_ixh ); ITB(0x8D, 4,adc_a_ixl ); ITB(0x8E,15,adc_a_x ); ITB(0x8F, 4,adc_a_a  );
					ITB(0x90, 4,sub_b     ); ITB(0x91, 4,sub_c     ); ITB(0x92, 4,sub_d   ); ITB(0x93, 4,sub_e    );
					ITB(0x94, 4,sub_ixh   ); ITB(0x95, 4,sub_ixl   ); ITB(0x96,15,sub_x   ); ITB(0x97, 4,sub_a    );
					ITB(0x98, 4,sbc_a_b   ); ITB(0x99, 4,sbc_a_c   ); ITB(0x9A, 4,sbc_a_d ); ITB(0x9B, 4,sbc_a_e  );
					ITB(0x9C, 4,sbc_a_ixh ); ITB(0x9D, 4,sbc_a_ixl ); ITB(0x9E,15,sbc_a_x ); ITB(0x9F, 4,sbc_a_a  );
					ITB(0xA0, 4,and_b     ); ITB(0xA1, 4,and_c     ); ITB(0xA2, 4,and_d   ); ITB(0xA3, 4,and_e    );
					ITB(0xA4, 4,and_ixh   ); ITB(0xA5, 4,and_ixl   ); ITB(0xA6,15,and_x   ); ITB(0xA7, 4,and_a    );
					ITB(0xA8, 4,xor_b     ); ITB(0xA9, 4,xor_c     ); ITB(0xAA, 4,xor_d   ); ITB(0xAB, 4,xor_e    );
					ITB(0xAC, 4,xor_ixh   ); ITB(0xAD, 4,xor_ixl   ); ITB(0xAE,15,xor_x   ); ITB(0xAF, 4,xor_a    );
					ITB(0xB0, 4,or_b      ); ITB(0xB1, 4,or_c      ); ITB(0xB2, 4,or_d    ); ITB(0xB3, 4,or_e     );
					ITB(0xB4, 4,or_ixh    ); ITB(0xB5, 4,or_ixl    ); ITB(0xB6,15,or_x    ); ITB(0xB7, 4,or_a     );
					ITB(0xB8, 4,cp_b      ); ITB(0xB9, 4,cp_c      ); ITB(0xBA, 4,cp_d    ); ITB(0xBB, 4,cp_e     );
					ITB(0xBC, 4,cp_ixh    ); ITB(0xBD, 4,cp_ixl    ); ITB(0xBE,15,cp_x    ); ITB(0xBF, 4,cp_a     );
					ITB(0xC0, 5,ret_nz    ); ITB(0xC1,10,pop_bc    ); ITB(0xC2,10,jp_nz   ); ITB(0xC3,10,jp       );
					ITB(0xC4,10,call_nz   ); ITB(0xC5,11,push_bc   ); ITB(0xC6, 7,add_a_n ); ITB(0xC7,11,rst_00   );
					ITB(0xC8, 5,ret_z     ); ITB(0xC9,10,ret       ); ITB(0xCA,10,jp_z    ); ITB(0xCB, 4,pref_ddcb);
					ITB(0xCC,10,call_z    ); ITB(0xCD,17,call      ); ITB(0xCE, 7,adc_a_n ); ITB(0xCF,11,rst_08   );
					ITB(0xD0, 5,ret_nc    ); ITB(0xD1,10,pop_de    ); ITB(0xD2,10,jp_nc   ); ITB(0xD3,11,out_n_a  );
					ITB(0xD4,10,call_nc   ); ITB(0xD5,11,push_de   ); ITB(0xD6, 7,sub_n   ); ITB(0xD7,11,rst_10   );
					ITB(0xD8, 5,ret_c     ); ITB(0xD9, 4,exx       ); ITB(0xDA,10,jp_c    ); ITB(0xDB,11,in_a_n   );
					ITB(0xDC,10,call_c    ); ITB(0xDD, 4,pref_dd   ); ITB(0xDE, 7,sbc_a_n ); ITB(0xDF,11,rst_18   );
					ITB(0xE0, 5,ret_po    ); ITB(0xE1,10,pop_ix    ); ITB(0xE2,10,jp_po   ); ITB(0xE3,19,ex_sp_ix );
					ITB(0xE4,10,call_po   ); ITB(0xE5,11,push_ix   ); ITB(0xE6, 7,and_n   ); ITB(0xE7,11,rst_20   );
					ITB(0xE8, 5,ret_pe    ); ITB(0xE9, 4,jp_ix     ); ITB(0xEA,10,jp_pe   ); ITB(0xEB, 4,ex_de_hl );
					ITB(0xEC,10,call_pe   ); ITB(0xED, 4,pref_ed   ); ITB(0xEE, 7,xor_n   ); ITB(0xEF,11,rst_28   );
					ITB(0xF0, 5,ret_p     ); ITB(0xF1,10,pop_af    ); ITB(0xF2,10,jp_p    ); ITB(0xF3, 4,di       );
					ITB(0xF4,10,call_p    ); ITB(0xF5,11,push_af   ); ITB(0xF6, 7,or_n    ); ITB(0xF7,11,rst_30   );
					ITB(0xF8, 5,ret_m     ); ITB(0xF9, 6,ld_sp_ix  ); ITB(0xFA,10,jp_m    ); ITB(0xFB, 4,ei       );
					ITB(0xFC,10,call_m    ); ITB(0xFD, 4,pref_fd   ); ITB(0xFE, 7,cp_n    ); ITB(0xFF,11,rst_38   );
				}
			break;
			case S_FD:
				R_R = inc_r[R_R];
				// FD (cc +4)
				switch (nb) {
					ITB(0x00, 4,nop       ); ITB(0x01,10,ld_bc_nn  ); ITB(0x02, 7,ld_mb_a ); ITB(0x03, 6,inc_bc   );
					ITB(0x04, 4,inc_b     ); ITB(0x05, 4,dec_b     ); ITB(0x06, 7,ld_b_n  ); ITB(0x07, 4,rlca     );
					ITB(0x08, 4,ex_af_af  ); ITB(0x09,11,add_iy_bc ); ITB(0x0A, 7,ld_a_mb ); ITB(0x0B, 6,dec_bc   );
					ITB(0x0C, 4,inc_c     ); ITB(0x0D, 4,dec_c     ); ITB(0x0E, 7,ld_c_n  ); ITB(0x0F, 4,rrca     );
					ITB(0x10, 8,djnz      ); ITB(0x11,10,ld_de_nn  ); ITB(0x12, 7,ld_md_a ); ITB(0x13, 6,inc_de   );
					ITB(0x14, 4,inc_d     ); ITB(0x15, 4,dec_d     ); ITB(0x16, 7,ld_d_n  ); ITB(0x17, 4,rla      );
					ITB(0x18, 7,jr        ); ITB(0x19,11,add_iy_de ); ITB(0x1A, 7,ld_a_md ); ITB(0x1B, 6,dec_de   );
					ITB(0x1C, 4,inc_e     ); ITB(0x1D, 4,dec_e     ); ITB(0x1E, 7,ld_e_n  ); ITB(0x1F, 4,rra      );
					ITB(0x20, 7,jr_nz     ); ITB(0x21,10,ld_iy_nn  ); ITB(0x22,16,ld_aa_iy); ITB(0x23, 6,inc_iy   );
					ITB(0x24, 4,inc_iyh   ); ITB(0x25, 4,dec_iyh   ); ITB(0x26, 4,ld_iyh_n); ITB(0x27, 4,daa      );
					ITB(0x28, 7,jr_z      ); ITB(0x29,11,add_iy_iy ); ITB(0x2A,16,ld_iy_aa); ITB(0x2B, 6,dec_iy   );
					ITB(0x2C, 4,inc_iyl   ); ITB(0x2D, 4,dec_iyl   ); ITB(0x2E, 4,ld_iyl_n); ITB(0x2F, 4,cpl      );
					ITB(0x30, 7,jr_nc     ); ITB(0x31,10,ld_sp_nn  ); ITB(0x32,13,ld_aa_a ); ITB(0x33, 6,inc_sp   );
					ITB(0x34,19,inc_y     ); ITB(0x35,19,dec_y     ); ITB(0x36,15,ld_y_n  ); ITB(0x37, 4,scf      );
					ITB(0x38, 7,jr_c      ); ITB(0x39,11,add_iy_sp ); ITB(0x3A,13,ld_a_aa ); ITB(0x3B, 6,dec_sp   );
					ITB(0x3C, 4,inc_a     ); ITB(0x3D, 4,dec_a     ); ITB(0x3E, 7,ld_a_n  ); ITB(0x3F, 4,ccf      );
					ITB(0x40, 4,ld_b_b    ); ITB(0x41, 4,ld_b_c    ); ITB(0x42, 4,ld_b_d  ); ITB(0x43, 4,ld_b_e   );
					ITB(0x44, 4,ld_b_iyh  ); ITB(0x45, 4,ld_b_iyl  ); ITB(0x46,15,ld_b_y  ); ITB(0x47, 4,ld_b_a   );
					ITB(0x48, 4,ld_c_b    ); ITB(0x49, 4,ld_c_c    ); ITB(0x4A, 4,ld_c_d  ); ITB(0x4B, 4,ld_c_e   );
					ITB(0x4C, 4,ld_c_iyh  ); ITB(0x4D, 4,ld_c_iyl  ); ITB(0x4E,15,ld_c_y  ); ITB(0x4F, 4,ld_c_a   );
					ITB(0x50, 4,ld_d_b    ); ITB(0x51, 4,ld_d_c    ); ITB(0x52, 4,ld_d_d  ); ITB(0x53, 4,ld_d_e   );
					ITB(0x54, 4,ld_d_iyh  ); ITB(0x55, 4,ld_d_iyl  ); ITB(0x56,15,ld_d_y  ); ITB(0x57, 4,ld_d_a   );
					ITB(0x58, 4,ld_e_b    ); ITB(0x59, 4,ld_e_c    ); ITB(0x5A, 4,ld_e_d  ); ITB(0x5B, 4,ld_e_e   );
					ITB(0x5C, 4,ld_e_iyh  ); ITB(0x5D, 4,ld_e_iyl  ); ITB(0x5E,15,ld_e_y  ); ITB(0x5F, 4,ld_e_a   );
					ITB(0x60, 4,ld_iyh_b  ); ITB(0x61, 4,ld_iyh_c  ); ITB(0x62, 4,ld_iyh_d); ITB(0x63, 4,ld_iyh_e );
					ITB(0x64, 4,ld_iyh_iyh); ITB(0x65, 4,ld_iyh_iyl); ITB(0x66,15,ld_h_y  ); ITB(0x67, 4,ld_iyh_a );
					ITB(0x68, 4,ld_iyl_b  ); ITB(0x69, 4,ld_iyl_c  ); ITB(0x6A, 4,ld_iyl_d); ITB(0x6B, 4,ld_iyl_e );
					ITB(0x6C, 4,ld_iyl_iyh); ITB(0x6D, 4,ld_iyl_iyl); ITB(0x6E,15,ld_l_y  ); ITB(0x6F, 4,ld_iyl_a );
					ITB(0x70,15,ld_y_b    ); ITB(0x71,15,ld_y_c    ); ITB(0x72,15,ld_y_d  ); ITB(0x73,15,ld_y_e   );
					ITB(0x74,15,ld_y_h    ); ITB(0x75,15,ld_y_l    ); ITB(0x76, 4,halt    ); ITB(0x77,15,ld_y_a   );
					ITB(0x78, 4,ld_a_b    ); ITB(0x79, 4,ld_a_c    ); ITB(0x7A, 4,ld_a_d  ); ITB(0x7B, 4,ld_a_e   );
					ITB(0x7C, 4,ld_a_iyh  ); ITB(0x7D, 4,ld_a_iyl  ); ITB(0x7E,15,ld_a_y  ); ITB(0x7F, 4,ld_a_a   );
					ITB(0x80, 4,add_a_b   ); ITB(0x81, 4,add_a_c   ); ITB(0x82, 4,add_a_d ); ITB(0x83, 4,add_a_e  );
					ITB(0x84, 4,add_a_iyh ); ITB(0x85, 4,add_a_iyl ); ITB(0x86,15,add_a_y ); ITB(0x87, 4,add_a_a  );
					ITB(0x88, 4,adc_a_b   ); ITB(0x89, 4,adc_a_c   ); ITB(0x8A, 4,adc_a_d ); ITB(0x8B, 4,adc_a_e  );
					ITB(0x8C, 4,adc_a_iyh ); ITB(0x8D, 4,adc_a_iyl ); ITB(0x8E,15,adc_a_y ); ITB(0x8F, 4,adc_a_a  );
					ITB(0x90, 4,sub_b     ); ITB(0x91, 4,sub_c     ); ITB(0x92, 4,sub_d   ); ITB(0x93, 4,sub_e    );
					ITB(0x94, 4,sub_iyh   ); ITB(0x95, 4,sub_iyl   ); ITB(0x96,15,sub_y   ); ITB(0x97, 4,sub_a    );
					ITB(0x98, 4,sbc_a_b   ); ITB(0x99, 4,sbc_a_c   ); ITB(0x9A, 4,sbc_a_d ); ITB(0x9B, 4,sbc_a_e  );
					ITB(0x9C, 4,sbc_a_iyh ); ITB(0x9D, 4,sbc_a_iyl ); ITB(0x9E,15,sbc_a_y ); ITB(0x9F, 4,sbc_a_a  );
					ITB(0xA0, 4,and_b     ); ITB(0xA1, 4,and_c     ); ITB(0xA2, 4,and_d   ); ITB(0xA3, 4,and_e    );
					ITB(0xA4, 4,and_iyh   ); ITB(0xA5, 4,and_iyl   ); ITB(0xA6,15,and_y   ); ITB(0xA7, 4,and_a    );
					ITB(0xA8, 4,xor_b     ); ITB(0xA9, 4,xor_c     ); ITB(0xAA, 4,xor_d   ); ITB(0xAB, 4,xor_e    );
					ITB(0xAC, 4,xor_iyh   ); ITB(0xAD, 4,xor_iyl   ); ITB(0xAE,15,xor_y   ); ITB(0xAF, 4,xor_a    );
					ITB(0xB0, 4,or_b      ); ITB(0xB1, 4,or_c      ); ITB(0xB2, 4,or_d    ); ITB(0xB3, 4,or_e     );
					ITB(0xB4, 4,or_iyh    ); ITB(0xB5, 4,or_iyl    ); ITB(0xB6,15,or_y    ); ITB(0xB7, 4,or_a     );
					ITB(0xB8, 4,cp_b      ); ITB(0xB9, 4,cp_c      ); ITB(0xBA, 4,cp_d    ); ITB(0xBB, 4,cp_e     );
					ITB(0xBC, 4,cp_iyh    ); ITB(0xBD, 4,cp_iyl    ); ITB(0xBE,15,cp_y    ); ITB(0xBF, 4,cp_a     );
					ITB(0xC0, 5,ret_nz    ); ITB(0xC1,10,pop_bc    ); ITB(0xC2,10,jp_nz   ); ITB(0xC3,10,jp       );
					ITB(0xC4,10,call_nz   ); ITB(0xC5,11,push_bc   ); ITB(0xC6, 7,add_a_n ); ITB(0xC7,11,rst_00   );
					ITB(0xC8, 5,ret_z     ); ITB(0xC9,10,ret       ); ITB(0xCA,10,jp_z    ); ITB(0xCB, 4,pref_fdcb);
					ITB(0xCC,10,call_z    ); ITB(0xCD,17,call      ); ITB(0xCE, 7,adc_a_n ); ITB(0xCF,11,rst_08   );
					ITB(0xD0, 5,ret_nc    ); ITB(0xD1,10,pop_de    ); ITB(0xD2,10,jp_nc   ); ITB(0xD3,11,out_n_a  );
					ITB(0xD4,10,call_nc   ); ITB(0xD5,11,push_de   ); ITB(0xD6, 7,sub_n   ); ITB(0xD7,11,rst_10   );
					ITB(0xD8, 5,ret_c     ); ITB(0xD9, 4,exx       ); ITB(0xDA,10,jp_c    ); ITB(0xDB,11,in_a_n   );
					ITB(0xDC,10,call_c    ); ITB(0xDD, 4,pref_dd   ); ITB(0xDE, 7,sbc_a_n ); ITB(0xDF,11,rst_18   );
					ITB(0xE0, 5,ret_po    ); ITB(0xE1,10,pop_iy    ); ITB(0xE2,10,jp_po   ); ITB(0xE3,19,ex_sp_iy );
					ITB(0xE4,10,call_po   ); ITB(0xE5,11,push_iy   ); ITB(0xE6, 7,and_n   ); ITB(0xE7,11,rst_20   );
					ITB(0xE8, 5,ret_pe    ); ITB(0xE9, 4,jp_iy     ); ITB(0xEA,10,jp_pe   ); ITB(0xEB, 4,ex_de_hl );
					ITB(0xEC,10,call_pe   ); ITB(0xED, 4,pref_ed   ); ITB(0xEE, 7,xor_n   ); ITB(0xEF,11,rst_28   );
					ITB(0xF0, 5,ret_p     ); ITB(0xF1,10,pop_af    ); ITB(0xF2,10,jp_p    ); ITB(0xF3, 4,di       );
					ITB(0xF4,10,call_p    ); ITB(0xF5,11,push_af   ); ITB(0xF6, 7,or_n    ); ITB(0xF7,11,rst_30   );
					ITB(0xF8, 5,ret_m     ); ITB(0xF9, 6,ld_sp_iy  ); ITB(0xFA,10,jp_m    ); ITB(0xFB, 4,ei       );
					ITB(0xFC,10,call_m    ); ITB(0xFD, 4,pref_fd   ); ITB(0xFE, 7,cp_n    ); ITB(0xFF,11,rst_38   );
				}
			break;
			case S_ED:
				R_R = inc_r[R_R];
				// ED (cc +4)
				switch (nb) {
					ITB(0x40, 8,in_b_c   ); ITB(0x48, 8,in_c_c   ); ITB(0x50, 8,in_d_c   ); ITB(0x58, 8,in_e_c   );
					ITB(0x60, 8,in_h_c   ); ITB(0x68, 8,in_l_c   ); ITB(0x70, 8,in_f_c   ); ITB(0x78, 8,in_a_c   );
					ITB(0x41, 8,out_c_b  ); ITB(0x49, 8,out_c_c  ); ITB(0x51, 8,out_c_d  ); ITB(0x59, 8,out_c_e  );
					ITB(0x61, 8,out_c_h  ); ITB(0x69, 8,out_c_l  ); ITB(0x71, 8,out_c_0  ); ITB(0x79, 8,out_c_a  );
					ITB(0x42,11,sbc_hl_bc); ITB(0x52,11,sbc_hl_de); ITB(0x62,11,sbc_hl_hl); ITB(0x72,11,sbc_hl_sp);
					ITB(0x4A,11,adc_hl_bc); ITB(0x5A,11,adc_hl_de); ITB(0x6A,11,adc_hl_hl); ITB(0x7A,11,adc_hl_sp);
					ITB(0x43,16,ld_aa_bc ); ITB(0x53,16,ld_aa_de ); ITB(0x63,16,ld_aa_hl ); ITB(0x73,16,ld_aa_sp );
					ITB(0x4B,16,ld_bc_aa ); ITB(0x5B,16,ld_de_aa ); ITB(0x6B,16,ld_hl_aa ); ITB(0x7B,16,ld_sp_aa );
					ITB(0x47, 5,ld_i_a   ); ITB(0x4F, 5,ld_r_a   ); ITB(0x57, 5,ld_a_i   ); ITB(0x5F, 5,ld_a_r   );
					case 0x44: case 0x4C: case 0x54: case 0x5C: case 0x64: case 0x6C: case 0x74: ITB(0x7C, 4,neg );
					case 0x45: case 0x55: case 0x65: ITB(0x75,10,retn);
					case 0x4D: case 0x5D: case 0x6D: ITB(0x7D,10,reti);
					case 0x46: case 0x4E: case 0x66: ITB(0x6E, 4,im_0);
					case 0x56: ITB(0x76, 4,im_1); case 0x5E: ITB(0x7E, 4,im_2);
					ITB(0x67,14,rrd); ITB(0x6F,14,rld);
					ITB(0xA0,12,ldi); ITB(0xA8,12,ldd); ITB(0xB0,12,ldir); ITB(0xB8,12,lddr);
					ITB(0xA1,12,cpi); ITB(0xA9,12,cpd); ITB(0xB1,12,cpir); ITB(0xB9,12,cpdr);
					ITB(0xA2,12,ini); ITB(0xAA,12,ind); ITB(0xB2,12,inir); ITB(0xBA,12,indr);
					ITB(0xA3,12,outi); ITB(0xAB,12,outd); ITB(0xB3,12,otir); ITB(0xBB,12,otdr);
					default: z80cctmp += 4; trap();
				}
			break;
			case S_CB:
				R_R = inc_r[R_R];
				// CB (cc +4)
				switch (nb) {
					ITB(0x00, 4,rlc_b  ); ITB(0x01, 4,rlc_c  ); ITB(0x02, 4,rlc_d  ); ITB(0x03, 4,rlc_e  );
					ITB(0x04, 4,rlc_h  ); ITB(0x05, 4,rlc_l  ); ITB(0x06,11,rlc_m  ); ITB(0x07, 4,rlc_a  );
					ITB(0x08, 4,rrc_b  ); ITB(0x09, 4,rrc_c  ); ITB(0x0A, 4,rrc_d  ); ITB(0x0B, 4,rrc_e  );
					ITB(0x0C, 4,rrc_h  ); ITB(0x0D, 4,rrc_l  ); ITB(0x0E,11,rrc_m  ); ITB(0x0F, 4,rrc_a  );
					ITB(0x10, 4,rl_b   ); ITB(0x11, 4,rl_c   ); ITB(0x12, 4,rl_d   ); ITB(0x13, 4,rl_e   );
					ITB(0x14, 4,rl_h   ); ITB(0x15, 4,rl_l   ); ITB(0x16,11,rl_m   ); ITB(0x17, 4,rl_a   );
					ITB(0x18, 4,rr_b   ); ITB(0x19, 4,rr_c   ); ITB(0x1A, 4,rr_d   ); ITB(0x1B, 4,rr_e   );
					ITB(0x1C, 4,rr_h   ); ITB(0x1D, 4,rr_l   ); ITB(0x1E,11,rr_m   ); ITB(0x1F, 4,rr_a   );
					ITB(0x20, 4,sla_b  ); ITB(0x21, 4,sla_c  ); ITB(0x22, 4,sla_d  ); ITB(0x23, 4,sla_e  );
					ITB(0x24, 4,sla_h  ); ITB(0x25, 4,sla_l  ); ITB(0x26,11,sla_m  ); ITB(0x27, 4,sla_a  );
					ITB(0x28, 4,sra_b  ); ITB(0x29, 4,sra_c  ); ITB(0x2A, 4,sra_d  ); ITB(0x2B, 4,sra_e  );
					ITB(0x2C, 4,sra_h  ); ITB(0x2D, 4,sra_l  ); ITB(0x2E,11,sra_m  ); ITB(0x2F, 4,sra_a  );
					ITB(0x30, 4,sll_b  ); ITB(0x31, 4,sll_c  ); ITB(0x32, 4,sll_d  ); ITB(0x33, 4,sll_e  );
					ITB(0x34, 4,sll_h  ); ITB(0x35, 4,sll_l  ); ITB(0x36,11,sll_m  ); ITB(0x37, 4,sll_a  );
					ITB(0x38, 4,srl_b  ); ITB(0x39, 4,srl_c  ); ITB(0x3A, 4,srl_d  ); ITB(0x3B, 4,srl_e  );
					ITB(0x3C, 4,srl_h  ); ITB(0x3D, 4,srl_l  ); ITB(0x3E,11,srl_m  ); ITB(0x3F, 4,srl_a  );
					ITB(0x40, 4,bit_0_b); ITB(0x41, 4,bit_0_c); ITB(0x42, 4,bit_0_d); ITB(0x43, 4,bit_0_e);
					ITB(0x44, 4,bit_0_h); ITB(0x45, 4,bit_0_l); ITB(0x46, 8,bit_0_m); ITB(0x47, 4,bit_0_a);
					ITB(0x48, 4,bit_1_b); ITB(0x49, 4,bit_1_c); ITB(0x4A, 4,bit_1_d); ITB(0x4B, 4,bit_1_e);
					ITB(0x4C, 4,bit_1_h); ITB(0x4D, 4,bit_1_l); ITB(0x4E, 8,bit_1_m); ITB(0x4F, 4,bit_1_a);
					ITB(0x50, 4,bit_2_b); ITB(0x51, 4,bit_2_c); ITB(0x52, 4,bit_2_d); ITB(0x53, 4,bit_2_e);
					ITB(0x54, 4,bit_2_h); ITB(0x55, 4,bit_2_l); ITB(0x56, 8,bit_2_m); ITB(0x57, 4,bit_2_a);
					ITB(0x58, 4,bit_3_b); ITB(0x59, 4,bit_3_c); ITB(0x5A, 4,bit_3_d); ITB(0x5B, 4,bit_3_e);
					ITB(0x5C, 4,bit_3_h); ITB(0x5D, 4,bit_3_l); ITB(0x5E, 8,bit_3_m); ITB(0x5F, 4,bit_3_a);
					ITB(0x60, 4,bit_4_b); ITB(0x61, 4,bit_4_c); ITB(0x62, 4,bit_4_d); ITB(0x63, 4,bit_4_e);
					ITB(0x64, 4,bit_4_h); ITB(0x65, 4,bit_4_l); ITB(0x66, 8,bit_4_m); ITB(0x67, 4,bit_4_a);
					ITB(0x68, 4,bit_5_b); ITB(0x69, 4,bit_5_c); ITB(0x6A, 4,bit_5_d); ITB(0x6B, 4,bit_5_e);
					ITB(0x6C, 4,bit_5_h); ITB(0x6D, 4,bit_5_l); ITB(0x6E, 8,bit_5_m); ITB(0x6F, 4,bit_5_a);
					ITB(0x70, 4,bit_6_b); ITB(0x71, 4,bit_6_c); ITB(0x72, 4,bit_6_d); ITB(0x73, 4,bit_6_e);
					ITB(0x74, 4,bit_6_h); ITB(0x75, 4,bit_6_l); ITB(0x76, 8,bit_6_m); ITB(0x77, 4,bit_6_a);
					ITB(0x78, 4,bit_7_b); ITB(0x79, 4,bit_7_c); ITB(0x7A, 4,bit_7_d); ITB(0x7B, 4,bit_7_e);
					ITB(0x7C, 4,bit_7_h); ITB(0x7D, 4,bit_7_l); ITB(0x7E, 8,bit_7_m); ITB(0x7F, 4,bit_7_a);
					ITB(0x80, 4,res_0_b); ITB(0x81, 4,res_0_c); ITB(0x82, 4,res_0_d); ITB(0x83, 4,res_0_e);
					ITB(0x84, 4,res_0_h); ITB(0x85, 4,res_0_l); ITB(0x86,11,res_0_m); ITB(0x87, 4,res_0_a);
					ITB(0x88, 4,res_1_b); ITB(0x89, 4,res_1_c); ITB(0x8A, 4,res_1_d); ITB(0x8B, 4,res_1_e);
					ITB(0x8C, 4,res_1_h); ITB(0x8D, 4,res_1_l); ITB(0x8E,11,res_1_m); ITB(0x8F, 4,res_1_a);
					ITB(0x90, 4,res_2_b); ITB(0x91, 4,res_2_c); ITB(0x92, 4,res_2_d); ITB(0x93, 4,res_2_e);
					ITB(0x94, 4,res_2_h); ITB(0x95, 4,res_2_l); ITB(0x96,11,res_2_m); ITB(0x97, 4,res_2_a);
					ITB(0x98, 4,res_3_b); ITB(0x99, 4,res_3_c); ITB(0x9A, 4,res_3_d); ITB(0x9B, 4,res_3_e);
					ITB(0x9C, 4,res_3_h); ITB(0x9D, 4,res_3_l); ITB(0x9E,11,res_3_m); ITB(0x9F, 4,res_3_a);
					ITB(0xA0, 4,res_4_b); ITB(0xA1, 4,res_4_c); ITB(0xA2, 4,res_4_d); ITB(0xA3, 4,res_4_e);
					ITB(0xA4, 4,res_4_h); ITB(0xA5, 4,res_4_l); ITB(0xA6,11,res_4_m); ITB(0xA7, 4,res_4_a);
					ITB(0xA8, 4,res_5_b); ITB(0xA9, 4,res_5_c); ITB(0xAA, 4,res_5_d); ITB(0xAB, 4,res_5_e);
					ITB(0xAC, 4,res_5_h); ITB(0xAD, 4,res_5_l); ITB(0xAE,11,res_5_m); ITB(0xAF, 4,res_5_a);
					ITB(0xB0, 4,res_6_b); ITB(0xB1, 4,res_6_c); ITB(0xB2, 4,res_6_d); ITB(0xB3, 4,res_6_e);
					ITB(0xB4, 4,res_6_h); ITB(0xB5, 4,res_6_l); ITB(0xB6,11,res_6_m); ITB(0xB7, 4,res_6_a);
					ITB(0xB8, 4,res_7_b); ITB(0xB9, 4,res_7_c); ITB(0xBA, 4,res_7_d); ITB(0xBB, 4,res_7_e);
					ITB(0xBC, 4,res_7_h); ITB(0xBD, 4,res_7_l); ITB(0xBE,11,res_7_m); ITB(0xBF, 4,res_7_a);
					ITB(0xC0, 4,set_0_b); ITB(0xC1, 4,set_0_c); ITB(0xC2, 4,set_0_d); ITB(0xC3, 4,set_0_e);
					ITB(0xC4, 4,set_0_h); ITB(0xC5, 4,set_0_l); ITB(0xC6,11,set_0_m); ITB(0xC7, 4,set_0_a);
					ITB(0xC8, 4,set_1_b); ITB(0xC9, 4,set_1_c); ITB(0xCA, 4,set_1_d); ITB(0xCB, 4,set_1_e);
					ITB(0xCC, 4,set_1_h); ITB(0xCD, 4,set_1_l); ITB(0xCE,11,set_1_m); ITB(0xCF, 4,set_1_a);
					ITB(0xD0, 4,set_2_b); ITB(0xD1, 4,set_2_c); ITB(0xD2, 4,set_2_d); ITB(0xD3, 4,set_2_e);
					ITB(0xD4, 4,set_2_h); ITB(0xD5, 4,set_2_l); ITB(0xD6,11,set_2_m); ITB(0xD7, 4,set_2_a);
					ITB(0xD8, 4,set_3_b); ITB(0xD9, 4,set_3_c); ITB(0xDA, 4,set_3_d); ITB(0xDB, 4,set_3_e);
					ITB(0xDC, 4,set_3_h); ITB(0xDD, 4,set_3_l); ITB(0xDE,11,set_3_m); ITB(0xDF, 4,set_3_a);
					ITB(0xE0, 4,set_4_b); ITB(0xE1, 4,set_4_c); ITB(0xE2, 4,set_4_d); ITB(0xE3, 4,set_4_e);
					ITB(0xE4, 4,set_4_h); ITB(0xE5, 4,set_4_l); ITB(0xE6,11,set_4_m); ITB(0xE7, 4,set_4_a);
					ITB(0xE8, 4,set_5_b); ITB(0xE9, 4,set_5_c); ITB(0xEA, 4,set_5_d); ITB(0xEB, 4,set_5_e);
					ITB(0xEC, 4,set_5_h); ITB(0xED, 4,set_5_l); ITB(0xEE,11,set_5_m); ITB(0xEF, 4,set_5_a);
					ITB(0xF0, 4,set_6_b); ITB(0xF1, 4,set_6_c); ITB(0xF2, 4,set_6_d); ITB(0xF3, 4,set_6_e);
					ITB(0xF4, 4,set_6_h); ITB(0xF5, 4,set_6_l); ITB(0xF6,11,set_6_m); ITB(0xF7, 4,set_6_a);
					ITB(0xF8, 4,set_7_b); ITB(0xF9, 4,set_7_c); ITB(0xFA, 4,set_7_d); ITB(0xFB, 4,set_7_e);
					ITB(0xFC, 4,set_7_h); ITB(0xFD, 4,set_7_l); ITB(0xFE,11,set_7_m); ITB(0xFF, 4,set_7_a);
				}
			break;
			case S_DDCB:
				// DDCB (cc +8)
				z80->idx = ((int8_t)nb) + R_IX;
				FETCH_VAL8(nb);
				switch (nb) {
					ITB(0x00,15,rlc_x_b); ITB(0x01,15,rlc_x_c); ITB(0x02,15,rlc_x_d); ITB(0x03,15,rlc_x_e);
					ITB(0x04,15,rlc_x_h); ITB(0x05,15,rlc_x_l); ITB(0x06,15,rlc_x  ); ITB(0x07,15,rlc_x_a);
					ITB(0x08,15,rrc_x_b); ITB(0x09,15,rrc_x_c); ITB(0x0A,15,rrc_x_d); ITB(0x0B,15,rrc_x_e);
					ITB(0x0C,15,rrc_x_h); ITB(0x0D,15,rrc_x_l); ITB(0x0E,15,rrc_x  ); ITB(0x0F,15,rrc_x_a);
					ITB(0x10,15,rl_x_b ); ITB(0x11,15,rl_x_c ); ITB(0x12,15,rl_x_d ); ITB(0x13,15,rl_x_e );
					ITB(0x14,15,rl_x_h ); ITB(0x15,15,rl_x_l ); ITB(0x16,15,rl_x   ); ITB(0x17,15,rl_x_a );
					ITB(0x18,15,rr_x_b ); ITB(0x19,15,rr_x_c ); ITB(0x1A,15,rr_x_d ); ITB(0x1B,15,rr_x_e );
					ITB(0x1C,15,rr_x_h ); ITB(0x1D,15,rr_x_l ); ITB(0x1E,15,rr_x   ); ITB(0x1F,15,rr_x_a );
					ITB(0x20,15,sla_x_b); ITB(0x21,15,sla_x_c); ITB(0x22,15,sla_x_d); ITB(0x23,15,sla_x_e);
					ITB(0x24,15,sla_x_h); ITB(0x25,15,sla_x_l); ITB(0x26,15,sla_x  ); ITB(0x27,15,sla_x_a);
					ITB(0x28,15,sra_x_b); ITB(0x29,15,sra_x_c); ITB(0x2A,15,sra_x_d); ITB(0x2B,15,sra_x_e);
					ITB(0x2C,15,sra_x_h); ITB(0x2D,15,sra_x_l); ITB(0x2E,15,sra_x  ); ITB(0x2F,15,sra_x_a);
					ITB(0x30,15,sll_x_b); ITB(0x31,15,sll_x_c); ITB(0x32,15,sll_x_d); ITB(0x33,15,sll_x_e);
					ITB(0x34,15,sll_x_h); ITB(0x35,15,sll_x_l); ITB(0x36,15,sll_x  ); ITB(0x37,15,sll_x_a);
					ITB(0x38,15,srl_x_b); ITB(0x39,15,srl_x_c); ITB(0x3A,15,srl_x_d); ITB(0x3B,15,srl_x_e);
					ITB(0x3C,15,srl_x_h); ITB(0x3D,15,srl_x_l); ITB(0x3E,15,srl_x  ); ITB(0x3F,15,srl_x_a);
					case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: ITB(0x47,12,bit_0_x);
					case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: ITB(0x4F,12,bit_1_x);
					case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: ITB(0x57,12,bit_2_x);
					case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: ITB(0x5F,12,bit_3_x);
					case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: ITB(0x67,12,bit_4_x);
					case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: ITB(0x6F,12,bit_5_x);
					case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: ITB(0x77,12,bit_6_x);
					case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: ITB(0x7F,12,bit_7_x);
					ITB(0x80,15,res_0_x_b); ITB(0x81,15,res_0_x_c); ITB(0x82,15,res_0_x_d); ITB(0x83,15,res_0_x_e);
					ITB(0x84,15,res_0_x_h); ITB(0x85,15,res_0_x_l); ITB(0x86,15,res_0_x  ); ITB(0x87,15,res_0_x_a);
					ITB(0x88,15,res_1_x_b); ITB(0x89,15,res_1_x_c); ITB(0x8A,15,res_1_x_d); ITB(0x8B,15,res_1_x_e);
					ITB(0x8C,15,res_1_x_h); ITB(0x8D,15,res_1_x_l); ITB(0x8E,15,res_1_x  ); ITB(0x8F,15,res_1_x_a);
					ITB(0x90,15,res_2_x_b); ITB(0x91,15,res_2_x_c); ITB(0x92,15,res_2_x_d); ITB(0x93,15,res_2_x_e);
					ITB(0x94,15,res_2_x_h); ITB(0x95,15,res_2_x_l); ITB(0x96,15,res_2_x  ); ITB(0x97,15,res_2_x_a);
					ITB(0x98,15,res_3_x_b); ITB(0x99,15,res_3_x_c); ITB(0x9A,15,res_3_x_d); ITB(0x9B,15,res_3_x_e);
					ITB(0x9C,15,res_3_x_h); ITB(0x9D,15,res_3_x_l); ITB(0x9E,15,res_3_x  ); ITB(0x9F,15,res_3_x_a);
					ITB(0xA0,15,res_4_x_b); ITB(0xA1,15,res_4_x_c); ITB(0xA2,15,res_4_x_d); ITB(0xA3,15,res_4_x_e);
					ITB(0xA4,15,res_4_x_h); ITB(0xA5,15,res_4_x_l); ITB(0xA6,15,res_4_x  ); ITB(0xA7,15,res_4_x_a);
					ITB(0xA8,15,res_5_x_b); ITB(0xA9,15,res_5_x_c); ITB(0xAA,15,res_5_x_d); ITB(0xAB,15,res_5_x_e);
					ITB(0xAC,15,res_5_x_h); ITB(0xAD,15,res_5_x_l); ITB(0xAE,15,res_5_x  ); ITB(0xAF,15,res_5_x_a);
					ITB(0xB0,15,res_6_x_b); ITB(0xB1,15,res_6_x_c); ITB(0xB2,15,res_6_x_d); ITB(0xB3,15,res_6_x_e);
					ITB(0xB4,15,res_6_x_h); ITB(0xB5,15,res_6_x_l); ITB(0xB6,15,res_6_x  ); ITB(0xB7,15,res_6_x_a);
					ITB(0xB8,15,res_7_x_b); ITB(0xB9,15,res_7_x_c); ITB(0xBA,15,res_7_x_d); ITB(0xBB,15,res_7_x_e);
					ITB(0xBC,15,res_7_x_h); ITB(0xBD,15,res_7_x_l); ITB(0xBE,15,res_7_x  ); ITB(0xBF,15,res_7_x_a);
					ITB(0xC0,15,set_0_x_b); ITB(0xC1,15,set_0_x_c); ITB(0xC2,15,set_0_x_d); ITB(0xC3,15,set_0_x_e);
					ITB(0xC4,15,set_0_x_h); ITB(0xC5,15,set_0_x_l); ITB(0xC6,15,set_0_x  ); ITB(0xC7,15,set_0_x_a);
					ITB(0xC8,15,set_1_x_b); ITB(0xC9,15,set_1_x_c); ITB(0xCA,15,set_1_x_d); ITB(0xCB,15,set_1_x_e);
					ITB(0xCC,15,set_1_x_h); ITB(0xCD,15,set_1_x_l); ITB(0xCE,15,set_1_x  ); ITB(0xCF,15,set_1_x_a);
					ITB(0xD0,15,set_2_x_b); ITB(0xD1,15,set_2_x_c); ITB(0xD2,15,set_2_x_d); ITB(0xD3,15,set_2_x_e);
					ITB(0xD4,15,set_2_x_h); ITB(0xD5,15,set_2_x_l); ITB(0xD6,15,set_2_x  ); ITB(0xD7,15,set_2_x_a);
					ITB(0xD8,15,set_3_x_b); ITB(0xD9,15,set_3_x_c); ITB(0xDA,15,set_3_x_d); ITB(0xDB,15,set_3_x_e);
					ITB(0xDC,15,set_3_x_h); ITB(0xDD,15,set_3_x_l); ITB(0xDE,15,set_3_x  ); ITB(0xDF,15,set_3_x_a);
					ITB(0xE0,15,set_4_x_b); ITB(0xE1,15,set_4_x_c); ITB(0xE2,15,set_4_x_d); ITB(0xE3,15,set_4_x_e);
					ITB(0xE4,15,set_4_x_h); ITB(0xE5,15,set_4_x_l); ITB(0xE6,15,set_4_x  ); ITB(0xE7,15,set_4_x_a);
					ITB(0xE8,15,set_5_x_b); ITB(0xE9,15,set_5_x_c); ITB(0xEA,15,set_5_x_d); ITB(0xEB,15,set_5_x_e);
					ITB(0xEC,15,set_5_x_h); ITB(0xED,15,set_5_x_l); ITB(0xEE,15,set_5_x  ); ITB(0xEF,15,set_5_x_a);
					ITB(0xF0,15,set_6_x_b); ITB(0xF1,15,set_6_x_c); ITB(0xF2,15,set_6_x_d); ITB(0xF3,15,set_6_x_e);
					ITB(0xF4,15,set_6_x_h); ITB(0xF5,15,set_6_x_l); ITB(0xF6,15,set_6_x  ); ITB(0xF7,15,set_6_x_a);
					ITB(0xF8,15,set_7_x_b); ITB(0xF9,15,set_7_x_c); ITB(0xFA,15,set_7_x_d); ITB(0xFB,15,set_7_x_e);
					ITB(0xFC,15,set_7_x_h); ITB(0xFD,15,set_7_x_l); ITB(0xFE,15,set_7_x  ); ITB(0xFF,15,set_7_x_a);
				}
			break;
			case S_FDCB:
				// FDCB (cc +8)
				z80->idx = ((int8_t)nb) + R_IY;
				FETCH_VAL8(nb);
				switch (nb) {
					ITB(0x00,15,rlc_y_b); ITB(0x01,15,rlc_y_c); ITB(0x02,15,rlc_y_d); ITB(0x03,15,rlc_y_e);
					ITB(0x04,15,rlc_y_h); ITB(0x05,15,rlc_y_l); ITB(0x06,15,rlc_y  ); ITB(0x07,15,rlc_y_a);
					ITB(0x08,15,rrc_y_b); ITB(0x09,15,rrc_y_c); ITB(0x0A,15,rrc_y_d); ITB(0x0B,15,rrc_y_e);
					ITB(0x0C,15,rrc_y_h); ITB(0x0D,15,rrc_y_l); ITB(0x0E,15,rrc_y  ); ITB(0x0F,15,rrc_y_a);
					ITB(0x10,15,rl_y_b ); ITB(0x11,15,rl_y_c ); ITB(0x12,15,rl_y_d ); ITB(0x13,15,rl_y_e );
					ITB(0x14,15,rl_y_h ); ITB(0x15,15,rl_y_l ); ITB(0x16,15,rl_y   ); ITB(0x17,15,rl_y_a );
					ITB(0x18,15,rr_y_b ); ITB(0x19,15,rr_y_c ); ITB(0x1A,15,rr_y_d ); ITB(0x1B,15,rr_y_e );
					ITB(0x1C,15,rr_y_h ); ITB(0x1D,15,rr_y_l ); ITB(0x1E,15,rr_y   ); ITB(0x1F,15,rr_y_a );
					ITB(0x20,15,sla_y_b); ITB(0x21,15,sla_y_c); ITB(0x22,15,sla_y_d); ITB(0x23,15,sla_y_e);
					ITB(0x24,15,sla_y_h); ITB(0x25,15,sla_y_l); ITB(0x26,15,sla_y  ); ITB(0x27,15,sla_y_a);
					ITB(0x28,15,sra_y_b); ITB(0x29,15,sra_y_c); ITB(0x2A,15,sra_y_d); ITB(0x2B,15,sra_y_e);
					ITB(0x2C,15,sra_y_h); ITB(0x2D,15,sra_y_l); ITB(0x2E,15,sra_y  ); ITB(0x2F,15,sra_y_a);
					ITB(0x30,15,sll_y_b); ITB(0x31,15,sll_y_c); ITB(0x32,15,sll_y_d); ITB(0x33,15,sll_y_e);
					ITB(0x34,15,sll_y_h); ITB(0x35,15,sll_y_l); ITB(0x36,15,sll_y  ); ITB(0x37,15,sll_y_a);
					ITB(0x38,15,srl_y_b); ITB(0x39,15,srl_y_c); ITB(0x3A,15,srl_y_d); ITB(0x3B,15,srl_y_e);
					ITB(0x3C,15,srl_y_h); ITB(0x3D,15,srl_y_l); ITB(0x3E,15,srl_y  ); ITB(0x3F,15,srl_y_a);
					case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: ITB(0x47,12,bit_0_y);
					case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: ITB(0x4F,12,bit_1_y);
					case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: ITB(0x57,12,bit_2_y);
					case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: ITB(0x5F,12,bit_3_y);
					case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: ITB(0x67,12,bit_4_y);
					case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: ITB(0x6F,12,bit_5_y);
					case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: ITB(0x77,12,bit_6_y);
					case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: ITB(0x7F,12,bit_7_y);
					ITB(0x80,15,res_0_y_b); ITB(0x81,15,res_0_y_c); ITB(0x82,15,res_0_y_d); ITB(0x83,15,res_0_y_e);
					ITB(0x84,15,res_0_y_h); ITB(0x85,15,res_0_y_l); ITB(0x86,15,res_0_y  ); ITB(0x87,15,res_0_y_a);
					ITB(0x88,15,res_1_y_b); ITB(0x89,15,res_1_y_c); ITB(0x8A,15,res_1_y_d); ITB(0x8B,15,res_1_y_e);
					ITB(0x8C,15,res_1_y_h); ITB(0x8D,15,res_1_y_l); ITB(0x8E,15,res_1_y  ); ITB(0x8F,15,res_1_y_a);
					ITB(0x90,15,res_2_y_b); ITB(0x91,15,res_2_y_c); ITB(0x92,15,res_2_y_d); ITB(0x93,15,res_2_y_e);
					ITB(0x94,15,res_2_y_h); ITB(0x95,15,res_2_y_l); ITB(0x96,15,res_2_y  ); ITB(0x97,15,res_2_y_a);
					ITB(0x98,15,res_3_y_b); ITB(0x99,15,res_3_y_c); ITB(0x9A,15,res_3_y_d); ITB(0x9B,15,res_3_y_e);
					ITB(0x9C,15,res_3_y_h); ITB(0x9D,15,res_3_y_l); ITB(0x9E,15,res_3_y  ); ITB(0x9F,15,res_3_y_a);
					ITB(0xA0,15,res_4_y_b); ITB(0xA1,15,res_4_y_c); ITB(0xA2,15,res_4_y_d); ITB(0xA3,15,res_4_y_e);
					ITB(0xA4,15,res_4_y_h); ITB(0xA5,15,res_4_y_l); ITB(0xA6,15,res_4_y  ); ITB(0xA7,15,res_4_y_a);
					ITB(0xA8,15,res_5_y_b); ITB(0xA9,15,res_5_y_c); ITB(0xAA,15,res_5_y_d); ITB(0xAB,15,res_5_y_e);
					ITB(0xAC,15,res_5_y_h); ITB(0xAD,15,res_5_y_l); ITB(0xAE,15,res_5_y  ); ITB(0xAF,15,res_5_y_a);
					ITB(0xB0,15,res_6_y_b); ITB(0xB1,15,res_6_y_c); ITB(0xB2,15,res_6_y_d); ITB(0xB3,15,res_6_y_e);
					ITB(0xB4,15,res_6_y_h); ITB(0xB5,15,res_6_y_l); ITB(0xB6,15,res_6_y  ); ITB(0xB7,15,res_6_y_a);
					ITB(0xB8,15,res_7_y_b); ITB(0xB9,15,res_7_y_c); ITB(0xBA,15,res_7_y_d); ITB(0xBB,15,res_7_y_e);
					ITB(0xBC,15,res_7_y_h); ITB(0xBD,15,res_7_y_l); ITB(0xBE,15,res_7_y  ); ITB(0xBF,15,res_7_y_a);
					ITB(0xC0,15,set_0_y_b); ITB(0xC1,15,set_0_y_c); ITB(0xC2,15,set_0_y_d); ITB(0xC3,15,set_0_y_e);
					ITB(0xC4,15,set_0_y_h); ITB(0xC5,15,set_0_y_l); ITB(0xC6,15,set_0_y  ); ITB(0xC7,15,set_0_y_a);
					ITB(0xC8,15,set_1_y_b); ITB(0xC9,15,set_1_y_c); ITB(0xCA,15,set_1_y_d); ITB(0xCB,15,set_1_y_e);
					ITB(0xCC,15,set_1_y_h); ITB(0xCD,15,set_1_y_l); ITB(0xCE,15,set_1_y  ); ITB(0xCF,15,set_1_y_a);
					ITB(0xD0,15,set_2_y_b); ITB(0xD1,15,set_2_y_c); ITB(0xD2,15,set_2_y_d); ITB(0xD3,15,set_2_y_e);
					ITB(0xD4,15,set_2_y_h); ITB(0xD5,15,set_2_y_l); ITB(0xD6,15,set_2_y  ); ITB(0xD7,15,set_2_y_a);
					ITB(0xD8,15,set_3_y_b); ITB(0xD9,15,set_3_y_c); ITB(0xDA,15,set_3_y_d); ITB(0xDB,15,set_3_y_e);
					ITB(0xDC,15,set_3_y_h); ITB(0xDD,15,set_3_y_l); ITB(0xDE,15,set_3_y  ); ITB(0xDF,15,set_3_y_a);
					ITB(0xE0,15,set_4_y_b); ITB(0xE1,15,set_4_y_c); ITB(0xE2,15,set_4_y_d); ITB(0xE3,15,set_4_y_e);
					ITB(0xE4,15,set_4_y_h); ITB(0xE5,15,set_4_y_l); ITB(0xE6,15,set_4_y  ); ITB(0xE7,15,set_4_y_a);
					ITB(0xE8,15,set_5_y_b); ITB(0xE9,15,set_5_y_c); ITB(0xEA,15,set_5_y_d); ITB(0xEB,15,set_5_y_e);
					ITB(0xEC,15,set_5_y_h); ITB(0xED,15,set_5_y_l); ITB(0xEE,15,set_5_y  ); ITB(0xEF,15,set_5_y_a);
					ITB(0xF0,15,set_6_y_b); ITB(0xF1,15,set_6_y_c); ITB(0xF2,15,set_6_y_d); ITB(0xF3,15,set_6_y_e);
					ITB(0xF4,15,set_6_y_h); ITB(0xF5,15,set_6_y_l); ITB(0xF6,15,set_6_y  ); ITB(0xF7,15,set_6_y_a);
					ITB(0xF8,15,set_7_y_b); ITB(0xF9,15,set_7_y_c); ITB(0xFA,15,set_7_y_d); ITB(0xFB,15,set_7_y_e);
					ITB(0xFC,15,set_7_y_h); ITB(0xFD,15,set_7_y_l); ITB(0xFE,15,set_7_y  ); ITB(0xFF,15,set_7_y_a);
				}
			break;
		}
		ICNT(z80cctmp);
	} while (z80->pref != S_NONE);
	z80->ie >>= 1;
}

// Forcing an interrupt without resetting the timer
void z80_interrupt() {
	// Waiting until any interrupt is possible
	while (z80->ie) z80_step();
	// Performing the step
	z80->iff1 = z80->iff2 = z80->hlt = 0;
	switch (z80->im) {
		// IM 0 - random values on the bus are simulated by randomly corrupting registers
		case 0: ICNT(13); ((char*)z80)[rand() % 28] = rand(); ((char*)z80)[rand() % 28] = rand(); break;
		// IM 1 - rst $38
		case 1: ICNT(13); rst_38(); break;
		// IM 2 - call bus_data + i * 256
		case 2: ICNT(19); PUSH_REG16(R_PC); READ_MEM16((R_I << 8) + emu->dbus, R_PC); break;
	}
}

// Calculating instruction LUTs
void z80_prepare() {
	int i, h, l, a, f;
	// Parity, logic
	for (i = 0; i < 0x100; i++) {
		for (h = 1, l = i; l > 0; l >>= 1) h ^= (l & 1);
		par[i] = h * F_P;
		log_f[i] = (i & (F_S | F_5 | F_3)) | (F_Z * (i == 0)) | par[i];
		inc_r[i] = ((i + 1) & 0x7f) | (i & 0x80);
	}
	// Arithmetic
	for (i = 0; i < 0x10000; i++) {
		h = i >> 8;
		l = i & 0xff;
		// INC: SZ!H!PNC **r*rV0-
		inc_f[i] = (l & F_C) | (h & (F_S | F_5 | F_3)) | (F_Z * (h == 0)) |
			(F_P * (h == 0x80)) | (F_H * ((h & 0x0f) == 0));
		// DEC: SZ!H!PNC **r*rV1-
		dec_f[i] = (l & F_C) | (h & (F_S | F_5 | F_3)) | (F_Z * (h == 0)) |
			(F_P * (h == 0x7f)) | (F_H * ((h & 0x0f) == 0x0f)) | F_N;
		// RLCA, RRCA, RLA, RRA: SZ!H!PNC --r0r-0*
		a = ((h << 1) | (h >> 7)) & 0xff;
		f = (l & (F_S | F_Z | F_P)) | (a & (F_5 | F_3 | F_C));
		rlca_lut[i] = (a << 8) + f;
		a = ((h >> 1) | (h << 7)) & 0xff;
		f = (l & (F_S | F_Z | F_P)) | (a & (F_5 | F_3)) | (h & F_C);
		rrca_lut[i] = (a << 8) + f;
		a = ((h << 1) | (l & F_C)) & 0xff;
		f = (l & (F_S | F_Z | F_P)) | (a & (F_5 | F_3)) | ((h >> 7) & F_C);
		rla_lut[i] = (a << 8) + f;
		a = ((h >> 1) | (l << 7)) & 0xff;
		f = (l & (F_S | F_Z | F_P)) | (a & (F_5 | F_3)) | (h & F_C);
		rra_lut[i] = (a << 8) + f;
		// CPL: SZ!H!PNC --r1r-1-
		a = (~h) & 0xff;
		f = (l & (F_S | F_Z | F_P | F_C)) | (a & (F_5 | F_3)) | F_N | F_H;
		cpl_lut[i] = (a << 8) + f;
		// SCF: SZ!H!PNC --|0|-01
		scf_lut[i] = (h << 8) + ((l & (F_S | F_Z | F_P | F_5 | F_3)) | (h & (F_5 | F_3)) | F_C);
		// CCF: SZ!H!PNC --|c|-0*
		ccf_lut[i] = (h << 8) + (((l ^ 1) & (F_S | F_Z | F_P | F_C | F_5 | F_3)) | (h & (F_5 | F_3)) | ((l << 4) & F_H));
		// ADD: SZ!H!PNC **r*rV0*
		a = (h + l) & 0xff;
		f = (int8_t)h + (int8_t)l;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | ((a ^ h ^ l) & F_H) |
			(F_P * (f != (int8_t)f)) | ((h + l) >> 8);
		adc_lut[i] = (a << 8) + f;
		// ADD+1
		a = (h + l + 1) & 0xff;
		f = (int8_t)h + (int8_t)l + 1;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | ((a ^ h ^ l) & F_H) |
			(F_P * (f != (int8_t)f)) | ((h + l + 1) >> 8);
		adc_lut[i + 0x10000] = (a << 8) + f;
		// SUB: SZ!H!PNC **r*rV1*
		a = (h - l) & 0xff;
		f = (int8_t)h - (int8_t)l;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | ((a ^ h ^ l) & F_H) |
			(F_P * (f != (int8_t)f)) | (F_C * (l > h)) | F_N;
		sbc_lut[i] = (a << 8) + f;
		// SUB-1
		a = (h - l - 1) & 0xff;
		f = (int8_t)h - (int8_t)l - 1;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | ((a ^ h ^ l) & F_H) |
			(F_P * (f != (int8_t)f)) | (F_C * (l + 1 > h)) | F_N;
		sbc_lut[i + 0x10000] = (a << 8) + f;
		// IN: SZ!H!PNC **r*rP0-
		in_f[i] = (l & F_C) | (h & (F_S | F_5 | F_3)) | (F_Z * (h == 0)) | par[h];
		// RLC, RRC, RL, RR, SLA, SRA, SLL, SRL: SZ!H!PNC **r0rP0*
		a = ((h << 1) | (h >> 7)) & 0xff;
		f = (a & (F_S | F_5 | F_3 | F_C)) | (F_Z * (a == 0)) | par[a];
		rlc_lut[i] = (a << 8) + f;
		a = ((h >> 1) | (h << 7)) & 0xff;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | (h & F_C);
		rrc_lut[i] = (a << 8) + f;
		a = ((h << 1) | (l & F_C)) & 0xff;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | ((h >> 7) & F_C);
		rl_lut[i] = (a << 8) + f;
		a = ((h >> 1) | (l << 7)) & 0xff;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | (h & F_C);
		rr_lut[i] = (a << 8) + f;
		a = (h << 1) & 0xff;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | ((h >> 7) & F_C);
		sla_lut[i] = (a << 8) + f;
		a |= 1;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | ((h >> 7) & F_C);
		sll_lut[i] = (a << 8) + f;
		a = (h >> 1) & 0xff;
		f = (a & (F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | (h & F_C);
		srl_lut[i] = (a << 8) + f;
		a |= h & F_S;
		f = (a & (F_S | F_5 | F_3)) | (F_Z * (a == 0)) | par[a] | (h & F_C);
		sra_lut[i] = (a << 8) + f;
		// DAA: SZ!H!PNC **r*rP-*
		a = h;
		if (l & F_N) {
			if ((l & F_H) || ((h & 0x0f) > 9)) a -= 0x06;
			if ((l & F_C) || (h > 0x99)) a -= 0x60;
		} else {
			if ((l & F_H) || ((h & 0x0f) > 9)) a += 0x06;
			if ((l & F_C) || (h > 0x99)) a += 0x60;
		}
		f = log_f[a & 0xff] | (h > 0x99) | (l & (F_N | F_C)) | ((h ^ a) & F_H);
		daa_lut[i] = ((a & 0xff) << 8) + f;
	}
}

// Power on defaults
void z80_reset() {
	R_AF = R_SP = 0xffff;
	R_PC = R_R = R_I = 0;
	R_BC = rand(); R_DE = rand(); R_HL = rand();
	R_IX = rand(); R_IY = rand(); R_AFS = rand();
	R_BCS = rand(); R_DES = rand(); R_HLS = rand();
	z80->hlt = z80->iff1 = z80->iff2 = z80->pref = z80->ie = 0;
	z80->im = 1;
}

// Macros for identical serialisation in both directions
#define lZ80C(x) x = fgetc(fp)
#define lZ80W(x) \
	x = fgetc(fp) << 8; \
	x += fgetc(fp)

#define sZ80C(x) fputc(x, fp)
#define sZ80W(x) \
	fputc(x >> 8, fp); \
	fputc(x, fp)

#define SERZ80(ls) \
	ls ## Z80W(R_AF); ls ## Z80W(R_BC); ls ## Z80W(R_DE); ls ## Z80W(R_HL); \
	ls ## Z80W(R_IX); ls ## Z80W(R_IY); ls ## Z80W(z80->idx); \
	ls ## Z80W(R_SP); ls ## Z80W(R_PC); ls ## Z80C(R_R); ls ## Z80C(R_I); \
	ls ## Z80W(R_AFS); ls ## Z80W(R_BCS); ls ## Z80W(R_DES); ls ## Z80W(R_HLS); \
	ls ## Z80C(z80->im); ls ## Z80C(z80->hlt); ls ## Z80C(z80->iff1); ls ## Z80C(z80->iff2); \
	ls ## Z80C(z80->pref); ls ## Z80C(z80->ie);

// Save the current CPU state
int z80_save(FILE *fp) {
	SERZ80(s);
	return 0;
}

// Load the current CPU state
int z80_load(FILE *fp) {
	SERZ80(l);
	return 0;
}


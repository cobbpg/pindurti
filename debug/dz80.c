#include <stdio.h>
#include <string.h>
#include "../hw/z80.h"

char debug_ins[64];

// B: byte, W: word, A: address (word), R: relative address (byte), P: port (byte), X: ix+index (byte)
char oc_base[256][12] = {
	"nop",       "ld bc,W",   "ld (bc),a", "inc bc",     "inc b",     "dec b",     "ld b,B",     "rlca",
	"ex af,af'", "add hl,bc", "ld a,(bc)", "dec bc",     "inc c",     "dec c",     "ld c,B",     "rrca",
	"djnz R",    "ld de,W",   "ld (de),a", "inc de",     "inc d",     "dec d",     "ld d,B",     "rla",
	"jr R",      "add hl,de", "ld a,(de)", "dec de",     "inc e",     "dec e",     "ld e,B",     "rra",
	"jr nz,R",   "ld hl,W",   "ld (A),hl", "inc hl",     "inc h",     "dec h",     "ld h,B",     "daa",
	"jr z,R",    "add hl,hl", "ld hl,(A)", "dec hl",     "inc l",     "dec l",     "ld l,B",     "cpl",
	"jr nc,R",   "ld sp,W",   "ld (A),a",  "inc sp",     "inc (hl)",  "dec (hl)",  "ld (hl),B",  "scf",
	"jr c,R",    "add hl,sp", "ld a,(A)",  "dec sp",     "inc a",     "dec a",     "ld a,B",     "ccf",
	"ld b,b",    "ld b,c",    "ld b,d",    "ld b,e",     "ld b,h",    "ld b,l",    "ld b,(hl)",  "ld b,a",
	"ld c,b",    "ld c,c",    "ld c,d",    "ld c,e",     "ld c,h",    "ld c,l",    "ld c,(hl)",  "ld c,a",
	"ld d,b",    "ld d,c",    "ld d,d",    "ld d,e",     "ld d,h",    "ld d,l",    "ld d,(hl)",  "ld d,a",
	"ld e,b",    "ld e,c",    "ld e,d",    "ld e,e",     "ld e,h",    "ld e,l",    "ld e,(hl)",  "ld e,a",
	"ld h,b",    "ld h,c",    "ld h,d",    "ld h,e",     "ld h,h",    "ld h,l",    "ld h,(hl)",  "ld h,a",
	"ld l,b",    "ld l,c",    "ld l,d",    "ld l,e",     "ld l,h",    "ld l,l",    "ld l,(hl)",  "ld l,a",
	"ld (hl),b", "ld (hl),c", "ld (hl),d", "ld (hl),e",  "ld (hl),h", "ld (hl),l", "halt",       "ld (hl),a",
	"ld a,b",    "ld a,c",    "ld a,d",    "ld a,e",     "ld a,h",    "ld a,l",    "ld a,(hl)",  "ld a,a",
	"add a,b",   "add a,c",   "add a,d",   "add a,e",    "add a,h",   "add a,l",   "add a,(hl)", "add a,a",
	"adc a,b",   "adc a,c",   "adc a,d",   "adc a,e",    "adc a,h",   "adc a,l",   "adc a,(hl)", "adc a,a",
	"sub b",     "sub c",     "sub d",     "sub e",      "sub h",     "sub l",     "sub (hl)",   "sub a",
	"sbc a,b",   "sbc a,c",   "sbc a,d",   "sbc a,e",    "sbc a,h",   "sbc a,l",   "sbc a,(hl)", "sbc a,a",
	"and b",     "and c",     "and d",     "and e",      "and h",     "and l",     "and (hl)",   "and a",
	"xor b",     "xor c",     "xor d",     "xor e",      "xor h",     "xor l",     "xor (hl)",   "xor a",
	"or b",      "or c",      "or d",      "or e",       "or h",      "or l",      "or (hl)",    "or a",
	"cp b",      "cp c",      "cp d",      "cp e",       "cp h",      "cp l",      "cp (hl)",    "cp a",
	"ret nz",    "pop bc",    "jp nz,A",   "jp A",       "call nz,A", "push bc",   "add a,B",    "rst $00",
	"ret z",     "ret",       "jp z,A",    "$cb",        "call z,A",  "call A",    "adc a,B",    "rst $08",
	"ret nc",    "pop de",    "jp nc,A",   "out (P),a",  "call nc,A", "push de",   "sub B",      "rst $10",
	"ret c",     "exx",       "jp c,A",    "in a,(P)",   "call c,A",  "$dd",       "sbc a,B",    "rst $18",
	"ret po",    "pop hl",    "jp po,A",   "ex (sp),hl", "call po,A", "push hl",   "and B",      "rst $20",
	"ret pe",    "jp (hl)",   "jp pe,A",   "ex de,hl",   "call pe,A", "$ed",       "xor B",      "rst $28",
	"ret p",     "pop af",    "jp p,A",    "di",         "call p,A",  "push af",   "or B",       "rst $30",
	"ret m",     "ld sp,hl",  "jp m,A",    "ei",         "call m,A",  "$fd",       "cp B",       "rst $38"
};

char oc_ix[256][12] = {
	"nop",       "ld bc,W",   "ld (bc),a", "inc bc",     "inc b",     "dec b",     "ld b,B",     "rlca",
	"ex af,af'", "add ix,bc", "ld a,(bc)", "dec bc",     "inc c",     "dec c",     "ld c,B",     "rrca",
	"djnz R",    "ld de,W",   "ld (de),a", "inc de",     "inc d",     "dec d",     "ld d,B",     "rla",
	"jr R",      "add ix,de", "ld a,(de)", "dec de",     "inc e",     "dec e",     "ld e,B",     "rra",
	"jr nz,R",   "ld ix,W",   "ld (A),ix", "inc ix",     "inc ixh",   "dec ixh",   "ld ixh,B",   "daa",
	"jr z,R",    "add ix,ix", "ld ix,(A)", "dec ix",     "inc ixl",   "dec ixl",   "ld ixl,B",   "cpl",
	"jr nc,R",   "ld sp,W",   "ld (A),a",  "inc sp",     "inc (X)",   "dec (X)",   "ld (X),B",   "scf",
	"jr c,R",    "add ix,sp", "ld a,(A)",  "dec sp",     "inc a",     "dec a",     "ld a,B",     "ccf",
	"ld b,b",    "ld b,c",    "ld b,d",    "ld b,e",     "ld b,ixh",  "ld b,ixl",  "ld b,(X)",  "ld b,a",
	"ld c,b",    "ld c,c",    "ld c,d",    "ld c,e",     "ld c,ixh",  "ld c,ixl",  "ld c,(X)",  "ld c,a",
	"ld d,b",    "ld d,c",    "ld d,d",    "ld d,e",     "ld d,ixh",  "ld d,ixl",  "ld d,(X)",  "ld d,a",
	"ld e,b",    "ld e,c",    "ld e,d",    "ld e,e",     "ld e,ixh",  "ld e,ixl",  "ld e,(X)",  "ld e,a",
	"ld ixh,b",  "ld ixh,c",  "ld ixh,d",  "ld ixh,e",   "ld ixh,ixh","ld ixh,ixl","ld h,(X)",   "ld ixh,a",
	"ld ixl,b",  "ld ixl,c",  "ld ixl,d",  "ld ixl,e",   "ld ixl,ixh","ld ixl,ixl","ld l,(X)",   "ld ixl,a",
	"ld (X),b",  "ld (X),c",  "ld (X),d",  "ld (X),e",   "ld (X),h",  "ld (X),l",  "halt",       "ld (X),a",
	"ld a,b",    "ld a,c",    "ld a,d",    "ld a,e",     "ld a,ixh",  "ld a,ixl",  "ld a,(X)",   "ld a,a",
	"add a,b",   "add a,c",   "add a,d",   "add a,e",    "add a,ixh", "add a,ixl", "add a,(X)",  "add a,a",
	"adc a,b",   "adc a,c",   "adc a,d",   "adc a,e",    "adc a,ixh", "adc a,ixl", "adc a,(X)",  "adc a,a",
	"sub b",     "sub c",     "sub d",     "sub e",      "sub ixh",   "sub ixl",   "sub (X)",    "sub a",
	"sbc a,b",   "sbc a,c",   "sbc a,d",   "sbc a,e",    "sbc a,ixh", "sbc a,ixl", "sbc a,(X)",  "sbc a,a",
	"and b",     "and c",     "and d",     "and e",      "and ixh",   "and ixl",   "and (X)",    "and a",
	"xor b",     "xor c",     "xor d",     "xor e",      "xor ixh",   "xor ixl",   "xor (X)",    "xor a",
	"or b",      "or c",      "or d",      "or e",       "or ixh",    "or ixl",    "or (X)",     "or a",
	"cp b",      "cp c",      "cp d",      "cp e",       "cp ixh",    "cp ixl",    "cp (X)",     "cp a",
	"ret nz",    "pop bc",    "jp nz,A",   "jp A",       "call nz,A", "push bc",   "add a,B",    "rst $00",
	"ret z",     "ret",       "jp z,A",    "$cb",        "call z,A",  "call A",    "adc a,B",    "rst $08",
	"ret nc",    "pop de",    "jp nc,A",   "out (P),a",  "call nc,A", "push de",   "sub B",      "rst $10",
	"ret c",     "exx",       "jp c,A",    "in a,(P)",   "call c,A",  "$dd",       "sbc a,B",    "rst $18",
	"ret po",    "pop ix",    "jp po,A",   "ex (sp),ix", "call po,A", "push ix",   "and B",      "rst $20",
	"ret pe",    "jp (ix)",   "jp pe,A",   "ex de,hl",   "call pe,A", "$ed",       "xor B",      "rst $28",
	"ret p",     "pop af",    "jp p,A",    "di",         "call p,A",  "push af",   "or B",       "rst $30",
	"ret m",     "ld sp,ix",  "jp m,A",    "ei",         "call m,A",  "$fd",       "cp B",       "rst $38"
};

char oc_ed[128][12] = {
	"in b,(c)",  "out (c),b", "sbc hl,bc", "ld (A),bc",  "neg",       "retn",      "im 0",       "ld i,a",
	"in c,(c)",  "out (c),c", "adc hl,bc", "ld bc,(A)",  "neg",       "reti",      "im 0",       "ld r,a",
	"in d,(c)",  "out (c),d", "sbc hl,de", "ld (A),de",  "neg",       "retn",      "im 1",       "ld a,i",
	"in e,(c)",  "out (c),e", "adc hl,de", "ld de,(A)",  "neg",       "reti",      "im 2",       "ld a,r",
	"in h,(c)",  "out (c),h", "sbc hl,hl", "ld (A),hl",  "neg",       "retn",      "im 0",       "rrd",
	"in l,(c)",  "out (c),l", "adc hl,hl", "ld hl,(A)",  "neg",       "reti",      "im 0",       "rld",
	"in f,(c)",  "out (c),?", "sbc hl,sp", "ld (A),sp",  "neg",       "retn",      "im 1",       "nop",
	"in a,(c)",  "out (c),a", "adc hl,sp", "ld sp,(A)",  "neg",       "reti",      "im 2",       "nop",
	"nop",       "nop",       "nop",       "nop",        "nop",       "nop",       "nop",        "nop",
	"nop",       "nop",       "nop",       "nop",        "nop",       "nop",       "nop",        "nop",
	"nop",       "nop",       "nop",       "nop",        "nop",       "nop",       "nop",        "nop",
	"nop",       "nop",       "nop",       "nop",        "nop",       "nop",       "nop",        "nop",
	"ldi",       "cpi",       "ini",       "outi",       "nop",       "nop",       "nop",        "nop",
	"ldd",       "cpd",       "ind",       "outd",       "nop",       "nop",       "nop",        "nop",
	"ldir",      "cpir",      "inir",      "otir",       "nop",       "nop",       "nop",        "nop",
	"lddr",      "cpdr",      "indr",      "otdr",       "nop",       "nop",       "nop",        "nop"
};

char oc_shift[8][4] = { "rlc", "rrc", "rl", "rr", "sla", "sra", "sll", "srl" };
char oc_op8[8][5] = { "b", "c", "d", "e", "h", "l", "(hl)", "a" };

void insert_hex(char *pos, int len) {
	memmove(pos + 5, pos + 1, 16);
	pos[0] = '$';
	pos[1] = '%';
	pos[2] = '0';
	pos[3] = '0' + len;
	pos[4] = 'x';
}

int opcode_name(WORD adr) {
	BYTE oc[4];
	char tmp[64];
	int i, l = 1, par1 = -1, par2 = -1;

	for (i = 0; i < 4; i++) oc[i] = z80_acc(adr + i);

	if (oc[0] == 0xcb) {
		l = 2;
		switch (oc[1] >> 6) {
			case 0x00: sprintf(tmp, "%s ", oc_shift[(oc[1] >> 3) & 0x07]); break;
			case 0x01: sprintf(tmp, "bit"); break;
			case 0x02: sprintf(tmp, "res"); break;
			case 0x03: sprintf(tmp, "set"); break;
		}
		if (oc[1] >= 0x40) sprintf(tmp + 3, " %d,", (oc[1] >> 3) & 0x07);
		sprintf(tmp + strlen(tmp), "%s", oc_op8[oc[1] & 0x07]);

	} else if (((oc[0] == 0xdd) || (oc[0] == 0xfd)) && (oc[1] == 0xcb)) {
		l = 2;
		switch (oc[3] >> 6) {
			case 0x00: sprintf(tmp, "%s ", oc_shift[(oc[3] >> 3) & 0x07]); break;
			case 0x01: sprintf(tmp, "bit"); break;
			case 0x02: sprintf(tmp, "res"); break;
			case 0x03: sprintf(tmp, "set"); break;
		}
		if (oc[3] >= 0x40) sprintf(tmp + 3, " %d,", (oc[3] >> 3) & 0x07);
		if ((oc[3] & 0x07) == 0x06) sprintf(tmp + strlen(tmp), "(X)");
		else sprintf(tmp + strlen(tmp), "(X)->%s", oc_op8[oc[3] & 0x07]);

	} else {
		strcpy(tmp, oc_base[oc[0]]);
		if (((oc[0] == 0xdd) || (oc[0] == 0xfd)) && (oc[1] != 0xdd) && (oc[1] != 0xed) && (oc[1] != 0xfd)) {
			strcpy(tmp, oc_ix[oc[1]]);
			l++;
		}
		if (oc[0] == 0xed) {
			if ((oc[1] < 0x40) || (oc[1] >= 0xc0))
				switch (oc[1]) {
				case 0x00: strcpy(tmp, "pti_dump_registers"); break;
				case 0x01: strcpy(tmp, "pti_printf"); break;
				default: strcpy(tmp, "nop");
				}
			else strcpy(tmp, oc_ed[oc[1] - 0x40]);
			l++;
		}

		for (i = 0; tmp[i]; i++)
			switch (tmp[i]) {
				// Address (word)
				case 'A':
					insert_hex(tmp + i, 4);
					par1 = oc[l] + (oc[l + 1] << 8);
					l += 2;
				break;
				// Byte
				case 'B':
					insert_hex(tmp + i, 2);
					par1 = oc[l];
					l += 1;
				break;
				// Port (byte)
				case 'P':
					insert_hex(tmp + i, 2);
					par1 = oc[l];
					l += 1;
				break;
				// Relative address (byte)
				case 'R':
					insert_hex(tmp + i, 4);
					par1 = adr + 2 + (char)oc[l];
					l += 1;
				break;
				// Word
				case 'W':
					insert_hex(tmp + i, 4);
					par1 = oc[l] + (oc[l + 1] << 8);
					l += 2;
				break;
			}
	}

	for (i = 0; tmp[i]; i++)
		if (tmp[i] == 'X') {
			memmove(tmp + i + 4, tmp + i + 1, 16);
			tmp[i] = 'i';
			tmp[i + 1] = 'x';
			tmp[i + 2] = ((char)oc[l] < 0) ? '-' : '+';
			insert_hex(tmp + i + 3, 2);
			if (par1 != -1) {
				par1 = ((char)oc[2] < 0) ? -(char)oc[2] : oc[2];
				par2 = oc[3];
				tmp[i + 2] = ((char)oc[2] < 0) ? '-' : '+';
			} else par1 = ((char)oc[l] < 0) ? -(char)oc[l] : oc[l];
			l++;
		}

	if (((oc[0] == 0xdd) || (oc[0] == 0xfd)) && (oc[1] == 0xcb)) l = 4;
	if (oc[0] == 0xfd) for (i = 0; tmp[i]; i++) if ((tmp[i] == 'i') && (tmp[i + 1] == 'x')) tmp[i + 1] = 'y';

	memset(debug_ins, ' ', 9);
	if (par2 != -1) sprintf(debug_ins + 9, tmp, par1, par2);
	else if (par1 != -1) sprintf(debug_ins + 9, tmp, par1);
	else sprintf(debug_ins + 9, tmp);
	for (i = 0; i < l; i++) sprintf(tmp + i * 2, "%02x", oc[i]);
	memcpy(debug_ins, tmp, strlen(tmp));

	return l;
}

WORD prev_address(WORD a) {
	int i;
	
	for (i = a - 12; i + opcode_name(i) < a; i += opcode_name(i));
//		if (opcode_name(a - i) == i) return a - i;
	return i;
}


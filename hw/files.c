#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "files.h"

#define KNOWN_SIGNATURES 39
#define KNOWN_FILETYPES 21

// Checksums of known ROMs (only the first 0x10000 bytes)
ROM_SIGNATURE rom_sig[KNOWN_SIGNATURES] = {

	{ 0x401E4AD9, 0x20000, FILE_ROM_IMAGE_82_16_0, "TI-82 16.0" },
	{ 0xF56AA959, 0x20000, FILE_ROM_IMAGE_82_17_0, "TI-82 17.0" },
	{ 0x33D491CB, 0x20000, FILE_ROM_IMAGE_82_18_0, "TI-82 18.0" },
	{ 0xAFC4AAB9, 0x20000, FILE_ROM_IMAGE_82_19_0, "TI-82 19.0" },
	{ 0x6FE33E8C, 0x20000, FILE_ROM_IMAGE_82_19_006, "TI-82 19.006" },

	{ 0xA0735C95, 0x40000, FILE_ROM_IMAGE_83_1_02, "TI-83 1.02" },
	{ 0xC96F25DE, 0x40000, FILE_ROM_IMAGE_83_1_03, "TI-83 1.03" },
	{ 0xBBBA9851, 0x40000, FILE_ROM_IMAGE_83_1_04, "TI-83 1.04" },
	{ 0x77F53171, 0x40000, FILE_ROM_IMAGE_83_1_06, "TI-83 1.06" },
	{ 0xABCB7B39, 0x40000, FILE_ROM_IMAGE_83_1_07, "TI-83 1.07" },
	{ 0x67921DD4, 0x40000, FILE_ROM_IMAGE_83_1_08, "TI-83 1.08" },
	{ 0x9952A94F, 0x40000, FILE_ROM_IMAGE_83_1_10, "TI-83 1.10" },
	{ 0xBB61ED72, 0x40000, FILE_ROM_IMAGE_83_1_10001, "TI-83 1.10001" },

	{ 0x7D29D3C6, 0x80000, FILE_ROM_IMAGE_83P_1_03, "TI-83+ 1.03" },
	{ 0x631DCE08, 0x80000, FILE_ROM_IMAGE_83P_1_10, "TI-83+ 1.10" },
	{ 0xEEA0BC9C, 0x80000, FILE_ROM_IMAGE_83P_1_12, "TI-83+ 1.12" },
	{ 0xE82550BC, 0x80000, FILE_ROM_IMAGE_83P_1_13, "TI-83+ 1.13" },
	{ 0x930FEAB8, 0x80000, FILE_ROM_IMAGE_83P_1_14, "TI-83+ 1.14" },
	{ 0x4BD49F58, 0x80000, FILE_ROM_IMAGE_83P_1_15, "TI-83+ 1.15" },
	{ 0x4BFCAD59, 0x80000, FILE_ROM_IMAGE_83P_1_16, "TI-83+ 1.16" },
	{ 0xB46DDA82, 0x80000, FILE_ROM_IMAGE_83P_1_18, "TI-83+ 1.18" },

	{ 0x1B1A4C53, 0x20000, FILE_ROM_IMAGE_85_3_0A, "TI-85 3.0A" },
	{ 0x4F16D661, 0x20000, FILE_ROM_IMAGE_85_4_0 , "TI-85 4.0" },
	{ 0x7E6E2496, 0x20000, FILE_ROM_IMAGE_85_4_0b, "TI-85 4.0" },
	{ 0xDF1719AB, 0x20000, FILE_ROM_IMAGE_85_5_0 , "TI-85 5.0" },
	{ 0xEE6FEB5C, 0x20000, FILE_ROM_IMAGE_85_5_0b, "TI-85 5.0" },
	{ 0x2E67AA3F, 0x20000, FILE_ROM_IMAGE_85_6_0 , "TI-85 6.0" },
	{ 0x1F1F58C8, 0x20000, FILE_ROM_IMAGE_85_6_0b, "TI-85 6.0" },
	{ 0x9B7C16E9, 0x20000, FILE_ROM_IMAGE_85_8_0 , "TI-85 8.0" },
	{ 0xAA04E41E, 0x20000, FILE_ROM_IMAGE_85_8_0b, "TI-85 8.0" },
	{ 0xB43D064E, 0x20000, FILE_ROM_IMAGE_85_9_0 , "TI-85 9.0" },
	{ 0x8545F4B9, 0x20000, FILE_ROM_IMAGE_85_9_0b, "TI-85 9.0" },
	{ 0x474736B9, 0x20000, FILE_ROM_IMAGE_85_10_0 , "TI-85 10.0" },
	{ 0x763FC44E, 0x20000, FILE_ROM_IMAGE_85_10_0b, "TI-85 10.0" },

	{ 0xB07AB446, 0x40000, FILE_ROM_IMAGE_86_1_2, "TI-86 1.2" },
	{ 0xDAF86E8A, 0x40000, FILE_ROM_IMAGE_86_1_3, "TI-86 1.3" },
	{ 0xFAA0A9EE, 0x40000, FILE_ROM_IMAGE_86_1_4, "TI-86 1.4" },
	{ 0xBAD002F0, 0x40000, FILE_ROM_IMAGE_86_1_5, "TI-86 1.5" },
	{ 0x129A4981, 0x40000, FILE_ROM_IMAGE_86_1_6, "TI-86 1.6" }

};

// Beginnings of known file types
FILE_SIGNATURE file_sig[KNOWN_FILETYPES] = {

	{ "VTIv2.0 TI-82 version 16.0",      FILE_ROM_IMAGE_82_16_0 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.0 TI-82 version 17.0",      FILE_ROM_IMAGE_82_17_0 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.0 TI-82 version 18.0",      FILE_ROM_IMAGE_82_18_0 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.0 TI-82 version 19.0",      FILE_ROM_IMAGE_82_19_0 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.02",      FILE_ROM_IMAGE_83_1_02 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.03",      FILE_ROM_IMAGE_83_1_03 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.04",      FILE_ROM_IMAGE_83_1_04 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.05",      FILE_ROM_IMAGE_83_1_05 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.06",      FILE_ROM_IMAGE_83_1_06 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.07",      FILE_ROM_IMAGE_83_1_07 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.08",      FILE_ROM_IMAGE_83_1_08 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.09",      FILE_ROM_IMAGE_83_1_09 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 version 1.10",      FILE_ROM_IMAGE_83_1_10 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 Plus version 1.03", FILE_ROM_IMAGE_83P_1_03 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 Plus version 1.10", FILE_ROM_IMAGE_83P_1_10 | FILE_TYPE_VTI_SAV },
	{ "VTIv2.1 TI-83 Plus version 1.12", FILE_ROM_IMAGE_83P_1_12 | FILE_TYPE_VTI_SAV },

	{ "**TI82**",                        FILE_MODEL_82 | FILE_TYPE_CALC },
	{ "**TI83**",                        FILE_MODEL_83 | FILE_TYPE_CALC },
	{ "**TI83F*",                        FILE_MODEL_83P | FILE_TYPE_CALC },
	{ "**TIFL**",                        FILE_MODEL_83P | FILE_TYPE_APP },
	{ ":020000020000FC",                 FILE_MODEL_83P | FILE_TYPE_APP }

};

// Determine file type and version ID
int detect_file_type(char *fname) {
	FILE *fp;
	char tmp[0x10000];
	int ftype = FILE_UNKNOWN, fsize;

	fp = fopen(fname, "rb");
	if (fp) {
		int i;
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		rewind(fp);
		fread(tmp, (fsize >= 0x10000) ? 0x10000 : 0x100, 1, fp);
		for (i = 0; i < KNOWN_FILETYPES && ftype == FILE_UNKNOWN; i++)
			if (!strncmp(tmp, file_sig[i].idstr, strlen(file_sig[i].idstr)))
				ftype = file_sig[i].type;
		if (ftype == FILE_UNKNOWN) {
			int crc;
			crc = calculate_crc32(tmp, 0x10000);
			for (i = 0; i < KNOWN_SIGNATURES && ftype == FILE_UNKNOWN; i++)
				if (crc == rom_sig[i].crc && fsize >= rom_sig[i].size)
					ftype = rom_sig[i].ver;
			// Custom 83 plus OS
			if ((ftype == FILE_UNKNOWN) && (fsize == 0x80000)) ftype = FILE_MODEL_83P;
			if (ftype != FILE_UNKNOWN) ftype |= FILE_TYPE_ROM;
		}
		fclose(fp);
	}

	return ftype;
}

// String representation of ROM version
char *rom_version_name(int ver) {
	int i;
	
	for (i = 0; i < KNOWN_SIGNATURES; i++)
		if (rom_sig[i].ver == (int)(ver & ~FILE_TYPE_MASK)) return rom_sig[i].name;
	return (char*)NULL;
}

// A naive CRC32 implementation
int calculate_crc32(unsigned char *tmp, int len) {
	unsigned char tb;
	int i, j, crc;

	if (len < 4) return -1;

	crc = ~((tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3]);
	for (i = 4; i < len; i++) {
		tb = tmp[i];
		for (j = 0; j < 8; j++) {
			crc = ((crc << 1) | (tb >> 7)) ^ ((crc & 0x80000000) ? 0x04c11db7 : 0);
			tb <<= 1;
		}
	}

	return crc;
}


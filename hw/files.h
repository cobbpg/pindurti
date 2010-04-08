#ifndef _files
#define _files

#define FILE_UNKNOWN 0

#define FILE_TYPE_MASK    0xFF000000
#define FILE_TYPE_ROM     0x01000000
#define FILE_TYPE_CALC    0x02000000
#define FILE_TYPE_APP     0x03000000
#define FILE_TYPE_VTI_SAV 0x10000000

#define FILE_MODEL_MASK 0xFF0000
#define FILE_MODEL_82   0x820000
#define FILE_MODEL_82b  0xA20000
#define FILE_MODEL_83   0x830000
#define FILE_MODEL_83P  0x840000
#define FILE_MODEL_83SE 0xA40000
#define FILE_MODEL_84P  0xB40000
#define FILE_MODEL_84SE 0xC40000
#define FILE_MODEL_85   0x850000
#define FILE_MODEL_86   0x860000

#define FILE_PROT_MASK 0x0F0000
#define FILE_PROT_82   0x020000
#define FILE_PROT_83   0x030000
#define FILE_PROT_83P  0x040000
#define FILE_PROT_85   0x050000
#define FILE_PROT_86   0x060000

#define FILE_ROM_MASK 0xFFFF00

#define FILE_ROM_IMAGE_82_16_0   0x821600
#define FILE_ROM_IMAGE_82_17_0   0x821700
#define FILE_ROM_IMAGE_82_18_0   0x821800
#define FILE_ROM_IMAGE_82_19_0   0x821900
#define FILE_ROM_IMAGE_82_19_006 0xA21900

#define FILE_ROM_IMAGE_83_1_02    0x830200
#define FILE_ROM_IMAGE_83_1_03    0x830300
#define FILE_ROM_IMAGE_83_1_04    0x830400
#define FILE_ROM_IMAGE_83_1_05    0x830500
#define FILE_ROM_IMAGE_83_1_06    0x830600
#define FILE_ROM_IMAGE_83_1_07    0x830700
#define FILE_ROM_IMAGE_83_1_08    0x830800
#define FILE_ROM_IMAGE_83_1_09    0x830900
#define FILE_ROM_IMAGE_83_1_10    0x831000
#define FILE_ROM_IMAGE_83_1_10001 0x831001

#define FILE_ROM_IMAGE_83P_1_03 0x840300
#define FILE_ROM_IMAGE_83P_1_10 0x841000
#define FILE_ROM_IMAGE_83P_1_11 0x841100
#define FILE_ROM_IMAGE_83P_1_12 0x841200
#define FILE_ROM_IMAGE_83P_1_13 0x841300
#define FILE_ROM_IMAGE_83P_1_14 0x841400
#define FILE_ROM_IMAGE_83P_1_15 0x841500
#define FILE_ROM_IMAGE_83P_1_16 0x841600
#define FILE_ROM_IMAGE_83P_1_17 0x841700
#define FILE_ROM_IMAGE_83P_1_18 0x841800

#define FILE_ROM_IMAGE_85_3_0A  0x850301
#define FILE_ROM_IMAGE_85_4_0   0x850400
#define FILE_ROM_IMAGE_85_4_0b  0x850401
#define FILE_ROM_IMAGE_85_5_0   0x850500
#define FILE_ROM_IMAGE_85_5_0b  0x850501
#define FILE_ROM_IMAGE_85_6_0   0x850600
#define FILE_ROM_IMAGE_85_6_0b  0x850601
#define FILE_ROM_IMAGE_85_7_0   0x850700
#define FILE_ROM_IMAGE_85_7_0b  0x850701
#define FILE_ROM_IMAGE_85_8_0   0x850800
#define FILE_ROM_IMAGE_85_8_0b  0x850801
#define FILE_ROM_IMAGE_85_9_0   0x850900
#define FILE_ROM_IMAGE_85_9_0b  0x850901
#define FILE_ROM_IMAGE_85_10_0  0x851000
#define FILE_ROM_IMAGE_85_10_0b 0x851001

#define FILE_ROM_IMAGE_86_1_2 0x860102
#define FILE_ROM_IMAGE_86_1_3 0x860103
#define FILE_ROM_IMAGE_86_1_4 0x860104
#define FILE_ROM_IMAGE_86_1_5 0x860105
#define FILE_ROM_IMAGE_86_1_6 0x860106

typedef struct {
	int crc; // CRC32 of the first 0x10000 bytes
	int size; // ROM file size
	int ver; // Version constant
	char name[15]; // Version name
} ROM_SIGNATURE;

typedef struct {
	char idstr[50]; // Identification string
	int type; // File type
} FILE_SIGNATURE;

int detect_file_type(char *fname); // Determine file type and version ID
char *rom_version_name(int ver); // String representation of ROM version
int calculate_crc32(unsigned char *tmp, int len); // Checksum calculation for any data

#endif

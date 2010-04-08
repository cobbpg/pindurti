#include <windows.h>
#include <stdio.h>
#include "dwin.h"

#define KEYVAL_EMPTY(name, str) \
KEYVAL_DATA keyval_ ## name = { \
	str, no_debug_info, {{-1, -1, -1, 0, NULL}} \
}; \

// Common components

KEYVAL_EMPTY(nothing, "Empty component");
KEYVAL_EMPTY(no_lcd_physics, "LCD physics");
KEYVAL_EMPTY(no_lcd_software, "LCD software");
KEYVAL_EMPTY(no_info, "Information");
KEYVAL_EMPTY(no_time, "Time");
KEYVAL_EMPTY(no_interrupt, "Interrupts");
KEYVAL_EMPTY(no_memory, "Memory");
KEYVAL_EMPTY(no_keys, "Keyboard");
KEYVAL_EMPTY(no_link, "Link");

KEYVAL_DATA keyval_lcd_physics = {
	"LCD physics",
	lcd_debug_physical,
	{
		{0, 0, 32, 1, lcd_enter_tmin},
		{1, 0, 32, 1, lcd_enter_tmax},
		{2, 0, 48, 1, lcd_enter_busy},
		{3, 0, 32, 1, lcd_enter_sinc},
		{4, 0, 32, 1, lcd_enter_sdec},
		{5, 0, 32, 1, lcd_enter_cmul},
		{6, 0, 32, 1, lcd_enter_amul},
		{7, 0, 32, 1, lcd_enter_cwht},
		{8, 0, 32, 1, lcd_enter_cblk},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_lcd_software = {
	"LCD software",
	lcd_debug_software,
	{
		{0, 0, 20, 0, lcd_enter_power},
		{1, 0, 16, 1, lcd_enter_contrast},
		{2, 0, 16, 1, lcd_enter_x_address},
		{3, 0, 16, 1, lcd_enter_y_address},
		{4, 0, 16, 1, lcd_enter_z_address},
		{5, 0, 7, 0, lcd_enter_counter_sel},
		{5, 12, 29, 0, lcd_enter_counter_dir},
		{6, 0, 8, 0, lcd_enter_word_length},
		{7, 0, 40, 0, lcd_enter_amp1},
		{7, 44, 40, 0, lcd_enter_amp2},
		{8, 0, 16, 0, lcd_enter_test},
		{9, 0, 16, 0, lcd_enter_dummy},
		{-1, -1, -1, 0, NULL}
	}
};

// TI-82 components

KEYVAL_DATA keyval_ti_82_info = {
	"Information",
	ti_82_debug_info,
	{
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_82_time = {
	"Time",
	ti_82_debug_time,
	{
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_82_interrupt = {
	"Interrupts",
	ti_82_debug_interrupt,
	{
		{0, 0, 16, 0, ti_82_enter_it_ie}, {1, 0, 88, 0, ti_82_enter_it_freq},
		{2, 0, 24, 0, ti_82_enter_it_mask_on}, {2, 25, 24, 0, ti_82_enter_it_mask_t1},
		{2, 50, 24, 0, ti_82_enter_it_mask_t2}, {2, 75, 40, 0, ti_82_enter_it_mask_power},
		{3, 0, 24, 0, ti_82_enter_it_active_on}, {3, 25, 24, 0, ti_82_enter_it_active_t1},
		{3, 50, 24, 0, ti_82_enter_it_active_t2},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_82_memory = {
	"Memory",
	ti_82_debug_memory,
	{
		{0, 0, 7, 0, ti_82_enter_mem_map},
		{1, 0, 14, 1, ti_82_enter_rom_page},
		{2, 0, 14, 1, ti_82_enter_port_02},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_82_keys = {
	"Keyboard",
	ti_82_debug_keys,
	{
		{0,  0, 6, 0, ti_82_enter_key_07}, {0,  6, 6, 0, ti_82_enter_key_06},
		{0, 12, 6, 0, ti_82_enter_key_05}, {0, 18, 6, 0, ti_82_enter_key_04},
		{0, 24, 6, 0, ti_82_enter_key_03}, {0, 30, 6, 0, ti_82_enter_key_02},
		{0, 36, 6, 0, ti_82_enter_key_01}, {0, 42, 6, 0, ti_82_enter_key_00},
		{1,  0, 6, 0, ti_82_enter_key_17}, {1,  6, 6, 0, ti_82_enter_key_16},
		{1, 12, 6, 0, ti_82_enter_key_15}, {1, 18, 6, 0, ti_82_enter_key_14},
		{1, 24, 6, 0, ti_82_enter_key_13}, {1, 30, 6, 0, ti_82_enter_key_12},
		{1, 36, 6, 0, ti_82_enter_key_11}, {1, 42, 6, 0, ti_82_enter_key_10},
		{2,  0, 6, 0, ti_82_enter_key_27}, {2,  6, 6, 0, ti_82_enter_key_26},
		{2, 12, 6, 0, ti_82_enter_key_25}, {2, 18, 6, 0, ti_82_enter_key_24},
		{2, 24, 6, 0, ti_82_enter_key_23}, {2, 30, 6, 0, ti_82_enter_key_22},
		{2, 36, 6, 0, ti_82_enter_key_21}, {2, 42, 6, 0, ti_82_enter_key_20},
		{3,  0, 6, 0, ti_82_enter_key_37}, {3,  6, 6, 0, ti_82_enter_key_36},
		{3, 12, 6, 0, ti_82_enter_key_35}, {3, 18, 6, 0, ti_82_enter_key_34},
		{3, 24, 6, 0, ti_82_enter_key_33}, {3, 30, 6, 0, ti_82_enter_key_32},
		{3, 36, 6, 0, ti_82_enter_key_31}, {3, 42, 6, 0, ti_82_enter_key_30},
		{4,  0, 6, 0, ti_82_enter_key_47}, {4,  6, 6, 0, ti_82_enter_key_46},
		{4, 12, 6, 0, ti_82_enter_key_45}, {4, 18, 6, 0, ti_82_enter_key_44},
		{4, 24, 6, 0, ti_82_enter_key_43}, {4, 30, 6, 0, ti_82_enter_key_42},
		{4, 36, 6, 0, ti_82_enter_key_41}, {4, 42, 6, 0, ti_82_enter_key_40},
		{5,  0, 6, 0, ti_82_enter_key_57}, {5,  6, 6, 0, ti_82_enter_key_56},
		{5, 12, 6, 0, ti_82_enter_key_55}, {5, 18, 6, 0, ti_82_enter_key_54},
		{5, 24, 6, 0, ti_82_enter_key_53}, {5, 30, 6, 0, ti_82_enter_key_52},
		{5, 36, 6, 0, ti_82_enter_key_51}, {5, 42, 6, 0, ti_82_enter_key_50},
		{6,  0, 6, 0, ti_82_enter_key_67}, {6,  6, 6, 0, ti_82_enter_key_66},
		{6, 12, 6, 0, ti_82_enter_key_65}, {6, 18, 6, 0, ti_82_enter_key_64},
		{6, 24, 6, 0, ti_82_enter_key_63}, {6, 30, 6, 0, ti_82_enter_key_62},
		{6, 36, 6, 0, ti_82_enter_key_61}, {6, 42, 6, 0, ti_82_enter_key_60},
		{7, 0, 25, 0, ti_82_enter_on_key},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_82_link = {
	"Link",
	ti_82_debug_link,
	{
		{-1, -1, -1, 0, NULL}
	}
};

// TI-83 components

KEYVAL_DATA keyval_ti_83_info = {
	"Information",
	ti_83_debug_info,
	{
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83_time = {
	"Time",
	ti_83_debug_time,
	{
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83_interrupt = {
	"Interrupts",
	ti_83_debug_interrupt,
	{
		{0, 0, 16, 0, ti_83_enter_it_ie}, {1, 0, 88, 0, ti_83_enter_it_freq},
		{2, 0, 24, 0, ti_83_enter_it_mask_on}, {2, 25, 24, 0, ti_83_enter_it_mask_t1},
		{2, 50, 24, 0, ti_83_enter_it_mask_t2}, {2, 75, 40, 0, ti_83_enter_it_mask_power},
		{3, 0, 24, 0, ti_83_enter_it_active_on}, {3, 25, 24, 0, ti_83_enter_it_active_t1},
		{3, 50, 24, 0, ti_83_enter_it_active_t2},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83_memory = {
	"Memory",
	ti_83_debug_memory,
	{
		{0, 0, 7, 0, ti_83_enter_mem_map},
		{1, 0, 14, 1, ti_83_enter_rom_page},
		{2, 0, 14, 1, ti_83_enter_port_02},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83_keys = {
	"Keyboard",
	ti_83_debug_keys,
	{
		{0,  0, 6, 0, ti_83_enter_key_07}, {0,  6, 6, 0, ti_83_enter_key_06},
		{0, 12, 6, 0, ti_83_enter_key_05}, {0, 18, 6, 0, ti_83_enter_key_04},
		{0, 24, 6, 0, ti_83_enter_key_03}, {0, 30, 6, 0, ti_83_enter_key_02},
		{0, 36, 6, 0, ti_83_enter_key_01}, {0, 42, 6, 0, ti_83_enter_key_00},
		{1,  0, 6, 0, ti_83_enter_key_17}, {1,  6, 6, 0, ti_83_enter_key_16},
		{1, 12, 6, 0, ti_83_enter_key_15}, {1, 18, 6, 0, ti_83_enter_key_14},
		{1, 24, 6, 0, ti_83_enter_key_13}, {1, 30, 6, 0, ti_83_enter_key_12},
		{1, 36, 6, 0, ti_83_enter_key_11}, {1, 42, 6, 0, ti_83_enter_key_10},
		{2,  0, 6, 0, ti_83_enter_key_27}, {2,  6, 6, 0, ti_83_enter_key_26},
		{2, 12, 6, 0, ti_83_enter_key_25}, {2, 18, 6, 0, ti_83_enter_key_24},
		{2, 24, 6, 0, ti_83_enter_key_23}, {2, 30, 6, 0, ti_83_enter_key_22},
		{2, 36, 6, 0, ti_83_enter_key_21}, {2, 42, 6, 0, ti_83_enter_key_20},
		{3,  0, 6, 0, ti_83_enter_key_37}, {3,  6, 6, 0, ti_83_enter_key_36},
		{3, 12, 6, 0, ti_83_enter_key_35}, {3, 18, 6, 0, ti_83_enter_key_34},
		{3, 24, 6, 0, ti_83_enter_key_33}, {3, 30, 6, 0, ti_83_enter_key_32},
		{3, 36, 6, 0, ti_83_enter_key_31}, {3, 42, 6, 0, ti_83_enter_key_30},
		{4,  0, 6, 0, ti_83_enter_key_47}, {4,  6, 6, 0, ti_83_enter_key_46},
		{4, 12, 6, 0, ti_83_enter_key_45}, {4, 18, 6, 0, ti_83_enter_key_44},
		{4, 24, 6, 0, ti_83_enter_key_43}, {4, 30, 6, 0, ti_83_enter_key_42},
		{4, 36, 6, 0, ti_83_enter_key_41}, {4, 42, 6, 0, ti_83_enter_key_40},
		{5,  0, 6, 0, ti_83_enter_key_57}, {5,  6, 6, 0, ti_83_enter_key_56},
		{5, 12, 6, 0, ti_83_enter_key_55}, {5, 18, 6, 0, ti_83_enter_key_54},
		{5, 24, 6, 0, ti_83_enter_key_53}, {5, 30, 6, 0, ti_83_enter_key_52},
		{5, 36, 6, 0, ti_83_enter_key_51}, {5, 42, 6, 0, ti_83_enter_key_50},
		{6,  0, 6, 0, ti_83_enter_key_67}, {6,  6, 6, 0, ti_83_enter_key_66},
		{6, 12, 6, 0, ti_83_enter_key_65}, {6, 18, 6, 0, ti_83_enter_key_64},
		{6, 24, 6, 0, ti_83_enter_key_63}, {6, 30, 6, 0, ti_83_enter_key_62},
		{6, 36, 6, 0, ti_83_enter_key_61}, {6, 42, 6, 0, ti_83_enter_key_60},
		{7, 0, 25, 0, ti_83_enter_on_key},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83_link = {
	"Link",
	ti_83_debug_link,
	{
		{-1, -1, -1, 0, NULL}
	}
};

// TI-83 plus components

KEYVAL_DATA keyval_ti_83p_info = {
	"Information",
	ti_83p_debug_info,
	{
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83p_time = {
	"Time",
	ti_83p_debug_time,
	{
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83p_interrupt = {
	"Interrupts",
	ti_83p_debug_interrupt,
	{
		{0, 0, 16, 0, ti_83p_enter_it_ie}, {1, 0, 88, 0, ti_83p_enter_it_freq},
		{2, 0, 24, 0, ti_83p_enter_it_mask_on}, {2, 25, 24, 0, ti_83p_enter_it_mask_t1},
		{2, 50, 24, 0, ti_83p_enter_it_mask_t2}, {2, 75, 40, 0, ti_83p_enter_it_mask_power},
		{3, 0, 24, 0, ti_83p_enter_it_active_on}, {3, 25, 24, 0, ti_83p_enter_it_active_t1},
		{3, 50, 24, 0, ti_83p_enter_it_active_t2},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83p_memory = {
	"Memory",
	ti_83p_debug_memory,
	{
		{0, 0, 7, 0, ti_83p_enter_mem_map},
		{1, 0, 14, 1, ti_83p_enter_bank_a},
		{2, 0, 14, 1, ti_83p_enter_bank_b},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83p_keys = {
	"Keyboard",
	ti_83p_debug_keys,
	{
		{0,  0, 6, 0, ti_83p_enter_key_07}, {0,  6, 6, 0, ti_83p_enter_key_06},
		{0, 12, 6, 0, ti_83p_enter_key_05}, {0, 18, 6, 0, ti_83p_enter_key_04},
		{0, 24, 6, 0, ti_83p_enter_key_03}, {0, 30, 6, 0, ti_83p_enter_key_02},
		{0, 36, 6, 0, ti_83p_enter_key_01}, {0, 42, 6, 0, ti_83p_enter_key_00},
		{1,  0, 6, 0, ti_83p_enter_key_17}, {1,  6, 6, 0, ti_83p_enter_key_16},
		{1, 12, 6, 0, ti_83p_enter_key_15}, {1, 18, 6, 0, ti_83p_enter_key_14},
		{1, 24, 6, 0, ti_83p_enter_key_13}, {1, 30, 6, 0, ti_83p_enter_key_12},
		{1, 36, 6, 0, ti_83p_enter_key_11}, {1, 42, 6, 0, ti_83p_enter_key_10},
		{2,  0, 6, 0, ti_83p_enter_key_27}, {2,  6, 6, 0, ti_83p_enter_key_26},
		{2, 12, 6, 0, ti_83p_enter_key_25}, {2, 18, 6, 0, ti_83p_enter_key_24},
		{2, 24, 6, 0, ti_83p_enter_key_23}, {2, 30, 6, 0, ti_83p_enter_key_22},
		{2, 36, 6, 0, ti_83p_enter_key_21}, {2, 42, 6, 0, ti_83p_enter_key_20},
		{3,  0, 6, 0, ti_83p_enter_key_37}, {3,  6, 6, 0, ti_83p_enter_key_36},
		{3, 12, 6, 0, ti_83p_enter_key_35}, {3, 18, 6, 0, ti_83p_enter_key_34},
		{3, 24, 6, 0, ti_83p_enter_key_33}, {3, 30, 6, 0, ti_83p_enter_key_32},
		{3, 36, 6, 0, ti_83p_enter_key_31}, {3, 42, 6, 0, ti_83p_enter_key_30},
		{4,  0, 6, 0, ti_83p_enter_key_47}, {4,  6, 6, 0, ti_83p_enter_key_46},
		{4, 12, 6, 0, ti_83p_enter_key_45}, {4, 18, 6, 0, ti_83p_enter_key_44},
		{4, 24, 6, 0, ti_83p_enter_key_43}, {4, 30, 6, 0, ti_83p_enter_key_42},
		{4, 36, 6, 0, ti_83p_enter_key_41}, {4, 42, 6, 0, ti_83p_enter_key_40},
		{5,  0, 6, 0, ti_83p_enter_key_57}, {5,  6, 6, 0, ti_83p_enter_key_56},
		{5, 12, 6, 0, ti_83p_enter_key_55}, {5, 18, 6, 0, ti_83p_enter_key_54},
		{5, 24, 6, 0, ti_83p_enter_key_53}, {5, 30, 6, 0, ti_83p_enter_key_52},
		{5, 36, 6, 0, ti_83p_enter_key_51}, {5, 42, 6, 0, ti_83p_enter_key_50},
		{6,  0, 6, 0, ti_83p_enter_key_67}, {6,  6, 6, 0, ti_83p_enter_key_66},
		{6, 12, 6, 0, ti_83p_enter_key_65}, {6, 18, 6, 0, ti_83p_enter_key_64},
		{6, 24, 6, 0, ti_83p_enter_key_63}, {6, 30, 6, 0, ti_83p_enter_key_62},
		{6, 36, 6, 0, ti_83p_enter_key_61}, {6, 42, 6, 0, ti_83p_enter_key_60},
		{7, 0, 25, 0, ti_83p_enter_on_key},
		{-1, -1, -1, 0, NULL}
	}
};

KEYVAL_DATA keyval_ti_83p_link = {
	"Link",
	ti_83p_debug_link,
	{
		{-1, -1, -1, 0, NULL}
	}
};

// Component references

KEYVAL_DATA *keyval_no_ptr[] = {
	&keyval_no_info,
	&keyval_no_time,
	&keyval_no_interrupt,
	&keyval_no_memory,
	&keyval_no_keys,
	&keyval_no_link,
	&keyval_no_lcd_physics,
	&keyval_no_lcd_software
};

KEYVAL_DATA *keyval_82_ptr[] = {
	&keyval_ti_82_info,
	&keyval_ti_82_time,
	&keyval_ti_82_interrupt,
	&keyval_ti_82_memory,
	&keyval_ti_82_keys,
	&keyval_ti_82_link,
	&keyval_lcd_physics,
	&keyval_lcd_software
};

KEYVAL_DATA *keyval_83_ptr[] = {
	&keyval_ti_83_info,
	&keyval_ti_83_time,
	&keyval_ti_83_interrupt,
	&keyval_ti_83_memory,
	&keyval_ti_83_keys,
	&keyval_ti_83_link,
	&keyval_lcd_physics,
	&keyval_lcd_software
};

KEYVAL_DATA *keyval_83p_ptr[] = {
	&keyval_ti_83p_info,
	&keyval_ti_83p_time,
	&keyval_ti_83p_interrupt,
	&keyval_ti_83p_memory,
	&keyval_ti_83p_keys,
	&keyval_ti_83p_link,
	&keyval_lcd_physics,
	&keyval_lcd_software
};

KEYVAL_DATA *keyval_data(int id, int model) {
	if (id < 0 || id > 7) return &keyval_nothing;
	model &= FILE_MODEL_MASK;
	switch (model) {
	case FILE_MODEL_82:
	case FILE_MODEL_82b: return keyval_82_ptr[id]; break;
	case FILE_MODEL_83: return keyval_83_ptr[id]; break;
	case FILE_MODEL_83P: return keyval_83p_ptr[id]; break;
	}
	return keyval_no_ptr[id];
}

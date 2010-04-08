#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include "dwin.h"
#include "../theme.h"

void lcd_data_paint(DBG_LCD_DATA *dat, HDC dc) {
	if (activate_calculator(dat->slot)) return;
	int i, j;
	char *src, *dest;
	RECT r;
	if ((cdat->r.xs < 32) || (cdat->r.ys < 38)) return;
	r.left = 14;
	r.top = 20;
	r.right = (cdat->r.xs > 272) ? 258 : cdat->r.xs - 14;
	r.bottom = (cdat->r.ys > 166) ? 152 : cdat->r.ys - 14;
	draw_sunken_box(dc, &r);
	src = lcd->dat;
	dest = dat->idat;
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 120; j++) {
			*dest++ = *src;
			*dest++ = *src++;
		}
		memcpy(dest, dest - 240, 240);
		dest += 240;
	}
	dest = dat->idat + (lcd->x & 0x3f) * 480 + (lcd->y % (lcd->w_len ? 15 : 20)) * (lcd->w_len ? 16 : 12);
	for (i = 0; i < (lcd->w_len ? 8 : 6); i++) {
		dest[240] |= 2;
		*dest++ |= 2;
		dest[240] |= 2;
		*dest++ |= 2;
	}
	i = r.right - r.left - 4;
	j = r.bottom - r.top - 4;
	if ((r.right > r.left + 4) && (r.bottom > r.top + 4))
		SetDIBitsToDevice(dc, 16, 22, i, j, 0, 0, 0, j, dat->idat, dat->bmi, DIB_RGB_COLORS);
}

int lcd_data_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	UNUSED_PARAMETER(hwnd);
	DBG_LCD_DATA *dat = cdat->state;

	switch (msg) {
		case WM_CREATE:
			dat->slot = -1;
			dat->idat = malloc(sizeof(char[240 * 128]));
			dat->bmi = malloc(sizeof(BITMAPINFOHEADER) + 16);
			dat->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			dat->bmi->bmiHeader.biWidth = 240;
			dat->bmi->bmiHeader.biHeight = -128;
			dat->bmi->bmiHeader.biPlanes = 1;
			dat->bmi->bmiHeader.biBitCount = 8;
			dat->bmi->bmiHeader.biCompression = BI_RGB;
			dat->bmi->bmiHeader.biSizeImage = 0;
			dat->bmi->bmiHeader.biXPelsPerMeter = 0;
			dat->bmi->bmiHeader.biYPelsPerMeter = 0;
			dat->bmi->bmiHeader.biClrUsed = 4;
			dat->bmi->bmiHeader.biClrImportant = 4;
			dat->bmi->bmiColors[0].rgbRed = 255;
			dat->bmi->bmiColors[0].rgbGreen = 255;
			dat->bmi->bmiColors[0].rgbBlue = 255;
			dat->bmi->bmiColors[1].rgbRed = 0;
			dat->bmi->bmiColors[1].rgbGreen = 0;
			dat->bmi->bmiColors[1].rgbBlue = 0;
			dat->bmi->bmiColors[2].rgbRed = 215;
			dat->bmi->bmiColors[2].rgbGreen = 215;
			dat->bmi->bmiColors[2].rgbBlue = 255;
			dat->bmi->bmiColors[3].rgbRed = 55;
			dat->bmi->bmiColors[3].rgbGreen = 55;
			dat->bmi->bmiColors[3].rgbBlue = 215;
		break;
		case WM_DESTROY:
			free(dat->bmi);
			free(dat->idat);
		break;
		case WM_PRINT:
			lcd_data_paint(dat, (HDC)wparam);
		break;
		case WM_USER:
			switch (wparam) {
				case DEBUG_SLOT:
					dat->slot = lparam;
				break;
			}
		break;
	}
	return 0;
}


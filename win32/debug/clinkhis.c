#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <winuser.h>
#include "dwin.h"
#include "../theme.h"

void link_history_paint(DBG_LINK_HISTORY_DATA *dat, HDC dc) {
	if (activate_calculator(dat->slot)) return;
	int xs, ys, xsr, i, s, s2, recp;
	char *idat, *iptr;
	RECT r;
	if ((cdat->r.xs < 32) || (cdat->r.ys < 38)) return;
	r.left = 14;
	r.top = 20;
	r.right = cdat->r.xs - 14;
	r.bottom = cdat->r.ys - 14;
	draw_sunken_box(dc, &r);
	if ((r.right <= r.left + 4) || (r.bottom <= r.top + 4)) return;
	xs = r.right - r.left - 4;
	ys = r.bottom - r.top - 4;
	xsr = (xs + 8 - (xs & 7));
	dat->bmi->bmiHeader.biWidth = xsr;
	dat->bmi->bmiHeader.biHeight = -ys;
	idat = malloc(xsr * ys);
	memset(idat, 0x00, xsr * ys);
	for (s2 = 0, s = 0; s < MAX_CALC; s++) {
		if (calc[s].rom_ver != -1) {
			if (s2 * 12 + 11 > ys) break;
			for (recp = linking.recp, iptr = idat + xsr * (4 + s2 * 12) + xs, i = 0; i < xs; iptr--, i++) {
				BYTE dt = linking.rec[s][recp];
				iptr[xsr * 2 * (dt & 1)] = 1;
				iptr[xsr * ((dt & 2) + 4)] = 2;
				if (i & 1) {
					recp--;
					if (recp < 0) recp = LINK_HISTORY - 1;
				}
			}
			s2++;
		}
	}
	SetDIBitsToDevice(dc, 16, 22, xs, ys, 0, 0, 0, ys, idat, dat->bmi, DIB_RGB_COLORS);
	free(idat);
}

int link_history_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	UNUSED_PARAMETER(hwnd);
	DBG_LINK_HISTORY_DATA *dat = cdat->state;

	switch (msg) {
	case WM_CREATE:
		dat->slot = -1;
		dat->bmi = malloc(sizeof(BITMAPINFOHEADER) + 12);
		dat->bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		dat->bmi->bmiHeader.biWidth = 240;
		dat->bmi->bmiHeader.biHeight = -128;
		dat->bmi->bmiHeader.biPlanes = 1;
		dat->bmi->bmiHeader.biBitCount = 8;
		dat->bmi->bmiHeader.biCompression = BI_RGB;
		dat->bmi->bmiHeader.biSizeImage = 0;
		dat->bmi->bmiHeader.biXPelsPerMeter = 0;
		dat->bmi->bmiHeader.biYPelsPerMeter = 0;
		dat->bmi->bmiHeader.biClrUsed = 3;
		dat->bmi->bmiHeader.biClrImportant = 3;
		dat->bmi->bmiColors[0].rgbRed = 255;
		dat->bmi->bmiColors[0].rgbGreen = 255;
		dat->bmi->bmiColors[0].rgbBlue = 255;
		dat->bmi->bmiColors[1].rgbRed = 0;
		dat->bmi->bmiColors[1].rgbGreen = 0;
		dat->bmi->bmiColors[1].rgbBlue = 0;
		dat->bmi->bmiColors[2].rgbRed = 255;
		dat->bmi->bmiColors[2].rgbGreen = 0;
		dat->bmi->bmiColors[2].rgbBlue = 0;
		break;
	case WM_DESTROY:
		free(dat->bmi);
		break;
	case WM_PRINT:
		link_history_paint(dat, (HDC)wparam);
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


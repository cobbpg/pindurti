#include <windows.h>
#include <stdio.h>
#include "dwin.h"
#include "../pindurti.h"

DEBUG_COMMON dcom;

INT_PTR CALLBACK debug_address_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	UNUSED_PARAMETER(lparam);
	if (msg == WM_COMMAND)
		switch (LOWORD(wparam)) {
			case IDOK: {
				char *err, val[8];
				GetDlgItemText(hwnd, IDC_ENTER_ADDRESS, val, 8);
				dcom.read_address = strtol(val, &err, 16);
				if (!val[0] || err[0])
					EndDialog(hwnd, IDCANCEL);
				else
					EndDialog(hwnd, IDOK);
			}
			break;
			case IDCANCEL: EndDialog(hwnd, IDCANCEL); break;
		}
	return FALSE;
}

int debug_read_address(HWND hwnd) {
	int success = (DialogBox(GetModuleHandle(NULL), (LPCTSTR)IDD_ENTER_ADDRESS, hwnd, debug_address_proc) == IDOK);
	SetFocus(hwnd);
	return success;
}

INT_PTR CALLBACK debug_number_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	UNUSED_PARAMETER(lparam);
	if (msg == WM_COMMAND)
		switch (LOWORD(wparam)) {
			case IDOK: {
				char *err, val[21];
				GetDlgItemText(hwnd, IDC_ENTER_NUMBER, val, 20);
				if (!val[0]) {
					EndDialog(hwnd, IDCANCEL);
					break;
				}
				dcom.read_number = strtol(val, &err, dcom.read_base);
				if (!err[0]) {
					EndDialog(hwnd, IDOK);
					break;
				}
				switch (val[0]) {
					case '%': dcom.read_number = strtol(val + 1, &err, 2); break;
					case '#': dcom.read_number = strtol(val + 1, &err, 10); break;
					case '$': dcom.read_number = strtol(val + 1, &err, 16); break;
					case '@': dcom.read_number = strtol(val + 1, &err, 8); break;
				}
				EndDialog(hwnd, err[0] ? IDCANCEL : IDOK);
			}
			break;
			case IDCANCEL: EndDialog(hwnd, IDCANCEL); break;
		}
	return FALSE;
}

int debug_read_number(HWND hwnd) {
	int success = (DialogBox(GetModuleHandle(NULL), (LPCTSTR)IDD_ENTER_NUMBER, hwnd, debug_number_proc) == IDOK);
	SetFocus(hwnd);
	return success;
}

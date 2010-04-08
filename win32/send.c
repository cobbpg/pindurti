#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winuser.h>

int main(int argc, char *argv[]) {
	HWND hwnd;
	COPYDATASTRUCT cds;
	char dat[258];
	int i;

	hwnd = FindWindow("PindurWnd", NULL);

	if (hwnd == NULL) {
		printf("No PindurTI instance found!\n");
		return 1;
	}

	if (argc < 3) {
		printf("No files specified!\n");
		return 1;
	}

	for (i = 2; i < argc; i++) {
		printf("Sending %s... ", argv[i]);

		cds.cbData = strlen(argv[i]) + 3;
		cds.lpData = dat;
		dat[0] = argv[1][0] - '0';
		dat[1] = strlen(argv[i]);
		strncpy(dat + 2, argv[i], 255);
		if (SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds)) {
			printf("FAILED\n");
			return 1;
		} else printf("OK\n");
	}

	return 0;
}


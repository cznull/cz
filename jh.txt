#include <iostream>
#include <atlimage.h>

int main() {
	CImage im;
	unsigned char *imb;
	HDC imdc;
	char cmds[1024];
	wchar_t cmdws[1024];
	int t = 0, i, j, l1, l2, x1, x2, y1, y2, a;
	unsigned int r[10], g[10], b[10];
	FILE *fi;
	SetCurrentDirectoryA("D:/");
	system("D:/download/platform-tools/adb.exe devices");
	for (a = 1;; a++) {
		sprintf_s(cmds, "D:/download/platform-tools/adb.exe shell screencap /sdcard/%d.png", 1);
		system(cmds);
		sprintf_s(cmds, "D:/%d.png", a);
		if (!fopen_s(&fi, cmds, "w")) {
			fclose(fi);
		}
		sprintf_s(cmds, "D:/download/platform-tools/adb.exe pull /sdcard/%d.png D:/%d.png", 1, a);
		system(cmds);
		swprintf_s(cmdws, _T("d:/%d.png"), a);
		im.Load(cmdws);
		if (im.GetPitch() < 0) {
			imb = (unsigned char*)im.GetBits() + (im.GetPitch()*(im.GetHeight() - 1));
		}
		else {
			imb = (unsigned char*)im.GetBits();
		}
		r[0] = imb[2];
		g[0] = imb[1];
		b[0] = imb[0];
		r[1] = imb[1400 * 1080 * 4 + 540 * 4 - 2];
		g[1] = imb[1400 * 1080 * 4 + 540 * 4 - 3];
		b[1] = imb[1400 * 1080 * 4 + 540 * 4 - 4];
		for (i = 1400; i > 540; i--) {
			for (j = 0; j < 1080; j++) {
				if (b[1] - 15 > imb[i * 1080 * 4 + j * 4] || b[1] + 5 < imb[i * 1080 * 4 + j * 4] || g[1] - 15 > imb[i * 1080 * 4 + j * 4 + 1] || g[1] + 5 < imb[i * 1080 * 4 + j * 4 + 1] || r[1] - 15 > imb[i * 1080 * 4 + j * 4 + 2] || r[1] + 5 < imb[i * 1080 * 4 + j * 4 + 2]) {
					x1 = j;
					y1 = 1919 - i;
					l1 = 0;
					for (; j < 1080; j++) {
						if (b[1] - 15 > imb[i * 1080 * 4 + j * 4] || b[1] + 5 < imb[i * 1080 * 4 + j * 4] || g[1] - 15 > imb[i * 1080 * 4 + j * 4 + 1] || g[1] + 5 < imb[i * 1080 * 4 + j * 4 + 1] || r[1] - 15 > imb[i * 1080 * 4 + j * 4 + 2] || r[1] + 5 < imb[i * 1080 * 4 + j * 4 + 2]) {
							l1++;
						}
					}
					x1 += l1 / 2;
					std::cout << '(' << x1 << ',' << 1919 - i << ")\n";
					goto next1;
				}
			}
		}
	next1:
		for (i = 1400; i > 540; i--) {
			for (j = 0; j < 1080; j++) {
				if (70 > imb[i * 1080 * 4 + j * 4] && 55 < imb[i * 1080 * 4 + j * 4] && 60 > imb[i * 1080 * 4 + j * 4 + 1] && 45 < imb[i * 1080 * 4 + j * 4 + 1] && 60 > imb[i * 1080 * 4 + j * 4 + 2] && 45 < imb[i * 1080 * 4 + j * 4 + 2] && (int)imb[(i - 29) * 1080 * 4 + j * 4 + 0] - (int)imb[(i - 29) * 1080 * 4 + j * 4 + 1] > 35 && (int)imb[(i - 29) * 1080 * 4 + j * 4 + 0] - (int)imb[(i - 29) * 1080 * 4 + j * 4 + 1] < 60 && (int)imb[(i - 29) * 1080 * 4 + j * 4 + 0]>90 && (int)imb[(i - 29) * 1080 * 4 + j * 4 + 0] < 135) {
					x2 = j;
					y2 = 1919 - i;
					l2 = 0;
					for (; j < 1080; j++) {
						if (79 > imb[i * 1080 * 4 + j * 4] && 55 < imb[i * 1080 * 4 + j * 4] && 60 > imb[i * 1080 * 4 + j * 4 + 1] && 45 < imb[i * 1080 * 4 + j * 4 + 1] && 60 > imb[i * 1080 * 4 + j * 4 + 2] && 45 < imb[i * 1080 * 4 + j * 4 + 2]) {
							l2++;
						}
					}
					x2 += l2 / 2;
					std::cout << '(' << x2 << ',' << 1919 - i << ")\n";
					goto next2;
				}
			}
		}
	next2:
		if (x1 > x2 + 10) {
			x2 = (300 + x2 - 1.732*(y2 - 942)) / 2;
		}
		else if (x1 < x2 - 10) {
			x2 = (847 + x2 + 1.732*(y2 - 956)) / 2;
		}
		else {
			std::cout << "error\n";
			for (i = 1919 - y2 + 10; i > 1919 - y2 - 230; i--) {
				for (j = x2 + l2 / 2 - 50; j < x2 + l2 / 2 + 50; j++) {
					imb[i * 1080 * 4 + j * 4] = b[1];
					imb[i * 1080 * 4 + j * 4 + 1] = g[1];
					imb[i * 1080 * 4 + j * 4 + 2] = r[1];
				}
			}
			for (i = 1400; i > 540; i--) {
				for (j = 0; j < 1080; j++) {
					if (b[1] - 15 > imb[i * 1080 * 4 + j * 4] || b[1] + 5 < imb[i * 1080 * 4 + j * 4] || g[1] - 15 > imb[i * 1080 * 4 + j * 4 + 1] || g[1] + 5 < imb[i * 1080 * 4 + j * 4 + 1] || r[1] - 15 > imb[i * 1080 * 4 + j * 4 + 2] || r[1] + 5 < imb[i * 1080 * 4 + j * 4 + 2]) {
						x1 = j;
						y1 = 1919 - i;
						l1 = 0;
						for (; j < 1080; j++) {
							if (b[1] - 15 > imb[i * 1080 * 4 + j * 4] || b[1] + 5 < imb[i * 1080 * 4 + j * 4] || g[1] - 15 > imb[i * 1080 * 4 + j * 4 + 1] || g[1] + 5 < imb[i * 1080 * 4 + j * 4 + 1] || r[1] - 15 > imb[i * 1080 * 4 + j * 4 + 2] || r[1] + 5 < imb[i * 1080 * 4 + j * 4 + 2]) {
								l1++;
							}
							else {
								break;
							}
						}
						x1 += l1 / 2;
						std::cout << '(' << x1 << ',' << 1919 - i << ")\n";
						goto next3;
					}
				}
			}
		next3:
			if (x1 > x2 + 10) {
				x2 = (300 + x2 - 1.732*(y2 - 942)) / 2;
			}
			else if (x1 < x2 - 10) {
				x2 = (847 + x2 + 1.732*(y2 - 956)) / 2;
			}
			else {
				printf("error\n");
				getchar();
				continue;
			}
		}
		t = abs(x2 - x1) * 1.59 + 60;
		if (!fopen_s(&fi, "d:/log.txt", "a")) {
			fprintf_s(fi, "(%d,%d),(%d,%d),%d\n", x1, y1, x2, y2, t);
			fclose(fi);
		}
		sprintf_s(cmds, "D:/download/platform-tools/adb.exe shell input swipe 250 250 250 250 %d", t);
		system(cmds);
		im.Destroy();
		Sleep(t * 1.3 + 1000);
	}
	return 0;
}

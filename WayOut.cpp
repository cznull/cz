// gamegui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "gamegui.h"
#include <gl/glew.h>
#include <math.h>
#include <opencv2/opencv.hpp>


#pragma comment(lib,"opencv_world401.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glew32.lib")

#define MAX_LOADSTRING 100

struct xort {
	int v;
	xort operator +=(xort a);
};

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

const int size = 32;
HDC hdc1, hdc2;
HGLRC m_hrc;
int mx, my, cx, cy;
double ang1, ang2, len, cenx, ceny, cenz;
GLuint imgtex;

int mapx1, mapy1;
int msx=size, msy=size;
int *map;
int *state;
xort *bu;
xort *mat;
int cs = 1;
int key[256] = { 0 };
float vm[16] = {
	1.8,0.0,0.0,0.0,
	0.0,1.8,0.0,0.0,
	0.0,0.0,1.0,0.0,
	-0.9,-0.9,0.0,1.000 };


xort operator +(xort a, xort b) {
	return { (a.v + b.v) % 2 };
}

xort xort::operator +=(xort a) {
	return { v = (a.v + v) % 2 };
}

int solve(xort *a, xort *y, int n) {
	int i, j, k;
	int r, c;
	xort temp;
	r = 0;
	c = 0;
	for (; c < n; c++) {
		for (j = r; j < n; j++) {
			if (a[j*n + c].v == 1) {
				for (k = c; k < n; k++) {
					temp = a[j*n + k];
					a[j*n + k] = a[r*n + k];
					a[r*n + k] = temp;
				}
				temp = y[j];
				y[j] = y[r];
				y[r] = temp;
				break;
			}
		}
		if (j == n) {
			continue;
		}
		for (j = r + 1; j < n; j++) {
			if (a[j*n + c].v == 1) {
				for (k = c; k < n; k++) {
					a[j*n + k] += a[r*n + k];
				}
				y[j] += y[r];
			}
		}
		r++;
	}
	for (i = n - 1; i >= 0; i--) {
		for (j = 0; j < n; j++) {
			if (a[i*n + j].v == 1) {
				break;
			}
		}
		if (j < n) {
			for (k = 0; k < n; k++) {
				a[j*n + k] = a[i*n + k];
				y[j] = y[i];
				//a[i*n + k].v = 0;
				//y[i].v = 0;
			}
		}
	}
	for (i = n - 1; i >= 0; i--) {
		if (a[i*n + i].v == 1) {
			for (j = 0; j < i; j++) {
				if (a[j*n + i].v == 1) {
					y[j] += y[i];
				}
			}
		}
	}
	return 0;
}
//0 empty
//1 normal
//2 out
//3 coherent
//4 horizontal
//5 vertical
//6 surround

int spread(int x1, int y1, int x2, int  y2, int x, int y, int *map, xort *mat,int size) {
	switch (map[y2*size + x2]) {
	case 0:
	case 2:
		break;
	case 1:
	case 4:
	case 5:
		mat[(y2*x + x2)*x*y + y1 * x + x1].v = 1;
		break;
	case 3: {
		mat[(y2*x + x2)*x*y + y1 * x + x1].v = 1;
		for (int j = 0; j < y; j++) {
			for (int i = 0; i < x; i++) {
				if (map[j*size+i] == 3) {
					mat[(j*x+i) * x*y + y1 * x + x1].v = 1;
				}
			}
		}
		break;
	}
	case 6:
		mat[(y2*x + x2)*x*y + y1 * x + x1].v = 1;
		break;
	}
	if (map[y1*size + x1] == 3) {
		for (int j = 0; j < y; j++) {
			for (int i = 0; i < x; i++) {
				if (map[j*size + i] == 3) {
					mat[(j*x + i) * x*y + y1 * x + x1].v = 1;
				}
			}
		}
	}
	return 0;
}


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


void draw(void) {
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(0x00004100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(vm);
	int i,j;
	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 0.0);
	for (i = 0; i < 33; i++) {
		glVertex3f(0.0, i*(1.0 / 32), 0.0);
		glVertex3f(1.0, i*(1.0 / 32), 0.0);
		glVertex3f(i*(1.0 / 32), 0.0, 0.0);
		glVertex3f(i*(1.0 / 32), 1.0, 0.0);
	}
	glEnd();
	glBegin(GL_QUADS);
	for (j = 0; j < 32; j++) {
		for (i = 0; i < 32; i++) {
			if (i < msx&&j < msy) {
				glColor3f(state[j*size + i] ? 0.5 : 1.0, 1.0, bu[j*size + i].v ? 0.0 : 1.0);
			}
			else {
				glColor3f(state[j*size + i] ? 0.3 : 0.5, 0.5, bu[j*size + i].v ? 0.0 : 0.5);
			}
			float s, t;
			s = (map[j*size + i] % 4)*0.25;
			t = (map[j*size + i] / 4)*0.25;
			glTexCoord2f(s + 0.0f, t + 0.0f);
			glVertex3f(i*(1.0 / 32), j*(1.0 / 32), 0.0);
			glTexCoord2f(s + 0.25f, t + 0.0f);
			glVertex3f((i + 1)*(1.0 / 32), j*(1.0 / 32), 0.0);
			glTexCoord2f(s + 0.25f, t + 0.25f);
			glVertex3f((i + 1)*(1.0 / 32), (j + 1)*(1.0 / 32), 0.0);
			glTexCoord2f(s + 0.0f, t + 0.25f);
			glVertex3f(i*(1.0 / 32), (j + 1)*(1.0 / 32), 0.0);
		}
	}
	glEnd();
	SwapBuffers(wglGetCurrentDC());
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAMEGUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEGUI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMEGUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAMEGUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		draw();
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CREATE: {
		int i, j;
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_STEREO,
			PFD_TYPE_RGBA,
			24,
			0,0,0,0,0,0,0,0,
			0,
			0,0,0,0,
			32,
			0,0,
			PFD_MAIN_PLANE,
			0,0,0,0
		};
		hdc1 = GetDC(hWnd);
		hdc2 = GetDC(NULL);
		int uds = ::ChoosePixelFormat(hdc1, &pfd);
		::SetPixelFormat(hdc1, uds, &pfd);
		m_hrc = ::wglCreateContext(hdc1);
		::wglMakeCurrent(hdc1, m_hrc);
		glewInit();
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D); 
		
		glGenTextures(1, &imgtex);
		glBindTexture(GL_TEXTURE_2D, imgtex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		//glEnable(GL_ALPHA_TEST);
		//glAlphaFunc(GL_GREATER, 0.0f);
		//glEnableClientState(GL_NORMAL_ARRAY);
		//((bool(_stdcall*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(1);

		ang1 = 0;
		ang2 = 0;
		len = 2;
		cenx = 0.0;
		ceny = 0.0;
		cenz = 0.0;
		cv::Mat frame(cv::imread("0.png"));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);

		map = (int *)malloc(msx*msy * sizeof(int));
		state = (int *)malloc(msx*msy * sizeof(int));
		bu = (xort *)malloc(msx*msy * sizeof(xort));
		mat = (xort *)malloc(msx*msy*msx*msy * sizeof(xort));
		for (i = 0; i < msx*msy; i++) {
			map[i] = 1;
			state[i] = 0;
			bu[i].v = 0;
		}

		break;
	}
	case WM_SIZE: {
		cx = lParam & 0xffff;
		cy = (lParam & 0xffff0000) >> 16;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (cx < cy) {
			glViewport(0, (cy - cx) / 2, cx, cx);
		}
		else {
			glViewport((cx - cy) / 2, 0, cy, cy);
		}
		break;
	}
	case WM_MOUSEMOVE: {
		int x, y, f;
		int mapx, mapy;
		f = 0;
		x = (lParam & 0xffff);
		y = ((lParam & 0xffff0000) >> 16);
		if (cx > cy) {
			mapx = (x - (cx - cy) / 2 - 0.05*cy) * 32 / 0.9 / cy;
			mapy = (0.95*cy - y) * 32 / 0.9 / cy;
		}
		else {
			mapx = (x - 0.05*cx) * 32 / 0.9 / cx;
			mapy = (0.95*cy - (cx - cy) / 2 - y) * 32 / 0.9 / cx;
		}
		if (MK_LBUTTON&wParam) {
			if (mapx != mapx1 || mapy != mapy1) {
				if (-1 < mapx&&mapx < 32 && -1 < mapy&&mapy < 32) {
					map[mapy*size + mapx] = cs;
					f = 1;
				}
			}
		}
		if (MK_RBUTTON&wParam) {
			if (mapx != mapx1 || mapy != mapy1) {
				if (-1 < mapx&&mapx < 32 && -1 < mapy&&mapy < 32) {
					state[mapy*size + mapx] = !state[mapy*size + mapx];
					f = 1;
				}
			}
		}
		if (MK_MBUTTON&wParam) {
			if (mapx != mapx1 || mapy != mapy1) {
				if (-1 < mapx&&mapx < 32 && -1 < mapy&&mapy < 32) {
					msx = mapx + 1;
					msy = mapy + 1;
					f = 1;
				}
			}
		}
		mx = x;
		my = y;
		mapx1 = mapx;
		mapy1 = mapy;
		if (f) {
			draw();
		}
		break;
	}
	case WM_RBUTTONDOWN: {
		int x, y, f;
		int mapx, mapy;
		f = 0;
		x = (lParam & 0xffff);
		y = ((lParam & 0xffff0000) >> 16);
		if (cx > cy) {
			mapx = (x - (cx - cy) / 2 - 0.05*cy) * 32 / 0.9 / cy;
			mapy = (0.95*cy - y) * 32 / 0.9 / cy;
		}
		else {
			mapx = (x - 0.05*cx) * 32 / 0.9 / cx;
			mapy = (0.95*cy - (cx - cy) / 2 - y) * 32 / 0.9 / cx;
		}
		if (-1 < mapx&&mapx < 32 && -1 < mapy&&mapy < 32) {
			state[mapy*size + mapx] = !state[mapy*size + mapx];
		}
		mx = x; 
		my = y;
		mapx1 = mapx;
		mapy1 = mapy;
		draw();
		break;
	}
	case WM_LBUTTONDOWN: {
		int x, y, f;
		int mapx, mapy;
		f = 0;
		x = (lParam & 0xffff);
		y = ((lParam & 0xffff0000) >> 16);
		if (cx > cy) {
			mapx = (x - (cx - cy) / 2 - 0.05*cy) * 32 / 0.9 / cy;
			mapy = (0.95*cy - y) * 32 / 0.9 / cy;
		}
		else {
			mapx = (x - 0.05*cx) * 32 / 0.9 / cx;
			mapy = (0.95*cy - (cx - cy) / 2 - y) * 32 / 0.9 / cx;
		}
		if (-1 < mapx&&mapx < 32 && -1 < mapy&&mapy < 32) {
			if (map[mapy*size + mapx]) {
				map[mapy*size + mapx] = cs;
			}
			else {
				map[mapy*size + mapx] = cs;
			}
		}
		mx = x;
		my = y;
		mapx1 = mapx;
		mapy1 = mapy;
		draw();
		break;
	}
	case WM_MBUTTONDOWN: {
		int x, y, f;
		int mapx, mapy;
		f = 0;
		x = (lParam & 0xffff);
		y = ((lParam & 0xffff0000) >> 16);
		if (cx > cy) {
			mapx = (x - (cx - cy) / 2 - 0.05*cy) * 32 / 0.9 / cy;
			mapy = (0.95*cy - y) * 32 / 0.9 / cy;
		}
		else {
			mapx = (x - 0.05*cx) * 32 / 0.9 / cx;
			mapy = (0.95*cy - (cx - cy) / 2 - y) * 32 / 0.9 / cx;
		}
		if (-1 < mapx&&mapx < 32 && -1 < mapy&&mapy < 32) {
			//state[mapy*size + mapx] = !state[mapy*size + mapx];
			msx = mapx+1;
			msy = mapy+1;
		}
		mx = x;
		my = y;
		mapx1 = mapx;
		mapy1 = mapy;
		draw();
		break;
	}
	case WM_MOUSEWHEEL: {
		short m;
		m = (wParam & 0xffff0000) >> 16;
		break;
	}
	case WM_KEYDOWN: {
		key[wParam & 0xff] = 1;
		switch (wParam) {
		case 32: {
			int i, j, n, x, y;
			x = msx;
			y = msy;
			n = msx * msy;
			memset(bu, 0, size*size * sizeof(xort));
			memset(mat, 0, x*y*x*y * sizeof(xort));
			for (j = 0; j < y; j++) {
				for (i = 0; i < x; i++) {
					mat[(j*x + i)*n + (j*x + i)].v = 1;
					bu[j*x + i].v = state[j*size + i];
					if (map[j*size + i] > 0) {
						if (i > 0 && map[j*size + i] != 4) {
							spread(i, j, i - 1, j, x, y, map, mat, size);
						}
						if (i < x - 1 && map[j*size + i] != 4) {
							spread(i, j, i + 1, j, x, y, map, mat, size);
						}
						if (j > 0 && map[j*size + i] != 5) {
							spread(i, j, i, j - 1, x, y, map, mat, size);
						}
						if (j < y - 1 && map[j*size + i] != 5) {
							spread(i, j, i, j + 1, x, y, map, mat, size);
						}
					}
				}
			}
			if (solve(mat, bu, n)) {
				memset(bu, 0, size*size * sizeof(xort));
			}
			else {
				for (i = msy - 1; i > 0; i--) {
					for (j = msx - 1; j >= 0; j--) {
						xort temp;
						 temp= bu[i*msx + j];
						 bu[i*msx + j].v = 0;
						 bu[i*size + j] = temp;
					}
				}
				draw();
			}
			break;
		}
		case 'S':
			cs = 2;
			break;
		case 'E':
			cs = 3;
			break;
		case 'W':
			cs = 4;
			break;
		case 'D':
			cs = 5;
			break;
		case 'A':
			cs = 6;
		case 'Q':
			cs = 0;
			break;
		case 'X':
			for (int i = 0; i < size*size; i++) {
				map[i] = 1;
				state[i] = 0;
				bu[i].v = 0;
			}
			draw();
			break;
		default:
			break;
		}
		break;
	}
	case WM_KEYUP: {
		key[wParam & 0xff] = 0;
		switch (wParam) {
		default:
			cs = 1;
			break;
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

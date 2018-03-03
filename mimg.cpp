#include "mimg.h"
#include "resource.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

const int size = 2048;
HDC hdc1, hdcc, hdc2;
int mx, my;
int cx, cy;
int startt = 0;
double left=-1.9, bottom=-1.0, d=4.0/2048;
GLuint pbo;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void draw(void) {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();
	SwapBuffers(hdc1);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	MSG msg;
	HWND hWnd;
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow, &hWnd))
	{
		return FALSE;
	}
st:
	if (startt) {
		for (;;) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (startt) {
				if (cudacalc(left, bottom, d)) {
					MessageBoxA(hWnd, "evo failed", "message", MB_OK);
					break;
				}
				draw();
			}
			else {
				goto st;
			}
		}
	}
	else {
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (startt) {
				goto st;
			}
		}
	}
	return cudafin();
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND* hWnd)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	*hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, 1500, 1040, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(*hWnd, nCmdShow);
	UpdateWindow(*hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId) {
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		draw();
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CREATE: {
		SetTimer(hWnd, 1, 10, NULL);
		hdc1 = GetDC(hWnd);
		hdc2 = GetDC(NULL);
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
		int uds = ::ChoosePixelFormat(hdc1, &pfd);
		::SetPixelFormat(hdc1, uds, &pfd);
		HGLRC m_hrc = ::wglCreateContext(hdc1);
		::wglMakeCurrent(hdc1, m_hrc);
		glewInit();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		((bool(_stdcall*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(0);
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, size*size * 4, 0, GL_STREAM_DRAW_ARB);
		glBindTexture(GL_TEXTURE_2D, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (cudainit(pbo)) {
			PostQuitMessage(0);
		}
		break;
	}
	case WM_SIZE: {
		cx = lParam & 0xffff;
		cy = (lParam & 0xffff0000) >> 16;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(0, float(cx) / size*0.5, 0, float(cy) / size*0.5, 0.5, 40.0);
		glViewport(0, 0, cx, cy);
		break;
	}
	case WM_MOUSEMOVE: {
		int x, y;
		x = (lParam & 0xffff);
		y = ((lParam & 0xffff0000) >> 16);
		if (MK_LBUTTON&wParam) {
			left -= (x - mx)*d;
			bottom += (y - my)*d;
			glFinish();
			if (cudaimginit()) {
				PostQuitMessage(0);
			}
		}
		mx = x;
		my = y;
		break;
	}
	case WM_KEYDOWN: {
		switch (wParam) {
		case ' ': {
			startt = !startt;
			break;
		}
		}
		break;
	}
	case WM_MOUSEWHEEL: {
		short i = wParam >> 16, j;
		if (i>0) {
			for (j = 0; j < i; j+=120) {
				d *= 0.5;
				left += mx*d;
				bottom += (cy - 1 - my)*d;
			}
		}
		else {
			for (j = 0; j < -i; j+=120) {
				left -= mx*d;
				bottom -= (cy - 1 - my)*d;
				d *= 2;
			}
		}
		glFinish();
		if (cudaimginit()) {
			PostQuitMessage(0);
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
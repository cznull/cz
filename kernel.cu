
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "resource.h"	
#include <stdio.h>
#include <windows.h>
#include <SDKDDKVer.h>

//#include <math_functions.h>
#include <gl/glew.h>
#include <cuda_gl_interop.h>
#include <math.h>

#define MAX_LOADSTRING 100
#define FR 1000.0
#define DT 0.2f 

const int arraySize = 2048;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

float *device_a = 0, *device_b = 0, *device_c = 0, *device_d = 0, *device_m = 0;
int *device_img = 0, *device_ct = 0;
cudaArray* device_array;
float *partten;
int colortable[4096];
HDC hdc1, hdcc, hdc2;
HGLRC m_hrc;
GLuint pbo, texbuffer;
struct cudaGraphicsResource *cuda_pbo_resource;
size_t num_bytes;
int cx, cy;
cudaError_t cui(HWND hWnd);
int startt = 0;
unsigned char getr(double x);
unsigned char getg(double x);
unsigned char getb(double x);

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

__global__ void Kernelevo1(float *a, float *b, float *c, float *d, float *m)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int p = (y+2)*(arraySize+4) + x+2;
	d[p] = b[p] + ((a[p + arraySize + 4] + a[p - arraySize - 4] + a[p + 1] + a[p - 1])*16.0f - a[p + arraySize * 2 + 8] - a[p - arraySize * 2 - 8] - a[p + 2] - a[p - 2] - 60.0f*a[p])*0.08333333333f*DT;
	c[p] = a[p] + (d[p]) * m[p] * DT;
}
__global__ void Kernelevo2(float *a, float *b, float *m) {
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int p = (y + 2)*(arraySize + 4) + x + 2;
	a[p] += b[p] * m[p] * 0.2f;
}

__global__ void Kernelimg(float *a, int *imgbits, int *ct) {
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int p = (y + 2)*(arraySize + 4) + x + 2;
	int c = (a[p] + 0.5f) * 2048;
	c = max(0, min(4095, c));
	imgbits[y*arraySize + x] = ct[c];
}

unsigned char getr(double x) {
	return (tanh((x - 0.375) * 8) + 1) * 127;
}
unsigned char getg(double x) {
	return (tanh((x - 0.625) * 8) + 1) * 127;
}
unsigned char getb(double x) {
	return (exp(-25 * (x - 0.28)*(x - 0.25)) *0.5 + 1 + tanh((x - 0.875) * 8)) * 255 / 2;
}

void draw(HDC hdc, HWND hWnd, unsigned int f) {
	/*himg = CreateBitmapIndirect(&img);
	preimg = SelectObject(hdcc, himg);
	BitBlt(hdc, 0, 0, arraySize, arraySize, hdcc, 0, 0, SRCCOPY);
	SelectObject(hdcc, preimg);
	DeleteObject(himg);*/
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, arraySize, arraySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glClear(GL_COLOR_BUFFER_BIT); 
	//glBindTexture(GL_TEXTURE_2D, texbuffer);
	//glCopyPixels(0, 0, arraySize, arraySize, GL_COLOR);
	//glDrawPixels(arraySize, arraySize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();
	SwapBuffers(hdc);
}
cudaError_t cui(HWND hWnd) {
	cudaError_t cudaStatus;
	int i, j, k;
	double r;
	double r1[10];

	cudaStatus = cudaMalloc(&device_a, (arraySize + 4) *(arraySize + 4) * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "malloc failed", "message", MB_OK);
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_b, (arraySize + 4) *(arraySize + 4) * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "malloc failed", "message", MB_OK);
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_c, (arraySize + 4) *(arraySize + 4) * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "malloc failed", "message", MB_OK);
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_d, (arraySize + 4) *(arraySize + 4) * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "malloc failed", "message", MB_OK);
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_m, (arraySize + 4) *(arraySize + 4) * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "malloc failed", "message", MB_OK);
		goto Error;
	}

	/*cudaStatus = cudaMalloc(&device_img, arraySize*arraySize * sizeof(int));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "mallloc failed", "message", MB_OK);
		goto Error;
	}*/
	cudaGraphicsMapResources(1, &cuda_pbo_resource, 0);
	cudaStatus = cudaGraphicsResourceGetMappedPointer((void **)&device_img, &num_bytes, cuda_pbo_resource);
	//cudaStatus = cudaGraphicsSubResourceGetMappedArray(&device_array, cuda_pbo_resource,0,0); 
	cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0);
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "res failed", "message", MB_OK);
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_ct, 4096 * sizeof(int));
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "malloc failed", "message", MB_OK);
		goto Error;
	}

	partten = (float*)malloc((arraySize + 4) *(arraySize + 4) * sizeof(float));
	srand(GetTickCount());
	for (i = 0; i < arraySize + 4; i++) {
		for (j = 0; j < arraySize + 4; j++) {
			r = ((double)(i - 2) / arraySize - 0.5)*((double)(i - 2) / arraySize - 0.5) + ((double)(j - 2) / arraySize - 0.25)*((double)(j - 2) / arraySize - 0.25);
			//r = sqrt(r);
			partten[i*(arraySize + 4) + j] = (r < 0.06) ? 0.5 + 1.0*(exp(-140.0*r))*cos((double)(j - 2) / arraySize * FR) : 0.5;
		}
	}

	cudaStatus = cudaMemcpy(device_a, partten, (arraySize + 4) *(arraySize + 4) * sizeof(float), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "copy failed", "message", MB_OK);
		goto Error;
	}
	cudaStatus = cudaMemcpy(device_c, partten, (arraySize + 4) *(arraySize + 4) * sizeof(float), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "copy failed", "message", MB_OK);
		goto Error;
	}

	partten = (float*)malloc((arraySize + 4) *(arraySize + 4) * sizeof(float));
	srand(GetTickCount());
	for (i = 0; i < arraySize + 4; i++) {
		for (j = 0; j < arraySize + 4; j++) {
			r = ((double)(i - 2) / arraySize - 0.5)*((double)(i - 2) / arraySize - 0.5) + ((double)(j - 2) / arraySize - 0.25)*((double)(j - 2) / arraySize - 0.25);
			//r = sqrt(r);
			partten[i*(arraySize + 4) + j] = (r < 0.06) ? 1.0*FR / arraySize*(exp(-140.0*r))*sin((double)(j - 2 + DT / 2) / arraySize * FR) : 0.0;
		}
	}

	cudaStatus = cudaMemcpy(device_b, partten, (arraySize + 4) *(arraySize + 4) * sizeof(float), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "copy failed", "message", MB_OK);
		goto Error;
	}

	for (i = 0; i < arraySize + 4; i++) {
		for (j = 0; j < arraySize + 4; j++) {
			partten[i*(arraySize + 4) + j] = 1;
			for (k = 0; k < 61; k++) {
				r = ((double)(i - 2) / arraySize - k*0.01 - 0.2)*((double)(i - 2) / arraySize - k*0.01 - 0.2) + ((double)(j - 2) / arraySize - k*0.003 - 0.41)*((double)(j - 2) / arraySize - k*0.003 - 0.41);
				if (r < 0.000005) {
					partten[i*(arraySize + 4) + j] = 0;
					break;
				}
			}
		}
	}

	cudaStatus = cudaMemcpy(device_m, partten, (arraySize + 4) *(arraySize + 4) * sizeof(float), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "copy failed", "message", MB_OK);
		goto Error;
	}

	cudaStatus = cudaMemcpy(device_ct, colortable, 4096 * sizeof(int), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		MessageBoxA(hWnd, "copy failed", "message", MB_OK);
		goto Error;
	}
	
	return cudaStatus;

Error:
	cudaFree(device_a);
	cudaFree(device_b);
	return cudaStatus;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	MSG msg;
	HWND hWnd;
	int i, j;
	unsigned int t, t1, t2, count, f;
	cudaError_t cudaStatus;
	dim3 blocksize, gridsize;
	char s[32];
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow, &hWnd))
	{
		return FALSE;
	}
	blocksize = dim3(128, 1, 1);
	gridsize = dim3(arraySize / blocksize.x, arraySize / blocksize.y, 1);
	t = GetTickCount();
	count = 0;
	f = 0;
	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (startt) {
			for (i = 0; i < startt; i++) {
				Kernelevo1 << <gridsize, blocksize >> > (device_a, device_b, device_c, device_d, device_m);
				Kernelevo1 << <gridsize, blocksize >> > (device_c, device_d, device_a, device_b, device_m);
			}
			//cudaStatus = cudaGraphicsMapResources(1, &cuda_pbo_resource, 0); 
			/*if (cudaStatus != cudaSuccess) {
				MessageBoxA(hWnd, "evo failed", "message", MB_OK);
				break;
			}*/
			Kernelimg << <gridsize, blocksize >> > (device_a, device_img, device_ct);
			//cudaStatus = cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0);
			cudaStatus = cudaStatus = cudaDeviceSynchronize();
			if (cudaStatus != cudaSuccess) {
				MessageBoxA(hWnd, "evo failed", "message", MB_OK);
				break;
			}
			draw(hdc1, hWnd, f);
			count += 1;
			t1 = GetTickCount();
			if (t1 - t > 1000) {
				t += 1000;
				sprintf(s, "%dfps", count);
				TextOutA(hdc2, 0, 0, s, strlen(s));
				count = 0;
			}
		}
	}

	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		return 1;
	}
	return 0;
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

	*hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0, 1060, 1040, nullptr, nullptr, hInstance, nullptr);

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
	}
	break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		draw(hdc1, hWnd, 0);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CREATE: {
		cudaError_t cudaStatus;
		SetTimer(hWnd, 1, 10, NULL);
		hdc1 = GetDC(hWnd);
		hdc2 = GetDC(NULL);
		hdcc = CreateCompatibleDC(hdc1);

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
		m_hrc = ::wglCreateContext(hdc1);
		::wglMakeCurrent(hdc1, m_hrc);
		glewInit();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glClearColor(1.0, 1.0, 1.0, 1.0); 

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-0.5, 0.5, -0.5, 0.5, 0.5, 2.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, arraySize*arraySize * 4, 0, GL_STREAM_DRAW_ARB);

		glGenTextures(1, &texbuffer);
		glBindTexture(GL_TEXTURE_2D, texbuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glBindTexture(GL_TEXTURE_2D, 0);

		cudaStatus = cudaSetDevice(0);
		if (cudaStatus != cudaSuccess) {
			MessageBoxA(hWnd, "device failed", "message", MB_OK);
			PostQuitMessage(1);
		}
		
		cudaStatus = cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, texbuffer, cudaGraphicsMapFlagsWriteDiscard);
		//cudaStatus = cudaGraphicsGLRegisterImage(&cuda_pbo_resource, texbuffer, GL_TEXTURE_2D, cudaGraphicsMapFlagsWriteDiscard);
		if (cudaStatus != cudaSuccess) {
			MessageBoxA(hWnd, "pbo failed", "message", MB_OK);
			PostQuitMessage(1);
		}
		for (int i = 0; i < 4096; i++) {
			colortable[i] = (255 << 24) + (getb((double)i / 2048 - 0.5) << 16) + (getg((double)i / 2048 - 0.5) << 8) + getr((double)i / 2048 - 0.5);
		}
		cudaStatus = cui(hWnd);
		if (cudaStatus != cudaSuccess) {
			PostQuitMessage(1);
		}
		Kernelimg << <dim3(arraySize / 128, arraySize / 1), dim3(128, 1) >> > (device_a, device_img, device_ct);
		cudaStatus = cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			MessageBoxA(hWnd, "evo failed", "message", MB_OK);
			PostQuitMessage(1);
		}

		break;
	}
	case WM_TIMER: {
		break;
	}
	case WM_SIZE: {
		cx = lParam & 0xffff;
		cy = (lParam & 0xffff0000) >> 16;
		if (cx <= cy) {
			glViewport(0, (cy - cx) / 2, cx, cx);
		}
		else {
			glViewport((cx - cy) / 2, 0, cy, cy);
		}
		break;
	}
	case WM_KEYDOWN: {
		switch (wParam) {
		case ' ': {
			startt = 10 - startt;
			break;
		}
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
//glew32.lib;glu32.lib;opengl32.lib;
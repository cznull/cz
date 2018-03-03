
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "mimg.h"
#include <cuda_gl_interop.h>


const float pi = 3.1415926536f;
const int size = 2048;
double2 *device_p1 = NULL;
int *device_img = NULL, *device_ct = NULL;
char *pattern = NULL;
struct cudaGraphicsResource *cuda_vbo_resource;
int colortable[256];

unsigned char getr(double x) {
	return (tanh((x - 0.4) * 8) + 2 + tanh((0.2 - x) * 10) - exp(-(x - 1.0)*(x - 1.0) * 100)*0.05) * 127;
}
unsigned char getg(double x) {
	return (tanh((x - 0.70) * 8) + 2 + tanh((0.2 - x) * 10) - exp(-(x - 1.0)*(x - 1.0) * 100)*0.0) * 127;
}
unsigned char getb(double x) {
	return (tanh((0.45 - x) * 8) + 2.0 + tanh((x - 0.92) * 10) + exp(-(x - 1.0)*(x - 1.0) * 100)*0.2) * 255 / 2;
}


__global__ void evo(double2 *p1, int *img,int *ct, double left, double bottom, double d) {
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int i = x + y*size;
	int j;
	double2 a, b, c;
	a.x = left + d*x;
	a.y = bottom + d*y;
	b = p1[i];
	c.x = b.x*b.x;
	c.y = b.y*b.y;
	if (c.x + c.y > 4.0) {
		return;
	}
	b.y *= b.x;
	b.y += b.y + a.y;
	b.x = c.x - c.y + a.x;
	for (j = 0; j < 255; j++) {
		c.x = b.x*b.x;
		c.y = b.y*b.y;
		if (c.x + c.y > 4.0) {
			img[i] = ct[j];
			p1[i] = b;
			return;
		}
		b.y *= b.x;
		b.y += b.y + a.y;
		b.x = c.x - c.y + a.x;
	}
	c.x = b.x*b.x;
	c.y = b.y*b.y;
	if (c.x + c.y > 4.0) {
		img[i] = ct[255];
	}
	p1[i] = b;
}
__global__ void clear(double2 *p1, int *img) {
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;
	int i = x + y*size;
	p1[i].x = 0.0;
	p1[i].y = 0.0;
	img[i] = 0;
}
int cudainit(GLuint pbo) {
	cudaError_t cudaStatus;
	size_t num_bytes;
	int i;
	float x;
	pattern = (char*)malloc(size*size * 4);
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_p1, size*size * sizeof(double2));
	if (cudaStatus != cudaSuccess) {
		goto Error;
	}

	cudaStatus = cudaGraphicsGLRegisterBuffer(&cuda_vbo_resource, pbo, cudaGraphicsMapFlagsWriteDiscard);
	if (cudaStatus != cudaSuccess) {
		goto Error;
	}

	cudaStatus = cudaMalloc(&device_ct, 1024 * sizeof(int));
	if (cudaStatus != cudaSuccess) {
		goto Error;
	}

	for (i = 0; i < 256; i++) {
		colortable[i] = (255 << 24) + (getb((double)i / 256) << 16) + (getg((double)i / 256) << 8) + getr((double)i / 256);
	}

	cudaStatus = cudaMemcpy(device_ct, colortable, 256 * sizeof(int), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		goto Error;
	}

	cudaGraphicsMapResources(1, &cuda_vbo_resource, 0);
	cudaStatus = cudaGraphicsResourceGetMappedPointer((void **)&device_img, &num_bytes, cuda_vbo_resource);
	cudaGraphicsUnmapResources(1, &cuda_vbo_resource, 0);
	if (cudaStatus != cudaSuccess) {
		goto Error;
	}

	return cudaimginit();
Error:
	cudaFree(device_p1);
	return 1;
}

int cudacalc(double left, double bottom, double d) {
	cudaError cudaStatus;
	//cudaGraphicsMapResources(1, &cuda_vbo_resource, 0);
	evo << <dim3(256, 135), dim3(8, 8) >> > (device_p1, device_img,device_ct, left, bottom, d);
	cudaStatus = cudaDeviceSynchronize();
	//cudaGraphicsUnmapResources(1, &cuda_vbo_resource, 0);
	if (cudaStatus != cudaSuccess) {
		return 1;
	}
	return 0;
}

int cudafin(void) {
	cudaError cudaStatus;
	cudaFree(device_p1);
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		return 1;
	}
	return 0;
}


int cudaimginit(void) {
	cudaError cudaStatus;
	clear << <dim3(16, 2048), dim3(128, 1) >> > (device_p1, device_img);
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		return 1;
	}
	return 0;
}
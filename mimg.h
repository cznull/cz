#pragma once
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <gl/glew.h>
#include <math.h>
#include <stdio.h>

int cudainit(GLuint);
int cudacalc(double x0,double y0,double d);
int cudafin(void);
int cudaimginit(void);
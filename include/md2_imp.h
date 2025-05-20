#pragma once

#ifdef _arch_dreamcast
#include "GL/glkos.h"
#include "GL/glu.h"
#include "GL/glext.h"
#include "kos.h"
#else
#include "GL/freeglut.h"
#endif

#include "stdint.h"

GLuint loadTexture(const char* filename);
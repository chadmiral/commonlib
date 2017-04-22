#pragma once

#if defined(_WIN32)
#include <Windows.h>
#if defined __LOKI__
#include <GL/gl3w.h>
#else
#include <GL/glew.h>
#endif //__LOKI__
#include <GL/gl.h>
#endif //_WIN32

#if defined(__APPLE__)
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif
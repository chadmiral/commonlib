#pragma once

//NOTE: glew *must* be included before gl.h
#if defined(_WIN32)
#if defined(__LOKI__)
#include <GL/gl3w.h>
#else
#include <GL/glew.h>
#endif //__LOKI__
#endif //_WIN32

#if defined(_WIN32)
#include <Windows.h>
#include <GL/gl.h>
#endif //_WIN32

#if defined(__APPLE__)
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

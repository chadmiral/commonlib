#ifndef __GL_ERROR_H__
#define __GL_ERROR_H__

#include <iostream>
#include "platform_gl.h"

#if defined (_GL_DEBUG)
namespace Graphics
{
  void gl_check_error(std::ostream &log = std::cerr);
}
#else
namespace Graphics {}
#define gl_check_error(A)
#endif //_GL_DEBUG

#endif

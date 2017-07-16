#include "gl_error.h"

#include <iostream>
#include <assert.h>
#include "platform.h"

#if defined (__APPLE__)
#include <OpenGL/gl.h>
#endif

#if defined (_GL_DEBUG)

void Graphics::gl_check_error(std::ostream &log)
{
  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
  {
    log << __CONSOLE_LOG_RED__;
    switch (err)
    {
    case GL_INVALID_ENUM:
      log << "GL_INVALID_ENUM:" << std::endl;
      log << "An unacceptable value is specified for an enumerated argument." << std::endl;
      log << "The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
      break;

    case GL_INVALID_VALUE:
      log << "GL_INVALID_VALUE:" << std::endl;
      log << "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
      break;

    case GL_INVALID_OPERATION:
      log << "GL_INVALID_OPERATION:" << std::endl;
      log << "The specified operation is not allowed in the current state." << std::endl;
      log << "The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
      break;

    case GL_INVALID_FRAMEBUFFER_OPERATION:
      log << "GL_INVALID_FRAMEBUFFER_OPERATION: " << std::endl;
      log << "The framebuffer object is not complete." << std::endl;
      log << "The offending command is ignored and has no other side effect than to set the error flag." << std::endl;
      break;

    case GL_OUT_OF_MEMORY:
      log << "GL_OUT_OF_MEMORY:" << std::endl;
      log << "There is not enough memory left to execute the command." << std::endl;
      log << "The state of the GL is undefined, except for the state of the error flags, after this error is recorded." << std::endl;
      break;

    case GL_STACK_UNDERFLOW:
      log << "GL_STACK_UNDERFLOW:" << std::endl;
      log << "An attempt has been made to perform an operation that would cause an internal stack to underflow." << std::endl;
      break;

    case GL_STACK_OVERFLOW:
      log << "GL_STACK_OVERFLOW:" << std::endl;
      log << "An attempt has been made to perform an operation that would cause an internal stack to overflow." << std::endl;
      break;
    }
    assert(false);
  }
}

#endif //_GL_DEBUG

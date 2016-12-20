#ifndef _GL_UTILITY_H
#define _GL_UTILITY_H

#include "platform_gl.h"

namespace Graphics
{
  /*
  template class T;
  class GeometryFactory<T>
  {
  public:
    GeometryFactory() {}
    ~GeometryFactory() {}

     StaticMesh<T> generate_sphere();
  };
  */


  //legacy
  void gl_sphere(const float *pos, const float *_dim, const float _radius);
  void gl_cube(const float *_pos, const float *_dim);
  void gl_cube(const float *_pos, const float *_dim, const float theta_z);
}

#endif //_GL_UTILITY_H

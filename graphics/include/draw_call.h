#ifndef __DRAW_CALL_H__
#define __DRAW_CALL_H__

#include "platform_gl.h"
#include "material.h"

namespace Graphics
{
  template <class VertexType>
  class DrawCall
  {
  public:
    DrawCall()
    {
      num_indices = 0;
      vbo = -1;
      ibo = -1;
      mat = NULL;
    }

    ~DrawCall() {}

    void set_num_indices(const int n) { num_indices = n; }

    void set_buffer_objects(const GLuint _vbo, const GLuint _ibo)
    {
      vbo = _vbo;
      ibo = _ibo;
    }

    void set_material(Material *m) { mat = m; }

    void render();

  private:
    Material *mat;

    int num_indices;
    unsigned int *indices;

    int num_vertices;
    VertexType *vertex_data;

    GLuint vbo;
    GLuint ibo;
  };
};

#endif

#ifndef __STATIC_MESH_H__
#define __STATIC_MESH_H__

#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#include <GL/glew.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include "math_utility.h"

#include "material.h"
#include "draw_call.h"

namespace Graphics
{
  struct StaticMeshVertex
  {
    float x, y, z;
    float r, g, b;
    float nx, ny, nz;
    float u0, v0;
  };

  class StaticMesh
  {
  public:
    StaticMesh();
    ~StaticMesh();

    void read_from_file(FILE *f, bool verbose = false);
    void init();
    void render(Material *m = NULL, GLenum primitive_type = GL_TRIANGLES);

  protected:
    std::vector<DrawCall<StaticMeshVertex> > draw_calls;

    int                    num_vertices;
    StaticMeshVertex       *vertices;

    int num_indices;
    unsigned int *indices;

    GLuint vbo;
    GLuint ibo;
  };
};

#endif // __STATIC_MESH_H__

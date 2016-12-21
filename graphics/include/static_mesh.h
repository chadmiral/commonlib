#ifndef __STATIC_MESH_H__
#define __STATIC_MESH_H__

#include <vector>

#include "platform_gl.h"

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

  /*
  enum MeshVertexAttribs
  {
    MESH_VERTEX_ATTRIB_XYZ,
    MESH_VERTEX_ATTRIB_RGB,
    MESH_VERTEX_ATTRIB_NORMAL,
    MESH_VERTEX_ATTRIB_UV0,

    NUM_MESH_VERTEX_ATTRIBS
  };
  */

  class StaticMesh
  {
  public:
    StaticMesh();
    ~StaticMesh();

    void read_from_file(FILE *f, bool verbose = false);
    void init();
    void render(Material *m = NULL, GLenum primitive_type = GL_TRIANGLES);

    void set_data(uint32_t num_verts, StaticMeshVertex *verts, uint32_t num_indices, uint32_t *indices);

  protected:
    std::vector<DrawCall<StaticMeshVertex> > draw_calls;

    //ShaderVertexAttrib      _vertex_attribs[NUM_MESH_VERTEX_ATTRIBS];

    uint32_t                _num_vertices;
    StaticMeshVertex       *_vertices;

    uint32_t                _num_indices;
    uint32_t               *_indices;

    GLuint                  _vbo;
    GLuint                  _ibo;
  };
};

#endif // __STATIC_MESH_H__

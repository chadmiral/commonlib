#pragma once

#include <iostream>
#include "material.h"

//Child class is responsible for allocation / deallocating vertex / index buffers and buffer objects

namespace Graphics
{
  template <class VertexType>
  class Renderable
  {
  protected:
    Material     *material;

    uint32_t     num_vertices;
    VertexType   *vertex_data;

    uint32_t     num_indices;
    uint32_t     *index_data;

    GLuint       vbo;
    GLuint       ibo;

    GLenum       geo_mode;
    GLenum       gl_usage;

    void allocate_buffers()
    {
      deallocate_buffers();
      vertex_data = new VertexType[num_vertices];
      index_data = new unsigned int[num_indices];
    }

    void deallocate_buffers()
    {
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ibo);
      if (vertex_data) { delete vertex_data; }
      if (index_data) { delete index_data; }
    }

    void init_buffers()
    {
      //create vbo / ibo
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(VertexType) * num_vertices, vertex_data, gl_usage);

      glGenBuffers(1, &ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * num_indices, index_data, gl_usage);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  public:
    Renderable()
    {
      material = NULL;
      vertex_data = NULL;
      index_data = NULL;
      geo_mode = GL_QUAD_STRIP;
      gl_usage = GL_STATIC_DRAW;
    }
    ~Renderable() {}

    virtual void init() = 0;
    virtual void simulate(const double game_time, const double frame_time) = 0;
    virtual void render(const double game_time) = 0;

    void set_material(Graphics::Material *m) { material = m; }
    void set_geometry_mode(GLenum gm) { geo_mode = gm; }
  };
};

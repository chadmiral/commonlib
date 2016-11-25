#include "billboard.h"

using namespace Graphics;

void Billboard::init()
{
  num_vertices = 4;
  num_indices = 4;

  geo_mode = GL_QUADS;//GL_TRIANGLE_STRIP;
  gl_usage = GL_STATIC_DRAW;

  Renderable::allocate_buffers();

  //
  // vertex order:
  //
  // 0---1
  // | / |
  // 3---2
  //

  float size = 23.0f;

  index_data[0] = 0;
  vertex_data[0].x = -size;
  vertex_data[0].y =  size;
  vertex_data[0].z =  0.0f;
  vertex_data[0].u0 = 0.0f;
  vertex_data[0].v0 = 0.0f;

  index_data[1] = 1;
  vertex_data[1].x =  size;
  vertex_data[1].y =  size;
  vertex_data[1].z =  0.0f;
  vertex_data[1].u0 = 1.0f;
  vertex_data[1].v0 = 0.0f;

  index_data[2] = 2;
  vertex_data[2].x =  size;
  vertex_data[2].y = -size;
  vertex_data[2].z =  0.0f;
  vertex_data[2].u0 = 1.0f;
  vertex_data[2].v0 = 1.0f;

  index_data[3] = 3;
  vertex_data[3].x = -size;
  vertex_data[3].y = -size;
  vertex_data[3].z =  0.0f;
  vertex_data[3].u0 = 0.0f;
  vertex_data[3].v0 = 1.0f;

  Renderable::init_buffers();
}

void Billboard::render(const double game_time)
{
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  material->render();

  gl_check_error();

  
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(BillboardVertex), (void *)0);

  gl_check_error();

  glClientActiveTexture(GL_TEXTURE0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(BillboardVertex), (void *)(sizeof(float) * 3));
  
  glDrawElements(geo_mode, num_indices, GL_UNSIGNED_INT, (void *)0);

  gl_check_error();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  gl_check_error();

  glClientActiveTexture(GL_TEXTURE0);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  material->cleanup();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
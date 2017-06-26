#include "lens_flare.h"

using namespace Graphics;
using namespace Math;

/*
LensFlareElement() :
  Renderable<LensFlareVertex>()
{
  _position_offset = pos_offset;
  _scale = scale;
  _rotation_offset = rot_offset;
  set_material(mat);
  set_geometry(GL_QUADS);
}
*/

void LensFlareElement::init()
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
  
  float size = 0.5f;
  
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

  assert(material);

  //collect shader uniform variables
  _proj_mat = (ShaderUniformMatrix4x4 *)material->find_uniform(Math::hash_value_from_string("proj_mat"));
  _center_point = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_center_point"));
  _screen_resolution = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_resolution"));
  _tint = (ShaderUniformFloat4 *)material->find_uniform(Math::hash_value_from_string("sun_lens_flare_tint"));
  _scale = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_scale"));
  _position_offset = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_offset"));

  _tint->set_var(Float4(random(0.0f, 1.0f), random(0.0f, 1.0f), random(0.0f, 1.0f), 1.0f));
  _scale->set_var(Float2(random(0.0f, 1.0f), random(0.0f, 1.0f)));
  _position_offset->set_var(Float2(random(-0.5f, 0.5f), random(-0.5f, 0.5f)));
}

void LensFlareElement::simulate(const double game_time, const double frame_time)
{
  
}

void LensFlareElement::render(const double game_time)
{
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  
  material->render();
    glDrawElements(geo_mode, num_indices, GL_UNSIGNED_INT, (void *)0);
  material->cleanup();
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

LensFlare::LensFlare()
{
}

LensFlare::~LensFlare()
{
}

//this requires a draw-call per element - not exactly optimal, but allows
//for flexibility with having different materials per element
//if that proves inefficient, a good optimization would be to use a single
//material and draw call for all lens flare elements
void LensFlare::init(const double game_time)
{
  Object3D::init(game_time);
  for(uint32_t i = 0; i < _elements.size(); i++)
  {
    _elements[i].init();
  }
}

void LensFlare::simulate(const double game_time, const double frame_time)
{
  Object3D::simulate(game_time, frame_time);
}

void LensFlare::render(const double game_time)
{
  for(uint32_t i = 0; i < _elements.size(); i++)
  {
    _elements[i].render(game_time);
  }
}

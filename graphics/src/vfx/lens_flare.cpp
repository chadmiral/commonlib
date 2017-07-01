#include "lens_flare.h"

using namespace Graphics;
using namespace Math;

// The lens flare code was largely written in the summer of 2017. Lila and I are living in a 2 bedroom house
// in the Central District in Seattle with our baby pupper Freyja. I largely got this code working in the week I
// finaled Destiny 2, and now, finishing up the code, Lila and I are on a plane bound to Champaign, IL to visit
// my parents.


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
  _center_point = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_flare_pos"));
  _screen_resolution = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_resolution"));
  _tint = (ShaderUniformFloat4 *)material->find_uniform(Math::hash_value_from_string("sun_lens_flare_tint"));
  _scale = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_scale"));
  _position_offset = (ShaderUniformFloat2 *)material->find_uniform(Math::hash_value_from_string("screen_offset"));
  _rotation_offset = (ShaderUniformFloat *)material->find_uniform(Math::hash_value_from_string("rotation_offset"));
  _rotation_speed = (ShaderUniformFloat *)material->find_uniform(Math::hash_value_from_string("rotation_speed"));
  _element_depth = (ShaderUniformFloat *)material->find_uniform(Math::hash_value_from_string("element_depth"));
  _brightness = (ShaderUniformFloat *)material->find_uniform(Math::hash_value_from_string("brightness"));
  _eye_dot_flare = (ShaderUniformFloat *)material->find_uniform(Math::hash_value_from_string("eye_dot_flare"));
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
  _occlusion_mesh = NULL;
  _occlusion_radius = 1.0f;
  _occlusion_pct = 0.0f;

  _screen_gunk_contribution = 1.0f;
  _eye_dot_flare = 1.0f;
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
    _elements[i]._eye_dot_flare->set_var(_eye_dot_flare);
    _elements[i]._brightness->set_var(_occlusion_pct);
    _elements[i].render(game_time);
  }
}

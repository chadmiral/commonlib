// originally written while on vacation in Vancouver, B.C. with Lila Garcia
// we ate and drank at various restaurants in Gastown and stayed in a condo airBnB.
// Lila nearly got electrocuted when she tried to plug in the coffee grinder,
// and the outlet shot out sparks and melted a spatula. (!!!)
// I cannot describe my excitement the first time I got HDR bloom to work. :)

#include <assert.h>
#include "render_surface.h"
#include "texture.h"
#include "gl_error.h"

using namespace std;
using namespace Graphics;
using namespace Math;

RenderSurface::RenderSurface(const uint16_t w, const uint16_t h)
{
  _index_data[0] = 0;
  _index_data[1] = 1;
  _index_data[2] = 2;
  _index_data[3] = 3;

  _vertex_data[0].x =  -1.0f;
  _vertex_data[0].y =  -1.0f;
  _vertex_data[0].z =   0.0f;
  _vertex_data[0].u =   0.0f;
  _vertex_data[0].v =   0.0f;

  _vertex_data[1].x =   1.0f;
  _vertex_data[1].y =  -1.0f;
  _vertex_data[1].z =   0.0f;
  _vertex_data[1].u =   1.0f;
  _vertex_data[1].v =   0.0f;

  _vertex_data[2].x =   1.0f;
  _vertex_data[2].y =   1.0f;
  _vertex_data[2].z =   0.0f;
  _vertex_data[2].u =   1.0f;
  _vertex_data[2].v =   1.0f;

  _vertex_data[3].x =  -1.0f;
  _vertex_data[3].y =   1.0f;
  _vertex_data[3].z =   0.0f;
  _vertex_data[3].u =   0.0f;
  _vertex_data[3].v =   1.0f;

  _vbo = 0;
  _ibo = 0;

  _target_fbo = 0;
  _target_tex = 0;

  _depth_buffer_is_mine = true;
  _use_depth = true;

  _tex_internal_format =  GL_RGB;//GL_RGBA16F_ARB;
  _tex_format =           GL_RGB;
  _tex_type =             GL_UNSIGNED_BYTE;//GL_HALF_FLOAT_ARB; //GL_FLOAT;
  _tex_filter =           GL_LINEAR;

  _fbo_res[0] = w;
  _fbo_res[1] = h;

  _depth_tex = NULL;
  _target_tex = NULL;
}

RenderSurface::~RenderSurface()
{
  deinit();
}

Material *RenderSurface::add_shader(Shader *s, std::string name, bool add_surface_tex)
{
  Material *m = new Material;

  m->set_shader(s);

  //add vertex attributes (used by every method)
  ShaderVertexAttrib xyz_sva, uv0_sva;
  xyz_sva.set_loc(s, "in_xyz", sizeof(RenderSurfaceVert), 3, 0 * sizeof(float));
  uv0_sva.set_loc(s, "in_uv0", sizeof(RenderSurfaceVert), 2, 3 * sizeof(float));
  _xyz_attribs.push_back(xyz_sva);
  _uv0_attribs.push_back(uv0_sva);

  m->enable_blending(false);
  m->enable_depth_write(false);
  m->enable_depth_read(false);
  m->enable_lighting(false);
  m->set_depth_range(Float2(0.0f, 1.0f));
  m->enable_backface_culling(false);

  _materials.push_back(m);
  _method_names.push_back(name);
  _add_surface_textures.push_back(add_surface_tex);

  return m;
}

void RenderSurface::create_target_texture()
{
  _target_tex = new Texture2D(_fbo_res[0], _fbo_res[1]);

  _target_tex->set_tex_format(_tex_format);
  _target_tex->set_internal_format(_tex_internal_format);
  _target_tex->set_data_format(_tex_type);
  _target_tex->set_filtering_mode(_tex_filter);
  _target_tex->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  _target_tex->set_resolution(_fbo_res[0], _fbo_res[1]);

  _target_tex->init();
}

void RenderSurface::create_depth_texture()
{
  _depth_tex = new Texture2D(_fbo_res[0], _fbo_res[1]);

  _depth_tex->set_tex_format(GL_DEPTH_COMPONENT);
  _depth_tex->set_internal_format(GL_DEPTH_COMPONENT32F);
  _depth_tex->set_data_format(GL_FLOAT);//GL_UNSIGNED_BYTE);
  _depth_tex->set_filtering_mode(GL_LINEAR);
  _depth_tex->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  _depth_tex->set_resolution(_fbo_res[0], _fbo_res[1]);

  _depth_tex->init();
}

void RenderSurface::bind_textures_to_fbo()
{
  glBindFramebuffer(GL_FRAMEBUFFER, _target_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _target_tex->get_tex_id(), 0);
  if (_use_depth)
  {
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth_tex->get_tex_id(), 0);
  }

  GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  //undbind the hdr render target (so that we're not rendering to it by default)
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSurface::delete_frame_buffer_object()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (_target_fbo != 0)
  {
    glDeleteFramebuffers(1, &_target_fbo);
  }
  _target_fbo = 0;
}

void RenderSurface::init()
{
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(_vertex_data), _vertex_data, GL_STATIC_DRAW);

  glGenBuffers(1, &_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 4, _index_data, GL_STATIC_DRAW);

  if (!_target_tex) { create_target_texture(); }
  if (_use_depth) { create_depth_texture(); }

  //this needs to happen *after* the target texture has been created
  for (uint16_t i = 0; i < _materials.size(); i++)
  {
    if(_add_surface_textures[i])
    {
      _materials[i]->add_texture(_target_tex, std::string("surface_tex"));
    }
    _materials[i]->add_vertex_attrib(&_xyz_attribs[i]);
    _materials[i]->add_vertex_attrib(&_uv0_attribs[i]);
    _materials[i]->init();
  }

  glGenFramebuffers(1, &_target_fbo);
  bind_textures_to_fbo();
}

void RenderSurface::deinit()
{
  delete_frame_buffer_object();

  if (_depth_tex && _depth_buffer_is_mine) { delete _depth_tex; }
  if (_target_tex) { delete _target_tex; }
}

void RenderSurface::resize(const uint16_t w, const uint16_t h)
{
  _fbo_res[0] = w;
  _fbo_res[1] = h;

  //delete_frame_buffer_object();
  if (_depth_tex && _depth_buffer_is_mine) { delete _depth_tex; }
  if (_target_tex) { delete _target_tex; }

  if (_use_depth && _depth_buffer_is_mine) { create_depth_texture(); }
  create_target_texture();
  bind_textures_to_fbo();
}

float RenderSurface::compute_frame_luminosity() const
{
  GLuint tex_id = _target_tex->get_tex_id();
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glGenerateMipmap(GL_TEXTURE_2D);

  uint32_t largest_dim = max(_fbo_res[0], _fbo_res[1]);

  GLint lowest_mip_level = (GLint)(min((int)floor(log2(largest_dim)), GL_TEXTURE_MAX_LEVEL) - 1);

  float pixels[4];

  //TODO: do we need to match type / format to the texture format we are querying?
  GLenum type = GL_FLOAT; //GL_UNSIGNED_BYTE
  GLenum format = GL_RGBA;
  glGetTexImage(GL_TEXTURE_2D, lowest_mip_level + 1, format, type, pixels);

  return (pixels[0] + pixels[1] + pixels[2]) / 3.0f;
}

void RenderSurface::attach_depth_buffer(Texture2D *d_tex)
{
  glBindFramebuffer(GL_FRAMEBUFFER, _target_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, d_tex->get_tex_id(), 0);
  _use_depth = true;
  _depth_buffer_is_mine = false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSurface::capture()
{
  glBindFramebuffer(GL_FRAMEBUFFER, _target_fbo);
  glGetIntegerv(GL_VIEWPORT, _win_viewport);
  glViewport(0, 0, _fbo_res[0], _fbo_res[1]);
}

void RenderSurface::release()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(_win_viewport[0], _win_viewport[1], _win_viewport[2], _win_viewport[3]);
}

void RenderSurface::render(const uint16_t method)
{
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

  _materials[method]->render();
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);
  _materials[method]->cleanup();
}

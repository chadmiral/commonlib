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
  index_data[0] = 0;
  index_data[1] = 1;
  index_data[2] = 2;
  index_data[3] = 3;

  vertex_data[0].x =  -1.0f;
  vertex_data[0].y =  -1.0f;
  vertex_data[0].z =   0.0f;
  vertex_data[0].u =   0.0f;
  vertex_data[0].v =   0.0f;

  vertex_data[1].x =   1.0f;
  vertex_data[1].y =  -1.0f;
  vertex_data[1].z =   0.0f;
  vertex_data[1].u =   1.0f;
  vertex_data[1].v =   0.0f;

  vertex_data[2].x =   1.0f;
  vertex_data[2].y =   1.0f;
  vertex_data[2].z =   0.0f;
  vertex_data[2].u =   1.0f;
  vertex_data[2].v =   1.0f;

  vertex_data[3].x =  -1.0f;
  vertex_data[3].y =   1.0f;
  vertex_data[3].z =   0.0f;
  vertex_data[3].u =   0.0f;
  vertex_data[3].v =   1.0f;

  vbo = 0;
  ibo = 0;

  target_fbo = 0;
  target_tex = 0;

  use_depth = true;
  depth_fbo = 0;

  tex_internal_format =  GL_RGB;//GL_RGBA16F_ARB;
  tex_format =           GL_RGB;
  tex_type =             GL_UNSIGNED_BYTE;//GL_HALF_FLOAT_ARB; //GL_FLOAT;
  tex_filter =           GL_LINEAR;

  fbo_res[0] = w;
  fbo_res[1] = h;

  depth_tex = NULL;
  target_tex = NULL;
}

RenderSurface::~RenderSurface()
{
  deinit();
}

Material *RenderSurface::add_shader(Shader *s, std::string name)
{
  Material *m = new Material;

  m->set_shader(s);

  //add vertex attributes (used by every method)
  ShaderVertexAttrib xyz_sva, uv0_sva;
  xyz_sva.set_loc(s, "in_xyz", sizeof(RenderSurfaceVert), 3, 0 * sizeof(float));
  uv0_sva.set_loc(s, "in_uv0", sizeof(RenderSurfaceVert), 2, 3 * sizeof(float));
  _xyz_attribs.push_back(xyz_sva);
  _uv0_attribs.push_back(uv0_sva);
  //m->add_vertex_attrib(&_xyz_attribs[_xyz_attribs.size() - 1]);
  //m->add_vertex_attrib(&_uv0_attribs[_uv0_attribs.size() - 1]);

  //add shader uniforms (used by every method)
  ShaderUniformMatrix4x4 proj_uniform;
  Matrix4x4 proj_mat;
  proj_mat.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f);
  proj_uniform.set_name("proj_mat");
  proj_uniform.set_var(proj_mat);
  _projection_matrices.push_back(proj_uniform);

  //m->add_uniform_var(&(_projection_matrices[_projection_matrices.size() - 1]));

  m->enable_blending(false);
  m->enable_depth_write(false);
  m->enable_depth_read(false);
  m->enable_lighting(false);
  m->set_depth_range(Float2(0.0f, 1.0f));
  m->enable_backface_culling(false);

  _materials.push_back(m);
  _method_names.push_back(name);

  return m;
}

void RenderSurface::create_target_texture()
{
  target_tex = new Texture2D(fbo_res[0], fbo_res[1]);

  target_tex->set_tex_format(tex_format);
  target_tex->set_internal_format(tex_internal_format);
  target_tex->set_data_format(tex_type);
  target_tex->set_filtering_mode(tex_filter);
  target_tex->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  target_tex->set_resolution(fbo_res[0], fbo_res[1]);

  target_tex->init();
}

void RenderSurface::create_depth_texture()
{
  depth_tex = new Texture2D(fbo_res[0], fbo_res[1]);

  depth_tex->set_tex_format(GL_DEPTH_COMPONENT);
  depth_tex->set_internal_format(GL_DEPTH_COMPONENT32);
  depth_tex->set_data_format(GL_FLOAT);//GL_UNSIGNED_BYTE);
  depth_tex->set_filtering_mode(GL_LINEAR);
  depth_tex->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  depth_tex->set_resolution(fbo_res[0], fbo_res[1]);

  depth_tex->init();
}

void RenderSurface::bind_textures_to_fbo()
{
  glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_tex->get_tex_id(), 0);
  if (use_depth)
  {
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex->get_tex_id(), 0);
  }

  GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  //undbind the hdr render target (so that we're not rendering to it by default)
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSurface::delete_frame_buffer_object()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &target_fbo);
}

void RenderSurface::init()
{
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 4, index_data, GL_STATIC_DRAW);

  create_target_texture();
  if (use_depth) { create_depth_texture(); }

  //this needs to happen *after* the target texture has been created
  for (uint16_t i = 0; i < _materials.size(); i++)
  {
    _materials[i]->add_texture(target_tex, std::string("surface_tex"));
    _materials[i]->add_vertex_attrib(&_xyz_attribs[i]);
    _materials[i]->add_vertex_attrib(&_uv0_attribs[i]);
    _materials[i]->add_uniform_var(&_projection_matrices[i]);
    _materials[i]->init();
  }

  glGenFramebuffers(1, &target_fbo);
  bind_textures_to_fbo();
}

void RenderSurface::deinit()
{
  delete_frame_buffer_object();

  if (depth_tex) { delete depth_tex; }
  if (target_tex) { delete target_tex; }
}

void RenderSurface::resize(const uint16_t w, const uint16_t h)
{
  fbo_res[0] = w;
  fbo_res[1] = h;

  //delete_frame_buffer_object();
  if (depth_tex) { delete depth_tex; }
  if (target_tex) { delete target_tex; }

  if (use_depth) { create_depth_texture(); }
  create_target_texture();

  bind_textures_to_fbo();
}

void RenderSurface::capture()
{
  glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);
  glGetIntegerv(GL_VIEWPORT, win_viewport);
  glViewport(0, 0, fbo_res[0], fbo_res[1]);
}

void RenderSurface::release()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(win_viewport[0], win_viewport[1], win_viewport[2], win_viewport[3]);
}

void RenderSurface::render(const uint16_t method)
{
  //TODO: eliminate need for fixed pipeline matrices entirely
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  _materials[method]->render();
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);
  _materials[method]->cleanup();
}

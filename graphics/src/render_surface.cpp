// originally written while on vacation in Vancouver, B.C. with Lila Garcia
// we ate and drank at various restaurants in Gastown and stayed in a condo airBnB.
// Lila nearly got electrocuted when she tried to plug in the coffee grinder,
// and the outlet shot out sparks and melted a spatula. (!!!)
// I cannot describe my excitement the first time I got HDR bloom to work. :)

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include <assert.h>
#include "render_surface.h"
#include "texture.h"
#include "gl_error.h"

using namespace std;
using namespace Graphics;
using namespace Math;

RenderSurface::RenderSurface(const int w, const int h)
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

  target_tex = new Texture2D(fbo_res[0], fbo_res[1]);
  depth_tex = new Texture2D(fbo_res[0], fbo_res[1]);
}

RenderSurface::~RenderSurface()
{
  deinit();
  delete target_tex;
}

void RenderSurface::init()
{
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 4, index_data, GL_STATIC_DRAW);

  target_tex->set_tex_format(tex_format);
  target_tex->set_internal_format(tex_internal_format);
  target_tex->set_data_format(tex_type);
  target_tex->set_filtering_mode(tex_filter);
  target_tex->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  target_tex->set_resolution(fbo_res[0], fbo_res[1]);
  target_tex->init();

  //create depth texture
  if (use_depth)
  {
    depth_tex->set_tex_format(GL_DEPTH_COMPONENT);
    depth_tex->set_internal_format(GL_DEPTH_COMPONENT32);
    depth_tex->set_data_format(GL_FLOAT);//GL_UNSIGNED_BYTE);
    depth_tex->set_filtering_mode(GL_LINEAR);
    depth_tex->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    depth_tex->set_resolution(fbo_res[0], fbo_res[1]);
    depth_tex->init();
  }

  //Set up material
  Matrix4x4 proj;
  proj.ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f);
  _projection_matrix.set_name("proj_mat");
  _projection_matrix.set_loc(mat.get_shader());
  _projection_matrix.set_var(proj);

  mat.add_uniform_var(&_projection_matrix);

  Shader *s = mat.get_shader();
  assert(s); //This should have been set already with set_shader()
  _xyz_attrib.set_loc(s, "in_xyz", sizeof(RenderSurfaceVert), 3, 0 * sizeof(float));
  _uv0_attrib.set_loc(s, "in_uv0", sizeof(RenderSurfaceVert), 2, 3 * sizeof(float));

  mat.add_vertex_attrib(&_xyz_attrib);
  mat.add_vertex_attrib(&_uv0_attrib);

  mat.add_texture(target_tex, std::string("surface_tex"));

  mat.enable_blending(false);
  mat.enable_depth_write(false);
  mat.enable_depth_read(false);
  mat.enable_lighting(false);
  mat.set_depth_range(Float2(0.0f, 1.0f));
  mat.enable_backface_culling(false);

  mat.init();

  //use_depth = false;

  // create depth renderbuffer
  /*if(use_depth)
  {
    glGenRenderbuffers(1, &depth_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_fbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo_res[0], fbo_res[1]);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }
  else
  {
    depth_fbo = 0;
  }*/


  // create FBO
  glGenFramebuffers(1, &target_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_tex->get_tex_id(), 0);
  if(use_depth)
  {
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex->get_tex_id(), 0);
  }

  GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  //undbind the hdr render target (so that we're not rendering to it by default)
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderSurface::deinit()
{

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

void RenderSurface::render()
{
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  mat.render();

  /*
  glActiveTexture(GL_TEXTURE0); glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1); glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE2); glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE3); glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE4); glDisable(GL_TEXTURE_2D);
  */

  //TODO: vertex attribs
  /*
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(RenderSurfaceVert), (void *)0);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(RenderSurfaceVert), (void *)(sizeof(float) * 3));
  */
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);

  mat.cleanup();
}

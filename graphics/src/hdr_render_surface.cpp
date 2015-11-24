#include "hdr_render_surface.h"

using namespace std;

HDRRenderSurface::HDRRenderSurface(const int w, const int h) : RenderSurface(w, h)
{
  exposure = 1.0f;
  bloom_threshold = 0.9f;
}

HDRRenderSurface::~HDRRenderSurface()
{
  deinit();
}

void HDRRenderSurface::init()
{
  std::string vs_name("../data/shaders/passthrough.vs");
  std::string fs_name("../data/shaders/down_sample.fs");
  set_shader_names(vs_name, fs_name);

  set_internal_format(GL_RGBA16F_ARB);
  set_filtering_mode(GL_LINEAR);

  RenderSurface::init();
}

void HDRRenderSurface::deinit()
{
  RenderSurface::deinit();
}

void HDRRenderSurface::render()
{
  Shader *shader = mat.get_shader();
  mat.render_gl(); //material needs to be bound for the uniforms to be set.

  /*GLint exposure_loc = glGetUniformLocation(shader->gl_shader_program, "exposure");
  glUniform1f(exposure_loc, exposure);
  GLint bloom_threshold_loc = glGetUniformLocation(shader->gl_shader_program, "bloom_threshold");
  glUniform1f(bloom_threshold_loc, bloom_threshold);*/

  //cout<<"win_dim: "<<win_viewport[2]<<win_viewport[3]<<endl;
  Float2 texel_size(1.0f / (float)fbo_res[0], 1.0f / (float)fbo_res[1]);
  GLint texel_size_loc = glGetUniformLocation(shader->gl_shader_program, "texel_size");
  glUniform2f(texel_size_loc, texel_size[0], texel_size[1]);

  RenderSurface::render();
}

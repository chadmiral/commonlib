#include "hdr_render_surface.h"

using namespace std;
using namespace Graphics;
using namespace Math;

HDRRenderSurface::HDRRenderSurface(const int w, const int h) : RenderSurface(w, h)
{
  exposure = 1.0f;
  bloom_threshold = 0.9f;

  shader2 = new Shader;
}

HDRRenderSurface::~HDRRenderSurface()
{
  deinit();
  delete shader2;
}

void HDRRenderSurface::init()
{
  //TODO: make these shader names safe
  std::string vs_name("../data/shaders/passthrough.vs");
  std::string fs_name("../data/shaders/hdr_tone_map.fs");
  std::string fs_name2("../data/shaders/hdr_tone_map_clamp.fs");
  set_shader_names(vs_name, fs_name);

  set_internal_format(GL_RGBA16F_ARB);
  set_filtering_mode(GL_LINEAR);

  shader2->set_shader_filenames(vs_name, fs_name2);
  shader2->load_link_and_compile();
  mat2.set_shader(shader2);
  mat2.add_texture(target_tex, "surface_tex");
  mat2.init();

  RenderSurface::init();
}

void HDRRenderSurface::deinit()
{
  RenderSurface::deinit();
}

void HDRRenderSurface::render()
{
  Shader *shader = mat.get_shader();
  mat.render(); //material needs to be bound for the uniforms to be set.

  //TODO: make these uniforms not super hacky
  GLint exposure_loc = glGetUniformLocation(shader->gl_shader_program, "exposure");
  glUniform1f(exposure_loc, exposure);
  GLint bloom_threshold_loc = glGetUniformLocation(shader->gl_shader_program, "bloom_threshold");
  glUniform1f(bloom_threshold_loc, bloom_threshold);

  RenderSurface::render();
}

void HDRRenderSurface::render_method_2()
{
  Shader *shader = mat2.get_shader();
  mat2.render(); //material needs to be bound for the uniforms to be set.

  GLint exposure_loc = glGetUniformLocation(shader->gl_shader_program, "exposure");
  glUniform1f(exposure_loc, exposure);
  GLint bloom_threshold_loc = glGetUniformLocation(shader->gl_shader_program, "bloom_threshold");
  glUniform1f(bloom_threshold_loc, bloom_threshold);

  //RenderSurface::render();

  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  //render the HDR render surface to a full-screen quad
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  mat2.render();

  for(unsigned int i = 0; i < uniforms.size(); i++)
  {
    Float2 *uval = uniforms[i].first;
    std::string uname = uniforms[i].second;
    GLint uloc = glGetUniformLocation(shader->gl_shader_program, uname.c_str());
    glUniform2f(uloc, (*uval)[0], (*uval)[1]);
  }

  /*for(int i = 0; i < tex_uniforms.size(); i++)
  {
    GLuint tex_id = tex_uniforms[i].first;
    std::string uname = tex_uniforms[i].second;

    GLint uloc = glGetUniformLocation(shader->gl_shader_program, uname.c_str());
    glUniform1i(uloc, i);

    glActiveTexture(GL_TEXTURE0 + i);
    glClientActiveTexture(GL_TEXTURE0 + i);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_id);
  }*/


  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(RenderSurfaceVert), (void *)0);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(RenderSurfaceVert), (void *)(sizeof(float) * 3));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);

  //reset shader
  glUseProgram(0);

  mat2.cleanup();

  /*for(int i = 0; i < tex_uniforms.size(); i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glClientActiveTexture(GL_TEXTURE0 + i);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
  }*/
}

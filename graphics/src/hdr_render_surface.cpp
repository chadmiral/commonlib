#include "hdr_render_surface.h"

using namespace std;
using namespace Graphics;
using namespace Math;

static char passthrough_vs_src[] =
"#version 120\n"
"\n"
"attribute vec3 in_xyz;\n"
"attribute vec2 in_uv0;\n"
"\n"
"varying vec2 uv0;\n"
"\n"
"void main(void)\n"
"{\n"
"  uv0 = in_uv0;\n"
"  gl_Position = gl_ModelViewProjectionMatrix * vec4(in_xyz, 1.0);\n"
"}\n";

static char hdr_tone_map_fs_src[] =
"#version 120\n"
"\n"
"uniform float exposure;\n"
"uniform float bloom_threshold;\n"
"\n"
"uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\n"
"\n"
"uniform sampler2D surface_tex;\n"
"uniform sampler3D lut_tex;\n"
"\n"
"varying vec2 uv0;\n"
"\n"
"void main()\n"
"{\n"
"  //const float exposure = 1.0;\n"
"  const float gamma = 1.0;//0.4545;//2.2;\n"
"  vec3 hdr_col = texture2D(surface_tex, uv0.st).rgb;\n"
"\n"
"  float luminance = 0.2126 * hdr_col.r + 0.7152 * hdr_col.g + 0.0722 * hdr_col.b;\n"
"\n"
"  //exposure tone mapping\n"
"  vec3 mapped = vec3(1.0) - exp(-hdr_col * exposure);\n"
"\n"
"  // Gamma correction\n"
"  mapped = pow(mapped, vec3(1.0 / gamma));\n"
"  vec3 color_graded = texture3D(lut_tex, mapped.rbg).rgb;\n"
"  gl_FragColor = vec4(color_graded, 1.0);\n"
"}\n";

static char hdr_tone_map_clamp_fs_src[] =
"#version 120\n"
"\n"
"uniform float exposure;\n"
"uniform float bloom_threshold;\n"
"\n"
"uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\n"
"\n"
"uniform sampler2D surface_tex;\n"
"uniform sampler3D lut_tex;\n"
"\n"
"void main()\n"
"{\n"
"  //const float exposure = 1.0;\n"
"  const float gamma = 1.0;//0.4545;//2.2;\n"
"  vec3 hdr_col = texture2D(surface_tex, gl_TexCoord[0].st).rgb;\n"
"\n"
"  float luminance = 0.2126 * hdr_col.r + 0.7152 * hdr_col.g + 0.0722 * hdr_col.b;\n"
"  if (luminance < bloom_threshold)\n"
"  {\n"
"    hdr_col = vec3(0.0, 0.0, 0.0);\n"
"  }\n"
"\n"
"  //exposure tone mapping\n"
"  vec3 mapped = vec3(1.0) - exp(-hdr_col * exposure);\n"
"\n"
"  // Gamma correction\n"
"  mapped = pow(mapped, vec3(1.0 / gamma));\n"
"  vec3 color_graded = texture3D(lut_tex, mapped.rbg).rgb;\n"
"  gl_FragColor = vec4(color_graded, 1.0);\n"
"}\n";

HDRRenderSurface::HDRRenderSurface(const int w, const int h) : RenderSurface(w, h)
{
  exposure = 1.0f;
  bloom_threshold = 0.9f;

  shader1 = new Shader;
  shader2 = new Shader;
}

HDRRenderSurface::~HDRRenderSurface()
{
  deinit();
  delete shader1;
  delete shader2;
}

void HDRRenderSurface::init()
{
  //TODO: make these shader names safe
  /*std::string vs_name("../data/shaders/passthrough.vs");
  std::string fs_name("../data/shaders/hdr_tone_map.fs");
  std::string fs_name2("../data/shaders/hdr_tone_map_clamp.fs");
  set_shader_names(vs_name, fs_name);
  */

  //TODO: make these shaders loaded through asset library
  shader1->compile_and_link_from_source(passthrough_vs_src, hdr_tone_map_fs_src);
  shader1->set_shader_filenames("internal_passthrough_hdr", "internal_tone_map_hdr");
  add_shader(shader1);

  //needs to happen after set_shader() call, but before we set up the second material
  RenderSurface::init();

  set_internal_format(GL_RGBA16F_ARB);
  set_filtering_mode(GL_LINEAR);

  //shader2->set_shader_filenames(vs_name, fs_name2);
  //shader2->load_link_and_compile();
  shader2->compile_and_link_from_source(passthrough_vs_src, hdr_tone_map_clamp_fs_src);
  shader2->set_shader_filenames("internal_passthrough_hdr", "internal_tone_map_clamp_hdr");
  
  mat2.set_shader(shader2);
  mat2.add_texture(target_tex, "surface_tex");
  mat2.init();
}

void HDRRenderSurface::deinit()
{
  RenderSurface::deinit();
}

void HDRRenderSurface::render()
{
  Shader *shader = _mat.get_shader();
  _mat.render(); //material needs to be bound for the uniforms to be set.

  //TODO: make these uniforms not super hacky
  GLint exposure_loc = glGetUniformLocation(shader->gl_shader_program, "exposure");
  glUniform1f(exposure_loc, exposure);
  GLint bloom_threshold_loc = glGetUniformLocation(shader->gl_shader_program, "bloom_threshold");
  glUniform1f(bloom_threshold_loc, bloom_threshold);

  //TODO: shader uniforms
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  RenderSurface::render();
}

void HDRRenderSurface::render_method_2()
{
  Shader *shader = mat2.get_shader();
  mat2.render(); //material needs to be bound for the uniforms to be set.

  //TODO: add all these uniforms via ShaderUniform system
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

  /*
  for(uint32_t i = 0; i < uniforms.size(); i++)
  {
    Float2 *uval = uniforms[i].first;
    std::string uname = uniforms[i].second;
    GLint uloc = glGetUniformLocation(shader->gl_shader_program, uname.c_str());
    glUniform2f(uloc, (*uval)[0], (*uval)[1]);
  }*/

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


  /*
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(RenderSurfaceVert), (void *)0);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(RenderSurfaceVert), (void *)(sizeof(float) * 3));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, (void *)0);
  */

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

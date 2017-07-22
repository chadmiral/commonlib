#include <assert.h>
#include <iostream>
#include <assert.h>
#include "gpu_voronoi.h"

using namespace Math;
using namespace std;
using namespace Graphics;

static char *passthrough_vs =
"#version 120\n"
"\n"
"attribute vec3 in_xyz;\n"
"attribute vec2 in_uv0;\n"
"\n"
"uniform mat4 proj_mat;\n"
"\n"
"varying vec2 uv0;\n"
"\n"
"void main(void)\n"
"{\n"
"  uv0 = in_uv0;\n"
"  gl_Position = proj_mat * gl_ModelViewMatrix * vec4(in_xyz, 1.0);\n"
"}\n"
"\n";

static char *passthrough_fs =
"#version 120\n"
"uniform sampler2D surface_tex;\n"
"\n"
"varying vec2 uv0;\n"
"\n"
"void main()\n"
"{\n"
"  gl_FragColor = texture2D(surface_tex, uv0.st);\n"
"}\n"
"\n";

GPUVoronoi2D::GPUVoronoi2D(const GLuint num_seg, const GLuint max_num_sites)
{
  _num_cone_segments = num_seg;
  _num_cone_verts = 2 + _num_cone_segments;

  _cone_vbo = 0;
  _cone_ibo = 0;

  _fbo_res[0] = 512;
  _fbo_res[1] = 512;

  _tex_format = GL_BGRA;

  _render_surface.set_fbo_res(_fbo_res[0], _fbo_res[1]);
  _render_surface.set_internal_format(GL_RGBA);
  _render_surface.set_format(_tex_format);
  _render_surface.set_filtering_mode(GL_NEAREST);
  _render_surface.use_depth(true);

  _max_num_sites = max_num_sites;
  _cpu_tex_data = NULL;
}

GPUVoronoi2D::~GPUVoronoi2D()
{
  deinit();
}

void GPUVoronoi2D::init()
{
  float r = 1.0f;

  ConeVert cv;
  cv.x = 0.0f; cv.y = 0.0f; cv.z = 1.0f;
  _cone_vertex_data.push_back(cv);
  _cone_index_data.push_back(0);
  for(uint32_t i = 0; i < _num_cone_verts; i++)
  {
    float theta = 2.0f * (float)M_PI * (float)i / (float)_num_cone_segments;
    cv.x = r * cos(theta);
    cv.y = r * sin(theta);
    cv.z = 0.0f;
    _cone_vertex_data.push_back(cv);
    _cone_index_data.push_back(i + 1);
  }

  glGenBuffers(1, &_cone_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _cone_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ConeVert) * _num_cone_verts, _cone_vertex_data.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &_cone_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cone_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * _num_cone_verts, _cone_index_data.data(), GL_STATIC_DRAW);

  _render_surface_shader = new Shader;
  _render_surface_shader->compile_and_link_from_source(passthrough_vs, passthrough_fs);

  _render_surface.add_shader(_render_surface_shader);
  _render_surface.init();

  assert(!_cpu_tex_data);
  _cpu_tex_data = new GLubyte[_fbo_res[0] * _fbo_res[1] * 4];
}

void GPUVoronoi2D::deinit()
{
  glDeleteBuffers(1, &_cone_vbo);
  glDeleteBuffers(1, &_cone_ibo);

  _render_surface.deinit();

  if (_cpu_tex_data) { delete _cpu_tex_data; }
}

void GPUVoronoi2D::reset()
{
  _sites.clear();
}

void GPUVoronoi2D::add_site(Float2 pt)
{
  _sites.push_back(pt);
}

void GPUVoronoi2D::build_voronoi_diagram()
{
  _render_surface.capture();

  glUseProgram(0);

  glEnable(GL_DEPTH_TEST);
  glDepthRange(0.0f, 1.0f);
  glDepthMask(GL_TRUE);

  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);

  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_2D);

  //set the render target to the voronoi diagram texture / fbo
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //want the depth range as tight as possible to minimize error
  glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);
  glEnableClientState(GL_VERTEX_ARRAY);

  //render a cone for each site
  //TODO: instanced rendering
  for(uint32_t i = 0; i < _sites.size(); i++)
  {
    glLoadIdentity();
    glTranslatef(_sites[i][0], _sites[i][1], -1.0f);

    //color stores site index, split into a byte for each color channel
    GLubyte b = i / 65536;
    GLubyte g = (i - b) / 256;
    GLubyte r = (i - b) - g;

    //TODO: proper material / shader for this
    glColor3ub(r, g, b);

    glBindBuffer(GL_ARRAY_BUFFER, _cone_vbo);
    glVertexPointer(3, GL_FLOAT, sizeof(ConeVert), (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cone_ibo);
    glDrawElements(GL_TRIANGLE_FAN, _num_cone_verts, GL_UNSIGNED_INT, (void *)0);
  }

  glDisableClientState(GL_VERTEX_ARRAY);

  //set the FBO back to default
  _render_surface.release();

  //now, copy the texture data from GPU -> CPU for fast queries
  //TODO: optimize by using a stack of PBOs
  memset(_cpu_tex_data, 0, _fbo_res[0] * _fbo_res[1] * 4);
  glBindTexture(GL_TEXTURE_2D, _render_surface.get_tex()->get_tex_id());
  glGetTexImage(GL_TEXTURE_2D, 0, _tex_format, GL_UNSIGNED_BYTE, _cpu_tex_data);
}

uint32_t GPUVoronoi2D::query_nearest_site(const Float2 p)
{
  assert(p[0] >= 0.0f && p[0] <= 1.0f);
  assert(p[1] >= 0.0f && p[1] <= 1.0f);

  uint32_t x = (int)(p[0] * (float)_fbo_res[0]);
  uint32_t y = (int)(p[1] * (float)_fbo_res[1]);

  if(x >= _fbo_res[0]) { x = _fbo_res[0] - 1; }
  if(y >= _fbo_res[1]) { y = _fbo_res[1] - 1; }

  GLubyte *pix = &(_cpu_tex_data[4 * (x + _fbo_res[0] * y)]);
  //return pix[0] + pix[1] * 256 + pix[2] * (256 * 256); //RGB
  return pix[2] + pix[1] * 256 + pix[0] * (256 * 256);  //BGR
}

void GPUVoronoi2D::render_voronoi_texture()
{
  _render_surface.render(0);
}


void GPUVoronoi2D::render_fullscreen_quad()
{
  glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
  glEnd();
}

void GPUVoronoi2D::setup_textured_quad_state()
{
  glUseProgram(0);
  glDisable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glActiveTexture(GL_TEXTURE0);
  //glClientActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

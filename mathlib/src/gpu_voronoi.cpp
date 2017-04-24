#include <assert.h>
#include <iostream>
#include <assert.h>
#include "gpu_voronoi.h"

using namespace Math;
using namespace std;

GPUVoronoi2D::GPUVoronoi2D(const GLuint num_seg, const GLuint _max_num_sites, const GLuint flags)
{
  behavior_flags = flags;
  num_cone_segments = num_seg;
  num_cone_verts = 2 + num_cone_segments;

  cone_vbo = 0;
  cone_ibo = 0;

  fbo_res[0] = 512;
  fbo_res[1] = 512;

  depth_fbo = 0;
  tex_format = GL_BGRA;
  voronoi_diagram_fbo = 0;
  voronoi_diagram_tex = 0;

  max_num_sites = _max_num_sites;

  cone_vertex_data = NULL;
  cpu_tex_data = NULL;
}

GPUVoronoi2D::~GPUVoronoi2D()
{
  if(cpu_tex_data) { delete cpu_tex_data; }
}

void GPUVoronoi2D::init()
{
  glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &max_draw_indices);
  glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_draw_verts);

  //allocate vertex data for the cones (GL_TRIANGLE_FAN)
  cone_vertex_data = new ConeVert[num_cone_verts];
  cone_index_data = new unsigned int[num_cone_verts];

  float r = 1.0f;

  cone_vertex_data[0].x = 0.0f;
  cone_vertex_data[0].y = 0.0f;
  cone_vertex_data[0].z = 1.0f;
  cone_index_data[0] = 0;
  for(unsigned int i = 0; i < num_cone_verts; i++)
  {
    float theta = 2.0f * (float)M_PI * (float)i / (float)num_cone_segments;
    cone_vertex_data[i + 1].x = r * cos(theta);
    cone_vertex_data[i + 1].y = r * sin(theta);
    cone_vertex_data[i + 1].z = 0.0f;
    cone_index_data[i + 1] = i + 1;
  }

  glGenBuffers(1, &cone_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, cone_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ConeVert) * num_cone_verts, cone_vertex_data, GL_STATIC_DRAW);

  glGenBuffers(1, &cone_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * num_cone_verts, cone_index_data, GL_STATIC_DRAW);

  //delete cone_vertex_data;
  //delete cone_index_data;

  //allocate the voronoi diagram texture and frame buffer object
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &voronoi_diagram_tex);
  glBindTexture(GL_TEXTURE_2D, voronoi_diagram_tex);
  assert(glIsTexture(voronoi_diagram_tex) == GL_TRUE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,       //internal format
               fbo_res[0],
               fbo_res[1],
               0,
               tex_format,
               GL_UNSIGNED_BYTE,
               NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);

  assert(glIsTexture(voronoi_diagram_tex) == GL_TRUE);

  glGenRenderbuffers(1, &depth_fbo);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_fbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo_res[0], fbo_res[1]);

  glGenFramebuffers(1, &voronoi_diagram_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, voronoi_diagram_fbo);

  assert(glIsFramebuffer(voronoi_diagram_fbo) == GL_TRUE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, voronoi_diagram_tex, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  cpu_tex_data = new GLubyte[fbo_res[0] * fbo_res[1] * 4];
}

void GPUVoronoi2D::deinit()
{
  delete cone_vertex_data;
  delete cone_index_data;
}

void GPUVoronoi2D::reset()
{
  sites.clear();
}

void GPUVoronoi2D::add_site(Float2 pt)
{
  sites.push_back(pt);
}

void GPUVoronoi2D::build_voronoi_diagram()
{
  //set up the render target
  GLint win_viewport[4];
  glBindFramebuffer(GL_FRAMEBUFFER, voronoi_diagram_fbo);
  glGetIntegerv(GL_VIEWPORT, win_viewport);
  glViewport(0, 0, fbo_res[0], fbo_res[1]);

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

  //render a cone for each site
  //TODO: instanced rendering
  for(unsigned int i = 0; i < sites.size(); i++)
  {
    glLoadIdentity();
    glTranslatef(sites[i][0], sites[i][1], -1.0f);

    //color stores site index, split into a byte for each color channel
    GLubyte b = i / 65536;
    GLubyte g = (i - b) / 256;
    GLubyte r = (i - b) - g;

    glColor3ub(r, g, b);

    glBindBuffer(GL_ARRAY_BUFFER, cone_vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(ConeVert), (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_ibo);
    glDrawElements(GL_TRIANGLE_FAN, num_cone_verts, GL_UNSIGNED_INT, (void *)0);
  }

  // set the FBO back to default
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glViewport(win_viewport[0], win_viewport[1], win_viewport[2], win_viewport[3]);

  //now, copy the texture data from GPU -> CPU for fast queries
  //TODO: optimize by using a stack of PBOs
  memset(cpu_tex_data, 0, fbo_res[0] * fbo_res[1] * 4);
  glBindTexture(GL_TEXTURE_2D, voronoi_diagram_tex);
  glGetTexImage(GL_TEXTURE_2D,
                0,
                tex_format,
                GL_UNSIGNED_BYTE,
                cpu_tex_data);
}

unsigned int GPUVoronoi2D::query_nearest_site(const Float2 p)
{
  assert(p[0] >= 0.0f && p[0] <= 1.0f);
  assert(p[1] >= 0.0f && p[1] <= 1.0f);

  unsigned int x = (int)(p[0] * (float)fbo_res[0]);
  unsigned int y = (int)(p[1] * (float)fbo_res[1]);

  if(x >= fbo_res[0]) { x = fbo_res[0] - 1; }
  if(y >= fbo_res[1]) { y = fbo_res[1] - 1; }

  GLubyte *pix = &(cpu_tex_data[4 * (x + fbo_res[0] * y)]);
  //return pix[0] + pix[1] * 256 + pix[2] * (256 * 256); //RGB
  return pix[2] + pix[1] * 256 + pix[0] * (256 * 256);  //BGR
}

void GPUVoronoi2D::render_voronoi_texture()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  setup_textured_quad_state();
  glBindTexture(GL_TEXTURE_2D, voronoi_diagram_tex);
  render_fullscreen_quad();

  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
}


void GPUVoronoi2D::render_fullscreen_quad()
{
  //TODO: VBO
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

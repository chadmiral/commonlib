#include <assert.h>
#include <iostream>
#include "gpu_voronoi.h"

using namespace Math;
using namespace std;

GPUVoronoi2D::GPUVoronoi2D(const GLuint num_seg, const GLuint flags)
{
  behavior_flags = flags;
  num_cone_segments = num_seg;
  num_cone_verts = 2 + num_cone_segments;

  cone_vbo = 0;
  cone_ibo = 0;

  fbo_res[0] = 512;
  fbo_res[1] = 512;

  depth_fbo = 0;
  voronoi_diagram_fbo = 0;
  voronoi_diagram_tex = 0;

  cone_vertex_data = NULL;
}

GPUVoronoi2D::~GPUVoronoi2D() {}

void GPUVoronoi2D::init()
{
  //allocate vertex data for the cones (GL_TRIANGLE_FAN)
  cone_vertex_data = new ConeVert[num_cone_verts];
  cone_index_data = new unsigned int[num_cone_verts];

  float r = 1.0f;

  cone_vertex_data[0].x = 0.0f;
  cone_vertex_data[0].y = 0.0f;
  cone_vertex_data[0].z = 1.0f;
  cone_index_data[0] = 0;
  for(int i = 0; i < num_cone_verts; i++)
  {
    float theta = 2.0f * M_PI * (float)i / (float)num_cone_segments;
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
  GLuint internal_format = GL_RGB;//GL_RGBA_FLOAT32_ATI;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &voronoi_diagram_tex);
  glBindTexture(GL_TEXTURE_2D, voronoi_diagram_tex);
  assert(glIsTexture(voronoi_diagram_tex) == GL_TRUE);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,
               fbo_res[0],
               fbo_res[1],
               0,
               GL_RGB, //GL_RGBA16F_ARB,
               GL_UNSIGNED_BYTE,
               NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR);

  assert(glIsTexture(voronoi_diagram_tex) == GL_TRUE);

  glGenRenderbuffersEXT(1, &depth_fbo);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_fbo);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, fbo_res[0], fbo_res[1]);

  glGenFramebuffersEXT(1, &voronoi_diagram_fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, voronoi_diagram_fbo);

  assert(glIsFramebuffer(voronoi_diagram_fbo) == GL_TRUE);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, voronoi_diagram_tex, 0);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void GPUVoronoi2D::deinit()
{

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
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, voronoi_diagram_fbo);
  glGetIntegerv(GL_VIEWPORT, win_viewport);
  glViewport(0, 0, fbo_res[0], fbo_res[1]);

  glUseProgramObjectARB(0);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  //set the render target to the voronoi diagram texture / fbo
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //want the depth range as tight as possible to minimize error
  glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);

  //render a cone for each site
  //TODO: optimize by reducing draw calls (1 drawcall for all cones)
  for(int i = 0; i < sites.size(); i++)
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
}

unsigned int GPUVoronoi2D::query_nearest_site(const Float2 p)
{
  //TODO: use PBOs for optimization
  glBindTexture(GL_TEXTURE_2D, voronoi_diagram_tex);

  //int buffer_size = fbo_res[0] * fbo_res[1] * sizeof(GLubyte) * 4;

  GLubyte *pixels = new GLubyte[fbo_res[0] * fbo_res[1] * 4];
  glGetTexImage(GL_TEXTURE_2D,
                0,
                GL_RGB, //GL_BGRA
                GL_UNSIGNED_BYTE,
                pixels);

  int x = p[0] * (float)fbo_res[0];
  int y = p[1] * (float)fbo_res[1];

  GLubyte *pix = &(pixels[3 * (x + fbo_res[0] * y)]);
  return pix[0] + pix[1] * 256 + pix[2] * (256 * 256);
}

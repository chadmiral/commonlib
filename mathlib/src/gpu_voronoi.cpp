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

  fbo_res[0] = 2048;
  fbo_res[1] = 1024;

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
  GLuint internal_format = GL_RGBA_FLOAT32_ATI;
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
               GL_RGBA, //GL_RGBA16F_ARB,
               GL_UNSIGNED_BYTE,
               NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  assert(glIsTexture(voronoi_diagram_tex) == GL_TRUE);
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
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  //set the render target to the voronoi diagram texture / fbo
  glClearColor(0.3f, 0.2f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //want the depth range as tight as possible to minimize error
  glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);

  //render a cone for each site
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
}

#include <iostream>

#include <assert.h>
#include <string.h>

//#include <GL/glew.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include "static_mesh.h"
#include "gl_error.h"

#define RENDER_METHOD_VBO   true

using namespace std;
using namespace Graphics;

StaticMesh::StaticMesh()
{
  vbo = ibo = 0;

  num_vertices = 0;
  vertices = NULL;

  num_indices = 0;
  indices = NULL;
}

StaticMesh::~StaticMesh()
{
  if(vertices)
  {
    delete vertices;
    glDeleteBuffersARB(1, &vbo);
  }

  if(indices)
  {
    glDeleteBuffersARB(1, &ibo);
    delete indices;
  }
}

void StaticMesh::read_from_file(FILE *f, bool verbose)
{
  int version;
  fread(&version, sizeof(int), 1, f);

  //int num_verts, num_indices;
  fread(&num_vertices, sizeof(int), 1, f);
  vertices = new StaticMeshVertex[num_vertices];
  fread(vertices, sizeof(StaticMeshVertex), num_vertices, f);

  fread(&num_indices, sizeof(int), 1, f);
  indices = new unsigned int[num_indices];
  fread(indices, sizeof(unsigned int), num_indices, f);

  if(verbose)
  {
    cout<<"StaticMesh::read_from_file(): "<<endl;
    cout<<"\t"<<num_vertices<<" vertices..."<<endl;
    /*(for(int i = 0; i < num_vertices; i++)
    {
      cout<<"\t\tp:  "<<vertices[i].x<<", "<<vertices[i].y<<", "<<vertices[i].z<<endl;
      cout<<"\t\tn:  "<<vertices[i].nx<<", "<<vertices[i].ny<<", "<<vertices[i].nz<<endl;
      cout<<"\t\tuv: "<<vertices[i].u0<<", "<<vertices[i].v0<<endl;
    }*/
    cout<<"\t"<<num_indices<<" indices..."<<endl;
  }
}

void StaticMesh::init()
{
  //create openGL buffer objects
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(StaticMeshVertex) * num_vertices, vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * num_indices, indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void StaticMesh::render(Material *m, GLenum primitive_type)
{
  //TODO: use DrawCall objects
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  if (m)
  {
    m->render();
  }
  else
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(StaticMeshVertex), (void *)0);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, sizeof(StaticMeshVertex), (void *)(sizeof(float) * 3));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(StaticMeshVertex), (void *)(sizeof(float) * 6));

    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(StaticMeshVertex), (void *)(sizeof(float) * 9));
  }
  
  glDrawElements(primitive_type, num_indices, GL_UNSIGNED_INT, (void *)0);

  if (m)
  {
    m->cleanup();
  }
  else
  {
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

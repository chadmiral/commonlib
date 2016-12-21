#include <iostream>

#include <assert.h>
#include <string.h>

#include "static_mesh.h"
#include "gl_error.h"

#define RENDER_METHOD_VBO   true

using namespace std;
using namespace Graphics;

StaticMesh::StaticMesh()
{
  _vbo = _ibo = 0;

  _num_vertices = 0;
  _vertices = NULL;

  _num_indices = 0;
  _indices = NULL;
}

StaticMesh::~StaticMesh()
{
  if(_vertices)
  {
    delete _vertices;
    glDeleteBuffers(1, &_vbo);
  }

  if(_indices)
  {
    glDeleteBuffers(1, &_ibo);
    delete _indices;
  }
}

void StaticMesh::read_from_file(FILE *f, bool verbose)
{
  int version;
  fread(&version, sizeof(int), 1, f);

  //int num_verts, num_indices;
  fread(&_num_vertices, sizeof(int), 1, f);
  _vertices = new StaticMeshVertex[_num_vertices];
  fread(_vertices, sizeof(StaticMeshVertex), _num_vertices, f);

  fread(&_num_indices, sizeof(int), 1, f);
  _indices = new unsigned int[_num_indices];
  fread(_indices, sizeof(unsigned int), _num_indices, f);

  if(verbose)
  {
    cout<<"StaticMesh::read_from_file(): "<<endl;
    cout<<"\t"<<_num_vertices<<" vertices..."<<endl;
    /*(for(int i = 0; i < num_vertices; i++)
    {
      cout<<"\t\tp:  "<<vertices[i].x<<", "<<vertices[i].y<<", "<<vertices[i].z<<endl;
      cout<<"\t\tn:  "<<vertices[i].nx<<", "<<vertices[i].ny<<", "<<vertices[i].nz<<endl;
      cout<<"\t\tuv: "<<vertices[i].u0<<", "<<vertices[i].v0<<endl;
    }*/
    cout<<"\t"<<_num_indices<<" indices..."<<endl;
  }
}

void StaticMesh::set_data(uint32_t num_verts, StaticMeshVertex *verts, uint32_t num_indices, uint32_t *indices)
{
  _num_vertices = num_verts;
  _vertices = verts;
  _num_indices = num_indices;
  _indices = indices;
}

void StaticMesh::init()
{
  //create openGL buffer objects
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(StaticMeshVertex) * _num_vertices, _vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * _num_indices, _indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void StaticMesh::render(Material *m, GLenum primitive_type)
{
  //TODO: use DrawCall objects
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

  if (m)
  {
    m->render();
  }

#ifndef __LOKI__
  else
  {
    //TODO: vertex attribs
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
#endif //__LOKI__
  
  glDrawElements(primitive_type, _num_indices, GL_UNSIGNED_INT, (void *)0);

  if (m)
  {
    m->cleanup();
  }

#ifndef __LOKI__
  else
  {
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
#endif //__LOKI__

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

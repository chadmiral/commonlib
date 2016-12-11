#include <iostream>
#include "skeleton.h"

#if defined(_WIN32)
#include <Windows.h>
#include <GL/glew.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

using namespace Animation;
using namespace std;
using namespace Math;

void Skeleton::transform(const float t, AnimAction *a)
{
  cout << "Skeleton::transform(): " << _name.c_str() << endl;
}

void Skeleton::render_debug()
{
  glDisable(GL_DEPTH_TEST);
  glBegin(GL_LINES);
  //glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
  for (uint32_t i = 0; i < _bones.size(); i++)
  {
    if (_bones[i].second._parent_idx == -1)
    {
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
      glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    }
    Float3 head = _bones[i].second._head_pos;
    Float3 tail = _bones[i].second._tail_pos;
    glVertex3f(head[0], head[2], head[1]);
    glVertex3f(tail[0], tail[2], tail[1]);
  }
  glEnd();
}
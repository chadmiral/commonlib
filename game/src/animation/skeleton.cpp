#include <iostream>
#include "skeleton.h"
#include "platform_gl.h"

using namespace Animation;
using namespace std;
using namespace Math;

void Skeleton::transform(const float t, BoneAnim *a)
{
  cout << "Skeleton::transform(): " << _name.c_str() << endl;
}

void Skeleton::render_debug(BoneAnim *animation, float anim_pct)
{
  glDisable(GL_DEPTH_TEST);
  glBegin(GL_LINES);
  //glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
  for (uint32_t i = 0; i < _bones.size(); i++)
  {
    if (_bones[i]._parent_idx == -1)
    {
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
      glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
    }

    if (animation)
    {
      //find the track that matches this bone
      //animation->get_track(_bones[i].first);
    }

    Float3 head = _bones[i]._head_pos;
    Float3 tail = _bones[i]._tail_pos;
    glVertex3f(head[0], head[2], head[1]);
    glVertex3f(tail[0], tail[2], tail[1]);
  }
  glEnd();
}
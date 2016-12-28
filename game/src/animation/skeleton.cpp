#include <iostream>
#include "skeleton.h"
#include "platform_gl.h"
#include "matrix.h"

using namespace Animation;
using namespace std;
using namespace Math;

void Skeleton::transform(const float t, SkeletonAnimation *a)
{
  cout << "Skeleton::transform(): " << _name.c_str() << endl;
}

void Skeleton::render_debug(SkeletonAnimation *animation, float anim_pct)
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

    BoneTransform bt;

    if (animation)
    {
      //find the track that matches this bone
      for (uint32_t j = 0; j < animation->_tracks.size(); j++)
      {
        if (animation->_tracks[j]._bone == &_bones[i])
        {
          animation->_tracks[j].evaluate(anim_pct, &bt);
        }
      }
    }

    Float3 head = _bones[i]._head_pos + bt._pos._pos;
    Float3 tail = _bones[i]._tail_pos + bt._pos._pos;
    glVertex3f(head[0], head[2], head[1]);
    glVertex3f(tail[0], tail[2], tail[1]);
  }
  glEnd();
}
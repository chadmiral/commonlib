

#include <iostream>

#include "object_3d.h"

using namespace std;
using namespace Game;
using namespace Math;

Object3D::Object3D()
{
  scale = Float3(1.0f, 1.0f, 1.0f);
  birth_time = -1;

  object_flags = 0;
  type = 0;
  data = NULL;
}

void Object3D::init(const double game_time)
{
  birth_time = game_time;
}

void Object3D::simulate(const double gt, const double dt)
{
  if (object_flags & OBJECT_FLAG_PHYSICS_XYZ)
  {
    pos = pos + (vel * (float)dt);
  }

  //TODO: angular velocity
}

void Object3D::render(const double game_time)
{
  GLdouble mat[16] =
  {
    scale[0] * rot(0, 0), rot(1, 0), rot(2, 0),    0.0,
    rot(0, 1), scale[1] * rot(1, 1), rot(2, 1),    0.0,
    rot(0, 2), rot(1, 2), scale[2] * rot(2, 2),    0.0,
    pos[0],    pos[1],    pos[2],       1.0
  };

  //glLoadMatrixd(mat);
  glMatrixMode(GL_MODELVIEW);
  glMultMatrixd(mat);
}

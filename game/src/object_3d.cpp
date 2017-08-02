

#include <iostream>

#include "object_3d.h"

using namespace std;
using namespace Game;
using namespace Math;

Object3D::Object3D()
{
  _scale = Float3(1.0f, 1.0f, 1.0f);
  _birth_time = -1;

  _object_flags = 0;
  _type = 0;
  _data = NULL;
}

void Object3D::init(const double game_time)
{
  _birth_time = game_time;
}

void Object3D::simulate(const double gt, const double dt)
{
  if (_object_flags & OBJECT_FLAG_PHYSICS_XYZ)
  {
    _pos = _pos + (_vel * (float)dt);
  }

  //TODO: angular velocity
  if (_object_flags & OBJECT_FLAG_PHYSICS_ROT)
  {
    _rot_quat = _rot_quat + (_ang_vel_quat * (float)dt);
  }
}

void Object3D::render(const double game_time)
{
  Matrix3x3 rot;
  //rot.rotation_from_quaternion(_rot_quat);
  rot.identity();
  
  Matrix3x3 scale;
  scale(0, 0) = _scale[0];
  scale(1, 1) = _scale[1];
  scale(2, 2) = _scale[2];

  Matrix3x3 rot_scale = rot * scale;

  //TODO: don't do all this matrix math every frame? (OpenCL)
  GLdouble mat[16] = 
  {
    rot_scale(0, 0),   rot_scale(1, 0),   rot_scale(2, 0),  0.0,
    rot_scale(0, 1),   rot_scale(1, 1),   rot_scale(2, 1),  0.0,
    rot_scale(0, 2),   rot_scale(1, 2),   rot_scale(2, 2),  0.0,
    _pos[0],     _pos[1],     _pos[2],    1.0
  };

  //glLoadMatrixd(mat);
  glMatrixMode(GL_MODELVIEW);
  glMultMatrixd(mat);
}

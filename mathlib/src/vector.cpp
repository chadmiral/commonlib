#include "vector.h"

using namespace Math;

Float2::Float2()
{
  _val[0] = _val[1] = 0.0f;
}

Float2::Float2(const float _x, const float _y)
{
  _val[0] = _x;
  _val[1] = _y;
}

Float2::Float2(const Float2 &b)
{
  _val[0] = b._val[0];
  _val[1] = b._val[1];
}

Float3::Float3()
{
  _val[0] = _val[1] = _val[2];
}

Float3::Float3(const float _x, const float _y, const float _z)
{
  _val[0] = _x;
  _val[1] = _y;
  _val[2] = _z;
}

Float3::Float3(const Float2 &b, const float _z = 0.0f)
{
  _val[0] = b._val[0];
  _val[1] = b._val[1];
  _val[2] = _z;
}

Float3::Float3(const Float3 &b)
{
  _val[0] = b._val[0];
  _val[1] = b._val[1];
  _val[2] = b._val[2];
}

Float4::Float4()
{
  _val[0] = _val[1] = _val[2] = _val[3] = 0.0f;
}

Float4::Float4(const Float4 &v)
{
  _val[0] = v._val[0];
  _val[1] = v._val[1];
  _val[2] = v._val[2];
  _val[3] = v._val[3];
}

Float4::Float4(const Float3 &v, const float w)
{
  _val[0] = v._val[0];
  _val[1] = v._val[1];
  _val[2] = v._val[2];
  _val[3] = w;
}

Float4::Float4(const float x, const float y, const float z, const float w)
{
  _val[0] = x;
  _val[1] = y;
  _val[2] = z;
  _val[2] = w;
}


#include <math.h>
#include "matrix.h"

// look into using an optimized linear algebra library (usually waaaaay faster)
// http://arma.sourceforge.net/download.html
// http://www.openblas.net/

using namespace Math;
using namespace std;

Matrix2x2::Matrix2x2()
{
  m[0][0] = 1.0f;   m[0][1] = 0.0f;
  m[1][0] = 0.0f;   m[1][1] = 1.0f;
}

Matrix2x2::Matrix2x2(const float _00, const float _01, const float _10, const float _11)
{
  m[0][0] = _00;    m[0][1] = _01;
  m[1][0] = _10;    m[1][1] = _11;
}

Matrix2x2 Matrix2x2::operator*(const Matrix2x2 &r)
{
  return Matrix2x2(m[0][0] * r.m[0][0] + m[0][1] * r.m[1][0],
                   m[0][0] * r.m[0][1] + m[0][1] * r.m[1][1],
                   m[1][0] * r.m[0][0] + m[1][1] * r.m[1][0],
                   m[1][0] * r.m[0][1] + m[1][1] * r.m[1][1]);
}

Float2 Matrix2x2::operator*(const Float2 &r)
{
  return Float2(m[0][0] * r[0] + m[0][1] * r[1], m[1][0] * r[0] + m[1][1] * r[1]);
}

void Matrix2x2::identity()
{
  m[0][0] = 1.0f;   m[0][1] = 0.0f;
  m[1][0] = 0.0f;   m[1][1] = 1.0f;
}

void Matrix2x2::rotation(const float rad)
{
  float cos_theta = cos(rad);
  float sin_theta = sin(rad);
  m[0][0] = cos_theta;   m[0][1] = -sin_theta;
  m[1][0] = sin_theta;   m[1][1] = cos_theta;
}


//3x3 matrix
Matrix3x3::Matrix3x3()
{
  identity();
}

Matrix3x3::Matrix3x3(const float _00, const float _01, const float _02,
                     const float _10, const float _11, const float _12,
                     const float _20, const float _21, const float _22)
{
    m[0][0] = _00;  m[0][1] = _01;  m[0][2] = _02;
    m[1][0] = _10;  m[1][1] = _11;  m[1][2] = _12;
    m[2][0] = _20;  m[2][1] = _21;  m[2][2] = _22;
}

Matrix3x3::Matrix3x3(const Float3 &a, const Float3 &b, const Float3 &c)
{
  for(int i = 0; i < 3; i++)
  {
    m[0][i] = a[i];
    m[1][i] = b[i];
    m[2][i] = c[i];
  }
}

Matrix3x3::Matrix3x3(const Matrix4x4 &in) : Matrix3x3()
{
  for (uint32_t i = 0; i < 3; i++)
  {
    for (uint32_t j = 0; j < 3; j++)
    {
      m[i][j] = in(i, j);
    }
  }
}

float Matrix3x3::determinant() const
{
  return m[0][0] * m[1][1] * m[2][2] - m[0][0] * m[1][2] * m[2][1] + // i row
         m[0][1] * m[1][2] * m[2][0] - m[0][1] * m[1][0] * m[2][2] + // j row
         m[0][2] * m[1][0] * m[2][1] - m[0][2] * m[1][1] * m[2][0];  // k row
}

Matrix3x3 &Matrix3x3::operator=(const Matrix3x3 &r)
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      m[i][j] = r.m[i][j];
    }
  }

  return *this;
}

Float3 Matrix3x3::operator*(const Float3 &r) const
{
  Float3 ret;
  ret[0] = m[0][0] * r[0] + m[0][1] * r[1] + m[0][2] * r[2];
  ret[1] = m[1][0] * r[0] + m[1][1] * r[1] + m[1][2] * r[2];
  ret[2] = m[2][0] * r[0] + m[2][1] * r[1] + m[2][2] * r[2];
  return ret;
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3 &a) const
{
  //order of operations: a * this
  //c_(np)	=	a_(n1)b_(1p)+a_(n2)b_(2p)+...+a_(nm)b_(mp).
  Matrix3x3 ret;
  for (uint32_t i = 0; i < 3; i++)
  {
    for (uint32_t j = 0; j < 3; j++)
    {
      ret.m[j][i] = a.m[j][0] * m[0][i] + a.m[j][1] * m[1][i] + a.m[j][2] * m[2][i];
    }
  }

  return ret;
}

void Matrix3x3::identity()
{
  m[0][0] = 1.0f;   m[0][1] = 0.0f;   m[0][2] = 0.0f;
  m[1][0] = 0.0f;   m[1][1] = 1.0f;   m[1][2] = 0.0f;
  m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = 1.0f;
}

void Matrix3x3::invert()
{
  //ref: http://mathworld.wolfram.com/MatrixInverse.html
  Matrix3x3 ret;

  //row 1
  ret.m[0][0] = m[1][1] * m[2][2] - m[2][1] * m[1][2];
  ret.m[0][1] = m[2][0] * m[2][1] - m[0][1] * m[2][2];
  ret.m[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];

  //row 2
  ret.m[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
  ret.m[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
  ret.m[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];

  //row 3
  ret.m[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
  ret.m[2][1] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
  ret.m[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];

  *this = ret;
}

void Matrix3x3::transpose()
{
  Matrix3x3 temp(m[0][0], m[0][1], m[0][2],
                 m[1][0], m[1][1], m[1][2],
                 m[2][0], m[2][1], m[2][2]);

  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      m[i][j] = temp.m[j][i];
    }
  }
}

void Matrix3x3::rotation_from_quaternion(const Quaternion &q)
{
  float x2 = q.q[1] * q.q[1];
  float y2 = q.q[2] * q.q[2];
  float z2 = q.q[3] * q.q[3];
  float w2 = q.q[0] * q.q[0];

  float two_x = 2.0f * q.q[1];
  float two_y = 2.0f * q.q[2];
  float two_w = 2.0f * q.q[0];

  float xy = two_x * q.q[2];
  float xz = two_x * q.q[3];
  float yz = two_y * q.q[3];
  float wx = two_w * q.q[1];
  float wy = two_w * q.q[2];
  float wz = two_w * q.q[3];

  m[0][0] = w2 + x2 - y2 - z2;
  m[0][1] = xy - wz;
  m[0][2] = xz + wy;

  m[1][0] = xy + wz;
  m[1][1] = w2 - x2 + y2 - z2;
  m[1][2] = yz - wx;

  //fill in the third row
  m[2][0] = xz - wy;
  m[2][1] = yz + wx;
  m[2][2] = w2 - x2 - y2 + z2;
}







//4x4 matrix
Matrix4x4::Matrix4x4()
{
  identity();
}

Matrix4x4::Matrix4x4(const float _00, const float _01, const float _02, const float _03,
                     const float _10, const float _11, const float _12, const float _13,
                     const float _20, const float _21, const float _22, const float _23,
                     const float _30, const float _31, const float _32, const float _33)
{
  m[0][0] = _00;  m[0][1] = _01;  m[0][2] = _02;  m[0][3] = _03;
  m[1][0] = _10;  m[1][1] = _11;  m[1][2] = _12;  m[1][3] = _13;
  m[2][0] = _20;  m[2][1] = _21;  m[2][2] = _22;  m[2][3] = _23;
  m[3][0] = _30;  m[3][1] = _31;  m[3][2] = _32;  m[3][3] = _33;
}

Matrix4x4::Matrix4x4(const Matrix3x3 &mat, const Float3 &pos)
{
  //identity();

  for (uint32_t i = 0; i < 3; i++)
  {
    for (uint32_t j = 0; j < 3; j++)
    {
      m[i][j] = mat(i, j);
    }
    m[i][3] = pos[i];
  }
  m[3][0] = m[3][1] = m[3][2] = 0.0f;
  //m[0][3] = m[1][3] = m[2][3] = 0.0f;
  m[3][3] = 1.0f;

  //transpose();
}

Matrix4x4::Matrix4x4(const double *mat)
{
  int k = 0;
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      m[i][j] = (float)mat[k++];
    }
  }
}

/*
Matrix4x4::Matrix4x4(const Float3 &a, const Float3 &b, const Float3 &c)
{
  for (int i = 0; i < 3; i++)
  {
    m[0][i] = a[i];
    m[1][i] = b[i];
    m[2][i] = c[i];
  }
}
*/

Matrix4x4 &Matrix4x4::operator=(const Matrix4x4 &r)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      m[i][j] = r.m[i][j];
    }
  }

  return *this;
}

Float4 Matrix4x4::operator*(const Float4 &r) const
{
  Float4 ret;
  ret[0] = m[0][0] * r[0] + m[0][1] * r[1] + m[0][2] * r[2] + m[0][3] * r[3];
  ret[1] = m[1][0] * r[0] + m[1][1] * r[1] + m[1][2] * r[2] + m[1][3] * r[3];
  ret[2] = m[2][0] * r[0] + m[2][1] * r[1] + m[2][2] * r[2] + m[2][3] * r[3];
  ret[3] = m[3][0] * r[0] + m[3][1] * r[1] + m[3][2] * r[2] + m[3][3] * r[3];
  return ret;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &a) const
{
  //order of operations: a * this
  //c_(np)	=	a_(n1)b_(1p)+a_(n2)b_(2p)+...+a_(nm)b_(mp).
  Matrix4x4 ret;
  for (uint32_t i = 0; i < 4; i++)
  {
    for (uint32_t j = 0; j < 4; j++)
    {
      ret.m[j][i] = a.m[j][0] * m[0][i] + a.m[j][1] * m[1][i] + a.m[j][2] * m[2][i] + a.m[j][3] * m[3][i];
    }
  }

  return ret;
}

void Matrix4x4::identity()
{
  m[0][0] = 1.0f;   m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
  m[1][0] = 0.0f;   m[1][1] = 1.0f;   m[1][2] = 0.0f;   m[1][3] = 0.0f;
  m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = 1.0f;   m[2][3] = 0.0f;
  m[3][0] = 0.0f;   m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
}

void Matrix4x4::invert()
{

  Matrix4x4 ret;

  //first row
  ret.m[0][0] =  m[1][1] * m[2][2] * m[3][3] -
                 m[1][1] * m[3][2] * m[2][3] -
                 m[1][2] * m[2][1] * m[3][3] +
                 m[1][2] * m[3][1] * m[2][3] +
                 m[1][3] * m[2][1] * m[3][2] -
                 m[1][3] * m[3][1] * m[2][2];

  ret.m[0][1] = -m[0][1] * m[2][2] * m[3][3] +
                 m[0][1] * m[3][2] * m[2][3] +
                 m[0][2] * m[2][1] * m[3][3] -
                 m[0][2] * m[3][1] * m[2][3] -
                 m[0][3] * m[2][1] * m[3][2] +
                 m[0][3] * m[3][1] * m[2][2];

  ret.m[0][2] =  m[0][1] * m[1][2] * m[3][3] -
                 m[0][1] * m[3][2] * m[1][3] -
                 m[0][2] * m[1][1] * m[3][3] +
                 m[0][2] * m[3][1] * m[1][3] +
                 m[0][3] * m[1][1] * m[3][2] -
                 m[0][3] * m[3][1] * m[1][2];

  ret.m[0][3] = -m[0][1] * m[1][2] * m[2][3] +
                 m[0][1] * m[2][2] * m[1][3] +
                 m[0][2] * m[1][1] * m[2][3] -
                 m[0][2] * m[2][1] * m[1][3] -
                 m[0][3] * m[1][1] * m[2][2] +
                 m[0][3] * m[2][1] * m[1][2];

  ret.m[1][0] = -m[1][0] * m[2][2] * m[3][3] +
                 m[1][0] * m[3][2] * m[2][3] +
                 m[1][2] * m[2][0] * m[3][3] -
                 m[1][2] * m[3][0] * m[2][3] -
                 m[1][3] * m[2][0] * m[3][2] +
                 m[1][3] * m[3][0] * m[2][2];

  ret.m[1][1] =  m[0][0] * m[2][2] * m[3][3] -
                 m[0][0] * m[3][2] * m[2][3] -
                 m[0][2] * m[2][0] * m[3][3] +
                 m[0][2] * m[3][0] * m[2][3] +
                 m[0][3] * m[2][0] * m[3][2] -
                 m[0][3] * m[3][0] * m[2][2];

  ret.m[1][2] = -m[0][0] * m[1][2] * m[3][3] +
                 m[0][0] * m[3][2] * m[1][3] +
                 m[0][2] * m[1][0] * m[3][3] -
                 m[0][2] * m[3][0] * m[1][3] -
                 m[0][3] * m[1][0] * m[3][2] +
                 m[0][3] * m[3][0] * m[1][2];

  ret.m[1][3] =  m[0][0] * m[1][2] * m[2][3] -
                 m[0][0] * m[2][2] * m[1][3] -
                 m[0][2] * m[1][0] * m[2][3] +
                 m[0][2] * m[2][0] * m[1][3] +
                 m[0][3] * m[1][0] * m[2][2] -
                 m[0][3] * m[2][0] * m[1][2];

  ret.m[2][0] =  m[1][0] * m[2][1] * m[3][3] -
                 m[1][0] * m[3][1] * m[2][3] -
                 m[1][1] * m[2][0] * m[3][3] +
                 m[1][1] * m[3][0] * m[2][3] +
                 m[1][3] * m[2][0] * m[3][1] -
                 m[1][3] * m[3][0] * m[2][1];

  ret.m[2][1] = -m[0][0] * m[2][1] * m[3][3] +
                 m[0][0] * m[3][1] * m[2][3] +
                 m[0][1] * m[2][0] * m[3][3] -
                 m[0][1] * m[3][0] * m[2][3] -
                 m[0][3] * m[2][0] * m[3][1] +
                 m[0][3] * m[3][0] * m[2][1];

  ret.m[2][2] =  m[0][0] * m[1][1] * m[3][3] -
                 m[0][0] * m[3][1] * m[1][3] -
                 m[0][1] * m[1][0] * m[3][3] +
                 m[0][1] * m[3][0] * m[1][3] +
                 m[0][3] * m[1][0] * m[3][1] -
                 m[0][3] * m[3][0] * m[1][1];

  ret.m[2][3] = -m[0][0] * m[1][1] * m[2][3] +
                 m[0][0] * m[2][1] * m[1][3] +
                 m[0][1] * m[1][0] * m[2][3] -
                 m[0][1] * m[2][0] * m[1][3] -
                 m[0][3] * m[1][0] * m[2][1] +
                 m[0][3] * m[2][0] * m[1][1];

  ret.m[3][0] = -m[1][0] * m[2][1] * m[3][2] +
                 m[1][0] * m[3][1] * m[2][2] +
                 m[1][1] * m[2][0] * m[3][2] -
                 m[1][1] * m[3][0] * m[2][2] -
                 m[1][2] * m[2][0] * m[3][1] +
                 m[1][2] * m[3][0] * m[2][1];

  ret.m[3][1] =  m[0][0] * m[2][1] * m[3][2] -
                 m[0][0] * m[3][1] * m[2][2] -
                 m[0][1] * m[2][0] * m[3][2] +
                 m[0][1] * m[3][0] * m[2][2] +
                 m[0][2] * m[2][0] * m[3][1] -
                 m[0][2] * m[3][0] * m[2][1];

  ret.m[3][2] = -m[0][0] * m[1][1] * m[3][2] +
                 m[0][0] * m[3][1] * m[1][2] +
                 m[0][1] * m[1][0] * m[3][2] -
                 m[0][1] * m[3][0] * m[1][2] -
                 m[0][2] * m[1][0] * m[3][1] +
                 m[0][2] * m[3][0] * m[1][1];

  ret.m[3][3] =  m[0][0] * m[1][1] * m[2][2] -
                 m[0][0] * m[2][1] * m[1][2] -
                 m[0][1] * m[1][0] * m[2][2] +
                 m[0][1] * m[2][0] * m[1][2] +
                 m[0][2] * m[1][0] * m[2][1] -
                 m[0][2] * m[2][0] * m[1][1];

  float det = m[0][0] * ret.m[0][0] + m[1][0] * ret.m[0][1] + m[2][0] * ret.m[0][2] + m[3][0] * ret.m[0][3];

  if (det == 0) { return; } //can't invert

  det = 1.0f / det;

  for (uint32_t i = 0; i < 4; i++)
  {
    for (uint32_t j = 0; j < 4; j++)
    {
      ret.m[i][j] = det * ret.m[i][j];
    }
  }

  *this = ret;
}

void Matrix4x4::transpose()
{
  Matrix4x4 temp(m[0][0], m[1][0], m[2][0], m[3][0],
                 m[0][1], m[1][1], m[2][1], m[3][1],
                 m[0][2], m[1][2], m[2][2], m[3][2],
                 m[0][3], m[1][3], m[2][3], m[3][3]);
  *this = temp;
}

/*
void Matrix3x3::rotation_from_quaternion(const Quaternion &q)
{
  float x2 = q.q[1] * q.q[1];
  float y2 = q.q[2] * q.q[2];
  float z2 = q.q[3] * q.q[3];
  float w2 = q.q[0] * q.q[0];

  float two_x = 2.0f * q.q[1];
  float two_y = 2.0f * q.q[2];
  float two_w = 2.0f * q.q[0];

  float xy = two_x * q.q[2];
  float xz = two_x * q.q[3];
  float yz = two_y * q.q[3];
  float wx = two_w * q.q[1];
  float wy = two_w * q.q[2];
  float wz = two_w * q.q[3];

  m[0][0] = w2 + x2 - y2 - z2;
  m[0][1] = xy - wz;
  m[0][2] = xz + wy;

  m[1][0] = xy + wz;
  m[1][1] = w2 - x2 + y2 - z2;
  m[1][2] = yz - wx;

  //fill in the third row
  m[2][0] = xz - wy;
  m[2][1] = yz + wx;
  m[2][2] = w2 - x2 - y2 + z2;
}
*/

void Matrix4x4::ortho(float left, float right, float bottom, float top, float near_clip, float far_clip)
{
  m[0][0] = 2.0f / (right - left);
  m[1][0] = 0.0f;
  m[2][0] = 0.0f;
  m[3][0] = -(right + left) / (right - left);

  m[0][1] = 0.0f;
  m[1][1] = 2.0f / (top - bottom);
  m[2][1] = 0.0f;
  m[3][1] = -(top + bottom) / (top - bottom);

  m[0][2] = 0.0f;
  m[1][2] = 0.0f;
  m[2][2] = -2.0f / (far_clip - near_clip);
  m[3][2] = -(far_clip + near_clip) / (far_clip - near_clip);

  m[0][3] = 0.0f;
  m[1][3] = 0.0f;
  m[2][3] = 0.0f;
  m[3][3] = 1.0f;
}

void Matrix4x4::perspective(float fovy, float aspect, float zNear, float zFar)
{
  float xymax = zNear * (float)tan(fovy * M_PI / 360.0f);// PI_OVER_360);
  float ymin = -xymax;
  float xmin = -xymax;

  float width = xymax - xmin;
  float height = xymax - ymin;

  float depth = zFar - zNear;
  float q = -(zFar + zNear) / depth;
  float qn = -2 * (zFar * zNear) / depth;

  float w = 2 * zNear / width;
  w = w / aspect;
  float h = 2 * zNear / height;

  m[0][0] = w;
  m[0][1] = 0;
  m[0][2] = 0;
  m[0][3] = 0;

  m[1][0] = 0;
  m[1][1] = h;
  m[1][2] = 0;
  m[1][3] = 0;

  m[2][0] = 0;
  m[2][1] = 0;
  m[2][2] = q;
  m[2][3] = -1.0f;

  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = qn;
  m[3][3] = 0;
}

void Matrix4x4::look_at(Float3 &eye_pos, Float3 &look_at_pos, Float3 &up)
{
  identity();

  Float3  f = look_at_pos - eye_pos;
  f.normalize();
  Float3 u = up;
  u.normalize();
  Float3 s = f ^ u;
  s.normalize();
  u = s ^ f;

  m[0][0] = s[0];
  m[1][0] = s[1];
  m[2][0] = s[2];

  m[0][1] = u[0];
  m[1][1] = u[1];
  m[2][1] = u[2];

  m[0][2] = -f[0];
  m[1][2] = -f[1];
  m[2][2] = -f[2];

  m[3][0] = -(u * eye_pos);
  m[3][1] = -(s * eye_pos);
  m[3][2] = (f * eye_pos);

  //transpose();

  /*
  //assume up vector is already normalized
  Float3 f = look_at_pos - eye_pos;
  f.normalize();

  Float3 right = f ^ up;
  Float3 u = right ^ f;

  m[0][0] = right[0];
  m[0][1] = right[1];
  m[0][2] = right[2];
  m[0][3] = eye_pos[0];

  m[1][0] = -u[0];
  m[1][1] = -u[1];
  m[1][2] = -u[2];
  m[1][3] = eye_pos[1];

  m[2][0] = f[0];
  m[2][1] = f[1];
  m[2][2] = f[2];
  m[2][3] = eye_pos[2];

  m[3][0] = 0.0f;
  m[3][1] = 0.0f;
  m[3][2] = 0.0f;
  m[3][3] = 1.0f;
  */
}

ostream& Math::operator<<(ostream &os, const Matrix2x2 &m)
{
  os << m(0, 0) << " " << m(0, 1) << endl;
  os << m(1, 0) << " " << m(1, 1) << endl;

  return os;
}

ostream& Math::operator<<(ostream &os, const Matrix3x3 &m)
{
  os << m(0, 0) << " " << m(0, 1) << " " << m(0, 2) << endl;
  os << m(1, 0) << " " << m(1, 1) << " " << m(1, 2) << endl;
  os << m(2, 0) << " " << m(2, 1) << " " << m(2, 2) << endl;

  return os;
}

ostream& Math::operator<<(ostream &os, const Matrix4x4 &m)
{
  os << m(0, 0) << " " << m(0, 1) << " " << m(0, 2) << " " << m(0, 3) << endl;
  os << m(1, 0) << " " << m(1, 1) << " " << m(1, 2) << " " << m(1, 3) << endl;
  os << m(2, 0) << " " << m(2, 1) << " " << m(2, 2) << " " << m(2, 3) << endl;
  os << m(3, 0) << " " << m(3, 1) << " " << m(3, 2) << " " << m(3, 3) << endl;

  return os;
}

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "quaternion.h"
#include "vector.h"

//
// TODO: use template<int N> to templatize the dimensions of the matrix
//

namespace Math {
  class Matrix2x2
  {
    public:
      Matrix2x2();
      Matrix2x2(const float _00, const float _01, const float _10, const float _11);
      ~Matrix2x2() {};

      Matrix2x2 operator*(const Matrix2x2 &r);
      Float2 operator*(const Float2 &r);

      inline float &operator()(const int &row, const int &col) { return m[row][col]; }
      inline float operator()(const int &row, const int &col) const { return m[row][col]; }

      void identity();
      void rotation(float rad);

    private:
      float m[2][2];
  };

  class Matrix3x3
  {
    public:
      Matrix3x3();
      Matrix3x3(const float _00, const float _01, const float _02,
                const float _10, const float _11, const float _12,
                const float _20, const float _21, const float _22);
      Matrix3x3(const Float3 &a, const Float3 &b, const Float3 &c);

      float determinant() const;

      ~Matrix3x3() {};

      Matrix3x3 &operator=(const Matrix3x3 &r);

      Float3 operator*(const Float3 &r) const;
      Matrix3x3 operator*(const Matrix3x3 &a) const;

      inline float &operator()(const int &row, const int &col) { return m[row][col]; }
      inline float operator()(const int &row, const int &col) const { return m[row][col]; }

      void identity();
      void invert();
      void transpose();
      void rotation_from_quaternion(const Quaternion &q);
    private:
      float m[3][3];
  };

  class Matrix4x4
  {
  public:
    Matrix4x4();
    Matrix4x4(const float _00, const float _01, const float _02, const float _03,
              const float _10, const float _11, const float _12, const float _13,
              const float _20, const float _21, const float _22, const float _23,
              const float _30, const float _31, const float _32, const float _33);
    //Matrix4x4(const Float4 &a, const Float4 &b, const Float4 &c, const Float4 &d);

    ~Matrix4x4() {};

    Matrix4x4 &operator=(const Matrix4x4 &r);

    //Float4 operator*(const Float4 &r) const;
    Matrix4x4 operator*(const Matrix4x4 &a) const;

    inline float &operator()(const int &row, const int &col) { return m[row][col]; }
    inline float operator()(const int &row, const int &col) const { return m[row][col]; }

    void identity();
    void invert();
    void transpose();
    //void rotation_from_quaternion(const Quaternion &q);
    
    //projection matrices
    void ortho(float left, float right, float bottom, float top, float near_clip, float far_clip);
    void perspective(float fov, float aspect_ratio, float znear, float zfar);

    //camera helpers
    void look_at(Float3 &pos, Float3 &look_at_pos, Float3 &up);
  private:
    float m[4][4];
  };


  inline void orient_to_sphere_surface(Float3 &pos, Matrix3x3 &orientation)
  {
    /*
    Float3 up(pos);
    up.normalize();

    Float3 left = up ^ Float3(0.0f, 1.0f, 0.0f);
    left.normalize();

    Float3 forward = left ^ up;

    Matrix3x3 orientation(left, up, forward);
    ht->set_orientation(orientation);*/
    Float3 up(0.0f, 1.0f, 0.0f);
    Float3 axis = Float3(0.0f, 1.0f, 0.0f) ^ pos;
    axis.normalize();
    float angle = acos(up * pos);

    Quaternion q;
    q.rotation_from_axis_angle(axis, angle);
    orientation.rotation_from_quaternion(q);
  }

  std::ostream& operator<<(std::ostream &os, const Matrix2x2 &m);
  std::ostream& operator<<(std::ostream &os, const Matrix3x3 &m);
  std::ostream& operator<<(std::ostream &os, const Matrix4x4 &m);
}


#endif //__MATRIX_H__

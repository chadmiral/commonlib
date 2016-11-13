#pragma once

#include <math.h>
#include <iostream>

namespace Math {

  inline float inv_sqrt(float x)
  {
    float xhalf = 0.5f * x;
    int i = *(int*)&x;              // get bits for floating value
    i = 0x5f375a86 - (i >> 1);      // gives initial guess y0
    x = *(float*)&i;                // convert bits back to float
    x = x * (1.5f - xhalf * x * x); // Newton step, repeating increases accuracy
    return x;
  }

  class Float2 {
  public:
    Float2();
    Float2(const float _x, const float _y);
    Float2(const Float2 &b);

    inline Float2 &operator=(const Float2 &r) { _val[0] = r._val[0]; _val[1] = r._val[1]; return *this; }
    inline Float2 operator+(const Float2 &r) const { return Float2(_val[0] + r._val[0], _val[1] + r._val[1]); }
    inline Float2 operator-(const Float2 &r) const { return Float2(_val[0] - r._val[0], _val[1] - r._val[1]); }
    inline Float2 operator/(const float &r) const { return Float2(_val[0] / r, _val[1] / r); }
    inline Float2 operator*(const float &r) const { return Float2(r * _val[0], r * _val[1]); }

    inline float &operator[](const int idx) { return _val[idx]; }
    inline float operator[](const int idx) const { return _val[idx]; }

    //dot product
    inline float operator*(const Float2 &b) const { return b._val[0] * _val[0] + b._val[1] * _val[1]; }

    inline float &x() { return _val[0]; }
    inline float &y() { return _val[1]; }

    //swizzle
    //inline Float2 xy() const { return Float2(_val[0], _val[1]); }
    inline Float2 yx() const { return Float2(_val[1], _val[0]); }

    inline void normalize()
    {
      float sqr_sum = _val[0] * _val[0] + _val[1] * _val[1];
      float i_sqrt = inv_sqrt(sqr_sum);
      _val[0] = i_sqrt * _val[0];
      _val[1] = i_sqrt * _val[1];
    }//float m = magnitude(); _val[0] /= m; _val[1] /= m; }
    inline float magnitude() const { return sqrt(_val[0] * _val[0] + _val[1] * _val[1]); }
    inline float mag_squared() const { return _val[0] * _val[0] + _val[1] * _val[1]; }

    //data
    float _val[2];
  };

  class Float3 {
  public:
    Float3();
    Float3(const float _x, const float _y, const float _z);
    Float3(const Float2 &b, const float _z);
    Float3(const Float3 &b);

    inline Float3 &operator=(const Float3 &r) { _val[0] = r._val[0]; _val[1] = r._val[1]; _val[2] = r._val[2]; return *this; }
    inline Float3 operator+(const Float3 &r) const { return Float3(_val[0] + r._val[0], _val[1] + r._val[1], _val[2] + r._val[2]); }
    inline Float3 operator-(const Float3 &r) const { return Float3(_val[0] - r._val[0], _val[1] - r._val[1], _val[2] - r._val[2]); }
    inline Float3 operator/(const float &r) const { return Float3(_val[0] / r, _val[1] / r, _val[2] / r); }
    inline Float3 operator*(const float &r) const { return Float3(r * _val[0], r * _val[1], r * _val[2]); }

    inline float &operator[](const int idx) { return _val[idx]; }
    inline float operator[](const int idx) const { return _val[idx]; }

    //dot product
    inline float operator*(const Float3 &r) const { return _val[0] * r._val[0] + _val[1] * r._val[1] + _val[2] * r._val[2]; }

    //cross product
    inline Float3 operator^(const Float3 &r) const {
      return Float3(_val[1] * r._val[2] - _val[2] * r._val[1],
        _val[2] * r._val[0] - _val[0] * r._val[2],
        _val[0] * r._val[1] - _val[1] * r._val[0]);
    }


    inline float &x() { return _val[0]; }
    inline float &y() { return _val[1]; }
    inline float &z() { return _val[2]; }

    inline float &phi() { return _val[0]; }
    inline float &theta() { return _val[1]; }
    inline float &rad() { return _val[2]; }

    //swizzle
    inline Float3 xzy() const { return Float3(_val[0], _val[2], _val[1]); }
    inline Float3 yxz() const { return Float3(_val[1], _val[0], _val[2]); }
    inline Float3 yzx() const { return Float3(_val[1], _val[2], _val[0]); }
    inline Float3 zxy() const { return Float3(_val[2], _val[0], _val[1]); }
    inline Float3 zyx() const { return Float3(_val[2], _val[1], _val[0]); }

    //length / normalization
    inline void normalize()
    {
      //float m = magnitude(); _val[0] /= m; _val[1] /= m; _val[2] /= m;
      float sqr_sum = _val[0] * _val[0] + _val[1] * _val[1] + _val[2] * _val[2];
      float i_sqrt = inv_sqrt(sqr_sum);
      _val[0] = i_sqrt * _val[0];
      _val[1] = i_sqrt * _val[1];
      _val[2] = i_sqrt * _val[2];
    }
    inline float magnitude() const { return sqrt(_val[0] * _val[0] + _val[1] * _val[1] + _val[2] * _val[2]); }
    inline float mag_squared() const { return _val[0] * _val[0] + _val[1] * _val[1] + _val[2] * _val[2]; }

    //data
    float _val[3];
  };

  inline Float2 operator*(const float &l, const Float2 &r) { return Float2(l * r._val[0], l * r._val[1]); }
  inline Float3 operator*(const float &l, const Float3 &r) { return Float3(l * r._val[0], l * r._val[1], l * r._val[2]); }
  inline std::ostream &operator<<(std::ostream &os, const Float3 &obj) { os << "(" << obj[0] << ", " << obj[1] << ", " << obj[2] << ")"; return os; }
  inline std::ostream &operator<<(std::ostream &os, const Float2 &obj) { os << "(" << obj[0] << ", " << obj[1] << ")"; return os; }

  inline Float2 midpoint(Float2 &a, Float2 &b) { return a + 0.5f * (b - a); }
  inline Float3 midpoint(Float3 &a, Float3 &b) { return a + 0.5f * (b - a); }

  inline float distance(const Float2 &a, const Float2 &b)
  {
    float x = a._val[0] - b._val[0];
    float y = a._val[1] - b._val[1];
    return sqrt(x * x + y * y);
  }
  
  inline float dist_squared(const Float2 &a, const Float2 &b)
  {
    float x = a._val[0] - b._val[0];
    float y = a._val[1] - b._val[1];
    return x * x + y * y;
  }

  inline float distance(const Float3 &a, const Float3 &b)
  {
    float x = a._val[0] - b._val[0];
    float y = a._val[1] - b._val[1];
    float z = a._val[2] - b._val[2];
    return sqrt(x * x + y * y + z * z);
  }

  inline float dist_squared(const Float3 &a, const Float3 &b)
  {
    float x = a._val[0] - b._val[0];
    float y = a._val[1] - b._val[1];
    float z = a._val[2] - b._val[2];
    return x * x + y * y + z * z;
  }
};

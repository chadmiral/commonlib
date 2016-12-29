#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include <iostream>
#include "math_utility.h"

namespace Math {
  class Quaternion
  {
  public:
    Quaternion();


    Quaternion(const float &w, const float &x, const float &y, const float &z);
    Quaternion(const float &x, const float &y, const float &z); // For 3-rotations
    Quaternion(const float &r);


    // Copy constructor and assignment
    Quaternion(const Quaternion &_q);
    Quaternion& operator=(const Quaternion &_q);

    // Unary operators
    Quaternion operator-() const;
    Quaternion operator~() const; // Conjugate

    inline float &operator[](const int idx) { return q[idx]; }
    inline float operator[](const int idx) const { return q[idx]; }


    float norm_squared() const;
    void normalize();

    //create a rotation quaternion from a NORMALIZED axis and an angle
    void rotation_from_axis_angle(const Float3 &norm_axis, const float theta_rad);

    // In-place operators
    Quaternion& operator+=(const float &r);
    Quaternion& operator+=(const Quaternion &_q);

    Quaternion& operator-=(const float &r) { q[0] -= r; return *this; }
    Quaternion& operator-=(const Quaternion &_q) { q[0] -= _q.q[0]; q[1] -= _q.q[1]; q[2] -= _q.q[2]; q[3] -= _q.q[3]; return *this; }

    Quaternion& operator*=(const float &r) { q[0] *= r; q[1] *= r; q[2] *= r; q[3] *= r; return *this; }
    Quaternion& operator*=(const Quaternion &_q);

    Quaternion& operator/=(const float &r) { q[0] /= r; q[1] /= r; q[2] /= r; q[3] /= r; return *this; }
    Quaternion& operator/=(const Quaternion &q);


    // Binary operators based on in-place operators
    Quaternion operator+(const float &r) const { return Quaternion(*this) += r; }
    Quaternion operator+(const Quaternion &_q) const { return Quaternion(*this) += _q; }
    Quaternion operator-(const float &r) const { return Quaternion(*this) -= r; }
    Quaternion operator-(const Quaternion &_q) const { return Quaternion(*this) -= _q; }
    Quaternion operator*(const float &r) const { return Quaternion(*this) *= r; }
    Quaternion operator*(const Quaternion &_q) const { return Quaternion(*this) *= _q; }
    Quaternion operator/(const float &r) const { return Quaternion(*this) /= r; }
    Quaternion operator/(const Quaternion &_q) const { return Quaternion(*this) /= _q; }


    // Comparison operators, as much as they make sense
    bool operator==(const Quaternion &_q) const { return (q[0] == _q.q[0]) && (q[1] == _q.q[1]) && (q[2] == _q.q[2]) && (q[3] == _q.q[3]); }
    bool operator!=(const Quaternion &_q) const { return !operator==(_q); }


    float q[4];
  };



  Quaternion operator+(const float &r, const Quaternion &_q);
  Quaternion operator-(const float &r, const Quaternion &_q);
  Quaternion operator*(const float &r, const Quaternion &_q);

  Quaternion operator/(const float &r, const Quaternion &_q);

  std::ostream& operator<<(std::ostream &io, const Quaternion &_q);
}


#endif //__QUATERNION_H__

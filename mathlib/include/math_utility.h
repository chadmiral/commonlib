#ifndef _MATH_UTILITY_H_
#define _MATH_UTILITY_H_

#include <math.h>
#include <iostream>

#include "vector.h"

#ifndef M_PHI
#define M_PHI   1.61803398874989484820
#endif //M_PHI

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif //M_PI

namespace Math
{
  inline float lerp(float x, float y, float m) { return (x * (1.0f - m) + y * m); }
  inline float cerp(float x, float y, float m) { float mu2 = (1.0f - (float)cos(m * M_PI)) / 2.0f; return (x * (1.0f - mu2) + y * mu2); }

  inline Float2 lerp(Float2 &x, Float2 &y, float m)
  {
    float one_minus_m = 1.0f - m;
    float ret_x = x._val[0] * one_minus_m + y._val[0] * m;
    float ret_y = x._val[1] * one_minus_m + y._val[1] * m;
    return Float2(ret_x, ret_y);
  }
  inline Float2 cerp(Float2 &x, Float2 &y, float m) { return Float2(cerp(x._val[0], y._val[0], m), cerp(x._val[1], y._val[1], m)); }

  inline Float3 lerp(Float3 &x, Float3 &y, float m)
  {
    float one_minus_m = 1.0f - m;
    float ret_x = x._val[0] * one_minus_m + y._val[0] * m;
    float ret_y = x._val[1] * one_minus_m + y._val[1] * m;
    float ret_z = x._val[2] * one_minus_m + y._val[2] * m;
    return Float3(ret_x, ret_y, ret_z);
  }
  inline Float3 cerp(Float3 &x, Float3 &y, float m) { return Float3(cerp(x._val[0], y._val[0], m), cerp(x._val[1], y._val[1], m), cerp(x._val[2], y._val[2], m)); }

  float clamp(float x, float a, float b);
  //float max(float a, float b);
  //float min(float a, float b);

  float inline remap_range(float x, float old_min, float old_max, float new_min, float new_max)
  {
    return (((x - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min;
  }

  Float2 inline remap_range(Float2 &x, Float2 &old_min, Float2 &old_max, Float2 &new_min, Float2 &new_max)
  {
    return Float2((((x._val[0] - old_min._val[0]) * (new_max._val[0] - new_min._val[0])) / (old_max._val[0] - old_min._val[0])) + new_min._val[0],
                  (((x._val[1] - old_min._val[1]) * (new_max._val[1] - new_min._val[1])) / (old_max._val[1] - old_min._val[1])) + new_min._val[1]);
  }

  Float3 inline remap_range(Float3 &x, Float3 &old_min, Float3 &old_max, Float3 &new_min, Float3 &new_max)
  {
    return Float3((((x._val[0] - old_min._val[0]) * (new_max._val[0] - new_min._val[0])) / (old_max._val[0] - old_min._val[0])) + new_min._val[0],
                  (((x._val[1] - old_min._val[1]) * (new_max._val[1] - new_min._val[1])) / (old_max._val[1] - old_min._val[1])) + new_min._val[1],
                  (((x._val[2] - old_min._val[2]) * (new_max._val[2] - new_min._val[2])) / (old_max._val[2] - old_min._val[2])) + new_min._val[2]);
  }

  float random(float a, float b);
  int random(int a, int b);

  inline float degrees_to_radians(float d) { return (d / 180.0f) * (float)M_PI; }
  inline float radians_to_degrees(float r) { return (r / (float)M_PI) * 180.0f; }

  inline float ccw(const Float2 &a, const Float2 &b, const Float2 &c)
  {
    return (b._val[0] - a._val[0]) * (c._val[1] - b._val[1]) - (b._val[1] - a._val[1]) * (c._val[0] - b._val[0]);
  }

  inline Float3 polar_to_cartesian(const float theta, const float phi, const float r)
  {
    float cp = cos(phi);
    float ct = cos(theta);
    float sp = sin(phi);
    float st = sin(theta);
    return Float3(r * ct * cp, r * sp, r * st * cp);
  }

  inline Float3 cartesian_to_polar(const Float3 p)
  {
    Float3 p_copy = p;
    Float3 phi_theta_r;
    float mag = p_copy.magnitude();
    phi_theta_r.rad() = mag;
    p_copy = p_copy / mag;
  
    phi_theta_r.phi() = 0.5f + (atan2(p_copy[2], p_copy[0]) / (float)M_PI) * 0.5f;
    phi_theta_r.theta() = asin(p_copy[1]) / (float)(M_PI) + 0.5f;
    
    return phi_theta_r;
  }

	uint32_t hash_value_from_string(const char *str);
}

#endif // _MATH_UTILITY_H_

#pragma once

#include "math_utility.h"

namespace Graphics
{
  class Light
  {
  protected:
    Math::Float3 _diffuse;
    Math::Float3 _specular;
    Math::Float3 _ambient;

    float _intensity;
    Math::Float3 _pos;
  public:
    Light() {}
    ~Light() {}

    void set_diffuse_color(Math::Float3 d) { _diffuse = d; }
    Math::Float3 get_diffuse_color() const { return _diffuse; }
    void set_specular_color(Math::Float3 s) { _specular = s; }
    Math::Float3 get_specular_color() const { return _specular; }
    void set_ambient_color(Math::Float3 a) { _ambient = a; }
    Math::Float3 get_ambient_color() const { return _ambient; }
    void set_intensity(const float i) { _intensity = i; }
    float get_intensity() const { return _intensity; }
    void set_pos(Math::Float3 p) { _pos = p; }
    Math::Float3 get_pos() const { return _pos; }
  };
};
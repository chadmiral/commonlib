#pragma once

#include "camera.h"
#include "light.h"

namespace Graphics
{
  class RenderContext
  {
  private:
    std::vector<Light *> _lights;
    Camera *_camera;
    float _camera_distance;
  public:
    RenderContext() { _camera = NULL; }
    ~RenderContext() {}

    Light *get_sun_light() { return _lights[0]; }

    //TODO: this is sloppy
    Math::Float3 transform_light_pos_to_screen_space(Light *l)
    {
      Math::Float3 up = _camera->get_up() * -1.0f;
      Math::Float3 lookat = _camera->get_lookat();
      Math::Float3 right = lookat ^ up;

      Math::Float3 view_pos = _camera->get_pos();

      Math::Matrix3x3 view_mat(right, up, lookat);
      Math::Float3 light_pos_transformed = view_mat * (l->get_pos() + view_pos);

      return light_pos_transformed;
    }

    void set_camera(Camera *c) { _camera = c; }
    void set_camera_distance(const float d) { _camera_distance = d; }
    void add_light(Light *l) { _lights.push_back(l); }
  };
};
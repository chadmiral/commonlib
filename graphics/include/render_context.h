#pragma once

#include "camera.h"
#include "light.h"
#include "matrix.h"

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

    Camera *get_camera() { return _camera; }

    Light *get_sun_light() { return _lights[0]; }

    //TODO: this is sloppy
    Math::Float3 transform_light_pos_to_screen_space(Light *l)
    {
      //transform sun_pos?
      Math::Matrix4x4 eye_mat;
      _camera->get_model_view_matrix(&eye_mat);
      eye_mat.transpose();
      //eye_mat.invert();

      Math::Float4 light_pos(l->get_pos(), 1.0f);
      Math::Float4 transformed_light_pos = eye_mat * light_pos;

      return transformed_light_pos.xyz();

      /*
      Math::Float3 up = _camera->get_up() * -1.0f;
      Math::Float3 lookat = _camera->get_lookat();

      up.normalize();
      lookat.normalize();
      
      Math::Float3 right = lookat ^ up;

      Math::Float3 view_pos = _camera->get_pos();

      Math::Matrix3x3 view_mat(right, up, lookat);
      Math::Float3 light_pos_transformed = view_mat * (l->get_pos() + view_pos);

      return light_pos_transformed;
      */
    }

    void set_camera(Camera *c) { _camera = c; }
    void set_camera_distance(const float d) { _camera_distance = d; }
    void add_light(Light *l) { _lights.push_back(l); }
  };
};

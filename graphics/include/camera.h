#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "platform_gl.h"

#include "math_utility.h"
#include "matrix.h"

namespace Graphics
{
  class Camera
  {
  public:
    Camera();
    ~Camera() {}

    void set_pos(const Math::Float3 p) { pos = p; }
    Math::Float3 get_pos() const { return pos; }
    void set_up(const Math::Float3 u) { up = u; }
    Math::Float3 get_up() const { return up; }
    void set_lookat(const Math::Float3 l) { lookat = l; }
    Math::Float3 get_lookat() const { return lookat; }
    Math::Float3 get_right() const { return lookat ^ up; }

    void set_projection_matrix(GLfloat *proj);
    void set_model_view_matrix(GLfloat *proj);

    void transform(const Math::Matrix3x3 &m);
    void rotate_in_place(const Math::Matrix3x3 &m);

    void set_window_dimensions(const Math::Float2 d) { window_dimensions = d; }
    void set_fov(const float f) { fov = f; }
    void set_f_stop(const float f) { f_stop = f; }
    void set_focal_distance(const float d) { focal_distance_m = d; }
    void set_focal_length(const float l) { focal_length_mm = l; }
    void set_shutter_speed(const float s) { shutter_speed_s = s; }

    void set_camera_parameters(const float fs, const float fd, const float fl, const float ss);

    void render_setup(const float znear = 0.05f, const float zfar = 100.0f);
    void render_cleanup();
  private:
    Math::Float3 pos;
    Math::Float3 up;
    Math::Float3 lookat;

    Math::Float2    window_dimensions;

    bool            use_proj_mat;
    bool            use_model_view_mat;
    GLfloat         model_view_mat[16];
    GLfloat         proj_mat[16];

    float           fov;                    //field of vision
    float           f_stop;                 //aperture size
    float           focal_distance_m;       //distance that we are focused on
    float           focal_length_mm;        //focal length (zoom)
    float           shutter_speed_s;        //how long the shutter is open (motion_blur)

    //std::vector<ScreenEffect *> effects;
    //float spherical_aberration;
    //float vignetting;
    //Float3 tint;
  };
};

#endif //_CAMERA_H__

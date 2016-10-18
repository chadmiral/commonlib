#include "camera.h"
#include "gl_error.h"

using namespace Math;
using namespace Graphics;

Camera::Camera()
{
  pos = Float3(0.0f, 0.0f, -10.0f);
  up = Float3(0.0f, 1.0f, 0.0f);
  lookat = Float3(0.0f, 0.0f, 1.0f);

  f_stop = 16.0f;
  focal_distance_m = 10000.0f;
  focal_length_mm = 35.0f;
  shutter_speed_s = 0.01f;

  fov = 32.0f;

  use_proj_mat = false;
  use_model_view_mat = false;
}

void Camera::transform(const Matrix3x3 &m)
{
  pos = m * pos;
  up = m * up;
  lookat = m * lookat;
}

void Camera::rotate_in_place(const Matrix3x3 &m)
{
  up = m * up;
  lookat = m * lookat;
}

void Camera::set_projection_matrix(GLfloat *proj)
{
  use_proj_mat = true;
  memcpy(proj_mat, proj, sizeof(GLfloat) * 16);
}

void Camera::set_model_view_matrix(GLfloat *proj)
{
  use_model_view_mat = true;
  memcpy(model_view_mat, proj, sizeof(GLfloat) * 16);
}

void Camera::set_camera_parameters(const float fs, const float fd, const float fl, const float ss)
{
  f_stop = fs;
  focal_distance_m = fd;
  focal_length_mm = fl;
  shutter_speed_s = ss;
}

void Camera::render_setup(const float znear, const float zfar)
{
  gl_check_error();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if (use_proj_mat)
  {
    glLoadMatrixf(proj_mat);
  }
  else
  {
    gluPerspective(fov, window_dimensions[0] / window_dimensions[1], znear, zfar);
  }

  gl_check_error();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (use_model_view_mat)
  {
    glLoadMatrixf(model_view_mat);
  }
  else
  {
    Float3 lookat_pos = pos + lookat;
    gluLookAt(pos[0], pos[1], pos[2],
              lookat_pos[0], lookat_pos[1], lookat_pos[2],
              up[0], up[1], up[2]);
  }

  gl_check_error();
}

void Camera::render_cleanup()
{
}

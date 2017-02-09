#include "camera.h"
#include "gl_error.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

void Camera::get_projection_matrix(Matrix4x4 *m, float znear, float zfar)
{
  //TODO: only compute this once per frame, instead of once every time we query
  if (use_proj_mat)
  {
    *m = Matrix4x4(proj_mat[0], proj_mat[1], proj_mat[2], proj_mat[3],
                   proj_mat[4], proj_mat[5], proj_mat[6], proj_mat[7],
                   proj_mat[8], proj_mat[9], proj_mat[10], proj_mat[11],
                   proj_mat[12], proj_mat[13], proj_mat[14], proj_mat[15]);
  }
  else
  {
    m->perspective(fov, window_dimensions[0] / window_dimensions[1], znear, zfar);
  }
}

void Camera::get_model_view_matrix(Matrix4x4 *m)
{
  Float3 lookat_pos = pos + lookat;
  //m->look_at(pos, lookat_pos, up);

  glm::vec3 eye(pos[0], pos[1], pos[2]);
  glm::vec3 center(lookat_pos[0], lookat_pos[1], lookat_pos[2]);
  glm::vec3 up(up[0], up[1], up[2]);

  glm::mat4 cam_mat = glm::lookAt(eye, center, up);

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      (*m)(i, j) = cam_mat[i][j];
    }
  }
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
#ifndef __LOKI__
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
#endif //__LOKI__
}

void Camera::render_cleanup()
{
}

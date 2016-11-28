#include "pawn.h"
#include "light.h"

using namespace Game;
using namespace Graphics;
using namespace Math;

void Pawn3D::init()
{
  sun_pos.set_name("sun_pos");
  mat.add_uniform_var(&sun_pos);
  sun_amb_rgb.set_name("sun_amb_rgb");
  mat.add_uniform_var(&sun_amb_rgb);
  sun_diff_rgb.set_name("sun_diff_rgb");
  mat.add_uniform_var(&sun_diff_rgb);
  sun_spec_rgb.set_name("sun_spec_rgb");
  mat.add_uniform_var(&sun_spec_rgb);
  cam_distance.set_name("cam_distance");
  mat.add_uniform_var(&cam_distance);

  mat.init();
  mesh.init();
}

void Pawn3D::render(const double game_time)
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  {
    Object3D::render(game_time);

    //update lighting uniform variables
    Light *sun_light = game_context->render_context.get_sun_light();
    Float3 sun_pos_screen_space = game_context->render_context.transform_light_pos_to_screen_space(sun_light);
    sun_pos.set_var(sun_pos_screen_space);
    sun_amb_rgb.set_var(sun_light->get_ambient_color());
    sun_diff_rgb.set_var(sun_light->get_diffuse_color());
    sun_spec_rgb.set_var(sun_light->get_specular_color());
    cam_distance.set_var(Float3(1.0f, 1.0f, 1.0f)); //TODO

    //mat.render();
    mesh.render(&mat);
    //mat.cleanup();
  }
  glPopMatrix();
}

void Pawn3D::simulate(const double game_time, const double frame_time)
{

}

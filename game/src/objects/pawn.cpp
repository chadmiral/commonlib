#include "pawn.h"
#include "light.h"

using namespace Game;
using namespace Graphics;
using namespace Math;

void Pawn3D::init()
{
  _sun_pos.set_name("sun_pos");
  _mat.add_uniform_var(&_sun_pos);
  _sun_amb_rgb.set_name("sun_amb_rgb");
  _mat.add_uniform_var(&_sun_amb_rgb);
  _sun_diff_rgb.set_name("sun_diff_rgb");
  _mat.add_uniform_var(&_sun_diff_rgb);
  _sun_spec_rgb.set_name("sun_spec_rgb");
  _mat.add_uniform_var(&_sun_spec_rgb);
  _cam_distance.set_name("cam_distance");
  _mat.add_uniform_var(&_cam_distance);

  _mat.init();
}

void Pawn3D::render(const double game_time)
{
  assert(_mesh);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  {
    Object3D::render(game_time);

    //update lighting uniform variables
    Light *sun_light = _game_context->render_context.get_sun_light();
    Float3 sun_pos_screen_space = _game_context->render_context.transform_light_pos_to_screen_space(sun_light);
    _sun_pos.set_var(sun_pos_screen_space);
    _sun_amb_rgb.set_var(sun_light->get_ambient_color());
    _sun_diff_rgb.set_var(sun_light->get_diffuse_color());
    _sun_spec_rgb.set_var(sun_light->get_specular_color());
    _cam_distance.set_var(Float3(1.0f, 1.0f, 1.0f)); //TODO

    _mat.render();
    _mesh->render();
    _mat.cleanup();
  }
  glPopMatrix();
}

void Pawn3D::simulate(const double game_time, const double frame_time)
{

}

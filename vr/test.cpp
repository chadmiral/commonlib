#if defined (_WIN32)
#include <Windows.h>
#include <GL/glew.h>
#endif //_WIN32

#include <iostream>

#include "sdl_game.h"
#include "vr.h"
#include "math_utility.h"
#include "perlin.h"
#include "camera.h"
#include "static_mesh.h"
#include "material.h"
#include "shader.h"

using namespace std;
using namespace VR;
using namespace Math;
using namespace PerlinNoise;
using namespace Graphics;

class VRGame : public SDLGame
{
private:
  VRContext vr_context;

  Camera cam[2];
  StaticMesh static_mesh;
  Material mat;
  Shader shader;
  float rot_angle;

  void init_sdl()
  {
    sdl_init_verbose();

    vr_context.init();
    vr_context.init_render_models();

    set_sdl_attributes();
    init_sdl_gl_context();

    vr_context.init_gl();

    game_controller_context.init();
  }

  void user_init()
  {
    vr_context.bind(this);
    for (int eye = 0; eye < 2; eye++)
    {
      cam[eye].set_window_dimensions(Float2((float)resolution[0], (float)resolution[1]));
    }

    static_mesh_init();

    //shader.set_shader_filenames("../../mundus/data/shaders/clay.vs", "../../mundus/data/shaders/clay.fs");
    //shader.load_link_and_compile();

    //mat.set_shader(&shader);
    //mat.init();
    rot_angle = 0.0f;
  }

  void static_mesh_init()
  {
    //mesh init
    FILE *f;
    f = fopen("../graphics/data/meshes/test_mesh.brick.bin", "rb");
    assert(f);

    static_mesh.read_from_file(f, true);
    static_mesh.init();

    fclose(f);
  }

  void user_run() {}
  void user_process_event(const SDL_Event &event) {}

  void render_static_mesh()
  {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_3D);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    GLfloat diff[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat amb[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat light_pos[] = { 0.0f, 2.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    int dev_id[2] = { -1, -1 };
    dev_id[0] = vr_context.find_next_device_of_type(0, 'C');
    if (dev_id[0] >= 0)
    {
      dev_id[1] = vr_context.find_next_device_of_type(dev_id[0] + 1, 'C');
    }

    for (uint32_t i = 0; i < 2; i++)
    {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      if (dev_id[i] >= 0)
      {
        Matrix4x4 c_mat = vr_context.get_device_pose(dev_id[i]);

        GLfloat controller_mat[16] = { c_mat(0, 0), c_mat(0, 1), c_mat(0, 2), c_mat(0, 3),
                                       c_mat(1, 0), c_mat(1, 1), c_mat(1, 2), c_mat(1, 3),
                                       c_mat(2, 0), c_mat(2, 1), c_mat(2, 2), c_mat(2, 3),
                                       c_mat(3, 0), c_mat(3, 1), c_mat(3, 2), c_mat(3, 3) };

        glMultMatrixf(controller_mat);
        glScalef(0.1f, 0.1f, 0.1f);

        static_mesh.render();
      }
      glPopMatrix();
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
  }

  void render_gl()
  {
    double game_time = get_game_time();

    //render once for each eye
    vr_context.retrieve_eye_poses();
    
    float r = PerlinNoise::scaled_octave_noise_2d(2.0f, 1.0f, 0.001f, 0.0f, 1.0f, (float)game_time, 0.0f);
    float g = PerlinNoise::scaled_octave_noise_2d(2.0f, 1.0f, 0.001f, 0.0f, 1.0f, (float)game_time, 1.2345f);
    float b = PerlinNoise::scaled_octave_noise_2d(2.0f, 1.0f, 0.001f, 0.0f, 1.0f, (float)game_time, 71.453f);

    for (int eye = 0; eye < 2; eye++)
    {
      vr_context.render_capture(eye);
      vr_context.get_eye_camera(eye, &cam[eye]);
      
      cam[eye].render_setup();

      glEnable(GL_DEPTH_TEST);

      glClearColor(0.25f, 0.3f, 0.4f, 100.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      render_static_mesh();

      cam[eye].render_cleanup();
      vr_context.render_release(eye);
    }
    vr_context.finalize_render();
  }

  void game_loop(const double game_time, const double frame_time)
  {
    //if(!paused)
    {
      rot_angle += 4.0f * (float)frame_time;
    }
    vr_context.simulate(game_time, frame_time);
  }
public:
  VRGame() : SDLGame(640, 480,
                     "VR Test",
                     SDL_GAME_GENERATE_PAUSE_MENU | SDL_GAME_LOCK_SIM_DT),
                     //SDL_GL_CONTEXT_PROFILE_CORE, 4, 1), 
    vr_context(&game_context)
  {
    diy_window_swap = true;
  }

  ~VRGame()
  {
    vr_context.deinit();
  }
};

int main(int argc, char **argv)
{
  VRGame game;
  game.enable_vsync(false);
  game.init();
  game.run();

  return 0;
}
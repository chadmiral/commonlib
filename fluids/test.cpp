#include <iostream>
#include <assert.h>

#include "texture.h"
#include "perlin.h"
#include "sdl_game.h"

#include "fluid2d.h"
#include "fluid2d_inflow.h"
#include "fluid2d_turbulence.h"
#include "fluid2d_turbulence_inflow.h"
#include "fluid2d_angle_snapper.h"
#include "fluid_gpu.h"

using namespace std;
using namespace Graphics;
using namespace Math;

class FluidGame : public SDLGame
{
private:
  bool                      _use_gpu;
  uint32_t                  _render_mode;
  int                       fluid_dim;
  float                     previous_game_time;
  float                     time_scale;
  float                     velocity_scale;
  float                     fluid_add_amount;

  int                       fluid_channel_display;

  Texture2D                 *fluid_tex;
  Fluid2D                   *fluid;
  GPUFluid2D                *fluid_gpu;
  Fluid2DInflow             *inflow;
  Fluid2DTurbulenceField    *turb, *turb2;
  Fluid2DTurbulenceInflow   *turb_in[3];
  Fluid2DTurbulenceInflow   *turb_out[3];
  Fluid2DAngleSnapper       *angle_snapper;
public:
  FluidGame() : SDLGame(512, 512, "Fluid Test"),
    _render_mode(0),
    _use_gpu(true)
  {
    fluid_dim = 128;
    previous_game_time = 0.0f;
    time_scale = 0.1f;
    velocity_scale = 50.0f;
    fluid_add_amount = 100.0f;
    fluid_channel_display = 2;

    fluid_tex = NULL;
    fluid = NULL;
    fluid_gpu = NULL;
    inflow = NULL;
    turb =  NULL;
    angle_snapper = NULL;

    for(int i = 0; i < 3; i++)
    {
      turb_in[i] = NULL;
      turb_out[i] = NULL;
    }
  }

  ~FluidGame()
  {
    delete fluid;
    delete fluid_gpu;
    delete fluid_tex;
    delete inflow;
    delete turb;
    delete turb2;
    for(int i = 0; i < 3; i ++)
    {
      delete turb_in[i];
      delete turb_out[i];
    }
  }
private:

  void render_fullscreen_quad()
  {
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();
  }

  void setup_textured_quad_state()
  {
    glUseProgramObjectARB(0);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 10.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void render_gl()
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setup_textured_quad_state();

    if (_use_gpu)
    {
      Texture2D *gpu_tex = fluid_gpu->get_prev_channel_tex(0);
      glBindTexture(GL_TEXTURE_2D, gpu_tex->get_tex_id());
    }
    else
    {
      glBindTexture(GL_TEXTURE_2D, fluid_tex->get_tex_id());
    }

    render_fullscreen_quad();

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
  }

  void game_loop(const double game_time, const double frame_time)
  {
    float sim_time = frame_time * time_scale;
    //sim_time = 0.005f;

    if(_use_gpu)
    {
      fluid_gpu->simulate(sim_time);
    }
    else
    {
      fluid->simulate(sim_time);
      fill_fluid_texture();
    }
  }

  void init_gpu_fluid()
  {
    fluid_gpu = new GPUFluid2D(fluid_dim, fluid_dim);
    fluid_gpu->init();
  }

  void user_init()
  {
    fluid_tex = new Texture2D(fluid_dim, fluid_dim);
    fluid_tex->set_dim(fluid_dim, fluid_dim);
    fluid_tex->init();

    fluid = new Fluid2D(fluid_dim, fluid_dim);
    fluid->set_diffusion_rate(0.0f);//0.002f);
    fluid->set_viscosity(0.0f);

    init_gpu_fluid();

    inflow = new Fluid2DInflow;
    inflow->set_rate(100.0f);
    //fluid->add_interactor(inflow);

    turb = new Fluid2DTurbulenceField;
    turb->set_scale(12.0f);
    turb->set_octaves(2);
    turb->set_speed(0.1f);
    turb->set_strength(1.0f);
    fluid->add_interactor(turb);

    turb2 = new Fluid2DTurbulenceField;
    turb2->set_scale(6.0f);
    turb2->set_octaves(1);
    turb2->set_speed(0.1f);
    turb2->set_strength(0.1f);
    fluid->add_interactor(turb2);

    for(int i = 0; i < 3; i++)
    {
      Float2 in_phase(random(0.0f, 100.0f), random(0.0f, 100.0f));
      Float2 out_phase(random(0.0f, 100.0f), random(0.0f, 100.0f));

      turb_in[i] = new Fluid2DTurbulenceInflow;
      turb_in[i]->set_scale(3.0f);
      turb_in[i]->set_octaves(3);
      turb_in[i]->set_speed(-2.0f);
      turb_in[i]->set_strength(5.5f);
      turb_in[i]->set_phase(in_phase);
      turb_in[i]->set_channel((FluidChannelType)(FLUID_CHANNEL_DENS_R + i));
      fluid->add_interactor(turb_in[i]);

      turb_out[i] = new Fluid2DTurbulenceInflow;
      turb_out[i]->set_scale(6.0f);
      turb_out[i]->set_octaves(3);
      turb_out[i]->set_speed(0.5f);
      turb_out[i]->set_strength(-5.5f);
      turb_out[i]->set_phase(out_phase);
      turb_out[i]->set_channel((FluidChannelType)(FLUID_CHANNEL_DENS_R + i));
      fluid->add_interactor(turb_out[i]);
    }

    angle_snapper = new Fluid2DAngleSnapper(3);
    angle_snapper->set_strength(1.0f);

    game_context.console.register_variable(fluid->get_viscosity_ptr(), "viscosity");
    game_context.console.register_variable(fluid->get_diffusion_rate_ptr(), "diffusion_rate");
    game_context.console.register_variable(&time_scale, "time_scale");
  }

  void user_run()
  {
  }

  void user_process_event(const SDL_Event &event)
  {
    int mouse_x, mouse_y, num_keys;
    Uint32 button_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    const Uint8 *keyboard_state = SDL_GetKeyboardState(&num_keys);
    if(button_state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
      float fluid_amt =  fluid_add_amount;
      if(keyboard_state[SDL_SCANCODE_LCTRL])
      {
        fluid_amt = -fluid_amt;
      }
      int w, h;
      fluid_tex->get_dim(w, h);
      Float2 click_pt((float)mouse_x / game_context.window_resolution[0], (float)mouse_y / game_context.window_resolution[1]);
      fluid->add_density_at_point(click_pt, Float3(fluid_amt, fluid_amt, fluid_amt), 0.025f);
    }

    switch(event.type)
    {
      case SDL_MOUSEWHEEL:
        //zoom += (float)event.wheel.y * 0.08f;
        break;
      case SDL_KEYUP:
        switch(event.key.keysym.sym)
        {
          case SDL_MOUSEMOTION:
          {
            if(event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
              //cout<<"button 1 pressed and mouse moving"<<endl;
              Float2 pt((float)event.motion.x / game_context.window_resolution[0], (float)event.motion.y / game_context.window_resolution[1]);
              Float2 vel(velocity_scale * (float)event.motion.xrel, velocity_scale * (float)event.motion.yrel);
              fluid->add_velocity_at_point(pt, vel, 0.03f);
            }
            break;
          }
          case 'x':
            _render_mode = (_render_mode + 1) % 2;
            break;
          case 'g':
            _use_gpu = !_use_gpu;
            break;
        }
    }
  }

  void fill_fluid_texture()
  {
    const FluidChannels *fc = fluid->get_channels();

    int w, h;
    fluid_tex->get_dim(w, h);

    Float3 color_a(80.0f, 55.0f, 35.0f);
    Float3 color_b(255.0f, 255.0f, 255.0f);

    GLuint mode = fluid_tex->get_tex_format();
    int num_bytes = (mode == GL_RGBA) ? 4 : 3;

    GLubyte *pixels = new GLubyte[w * h * num_bytes];
    for(int i = 0; i < w; i++)
    {
      for (int j = 0; j < h; j++)
      {
        int fluid_idx = i + (w + 2) * j;
        float r(0.0f), g(0.0f), b(0.0f);
        if (_render_mode == 0)
        {
          r = g = b = 255.0f * 0.005f * clamp(fc[fluid_idx].data[FLUID_CHANNEL_DENS_R], 0.0f, 100.0f);
        }
        else
        {
          r = 255.0f * (0.125f * clamp(fc[fluid_idx].data[FLUID_CHANNEL_VEL_X], -4.0f, 4.0f) + 0.5f);
          g = 255.0f * (0.125f * clamp(fc[fluid_idx].data[FLUID_CHANNEL_VEL_Y], -4.0f, 4.0f) + 0.5f);
        }
        //float r = 255.0f * clamp(fc[fluid_idx].data[FLUID_CHANNEL_DENS_R], 0.0f, 1.0f);
        //float g = 255.0f * clamp(fc[fluid_idx].data[FLUID_CHANNEL_DENS_G], 0.0f, 1.0f);
        
        Float3 final_color(r, g, b);

        for(int oct = 0; oct < 3; oct++)
        {
          pixels[((i * h + j) * num_bytes) + oct] = (GLubyte)final_color[oct];//(val * 255.0f);
        }
        if(num_bytes == 4)
        {
          pixels[((i * h + j) * 4) + 3] = (GLubyte)255.0f;//(GLubyte)(val * 255.0f);
        }
      }
    }

    fluid_tex->update_pixels_from_mem(pixels);
    delete pixels;
  }
};


/*
void process_events()
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if(keystate[SDLK_ESCAPE]) quit_app();

    int mouse_x, mouse_y, num_keys;
    Uint32 button_state = SDL_GetMouseState(&mouse_x, &mouse_y);
    const Uint8 *keyboard_state = SDL_GetKeyboardState(&num_keys);
    if(button_state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
      float fluid_amt =  Fluid_add_amount;
      if(keyboard_state[SDL_SCANCODE_LCTRL])
      {
        fluid_amt = -fluid_amt;
      }
      int w, h;
      fluid_tex->get_dim(w, h);
      Float2 click_pt((float)mouse_x / WIN_WIDTH, (float)mouse_y / WIN_HEIGHT);
      //fluid->add_density_at_point(click_pt, fluid_amt, 0.025f);
    }

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_KEYDOWN:
        {
          if(keyboard_state[SDL_SCANCODE_D])
          {
            float diff_rate = fluid->get_diffusion_rate();
            if(keyboard_state[SDL_SCANCODE_UP])
            {
              diff_rate *= 2.0f;
            } else if(keyboard_state[SDL_SCANCODE_DOWN])
            {
              diff_rate *= 0.5f;
            }
            fluid->set_diffusion_rate(diff_rate);
            cout<<"diffusion rate: "<<diff_rate<<endl;
          }
          if(keyboard_state[SDL_SCANCODE_V])
          {
            float viscosity = fluid->get_viscosity();
            if(keyboard_state[SDL_SCANCODE_UP])
            {
              viscosity *= 2.0f;
            }
            else if(keyboard_state[SDL_SCANCODE_DOWN])
            {
              viscosity *= 0.5f;
            }
            fluid->set_viscosity(viscosity);
            cout<<"viscosity: "<<viscosity<<endl;
          }
          if(keyboard_state[SDL_SCANCODE_T])
          {
              if(keyboard_state[SDL_SCANCODE_UP])
              {
                Time_scale *= 2.0f;
              } else if(keyboard_state[SDL_SCANCODE_DOWN])
              {
                Time_scale *= 0.5f;
              }
              cout<<"time scale: "<<Time_scale<<endl;
          }
          if(keyboard_state[SDL_SCANCODE_P])
          {
            int project_steps = fluid->get_project_steps();
            if(keyboard_state[SDL_SCANCODE_UP])
            {
              project_steps++;
            }
            else if(keyboard_state[SDL_SCANCODE_DOWN])
            {
              project_steps--;
            }
            fluid->set_project_steps(project_steps);
            cout<<"project steps: "<<project_steps<<endl;
          }
          break;
        }
        case SDL_MOUSEMOTION:
        {
          if(event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
          {
            //cout<<"button 1 pressed and mouse moving"<<endl;
            Float2 pt((float)event.motion.x / WIN_WIDTH, (float)event.motion.y / WIN_HEIGHT);
            Float2 vel(Velocity_scale * (float)event.motion.xrel, Velocity_scale * (float)event.motion.yrel);
            fluid->add_velocity_at_point(pt, vel, 0.03f);
          }
          break;
        }
        case SDL_QUIT:
          quit_app();
          break;
      }
    }
}*/



int main(int argc, char **argv)
{
  bool record_video = false;
  if(argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      if(!strcmp(argv[i], "-v"))
      {
        record_video = true;
      }
    }
  }
  FluidGame app;
  app.init();

  if(record_video) { app.begin_video_capture(); }

  app.run();

	return 0;
}

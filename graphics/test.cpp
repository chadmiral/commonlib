#include <time.h>
#include "sdl_game.h"
#include "camera.h"
#include "gpu_hair_sim.h"
#include "perlin.h"
#include "gpu_particle_system.h"
#include "static_mesh.h"
#include "ribbon.h"
#include "gpu_voronoi.h"

using namespace Graphics;
using namespace PerlinNoise;
using namespace std;
using namespace Math;

const char passthrough_vs[] =
"\
#version 120\
\
void main(void) {\
  gl_TexCoord[0] = gl_MultiTexCoord0;\
  gl_Position = ftransform();\
}\
";

enum RenderMode
{
  RENDER_HAIR,
  RENDER_HAIR_TEXTURE,
  RENDER_FORCE_TEXTURE,
  RENDER_UV_TEXTURE,

  RENDER_PARTICLES,
  RENDER_PARTICLE_POSITION_TEXTURE,
  RENDER_PARTICLE_VELOCITY_TEXTURE,
  RENDER_PARTICLE_DATA_TEXTURE,

  RENDER_STATIC_MESH,

  RENDER_TEXTURE_3D,

  RENDER_VORONOI_SPHERE,
  RENDER_VORONOI_SPHERE_TEXTURE,

  NUM_RENDER_MODES
};

class GraphicsApp : public SDLGame
{
public:
  GraphicsApp() : SDLGame(512, 512, "Graphics Test", SDL_GAME_GENERATE_PAUSE_MENU)
  {
    rot_angle = 0.0f;
    color_tex = NULL;
    tex_3d = NULL;
    render_mode = RENDER_STATIC_MESH;
    force_tex_dim[0] = 64;
    force_tex_dim[1] = 64;

    zoom = 0.0f;

    paused = false;
  }
  ~GraphicsApp()
  {
    delete color_tex;
    delete tex_3d;
  }
private:
  void render_hair()
  {
    glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
      glLoadIdentity();
      //glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
      glRotatef(rot_angle, 0.0f, 1.0f, 0.0f);
      //glTranslatef(0.0f, -0.3f, 0.0f);
      glScalef(1.2f, 1.2f, 1.2f);

      //cam.render_setup();
      gpu_hair.render();
      //cam.render_cleanup();

    //glPopMatrix();
  }

  void render_particles()
  {
    glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
      glLoadIdentity();
      //glRotatef(rot_angle, 0.0f, 1.0f, 0.0f);
      //glTranslatef(0.f, 0.f, cos(rot_angle));
      //glScalef(0.5, 0.5, 0.5);

      gpu_particle_sim.render();

   // glPopMatrix();
  }

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

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    cam.set_fov(65.0f);
    cam.set_pos(Float3(0.0f, 0.0f, -1.0f + zoom));
    cam.set_lookat(Float3(0.0f, 0.0f, 1.0f));
    cam.set_up(Float3(0.0f, 1.0f, 0.0f));

    cam.render_setup();
      glRotatef(rot_angle, 0.0f, 1.0f, 0.0f);
      glRotatef(rot_angle * 0.37f, 0.0f, 0.0f, 1.0f);

      static_mesh.render();
    cam.render_cleanup();

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
  }

  void render_fullscreen_quad_3d()
  {
    float r = (float)get_game_time() * 0.001f;
    glBegin(GL_QUADS);
      glColor3f(1.0f, 1.0f, 1.0f);
      glTexCoord3f(0.0f, 0.0f, r);
      glVertex3f(-1.0f, 1.0f, 0.0f);
      glTexCoord3f(1.0f, 0.0f, r);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glTexCoord3f(1.0f, 1.0f, r);
      glVertex3f(1.0f, -1.0f, 0.0f);
      glTexCoord3f(0.0f, 1.0f, r);
      glVertex3f(-1.0f, -1.0f, 0.0f);
    glEnd();
  }

  void render_texture_3d()
  {
    cout<<"render_texture_3d()"<<endl;
    cout<<"\ttex_id: "<<tex_3d->get_tex_id()<<endl;
    setup_textured_quad_state(true);
    glBindTexture(GL_TEXTURE_3D, tex_3d->get_tex_id());
    render_fullscreen_quad_3d();
    glDisable(GL_TEXTURE_3D);
  }

  void render_fullscreen_quad()
  {
    glBegin(GL_QUADS);
      glColor3f(1.0f, 1.0f, 1.0f);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(-1.0f, 1.0f, 0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(1.0f, 1.0f, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(1.0f, -1.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(-1.0f, -1.0f, 0.0f);
    glEnd();
  }

  void render_voronoi_sphere()
  {
    gpu_voronoi.render_voronoi_texture();
  }

  void render_voronoi_sphere_texture()
  {

  }

  void setup_textured_quad_state(bool is_3d = false)
  {
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);

    if(is_3d)
    {
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_TEXTURE_3D);
    }
    else
    {
      glDisable(GL_TEXTURE_3D);
      glEnable(GL_TEXTURE_2D);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  void render_particle_pos_texture()
  {
    cout<<"render_particle_pos_texture()"<<endl;
    cout<<"\ttex_id: "<<gpu_particle_sim.get_pos_tex(0)<<endl;
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_particle_sim.get_pos_tex(0));
    render_fullscreen_quad();
  }

  void render_particle_vel_texture()
  {
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_particle_sim.get_vel_tex(0));
    render_fullscreen_quad();
  }
  void render_particle_data_texture()
  {
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_particle_sim.get_data_tex());
    render_fullscreen_quad();
  }

  void render_hair_pos_texture()
  {
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_hair.get_pos_tex(1));
    render_fullscreen_quad();
  }

  void render_force_texture()
  {
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_hair.get_force_tex());
    render_fullscreen_quad();
  }

  void render_uv_texture()
  {
    setup_textured_quad_state();
    glBindTexture(GL_TEXTURE_2D, gpu_hair.get_uv_tex());
    render_fullscreen_quad();
  }

  void render_gl()
  {
    glViewport(0, 0, game_context.window_resolution[0], game_context.window_resolution[1]);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthRange(0.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch(render_mode)
    {
      case RENDER_HAIR:
        render_hair();
        break;
      case RENDER_HAIR_TEXTURE:
        render_hair_pos_texture();
        break;
      case RENDER_FORCE_TEXTURE:
        render_force_texture();
        break;
      case RENDER_UV_TEXTURE:
        render_uv_texture();
        break;
      case RENDER_PARTICLES:
        render_particles();
        break;
      case RENDER_PARTICLE_POSITION_TEXTURE:
        render_particle_pos_texture();
        break;
      case RENDER_PARTICLE_VELOCITY_TEXTURE:
        render_particle_vel_texture();
        break;
      case RENDER_PARTICLE_DATA_TEXTURE:
        render_particle_data_texture();
        break;
      case RENDER_STATIC_MESH:
        render_static_mesh();
        break;
      case RENDER_TEXTURE_3D:
        render_texture_3d();
        break;
      case RENDER_VORONOI_SPHERE:
        render_voronoi_sphere();
        break;
      case RENDER_VORONOI_SPHERE_TEXTURE:
        render_voronoi_sphere_texture();
        break;
      default:
        render_hair();
        break;
    }

    glFlush();
  }

  void update_forces(const float game_time, const float frame_time)
  {
    float speed = 0.0003f;
    float scale = 1.2f;

    //TODO: don't reallocate this every frame!
    int num_bytes = 4;
    GLfloat *pixels = new GLfloat[force_tex_dim[0] *
                                  force_tex_dim[1] *
                                  num_bytes];


    int pix_idx = 0;
    for(unsigned int i = 0; i < force_tex_dim[0]; i++)
    {
      for(unsigned int j = 0; j < force_tex_dim[1]; j++)
      {
        float u = (float)i / (float)force_tex_dim[0];
        float v = (float)j / (float)force_tex_dim[1];

        float theta = 2.0f * (float)M_PI * v;
        float phi = ((float)M_PI / 2.0f) + (float)M_PI * (1.0f - u);

        //TODO: LUT for (lat, lon) -> (x, y, z)
        float r = 1.0f;
        float x =  r * cos(theta) * cos(phi);
        float y =  r * sin(phi);
        float z =  r * sin(theta) * cos(phi);

        Float3 p(x, y, z);
        p.normalize();

        Float3 wind;
        wind[0] = scaled_octave_noise_4d(2.0f, 1.0f, scale, -1.0f, 1.0f, p[0] + game_time * speed, p[1], p[2], game_time * speed * 0.3f);
        wind[1] = scaled_octave_noise_4d(2.0f, 1.0f, scale * 0.95f, -1.0f, 1.0f, p[0] + 7.15f + game_time * speed, p[1] + 13.76f, p[2] + 12.74f, game_time * speed * 0.3f);
        wind[2] = scaled_octave_noise_4d(2.0f, 1.0f, scale * 1.2f, -1.0f, 1.0f, p[0] + 3.12f + game_time * speed, p[1] + 67.12f, p[2] - 4.1784f, game_time * speed * 0.3f);

        pixels[pix_idx++] = p[0] + wind[0];// + scaled_octave_noise_2d(3, 1.0f, 2.0f, 0.2f, 1.0f, 0.5f * speed * game_time, p[2]);
        pixels[pix_idx++] = p[1] + wind[1];
        pixels[pix_idx++] = p[2] + wind[2];

        //hair height multiplier
        float hair_height = 1.25f;
        float h = scaled_octave_noise_3d(3, 1.0f, scale * 1.1f, 0.0, hair_height, x + 7.12f, y + 4.12f, game_time * speed * 0.24f);
        h = h * h;
        pixels[pix_idx++] = h;
      }
    }

    gpu_hair.update_forces(pixels);

    delete pixels;
  }

  void game_loop(const double game_time, const double frame_time)
  {
    //cout<<"dt: "<<frame_time<<endl;
    update_forces((float)game_time, (float)frame_time);
    gpu_hair.simulate((float)game_time, (float)frame_time);

    if(!paused)
    {
      gpu_particle_sim.simulate((float)game_time, (float)frame_time);

      rot_angle += 10.0f * (float)frame_time;
    }
  }

  void particle_init()
  {
    gpu_particle_sim.set_shader_directory("./data/shaders/");

    gpu_particle_sim.addCurveVec4("Curves/cosine.curve",
                                  "Curves/g_color_channal.curve",
                                  "Curves/b_color_channal.curve",
                                  "Curves/a_color_channal.curve",
                                  "colors");

    gpu_particle_sim.addCurveVec4("Curves/sine.curve",
                                  "Curves/cosine.curve",
                                  "Curves/always_zero.curve",
                                  "Curves/always_one.curve",
                                  "emitter_dir");

    gpu_particle_sim.addCurve("Curves/always_one.curve", "size");
    gpu_particle_sim.addCurve("Curves/linear.curve", "age");

    ParticleForce **forces = new ParticleForce*[2];

    forces[0] = new Attractor(Float3(0.3f, -1.f, 0.f), 0.03f);
    forces[1] = new Attractor(Float3(-0.05f, 0.2f, 0.f), 0.04f);

    /*
     * addParticleSystem(num_particles, particle_size
     * forces, num_forces,
     * emitter_dir_curve_handle
     * emitter_loc, emitter_radius,
     * emitter_range, emitter_strength, emitter_duration,
     * lifespan,
     * loop,
     * age_curve_handle,
     * color_curve_handle,
     * size_curve_handle
     * sprite_file);
     */
    gpu_particle_sim.addParticleSystem(10000, 0.015f,
                                       forces, 2,
                                       "emitter_dir",
                                       Float3(0.f, 0.f, 0.f), 0.1f,
                                       0.4f, 0.5f, 2.f,
                                       2.f,
                                       true,
                                       "age",
                                       "colors",
                                       "size",
                                       "data/particle.tiff");

    /*forces[0] = new Attractor(Float3(0.f, 0.5f, 0.f), 0.25f);
    forces[1] = new Attractor(Float3(-0.5f, 0.f, 0.f), 0.5f);
    forces[2] = new Attractor(Float3(0.f, 0.f, 0.f), 0.5f);

    gpu_particle_sim.addParticleSystem(10000,
                                      forces, 3,
                                      Float3(0.f, -0.5f, 0.f), 0.1f,
                                      Float3(1.f, 1.f, 0.f), 2.f, 0.5f, 10.f,
                                      3.f,
                                      true,
                                      "data/particle.tiff");

    gpu_particle_sim.addParticleSystem(10000,
                                      forces, 3,
                                      Float3(-0.75f, 0.f, 0.f), 0.1f,
                                      Float3(0.f, 1.f, 0.f), 2.f, 0.5f, 5.f,
                                      3.f,
                                      true,
                                      "data/particle.tiff");*/


    /*
    Float3 cam_pos(0.0f, 0.0f, -5.0f);
    cam.set_pos(cam_pos);
    cam.set_lookat(Float3(0.0f, 0.0f, 0.0f) - cam_pos);
    cam.set_up(Float3(0.0f, 1.0f, 0.0f));
    */
  }

  void hair_init()
  {
    int num_hairs = 10000;

    Float3 *hair_pos = new Float3[num_hairs];
    Float3 *hair_uvs = new Float3[num_hairs];
    for(int i = 0; i < num_hairs; i++)
    {
      hair_pos[i] = Float3(random(-1.0f, 1.0f), random(-1.0f, 1.0f), random(-1.0f, 1.0f));
      hair_pos[i].normalize();
      float height_variance = random(0.5f, 1.0f);

      float u = 0.5f + (atan2(hair_pos[i][2], hair_pos[i][0]) / (float)M_PI) * 0.5f;
      float v = asin(hair_pos[i][1]) / (float)(M_PI) + 0.5f;

      hair_uvs[i] = Float3(u, v, height_variance);
    }

    gpu_hair.set_num_hairs(num_hairs);
    gpu_hair.set_num_segments(6);
    gpu_hair.set_force_tex_dim(force_tex_dim[0], force_tex_dim[1]);
    gpu_hair.init(hair_pos, hair_uvs);

    delete hair_pos;
    delete hair_uvs;

    color_tex = new Texture2D("data/grass.jpg");
    color_tex->load();
    gpu_hair.set_color_tex(color_tex);

    tex_3d = new Texture3D("../../mundus/data/textures/lut_basic.tif");
    tex_3d->set_wrap_mode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    tex_3d->load(32);

    Float3 cam_pos(0.0f, 0.5f, -10.0f);
    cam.set_pos(cam_pos);
    cam.set_lookat(Float3(0.0f, 0.0f, 0.0f) - cam_pos);
    cam.set_up(Float3(0.0f, 1.0f, 0.0f));
  }

  void static_mesh_init()
  {
    //mesh init
    FILE *f;
    f = fopen("data/meshes/test_mesh.brick.bin", "rb");
    assert(f);

    static_mesh.read_from_file(f, true);
    static_mesh.init();

    fclose(f);
  }

  void ribbon_init()
  {
    ribbon.init();
  }

  void voronoi_init()
  {
    uint32_t num_voronoi_cells = 1000;
    gpu_voronoi.init();

    for(uint32_t i = 0; i < num_voronoi_cells; i++)
    {
      gpu_voronoi.add_site(Float2(random(0.0f, 1.0f), random(0.0f, 1.0f)));
    }
    gpu_voronoi.build_voronoi_diagram();
  }

  void user_init()
  {
    cam.set_window_dimensions(Float2((float)game_context.window_resolution[0], (float)game_context.window_resolution[1]));
    particle_init();
    hair_init();
    static_mesh_init();
    ribbon_init();
    voronoi_init();
  }

  void user_run() {}
  void user_process_event(const SDL_Event &event)
  {
    switch(event.type)
    {
      case SDL_MOUSEWHEEL:
        zoom += (float)event.wheel.y * 0.08f;
        break;
      case SDL_KEYUP:
        switch(event.key.keysym.sym)
        {
          case ' ':
            paused = !paused;
            break;
          case '1':
            render_mode = RENDER_HAIR;
            break;
          case '2':
            render_mode = RENDER_HAIR_TEXTURE;
            break;
          case '3':
            render_mode = RENDER_FORCE_TEXTURE;
            break;
          case '4':
            render_mode = RENDER_UV_TEXTURE;
            break;
          case '5':
            render_mode = RENDER_PARTICLES;
            break;
          case '6':
            render_mode = RENDER_PARTICLE_POSITION_TEXTURE;
            break;
          case '7':
            render_mode = RENDER_PARTICLE_VELOCITY_TEXTURE;
            break;
          case '8':
            render_mode = RENDER_PARTICLE_DATA_TEXTURE;
            break;
          case '9':
            render_mode = RENDER_STATIC_MESH;
            break;
          case '0':
            render_mode = RENDER_TEXTURE_3D;
            break;
          case 'q':
            render_mode = RENDER_VORONOI_SPHERE;
            break;
          case 'r':
            render_mode = RENDER_VORONOI_SPHERE_TEXTURE;
            break;
        }
        break;
    }
  }

  GPUHairSim           gpu_hair;
  GPUParticleSim       gpu_particle_sim;
  Math::GPUVoronoi2D   gpu_voronoi;

  Ribbon ribbon;

  RenderMode render_mode;
  Camera cam;
  float rot_angle;
  StaticMesh static_mesh;

  float zoom;

  bool paused;

  Texture2D *color_tex;
  Texture3D *tex_3d;

  unsigned int force_tex_dim[2];
};

int main(int argc, char **argv)
{
  srand((unsigned int)time(NULL));

  GraphicsApp app;
  app.init();
  app.run();

  return 0;
}

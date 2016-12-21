#ifndef __GPU_PARTICLE_SYSTEM_H__
#define __GPU_PARTICLE_SYSTEM_H__

#include "platform_gl.h"

#include <vector>

#include "material.h"
#include "texture.h"
#include "render_surface.h"
#include "texture.h"
#include "curve.h"
#include "quaternion.h"

#define MAX_NUM_ATTRACTORS 5
#define MAX_AGE 100.f
#define DATA_TEXTURE_LENGTH 10000

namespace Graphics
{
  enum ParticleForceType
  {
    ATTRACTOR,
    EMPTY
  };

  class ParticleForce
  {
  public:
    ParticleForce(int fType) : forceType(fType) {}
    int getForceType() { return forceType; }

  private:
    int forceType;
  };

  class Attractor : public ParticleForce
  {
  public:
    Attractor(Math::Float3 loc, float s) : ParticleForce(ATTRACTOR), location(loc), strength(s) {}
    virtual Math::Float3 getLocation() { return location; }
    virtual float getStrength() { return strength; }

  private:
    Math::Float3 location;
    float strength;
  };
  
  
  struct Particle3D
  {
    Math::Float3       position;
    Math::Float3       velocity;
    Math::Float3       scale;
    
    Math::Float4       rgba;
    
    Math::Quaternion   rotation;
    //Math::Float3 rotation_axis;
    //float rotation_amount
    
    float              age;
    
    uint32_t           user_data_a;
  };
  
  //base class
  class ParticleSystem3D
  {
  protected:
    std::vector<Particle3D> particles;
  public:
    ParticleSystem3D();
    ~ParticleSystem3D();
    
    void simulate(const double game_time, const double frame_time);
  };
  
  
  class ParticleSimulator
  {
  private:
    std::vector<ParticleSystem3D *> particle_systems;
    //std::vector<Physics::Force *> forces; //interactors
  public:
    ParticleSimulator() {}
    ~ParticleSimulator() {}
    
    void init() {}
    void simulate(const double game_time, const float frame_time) {}
    void render(const double game_time) {}
    
  };
  
  
  struct GPUParticleSystemTemplate
  {
    int               num_particles;
    float             particle_size;
    ParticleForce     **forces;
    int               num_forces;
    const char        *emitter_dir_handle;
    Math::Float3      emitter_loc;
    float             emitterRadius;
    float             emitter_range;
    float             emitter_strength;
    float             emitter_duration;
    float             lifespan;
    bool              loop;
    char              *age_handle;
    char              *color_handle;
    char              *size_handle;
    char              *tex_file;
  };

  class GPUParticleSystem
  {

  public:
    GPUParticleSystem(const char *shader_directory);
    ~GPUParticleSystem();

    void init(float particle_size, Math::Float3 *initial_particle_pos, Math::Float3 *initial_particle_vel, float *age, float *data);
    void deinit();
    void simulate(const double game_time, const double dt);
    void render();

    bool should_kill() {return is_dead;}

    void set_render_shader_names(std::string vs, std::string fs)
    {
      update_pos_shader_names[0] = vs;
      update_pos_shader_names[1] = fs;
    }
    void set_update_pos_shader_names(std::string vs, std::string fs)
    {
      update_vel_shader_names[0] = vs;
      update_vel_shader_names[1] = fs;
    }
    void set_update_vel_shader_names(std::string vs, std::string fs)
    {
      render_shader_names[0] = vs;
      render_shader_names[1] = fs;
    }

    GLuint get_pos_tex(const int i) { return pos_tex[i]; }
    GLuint get_vel_tex(const int i) { return vel_tex[i]; }
    GLuint get_data_tex() { return data_tex; }

    void add_force(ParticleForce *f)
    {
      if(f->getForceType() == ATTRACTOR && numAttractors < MAX_NUM_ATTRACTORS)
      {
        numAttractors++;
        forces.push_back(f);
      }
    }

    void set_curve_values(int e_dir_id, int a_id, int color_id, int size_id, int num_curves)
    {
      data_tex_height = num_curves + 1;
      color_curve_id = color_id + 1;
      size_curve_id = size_id + 1;
      emitter_dir_id = e_dir_id + 1;
      age_curve_id = a_id + 1;
    }

    void set_system_values(float eRadius,
                           float eStrength,
                           float eRange,
                           float eDur,
                           Math::Float3 eLoc,
                           int numP,
                           float life,
                           float dur,
                           bool loop,
                           const char *sprite_tex_file)
    {
      emitter_radius = eRadius;
      emitter_strength = eStrength;
      emitter_range = eRange;
      emitterLocation = eLoc;
      num_particles = numP;
      particleLifespan = life;
      system_duration = dur;
      emitter_duration = eDur;
      does_loop = loop;
      sprite = new Texture2D(sprite_tex_file);
      sprite->load();
    }

    void update_emitter_location(const Math::Float3 &new_pos) { emitterLocation = new_pos; }

  private:

    enum ParticleSystemUniforms
    {
      UNIFORM_UPDATEPOS_EMITTER_LOC,
      UNIFORM_UPDATEPOS_POS_TEX,
      UNIFORM_UPDATEPOS_VEL_TEX,
      UNIFORM_UPDATEPOS_DATA_TEX,
      UNIFORM_UPDATEPOS_CONSTANTS,

      UNIFORM_UPDATEVEL_ATTRACTORS,
      UNIFORM_UPDATEVEL_POS_TEX,
      UNIFORM_UPDATEVEL_VEL_TEX,
      UNIFORM_UPDATEVEL_DATA_TEX,
      UNIFORM_UPDATEVEL_CONSTANTS,

      UNIFORM_RENDER_POS_TEX,
      UNIFORM_RENDER_SPRITE,
      UNIFORM_RENDER_DATA_TEX,
      UNIFORM_RENDER_CONSTANTS,

      NUM_PARTICLE_UNIFORMS
    };

    struct ParticleVert
    {
      float x, y, z;
      float u, v;
      float sprite_u, sprite_v;
    };

    struct FBOParticleVert
    {
      float x, y, z;
      float u, v;
    };

    std::vector<ParticleForce *> forces;

    int         numAttractors;
    float       billboard_size;
    bool        does_loop;
    bool        is_dead;
    double      start_time;
    Math::Float3      emitterLocation;

    float emitter_range;
    float emitter_strength;
    float emitter_radius;
    float emitter_duration;

    int color_curve_id;
    int size_curve_id;
    int emitter_dir_id;
    int age_curve_id;
    int data_tex_height;

    float particleLifespan;

    float system_duration;

    void update_velocities(const float game_time, const float dt);
    void update_positions(const float game_time, const float dt);

    int num_particles;

    Material render_mat;
    Shader   *render_shader;

    Material update_pos_mat;
    Shader   *update_pos_shader;

    Material update_vel_mat;
    Shader   *update_vel_shader;

    GLuint pos_fbo[2];
    GLuint pos_tex[2];

    GLuint vel_fbo[2];
    GLuint vel_tex[2];

    GLuint data_tex;

    Texture2D *sprite;

    GLuint vbo;
    GLuint ibo;

    ParticleVert *verts;
    unsigned int *indices;

    GLuint fbo_vbo;
    GLuint fbo_ibo;

    GLuint internal_format;

    std::string update_pos_shader_names[2];
    std::string update_vel_shader_names[2];
    std::string render_shader_names[2];

    GLint uniform_locations[NUM_PARTICLE_UNIFORMS];
  };

  class GPUParticleSim
  {
  public:

    GPUParticleSim();
    ~GPUParticleSim();

    void set_shader_directory(const char * shader_dir) { shader_directory = shader_dir; }

    void addCurve(const char *fileName, const char *handle);
    void addCurveVec4(const char *file_name_r,
                      const char *file_name_g,
                      const char *file_name_b,
                      const char *file_name_a,
                      const char *handle);

    void addParticleSystem(int numParticles,
                           float particle_size,
                           ParticleForce **forces,
                           int numForces,
                           const char *emitter_dir_handle,
                           Math::Float3 emitterLoc,
                           float emitterRadius,
                           float emitterRange,
                           float emitterStrength,
                           float emitterDuration,
                           float lifespan,
                           bool loop,
                           const char *age_handle,
                           const char *color_handle,
                           const char *size_handle,
                           const char *tex_file);

    void simulate(const double game_time, const double dt);

    void render();

    GLuint get_pos_tex(const int i) { return particle_systems[i]->get_pos_tex(1); }
    GLuint get_vel_tex(const int i) { return particle_systems[i]->get_vel_tex(1); }
    GLuint get_data_tex() { return particle_systems[0]->get_data_tex(); }

    GPUParticleSystem *get_particle_system_by_index(const unsigned int idx) { return particle_systems[idx]; }

  private:

    std::vector<GPUParticleSystem *>    particle_systems;
    std::vector<float>                  rand_data;
    std::vector<const char *>           curve_handles;

    const char *shader_directory;
  };
};

#endif //__GPU_PARTICLE_SYSTEM_H__

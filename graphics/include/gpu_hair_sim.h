#ifndef __GPU_HAIR_SIM_H__
#define __GPU_HAIR_SIM_H__

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "material.h"
#include "texture.h"
#include "render_surface.h"

namespace Graphics {

  enum HairShaderUniforms
  {
    UNIFORM_SIM_CONSTANTS,
    UNIFORM_SIM_POS_TEX,
    UNIFORM_SIM_FORCE_TEX,
    UNIFORM_SIM_UV_TEX,

    UNIFORM_RENDER_HAIR_TEX,
    UNIFORM_RENDER_UV_TEX,
    UNIFORM_RENDER_COLOR_TEX,
    UNIFORM_RENDER_SUN_POS,
    UNIFORM_RENDER_DIFF,
    UNIFORM_RENDER_SPEC,
    UNIFORM_RENDER_AMBIENT,
    UNIFORM_RENDER_CAM_DISTANCE,

    NUM_HAIR_UNIFORMS
  };

  struct HairVert
  {
    float x, y, z;
    float r, g, b;
    float u0, v0;       //index into pos tex
  };

  struct FBOVert
  {
    float x, y, z;
    float u, v;
  };

  class GPUHairSim
  {
  public:
    GPUHairSim();
    ~GPUHairSim();

    void init(Float3 *hair_pos, Float3 *hair_uvs);
    void deinit();
    void simulate(const float game_time, const float dt);
    void render();

    //only allowed to call these *before* init!!!
    void set_num_hairs(const int h) { num_hairs = h; }
    void set_num_segments(const int s) { num_segments = s; }
    void set_color_tex(Texture *t) { color_tex = t; }
    void set_force_tex_dim(const int w, const int h) { force_tex_dim[0] = w; force_tex_dim[1] = h; }

    int get_num_hairs() const { return num_hairs; }
    int get_num_segments() const { return num_segments; }
    GLuint get_pos_tex(const int i) { return pos_tex[i]; }
    GLuint get_force_tex() { return force_tex; }
    GLuint get_uv_tex() { return uv_tex; }

    void update_forces(const GLfloat *force_data);
    void update_lighting(Float3 sun_pos, Float3 sun_diff, Float3 sun_spec, Float3 ambient, float cam_dist);

    void set_render_shader_names(std::string vs, std::string fs);
    void set_simulation_shader_names(std::string vs, std::string fs);
  private:
    GLuint create_texture();

    int           num_hairs;
    int           num_segments;
    float         hair_height;

    //texture names where we store hair data (prev & curr)
    GLuint        pos_fbo[2];
    GLuint        pos_tex[2];
    GLuint        uv_tex;

    GLuint        force_tex;
    int           force_tex_dim[2];

    Texture       *color_tex;

    Material      render_mat;
    Material      sim_mat;

    Float3        sun_pos_xyz;
    Float3        sun_diff_rgb;
    Float3        sun_spec_rgb;
    Float3        ambient_rgb;
    float         cam_distance;

    //vertex and index buffers for the geo
    GLuint        vbo;
    GLuint        ibo;
    HairVert      *verts;
    int           num_verts;
    unsigned int  *indices;
    int           num_indices;

    //vertex and index buffers for the FBOs
    GLuint        fbo_vbo;
    GLuint        fbo_ibo;
    FBOVert       fbo_verts[4];
    int           num_fbo_verts;
    unsigned int  fbo_indices[4];
    int           num_fbo_indices;

    GLuint        internal_format;

    float         k;

    std::string simulation_shader_names[2];
    std::string render_shader_names[2];
    GLuint uniform_locations[NUM_HAIR_UNIFORMS];
  };
};

#endif //__GPU_HAIR_SIM_H__

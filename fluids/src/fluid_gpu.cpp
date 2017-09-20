#include "fluid_gpu.h"
#include "perlin.h"

using namespace std;
using namespace Graphics;

GPUFluid2D::GPUFluid2D() :
  _num_projection_steps(32),
  _density_allowable_range(0.0f, 1.0f),
  _diffusion_rate(0.0001f),
  _viscosity(0.001f)
{
  for (uint32_t i = 0; i < NUM_FLUID_COMPUTE_STAGES; i++)
  {
    _m_compute_stage_materials[i] = NULL;
    _s_compute_stage_shaders[i] = NULL;
  }

  //this is sloppy, but it'll do
  _cs_fluid_stage_source[FLUID_COMPUTE_PROJECT] =
#include "../shaders/project.glsl"
    ;
  _cs_fluid_stage_source[FLUID_COMPUTE_ADVECT] =
#include "../shaders/advect.glsl"
    ;
  _cs_fluid_stage_source[FLUID_COMPUTE_DIFFUSE] =
#include "../shaders/diffuse.glsl"
    ;
  _cs_fluid_stage_source[FLUID_COMPUTE_BOUNDARIES] =
#include "../shaders/boundaries.glsl"
    ;
  _cs_fluid_stage_source[FLUID_COMPUTE_ADD_SOURCE] =
#include "../shaders/add_source.glsl"
    ;

#ifndef FLUID_GPU_USE_COMPUTE_SHADER
  _vs_fluid_passthrough_source =
#include "../shaders/vs_pass_through.glsl"
    ;

  _fs_fluid_stage_source[FLUID_COMPUTE_PASSTHROUGH] =
#include "../shaders/fs_pass_through.glsl"
    ;
  _fs_fluid_stage_source[FLUID_COMPUTE_PROJECT] =
#include "../shaders/fs_project.glsl"
    ;
  _fs_fluid_stage_source[FLUID_COMPUTE_ADVECT] =
#include "../shaders/fs_advect.glsl"
    ;
  _fs_fluid_stage_source[FLUID_COMPUTE_DIFFUSE] =
#include "../shaders/fs_diffuse.glsl"
    ;
  _fs_fluid_stage_source[FLUID_COMPUTE_BOUNDARIES] =
#include "../shaders/fs_boundaries.glsl"
    ;
  _fs_fluid_stage_source[FLUID_COMPUTE_ADD_SOURCE] =
#include "../shaders/fs_add_source.glsl"
    ;

#endif //FLUID_GPU_USE_COMPUTE_SHADER

  _dimensions[0] = _dimensions[1] = FLUID_GPU_DEFAULT_DIM;
}

GPUFluid2D::GPUFluid2D(uint32_t w, uint32_t h) : GPUFluid2D()
{
  _dimensions[0] = w;
  _dimensions[1] = h;
}

GPUFluid2D::~GPUFluid2D()
{
  deinit();
}

void GPUFluid2D::init()
{
#if defined (FLUID_GPU_USE_COMPUTE_SHADER)
  Texture2D *t_prev = new Graphics::Texture2D(_dimensions[0], _dimensions[1], GL_FLOAT, GL_RGBA, GL_RGBA);
  Texture2D *t_curr = new Graphics::Texture2D(_dimensions[0], _dimensions[1], GL_FLOAT, GL_RGBA, GL_RGBA);
#else
  Texture2D *t_prev = new Graphics::Texture2D(_dimensions[0], _dimensions[1], GL_FLOAT, GL_RGBA, GL_RGBA);
  t_prev->init();

  Texture2D *t_curr = new Graphics::Texture2D(_dimensions[0], _dimensions[1], GL_FLOAT, GL_RGBA, GL_RGBA);
  t_curr->init();
#endif

  float *pixels = new float[_dimensions[0] * _dimensions[1] * 4];

  //fill textures with perlin noise
  for (uint32_t i = 0; i < _dimensions[0]; i++)
  {
    float x = (float)i / (float)_dimensions[0];
    for (uint32_t j = 0; j < _dimensions[1]; j++)
    {
      int idx = 4 * (i + _dimensions[0] * j);
      float y = (float)j / (float)_dimensions[1];
      float n = PerlinNoise::octave_noise_2d(2.0f, 1.0f, 1.0f, x, y);

      pixels[idx] = 1.0f * n;
      pixels[idx + 1] = 1.0f * n;
      pixels[idx + 2] = 1.0f * n;
      pixels[idx + 3] = 1.0f * n;
    }
  }

  t_prev->update_pixels_from_mem(pixels);
  delete[] pixels;

#if defined(FLUID_GPU_USE_COMPUTE_SHADER)
  _prev_channels.push_back(t_prev);
  _curr_channels.push_back(t_curr);
#else
  _rs_prev_channels.push_back(RenderSurface(_dimensions[0], _dimensions[1]));
  _rs_prev_channels[0].set_tex(t_prev);
  _rs_prev_channels[0].use_depth(false);

  _rs_curr_channels.push_back(RenderSurface(_dimensions[0], _dimensions[1]));
  _rs_curr_channels[0].set_tex(t_prev);
  _rs_curr_channels[0].use_depth(false);
#endif //FLUID_GPU_USE_COMPUTE_SHADER

#if defined(FLUID_GPU_USE_COMPUTE_SHADER)
  for (uint32_t i = 0; i < NUM_FLUID_COMPUTE_STAGES; i++)
  {
    cout << "Loading compute shader \"" << FluidComputeStageNames[i] << "\"" << endl;
    cout << _cs_fluid_stage_source[i].c_str() << endl;
    Shader *cs = _s_compute_stage_shaders[i] = new Shader;
    cs->set_local_size(1, 1, 1);
    cs->compile_and_link_compute_shader(_cs_fluid_stage_source[i].c_str());

    _sui_dest_tex[i].set_name("dest_tex");
    _sui_dest_tex[i].set_loc(cs);
    _sui_dest_tex[i].set_var(0);

    //_rs_prev_channels[0].add_shader(cs, FluidComputeStageNames[i], false);
    //_rs_curr_channels[0].add_shader(cs, FluidComputeStageNames[i], false);
  }
#else

  for (uint32_t i = 0; i < NUM_FLUID_COMPUTE_STAGES; i++)
  {
    Shader *s = _s_compute_stage_shaders[i] = new Shader;
    s->compile_and_link_from_source(_vs_fluid_passthrough_source.c_str(), _fs_fluid_stage_source[i].c_str());

    _rs_prev_channels[0].add_shader(s, FluidComputeStageNames[i], false);
    _rs_curr_channels[0].add_shader(s, FluidComputeStageNames[i], false);
  }

  _rs_prev_channels[0].init();
  _rs_curr_channels[0].init();
#endif //FLUID_GPU_USE_COMPUTE_SHADER
}

void GPUFluid2D::deinit()
{
  for (uint32_t i = 0; i < NUM_FLUID_COMPUTE_STAGES; i++)
  {
    if (_s_compute_stage_shaders[i])
    {
      _s_compute_stage_shaders[i]->deinit();
      delete _s_compute_stage_shaders[i];
    }
  }
}

void GPUFluid2D::add_source(const float dt)
{
#if defined (FLUID_GPU_USE_COMPUTE_SHADER)
  //set shader uniforms, etc...
  glUseProgram(_s_compute_stage_shaders[FLUID_COMPUTE_ADD_SOURCE]->gl_shader_program);

  _sui_dest_tex[FLUID_COMPUTE_ADD_SOURCE].render();

  glActiveTexture(GL_TEXTURE0 + 0);
  //glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, _prev_channels[0]->get_tex_id());
  glBindImageTexture(0, _prev_channels[0]->get_tex_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

  gl_check_error();

  _s_compute_stage_shaders[FLUID_COMPUTE_ADD_SOURCE]->execute(_dimensions[0], _dimensions[1], 1);
#else
  _rs_curr_channels[0].capture();
  _rs_prev_channels[0].render(FLUID_COMPUTE_ADD_SOURCE);
  _rs_curr_channels[0].release();
#endif
}

void GPUFluid2D::diffuse(const float dt)
{
  //_s_compute_stage_shaders[FLUID_COMPUTE_DIFFUSE]->execute();
}

void GPUFluid2D::project(const float dt)
{
  //_s_compute_stage_shaders[FLUID_COMPUTE_PROJECT]->execute();
}

void GPUFluid2D::advect(const float dt)
{
  //_s_compute_stage_shaders[FLUID_COMPUTE_ADVECT]->execute();
}

void GPUFluid2D::velocity_step(const float dt)
{
  add_source(dt);
  
  //SWAP
  diffuse(dt);
  project(dt);
  
  //SWAP
  advect(dt);
  project(dt);

  /*
  add_source(curr_channels, prev_channels, 0, 2, dt);
  //SWAP

  //note - passing 0 here for the first parameter is a little different than
  //before (will have no boundary collision). Before we negated the u or v
  diffuse(0, prev_channels, curr_channels, 0, 2, viscosity, dt);
  project(prev_channels, curr_channels, 0, 1);
  //SWAP
  advect(0, curr_channels, prev_channels, prev_channels, 0, 2, dt);
  project(curr_channels, prev_channels, 0, 1);
  */
}

void GPUFluid2D::density_step(const float dt)
{
  //_s_compute_stage_shaders[FLUID_COMPUTE_ADD_SOURCE]->execute();
  //SWAP
  //_s_compute_stage_shaders[FLUID_COMPUTE_DIFFUSE]->execute();
  //SWAP
  //_s_compute_stage_shaders[FLUID_COMPUTE_ADVECT]->execute();


  /*
  add_source(curr_channels, prev_channels, 2, NUM_FLUID_CHANNELS, dt);
  //SWAP
  diffuse(0, prev_channels, curr_channels, 2, NUM_FLUID_CHANNELS, diffusion_rate, dt);
  //SWAP
  advect(0, curr_channels, prev_channels, curr_channels, 2, NUM_FLUID_CHANNELS, dt);
  */
}

#ifndef FLUID_GPU_USE_COMPUTE_SHADER
void GPUFluid2D::copy_last_frames_textures()
{
  
}
#endif //FLUID_GPU_USE_COMPUTE_SHADER

void GPUFluid2D::simulate(const float dt)
{
#ifndef FLUID_GPU_USE_COMPUTE_SHADER
  copy_last_frames_textures();
#endif
  velocity_step(dt);
  density_step(dt);

  /*
  for (int i = 0; i < interactors.size(); i++)
  {
    Fluid2DInteractor *fi = interactors[i];
    fi->simulate(dt);
  }
  */
}
#include "fluid_gpu.h"

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
  Texture2D *t_prev = new Graphics::Texture2D(_dimensions[0], _dimensions[1], GL_FLOAT, GL_RGBA, GL_RGBA);
  t_prev->init();

  Texture2D *t_curr = new Graphics::Texture2D(_dimensions[0], _dimensions[1], GL_FLOAT, GL_RGBA, GL_RGBA);
  t_curr->init();

  _prev_channels.push_back(t_prev);
  _curr_channels.push_back(t_curr);

  for (uint32_t i = 0; i < NUM_FLUID_COMPUTE_STAGES; i++)
  {
    cout << "Loading compute shader \"" << FluidComputeStageNames[i] << "\"" << endl;
    cout << _cs_fluid_stage_source[i].c_str() << endl;
    Shader *cs = _s_compute_stage_shaders[i] = new Shader;
    cs->compile_and_link_compute_shader(_cs_fluid_stage_source[i].c_str());

    _sui_dest_tex[i].set_name("dest_tex");
    _sui_dest_tex[i].set_loc(cs);
    _sui_dest_tex[i].set_var(0);
  }
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
  //set shader uniforms, etc...
  _prev_channels[0]->render_gl();
  _sui_dest_tex[FLUID_COMPUTE_ADD_SOURCE].render();
  _s_compute_stage_shaders[FLUID_COMPUTE_ADD_SOURCE]->execute();
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

void GPUFluid2D::simulate(const float dt)
{
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
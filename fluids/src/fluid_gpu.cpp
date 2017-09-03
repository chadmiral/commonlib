#include "fluid_gpu.h"

GPUFluid2D::GPUFluid2D() :
  _m_density_step(NULL),
  _m_velocity_step(NULL),
  _m_project(NULL),
  _m_advect(NULL),
  _m_diffuse(NULL),
  _m_boundaries(NULL),
  _num_projection_steps(32),
  _density_allowable_range(0.0f, 1.0f),
  _diffusion_rate(0.0001f),
  _viscosity(0.001f)
{
  _dimensions[0] = _dimensions[1] = FLUID_GPU_DEFAULT_DIM;
}

GPUFluid2D::~GPUFluid2D()
{
  deinit();
}

void GPUFluid2D::init()
{

}

void GPUFluid2D::deinit()
{

}


void GPUFluid2D::velocity_step(const float dt)
{
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
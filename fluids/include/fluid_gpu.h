#pragma once

#include <vector>
#include "texture.h"
#include "material.h"
#include "fluid2d_interactor.h"

#define FLUID_GPU_DEFAULT_DIM 128

class GPUFluid2D
{
private:
  uint32_t                             _dimensions[2];
  std::vector<Graphics::Texture2D *>   _prev_channels;
  std::vector<Graphics::Texture2D *>   _curr_channels;
  std::vector<Fluid2DInteractor *>     _interactors;

  Graphics::Material                   *_m_density_step;    // density step shader
  Graphics::Material                   *_m_velocity_step;   // velocity step shader
  Graphics::Material                   *_m_project;         // projection shader
  Graphics::Material                   *_m_advect;          // advection shader
  Graphics::Material                   *_m_diffuse;         // diffusion shader
  Graphics::Material                   *_m_boundaries;      // set boundary conditions shader
  Graphics::Material                   *_m_add_source;      // add_source shader

  uint32_t                             _num_projection_steps;     //number of projection steps to run each step
  Math::Float2                         _density_allowable_range;	//min/max density allowed in the simulation
  float                                _diffusion_rate;						//rate of diffusion
  float                                _viscosity;								//viscosity of the fluid

public:
  GPUFluid2D();
  ~GPUFluid2D();

  void init();
  void deinit();

  void simulate(const float dt);
private:
  void velocity_step(const float dt);
  void density_step(const float dt);
};
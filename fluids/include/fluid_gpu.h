#pragma once

#include <vector>
#include "texture.h"
#include "material.h"
#include "fluid2d_interactor.h"

#define FLUID_GPU_DEFAULT_DIM 128

class GPUFluid2D
{
private:
  std::vector<Graphics::Texture2D *>   _prev_channels;
  std::vector<Graphics::Texture2D *>   _curr_channels;

  enum FluidComputeStage
  {
    FLUID_COMPUTE_PROJECT,
    FLUID_COMPUTE_ADVECT,
    FLUID_COMPUTE_DIFFUSE,
    FLUID_COMPUTE_BOUNDARIES,
    FLUID_COMPUTE_ADD_SOURCE,

    NUM_FLUID_COMPUTE_STAGES
  };

  std::string                          _cs_fluid_stage_source[NUM_FLUID_COMPUTE_STAGES];
  Graphics::Material                  *_m_compute_stage_materials[NUM_FLUID_COMPUTE_STAGES];
  Graphics::Shader                    *_s_compute_stage_shaders[NUM_FLUID_COMPUTE_STAGES];
  Graphics::ShaderUniformInt           _sui_dest_tex[NUM_FLUID_COMPUTE_STAGES];

  uint32_t                             _dimensions[2];
  uint32_t                             _num_projection_steps;     // number of projection steps to run each step
  Math::Float2                         _density_allowable_range;	// min/max density allowed in the simulation
  float                                _diffusion_rate;						// rate of diffusion
  float                                _viscosity;								// viscosity of the fluid

  //std::vector<Fluid2DInteractor *>     _interactors;              // all the interactors / modifiers associated w/ this fluid

public:
  GPUFluid2D();
  GPUFluid2D(uint32_t w, uint32_t h);
  ~GPUFluid2D();

  void init();
  void deinit();

  void simulate(const float dt);
private:
  void velocity_step(const float dt);
  void density_step(const float dt);

  void add_source(const float dt);
  void diffuse(const float dt);
  void project(const float dt);
  void advect(const float dt);
};
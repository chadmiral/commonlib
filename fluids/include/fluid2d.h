#pragma once

#define FLUID_DIM_DEFAULT   64

#include <vector>
#include "fluid2d_interactor.h"
#include "math_utility.h"
#include "fluid_channels.h"

//#define SWAP(x0, x) { FluidChannels *tmp = x0; x0 = x; x = tmp;}

//TODO
/*
clarify the set_bnd function...
#define FLUID_BOUNDARY_SAME_VERT		(1<<0)		//set the vertical boundaries to the same value as their neighbor
#define FLUID_BOUNDARY_NEG_VERT			(1<<1)		//set the vertical boundaries to the negative value of their neighbor
#define FLUID_BOUNDARY_SAME_HORIZ		(1<<2)		//set the horizontal boundaries to the same value as their neighbor
#define FLUID_BOUNDARY_NEG_HORIZ		(1<<3)		//set the horizontal boundaries to the negative value as their neighbor
*/

class Fluid2D
{
public:
	Fluid2D();
	Fluid2D(const int w, const int h);
	~Fluid2D();

	//const Float3 *get_density_array() { return dens; }
	const FluidChannels *get_channels() { return curr_channels; }

	float get_x_dim() const { return dim[0]; }
	float get_y_dim() const { return dim[1]; }

	float get_viscosity() const { return viscosity; }
	float *get_viscosity_ptr() { return &viscosity; }
	float get_diffusion_rate() const { return diffusion_rate; }
	float *get_diffusion_rate_ptr() { return &diffusion_rate; }
	int   get_project_steps() const { return project_steps; }
	int		*get_project_steps_ptr() { return &project_steps; }

	void set_viscosity(const float visc) { viscosity = visc; }
	void set_diffusion_rate(const float diff) { diffusion_rate = diff; }
	void set_project_steps(const int p) { project_steps = p; }

	void simulate(const float t);

	void add_density_at_point(const Math::Float2 pt, const Math::Float3 density, const float radius);
	void add_velocity_at_point(const Math::Float2 pt, const Math::Float2 vel, const float radius);
	void add_interactor(Fluid2DInteractor *fi);

protected:
	void init_helper();		//helper function for the constructor

	void add_source(FluidChannels *_x, FluidChannels *_s, int a, int b, float dt);

	void density_step(float dt);
	void velocity_step(float dt);
	void project(FluidChannels *c, FluidChannels *c0, int a, int b);
	void advect(int bnd, FluidChannels *d, FluidChannels *d0, FluidChannels *vel, int a, int b, float dt);
	void diffuse(int bnd, FluidChannels *c, FluidChannels *c0, int a, int b, float diff, float dt);

	//helper function access our 1-D fluid array with 2 coordinates
	//inline just as fast as a macro? *shrug*
	inline int idx(const int &i, const int &j) const
	{
		return i + (dim[0] + 2) * j;
	}

	//void set_bnd(int bnd, float *x);
	void set_boundaries(bool vel, bool dens, FluidChannels *fc, int a, int b);

private:
	int N;								//TEMPORARY UNTIL I FIGURE OUT HOW TO HAVE DIFF DIMS
	int dim[2];						//dimensions of the grid (+2 for boundary conditions)

	//channels 0-1: velocity_step
	//channels 2-4: RGB density
	//channel    5: temperature?
	//channel		 6: fuel?
	FluidChannels *curr_channels;
	FluidChannels *prev_channels;

	int project_steps;							      //number of steps to use in the project (and diffuse) functions
  Math::Float2 density_allowable_range;	//min/max density allowed in the simulation
	float diffusion_rate;						      //rate of diffusion
	float viscosity;								      //viscosity of the fluid

	//list of objects that can interact w/ the fluid sim
	std::vector<Fluid2DInteractor *> interactors;
};

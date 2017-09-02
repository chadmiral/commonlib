#include <assert.h>
#include <iostream>
#include <cstring>

#include "fluid2d.h"
#include "perlin.h"

using namespace std;

Fluid2D::Fluid2D()
{
  N = FLUID_DIM_DEFAULT;
  dim[0] = dim[1] = FLUID_DIM_DEFAULT;

  init_helper();
}

Fluid2D::Fluid2D(const int w, const int h)
{
  N = w;
  assert(w == h);
  dim[0] = w;
  dim[1] = h;

  init_helper();
}

//helper function for the constructor
void Fluid2D::init_helper()
{
  density_allowable_range = Float2(-1.0f, 1.0f);
  viscosity = 0.0075f;
  diffusion_rate = 0.001f;
  project_steps = 4;

  //Ellis' values:
  //viscosity = 0.0001f;
  //diffusion_rate = 0.001f;

  int size = (N + 2) * (N + 2);

  curr_channels = new FluidChannels[size];
  prev_channels = new FluidChannels[size];

  float perlin_scale = 0.1f;

  cout<<"initializing fluid of size "<<N<<endl;
  for(int i = 0; i < N; i++)
  {
    for(int j = 0; j < N; j++)
    {
      float r = 0.0f;// (float)i / N;
      float g = 0.0f;// (float)j / N;
      float b = 0.0f;// 0.5f;

      curr_channels[idx(i, j)].data[FLUID_CHANNEL_DENS_R] = r;
      curr_channels[idx(i, j)].data[FLUID_CHANNEL_DENS_G] = g;
      curr_channels[idx(i, j)].data[FLUID_CHANNEL_DENS_B] = b;

      prev_channels[idx(i, j)].data[FLUID_CHANNEL_DENS_R] = r;
      prev_channels[idx(i, j)].data[FLUID_CHANNEL_DENS_G] = g;
      prev_channels[idx(i, j)].data[FLUID_CHANNEL_DENS_B] = b;
    }
  }
}

Fluid2D::~Fluid2D()
{
  /*
  delete u;
  delete u_prev;
  delete v;
  delete v_prev;
  delete dens;
  delete dens_prev;
  */
  delete curr_channels;
  delete prev_channels;
}

void Fluid2D::add_density_at_point(const Float2 pt, const Float3 density, const float radius)
{
  //TODO
  /*Float3 *src = new Float3[(N + 2) * (N + 2)];
  memset(src, 0, (N + 2) * (N + 2) * sizeof(float));
  for(int i = 0; i < dim[0]; i++)
  {
    for(int j = 0; j < dim[1]; j++)
    {
      Float2 p0((float)i / (float)dim[0], (float)j / (float)dim[1]);
      Float2 dir(pt[0] - p0[0], pt[1] - p0[1]);
      float d2 = dir.mag_squared();

      if(d2 < radius * radius)
      {
        src[idx(i, j)] = density * (1.0f - d2 / (radius * radius));
      }
    }
  }

  add_source(dens_prev, src, 1.0f);
  delete src;
  */
}

void Fluid2D::add_velocity_at_point(const Float2 pt, const Float2 vel, const float radius)
{
  /*
  int array_size = (N + 2) * (N + 2);

  float *src_u = new float[array_size];
  float *src_v = new float[array_size];

  memset(src_u, 0, array_size * sizeof(float));
  memset(src_v, 0, array_size * sizeof(float));

  for(int i = 0; i < dim[0]; i++)
  {
    for(int j = 0; j < dim[1]; j++)
    {
      Float2 p0((float)i / (float)dim[0], (float)j / (float)dim[1]);
      Float2 v(pt[0] - p0[0], pt[1] - p0[1]);
      float dist = v.magnitude();

      if(dist < radius)
      {
        src_u[idx(i, j)] = vel[0] * (1.0f - dist / radius);
        src_v[idx(i, j)] = vel[1] * (1.0f - dist / radius);
      }
    }
  }

  add_source(u_prev, src_u, 1.0f);
  add_source(v_prev, src_v, 1.0f);

  delete src_u;
  delete src_v;
  */
}

void Fluid2D::add_interactor(Fluid2DInteractor *fi)
{
  assert(fi);
  assert(prev_channels);
  assert(curr_channels);

  fi->set_fluid_channels(curr_channels, prev_channels);
  fi->set_grid_dimensions(Float2(dim[0], dim[1]));

  interactors.push_back(fi);
}

void Fluid2D::simulate(const float dt)
{


  velocity_step(dt);
  density_step(dt);

  for (int i = 0; i < interactors.size(); i++)
  {
    Fluid2DInteractor *fi = interactors[i];
    fi->simulate(dt);
  }
}

void Fluid2D::add_source(FluidChannels *_x, FluidChannels *_s, int a, int b, float dt)
{
  int size = (N + 2) * (N + 2);
  for(int i = 0; i < size; i++)
  {
    for(int j = a; j < b; j++)
    {
      _x[i].data[j] += dt * _s[i].data[j];
    }
  }
}

void Fluid2D::density_step(float dt)
{
  add_source(curr_channels, prev_channels, 2, NUM_FLUID_CHANNELS, dt);
  //SWAP
  diffuse(0, prev_channels, curr_channels, 2, NUM_FLUID_CHANNELS, diffusion_rate, dt);
  //SWAP
  advect(0, curr_channels, prev_channels, curr_channels, 2, NUM_FLUID_CHANNELS, dt);
}

void Fluid2D::velocity_step(float dt)
{
  add_source(curr_channels, prev_channels, 0, 2, dt);
  //SWAP

  //note - passing 0 here for the first parameter is a little different than
  //before (will have no boundary collision). Before we negated the u or v
  diffuse(0, prev_channels, curr_channels, 0, 2, viscosity, dt);
  project(prev_channels, curr_channels, 0, 1);
  //SWAP
  advect(0, curr_channels, prev_channels, prev_channels, 0, 2, dt);
  project(curr_channels, prev_channels, 0, 1);
}

//
// Hodge decomposition to conserve mass
// solve linear system (Poisson equation)
void Fluid2D::project(FluidChannels *vel, FluidChannels *div_p, int a, int b)
{
  float h = 1.0f / N;

  a = FLUID_CHANNEL_VEL_X;
  b = FLUID_CHANNEL_VEL_Y;

  int pidx = 0;
  int didx = 1;

  int uidx = 0;
  int vidx = 1;

  for(int i = 1; i <= N; i++)
  {
    for(int j = 1; j <= N; j++)
    {
      div_p[idx(i, j)].data[didx] = -0.5f * h * (vel[idx(i + 1, j)].data[uidx] -
                                                 vel[idx(i - 1, j)].data[uidx] +  //vel(x)
                                                 vel[idx(i, j + 1)].data[vidx] -
                                                 vel[idx(i, j - 1)].data[vidx]);   //vel(y)

      div_p[idx(i, j)].data[pidx] = 0.0f;
    }
  }

  set_boundaries(false, true, div_p, 0, 2);

  for(int k = 0; k < project_steps; k++)
  {
    for(int i = 1; i <= N; i++)
    {
      for(int j = 1; j <=N; j++)
      {
        div_p[idx(i, j)].data[pidx] = (div_p[idx(i, j)].data[didx] +
                                       div_p[idx(i - 1, j)].data[pidx] +
                                       div_p[idx(i + 1, j)].data[pidx] +
                                       div_p[idx(i, j - 1)].data[pidx] +
                                       div_p[idx(i, j + 1)].data[pidx]) / 4.0f;
      }
    }
    set_boundaries(false, true, div_p, 0, 2);
  }

  for(int i = 1; i <= N; i++)
  {
    for(int j = 1; j <= N; j++)
    {
      vel[idx(i, j)].data[uidx] -= 0.5f * (div_p[idx(i + 1, j)].data[pidx] -
                                           div_p[idx(i - 1, j)].data[pidx]) / h;
      vel[idx(i, j)].data[vidx] -= 0.5f * (div_p[idx(i, j + 1)].data[pidx] -
                                           div_p[idx(i, j - 1)].data[pidx]) / h;
    }
  }

  set_boundaries(true, false, vel, 0, 2);
}

//Gauss-Seidel relaxation (Jacobi another option?)
void Fluid2D::diffuse(int bnd, FluidChannels *c, FluidChannels *c0, int a, int b, float diff, float dt)
{
  float alpha = dt * diff * N * N;
  int diffuse_steps = project_steps; //TODO separate diffuse_steps
  for(int k = 0; k < diffuse_steps; k++)
  {
    for(int i = 1; i <= N; i++)
    {
      for(int j = 1; j <= N; j++)
      {
        for(int ch = a; ch < b; ch++)
        {
          c[idx(i, j)].data[ch] = (c0[idx(i, j)].data[ch] +
                                    alpha * (c[idx(i - 1, j)].data[ch] +
                                             c[idx(i + 1, j)].data[ch] +
                                             c[idx(i, j - 1)].data[ch] +
                                             c[idx(i, j + 1)].data[ch])) / (1.0f + 4.0f * alpha);
        }
      }
    }
    bool is_vel = b <= 2;
    set_boundaries(is_vel, !is_vel, c, a, b);
  }
}

void Fluid2D::advect(int bnd, FluidChannels *d, FluidChannels *d0, FluidChannels *vel, int a, int b, float dt)
{
  float dt0 = dt * N;
  for(int i = 1; i <= N; i++)
  {
    for(int j = 1; j <= N; j++)
    {
      //backtrace the (x,y) position from the velocity & clamp to the boundaries of the fluid domain
      float x = clamp((float)i - dt0 * vel[idx(i, j)].data[FLUID_CHANNEL_VEL_X], 0.5f, N + 0.5f);
      float y = clamp((float)j - dt0 * vel[idx(i, j)].data[FLUID_CHANNEL_VEL_Y], 0.5f, N + 0.5f);

      //
      //  backtraced voxel
      //  +-------+-------+
      //  | i0,j0 | i1,j0 |
      //  +-------+-------+
      //  | i0,j1 | i1,j1 |
      //  +-------+-------+
      //

      int i0 = (int)x;
      int i1 = i0 + 1;

      int j0 = (int)y;
      int j1 = j0 + 1;

      //s0, s1 are remainder values used for blending between the above voxels
      float s1 = x - (float)i0;
      float s0 = 1.0f - s1;
      float t1 = y - (float)j0;
      float t0 = 1.0f - t1;

      // we sample from the 4 backtraced voxels above in proportions based
      // on where the exact backtraced position fell. (i.e. we almost certainly
      // didn't land exactly in the center of a voxel, so we need to blend between
      // several)
      for(int k = a; k < b; k++)
      {
        d[idx(i, j)].data[k] = s0 * (t0 * d0[idx(i0, j0)].data[k] +
                                     t1 * d0[idx(i0, j1)].data[k]) +
                               s1 * (t0 * d0[idx(i1, j0)].data[k] +
                                     t1 * d0[idx(i1, j1)].data[k]);
      }
    }
  }
  set_boundaries(false, true, d, 2, NUM_FLUID_CHANNELS);
}

//sets the behavior for the fluid boundaries
void Fluid2D::set_boundaries(bool vel, bool dens, FluidChannels *fc, int a, int b)
{
  if(vel)
  {
    for(int i = 1; i <= N; i++)
    {
      fc[idx(0, i)].data[0] = -fc[idx(1, i)].data[0];
      fc[idx(N + 1, i)].data[0] = -fc[idx(N, i)].data[0];
      fc[idx(i, 0)].data[0] = fc[idx(i, 1)].data[0];
      fc[idx(i, N + 1)].data[0] = fc[idx(i, N)].data[0];

      fc[idx(0, i)].data[1] = -fc[idx(1, i)].data[1];
      fc[idx(N + 1, i)].data[1] = -fc[idx(N, i)].data[1];
      fc[idx(i, 0)].data[1] = fc[idx(i, 1)].data[1];
      fc[idx(i, N + 1)].data[1] = fc[idx(i, N)].data[1];
    }
  }
  if(dens)
  {
    for(int i = 1; i <= N; i++)
    {
      for(int j = a; j < b; j++)
      {
        fc[idx(0, i)].data[j] = fc[idx(1, i)].data[j];
        fc[idx(N + 1, i)].data[j] = fc[idx(N, i)].data[j];
        fc[idx(i, 0)].data[j] = fc[idx(i, 1)].data[j];
        fc[idx(i, N + 1)].data[j] = fc[idx(i, N)].data[j];
      }
    }
  }

  //corners
  for(int j = a; j < b; j++)
  {
    //corners = half the sum of the two perpendicular neighbors
    fc[idx(0, 0)].data[j] =          0.5 * (fc[idx(1, 0)].data[j] +
                                            fc[idx(0, 1)].data[j]);       //top left
    fc[idx(0, N + 1)].data[j] =      0.5 * (fc[idx(1, N + 1)].data[j] +
                                            fc[idx(0, N)].data[j]);       //bottom left
    fc[idx(N + 1, 0)].data[j] =      0.5 * (fc[idx(N, 0)].data[j] +
                                            fc[idx(N + 1, 1)].data[j]);   //top right
    fc[idx(N + 1, N + 1)].data[j] =  0.5 * (fc[idx(N, N + 1)].data[j] +
                                            fc[idx(N + 1, N)].data[j]);   //bottom right
  }
}

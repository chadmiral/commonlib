#include <assert.h>
#include "fluid2d_angle_snapper.h"
#include "matrix.h"

Fluid2DAngleSnapper::Fluid2DAngleSnapper()
{
  num_angles = 6;
  allocate_angles();
  strength = 1.0f;
}

Fluid2DAngleSnapper::Fluid2DAngleSnapper(const int _num_angles)
{
  num_angles = _num_angles;
  strength = 1.0f;
  allocate_angles();
}

Fluid2DAngleSnapper::~Fluid2DAngleSnapper()
{
  delete angles;
}

void Fluid2DAngleSnapper::allocate_angles()
{
  angles = new Float2[num_angles];
  Float2 v(1.0f, 0.0f);
  float rot_angle = 0.0f;
  for(int i = 0; i < num_angles; i++)
  {
    Matrix2x2 m;
    m.rotation(rot_angle);

    angles[i] = m * v;

    rot_angle += (2.0f * M_PI) / (float)num_angles;
  }
}

void Fluid2DAngleSnapper::simulate(const float dt)
{
  for(int i = 0; i < fluid_dim[0]; i++)
  {
    for(int j = 0; j < fluid_dim[1]; j++)
    {
      Float2 vel(curr[idx(i, j)].data[0], curr[idx(i, j)].data[1]);
      float mag = vel.magnitude();

      float max_dp = -1.0f;
      int closest_angle = -1;
      for(int a = 0; a < num_angles; a++)
      {
        float dp = angles[a] * vel;
        if(dp > max_dp || closest_angle < 0)
        {
          closest_angle = a;
          max_dp = dp;
        }
      }

      assert(closest_angle >= 0);
      Float2 snap_vel = Float2(0.707f, 0.707f);//angles[closest_angle] * mag;
      Float2 new_vel = snap_vel;//lerp(vel, snap_vel, strength);
      curr[idx(i, j)].data[FLUID_CHANNEL_VEL_X] = snap_vel[0];
      curr[idx(i, j)].data[FLUID_CHANNEL_VEL_Y] = snap_vel[1];

      prev[idx(i, j)].data[FLUID_CHANNEL_VEL_X] = snap_vel[0];
      prev[idx(i, j)].data[FLUID_CHANNEL_VEL_Y] = snap_vel[1];
    }
  }
}

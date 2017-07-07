#ifndef __BPHYS_BAKER_H__
#define __BPHYS_BAKER_H__

//class to bake data from a blender physics cache (fluid sim, particles, etc)
//to textures or vertex colors usable for realtime applications

#include <stdio.h>
#include "texture.h"
#include "math_utility.h"

#define WITH_LZO

/* smoke data fileds (active_fields) */
#define SM_ACTIVE_HEAT		(1<<0)
#define SM_ACTIVE_FIRE		(1<<1)
#define SM_ACTIVE_COLORS	(1<<2)
#define SM_ACTIVE_COLOR_SET	(1<<3)

namespace Tool
{
  enum BPhysDataType
  {
    BPHYS_DATA_SOFT_BODY,
    BPHYS_DATA_PARTICLES,
    BPHYS_DATA_CLOTH,
    BPHYS_DATA_SMOKE_LOW,
    BPHYS_DATA_SMOKE_HIGH,
    BPHYS_DATA_DYNAMICPAINT,
    BPHYS_DATA_RIGID_BODY
  };

  struct BPhysHeader
  {
    char bphys[8];
    char data_type;
  };

  class BPhysBaker
  {
  public:
    BPhysBaker() {}
    ~BPhysBaker() {}

    void init(std::ostream &log = std::cout);
    void bake(FILE *f, std::string output_filename, std::ostream &log = std::cout, std::string tabs = "");
  private:
    std::string out_fname;

    void read_smoke_data(FILE *f, std::ostream &log = std::cout);
    /*void write_vertical_voxel_slice(float *voxels,
                                    unsigned int *vox_dim,
                                    Math::Float2 vox_range,
                                    int slice_idx,
                                    int tex_width,
                                    int tex_height,
                                    std::string output_tga_fname);*/
    void splat_voxel_data_onto_sphere_surface(unsigned int *vox_dim,
                                              Math::Float2 vox_range_r,
                                              Math::Float2 vox_range_g,
                                              Math::Float2 vox_range_b,
                                              bool convert_to_tangent_space,
                                              float radius,
                                              int tex_width,
                                              int tex_height,
                                              std::string output_tga_fname,
                                              float *voxels_r,
                                              float *voxels_g = NULL,
                                              float *voxels_b = NULL);

    void write_tga_from_pixels(std::string fname, int w, int h, unsigned char *pixels);
  };
};

#endif //__BPHYS_BAKER_H__

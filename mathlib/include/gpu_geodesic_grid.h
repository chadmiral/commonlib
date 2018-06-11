#pragma once

#include "shader.h"
#include "math_utility.h"

#if 0

namespace Math {
  class GPUGeodesicGrid {
  private:
    unsigned int           _subdivision_level; // # of subdivisions from an icosahedron
    Graphics::Shader      *_compute_shader;
    Graphics::Texture2D   *_data_texture;
    unsigned int           _data_tex_dim[2];
  public:
    GPUGeodesicGrid(unsigned int subdivisions = 0) {
      _subdivision_level = subdivisions;

      //base dim for an icosahedron is 6x6
      _data_tex_dim[0] = Math::next_power_of_2(7 * (1 + _subdivision_level));
      _data_tex_dim[1] = Math::next_power_of_2(6 * (1 + _subdivision_level));
    }

    void set_compute_shader(Graphics::Shader *c) { _compute_shader = c; }


    void init() {
      //setup the texture
      int bi = 2;


      
      int channels = 3;
      float *pixels = new float[channels * _data_tex_dim[0] * _data_tex_dim[1]];
      for (int i = 0; i < _data_tex_dim[0]; i++) {
        for (int j = 0; j < _data_tex_dim[1]; j++) {
          unsigned int idx = i * _data_tex_dim[0] + j;

          pixels[idx + 0] = x;
          pixels[idx + 1] = y;
          pixels[idx + 2] = z;
        }
      }
      _data_texture->update_pixels_from_mem(pixels);
    }

    void simulate(const double game_time, const double frame_time) {

    }
  };
};
#endif
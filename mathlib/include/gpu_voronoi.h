#ifndef __GPU_VORONOI_H__
#define __GPU_VORONOI_H__

#if defined(_WIN32)
#include <Windows.h>
#if defined __LOKI__
#include <GL/gl3w.h>
#include <GL/gl.h>
#else
#include <gl/glew.h>
#endif //__LOKI__
#endif //_WIN32

#if defined (__APPLE__)
#include <OpenGL/GL.h>
#else
#include <GL/GL.h>
#endif

#include <vector>
#include "math_utility.h"
#include <cstring>

// 1. collect points (CPU)
// 2. render 2D voronoi diagram in lat / lon (w / wrapping) by
//    rendering a cone for each cell location
// 3. the color of each cone corresponds to a cell location id
// 4. query consists of converting x,y,z to u,v then sampling
//    the voronoi diagram texture

#define GPU_VORONOI_WRAP_U  (1<<0)
#define GPU_VORONOI_WRAP_V  (1<<1)

namespace Math
{
  struct ConeVert
  {
    float x, y, z;
  };

  struct VoronoiVert
  {
    float x, y, z;
    float r, g, b;
  };

  class GPUVoronoi2D
  {
    public:
      GPUVoronoi2D(const GLuint num_seg = 24, const GLuint _max_num_sites = 1000, const GLuint flags = 0);
      ~GPUVoronoi2D();

      void init();
      void deinit();
      void reset();

      void set_num_segments(const GLuint num_seg) { num_cone_segments = num_seg; }
      GLuint get_num_segments() const { return num_cone_segments; }

      void set_flags(const GLuint flags) { behavior_flags = flags; }
      GLuint get_flags() const { return behavior_flags; }

      void set_tex_res(const int w, const int h) { fbo_res[0] = w; fbo_res[1] = h; }
      GLuint get_tex() const { return voronoi_diagram_tex; }

      void set_max_num_sites(const int n) { max_num_sites = n; }
      void add_site(Math::Float2 pt);
      void build_voronoi_diagram();

      unsigned int query_nearest_site(const Float2 p);

      void render_voronoi_texture();
    private:
      void render_fullscreen_quad();
      void setup_textured_quad_state();

      GLuint                        behavior_flags;
      std::vector<Math::Float2>     sites;

      int                           max_num_sites;

      int                           max_draw_indices;
      int                           max_draw_verts;
      
      GLuint                        num_cone_segments;
      GLuint                        num_cone_verts;
      GLuint                        cone_vbo;
      GLuint                        cone_ibo;
      ConeVert                      *cone_vertex_data;
      unsigned int                  *cone_index_data;

      GLuint                        fbo_res[2];
      GLuint                        depth_fbo;
      GLuint                        tex_format;
      GLuint                        voronoi_diagram_fbo;
      GLuint                        voronoi_diagram_tex;

      GLubyte                       *cpu_tex_data;
  };
};

#endif //__GPU_VORONOI_H__

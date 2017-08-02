#ifndef __GPU_VORONOI_H__
#define __GPU_VORONOI_H__

#include "render_surface.h"
#include "platform_gl.h"
#include "material.h"

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

  struct ConeInstanceData
  {
    float       x, y;
    GLubyte     r, g, b;
  };

  struct QuadVert
  {
    float x, y, z;
    float u, v;
  };

  class GPUVoronoi2D
  {
  private:
    std::vector<Math::Float2>     _sites;

    uint32_t                      _max_num_sites;

    Graphics::Material            *_cone_mat;
    Graphics::Shader              *_cone_shader;
    GLuint                        _num_cone_segments;
    GLuint                        _num_cone_verts;
    GLuint                        _cone_vbo;
    GLuint                        _cone_ibo;
    std::vector<ConeVert>         _cone_vertex_data;
    std::vector<uint32_t>         _cone_index_data;

    //instancing
    GLuint                        _cone_instance_bo;
    std::vector<ConeInstanceData> _cone_instance_data;

    GLuint                        _fbo_res[2];
    GLuint                        _tex_format;

    Graphics::Shader             *_render_surface_shader;
    Graphics::RenderSurface       _render_surface;

    GLubyte                      *_cpu_tex_data;

  public:
    GPUVoronoi2D(const GLuint num_seg = 24, const GLuint _max_num_sites = 1000);
    ~GPUVoronoi2D();

    void init();
    void deinit();
    void reset();

    void set_num_segments(const GLuint num_seg) { _num_cone_segments = num_seg; }
    GLuint get_num_segments() const { return _num_cone_segments; }

    void set_tex_res(const int w, const int h);
    GLuint get_tex() const { return _render_surface.get_tex()->get_tex_id(); }

    void set_max_num_sites(const int n) { _max_num_sites = n; }
    void add_site(Math::Float2 pt);
    void build_voronoi_diagram();

    uint32_t query_nearest_site(const Float2 p);

    void render_voronoi_texture();

  private:
    //void render_fullscreen_quad();
    //void setup_textured_quad_state();
    void update_instance_data();
  };
};

#endif //__GPU_VORONOI_H__

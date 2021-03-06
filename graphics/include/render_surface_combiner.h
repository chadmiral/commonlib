#ifndef __RENDER_SURFACE_COMBINER_H__
#define __RENDER_SURFACE_COMBINER_H__

#include "render_surface.h"
#include "texture.h"

namespace Graphics
{
  class RenderSurfaceCombiner
  {
  public:
    RenderSurfaceCombiner();
    ~RenderSurfaceCombiner();

    void set_fbo_res(const int w, const int h) { fbo_res[0] = w; fbo_res[1] = h; }

    void set_shader_names(std::string vs, std::string fs);
    void set_shader(Shader *s) { shader = s; }

    Material *get_mat() { return &mat; }

    //void set_lut_texture(Texture3D *lt) { lut3D = lt; }
    //void set_vignette_texture(Texture2D *vt) { vignette = vt; }

    void init();
    void deinit();
    void render();

  private:
    std::vector<RenderSurface *> _inputs;
    std::vector<RenderSurface *> _outputs;

    std::string vertex_shader_name;
    std::string fragment_shader_name;

    uint16_t               fbo_res[2];

    //quad data
    uint32_t               index_data[4];
    RenderSurfaceVert      vertex_data[4];
    GLuint                 vbo;
    GLuint                 ibo;

    Material               mat;
    Shader                 *shader;
    //Texture3D              *lut3D;
    //Texture2D              *vignette;

    ShaderUniformMatrix4x4 _proj_mat_uniform;
    ShaderUniformFloat2    gpu_texel_size;

    ShaderVertexAttrib     _xyz_attrib;
    ShaderVertexAttrib     _uv0_attrib;
  };
};

#endif //__RENDER_SURFACE_COMBINER_H__

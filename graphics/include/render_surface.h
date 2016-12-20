#ifndef __RENDER_SURFACE_H__
#define __RENDER_SURFACE_H__

#include "platform_gl.h"

#include <vector>

#include "material.h"
#include "texture.h"

namespace Graphics
{
  struct RenderSurfaceVert
  {
    float x, y, z;
    float u, v;
  };

  class RenderSurface
  {
  public:
    RenderSurface(const int w = 256, const int h = 256);
    ~RenderSurface();

    void set_fbo_res(const int w, const int h) { fbo_res[0] = w; fbo_res[1] = h; }
    void set_internal_format(GLenum f) { tex_internal_format = f; }
    void set_format(GLenum f) { tex_format = f; }
    void set_filtering_mode(GLenum f) { tex_filter = f; }

    Texture2D *get_tex() const { return target_tex; }
    Material *get_mat() { return &mat; }
    void set_shader(Graphics::Shader *s) { mat.set_shader(s); }

    virtual void init();
    virtual void deinit();
    virtual void render();

    void capture();
    void release();

  protected:
    uint32_t               index_data[4];
    RenderSurfaceVert      vertex_data[4];
    GLuint                 vbo;
    GLuint                 ibo;

    GLuint                 target_fbo;
    Texture2D             *target_tex;
    Texture2D             *depth_tex;

    bool                   use_depth;
    GLuint                 depth_fbo;

    int                    fbo_res[2];
    GLint                  win_viewport[4];
    GLenum                 tex_internal_format;
    GLenum                 tex_format;
    GLenum                 tex_type;
    GLenum                 tex_filter;

    ShaderUniformMatrix4x4 _projection_matrix;
    //ShaderUniformMatrix4x4 _modelview_matrix; //Identity

    ShaderVertexAttrib     _xyz_attrib;
    ShaderVertexAttrib     _uv0_attrib;

    Material                mat;

    //TODO: clean all this uniform BS up - this should just use the Material / Shader system
    //std::vector<Math::Float2>                                f2_uni_const;
    //std::vector<std::pair<Math::Float2 *, std::string> >     uniforms;
    //std::vector<std::pair<Math::Float3, std::string> >       float3_uniforms;
  };
};

#endif //__RENDER_SURFACE_H__

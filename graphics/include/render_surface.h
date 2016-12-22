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
  private:
    void create_target_texture();
    void create_depth_texture();
    void bind_textures_to_fbo();

    void delete_frame_buffer_object();
  public:
    RenderSurface(const int w = 256, const int h = 256);
    ~RenderSurface();

    void set_fbo_res(const int w, const int h) { fbo_res[0] = w; fbo_res[1] = h; }
    void get_fbo_res(int &w, int &h) { w = fbo_res[0]; h = fbo_res[1]; }
    void set_internal_format(GLenum f) { tex_internal_format = f; }
    void set_format(GLenum f) { tex_format = f; }
    void set_filtering_mode(GLenum f) { tex_filter = f; }

    Texture2D *get_tex() const { return target_tex; }
    Material *get_mat() { return &mat; }
    void set_shader(Graphics::Shader *s) { mat.set_shader(s); }

    virtual void init();
    virtual void deinit();
    virtual void render();

    void resize(const uint32_t w, const uint32_t h);

    void capture();
    void release();
  };
};

#endif //__RENDER_SURFACE_H__

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

    uint16_t               fbo_res[2];
    GLint                  win_viewport[4];
    GLenum                 tex_internal_format;
    GLenum                 tex_format;
    GLenum                 tex_type;
    GLenum                 tex_filter;

    std::vector<ShaderUniformMatrix4x4> _projection_matrices;
    //ShaderUniformMatrix4x4 _modelview_matrix; //Identity

    std::vector<ShaderVertexAttrib>     _xyz_attribs;
    std::vector<ShaderVertexAttrib>     _uv0_attribs;

    std::vector<Material *>  _materials; //a material for each render method
    std::vector<std::string> _method_names;
    std::vector<bool>        _add_surface_textures;
  private:
    void create_target_texture();
    void create_depth_texture();
    void bind_textures_to_fbo();

    void delete_frame_buffer_object();
  public:
    RenderSurface(const uint16_t w = 256, const uint16_t h = 256);
    ~RenderSurface();

    void set_fbo_res(const uint16_t w, const uint16_t h) { fbo_res[0] = w; fbo_res[1] = h; }
    void get_fbo_res(int &w, int &h) { w = fbo_res[0]; h = fbo_res[1]; }
    void set_internal_format(GLenum f) { tex_internal_format = f; }
    void set_format(GLenum f) { tex_format = f; }
    void set_filtering_mode(GLenum f) { tex_filter = f; }

    Texture2D *get_tex() const { return target_tex; }
    Texture2D *get_depth_tex() const { return depth_tex; }
    Material *get_mat(const uint16_t method = 0) { return _materials[method]; }
    uint16_t get_num_methods() const { return (uint16_t)_method_names.size(); }
    std::string get_method_name(const uint16_t method) const { return _method_names[method]; }
    Material *add_shader(Graphics::Shader *s, std::string name = "<none>", bool add_surface_tex = true);

    virtual void init();
    virtual void deinit();
    virtual void render(const uint16_t method = 1);

    void resize(const uint16_t w, const uint16_t h);

    void capture();
    void release();
  };
};

#endif //__RENDER_SURFACE_H__

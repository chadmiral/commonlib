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
    uint32_t               _index_data[4];
    RenderSurfaceVert      _vertex_data[4];
    GLuint                 _vbo;
    GLuint                 _ibo;

    GLuint                 _target_fbo;
    Texture2D             *_target_tex;
    Texture2D             *_depth_tex;

    bool                   _depth_buffer_is_mine;
    bool                   _use_depth;
    //GLuint                 _depth_fbo;

    uint16_t               _fbo_res[2];
    GLint                  _win_viewport[4];
    GLenum                 _tex_internal_format;
    GLenum                 _tex_format;
    GLenum                 _tex_type;
    GLenum                 _tex_filter;

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

    void set_fbo_res(const uint16_t w, const uint16_t h) { _fbo_res[0] = w; _fbo_res[1] = h; }
    void get_fbo_res(int &w, int &h) { w = _fbo_res[0]; h = _fbo_res[1]; }
    void set_internal_format(GLenum f) { _tex_internal_format = f; }
    void set_format(GLenum f) { _tex_format = f; }
    void set_filtering_mode(GLenum f) { _tex_filter = f; }
    void use_depth(const bool d) { _use_depth = d; }

    float compute_frame_luminosity() const;
    
    void attach_depth_buffer(Texture2D *depth_tex);

    Texture2D *get_tex() const { return _target_tex; }
    void set_tex(Texture2D *t) { _target_tex = t; } //probably only want to use this if 
    Texture2D *get_depth_tex() const { return _depth_tex; }
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

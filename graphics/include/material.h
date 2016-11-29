#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#if defined (_WIN32)
#include <Windows.h>
#include <GL/glew.h>
#endif//_WIN32

#if defined (__APPLE__)
#include <OpenGL/gl.h>
#endif

#include <vector>

#include "math_utility.h"
#include "shader.h"
#include "texture.h"

namespace Graphics {
  class Material {
    public:
      Material();
      ~Material();

      void init();

      //void set_shader_filenames(std::string vs_name, std::string fs_name);
      void set_shader(Shader *s) { shader = s; }
      Shader *get_shader() { return shader; }

      void add_texture(Graphics::Texture2D *t, std::string name);
      void add_texture(Graphics::Texture3D *t, std::string name);

      unsigned int get_num_textures() const { return (unsigned int)(textures_2d.size() + textures_3d.size()); }

      void add_uniform_var(ShaderUniformVariable *suv) { shader_uniforms.push_back(suv); }
      void add_vertex_attrib(ShaderVertexAttrib *sva) { shader_vertex_attribs.push_back(sva); }

      void enable_lighting(const bool l); //archaic?

      void enable_blending(const bool b);
      void set_blend_mode(const GLenum src, const GLenum dst);
      void set_blend_mode_additive() { src_blend_param = GL_SRC_ALPHA; dst_blend_param = GL_ONE; }

      void enable_depth_write(const bool d) { depth_write = d; }
      void enable_depth_read(const bool d) { depth_read = d; }
      void set_depth_function(GLenum df) { depth_func = df; }
      void set_depth_range(const Math::Float2 dr) { depth_range = dr; }

      void enable_backface_culling(const bool bc) { backface_cull = bc; }
      void set_culling_winding(const GLenum cw) { backface_cull_winding = cw; }

      void render() const;
      void cleanup() const;

      void add_tex_idx(const int tid);

#ifdef __DEBUG__
      void set_verbose(const bool v) { verbose = v; }
#endif //__DEBUG__

    private:
      Shader                                               *shader;
      std::vector<ShaderUniformVariable *>                 shader_uniforms;
      std::vector<ShaderVertexAttrib *>                    shader_vertex_attribs;
      std::vector<ShaderUniformInt>                        texture_uniforms; //TODO: maybe a better way?
      std::vector<std::pair<Texture2D *, std::string> >    textures_2d;
      std::vector<std::pair<Texture3D *, std::string> >    textures_3d;

      //lighting parameters
      bool lighting;

      //transparency parameters
      bool transparent;
      GLenum src_blend_param;
      GLenum dst_blend_param;

      //depth parameters
      GLenum depth_func;
      bool depth_write;
      bool depth_read;
      Math::Float2 depth_range;

      bool backface_cull;
      GLenum backface_cull_winding;

#ifdef __DEBUG__
      bool verbose;
#endif //__DEBUG__
  };
};

#endif // __MATERIAL_H__

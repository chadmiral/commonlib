#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#if defined (__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
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

      void add_uniform_var(ShaderUniformVariable *suv) { shader_uniforms.push_back(suv); }

      void enable_lighting(const bool l); //archaic?
      void enable_blending(const bool b);

      void set_blend_mode(const GLenum src, const GLenum dst);

      void render() const;
      void cleanup() const;

      void add_tex_idx(const int tid);

    private:
      Shader                                               *shader;
      std::vector<ShaderUniformVariable *>                 shader_uniforms;
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
      Math::Float2 depth_range;
  };
};

#endif // __MATERIAL_H__

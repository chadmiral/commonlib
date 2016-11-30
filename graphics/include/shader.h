#ifndef _SHADER_H_
#define _SHADER_H_

#if defined (_WIN32)
#include <Windows.h>
#include <GL/glew.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include <iostream>
#include <assert.h>

#include "math_utility.h"
#include "gl_error.h"
#include "platform.h"

namespace Graphics
{
  class Shader
  {
  public:
    Shader();
    ~Shader();

    void deinit();

    void create_program();
    void set_shader_filenames(std::string vs_fname, std::string fs_fname);
    void compile_and_link_from_source(const char *vs, const char *fs);
    GLuint compile_shader_from_source(GLenum shader_type, const char *source);
    void link_shader();
    
    bool load_link_and_compile();
    void render();

    GLuint gl_fragment_shader;
    GLuint gl_vertex_shader;
    GLuint gl_shader_program;

    std::string gl_fragment_shader_fname;
    std::string gl_vertex_shader_fname;
  };

  class ShaderUniformVariable
  {
  protected:
    std::string    _name;
    GLint          _loc;
  public:
    ShaderUniformVariable() { _name = "foo"; _loc = -1; }
    ~ShaderUniformVariable() {}

    std::string get_name() const { return _name; }
    void set_name(std::string name) { _name = name; }
    //void set_loc(GLint loc) { _loc = loc; }
    void set_loc(Shader *sp)
    {
      _loc = glGetUniformLocation(sp->gl_shader_program, _name.c_str());
      if (_loc == -1)
      {
        SET_TEXT_COLOR(CONSOLE_COLOR_ERROR);
        std::cerr << "Could not find shader uniform location!!!" << std::endl;
        std::cerr << "\tshader: " << sp->gl_vertex_shader_fname.c_str() << ", " << sp->gl_fragment_shader_fname.c_str() << std::endl;
        std::cerr << "\tuniform name: " << _name.c_str() << std::endl;
        SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
        //assert(false);
      }
      gl_check_error();
    }

    virtual void render() const = 0;  //set uniform variables - to be called prior to Shader::render()
  };

  class ShaderUniformInt : public ShaderUniformVariable
  {
  private:
    int var;
  public:
    ShaderUniformInt() : ShaderUniformVariable() { var = 0; }
    ~ShaderUniformInt() {}

    virtual void render() const { glUniform1i(_loc, var); gl_check_error(); }

    void set_var(const int v) { var = v; }
    int get_var() const { return var; }
  };

  class ShaderUniformFloat : public ShaderUniformVariable
  {
  private:
    float var;
  public:
    ShaderUniformFloat() : ShaderUniformVariable() { var = 0.0f; }
    ~ShaderUniformFloat() {}

    virtual void render() const { glUniform1f(_loc, var); gl_check_error(); }
    void set_var(const float v) { var = v; }
  };

  class ShaderUniformFloat2 : public ShaderUniformVariable
  {
  private:
    Math::Float2 var;
  public:
    ShaderUniformFloat2() : ShaderUniformVariable() {}
    ~ShaderUniformFloat2() {}

    virtual void render() const { glUniform2f(_loc, var._val[0], var._val[1]); gl_check_error(); }
    void set_var(const Math::Float2 v) { var = v; }
    Math::Float2 *get_var_ptr() { return &var; }
  };

  class ShaderUniformFloat3 : public ShaderUniformVariable
  {
  private:
    Math::Float3 var;
  public:
    ShaderUniformFloat3() : ShaderUniformVariable() {}
    ~ShaderUniformFloat3() {}

    virtual void render() const { glUniform3f(_loc, var._val[0], var._val[1], var._val[2]); gl_check_error(); }
    void set_var(const Math::Float3 v) { var = v; }
  };

  //TODO
  class ShaderVertexAttrib
  {
  protected:
    std::string    _name;
    GLint          _loc;
    
    uint32_t       _type;
    uint32_t       _count;
    uint32_t       _offset;
    uint32_t       _vertex_size;
  public:
    ShaderVertexAttrib() {}
    ~ShaderVertexAttrib() {}

    std::string get_name() const { return _name; }
    void set_name(std::string name) { _name = name; }
    void set_loc(uint32_t loc_id) { _loc = loc_id; }
    void set_loc(Shader *sp, std::string name, uint32_t vertex_size, uint32_t count, uint32_t offset, uint32_t type = GL_FLOAT)
    {
      _name = name;
      _type =          type;
      _count =         count;
      _offset =        offset;
      _vertex_size =   vertex_size;

      _loc = glGetAttribLocation(sp->gl_shader_program, _name.c_str());
      if (_loc == -1)
      {
        SET_TEXT_COLOR(CONSOLE_COLOR_ERROR);
        std::cerr << "Could not find shader vertex location!!!" << std::endl;
        std::cerr << "\tshader: " << sp->gl_vertex_shader_fname.c_str() << ", " << sp->gl_fragment_shader_fname.c_str() << std::endl;
        std::cerr << "\tattrib: " << _name.c_str() << std::endl;
        SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
        //assert(false);
      }
      gl_check_error();
    }

    //to be called prior to Shader::render()
    void render() const
    {
#if defined (_DEBUG)
      if (_loc != -1)
#endif
      {
        glEnableVertexAttribArray(_loc);
        glVertexAttribPointer(_loc, 3, _type, GL_FALSE, _vertex_size, (void *)_offset);
        gl_check_error();
      }
    }

    void cleanup() const
    {
#if defined (_DEBUG)
      if (_loc != -1)
#endif
      {
        glDisableVertexAttribArray(_loc);
        gl_check_error();
      }
    }
  };
};

#endif // _SHADER_H_

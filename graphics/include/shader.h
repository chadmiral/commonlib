#ifndef _SHADER_H_
#define _SHADER_H_

#include "platform_gl.h"

#include <iostream>
#include <assert.h>
#include <vector>

#include "math_utility.h"
#include "gl_error.h"
#include "platform.h"
#include "matrix.h"

namespace Graphics
{

  enum ShaderType
  {
    SHADER_TYPE_RENDER,
    SHADER_TYPE_COMPUTE,

    NUM_SHADER_TYPES
  };

  class Shader
  {
  public:
    Shader();
    ~Shader();

    void deinit();

    void create_program(std::ostream &log = std::cout);
    void set_shader_filenames(std::string vs_fname, std::string fs_fname);
    void compile_and_link_from_source(const char *vs, const char *fs, std::string tabs = "", std::ostream &log = std::cout);
    void compile_and_link_compute_shader(const char *cs_src, std::string tabs = "", std::ostream &log = std::cout);
    GLuint compile_shader_from_source(GLenum shader_type, const char *source, std::string tabs = "", std::ostream &log = std::cout);
    void link_shader(std::string tabs = "", std::ostream &log = std::cout);

    std::string include_glsl(std::string inc_fname, std::vector<std::string> *path);

    void parse_source(std::string source, std::string &dest, std::vector<std::string> *path); //load includes, etc... before compiling
    bool load_link_and_compile(std::vector<std::string> *path = NULL, std::string tabs = "", std::ostream &log = std::cout);
    void render();
    void execute();

    GLuint gl_fragment_shader;
    GLuint gl_vertex_shader;
    GLuint gl_compute_shader;

    GLuint gl_shader_program;

    std::string gl_fragment_shader_fname;
    std::string gl_vertex_shader_fname;
    std::string gl_compute_shader_fname;

    GLuint _local_size_x;
    GLuint _local_size_y;
    GLuint _local_size_z;

    GLuint _num_work_groups_x;
    GLuint _num_work_groups_y;
    GLuint _num_work_groups_z;
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
      }
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

    virtual void render() const { glUniform1i(_loc, var); }

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

    virtual void render() const { glUniform1f(_loc, var); }
    void set_var(const float v) { var = v; }
    float get_var() const { return var; }
    float *get_var_ptr() { return &var; }
  };

  class ShaderUniformFloat2 : public ShaderUniformVariable
  {
  private:
    Math::Float2 var;
  public:
    ShaderUniformFloat2() : ShaderUniformVariable() {}
    ~ShaderUniformFloat2() {}

    virtual void render() const { glUniform2f(_loc, var._val[0], var._val[1]); }
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

    virtual void render() const { glUniform3f(_loc, var._val[0], var._val[1], var._val[2]); }
    void set_var(const Math::Float3 v) { var = v; }
  };

  class ShaderUniformFloat4 : public ShaderUniformVariable
  {
  private:
    Math::Float4 var;
  public:
    ShaderUniformFloat4() : ShaderUniformVariable() {}
    ~ShaderUniformFloat4() {}

    virtual void render() const { glUniform4f(_loc, var._val[0], var._val[1], var._val[2], var._val[3]); }
    void set_var(const Math::Float4 v) { var = v; }
  };

  class ShaderUniformMatrix3x3 : public ShaderUniformVariable
  {
  private:
    GLfloat var[9];
  public:
    ShaderUniformMatrix3x3() : ShaderUniformVariable() {}
    ~ShaderUniformMatrix3x3() {}

    virtual void render() const { glUniformMatrix3fv(_loc, 1, false, var); }
    void set_var(const Math::Matrix3x3 &m)
    {
      uint32_t k = 0;
      for (uint32_t i = 0; i < 3; i++)
      {
        for (uint32_t j = 0; j < 3; j++)
        {
          var[k++] = m(i, j);
        }
      }
    }
  };

  class ShaderUniformMatrix4x4 : public ShaderUniformVariable
  {
  private:
    GLfloat var[16];
  public:
    ShaderUniformMatrix4x4() : ShaderUniformVariable() {}
    ~ShaderUniformMatrix4x4() {}

    virtual void render() const { glUniformMatrix4fv(_loc, 1, false, var); }
    void set_var(const Math::Matrix4x4 &m)
    {
      uint32_t k = 0;
      for (uint32_t i = 0; i < 4; i++)
      {
        for (uint32_t j = 0; j < 4; j++)
        {
          var[k++] = m(i, j);
        }
      }
    }
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
    void set_loc(Shader *sp, std::string name, uint32_t vertex_size, uint32_t count, uint32_t offset, uint32_t type = GL_FLOAT, std::ostream &log = std::cout)
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
        log << "Could not find shader vertex location!!!" << std::endl;
        log << "\tshader: " << sp->gl_vertex_shader_fname.c_str() << ", " << sp->gl_fragment_shader_fname.c_str() << std::endl;
        log << "\tattrib: " << _name.c_str() << std::endl;
        SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
        //assert(false);
      }
      gl_check_error(log);
    }

    //to be called prior to Shader::render()
    void render() const
    {
#if defined (_DEBUG)
      if (_loc != -1)
#endif
      {
        glEnableVertexAttribArray(_loc);
        glVertexAttribPointer(_loc, _count, _type, GL_FALSE, _vertex_size, (void *)_offset);
        gl_check_error(std::cout);
      }
    }

    void cleanup() const
    {
#if defined (_DEBUG)
      if (_loc != -1)
#endif
      {
        glDisableVertexAttribArray(_loc);
        gl_check_error(std::cout);
      }
    }
  };
};

#endif // _SHADER_H_

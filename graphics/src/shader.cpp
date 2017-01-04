#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "platform.h"
#include "shader.h"
#include "gl_error.h"

using namespace std;
using namespace Graphics;

Shader::Shader()
{
  gl_fragment_shader =  0;
  gl_vertex_shader =    0;
  gl_shader_program =   0;

  gl_fragment_shader_fname[0] = '\0';
  gl_vertex_shader_fname[0] = '\0';
}

Shader::~Shader()
{
  deinit();
}

void Shader::deinit()
{
  glDeleteProgram(gl_shader_program);
  glDeleteProgram(gl_vertex_shader);
  glDeleteProgram(gl_fragment_shader);
}

void Shader::set_shader_filenames(std::string vs_fname, std::string fs_fname)
{
  gl_vertex_shader_fname = vs_fname;
  gl_fragment_shader_fname = fs_fname;
}

void print_log(GLuint obj, std::ostream &log)
{
  int infologLength = 0;
  int maxLength = 1024;

  if (glIsShader(obj))
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &maxLength);
  else
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &maxLength);

  char *infoLog = new char[maxLength];

  if (glIsShader(obj))
    glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
  else
    glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);

  if (infologLength > 0)
    printf("%s\n", infoLog);

  delete infoLog;
}

void Shader::create_program(std::ostream &log)
{
  //create the shader program
  gl_shader_program = glCreateProgram();
  gl_check_error(log);
  assert(gl_shader_program);
}

void Shader::compile_and_link_from_source(const char *vs, const char *fs, std::ostream &log)
{
  create_program(log);

  log << "compiling vertex shader..." << endl;
  gl_vertex_shader = compile_shader_from_source(GL_VERTEX_SHADER, vs);
  log << "compiling fragment shader..." << endl;
  gl_fragment_shader = compile_shader_from_source(GL_FRAGMENT_SHADER, fs);
  link_shader(log);
}

GLuint Shader::compile_shader_from_source(GLenum shader_type, const char *source, std::ostream &log)
{
  GLuint my_shader = glCreateShader(shader_type);
  glShaderSource(my_shader, 1, &source, NULL);
  glCompileShader(my_shader);

  gl_check_error(log);

  GLint compiled = false;
  glGetShaderiv(my_shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled)
  {
    GLint maxLength = 0;
  	glGetShaderiv(my_shader, GL_INFO_LOG_LENGTH, &maxLength);

  	// The maxLength includes the NULL character
  	GLchar *errorLog = new char[maxLength];
  	glGetShaderInfoLog(my_shader, maxLength, &maxLength, &errorLog[0]);

    SET_TEXT_COLOR(CONSOLE_COLOR_RED);
    log << errorLog << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);

  	glDeleteShader(my_shader);
	  delete [] errorLog;
  }
  else
  {
    SET_TEXT_COLOR(CONSOLE_COLOR_CYAN);
    log << "OK" << endl;
    SET_TEXT_COLOR(CONSOLE_COLOR_DEFAULT);
  }

  glAttachShader(gl_shader_program, my_shader);
  if (shader_type == GL_VERTEX_SHADER)
  {
    gl_vertex_shader = my_shader;
  }
  else if (shader_type == GL_FRAGMENT_SHADER)
  {
    gl_fragment_shader = my_shader;
  }

  gl_check_error(log);
  return my_shader;
}

void Shader::link_shader(std::ostream &log)
{
  glLinkProgram(gl_shader_program);
  print_log(gl_shader_program, log);
}

bool Shader::load_link_and_compile(std::ostream &log)
{
    log<<"loading vertex shader "<<gl_vertex_shader_fname.c_str()<<endl;

    create_program();

    //load shader file from disk
    FILE *fp = NULL;
    FOPEN(fp, gl_vertex_shader_fname.c_str(), "r");
    if(fp)
    {
      fseek(fp, 0, SEEK_END);
      uint32_t string_size = ftell(fp);
      rewind(fp);

      char *gl_vertex_source = (char *)malloc(sizeof(char) * (string_size + 1));
      memset(gl_vertex_source, 0, string_size + 1);
      fread(gl_vertex_source, sizeof(char), string_size, fp);

      gl_vertex_shader = compile_shader_from_source(GL_VERTEX_SHADER, gl_vertex_source);
      print_log(gl_vertex_shader, log);
      print_log(gl_shader_program, log);

      free(gl_vertex_source);
      fclose(fp);
    }
    else
    {
      log<<"could not open vertex shader file! (no file handle)"<<endl;
    }

    log<<"loading fragment shader "<<gl_fragment_shader_fname.c_str()<<endl;
    fp = NULL;
    FOPEN(fp, gl_fragment_shader_fname.c_str(), "r");
    if(fp)
    {
      fseek(fp, 0, SEEK_END);
      uint32_t string_size = ftell(fp);
      rewind(fp);

      char *gl_fragment_source = (char *)malloc(sizeof(char) * (string_size + 1));
      memset(gl_fragment_source, 0, string_size + 1);
      fread(gl_fragment_source, sizeof(char), string_size, fp);

      gl_fragment_shader = compile_shader_from_source(GL_FRAGMENT_SHADER, gl_fragment_source);
      print_log(gl_fragment_shader, log);
      print_log(gl_shader_program, log);

      free(gl_fragment_source);
      fclose(fp);
    }

    link_shader(log);

    return true;
}

//apply the shader before rendering
void Shader::render()
{
  glUseProgram(gl_shader_program);
}

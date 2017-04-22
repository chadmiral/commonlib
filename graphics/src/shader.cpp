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
  {
    log << infoLog << endl;
  }

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

std::string Shader::include_glsl(std::string inc_fname, std::vector<std::string> *path)
{
  std::string source;
  //cout << "including file " << inc_fname.c_str() << endl;
  assert(path);

  for (uint32_t i = 0; i < path->size(); i++)
  {
    std::string full_fname = (*path)[i] + inc_fname;
    //cout << full_fname << endl;

    FILE *f = fopen(full_fname.c_str(), "r");
    if (f)
    {
      fseek(f, 0, SEEK_END);
      uint32_t fsize = ftell(f);
      rewind(f);

      char *tmp = (char *)malloc(fsize + 1);//new char[(fsize + 1) / sizeof(char)];
      memset(tmp, 0, fsize + 1);
      fread(tmp, fsize, 1, f);
      fclose(f);

      source = tmp;
      free(tmp);//delete tmp;
      return source;
    }
  }

  return source;
}

void Shader::parse_source(std::string source, std::string &dest, std::vector<std::string> *path)
{
  int last = 0; //must be int for npos comparison to work
  int next = 0; //must be int for npos comparison to work
  std::string line;

  //loop through every line of the source
  while ((next = source.find("\n", last)) != std::string::npos)
  {
    line = source.substr(last, next - last);

    //see if we have a "#include in this line
    int pos = line.find("#include");
    if (pos != std::string::npos)
    {
      uint32_t start_quote = line.find("\"", pos);
      uint32_t end_quote = line.find("\"", start_quote + 1);
      std::string inc_fname = line.substr(start_quote + 1, end_quote - start_quote - 1);
      std::string inc_src = include_glsl(inc_fname, path);
      //cout << "pasting in this source: " << endl;
      //cout << inc_src.c_str() << endl;
      source.replace(last, next - last, inc_src);
      next = last = 0;
    }
    else
    {
      last = next + 1;
    }
  }

  //copy the new source into our destination
  dest = source;
}

bool Shader::load_link_and_compile(std::vector<std::string> *path, std::ostream &log)
{
    log << "loading vertex shader " << gl_vertex_shader_fname.c_str() << endl;

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

      std::string final_vs_source;
      parse_source(gl_vertex_source, final_vs_source, path);

      gl_vertex_shader = compile_shader_from_source(GL_VERTEX_SHADER, final_vs_source.c_str());
      print_log(gl_vertex_shader, log);
      print_log(gl_shader_program, log);

      free(gl_vertex_source);
      fclose(fp);
    }
    else
    {
      log << "could not open vertex shader file! (no file handle)" << endl;
    }

    log << "loading fragment shader " << gl_fragment_shader_fname.c_str() << endl;
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

      std::string final_fs_source;
      parse_source(gl_fragment_source, final_fs_source, path);

      gl_fragment_shader = compile_shader_from_source(GL_FRAGMENT_SHADER, final_fs_source.c_str());
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

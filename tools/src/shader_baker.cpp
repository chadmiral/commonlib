#include "platform_gl.h"

#include <iostream>
#include <SDL.h>

#include "tool.h"
#include "shader_baker.h"
#include "platform.h"

using namespace std;
using namespace Tool;

ShaderBaker::ShaderBaker() {}
ShaderBaker::~ShaderBaker() {}

void ShaderBaker::init()
{
  //this is purely so we can compile shaders and get output
  SDL_Window *win = SDL_CreateWindow("tmp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  SDL_GL_CreateContext(win);

#ifndef __LOKI__
  glewInit();
#endif //__LOKI__
}

void ShaderBaker::bake(mxml_node_t *tree, std::string output_fname, std::ostream &log, std::string tabs)
{
  log << "ShaderBaker::bake() - I didn't implement this shit yet!!!" << endl;
}

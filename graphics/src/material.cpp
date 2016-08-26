#include <GL/glew.h>
#include <iostream>
#include <assert.h>
#include <cstring>

#include "material.h"
#include "gl_error.h"

using namespace std;
using namespace Graphics;
using namespace Math;

Material::Material()
{
  lighting = false;
  transparent = false;

  //standard alpha blending
  src_blend_param = GL_SRC_COLOR;
  dst_blend_param = GL_ONE_MINUS_SRC_ALPHA;

  depth_read = true;
  depth_write = true;
  depth_func = GL_LEQUAL;
  depth_range = Float2(0.0f, 1.0f);

  backface_cull = true;
  backface_cull_winding = GL_CCW;

  shader = NULL;
#ifdef __DEBUG__
  verbose = false;
#endif //__DEBUG__
}

Material::~Material()
{
}

void Material::init()
{
#ifdef __DEBUG__
  if(verbose)
  {
    cout<<"Material::init()"<<endl;
    cout<<"\tnum 2D textures: "<<textures_2d.size()<<endl;
    cout<<"\tnum 3D textures: "<<textures_3d.size()<<endl;
  }
#endif //__DEBUG__


  //NOTE: we assume that by this point the shader has been loaded, compiled, linked, etc...
  //create uniforms for all the textures
  if (shader)
  {
    for (uint32_t i = 0; i < textures_2d.size(); i++)
    {
      ShaderUniformInt sui;
      sui.set_name(textures_2d[i].second);
      sui.set_loc(shader);
      sui.set_var(i);

      texture_uniforms.push_back(sui);
    }

    gl_check_error();

    for (uint32_t i = 0; i < textures_3d.size(); i++)
    {
      ShaderUniformInt sui;
      sui.set_name(textures_3d[i].second);
      sui.set_loc(shader);
      sui.set_var(i + (unsigned int)textures_2d.size());

      texture_uniforms.push_back(sui);
    }

    gl_check_error();

    //collect all the uniform variable locations
    for (uint32_t i = 0; i < shader_uniforms.size(); i++)
    {
      shader_uniforms[i]->set_loc(shader);
    }
  }

  gl_check_error();

#ifdef __DEBUG__
  if(verbose)
  {
    cout<<"\tTexture Uniform Variables: "<<endl;
    for(int i = 0; i < texture_uniforms.size(); i++)
    {
      cout<<"\t\t"<<texture_uniforms[i].get_name()<<": "<<texture_uniforms[i].get_var()<<endl;
    }
  }
#endif //__DEBUG__
}

void Material::add_texture(Texture2D *t, string name)
{
  assert(t);
  std::pair<Texture2D *, string> p(t, name);
  textures_2d.push_back(p);
}

void Material::add_texture(Texture3D *t, string name)
{
  //assert(false);
  assert(t);
  std::pair<Texture3D *, string> p(t, name);
  textures_3d.push_back(p);
}

void Material::enable_lighting(const bool l)
{
  lighting = l;
}

void Material::enable_blending(const bool b)
{
  transparent = b;
}

void Material::set_blend_mode(const GLenum src, const GLenum dst)
{
  src_blend_param = src;
  dst_blend_param = dst;
}

void Material::render() const
{
  gl_check_error();
  if (shader)
  {
    shader->render();

    gl_check_error();

    //set up shader uniform variables
    for (uint32_t i = 0; i < shader_uniforms.size(); i++)
    {
      shader_uniforms[i]->render();
      gl_check_error();
    }

    //set up shader vertex attribs
    for (uint32_t i = 0; i < shader_vertex_attribs.size(); i++)
    {
      shader_vertex_attribs[i]->render();
      gl_check_error();
    }

    for (uint32_t i = 0; i < texture_uniforms.size(); i++)
    {
      texture_uniforms[i].render();
      gl_check_error();
    }
  }
  else
  {
    glUseProgram(0);
  }

  //textures
  for (uint32_t i = 0; i < textures_2d.size(); i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures_2d[i].first->get_tex_id());

    gl_check_error();
  }

  for (uint32_t i = 0; i < textures_3d.size(); i++)
  {
    GLuint actual_tex_slot = GL_TEXTURE0 + textures_2d.size() + i;
    glActiveTexture(actual_tex_slot);
    glEnable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, textures_3d[i].first->get_tex_id());
    gl_check_error();
  }

  if(lighting) { glEnable(GL_LIGHTING); }
  else { glDisable(GL_LIGHTING); }

  gl_check_error();

  //TODO - we should really render all transparents
  //       at once... with back to front sorting... yeah.
  if(transparent)
  {
    glEnable(GL_BLEND);
    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER, 0.0f);
    glBlendFunc(src_blend_param, dst_blend_param);
  }
  else
  {
    glDisable(GL_BLEND);
  }

  gl_check_error();

  //depth stuff
  if(depth_read) { glEnable(GL_DEPTH_TEST); }
  else { glDisable(GL_DEPTH_TEST); }
  gl_check_error();
  if(depth_write) { glDepthMask(GL_TRUE); }
  else { glDepthMask(GL_FALSE); }
  gl_check_error();
  glDepthRange(depth_range[0], depth_range[1]);

  gl_check_error();

  //backface culling
  if(backface_cull)
  {
    glEnable(GL_CULL_FACE);
    glFrontFace(backface_cull_winding);
  }
  else { glDisable(GL_CULL_FACE); }
  gl_check_error();
}

void Material::cleanup() const
{
  glUseProgramObjectARB(0);

  for(uint32_t i = 0; i < textures_2d.size(); i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  for(uint32_t i = 0; i < textures_3d.size(); i++)
  {
    glActiveTexture(GL_TEXTURE0 + i + textures_2d.size());
    glDisable(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);
  }

  for (uint32_t i = 0; i < shader_vertex_attribs.size(); i++)
  {
    shader_vertex_attribs[i]->cleanup();
  }
}

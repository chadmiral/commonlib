#include "texture.h"

#if defined(_WIN32)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

//#define __USE_SOIL__

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#if defined(__USE_SOIL__)
#include <SOIL/SOIL.h>
#endif

#include <iostream>
#include <assert.h>

#include "gl_error.h"


using namespace std;
using namespace Graphics;

Texture2D::Texture2D(const unsigned int w,
                     const unsigned int h,
                     const GLuint _data_format,
                     const GLuint _internal_format,
                     const GLuint _tex_format)
{
  fname[0] = '\0';

  assert(w > 0 && h > 0);
  dim[0] = w; dim[1] = h;

  filter_mode = GL_LINEAR;
  wrap_mode[0] = GL_REPEAT;
  wrap_mode[1] = GL_REPEAT;
  data_format = _data_format;
  internal_format = _internal_format;
  tex_format = _tex_format;
}

Texture2D::Texture2D(const char *n)
{
  fname = n;
  filter_mode = GL_LINEAR;
  wrap_mode[0] = GL_REPEAT;
  wrap_mode[1] = GL_REPEAT;
  data_format = GL_UNSIGNED_BYTE;
  internal_format = GL_RGBA;
  tex_format = GL_RGBA;
}

Texture2D::~Texture2D()
{
  deinit();
}

void Texture2D::init()
{
  glGenTextures(1, &gl_texture);
  glBindTexture(GL_TEXTURE_2D, gl_texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  assert(glIsTexture(gl_texture) == GL_TRUE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode);

  //for depth textures
#ifndef __LOKI__
  if (internal_format == GL_DEPTH_COMPONENT ||
      internal_format == GL_DEPTH_COMPONENT16 ||
      internal_format == GL_DEPTH_COMPONENT24 ||
      internal_format == GL_DEPTH_COMPONENT32 ||
      internal_format == GL_DEPTH_COMPONENT32F)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  }
#endif //__LOKI__
  
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,
               dim[0],
               dim[1],
               0,
               tex_format,
               data_format,
               NULL);

  assert(glIsTexture(gl_texture) == GL_TRUE);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::deinit()
{
  glDeleteTextures(1, &gl_texture);
}

bool Texture2D::load()
{
    int width, height;

#if defined(__USE_SOIL__)
	int channels;
    unsigned char *image = SOIL_load_image(fname, &width, &height, &channels, SOIL_LOAD_RGBA);
    assert(image);
    dim[0] = width;
    dim[1] = height;
#else
    //SDL2 way of loading
    cout<<"IMG_Load("<<fname<<")"<<endl;
    SDL_Surface *image = IMG_Load(fname.c_str());
    if(!image)
    {
      cerr<<"Texture2D::load() - "<<IMG_GetError()<<endl;
    }

    assert(image);
    dim[0] = width = image->w;
    dim[1] = height = image->h;

    cout<<"image dimensions: "<<dim[0]<<", "<<dim[1]<<endl;

    if(image->format->BytesPerPixel == 3)
    {
      tex_format = GL_RGB;
      internal_format = GL_RGB;
    }
    else if(image->format->BytesPerPixel == 4)
    {
      tex_format = GL_RGBA;
      internal_format = GL_RGBA;
    }

#endif

    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_2D, gl_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if(glIsTexture(gl_texture) != GL_TRUE)
    {
      assert(false);
      return false;
    }

    glTexImage2D(GL_TEXTURE_2D,
                   0,
                   internal_format,
                   width,
                   height,
                   0,
                   tex_format,
                   data_format,
#if defined(__USE_SOIL__)
                   image);
#else
                   image->pixels);
#endif

    //not sure if these should go here, or in the render loop
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode);

#if defined(__USE_SOIL__)
    SOIL_free_image_data(image);
#else
    //SDL_UnlockSurface(image);
    SDL_FreeSurface(image);
#endif

  assert(glIsTexture(gl_texture) == GL_TRUE);

  return true;
}

//set up the texture for rendering
void Texture2D::render_gl(GLuint tex_stage) const
{
  glActiveTexture(tex_stage);
  //glClientActiveTexture(GL_TEXTURE0);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);

  glBindTexture(GL_TEXTURE_2D, gl_texture);
}

void Texture2D::bind_compute_target()
{
  glBindImageTexture(0, gl_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
  gl_check_error();
}

bool Texture2D::update_pixels_from_mem(void *pixels)
{
  int mip_level = 0;
  glBindTexture(GL_TEXTURE_2D, gl_texture);
  //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, dim[0], dim[1]);
  glTexSubImage2D(GL_TEXTURE_2D,
                  mip_level,        //mip level to overwrite
                  0,                //starting u-coord
                  0,                //starting v-coord
                  dim[0],           //width of update rect
                  dim[1],           //height of update rect
                  tex_format,       //pixel format
                  data_format,      //GL_UNSIGNED_BYTE, etc.
                  pixels);          //pointer to pixel data
  return true;
}

Texture3D::Texture3D(const unsigned int w, const unsigned int h, const unsigned int d, const GLuint m)
{
  fname[0] = '\0';

  assert(w > 0 && h > 0 && d > 0);
  dim[0] = w; dim[1] = h; dim[2] = d;

  gl_mode = m;
  wrap_mode[0] = wrap_mode[1] = wrap_mode[2] = GL_REPEAT;
  filter_mode = GL_NEAREST;
}

Texture3D::Texture3D(const char *n)
{
#if defined (_WIN32)
  strcpy_s(fname, n);
#else
  strcpy(fname, n);
#endif
  gl_mode = GL_RGBA;
  wrap_mode[0] = wrap_mode[1] = wrap_mode[2] = GL_REPEAT;
  filter_mode = GL_LINEAR;
}

Texture3D::~Texture3D()
{
  glDeleteTextures(1, &gl_texture);
}

void Texture3D::init()
{
  glGenTextures(1, &gl_texture);
  glBindTexture(GL_TEXTURE_3D, gl_texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  assert(glIsTexture(gl_texture) == GL_TRUE);

  glTexImage3D(GL_TEXTURE_3D,
               0,
               gl_mode,
               dim[0],
               dim[1],
               dim[2],
               0,
               gl_mode,
               GL_UNSIGNED_BYTE,
               NULL);

   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode[0]);
   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode[1]);
   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode[2]);
   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter_mode);
   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter_mode);

   assert(glIsTexture(gl_texture) == GL_TRUE);
}

bool Texture3D::load(const unsigned int depth)
{
    cout<<"Texture3D::load()..."<<endl;
    cout<<"\t"<<fname<<endl;
    int width, height;

#if defined(__USE_SOIL__)
	int channels;
    unsigned char *image = SOIL_load_image(fname, &width, &height, &channels, SOIL_LOAD_RGBA);
    assert(image);
    dim[0] = width / depth;
    dim[1] = height;
    dim[2] = depth;
#else
    //SDL2 way of loading
    cout<<"IMG_Load("<<fname<<")"<<endl;
    SDL_Surface *image = IMG_Load(fname);

    assert(image);
    dim[0] = width = image->w / depth;
    dim[1] = height = image->h;
    dim[2] = depth;

    cout<<"image dimensions: "<<dim[0]<<", "<<dim[1]<<endl;

    if(image->format->BytesPerPixel == 3)
    {
      gl_mode = GL_RGB;
    }
    else if(image->format->BytesPerPixel == 4)
    {
      gl_mode = GL_RGBA;
    }

#endif

    glGenTextures(1, &gl_texture);
    glBindTexture(GL_TEXTURE_3D, gl_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if(glIsTexture(gl_texture) != GL_TRUE)
    {
      assert(false);
      return false;
    }

    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 gl_mode,
                 dim[0],
                 dim[1],
                 dim[2],
                 0,
                 gl_mode,
                 GL_UNSIGNED_BYTE,
#if defined(__USE_SOIL__)
                 image);
#else
                 image->pixels);
#endif

    //not sure if these should go here, or in the render loop
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode[0]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode[1]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode[2]);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter_mode); // Linear Filtering
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter_mode); // Linear Filtering

#if defined(__USE_SOIL__)
    SOIL_free_image_data(image);
#else
    SDL_FreeSurface(image);
#endif

  assert(glIsTexture(gl_texture) == GL_TRUE);

  return true;
}

//set up the texture for rendering
bool Texture3D::render_gl(GLuint tex_stage) const
{
  glActiveTexture(tex_stage);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_3D);
  glDisable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_3D, gl_texture);

  return true;
}
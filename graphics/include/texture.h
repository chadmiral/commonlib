#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "platform_gl.h"
#include <string>

namespace Graphics
{
  typedef struct
  {
    int hash_id;
    char file_name[256];
    int dim[2];

    int img_size;
    unsigned char *image;
  } TextureFileData;

  class Texture2D
  {
  public:
    Texture2D(const unsigned int w = 32,
              const unsigned int h = 32,
              const GLuint _data_format = GL_UNSIGNED_BYTE,
              const GLuint _internal_format = GL_RGBA,
              const GLuint _tex_format = GL_RGBA);
    Texture2D(const char *n);

    ~Texture2D();

    GLuint get_tex_id() const { return gl_texture; }
    void get_dim(int &w, int &h) const { w = dim[0]; h = dim[1]; }
    void set_dim(const int w, const int h) { dim[0] = w; dim[1] = h; }
    GLenum get_tex_format() const { return tex_format; }
    void set_tex_format(const GLenum tf) { tex_format = tf; }
    GLint get_internal_format() const { return internal_format; }
    void set_internal_format(const GLint int_f) { internal_format = int_f; }
    GLenum get_data_format() const { return data_format; }
    void set_data_format(const GLenum df) { data_format = df; }
    GLuint get_wrap_mode_u() { return wrap_mode[0]; }
    GLuint get_wrap_mode_v() { return wrap_mode[1]; }
    void set_wrap_mode(GLuint u, GLuint v) { wrap_mode[0] = u; wrap_mode[1] = v; }
    GLuint get_filtering_mode() { return filter_mode; }
    void set_filtering_mode(GLuint f) { filter_mode = f; }
    void set_resolution(const unsigned int w, const unsigned int h) { dim[0] = w; dim[1] = h; }

    void set_filename(const char *fn) { fname = fn; }

    void init();
    void deinit();

    bool load();
    //bool load_from_file_data(TextureFileData &tfd);
    void render_gl(GLuint tex_stage = GL_TEXTURE0) const;
    void bind_compute_target();

    bool update_pixels_from_mem(void *pixels);

  private:
    unsigned int       dim[2];          //dimensions of the texture
    std::string        fname;           //filename
    GLenum             tex_format;
    GLenum             data_format;
    GLint              internal_format;
    GLuint             gl_texture;      //OpenGL texture name
    GLuint             filter_mode;     //GL_LINEAR, GL_NEAREST, etc...
    GLuint             wrap_mode[2];    //wrapping mode
  };

  class Texture3D
  {
  public:
    Texture3D(const unsigned int w = 32, const unsigned int h = 32, const unsigned int d = 32, const GLuint m = GL_RGBA);
    Texture3D(const char *n);

    ~Texture3D();

    GLuint get_tex_id() const { return gl_texture; }
    void get_dim(int &w, int &h) const { w = dim[0]; h = dim[1]; }
    void set_dim(const int w, const int h) { dim[0] = w; dim[1] = h; }
    GLuint get_pixel_mode() { return gl_mode; }
    void set_pixel_mode(GLuint m) { gl_mode = m; }
    void set_wrap_mode(GLuint s, GLuint t, GLuint r) { wrap_mode[0] = s; wrap_mode[1] = t; wrap_mode[2] = r; }
    void set_filter_mode(GLuint f) { filter_mode = f; }

    void init();

    bool load(const unsigned int depth);
    bool load_from_file_data(TextureFileData &tfd);
    bool render_gl(GLuint tex_stage = GL_TEXTURE0) const;

    bool update_pixels_from_mem(void *pixels);
  private:
    unsigned int       dim[3];
    char               fname[256];
    GLuint             gl_texture;
    GLuint             gl_mode;
    GLuint             wrap_mode[3];
    GLuint             filter_mode;
  };
};

#endif // _TEXTURE_H_

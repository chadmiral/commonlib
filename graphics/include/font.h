#ifndef __FONT_H__
#define __FONT_H__

#include "platform_gl.h"

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftoutln.h>
#include <fttrigon.h>

namespace Graphics
{
  class Font
  {
  public:
    //Font();
    Font(const char *fname, unsigned int height);
    ~Font();

    void init();
    void print(float x, float y, const char *fmt, ...);
    float get_string_width(char *s) const;
    float get_height() const { return h; }
  private:
    void create_display_list(FT_Face face, char ch, GLuint list_base, GLuint *tex_base);
    void pushScreenCoordinateMatrix();

    float h;                //font height
    float character_w[128]; //character widths
    char face_fname[256];   //font face name
    GLuint *textures;       //texture ids
    GLuint list_base;       //first display list id
    GLint viewport[4];
  };
};

//void gl_print(const Font &font, float x, float y, const char *fmt, ...);

#endif // __FONT_H__

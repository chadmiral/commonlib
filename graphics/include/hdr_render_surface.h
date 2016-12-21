#ifndef __HDR_RENDER_SURFACE_H__
#define __HDR_RENDER_SURFACE_H__

#include "render_surface.h"

//TODO: this whole setup is super hacky... total rewrite?

namespace Graphics
{
  class HDRRenderSurface : public RenderSurface
  {
  public:
    HDRRenderSurface(const int w = 256, const int h = 256);
    ~HDRRenderSurface();

    void set_exposure(const float e) { exposure = e; }
    void set_bloom_threshold(const float b) { bloom_threshold = b; }

    virtual void init();
    virtual void deinit();
    virtual void render();
    void render_method_2();

    Material *get_mat2() { return &mat2; }
  private:
    float                  exposure;
    float                  bloom_threshold;

    Shader                 *shader1;
    Shader                 *shader2;
    Material               mat2;
  };
};

#endif //__HDR_RENDER_SURFACE_H__

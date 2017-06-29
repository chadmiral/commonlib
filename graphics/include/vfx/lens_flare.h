#ifndef __LENS_FLARE_H__
#define __LENS_FLARE_H__

#include "object_3d.h"
#include "material.h"
#include "vector.h"
#include "renderable.h"

//
// Currently flying from SEA -> TUS (12.2.16)
//

namespace Graphics
{
  struct LensFlareVertex
  {
    float x, y, z;
    float u0, v0;
  };
  
  class LensFlareElement : public Renderable<LensFlareVertex>
  {
    friend class LensFlare;
  private:    
    Graphics::ShaderVertexAttrib       _xyz_attrib;
    Graphics::ShaderVertexAttrib       _uv0_attrib;

    Graphics::ShaderUniformMatrix4x4  *_proj_mat;
    Graphics::ShaderUniformFloat2     *_center_point;
    Graphics::ShaderUniformFloat2     *_screen_resolution;

    Graphics::ShaderUniformFloat4     *_tint;
    Graphics::ShaderUniformFloat2     *_scale;
    Graphics::ShaderUniformFloat2     *_position_offset;
    Graphics::ShaderUniformFloat      *_element_depth;
    
  public:
    LensFlareElement() : Renderable<LensFlareVertex>() {}
    ~LensFlareElement() { deallocate_buffers(); }
    
    void set_position_offset(Math::Float2 pos_offset) { _position_offset->set_var(pos_offset); }
    void set_scale(Math::Float2 scale) { _scale->set_var(scale); }
    void set_rotation_offset(float rot_offset) {  }
    void set_tint(Math::Float4 c) { _tint->set_var(c); }
    void set_element_depth(float d) { _element_depth->set_var(d); }
    
    virtual void init();
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);
  };
  
  
  class LensFlare : public Game::Object3D//, public Renderable<LensFlareVertex>
  {
  private:
    std::vector<LensFlareElement> _elements;
    
    float _occlusion_radius;
    
  public:
    LensFlare();
    ~LensFlare();
    
    void set_occlusion_radius(float r) { _occlusion_radius = r; }

    void set_screen_resolution(Math::Float2 sr)
    {
      for (uint32_t i = 0; i < _elements.size(); i++)
      {
        _elements[i]._screen_resolution->set_var(sr);
      }
    }
    void set_center_point(Math::Float2 cp)
    {
      for (uint32_t i = 0; i < _elements.size(); i++)
      {
        _elements[i]._center_point->set_var(cp);
      }
    }

    void add_element(LensFlareElement &element)
    {
      _elements.push_back(element);
    }

    uint32_t size() const { return _elements.size(); }

    inline LensFlareElement &operator[](const int idx) { return _elements[idx]; }
    inline LensFlareElement operator[](const int idx) const { return _elements[idx]; }
    
    virtual void init(const double game_time);
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);
  };
};

#endif //__LENS_FLARE_H__

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
  private:
    Math::Float2                    _position_offset;   //TODO: uniform variables?
    Math::Float2                    _scale;
    float                           _rotation_offset;
    
    Graphics::ShaderVertexAttrib    _xyz_attrib;
    Graphics::ShaderVertexAttrib    _uv0_attrib;
    
  public:
    LensFlareElement() : Renderable<LensFlareVertex>() { _rotation_offset = 0.0f; }
    //LensFlareElement(Graphics::Material *mat, Math::Float2 pos_offset, Math::Float2 scale, float rot_offset);
    ~LensFlareElement() { deallocate_buffers(); }
    
    void set_position_offset(Math::Float2 pos_offset) { _position_offset = pos_offset; }
    void set_scale(Math::Float2 scale) { _scale = scale; }
    void set_rotation_offset(float rot_offset) { _rotation_offset = rot_offset; }
    
    virtual void init();
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);
  };
  
  
  class LensFlare : public Game::Object3D//, public Renderable<LensFlareVertex>
  {
  private:
    std::vector<LensFlareElement> _elements;
    
    /*GLuint _vbo;
    GLuint _ibo;
    
    uint32_t _num_vertices;
    uint32_t _num_indices;
    
    LensFlareVertex *_vertices;
    uint32_t        *_indices;
    */
    
    float _occlusion_radius;
    
  public:
    LensFlare();
    ~LensFlare();
    
    void set_occlusion_radius(float r) { _occlusion_radius = r; }
    void add_element(LensFlareElement &element) { _elements.push_back(element); }
    
    virtual void init(const double game_time);
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);
  };
};

#endif //__LENS_FLARE_H__

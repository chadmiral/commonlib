#pragma once

#include "object_3d.h"
#include "static_mesh.h"

namespace Game
{
  class Pawn3D : public Object3D
  {
  protected:
    //Graphics::Renderable<StaticMeshVertex> renderable; //mesh, material, shader, etc...
    Graphics::Material                mat;
    Graphics::Shader                  *shader;
    Graphics::ShaderUniformFloat3     sun_pos;
    Graphics::ShaderUniformFloat3     sun_amb_rgb;
    Graphics::ShaderUniformFloat3     sun_diff_rgb;
    Graphics::ShaderUniformFloat3     sun_spec_rgb;
    Graphics::ShaderUniformFloat3     cam_distance;
    Graphics::StaticMesh              mesh;

    //TODO: components, etc
  public:
    Pawn3D() {}
    ~Pawn3D() {}

    virtual void init() = 0;
    virtual void simulate() = 0;
    virtual void render(const double game_time, const double frame_time);
  };
};
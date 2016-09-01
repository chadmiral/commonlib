#pragma once

#include "object_3d.h"
#include "static_mesh.h"
#include "sdl_game.h"

namespace Game
{
  class Pawn3D : public Object3D
  {
  protected:
    //Graphics::Renderable<StaticMeshVertex> renderable; //mesh, material, shader, etc...
    Graphics::Material                mat;
    Graphics::ShaderUniformFloat3     sun_pos;
    Graphics::ShaderUniformFloat3     sun_amb_rgb;
    Graphics::ShaderUniformFloat3     sun_diff_rgb;
    Graphics::ShaderUniformFloat3     sun_spec_rgb;
    Graphics::ShaderUniformFloat3     cam_distance;
    Graphics::StaticMesh              mesh;

    //TODO: components, etc

    GameContext *game_context;
  public:
    Pawn3D(GameContext *gc) { game_context = gc; }
    ~Pawn3D() {}
    
    Graphics::Material *get_material() { return &mat; }
    Graphics::StaticMesh *get_mesh() { return &mesh; }

    virtual void init();
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);
  };
};
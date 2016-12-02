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
    Graphics::Material                _mat;
    Graphics::ShaderUniformFloat3     _sun_pos;
    Graphics::ShaderUniformFloat3     _sun_amb_rgb;
    Graphics::ShaderUniformFloat3     _sun_diff_rgb;
    Graphics::ShaderUniformFloat3     _sun_spec_rgb;
    Graphics::ShaderUniformFloat3     _cam_distance;
    Graphics::StaticMesh             *_mesh;

    //TODO: components, etc

    GameContext *_game_context;
  public:
    Pawn3D(GameContext *gc) { _game_context = gc; _mesh = NULL; }
    ~Pawn3D() {}
    
    Graphics::Material *get_material() { return &_mat; }
    Graphics::StaticMesh *get_mesh() { return _mesh; }
    void set_mesh(Graphics::StaticMesh *m) { _mesh = m; }

    virtual void init();
    virtual void simulate(const double game_time, const double frame_time);
    virtual void render(const double game_time);
  };
};
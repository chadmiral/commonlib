#pragma once

#include "object_3d.h"
#include "static_mesh.h"

namespace Game
{
  class Pawn3D : public Object3D
  {
  protected:
    Graphics::Renderable<StaticMeshVertex> renderable; //mesh, material, shader, etc...

    //TODO: components, etc
  public:
    Pawn3D() {}
    ~Pawn3D() {}

    virtual void simulate() = 0;
    virtual void render(const double game_time, const double frame_time);
  };
};
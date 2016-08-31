#include "pawn.h"

using namespace Game;

void Pawn3D::init()
{

}

void Pawn3D::render(const double game_time)
{
  Object3D::render(game_time);

  mat.render();
  mesh.render();
}

void Pawn3D::simulate(const double game_time, const double frame_time)
{

}
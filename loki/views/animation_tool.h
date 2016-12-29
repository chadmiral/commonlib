#pragma once

#include <imgui.h>

#include "static_mesh.h"
#include "render_surface.h"
#include "camera.h"
#include "string.h"
#include "skeleton.h"

class AnimationTool
{
private:
  Animation::SkeletonAnimation *_anim;
  std::string                   _anim_fname;
  std::string                   _data_root; //TODO: app level variable
public:
  AnimationTool();
  ~AnimationTool() {}

  void init() { _anim = NULL; }
  void deinit() {}

  void render();

  bool _visible;
private:
  void load_animation();
};
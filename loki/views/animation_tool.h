#pragma once

#include <imgui.h>

#include "static_mesh.h"
#include "render_surface.h"
#include "camera.h"
#include "string.h"
#include "skeleton.h"

struct UIBone
{
  uint32_t                _bone_id;
  UIBone                 *_parent;
  std::vector<UIBone *>   _children;
};

class AnimationTool
{
private:
  Animation::SkeletonAnimation *_anim;
  std::string                   _anim_fname;
  std::string                   _data_root; //TODO: app level variable
  UIBone                       *_skeleton_tree;
  std::vector<UIBone *>         _all_bones;
  uint32_t                      _selected_bone;
public:
  AnimationTool();
  ~AnimationTool() {}

  void init() { _anim = NULL; }
  void deinit() {}

  void render();

  bool _visible;
private:
  void render_menu();
  void render_animation_curves();
  void load_animation();
  void build_skeleton_tree();
};